# CARMA Cloud Plugin Documentation

## Introduction

The CARMA Cloud Plugin is responsible for connecting V2X-Hub to [CARMA Cloud](https://github.com/usdot-fhwa-stol/carma-cloud), the STOL Cloud Cooperative Driving Automation (CDA) application supporting use cases such as work zones.

## Related Plugins

A list of plugins related to the CARMA Cloud Plugin.

### Immediate Forward Plugin

Provides RSU Immediate Message Forwarding (IMF) functionality for broadcasting V2X messages, such as Traffic Control Messages (TCMs), from CARMA Cloud to V2X actors.

### Message Receiver Plugin

Receives V2X communications from V2X actors, such as Connected and Automated Vehicles (CAVs), including Traffic Control Requests (TCRs).

### Emergency Response Vehicle (ERV) Cloud Forwarding

For forwarding ERV communications from V2X actors to CARMA cloud.

## Configuration / Deployment

The CARMA Cloud Plugin supports the following configuration parameters.

### Connection Settings

| Parameter                | Description                                                                                                              |
| ------------------------ | ------------------------------------------------------------------------------------------------------------------------ |
| `WebServicePort`         | Port used by V2X-Hub to receive TCM messages from CARMA Cloud through the SSH tunnel.                                    |
| `CARMACloudBaseUrl`      | Base HTTPS URL used to communicate with CARMA Cloud through the SSH tunnel. Example: `https://host.docker.internal:8443` |
| `enforceTLSVerification` | Enables TLS certificate verification for CARMA Cloud connections. Default: `true`.                                       |

> [!WARNING]
> `enforceTLSVerification=false` should **ONLY** be used for local development or debugging purposes. V2X-Hub must be compiled with `BUILD_TYPE` of `Debug`. Production deployments should always enable TLS verification.

---

### TCM Settings

| Parameter                          | Description                                                                                        |
| ---------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `fetchTime` | Number of days in the past for which TCMs are requested from CARMA Cloud.                                             |
| `listTCM`   | Determines whether CARMA Cloud returns a list of TCMs (`true`) or individual seperate TCM messages (`false`). Default: `true`. |
| `TCMRepeatedlyBroadcastTimeOut`    | Duration in milliseconds that a received TCM will continue to be rebroadcast.                      |
| `TCMRepeatedlyBroadcastSleep`      | Sleep interval in milliseconds between repeated TCM broadcasts.                                    |
| `TCMRepeatedlyBroadCastTotalTimes` | Maximum number of repeated broadcasts for TCMs with the same request ID during the timeout period. |
| `TCMNOAcknowledgementDescription`  | If no acknowledgement is received from a CMV within the configured timeout period for a matching TCM, the plugin generates a `NO ACK` message for display in the UI. |


### TCP Tunnel

To securely connect V2X-Hub to a remotely hosted CARMA Cloud instance, SSH forward and reverse tunnels must be configured. The forward tunnel forwards HTTPS traffic from V2X-Hub's host environment to the remote CARMA Cloud server.  The reverse tunnel forwards TCM reply traffic from the remote CARMA Cloud server to V2X-Hub's host environment. The steps to configure these tunnels are:

1. Provision the required `.pem` key file for SSH authentication and place it in the `./scripts/` directory.

2. Run `./call.sh` from the `./scripts/` directory to establish the SSH tunnel.

3. Start V2X-Hub. Configure the `CARMACloudPlugin`:
   * Set `CARMACloudBaseUrl` to use HTTPS and port `8443`
   * Example: `https://host.docker.internal:8443`

## Design
![Alt text](docs/CARMACloudCommunicationArchitecture.png)
The diagram above illustrates how the CARMA Cloud Plugin functions within V2X-Hub. The plugin maintains communication with CARMA Cloud to exchange traffic control information.

When Traffic Control Requests (TCRs) are received, the plugin forwards them to CARMA Cloud. CARMA Cloud responds with Traffic Control Messages (TCMs), which are then broadcast to vehicles to provide updates to their local maps, such as lane closures, speed limits, and other dynamic work zone information.

### Messages

**TCR**: This message contains information from a requesting CAV about what traffic controls it wants to know about. This includes information about time and location for which it wants traffic controls. 

**TCM**: This message contains information about traffic controls like speed limits or lane closures and geographic information about the locations they apply to.
> [!NOTE]
> **TCM** and **TCR** are CARMA ecosystem prototype messages that have been proposed to the SAE standard for inclusion in J2735 V2X Message set.

## Technical Communication Flow

### Request traffic
The Message Receiver Plugin receives a TCR from an RSU and publishes it to the CARMA Cloud Plugin. The CARMA Cloud Plugin then sends a TCM request with the TCR message to CARMA Cloud using an HTTPS POST over the SSH forward tunnel. CARMA Cloud processes the request and prepares to return the appropriate HTTP TCM response initiated by CARMA Cloud server to V2X-Hub. 

> [!WARNING]
> This does not follow the typical HTTP request/response pattern.  CARMA Cloud initiates a separate HTTP TCM response back to V2X-Hub after processing the TCM request.

### Request communication flow with code references:
```
Message Receiver Plugin
    ↓
CARMA Cloud Plugin: CARMACloudPlugin::CloudSend() sends HTTPS POST to `https://host.docker.internal:8443/carmacloud/tcmreq` (or the configured CARMACloudBaseUrl/carmacloud/tcmreq)
    ↓
SSH forward tunnel maps the host machine HTTPS endpoint to the remote CARMA Cloud HTTPS endpoint (e.g., V2X-Hub’s host machine localhost:33333 → CARMA Cloud localhost:8443)
    ↓
CARMA Cloud: TcmReqServlet.doPost() / run() handles `/carmacloud/tcmreq`
```

> [!IMPORTANT]
> - HTTPS is used for communication between V2X-Hub and CARMA Cloud to improve transport security and support stronger TLS configurations.
> - WARNING: The SSH forward tunnel is required still because CARMA Cloud's `TcmReqServlet.java` reads the caller’s IP address with `getRemoteAddr()` for the TCM response.
> - WARNING: The SSH forward tunnel remains required because CARMA Cloud's `TcmReqServlet.java` reads the caller’s IP address from incoming request with `getRemoteAddr()` to determine the callback destination for TCM responses.
> - The SSH forward tunnel is configured with the `-g` option so Docker containers can access the forwarded port through `host.docker.internal`.
> - `host.docker.internal` should be used when the SSH tunnel is established on the host machine while V2X-Hub is running inside a Docker container.
> - The default HTTPS forwarding port is `8443`, but this may vary depending on deployment configuration.

### Reply Traffic
After processing the TCR request, CARMA Cloud sends the corresponding TCM response back to V2X-Hub using an HTTP POST over the SSH reverse tunnel. The response destination is determined using the source host from the original HTTPS TCR request together with the callback port defined in the TCR message.

> [!WARNING]
> This communication pattern does not follow the traditional HTTP request/response flow.
> CARMA Cloud initiates a separate outbound HTTP POST back to V2X-Hub after asynchronously processing the original TCR request.

### Reply communication flow with code references:

```
CARMA Cloud: TcmReqServlet.run() processes TCR request sends HTTP POST to `http://127.0.0.1:22222/tcmreply` (or the source host from the original /carmacloud/tcmreq request and callback port defined in the TCR)
    ↓
SSH reverse tunnel which maps the remote CARMA Cloud server HTTP endpoint to V2X-Hub (e.g., CARMA Cloud’s localhost:22222 to V2X-Hub’s container localhost:22222)
    ↓
CARMACloudPlugin::CARMAResponseHandler handles /tcmreply
```

> [!IMPORTANT]
> - CARMA Cloud determines the callback destination from:
>    - the source IP/host of the original `/carmacloud/tcmreq` request
>    - the callback port provided in the TCR message
> - The SSH reverse tunnel allows CARMA Cloud to securely initiate communication back to V2X-Hub even when V2X-Hub is running behind NAT or inside a Docker container.
> - The reverse tunnel traffic currently uses HTTP because the communication is protected by the SSH tunnel itself.
> - The `/tcmreply` endpoint is hosted by the CARMA Cloud Plugin within V2X-Hub.

## Functionality Testing

To test functionality of CARMA Cloud Plugin without an active vehicle, we have provided a script which can send mock TCRs to V2X-Hub. These TCRs should be received by the CARMA Cloud Plugin, forwarded to CARMA Cloud, and if there are any relevant active traffic controls, should result in response TCMs being sent to the CARMA Cloud Plugin. Steps to conduct this test are outlined below:

1. Deploy V2X-Hub and CARMA Cloud.
2. Configure the CARMA Cloud Plugin, including the required SSH tunnels.
3. Enable both the CARMA Cloud Plugin and Message Receiver Plugin.
4. Run `python3 tcr_script.py` to send a mock TCR to the Message Receiver Plugin.
5. Confirm that TCM messages are received by the CARMA Cloud Plugin using the Messages tab in the V2X-Hub Admin UI.