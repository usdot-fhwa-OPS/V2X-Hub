#!/usr/bin/env python3
"""Detect stalls in the FLIR camera detection stream and help pinpoint where frames are being held.

A stall is when the camera keeps producing frames (its timestamps stay ~100 ms apart) but they stop arriving,
then arrive late all at once. Downstream services discard detections that are this late, so a stall shows up
as a gap followed by a burst on the detection topic.

Subcommands
-----------
live
    Subscribes to the camera as an independent WebSocket client (it can run next to the V2X Hub plugin) and,
    at the same time, repeatedly opens TCP connections to the camera and to optional control hosts. When a
    stall happens it reports whether the camera stayed reachable, which separates a link outage from a
    sender-side hold, and records how the backlog arrived at the TCP level.

        python3 flir_stream_probe.py live --host 192.168.55.83 --control 192.168.55.114:22

analyze
    Finds stalls in existing logs: V2X Hub plugin logs (the plugin's "FLIR link stall" warnings, or the DEBUG
    "Received:" lines from older builds), and optionally correlates them with sensor data sharing service
    discards, other Kafka traffic and the events recorded by "live".

        python3 flir_stream_probe.py analyze --v2xhub v2xhub_pc2.log --sdss sdss.log \\
            --other-traffic kafka_topics/v2xhub_bsm_in.log --probe-events flir_probe_*/events.jsonl

Uses only the Python standard library (3.8+).
"""
import argparse
import base64
import bisect
import calendar
import csv
import hashlib
import json
import os
import re
import select
import socket
import struct
import sys
import threading
import time
from collections import deque
from dataclasses import asdict, dataclass
from datetime import datetime, timezone
from typing import Deque, Dict, List, Optional, Tuple

SUBSCRIPTION = {
    "messageType": "Subscription",
    "subscription": {
        "type": "Data",
        "action": "Subscribe",
        "inclusions": [{"type": "PedestrianPresenceTracking"}],
    },
}


# --------------------------------------------------------------------------------------------------------------
# Time helpers
# --------------------------------------------------------------------------------------------------------------

def now_ms() -> int:
    return time.time_ns() // 1_000_000


def utc(ms: Optional[int]) -> str:
    if ms is None:
        return "-"
    return datetime.fromtimestamp(ms / 1000, tz=timezone.utc).strftime("%Y-%m-%dT%H:%M:%S.%f")[:-3] + "Z"


_CAMERA_TIME_RE = re.compile(r"(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})\.(\d+)([+-])(\d{2}):(\d{2})$")


def parse_camera_time(text: str) -> int:
    """Parse a camera timestamp such as 2026-09-15T13:47:10.032-04:00 into ms since epoch.

    Mirrors the plugin's timeStringParser so both report the same values.
    """
    m = _CAMERA_TIME_RE.match(text.strip())
    if not m:
        dt = datetime.fromisoformat(text.strip().replace("Z", "+00:00"))
        if dt.tzinfo is None:
            raise ValueError(f"camera time without UTC offset: {text}")
        return round(dt.timestamp() * 1000)
    year, month, day, hour, minute, second = (int(m.group(i)) for i in range(1, 7))
    millis = int(m.group(7))
    offset = (int(m.group(9)) * 3600 + int(m.group(10)) * 60) * (-1 if m.group(8) == "-" else 1)
    seconds = calendar.timegm((year, month, day, hour, minute, second, 0, 0, 0)) - offset
    return seconds * 1000 + millis


def parse_iso_utc(text: str) -> int:
    return round(datetime.fromisoformat(text.replace("Z", "+00:00")).timestamp() * 1000)


# --------------------------------------------------------------------------------------------------------------
# Stall detection (same rules as FLIRStreamMonitor in the plugin)
# --------------------------------------------------------------------------------------------------------------

@dataclass
class Burst:
    """Consecutive frames that arrived late, i.e. the backlog delivered after a stall."""
    first_recv_ms: int
    last_recv_ms: int
    first_camera_ms: int
    last_camera_ms: int
    frames: int = 0
    max_excess_ms: int = 0
    first_data_number: Optional[int] = None
    last_data_number: Optional[int] = None


@dataclass
class Report:
    recv_ms: int
    camera_ms: int
    data_number: Optional[int]
    latency_ms: int
    excess_ms: int
    prev_recv_ms: Optional[int] = None
    prev_camera_ms: Optional[int] = None
    prev_data_number: Optional[int] = None
    arrival_gap_ms: int = 0
    camera_gap_ms: int = 0
    hold_ms: int = 0
    hold: str = "none"  # none | minor | stall
    missing: int = 0
    late: bool = False
    completed_burst: Optional[Burst] = None


class StreamMonitor:
    """Compares frame arrival times with camera timestamps.

    stall_ms:    a frame held at least this much longer in transit than the previous frame, while the camera kept
                 producing frames, is a stall; frames at least this much above baseline latency are late.
    minor_ms:    holds from this value up to stall_ms are minor holds (~200 ms = one TCP retransmission).
    track_gap_ms: camera timestamps further apart than this mean the camera went quiet (nothing tracked).
    baseline_window_ms: window for the normal latency estimate, which absorbs camera/host clock offset.
    """

    def __init__(self, stall_ms: int = 500, minor_ms: int = 150, track_gap_ms: int = 1000,
                 baseline_window_ms: int = 10000):
        self.stall_ms = stall_ms
        self.minor_ms = minor_ms
        self.track_gap_ms = track_gap_ms
        self.baseline_window_ms = baseline_window_ms
        self._window: Deque[Tuple[int, int]] = deque()
        self._last_baseline: Optional[int] = None
        self._prev: Optional[Tuple[int, int, Optional[int]]] = None
        self._burst: Optional[Burst] = None
        self.stats = {"frames": 0, "stalls": 0, "minor_holds": 0, "late_frames": 0, "missing_frames": 0,
                      "max_stall_ms": 0}

    def _baseline(self, recv_ms: int, latency_ms: int) -> int:
        while self._window and recv_ms - self._window[0][0] > self.baseline_window_ms:
            self._window.popleft()
        if self._window:
            baseline = min(latency for _, latency in self._window)
        elif self._last_baseline is not None:
            baseline = self._last_baseline
        else:
            baseline = latency_ms
        self._last_baseline = baseline
        self._window.append((recv_ms, latency_ms))
        return baseline

    def _close(self) -> Optional[Burst]:
        burst, self._burst = self._burst, None
        return burst

    def on_frame(self, recv_ms: int, camera_ms: int, data_number: Optional[int]) -> Report:
        latency = recv_ms - camera_ms
        report = Report(recv_ms, camera_ms, data_number, latency, latency - self._baseline(recv_ms, latency))
        self.stats["frames"] += 1
        continuous = False
        if self._prev is not None:
            prev_recv, prev_camera, prev_dn = self._prev
            report.prev_recv_ms, report.prev_camera_ms, report.prev_data_number = prev_recv, prev_camera, prev_dn
            report.arrival_gap_ms = recv_ms - prev_recv
            report.camera_gap_ms = camera_ms - prev_camera
            report.hold_ms = report.arrival_gap_ms - report.camera_gap_ms
            continuous = 0 <= report.camera_gap_ms <= self.track_gap_ms
            if continuous:
                if report.hold_ms >= self.stall_ms:
                    report.hold = "stall"
                    self.stats["stalls"] += 1
                    self.stats["max_stall_ms"] = max(self.stats["max_stall_ms"], report.hold_ms)
                elif report.hold_ms >= self.minor_ms:
                    report.hold = "minor"
                    self.stats["minor_holds"] += 1
                if data_number is not None and prev_dn is not None and data_number > prev_dn + 1:
                    report.missing = data_number - prev_dn - 1
                    self.stats["missing_frames"] += report.missing
        if not continuous and self._burst is not None:
            report.completed_burst = self._close()
        report.late = report.excess_ms >= self.stall_ms
        if report.late:
            self.stats["late_frames"] += 1
            if self._burst is None:
                self._burst = Burst(recv_ms, recv_ms, camera_ms, camera_ms, first_data_number=data_number)
            burst = self._burst
            burst.last_recv_ms, burst.last_camera_ms, burst.last_data_number = recv_ms, camera_ms, data_number
            burst.frames += 1
            burst.max_excess_ms = max(burst.max_excess_ms, report.excess_ms)
        elif self._burst is not None:
            report.completed_burst = self._close()
        self._prev = (recv_ms, camera_ms, data_number)
        return report

    def close_if_idle(self, now: int) -> Optional[Burst]:
        if self._burst is not None and self._prev is not None and now - self._prev[0] > self.track_gap_ms:
            return self._close()
        return None

    def flush(self) -> Optional[Burst]:
        return self._close()


# --------------------------------------------------------------------------------------------------------------
# live: independent WebSocket subscriber + reachability probes
# --------------------------------------------------------------------------------------------------------------

_WS_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"


class WebSocketClient:
    """Minimal RFC 6455 client: enough to subscribe to the camera and read its text frames."""

    def __init__(self, host: str, port: int, path: str, timeout_s: float):
        self.host, self.port, self.path, self.timeout_s = host, port, path, timeout_s
        self.sock: Optional[socket.socket] = None
        self._rx = bytearray()
        self._fragments: List[bytes] = []
        self._fragment_opcode: Optional[int] = None

    def connect(self) -> None:
        sock = socket.create_connection((self.host, self.port), timeout=self.timeout_s)
        sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        key = base64.b64encode(os.urandom(16)).decode()
        request = (f"GET {self.path} HTTP/1.1\r\nHost: {self.host}:{self.port}\r\nUpgrade: websocket\r\n"
                   f"Connection: Upgrade\r\nSec-WebSocket-Key: {key}\r\nSec-WebSocket-Version: 13\r\n"
                   f"User-Agent: flir-stream-probe\r\n\r\n")
        sock.sendall(request.encode())
        data = b""
        while b"\r\n\r\n" not in data:
            chunk = sock.recv(4096)
            if not chunk:
                raise ConnectionError("connection closed during WebSocket handshake")
            data += chunk
            if len(data) > 65536:
                raise ConnectionError("WebSocket handshake response too large")
        head, rest = data.split(b"\r\n\r\n", 1)
        lines = head.decode("latin-1").split("\r\n")
        if len(lines[0].split()) < 2 or lines[0].split()[1] != "101":
            raise ConnectionError(f"WebSocket handshake rejected: {lines[0]}")
        headers = {k.strip().lower(): v.strip() for k, v in (line.split(":", 1) for line in lines[1:] if ":" in line)}
        expected = base64.b64encode(hashlib.sha1((key + _WS_GUID).encode()).digest()).decode()
        if headers.get("sec-websocket-accept") != expected:
            raise ConnectionError("WebSocket handshake returned an unexpected Sec-WebSocket-Accept")
        self.sock = sock
        self._rx = bytearray(rest)
        self._fragments, self._fragment_opcode = [], None

    def send(self, opcode: int, payload: bytes) -> None:
        header = bytearray([0x80 | opcode])
        length = len(payload)
        if length < 126:
            header.append(0x80 | length)
        elif length < 65536:
            header.append(0x80 | 126)
            header += struct.pack("!H", length)
        else:
            header.append(0x80 | 127)
            header += struct.pack("!Q", length)
        mask = os.urandom(4)
        header += mask
        masked = bytes(b ^ mask[i % 4] for i, b in enumerate(payload))
        assert self.sock is not None
        self.sock.sendall(bytes(header) + masked)

    def feed(self, data: bytes) -> List[Tuple[int, bytes]]:
        """Add received bytes; return the complete messages as (opcode, payload)."""
        self._rx += data
        messages = []
        while len(self._rx) >= 2:
            b0, b1 = self._rx[0], self._rx[1]
            fin, opcode, masked, length, pos = b0 & 0x80, b0 & 0x0F, b1 & 0x80, b1 & 0x7F, 2
            if length == 126:
                if len(self._rx) < 4:
                    break
                length, pos = struct.unpack("!H", bytes(self._rx[2:4]))[0], 4
            elif length == 127:
                if len(self._rx) < 10:
                    break
                length, pos = struct.unpack("!Q", bytes(self._rx[2:10]))[0], 10
            mask = b""
            if masked:
                if len(self._rx) < pos + 4:
                    break
                mask, pos = bytes(self._rx[pos:pos + 4]), pos + 4
            if len(self._rx) < pos + length:
                break
            payload = bytes(self._rx[pos:pos + length])
            del self._rx[:pos + length]
            if masked:
                payload = bytes(b ^ mask[i % 4] for i, b in enumerate(payload))
            if opcode >= 8:  # control frames are never fragmented
                messages.append((opcode, payload))
                continue
            if opcode != 0:
                self._fragment_opcode, self._fragments = opcode, [payload]
            else:
                self._fragments.append(payload)
            if fin and self._fragment_opcode is not None:
                messages.append((self._fragment_opcode, b"".join(self._fragments)))
                self._fragments, self._fragment_opcode = [], None
        return messages

    def close(self) -> None:
        if self.sock is not None:
            try:
                self.sock.close()
            finally:
                self.sock = None


class ConnectProbe(threading.Thread):
    """Repeatedly opens (and immediately closes) a TCP connection to host:port and records how long it took.

    During a link outage the connection request is lost and the connect takes about 1 s longer (SYN
    retransmission) or times out, independently of the detection stream.
    """

    def __init__(self, label: str, host: str, port: int, interval_s: float, timeout_s: float):
        super().__init__(daemon=True)
        self.label, self.host, self.port = label, host, port
        self.interval_s, self.timeout_s = interval_s, timeout_s
        self._results: Deque[Tuple[int, int, bool, str]] = deque(maxlen=20000)  # start_ms, duration_ms, ok, error
        self._lock = threading.Lock()
        self._stop = threading.Event()

    def run(self) -> None:
        while not self._stop.is_set():
            start = now_ms()
            began = time.monotonic()
            ok, error = True, ""
            try:
                with socket.create_connection((self.host, self.port), timeout=self.timeout_s):
                    pass
            except OSError as e:
                ok, error = False, type(e).__name__
            duration = int((time.monotonic() - began) * 1000)
            with self._lock:
                self._results.append((start, duration, ok, error))
            self._stop.wait(max(0.0, self.interval_s - (time.monotonic() - began)))

    def stop(self) -> None:
        self._stop.set()

    def summarize(self, start_ms: int, end_ms: int, slow_ms: int) -> Dict:
        with self._lock:
            results = [r for r in self._results if start_ms <= r[0] <= end_ms]
        bad = [r for r in results if not r[2] or r[1] >= slow_ms]
        return {
            "target": self.label,
            "sent": len(results),
            "failed_or_slow": len(bad),
            "worst_ms": max((r[1] for r in results), default=None),
            "first_bad": utc(bad[0][0]) if bad else None,
            "last_bad": utc(bad[-1][0]) if bad else None,
        }


def probe_verdict(camera: Optional[Dict], controls: List[Dict]) -> str:
    if camera is None:
        return "No reachability probes were running (--no-probe)."
    if camera["sent"] == 0:
        return "No camera probe fell inside the stall window; use a shorter --probe-interval."
    camera_bad = camera["failed_or_slow"] > 0
    controls_probed = [c for c in controls if c["sent"] > 0]
    controls_bad = any(c["failed_or_slow"] > 0 for c in controls_probed)
    controls_ok = bool(controls_probed) and not controls_bad
    if camera_bad and controls_bad:
        return ("The camera and the control targets were all unreachable: the problem is on this host's side or in "
                "shared equipment (this host's network interface or VM networking, the switch).")
    if camera_bad and controls_ok:
        return ("Only the camera was unreachable while the control targets answered: packets were lost between the "
                "shared network and the camera (camera network interface, its cable, PoE/BPL link or switch port).")
    if camera_bad:
        return ("The camera stopped answering new TCP connections during the stall, so the network path to it was "
                "down. Add --control HOST:PORT to tell this host's network apart from the camera's link.")
    return ("The camera kept answering new TCP connections throughout the stall, so the network path was up. The "
            "frames were held by the camera's sender, or by this connection's TCP retransmission backoff after a "
            "loss too short for the probes to catch.")


class EventLog:
    def __init__(self, path: str):
        self._file = open(path, "a", buffering=1)

    def write(self, event_type: str, **fields) -> None:
        record = {"type": event_type, "time": utc(now_ms())}
        record.update(fields)
        self._file.write(json.dumps(record) + "\n")

    def close(self) -> None:
        self._file.close()


def run_live(args: argparse.Namespace) -> int:
    out_dir = args.out or f"flir_probe_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
    os.makedirs(out_dir, exist_ok=True)
    events = EventLog(os.path.join(out_dir, "events.jsonl"))
    frames_file = open(os.path.join(out_dir, "frames.csv"), "w", newline="", buffering=1)
    chunks_file = open(os.path.join(out_dir, "chunks.csv"), "w", newline="", buffering=1)
    frames_csv, chunks_csv = csv.writer(frames_file), csv.writer(chunks_file)
    frames_csv.writerow(["recv_utc", "recv_ms", "camera_ms", "data_number", "tracks", "latency_ms", "excess_ms",
                         "arrival_gap_ms", "camera_gap_ms", "hold_ms", "hold", "missing", "late"])
    chunks_csv.writerow(["recv_utc", "recv_ms", "bytes"])

    probes: List[ConnectProbe] = []
    camera_probe: Optional[ConnectProbe] = None
    if not args.no_probe:
        camera_probe = ConnectProbe(f"camera {args.host}:{args.port}", args.host, args.port,
                                    args.probe_interval, args.probe_timeout)
        probes.append(camera_probe)
        for target in args.control:
            host, _, port = target.rpartition(":")
            if not host or not port.isdigit():
                print(f"--control expects HOST:PORT, got {target}", file=sys.stderr)
                return 2
            probes.append(ConnectProbe(f"control {target}", host, int(port), args.probe_interval, args.probe_timeout))
        for probe in probes:
            probe.start()

    monitor = StreamMonitor(stall_ms=args.stall_ms)
    recent_chunks: Deque[Tuple[int, int]] = deque(maxlen=5000)
    pending_stalls: List[Dict] = []
    probe_settle_ms = int(args.probe_timeout * 1000) + 500

    print(f"Writing results to {out_dir}/ (events.jsonl, frames.csv, chunks.csv). Ctrl-C to stop.")
    events.write("start", host=args.host, port=args.port, path=args.path, stall_ms=args.stall_ms,
                 probes=[p.label for p in probes])

    def report_burst(burst: Burst) -> None:
        fields = asdict(burst)
        fields.update(first_recv_utc=utc(burst.first_recv_ms), last_recv_utc=utc(burst.last_recv_ms),
                      first_camera_utc=utc(burst.first_camera_ms), last_camera_utc=utc(burst.last_camera_ms))
        events.write("late_burst", **fields)
        print(f"{utc(burst.first_recv_ms)}  LATE BURST  {burst.frames} frame(s) >= {args.stall_ms} ms late "
              f"(worst {burst.max_excess_ms} ms), delivered over {burst.last_recv_ms - burst.first_recv_ms} ms, "
              f"dataNumber {burst.first_data_number} -> {burst.last_data_number}")

    def settle_stalls(force: bool = False) -> None:
        now = now_ms()
        for stall in list(pending_stalls):
            if not force and now < stall["end_ms"] + probe_settle_ms:
                continue
            pending_stalls.remove(stall)
            chunks = [(t - stall["end_ms"], n) for t, n in recent_chunks if stall["end_ms"] - 5 <= t <= stall["end_ms"] + 500]
            stall["chunks_after_stall"] = [{"offset_ms": o, "bytes": n} for o, n in chunks[:8]]
            camera = camera_probe.summarize(stall["start_ms"], stall["end_ms"], args.slow_ms) if camera_probe else None
            controls = [p.summarize(stall["start_ms"], stall["end_ms"], args.slow_ms) for p in probes if p is not camera_probe]
            stall["camera_probe"], stall["control_probes"] = camera, controls
            stall["verdict"] = probe_verdict(camera, controls)
            events.write("stall", **stall)
            shape = ", ".join(f"+{c['offset_ms']}ms/{c['bytes']}B" for c in stall["chunks_after_stall"])
            print(f"{stall['start_utc']}  STALL details ({stall['hold_ms']} ms, ended {stall['end_utc']}):")
            print(f"    TCP data after the stall (offset/bytes): {shape or '-'}")
            for summary in ([camera] if camera else []) + controls:
                print(f"    probe {summary['target']}: {summary['sent']} sent, {summary['failed_or_slow']} failed/slow, "
                      f"worst {summary['worst_ms']} ms")
            print(f"    -> {stall['verdict']}")

    deadline = time.monotonic() + args.duration if args.duration > 0 else None
    reconnect_delay = 1.0
    try:
        while deadline is None or time.monotonic() < deadline:
            client = WebSocketClient(args.host, args.port, args.path, args.connect_timeout)
            try:
                client.connect()
                client.send(0x1, json.dumps(SUBSCRIPTION).encode())
            except OSError as e:
                events.write("connect_failed", error=str(e))
                print(f"{utc(now_ms())}  connect to {args.host}:{args.port}{args.path} failed: {e}")
                client.close()
                settle_stalls()
                time.sleep(reconnect_delay)
                reconnect_delay = min(reconnect_delay * 2, 30.0)
                continue
            reconnect_delay = 1.0
            events.write("connected")
            print(f"{utc(now_ms())}  subscribed to {args.host}:{args.port}{args.path}")
            # Frames that arrived together with the handshake response
            pending = [(now_ms(), opcode, payload) for opcode, payload in client.feed(b"")]
            try:
                while deadline is None or time.monotonic() < deadline:
                    messages, pending = pending, []
                    assert client.sock is not None
                    readable, _, _ = select.select([client.sock], [], [], 0.1)
                    if readable:
                        data = client.sock.recv(65536)
                        recv = now_ms()
                        if not data:
                            raise ConnectionError("camera closed the connection")
                        recent_chunks.append((recv, len(data)))
                        chunks_csv.writerow([utc(recv), recv, len(data)])
                        messages += [(recv, opcode, payload) for opcode, payload in client.feed(data)]
                    for recv, opcode, payload in messages:
                        if opcode == 0x9:
                            client.send(0xA, payload)
                            continue
                        if opcode == 0x8:
                            raise ConnectionError("camera sent a WebSocket close")
                        if opcode != 0x1:
                            continue
                        try:
                            message = json.loads(payload)
                        except ValueError:
                            continue
                        if message.get("messageType") != "Data" or "time" not in message:
                            if message.get("messageType") == "Subscription":
                                print(f"{utc(recv)}  subscription response: {json.dumps(message.get('subscription'))}")
                            continue
                        data_number = None
                        try:
                            data_number = int(message.get("dataNumber"))
                        except (TypeError, ValueError):
                            pass
                        try:
                            camera_ms = parse_camera_time(str(message["time"]))
                        except ValueError:
                            events.write("bad_frame", recv_utc=utc(recv), time=str(message["time"]))
                            continue
                        report = monitor.on_frame(recv, camera_ms, data_number)
                        tracks = len(message.get("track") or [])
                        frames_csv.writerow([utc(recv), recv, report.camera_ms, data_number, tracks, report.latency_ms,
                                             report.excess_ms, report.arrival_gap_ms, report.camera_gap_ms,
                                             report.hold_ms, report.hold, report.missing, int(report.late)])
                        if report.completed_burst:
                            report_burst(report.completed_burst)
                        if report.hold == "stall":
                            pending_stalls.append({
                                "start_ms": report.prev_recv_ms, "end_ms": recv,
                                "start_utc": utc(report.prev_recv_ms), "end_utc": utc(recv),
                                "hold_ms": report.hold_ms, "arrival_gap_ms": report.arrival_gap_ms,
                                "camera_gap_ms": report.camera_gap_ms, "latency_ms": report.latency_ms,
                                "prev_data_number": report.prev_data_number, "data_number": data_number,
                                "missing": report.missing})
                            print(f"{utc(report.prev_recv_ms)}  STALL  frames held {report.hold_ms} ms "
                                  f"(no data {report.arrival_gap_ms} ms, camera frames {report.camera_gap_ms} ms apart), "
                                  f"dataNumber {report.prev_data_number} -> {data_number} ({report.missing} missing), "
                                  f"ended {utc(recv)}. Probe results in {probe_settle_ms / 1000:.1f} s.")
                        elif report.hold == "minor":
                            events.write("minor_hold", recv_utc=utc(recv), hold_ms=report.hold_ms, data_number=data_number)
                            print(f"{utc(recv)}  minor hold {report.hold_ms} ms (dataNumber {data_number})")
                        if report.missing and report.hold != "stall":
                            events.write("missing_frames", recv_utc=utc(recv), missing=report.missing,
                                         prev_data_number=report.prev_data_number, data_number=data_number)
                    burst = monitor.close_if_idle(now_ms())
                    if burst:
                        report_burst(burst)
                    settle_stalls()
            except (OSError, ConnectionError, ValueError) as e:
                events.write("disconnected", error=str(e))
                print(f"{utc(now_ms())}  disconnected: {e}")
                burst = monitor.flush()
                if burst:
                    report_burst(burst)
            finally:
                client.close()
    except KeyboardInterrupt:
        pass
    finally:
        burst = monitor.flush()
        if burst:
            report_burst(burst)
        settle_stalls(force=True)
        for probe in probes:
            probe.stop()
        events.write("summary", **monitor.stats)
        print(f"Summary: {json.dumps(monitor.stats)}")
        events.close()
        frames_file.close()
        chunks_file.close()
    return 0


# --------------------------------------------------------------------------------------------------------------
# analyze: stalls in existing logs
# --------------------------------------------------------------------------------------------------------------

_LOG_TIME_RE = re.compile(r"^\[(\d{4}-\d\d-\d\d \d\d:\d\d:\d\d\.\d{3})\]")
_RECEIVED_RE = re.compile(r"WebsockAsyncClnSession\.cpp \(\d+\)\s*-\s*\w+\s*:\s*Received:\s*(\{.*)$")
_FIELD_RE = {name: re.compile(r'"%s":\s*"?([^",}]+)"?' % name) for name in ("dataNumber", "messageType", "time")}
_STALL_RE = re.compile(
    r"FLIR link stall on (?P<sensor>.+?): no data for (?P<gap>-?\d+) ms while the camera kept producing frames "
    r"(?P<cgap>-?\d+) ms apart, so frames were held (?P<hold>-?\d+) ms in transit\. Stall from (?P<start>\S+) to "
    r"(?P<end>\S+), dataNumber (?P<dn0>\S+) -> (?P<dn1>\S+) \((?P<missing>\d+) missing\)")
_BURST_RE = re.compile(
    r"FLIR late burst on (?P<sensor>.+?): (?P<frames>\d+) frame\(s\) arrived at least (?P<thr>\d+) ms later than "
    r"normal \(worst (?P<worst>-?\d+) ms\)\. Received from (?P<r0>\S+) to (?P<r1>\S+) \((?P<spread>-?\d+) ms\), "
    r"produced by the camera from (?P<c0>\S+) to (?P<c1>\S+), dataNumber (?P<dn0>\S+) -> (?P<dn1>\S+)\.")
_MINOR_RE = re.compile(r"FLIR link minor hold on (?P<sensor>.+?): frame held (?P<hold>-?\d+) ms in transit")
_SDSS_SKIP_RE = re.compile(r"Skipping incoming detection at (\d+)ms is not current or has invalid timestamp of (\d+)ms")
_KAFKA_CREATE_RE = re.compile(rb"^CreateTime:(\d+)\|")


def _int_or_none(text: Optional[str]) -> Optional[int]:
    try:
        return int(text) if text is not None else None
    except ValueError:
        return None


def analyze_logs(args: argparse.Namespace) -> int:
    offset_ms = int(args.log_utc_offset * 3600 * 1000)
    received: Dict[Tuple[int, Optional[int], int], None] = {}
    warn_stalls: List[Dict] = []
    warn_bursts: List[Burst] = []
    warn_minor = 0
    for path in args.v2xhub:
        with open(path, errors="replace") as f:
            for line in f:
                if "FLIR link stall on" in line:
                    m = _STALL_RE.search(line)
                    if m:
                        warn_stalls.append({
                            "sensor": m["sensor"], "start_ms": parse_iso_utc(m["start"]), "end_ms": parse_iso_utc(m["end"]),
                            "hold_ms": int(m["hold"]), "arrival_gap_ms": int(m["gap"]), "camera_gap_ms": int(m["cgap"]),
                            "prev_data_number": _int_or_none(m["dn0"]), "data_number": _int_or_none(m["dn1"]),
                            "missing": int(m["missing"])})
                    continue
                if "FLIR late burst on" in line:
                    m = _BURST_RE.search(line)
                    if m:
                        warn_bursts.append(Burst(parse_iso_utc(m["r0"]), parse_iso_utc(m["r1"]), parse_iso_utc(m["c0"]),
                                                 parse_iso_utc(m["c1"]), int(m["frames"]), int(m["worst"]),
                                                 _int_or_none(m["dn0"]), _int_or_none(m["dn1"])))
                    continue
                if "FLIR link minor hold on" in line and _MINOR_RE.search(line):
                    warn_minor += 1
                    continue
                if "Received:" not in line or "WebsockAsyncClnSession" not in line:
                    continue
                t = _LOG_TIME_RE.match(line)
                m = _RECEIVED_RE.search(line)
                if not t or not m:
                    continue
                body = m.group(1)
                fields = {name: (rx.search(body) or [None, None])[1] for name, rx in _FIELD_RE.items()}
                if fields["messageType"] != "Data" or not fields["time"]:
                    continue
                try:
                    camera_ms = parse_camera_time(fields["time"])
                except ValueError:
                    continue
                recv_ms = calendar.timegm(datetime.strptime(t.group(1), "%Y-%m-%d %H:%M:%S.%f").timetuple()) * 1000 \
                    + int(t.group(1)[-3:]) - offset_ms
                received[(recv_ms, _int_or_none(fields["dataNumber"]), camera_ms)] = None

    stalls: List[Dict] = []
    bursts: List[Burst] = []
    stats: Dict = {}
    if received:
        source = f"{len(received)} DEBUG 'Received:' frames (assumes a single camera)"
        monitor = StreamMonitor(stall_ms=args.stall_ms)
        for recv_ms, data_number, camera_ms in sorted(received, key=lambda k: (k[0], k[1] or 0)):
            burst = monitor.close_if_idle(recv_ms)
            if burst:
                bursts.append(burst)
            report = monitor.on_frame(recv_ms, camera_ms, data_number)
            if report.completed_burst:
                bursts.append(report.completed_burst)
            if report.hold == "stall":
                stalls.append({"sensor": "-", "start_ms": report.prev_recv_ms, "end_ms": recv_ms,
                               "hold_ms": report.hold_ms, "arrival_gap_ms": report.arrival_gap_ms,
                               "camera_gap_ms": report.camera_gap_ms, "prev_data_number": report.prev_data_number,
                               "data_number": data_number, "missing": report.missing})
        burst = monitor.flush()
        if burst:
            bursts.append(burst)
        stats = monitor.stats
    elif warn_stalls or warn_bursts:
        source = "plugin 'FLIR link stall' / 'FLIR late burst' warnings"
        stalls, bursts = warn_stalls, warn_bursts
        stats = {"stalls": len(stalls), "minor_holds": warn_minor, "late_frames": sum(b.frames for b in bursts),
                 "max_stall_ms": max((s["hold_ms"] for s in stalls), default=0)}
    else:
        print("No camera frames or FLIR link warnings found. For older plugin builds the plugin LogLevel must be "
              "DEBUG so 'Received:' lines are logged.", file=sys.stderr)
        return 1

    sdss_skips: List[int] = []
    for path in args.sdss or []:
        with open(path, errors="replace") as f:
            for line in f:
                m = _SDSS_SKIP_RE.search(line)
                if m:
                    sdss_skips.append(int(m.group(2)))
    sdss_skips.sort()

    other_traffic: List[int] = []
    for path in args.other_traffic or []:
        with open(path, "rb") as f:
            for line in f:
                m = _KAFKA_CREATE_RE.match(line)
                if m:
                    other_traffic.append(int(m.group(1)))
    other_traffic.sort()

    probe_stalls: List[Dict] = []
    for path in args.probe_events or []:
        with open(path) as f:
            for line in f:
                try:
                    record = json.loads(line)
                except ValueError:
                    continue
                if record.get("type") == "stall":
                    probe_stalls.append(record)

    def count_between(values: List[int], lo: int, hi: int) -> int:
        return bisect.bisect_right(values, hi) - bisect.bisect_left(values, lo)

    rows = []
    matched_probe_ids = set()
    for stall in stalls:
        burst = next((b for b in bursts if abs(b.first_recv_ms - stall["end_ms"]) <= 1), None)
        row = {
            "start_utc": utc(stall["start_ms"]), "end_utc": utc(stall["end_ms"]), "sensor": stall["sensor"],
            "hold_ms": stall["hold_ms"], "no_data_ms": stall["arrival_gap_ms"], "camera_gap_ms": stall["camera_gap_ms"],
            "data_number": f"{stall['prev_data_number']}->{stall['data_number']}", "missing": stall["missing"],
            "late_frames": burst.frames if burst else "",
            "worst_late_ms": burst.max_excess_ms if burst else "",
            "burst_spread_ms": (burst.last_recv_ms - burst.first_recv_ms) if burst else "",
        }
        if args.sdss:
            row["sdss_discarded"] = count_between(sdss_skips, burst.first_camera_ms, burst.last_camera_ms) if burst else ""
        if args.other_traffic:
            row["other_traffic_msgs"] = count_between(other_traffic, stall["start_ms"] + 1, stall["end_ms"] - 1)
        if args.probe_events:
            match = next((i for i, p in enumerate(probe_stalls)
                          if p["start_ms"] <= stall["end_ms"] + 1000 and p["end_ms"] >= stall["start_ms"] - 1000), None)
            if match is not None:
                matched_probe_ids.add(match)
                row["probe"] = "also stalled: " + probe_stalls[match].get("verdict", "")
            else:
                row["probe"] = "probe saw no stall: frames were held on this host (plugin / V2X Hub side)"
        rows.append(row)

    print(f"Source: {source}")
    print(f"Stall threshold {args.stall_ms} ms. Stats: {json.dumps(stats)}")
    if not rows:
        print("No stalls found.")
    for row in rows:
        line = (f"{row['start_utc']} -> {row['end_utc']}  held {row['hold_ms']} ms (no data {row['no_data_ms']} ms, "
                f"camera frames {row['camera_gap_ms']} ms apart)  dataNumber {row['data_number']} ({row['missing']} missing)")
        if row["late_frames"] != "":
            line += f"  late frames {row['late_frames']} (worst {row['worst_late_ms']} ms, delivered over {row['burst_spread_ms']} ms)"
        if "sdss_discarded" in row:
            line += f"  SDSS discarded {row['sdss_discarded']}"
        if "other_traffic_msgs" in row:
            line += f"  other traffic during stall {row['other_traffic_msgs']} msgs"
        print(line)
        if "probe" in row:
            print(f"    probe: {row['probe']}")
    unmatched = [p for i, p in enumerate(probe_stalls) if i not in matched_probe_ids]
    for p in unmatched:
        print(f"Probe-only stall {p.get('start_utc')} -> {p.get('end_utc')} ({p.get('hold_ms')} ms): not seen in the plugin logs.")
    if any(row.get("other_traffic_msgs") for row in rows):
        print("Note: other traffic arriving during a stall means this host's network and V2X Hub kept working, "
              "so that stall was specific to the camera stream. 0 means there was no other traffic to compare with.")
    if args.csv and rows:
        with open(args.csv, "w", newline="") as f:
            writer = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
            writer.writeheader()
            writer.writerows(rows)
        print(f"Wrote {len(rows)} stall(s) to {args.csv}")
    return 0


# --------------------------------------------------------------------------------------------------------------

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    common = argparse.ArgumentParser(add_help=False)
    common.add_argument("--stall-ms", type=int, default=500,
                        help="Extra transit time that counts as a stall, and lateness that counts as late (default 500, "
                             "the plugin's LinkStallThresholdMs default).")
    sub = parser.add_subparsers(dest="command", required=True)

    live = sub.add_parser("live", parents=[common], help="Subscribe to the camera independently and probe the network.")
    live.add_argument("--host", required=True, help="Camera IP address.")
    live.add_argument("--port", type=int, default=80, help="Camera WebSocket port (default 80).")
    live.add_argument("--path", default="/api/subscriptions", help="Camera WebSocket endpoint.")
    live.add_argument("--control", action="append", default=[], metavar="HOST:PORT",
                      help="Another host on the same network to probe for comparison, e.g. pc1 or the switch "
                           "(repeatable). Must have an open TCP port.")
    live.add_argument("--probe-interval", type=float, default=0.5, help="Seconds between TCP connect probes (default 0.5).")
    live.add_argument("--probe-timeout", type=float, default=3.0, help="TCP connect probe timeout in seconds (default 3).")
    live.add_argument("--slow-ms", type=int, default=500,
                      help="A connect probe slower than this counts as failed (default 500; a lost SYN adds ~1000 ms).")
    live.add_argument("--no-probe", action="store_true", help="Only subscribe; do not run connect probes.")
    live.add_argument("--connect-timeout", type=float, default=5.0, help="WebSocket connect timeout in seconds.")
    live.add_argument("--duration", type=float, default=0, help="Stop after this many seconds (default: run until Ctrl-C).")
    live.add_argument("--out", help="Output directory (default flir_probe_<timestamp>).")

    analyze = sub.add_parser("analyze", parents=[common], help="Find stalls in existing logs.")
    analyze.add_argument("--v2xhub", nargs="+", required=True, help="V2X Hub log file(s) containing the FLIR plugin output.")
    analyze.add_argument("--log-utc-offset", type=float, default=0.0,
                         help="UTC offset in hours of the V2X Hub log timestamps (default 0: logs are in UTC).")
    analyze.add_argument("--sdss", nargs="+", help="Sensor data sharing service log(s), to count discarded stale detections.")
    analyze.add_argument("--other-traffic", nargs="+",
                         help="Kafka topic export(s) with CreateTime of traffic not from the camera (e.g. v2xhub_bsm_in.log), "
                              "to check whether this host kept receiving other data during each stall.")
    analyze.add_argument("--probe-events", nargs="+", help="events.jsonl file(s) written by 'live', to compare with the plugin.")
    analyze.add_argument("--csv", help="Write one row per stall to this CSV file.")

    args = parser.parse_args()
    if args.command == "live":
        return run_live(args)
    return analyze_logs(args)


if __name__ == "__main__":
    sys.exit(main())
