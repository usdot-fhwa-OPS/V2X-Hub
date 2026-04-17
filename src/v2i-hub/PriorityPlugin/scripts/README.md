# Mock-SRM

The [mock-srm.py](mock-srm.py) script allows for testing multiple SRMs with the Priority Plugin.

## Installation

Run the [installation script](install.sh) to install all the dependencies.

## Usage

1. Enable the MessageReceiver plugin with RouteJ2735 set to true. Use all other default configurations.

2. Optional: Enable the JSONMessageLogger plugin to decode and log all received SRMs and generated SSMs.

3. Run the mock-srm.py script.

4. Add any amount of SRM entries to send. Edit the fields as desired.

5. Send SRM.
