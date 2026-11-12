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

## Status Information

The plugin reports the following status for real-time monitoring of FLIR camera detection data, as totals over all FLIR camera connections:

* **Dropped Pedestrian Detections**: Detections dropped because they could not be processed.
* **Modified Pedestrian Detections (Bad Data: Assumed Stationary)**: Detections modified because of bad incoming data (see the note below).
* **Unique Pedestrian Detections**: Detections of new pedestrians, found by comparing incoming detection IDs with the previously received IDs. This is a rough estimate of the number of pedestrians detected.
* **Total Pedestrian Detections**: All pedestrian detections, including dropped and modified ones.
* **Live number of currently detected pedestrians**: Unique pedestrians currently detected.
* **Camera Link Stalls**: Number of [link stalls](#camera-link-monitoring) on all camera connections.
* **Camera Link Last Stall**: The latest stall's sensor, hold, and start and end times (UTC).

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

## Functionality Testing

`scripts/mockFLIRCamera.py` mocks the WebSocket that the FLIR camera hosts for pedestrian presence tracking. Use it to check the plugin without a camera:

1. Install Python 3 and pip.
2. Run `python3 -m pip install -r scripts/requirements.txt` to install the dependencies.
3. Run `python3 scripts/mockFLIRCamera.py --ip <ip> --port <port>`.
4. Configure the FLIR Camera Driver Plugin to connect to the mock. Sensor Detected Objects should appear in the plugin's **Messages** tab.

To test camera link monitoring, add `--stall-every <seconds> --stall-ms <milliseconds>` to the mock. It then periodically holds the frames it produces for the given time and sends them all at once. The plugin should log a `FLIR link stall` for each, and the **Camera Link Stalls** status should count them.
