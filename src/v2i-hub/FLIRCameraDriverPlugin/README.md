# FLIR Camera Driver Plugin Documentation

## Introduction

The FLIR Camera Driver Plugin is the hardware interface for the FLIR sensor. It translates FLIR pedestrian presence tracking data into V2X Hub Sensor Detected Object messages. Downstream plugins or services can consume these detections for J2735 message generation. Currently this is supported by the [CARMA Streets](https://github.com/usdot-fhwa-stol/carma-streets) [Sensor Data Sharing Service](https://github.com/usdot-fhwa-stol/carma-streets/tree/develop/sensor_data_sharing_service), which generates SDSMs.

## Related Plugins

### Immediate Forward Plugin

Forwards any generated J2735 message, e.g. SDSM, to the RSU for RSU Immediate Message Forwarding (IMF).

### CARMA Streets Plugin

Required for SDSM generation, which is handled by the [CARMA Streets](https://github.com/usdot-fhwa-stol/carma-streets) [Sensor Data Sharing Service](https://github.com/usdot-fhwa-stol/carma-streets/tree/develop/sensor_data_sharing_service). The CARMA Streets plugin forwards the Sensor Detected Object messages from this plugin to the Sensor Data Sharing Service. The service generates SDSMs and sends them back to V2X Hub through the CARMA Streets plugin, and the Immediate Forward Plugin broadcasts them through the RSU.

## Configuration/Deployment

**FLIRConfigurations**: JSON list of one or more FLIR cameras to connect to.

```json
[
  {
    "WebSocketHost": "127.0.0.1", // IP address of the FLIR camera
    "WebSocketPort": "8081", // WebSocket port
    "Endpoint": "/api/subscriptions", // WebSocket endpoint
    "CameraRotation": 16.13, // Camera heading in degrees clockwise from North
    "SensorId": "FLIR_Turner_Parking_Lot", // Sensor ID
    "CameraLatitude": 38.95499217, // Camera reference point: detection positions are relative to it
    "CameraLongitude": -77.14920953
  },
  {
    "WebSocketHost": "127.0.0.1",
    "WebSocketPort": "8082",
    "Endpoint": "/api/subscriptions",
    "CameraRotation": 11.13,
    "SensorId": "FLIR_Crosswalk",
    "CameraLatitude": 38.95499217,
    "CameraLongitude": -77.14920953
  }
]
```

The following parameters configure [camera link monitoring](#camera-link-monitoring). Like `FLIRConfigurations`, they are read when the plugin starts.

**LinkStallThresholdMs** (default `500`, minimum `200`): A camera frame held this many milliseconds longer in transit than the frame before it is logged as a link stall. 500 ms matches the freshness limit of the Sensor Data Sharing Service, which discards older detections.

**LinkCheckIntervalMs** (default `200`, minimum `50`): Milliseconds between link checks, TCP connections opened and closed again, of each camera and control host. It is also the time each check may take. `0` disables link checks. Raise it if round trips to the cameras are longer than about 50 ms, e.g. over a cellular or radio backhaul.

**LinkCheckControlHosts** (default empty): Comma-separated `host[:port]` list (port 80 by default) of other devices on the network, checked like the cameras, e.g. `192.168.55.1:80` for the gateway. Any port works: a host that refuses the connection still proves the network path to it is up. Choose devices whose path from V2X Hub is the same as the cameras' as far as possible, such as the gateway or another device on the cameras' switch.

## Status Information

The plugin reports the following status for real-time monitoring of FLIR camera detection data, as totals over all FLIR camera connections:

* **Dropped Pedestrian Detections**: Detections dropped because they could not be processed.
* **Modified Pedestrian Detections (Bad Data: Assumed Stationary)**: Detections modified because of bad incoming data (see the note below).
* **Unique Pedestrian Detections**: Detections of new pedestrians, found by comparing incoming detection IDs with the previously received IDs. This is a rough estimate of the number of pedestrians detected.
* **Total Pedestrian Detections**: All pedestrian detections, including dropped and modified ones.
* **Live number of currently detected pedestrians**: Unique pedestrians currently detected.
* **Camera Link Stalls**: Number of [link stalls](#camera-link-monitoring) on all camera connections.
* **Camera Link Last Stall**: The latest stall's sensor, hold, start and end times (UTC), and once known, its verdict.
* **Camera Link Failed Checks**: Link checks to cameras that were not answered or were refused.
* **Control Host Failed Checks**: Link checks to control hosts that were not answered.
* **Silent Cameras**: Cameras whose connection has carried nothing for 90 s, or `None`.

> [!NOTE]
> Only detections with incomplete or invalid heading and speed are modified. In testing, when a detected object stops, the FLIR camera occasionally keeps broadcasting detections that **do not** include an angle but **do** include a speed. This appears to be an intermittent bug for stationary objects. To handle it, the plugin assumes a speed of 0 when no angle is provided. For traceability, the Modified Pedestrian Detections status counts these detections, and each Sensor Detected Object message has a flag that indicates whether its data was modified.

![FLIR Camera Driver Plugin status](docs/FLIRCameraDriverStatus.png)

## Design

![FLIR Camera Driver Plugin architecture](docs/FLIRCameraDriverArchitecture.drawio.png)

The diagram above shows how the FLIR Camera Driver Plugin is used to create Sensor Data Sharing Messages (SDSM) for cooperative perception.

### Messages

**SensorDetectedObject**: V2X Hub message that contains detection data. It can be used to generate other messages, such as SDSM.

**SensorDataSharingMessage**: V2X cooperative perception message defined in the J3224 standard.

### Camera Link Monitoring

The camera streams tracking data over a WebSocket, a single TCP connection. If packets on it are lost, TCP holds back every later frame until a retransmission gets through, so detections stop for a few seconds and then arrive all at once. The Sensor Data Sharing Service discards detections that late. See the [design](docs/CameraLinkMonitoring.md) for details; parts of it are still being added.

**Stall detection.** The plugin compares when each frame arrives with the camera's `time` field. A frame held at least `LinkStallThresholdMs` longer in transit than the frame before it is logged as a stall. Only differences between consecutive frames are used, so the camera and V2X Hub clocks do not need to be synchronized:

```
WARNING FLIR link stall on FLIR_North: frames held 3483 ms in transit, from 2026-09-15T17:47:10.136Z to 2026-09-15T17:47:13.619Z (the camera produced the held frame 100 ms after the previous one).
```

The times are UTC: use them to look up the same moment in a packet capture or switch log. If the camera produced the held frame much more than 100 ms after the previous one, the camera was idle before it, and the outage began at some point during that time.

**Link checks.** Independently of the stream, the plugin opens a TCP connection to each camera, on its WebSocket port, and to each control host every `LinkCheckIntervalMs`, and closes it again. A check that is not answered within the interval, or fails with an error such as "No route to host", has failed. A camera that refuses the connection is up on the network, but its web service is not. A run of failed checks is logged when it starts and when it ends, so outages are recorded even while the camera is idle:

```
WARNING FLIR link check: camera FLIR_North (192.168.55.83:80) stopped answering at 2026-11-10T14:18:02.200Z: no answer within 200 ms.
WARNING FLIR link check: camera FLIR_North (192.168.55.83:80) answering again at 2026-11-10T14:24:11.400Z, after 1846 failed checks over 369200 ms.
```

**Verdict.** For each stall, and for each run of failed camera checks when it ends, the plugin logs where the frames were held, judged from the link checks that ran at the time:

```
WARNING FLIR link stall on FLIR_North from 2026-09-15T17:47:10.136Z to 2026-09-15T17:47:13.619Z: camera link. The camera stopped answering while the control hosts answered, so packets were lost between the network and the camera: its network port, its cable or PoE, or its switch port. Link checks during this time: camera answered 11 of 17, control hosts 17 of 17.
```

| Verdict | Link checks during the stall | Where to look |
|---|---|---|
| network path up | The camera answered every check. | Software: the camera's sender, or this host. |
| camera service down | The camera refused connections. | The camera's web service. |
| camera link | The camera stopped answering, the control hosts answered. | The camera's network port, its cable or PoE, its switch port. |
| host network | The camera and the control hosts stopped answering. | This host's network port, cable or switch, or equipment shared with the camera. |
| camera unreachable | The camera stopped answering; no control hosts are configured. | Set `LinkCheckControlHosts` to narrow it down. |
| not checked | No checks ran, e.g. `LinkCheckIntervalMs` is 0. | |

Longer stalls overlap more checks, so their verdicts are more certain. Packet loss that affects only large frames, such as an MTU mismatch, does not show up in the checks.

**Silence watchdog.** While nothing is tracked, the camera sends no data but sends a WebSocket ping every 60 s. If a camera connection carries nothing at all for 90 s, the plugin logs a warning with the verdict for the silent period, and logs again when data resumes. The plugin does not reconnect on its own, so a connection that stays silent, e.g. after the camera restarted, needs a plugin restart.

```
WARNING FLIR camera FLIR_North has sent nothing, not even keepalive pings, since 2026-11-10T14:18:01.950Z (90 s). Link verdict: camera link. ...
```

## Functionality Testing

`scripts/mockFLIRCamera.py` mocks the WebSocket that the FLIR camera hosts for pedestrian presence tracking. Use it to check the plugin without a camera:

1. Install Python 3 and pip.
2. Run `python3 -m pip install -r scripts/requirements.txt` to install the dependencies.
3. Run `python3 scripts/mockFLIRCamera.py --ip <ip> --port <port>`.
4. Configure the FLIR Camera Driver Plugin to connect to the mock. Sensor Detected Objects should appear in the plugin's **Messages** tab.

To test camera link monitoring, add `--stall-every <seconds> --stall-ms <milliseconds>` to the mock. It then periodically holds the frames it produces for the given time and sends them all at once. The plugin should log a `FLIR link stall` for each, and the **Camera Link Stalls** status should count them. Since the mock holds the frames in software, each stall's verdict should be `network path up`. To test the silence watchdog, suspend the mock with `kill -STOP <pid>` for more than 90 s, then resume it with `kill -CONT <pid>`.
