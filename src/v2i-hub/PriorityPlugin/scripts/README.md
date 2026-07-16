# Mock-SRM

The [mock-srm.py](mock-srm.py) script allows for testing multiple SRMs with the Priority Plugin.
### Inputs

- Configurations to create a single or multiple Signal Request Messages (SRMs), entered via a UI.

### Outputs

- One SRM, udp-sent to the Message Receiver Plugin. 
- A series of SRMs, sequentially updated to simulate an approach to an intersection. UDP-sent to the Message Receiver Plugin. 

## Installation

Run the [installation script](install.sh) to install all the dependencies for the mock-srm.py script.

## Usage

1. Enable the MessageReceiver plugin with RouteJ2735 set to **true**. Use all other default configurations.

2. Optional: Enable the JSONMessageLogger plugin to decode and log all received SRMs and generated SSMs.

3. Run the mock-srm.py script.

4. Add any amount of SRM entries to send. Edit the fields as desired.

5. Select `Send SRM` or `Simulate Approach`.
