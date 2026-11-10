# FLIR Camera Driver Plugin Documentation

## Introduction
The FLIR Camera Driver Plugin is the hardware interface for the FLIR Sensor. It contains logic to translate FLIR Sensor Pedestrian Presence Tracking information into V2X Hub Sensor Detections. These detections can be consumed by downstream plugins or services for J2735 Message Generation. Currently this is support by the [CARMA Streets](https://github.com/usdot-fhwa-stol/carma-streets) [Sensor Data Sharing Service](https://github.com/usdot-fhwa-stol/carma-streets/tree/develop/sensor_data_sharing_service) which will generate SDSMs

## Related Plugins

A list of plugins related to the FLIR Camera Driver Plugin.

### Immediate Forward Plugin

For RSU Immediate Message Forwarding (IMF) functionality forward any generated J2735 message to RSU (e.g. SDSM)

### CARMA Streets Plugin

The CARMA Streets plugin is required for SDSM generation since this is handled by the [CARMA Streets](https://github.com/usdot-fhwa-stol/carma-streets) [Sensor Data Sharing Service](https://github.com/usdot-fhwa-stol/carma-streets/tree/develop/sensor_data_sharing_service). For this functionality the FLIR Camera Driver Plugin generates Sensor Detected Object messages, which are forwarded to the Sensor Data Sharing service via the CARMA Streets plugin. The Sensor Data Sharing Service then generates SDSMs which are sent back to V2X Hub via the CARMA Streets Plugin  and broadcast to the RSU via the Immediate Forward Plugin.


## Configuration/Deployment

This plugin has several configuration parameters. Below these are listed out as together with descriptions on how to set them.

**FLIRConfigurations**: JSON string configuration parameters which contains on or more FLIR Camera(s) to connect to.

```json
[
  {
    "WebSocketHost": "127.0.0.1", // IP Address of FLIR Camera
    "WebSocketPort": "8081", // Websocket port
    "Endpoint": "/api/subscriptions", // Websocket endpoint
    "CameraRotation": 16.13, // Camera heading measure in degrees clockwise from North
    "SensorId": "FLIR_Turner_Parking_Lot" // Sensor Id 
  },
  {
    "WebSocketHost": "127.0.0.1",
    "WebSocketPort": "8082",
    "Endpoint": "/api/subscriptions",
    "CameraRotation": 11.13,
    "SensorId": "FLIR_Crosswalk"
  }
]
```

**LinkStallThresholdMs** (default 500, minimum 200): camera link monitoring threshold. A camera frame that spent this many milliseconds longer in transit than the frame before it is logged as a link stall, including when the camera had been idle (nothing tracked) before it. Frames this much later than normal are counted as late. 500 ms matches the freshness limit the CARMA Streets Sensor Data Sharing Service applies before discarding detections.

**LinkTimeoutMs** (default 500, minimum 200, 0 disables): camera link watchdog. When nothing has arrived from a camera for half this time, the plugin pings it; when nothing at all, neither data nor the reply to the ping, arrives within this time, it drops the connection and connects again. See [Link Watchdog and Reconnect](#link-watchdog-and-reconnect).

## Status Information

This plugin includes some custom status information to allow for real-time monitoring of FLIR Camera detection data. Included are:

**Dropped Pedestrian Detections** : A complete count of all pedestrian detections from all FLIR Camera connections that were dropped due to inability to process the information
**Modified Pedestrian Detections (Bad Data: Assumed Stationary)**: A complete count of all pedestrian detections from all FLIR Camera connections that were modified due to bad incoming data.
> [!NOTE]  
> We only modify incoming detections that have incomplete/invalid heading/speed information. After testing we have found that occassionally, when a detected object stops, the FLIR Camera will continue to broadcast detections that **do not** include angle or heading but **do** include speed. This seems to be a bug that only occurs for stationary objects intermittently. To address this, when no angle is provided, we assume speed is 0. For tracebility we have added this status as well as a flag in each SensorDetectedObjectMessage which indicates whether we have modified the detection data from the source sensor at all.
**Unique Pedestrian Detections**: A complete count of all pedestrian detections from all FLIR Camera connections that are unique. We evaluate this by comparing the incoming detection ids to the previously received ids. This is meant to be a rough estimate of all pedestrians detected by the FLIR Cameras connected.
**Total Pedestrian Detections**: A complete count of all pedestrian detections from all FLIR Camera connections. This includes any detections that where dropped or modified.
**Live number of currently detected pedestrians**: A current/live count of unique pedestrian detections from all FLIR Camera connections.

The following camera link statuses are totals since the camera connections started (see [Camera Link Monitoring](#camera-link-monitoring)):

**Camera Link Stalls**: Number of times camera frames were held in transit for at least `LinkStallThresholdMs`, after which the backlog arrived together. This includes outages that began while the camera was idle: the first frame it produced during the outage arrives as late as the time it waited.
**Camera Link Longest Stall (ms)**: Longest time frames were held in transit.
**Camera Link Last Stall (UTC)**: Start and end of the most recent stall, for matching against packet captures and network logs.
**Camera Link Late Frames**: Frames that arrived at least `LinkStallThresholdMs` later than normal, i.e. every frame of each backlog. Detections in these frames are likely discarded downstream as stale.
**Camera Link Minor Holds**: Frames held 150 ms to `LinkStallThresholdMs` in transit. About 200 ms is the signature of a single TCP retransmission, so a rising count is an early sign of packet loss on the camera link.
**Camera Missing Frames**: Gaps in the camera's `dataNumber` sequence while it was tracking continuously.
**Camera Link Reconnects**: Number of times a camera connection that had been delivering data was lost and re-established.
**Camera Links Connected**: Cameras currently connected and delivering data, e.g. `2 of 2`.

![alt text](docs/FLIRCameraDriverStatus.png)


## Design
![Alt text](docs/FLIRCameraDriverArchitecture.drawio.png)

Above is a diagram illustrates how the FLIR Camera Driver Plugin is used to create Sensor Data Sharing Messages (SDSM) for Cooperative Perception.

### Messages

**SensorDetectedObject**: V2X Hub message that contains detection data. Can be used to generate other messages like SDSM.

**SensorDataSharingMessage**: V2X Cooperative Perception message defined in the J3224 standard.

## Camera Link Monitoring

The camera streams tracking data over a WebSocket (TCP). If packets are lost on the camera link, TCP holds back everything after the lost packet until a retransmission gets through, so detections stop for a few seconds and then arrive all at once. Downstream services discard detections that late, which shows up as a gap followed by a burst on the detection topic.

The plugin compares when each frame arrives with the camera's own `time` field. It compares differences between consecutive frames rather than absolute times, so it does not depend on the camera and V2X Hub clocks being synchronized. Late frames are measured against the normal latency of recent on-time frames; if the latency stays higher at a steady level for 10 s, the clock offset changed and the new level becomes the normal. It logs:

* `WARNING ... FLIR link stall on <sensor>: no data for 3583 ms while the camera kept producing frames 100 ms apart, so frames were held 3483 ms in transit. Stall from <start> to <end> ...`: the camera kept producing frames but none arrived. Start and end are UTC; use them to look up the same moment in a packet capture, ping log or switch log. If the camera was idle before the stall, the warning ends with `The camera was idle (nothing tracked) before this frame ...`; the camera gap is then the idle time, and the link went down at some point during it.
* `WARNING ... FLIR late burst on <sensor>: 26 frame(s) arrived at least 500 ms later than normal ...`: the backlog delivered after a stall.
* `INFO ... FLIR link minor hold on <sensor>: frame held 200 ms in transit ...`: usually one TCP retransmission.
* `DEBUG ... FLIR camera frames missing on <sensor>: dataNumber ...`: gaps in the camera's frame sequence.
* `INFO ... FLIR baseline latency on <sensor> moved to ...`: the camera/host clock offset changed; frames counted as late since the change are no longer counted.

### Link Watchdog and Reconnect

Each camera connection recovers by itself:

* **Stalled link.** When nothing has arrived for half of `LinkTimeoutMs`, the plugin sends the camera a WebSocket ping. An idle camera (nothing tracked) answers right away. On a stalled link the reply cannot arrive, because TCP delivers it only after the frames being held, so after `LinkTimeoutMs` with nothing received the plugin drops the connection. The held frames are discarded with it; they would be stale anyway.
* **Reconnect.** A new attempt starts 100 ms after each failure. Each TCP connect is abandoned after 10 times the previous connect time (at least 250 ms, at most `LinkTimeoutMs`), so while the network is down a new connection request goes out every ~350 ms instead of waiting for the kernel's own retries after 1 s, 2 s, 4 s, .... The same applies when the camera closes or refuses the connection, or is not up when the plugin starts; before, a failed connection stayed down until the plugin was restarted.
* **Ping support.** Right after connecting, the plugin pings the camera once, and the watchdog only drops connections once the camera has answered a ping (`INFO ... answers WebSocket pings, link watchdog active.`). A camera that never answers logs `WARNING ... has not answered a WebSocket ping ...` and keeps its connection through silences, as before.

What this changes: a stall is the time the network was down plus a wait for TCP. While the link is down, TCP resends the oldest frame after about 0.2 s, 0.6 s, 1.4 s, 3.0 s, 6.2 s, ...; when the link comes back, the old connection waits for the next of these resends. That is why stalls cluster at about 1.8 s, 3.3 s and 6.6 s: any outage of about 1.5 s to 3 s ends as a stall of about 3.3 s (3.5 s with the RTT of a real network). The reconnect removes the wait, not the outage. Measured with all packets between plugin and camera dropped for the given time, camera sending every 100 ms (`LinkTimeoutMs` 1000; while the whole path is down the timeout makes no difference):

| Network down | No frames, without watchdog | No frames, with watchdog |
|---|---|---|
| 0.5 s | 0.7 s | 0.7 s (TCP recovers before the watchdog fires) |
| 1.0 s | 1.6 s | 1.2 s |
| 1.5 s | 3.2 s | 1.8 s |
| 2.0 s | 3.2 s | 2.2 s |
| 2.5 s | 3.2 s | 2.9 s |
| 3.0 s | 3.2 s | 3.2 s |
| 5.0 s | 6.5 s | 5.3 s |
| 20 s | 26.7 s | 20.4 s |

With the watchdog, frames resumed 0.05 s to 0.3 s after the network came back, against up to 1.6 s (6.6 s after the 20 s outage) without it. So a ~3.5 s stall becomes about as long as the network was really down (1.5 s to 3 s) plus up to ~0.3 s: shorter on average, but not gone, and close to unchanged when the outage itself was ~3 s. Shortening the outage itself needs the network fixed: see [Pinpointing where frames are held](#pinpointing-where-frames-are-held).

The table above is for the whole path being down. Whether a reconnect helps depends on where the frames are held, because the new connection takes the same path. With 3 s disturbances:

| Where frames are held | No frames, without watchdog | `LinkTimeoutMs` 1000 | 500 | 200 |
|---|---|---|---|---|
| Whole path (link, switch, host network): new connections cannot get through either | 3.2 s | 3.1 s | 3.3 s | 3.2 s |
| Only the existing connection: a new connection gets through | 3.2 s | 1.0 s | 0.5 s | 0.2 s |

The `restored` log tells the two apart: after a stall of only the existing connection the link is restored with `0 failed reconnect attempt(s)` right after it was lost; when the whole path was down, the attempts fail until the path is back. A lower `LinkTimeoutMs` only helps in the first case. In both cases it also drops connections that a single TCP retransmission (0.2 s to 0.4 s) would have recovered, which costs about as long as waiting.

It logs:

* `WARNING ... FLIR link to <sensor> lost at <time> (read: The socket was closed due to a timeout), 1001 ms after the last message. Reconnecting.`
* `WARNING ... Unable to connect to FLIR camera <sensor> at <host>:<port> (connect: ...), 10 failed attempt(s). Retrying.`: logged at the 1st, 10th, 100th, ... failed attempt.
* `WARNING ... FLIR link to <sensor> restored at <time>: no messages for 2203 ms, ...`: how long the camera was cut off, as seen by the plugin.

An outage that ends with a reconnect is not counted under **Camera Link Stalls**: the new connection delivers current frames, not a backlog. It is counted under **Camera Link Reconnects** and logged as above. Stalls shorter than `LinkTimeoutMs` are reported as before.

### Pinpointing where frames are held

`scripts/flir_stream_probe.py` uses only the Python standard library.

**live** subscribes to the camera as a second, independent client and at the same time keeps opening TCP connections to the camera and to optional control hosts. Run it on the V2X Hub host while testing:

```shell
python3 scripts/flir_stream_probe.py live --host 192.168.55.183 \
    --control 192.168.55.1:80 --control 192.168.55.125:22
```

The examples in this section use these addresses; replace them with yours:

* `192.168.55.183`: the FLIR camera (WebSocket on port 80).
* `192.168.55.1:80`: the gateway, a control host. Any other machine on the network with an open TCP port works; add more with extra `--control HOST:PORT` options, e.g. another host on the camera's switch.
* `192.168.55.125:22`: this PC, the host running the probe, as a self check (needs an open port, e.g. SSH).

A `--control` that is one of this host's own addresses (`192.168.55.125` above) is probed as a *self* check, which shows whether this host lost its own network address (link down, Wi-Fi disconnected, or a user-private NetworkManager connection taken down when the user logs out). Use other machines, such as the gateway, as controls.

For each stall it prints whether the camera stayed reachable and how the backlog arrived at the TCP level, and writes `events.jsonl`, `frames.csv`, `chunks.csv` and `probes.csv` (every connect probe with its error, e.g. `ENETUNREACH`). It also reports any period with no data at all, not even the camera's 60 s keepalive ping (`--silence-ms`, default 90000), so outages are caught even while nothing is tracked:

* Self check failed too: this host lost its network address or interface; the camera and the network were not the cause.
* Camera unreachable while the control host answered: packets were lost between the switch and the camera (camera network interface, its cable, PoE/BPL link or switch port).
* Camera and control hosts all unreachable while this host kept its address: the problem is between this host and the rest of the network (its link to the switch or Wi-Fi access point, or that equipment itself).
* Camera reachable throughout: the network path was up; frames were held by the camera's sender or by TCP retransmission backoff.

If the plugin logs a stall at a time when the probe did not see one, the frames were held on the V2X Hub host. Running the probe on a laptop on the same switch separates the camera's link from the V2X Hub host's link.

#### Running the probe in the background

Stalls can be hours apart, so the probe is usually left running unattended. All commands below are run from the plugin directory, `src/v2i-hub/FLIRCameraDriverPlugin` in the V2X Hub repository; change the `cd` path to where you cloned it. Each run writes to its own `scripts/flir_probe_<date>_<time>/` directory.

Start it detached from the terminal so it keeps running after you log out or your SSH session ends, with unbuffered output (`-u`) so `console.log` fills as it runs:

```shell
cd ~/v2x-hub/src/v2i-hub/FLIRCameraDriverPlugin
OUT=scripts/flir_probe_$(date +%Y%m%d_%H%M%S); mkdir -p "$OUT"
setsid nohup python3 -u scripts/flir_stream_probe.py live --host 192.168.55.183 \
    --control 192.168.55.1:80 --control 192.168.55.125:22 \
    --out "$OUT" > "$OUT/console.log" 2>&1 < /dev/null &
echo "Run directory: $OUT"
```

Check on it later, e.g. after logging back in. `$OUT` from the start commands is gone in a new shell, so look up the newest run directory first:

```shell
cd ~/v2x-hub/src/v2i-hub/FLIRCameraDriverPlugin
RUN=$(ls -td scripts/flir_probe_* | head -1); echo "$RUN"        # newest run directory
pgrep -af '[p]ython3 .*flir_stream_probe'                         # PID and command line; no output if it stopped
ps -o pid,etime,cmd -p "$(pgrep -d, -f '[p]ython3 .*flir_stream_probe')"   # how long it has been running
date +%T; ls -l --time-style=+%T "$RUN/probes.csv"              # updated every 0.5 s while it runs
grep -E 'SILENCE details|STALL details|LATE BURST' "$RUN/console.log"   # outages found so far
tail -f "$RUN/console.log"                                      # live output (Ctrl-C stops tail, not the probe)
```

Runs started without the command above (e.g. in a terminal or with plain `nohup`) have no `console.log` in their directory; their output went to the terminal or to `nohup.out`.

Stop it with SIGINT, the same as Ctrl-C, so it writes its summary to `events.jsonl` and `console.log` before exiting. Check that it has exited, then run the start commands above again, which create a new run directory:

```shell
pkill -INT -f '[p]ython3 .*flir_stream_probe'
sleep 5; pgrep -af '[p]ython3 .*flir_stream_probe' || echo "stopped"
```

**analyze** finds stalls in existing logs and cross-checks them:

```shell
python3 scripts/flir_stream_probe.py analyze --v2xhub <v2xhub log> --sdss <sensor data sharing service log> \
    --other-traffic <kafka export of another topic, e.g. v2xhub_bsm_in.log> --probe-events <live run>/events.jsonl
```

It reads the plugin's stall warnings, or the DEBUG `Received:` lines of older builds (plugin `LogLevel` DEBUG, single camera). Per stall it reports how many detections the Sensor Data Sharing Service discarded as stale, whether other traffic kept arriving (which rules out the V2X Hub host stalling as a whole), and whether the probe saw the same stall.


## Functionality Testing
In the `./scripts/` directory in this plugin we have created a simply python script that mocks the websocket hosted by the FLIR Camera for pedestrian presence tracking. To confirm plugin functionality this script can be used to mock the output of the FLIR Camera

1) Install Python3 and Pip.
2) Run `python3 -m pip install -r scripts/requirements.txt` to install depedencies.
3) Run `python3 scripts/mockFLIRCamera.py --ip <ip> --port <port>`
4) Configure FLIR Camera Driver Plugin for connection to the websocket and in the **Messages** tab of the plugin you should see Sensor Detected Objects publishing.

To test the link watchdog, drop the traffic between the plugin and the mock for a few seconds, e.g. with nftables on the V2X Hub host (port 9000 is the mock's default). The plugin should log `FLIR link to <sensor> lost`, connection attempts while the port is blocked, and `restored` about 0.1 s to 0.4 s after the rule is removed:

```shell
sudo nft add table inet flirtest
sudo nft add chain inet flirtest c '{ type filter hook input priority 0 ; }'
sudo nft add rule inet flirtest c tcp sport 9000 drop
sudo nft add rule inet flirtest c tcp dport 9000 drop
sleep 3; sudo nft delete table inet flirtest
```

To test camera link monitoring, add `--stall-every <seconds> --stall-ms <milliseconds>` to the mock. It then periodically holds the messages it produces for the given time and sends them all at once. The plugin should log a `FLIR link stall` and a `FLIR late burst` for each one, and the **Camera Link** statuses should count them. 
