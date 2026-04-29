#!/usr/bin/env python3

import json
import time
import socket
import signal
import binascii
import threading
import collections
import queue
from dataclasses import dataclass
from typing import Any, Dict, List, Optional, Tuple
from collections import defaultdict, deque

from CAVmessages import J2735_decode
from appJar import gui
from datetime import date


report_file = open("report.csv", "w")

createdPreview = False
createdTestSetup = False
createdTestResult = False
ValidateSample = False
testlist = ""
testTemplate = {}
parsed = {}
message = ""
initialMessage = ""

running = False
listener_thread = None
listener_socket = None

# per-message-type processing
message_queues = {}
message_workers = {}
queue_lock = threading.Lock()
stop_worker = object()

'''
list of message IDs to look for in payload(s);
BSM=0014,
MAP=0012,
SPAT=0013,
TIM=001f,
PSM=0020,
SDSM=0029
'''
message_ids = {
    "0014": "BSM",
    "0012": "MAP",
    "0013": "SPAT",
    "001f": "TIM",
    "0020": "PSM",
    "0029": "SDSM",
}

DSRC_msg_ids = {
    18: "MAP",
    19: "SPAT",
    20: "BSM",
    31: "TIM",
    32: "PSM",
    41: "SDSM"
}

def LoadTests(msg_id):
    global testTemplate
    testfile = "./TestTemplate_2024/test"
    msg_type = DSRC_msg_ids.get(msg_id)
    if not msg_type:
        print(f"No Message ID found for {msg_id}")
        return 0
    testfile += f"{msg_type}.json"
    with open(testfile) as file:
        testTemplate = json.load(file)
    setupTestTemplate()
    return 1    

def ValidateMessage(msg_id,decoded_json):
    global testTemplate, parsed
    #stop_listener()
    msg_type = DSRC_msg_ids.get(msg_id)
    if not msg_type:
        print(f"No Message ID found for {msg_id}")
        return 0
    parsed = decoded_json
    certify()
    print("certify complete")
    stop_listener()
    return 0

@dataclass
class DecodedPacket:
    recv_time: float
    source_ip: str
    source_port: int
    psid: Optional[str]
    payload_hex: str
    decoded_json: Dict[str, Any]
    message_id: Optional[int]
    message_name: Optional[str]


class MessageStore:
    def __init__(self) -> None:
        self._lock = threading.Lock()
        self._by_message_id = defaultdict(deque)
        self._counts = collections.Counter()

    def add(self, pkt: DecodedPacket) -> None:
        with self._lock:
            self._by_message_id[pkt.message_id].append(pkt)
            self._counts[pkt.message_id] += 1

    def snapshot_counts(self) -> Dict[Optional[int], int]:
        with self._lock:
            return dict(sorted(self._counts.items(), key=lambda kv: (999999 if kv[0] is None else kv[0])))

    def snapshot_sorted(self) -> Dict[Optional[int], List[DecodedPacket]]:
        with self._lock:
            return {
                k: list(v)
                for k, v in sorted(self._by_message_id.items(), key=lambda kv: (999999 if kv[0] is None else kv[0]))
            }


MESSAGE_STORE = MessageStore()


def find(key, dictionary):
    for k, v in dictionary.items():
        if k == key:
            yield v
        elif isinstance(v, dict):
            for result in find(key, v):
                yield result
        elif isinstance(v, list):
            for d in v:
                if isinstance(d, dict):
                    for result in find(key, d):
                        yield result

# in the event of spaces or payload= values, strips those
def normalize_hex(s: str) -> str:
    return "".join(ch for ch in s if ch in "0123456789abcdefABCDEF").lower()

# used to normalize values to whole numbers for grouping by msg ID
def value_as_int(value: Any) -> Optional[int]:
    try:
        if isinstance(value, bool):
            return int(value)
        if isinstance(value, int):
            return value
        if isinstance(value, float):
            return int(value)
        if isinstance(value, str):
            value = value.strip()
            if value.lower().startswith("0x"):
                return int(value, 16)
            return int(value)
    except Exception:
        pass
    return None

# to return none if decoding fails (uses j2735decoder repo which crashes if a bad payload is attempted to be decoded)
def safe_json_loads(s: str) -> Optional[Dict[str, Any]]:
    try:
        return json.loads(s)
    except Exception:
        return None


def get_first_existing_key(d: Dict[str, Any], keys: List[str]) -> Any:
    for key in keys:
        if key in d:
            return d[key]
    return None


def extract_psid_from_text(text: str) -> Optional[str]:
    lowered = text.lower()
    idx = lowered.find("psid")
    if idx == -1:
        return None

    tail = lowered[idx:idx + 80]
    separators = ["=", ":"]
    val = None

    for sep in separators:
        if sep in tail:
            rhs = tail.split(sep, 1)[1].strip()
            token = rhs.split()[0].strip(",;")
            val = token
            break

    if not val:
        return None

    if val.startswith("0x"):
        val = val[2:]

    val = normalize_hex(val)
    return val if val else None


def find_earliest_message_id(hex_string: str) -> Tuple[Optional[str], Optional[int]]:
    best_msg_id = None
    best_idx = None

    for msg_id in message_ids.keys():
        idx = hex_string.find(msg_id)
        if idx != -1:
            if best_idx is None or idx < best_idx:
                best_idx = idx
                best_msg_id = msg_id

    return best_msg_id, best_idx


def slice_to_j2735_payload(hex_string: str) -> Optional[str]:
    msg_id, idx = find_earliest_message_id(hex_string)
    if msg_id is None or idx is None:
        return None

    payload = hex_string[idx:]

    if len(payload) % 2 != 0:
        payload = payload[:-1]

    return payload if payload else None


def extract_payload(datagram_bytes: bytes) -> List[Tuple[str, Optional[str]]]:
    candidates = []

    try:
        text = datagram_bytes.decode("utf-8", "ignore")
    except Exception:
        text = ""

    if text:
        psid = extract_psid_from_text(text)

        if "Payload=" in text or "payload=" in text:
            for raw_line in text.splitlines():
                if "Payload=" in raw_line or "payload=" in raw_line:
                    marker = "Payload=" if "Payload=" in raw_line else "payload="
                    payload_part = raw_line.split(marker, 1)[1].strip()
                    payload_token = payload_part.split()[0].strip(",;")
                    payload_hex = normalize_hex(payload_token)

                    if payload_hex:
                        sliced = slice_to_j2735_payload(payload_hex)
                        if sliced:
                            candidates.append((sliced, psid))

        whole_text_hex = normalize_hex(text)
        if whole_text_hex and len(whole_text_hex) >= 8:
            sliced = slice_to_j2735_payload(whole_text_hex)
            if sliced:
                candidates.append((sliced, psid))

    raw_hex = binascii.hexlify(datagram_bytes).decode("ascii").lower()
    if raw_hex and len(raw_hex) >= 8:
        sliced = slice_to_j2735_payload(raw_hex)
        if sliced:
            candidates.append((sliced, None))

    deduped = []
    seen = set()
    for payload_hex, psid in candidates:
        key = (payload_hex, psid)
        if key not in seen:
            seen.add(key)
            deduped.append((payload_hex, psid))

    return deduped


def j2735decoder(payload_hex: str) -> Dict[str, Any]:
    result = J2735_decode(payload_hex)

    if hasattr(result, "json"):
        if isinstance(result.json, dict):
            return result.json
        if isinstance(result.json, str):
            parsed_result = safe_json_loads(result.json)
            if parsed_result is not None:
                return parsed_result

    if isinstance(result, dict):
        return result

    if isinstance(result, str):
        parsed_result = safe_json_loads(result)
        if parsed_result is not None:
            return parsed_result

    raise ValueError("Unable to normalize J2735_decode output into JSON")


def extract_message_id(decoded_json: Dict[str, Any]) -> Optional[int]:
    possible_keys = ["messageId", "messageID", "messageid", "id"]

    for key in possible_keys:
        values = list(find(key, decoded_json))
        for value in values:
            num = value_as_int(value)
            if num is not None:
                return num

    return None


def extract_message_name(decoded_json: Dict[str, Any]) -> Optional[str]:
    possible_keys = ["messageName", "messageType", "msgType", "type", "name"]

    for key in possible_keys:
        values = list(find(key, decoded_json))
        for value in values:
            if isinstance(value, str):
                return value

    return None


def message_queue_thread(message_id: Optional[int]):
    """
    Create the queue and worker threads for a message type if they do not exist yet.
    Returns the queue for that message_id.
    """
    global message_queues, message_workers

    with queue_lock:
        if message_id in message_queues:
            return message_queues[message_id]

        msg_queue = queue.Queue()
        message_queues[message_id] = msg_queue

        def worker_loop():
            print("[worker {}] started".format(message_id))
            while True:
                pkt = msg_queue.get()
                try:
                    if pkt is stop_worker:
                        print("[worker {}] stop requested".format(message_id))
                        return

                    print("\n[worker {}] processing packet".format(message_id))
                    print("[worker {}] source={}:{}".format(message_id, pkt.source_ip, pkt.source_port))
                    print("[worker {}] messageName={}".format(message_id, pkt.message_name))
                    print("[worker {}] payloadLen={} bytes".format(message_id, len(pkt.payload_hex) // 2))
                    print("[worker {}] queue size now={}".format(message_id, msg_queue.qsize()))

                    # Add per-message-type processing here later if needed.
                    # Example:
                    # if message_id == 20:
                    #     do_bsm_work(pkt)
                    # elif message_id == 18:
                    #     do_map_work(pkt)

                except Exception as exc:
                    print("[worker {}] processing error: {}".format(message_id, exc))
                finally:
                    msg_queue.task_done()

        worker = threading.Thread(
            target=worker_loop,
            daemon=True,
            name="message-worker-{}".format(message_id),
        )
        message_workers[message_id] = worker
        worker.start()

        print("Created queue and worker for messageId {}".format(message_id))
        return msg_queue


def stop_message_processors():
    global message_queues, message_workers

    with queue_lock:
        queues = list(message_queues.items())

    for _, msg_queue in queues:
        try:
            msg_queue.put(stop_worker)
        except Exception:
            pass


def print_message_queue_status():
    with queue_lock:
        if not message_queues:
            print("No message queues created yet.")
            return

        print("\n=== MESSAGE QUEUE STATUS ===")
        for message_id in sorted(message_queues.keys(), key=lambda x: (999999 if x is None else x)):
            msg_queue = message_queues[message_id]
            worker = message_workers.get(message_id)
            worker_alive = worker.is_alive() if worker is not None else False
            print(
                "messageId {} | queue size {} | worker alive {}".format(
                    message_id,
                    msg_queue.qsize(),
                    worker_alive,
                )
            )


def certify():
    global parsed, createdTestResult, testlist

    initialMessageText = app.getTextArea("message content")

    if createdTestResult is False:
        app.startPanedFrame("r1")
        app.startLabelFrame("Results: Summary")
        createdTestResult_local = True
    else:
        app.openPanedFrame("r1")
        app.emptyCurrentContainer()
        app.startLabelFrame("Results: Summary")
        createdTestResult_local = True

    testlist = testTemplate["testlist"]

    for key, value in testlist.items():
        testvector = app.getCheckBox(key)

        if testvector is True:
            app.addLabel(key)
            testtype = ""
            
            testtype = value.get("testtype")
            testobj = value.get("testobj")
            if not testtype or not testobj:
                continue

            if testtype == "exist":
                ret = list(find(testobj, parsed))
                if len(ret) > 0:
                    app.setLabel(key, key + " :: " + testobj + " :: PASSED")
                    app.setLabelFg(key, "green")
                else:
                    app.setLabel(key, key + " :: " + testobj + " :: FAILED")
                    app.setLabelFg(key, "red")

            elif testtype == "verify":
                msg1 = list(find(testobj, parsed))
                try:
                    msg1set = set(msg1)
                except:
                    flatmsg1 = []
                    for x in msg1:
                        deepsearch = list(find(testobj, x))
                        if(deepsearch):
                            flatmsg1 += deepsearch
                    if not flatmsg1:
                        msg1set = msg1
                    else:
                        msg1set = set(flatmsg1)
                msg1s = list(map(str, msg1))
                verificationtype = value.get("verificationtype")
                report_file.write("%s:: (Received):: " % testobj)
                for ls in msg1s:
                    report_file.write("%s , " % ls)
                report_file.write("\n")
                if not msg1set:
                    app.setLabel(key, key + " :: " + testobj + " :: UNAVAILABLE")
                    app.setLabelFg(key, "orange")
                    continue

                #Verify an exact value is meet for a given testobj
                if verificationtype == "exact":
                    exactvalue = value.get("value")
                    result = all(str(x) == str(exactvalue) for x in msg1set)

                #Verify a testobj value is within acceptable range
                elif verificationtype == "range":
                    rangevalue = value.get("value")
                    minvalue = rangevalue[0]
                    maxvalue = rangevalue[1]
                    result = all((int(x) >= minvalue and int(x) <= maxvalue) for x in msg1set)
                    
                #Verify that a testobj is a valid choice
                elif verificationtype == "choice":
                    choices = value.get("choices")
                    result = all(all((y in choices)for y in x.keys())for x in msg1set)

                #Verify that a testobj is complying with a bitstring size
                elif verificationtype == "bitstring":
                    size = value.get("size")
                    
                    #result = all(not(x << size) for x in msg1set)
                    result = True
                    for x in msg1set:
                        convertvalue = int(x,16)
                        totalbits = len(x)*4
                        lowerbits = totalbits - size
                        mask = (1 << lowerbits) - 1
                        if not ((convertvalue & mask) == 0):
                            result = False
                            break
 
                #Display Results
                if result:
                    app.setLabel(key, key + " :: " + testobj + " :: PASSED")
                    app.setLabelFg(key, "green")
                else:
                    app.setLabel(key, key + " :: " + testobj + " :: FAILED")
                    app.setLabelFg(key, "red") 

            elif testtype == "print":
                ret = list(find(testobj, parsed))
                if len(ret) > 0:
                    app.setLabel(key, key + " :: " + testobj + ":: " + str(ret))
                    app.setLabelFg(key, "green")
                else:
                    app.setLabel(key, key + " :: " + testobj + " :: UNAVAILABLE")
                    app.setLabelFg(key, "orange")

    app.stopLabelFrame()

    if createdTestResult_local:
        createdTestResult = True

    app.stopAllPanedFrames()


def process_payload(payload_hex: str, source_ip: str, source_port: int, psid: Optional[str]) -> None:
    global parsed

    try:
        decoded_json = j2735decoder(payload_hex)
        message_id = extract_message_id(decoded_json)
        message_name = extract_message_name(decoded_json)

        pkt = DecodedPacket(
            recv_time=time.time(),
            source_ip=source_ip,
            source_port=source_port,
            psid=psid,
            payload_hex=payload_hex,
            decoded_json=decoded_json,
            message_id=message_id,
            message_name=message_name,
        )

        MESSAGE_STORE.add(pkt)

        msg_queue = message_queue_thread(message_id)
        msg_queue.put(pkt)

        parsed = json.loads(json.dumps(decoded_json), parse_int=str)

        print("\n**** DECODE OK ****")
        print("Source     : {}:{}".format(source_ip, source_port))
        # print("PSID       : {}".format(psid if psid is not None else "N/A"))
        print("messageId  : {}".format(message_id if message_id is not None else "UNKNOWN"))
        print("messageName: {}".format(message_name if message_name is not None else "UNKNOWN"))
        # print("payloadLen : {} bytes".format(len(payload_hex) // 2))
        print("payloadHex : {}".format(payload_hex))
        # print("Queue counts by messageId:", MESSAGE_STORE.snapshot_counts())
        print("Decoded JSON:")
        print(json.dumps(decoded_json, indent=2))
        print_message_queue_status()

        msg_filter = app.getOptionBox("Message Filter")
        if (msg_filter == DSRC_msg_ids.get(message_id)):
            ValidateMessage(message_id,decoded_json)
#######  uncomment app.queueFunction(certify) when ready to try validating messages, commented out to avoid needing test template ######
        # app.queueFunction(certify)

    except Exception as exc:
        print("\n**** decode failure ****")
        print("Source     : {}:{}".format(source_ip, source_port))
        print("PSID       : {}".format(psid if psid is not None else "N/A"))
        print("payloadLen : {} bytes".format(len(payload_hex) // 2))
        print("payloadHex : {}".format(payload_hex))
        print("Error      : {}".format(exc))


def udp_listener(bind_ip: str, bind_port: int) -> None:
    global running, listener_socket

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        listener_socket = sock
        sock.bind((bind_ip, bind_port))
        sock.settimeout(1.0)

        print("Listening on UDP {}:{}".format(bind_ip, bind_port), flush=True)

        while running:
            try:
                data, addr = sock.recvfrom(65535)
                print("UDP packet received from {}:{} len={}".format(addr[0], addr[1], len(data)), flush=True)
            except socket.timeout:
                continue
            except Exception as exc:
                if running:
                    print("Socket receive error:", exc, flush=True)
                continue

            source_ip, source_port = addr[0], addr[1]
            candidates = extract_payload(data)

            if not candidates:
                print("\nNo J2735 payload found from {}:{}".format(source_ip, source_port), flush=True)
                raw_hex = binascii.hexlify(data).decode("ascii").lower()
                print("Raw datagram hex:", raw_hex, flush=True)
                continue

            for payload_hex, psid in candidates:
                if payload_hex:
                    process_payload(payload_hex, source_ip, source_port, psid)

    except Exception as exc:
        print("Listener startup error:", exc, flush=True)

    finally:
        if listener_socket is not None:
            try:
                listener_socket.close()
            except Exception:
                pass
        listener_socket = None
        print("Listener stopped.", flush=True)


def stop_listener():
    global running, listener_thread, listener_socket

    print("\nStopping listener (no new packets)...")
    running = False

    if listener_socket is not None:
        try:
            listener_socket.close()
        except Exception:
            pass

    # Wait for listener thread to exit
    if listener_thread is not None:
        listener_thread.join(timeout=2)

    print("\nWaiting for all message queues to finish processing...")

    # Wait for all queues to drain
    with queue_lock:
        queues = list(message_queues.items())

    for message_id, msg_queue in queues:
        print(f"Waiting for messageId {message_id} queue to empty...")
        msg_queue.join()

    print("\nAll queues finished. Stopping workers...")

    stop_message_processors()

    # Wait for workers to exit
    with queue_lock:
        workers = list(message_workers.items())

    for message_id, worker in workers:
        print(f"Waiting for worker {message_id} to exit...")
        worker.join(timeout=2)

    print("\nShutdown complete.")


def run():
    global running, listener_thread, ValidateSample

    print("Run Button Pressed", flush=True)

    if ValidateSample:
        initialMessageText = app.getTextArea("message content")
        with open("./.localsample", "w") as f:
            f.write(initialMessageText)
        initm = json.loads(initialMessageText)
        dict2str2 = json.dumps(initm, indent=4, sort_keys=True, ensure_ascii=False)
        decoded_msg = json.loads(dict2str2, parse_int=str)
        ValidateMessage(int(decoded_msg.get("messageId")),decoded_msg)
        return 0
    
    port_value = app.getEntry("Server port")
    print("Server port raw value:", repr(port_value), flush=True)

    try:
        serverPort = int(float(port_value))
    except (TypeError, ValueError):
        print("Invalid Server port:", repr(port_value), flush=True)
        app.setLabel("er1", "Invalid Server port")
        app.setLabelBg("er1", "red")
        return

    if listener_thread is not None and listener_thread.is_alive():
        print("Listener is already running on port {}".format(serverPort), flush=True)
        return

    running = True
    listener_thread = threading.Thread(
        target=udp_listener,
        args=("0.0.0.0", serverPort),
        daemon=True,
    )
    listener_thread.start()
    print("Started listener thread on UDP port {}".format(serverPort), flush=True)


def preview():
    global testTemplate, message, createdTestSetup, ValidateSample
    app.clearLabel("er1")
    app.clearLabel("er2")
    app.clearLabel("er3")
    app.clearLabel("er4")

    filepath1 = app.getEntry("Local sample file")
    snifferscript = app.getTextArea("Sniffer Script")

    flag = 1
    if filepath1:
        ValidateSample = True
        with open(filepath1, "r") as file:
            sampleMsg = file.read()
        message = sampleMsg
        app.clearTextArea("message content")
        app.setTextArea("message content", message, end=False)
        samplejson = safe_json_loads(sampleMsg)
        LoadTests(samplejson.get("messageId"))
    else:
        ValidateSample = False
        message_filter = app.getOptionBox("Message Filter")
        msg_id = int(list(DSRC_msg_ids.keys())[list(DSRC_msg_ids.values()).index(message_filter)])
        LoadTests(msg_id)


def setupTestTemplate():
    global testTemplate, createdTestSetup, testlist

    if createdTestSetup is False:
        testlist = testTemplate.get("testlist")
        app.startPanedFrame("t1")

        app.startLabelFrame("Test information")
        app.addLabel("Testgroup", "Testgroup : " + testTemplate.get("testgroup",""))
        app.addLabel("Testoperator", "Testoperator : " + testTemplate.get("testoperator",""))
        if testTemplate.get("testdate","") == "":
            today = date.today()
            app.addLabel("Testdate", "Date : " + today.strftime("%d/%m/%Y"))
        else:
            app.addLabel("Testdate", "Date : " + testTemplate["testdate"])

        if testlist != None:
            for keys in testlist.keys():
                app.addCheckBox(keys)
                app.setCheckBox(keys)
        else:
            app.addMessage(f"Press run to start UDP server.")

        createdTestSetup = True
        app.stopLabelFrame()
        app.addButton("Run", run)
        app.addButton("Stop", lambda btn=None: stop_listener())

    else:
        app.openLabelFrame("Test information")
        if testlist != None:
            for keys in testlist.keys():
                app.removeCheckBox(keys)

        testlist = testTemplate.get("testlist")
        app.setLabel("Testgroup", "Testgroup : " + testTemplate.get("testgroup",""))
        app.setLabel("Testoperator", "Testoperator : " + testTemplate.get("testoperator",""))

        if testTemplate.get("testdate","") == "":
            today = date.today()
            app.setLabel("Testdate", "Date : " + today.strftime("%d/%m/%Y"))
        else:
            app.setLabel("Testdate", "Date : " + testTemplate["testdate"])
      
        if testlist != None:
            for keys in testlist.keys():
                app.addCheckBox(keys)
                app.setCheckBox(keys)
        app.stopLabelFrame()

def on_stop():
    stop_listener()
    app.stop()


app = gui()
app.setTitle("RSE Test Plan: Message Certification Tool v1.1 - J2735_2024")
app.setFont(size=11)
app.setSize("1800x800")
app.setGuiPadding(1, 1)
app.setLabelFont(size=11, underline=False)

app.startPanedFrame("ts1")
app.startLabelFrame("DUT Configurations")
app.addLabelOpenEntry("Local sample file")
app.stopLabelFrame()

app.startLabelFrame("Test Setup")
app.addLabelOptionBox("Message Filter",DSRC_msg_ids.values())
app.addLabelNumericEntry("Server port")
app.setEntryDefault("Server port", "5398")
app.stopLabelFrame()

app.addTextArea("Sniffer Script")
app.setTextAreaAspect("Sniffer Script", 400)

app.addButton("Preview", preview)

app.startLabelFrame("Debug")
app.addLabel("er1", "")
app.addLabel("er2", "")
app.addLabel("er3", "")
app.addLabel("er4", "")
app.stopLabelFrame()

app.startPanedFrame("ms1")
app.setSticky("news")
app.addScrolledTextArea("message content", 1, 1, 2, 2)

app.setStopFunction(on_stop)
app.go()