# V2X-Hub Integration Test Inputs

This folder stores **input payloads** used for V2X-Hub integration testing.

## What this gives you

- A single place to store test input payloads
- Inputs grouped by plugin so it is easy to find and reuse
- Payloads that can be injected using the tools in `tests/integration/injectors/`

---

## Folder structure

Inputs are grouped by plugin name as below examples:

- `inputs/MAP/`
  - File-based inputs for MAP plugin
- `inputs/SPAT/`
  - UDP payloads for SPAT plugin
- `inputs/MessageReceiver/`
  - UDP payloads for MessageReceiver plugin

Add more folders as we add more plugins.

---

## Input injector to use

- If the plugin configuration shows a **file path** (`FilePath` / `*Files`)
  → use `injectors/inject_file.py`

- If the plugin configuration shows a **UDP port** (`Port` / `Local_UDP_Port`)
  → use `injectors/inject_udp.py`

---

## Notes

- Keep filenames simple.
- These payloads are inputs only. Output validation is done by pytest
---

## Example usage commands

### File-based injection (example: MAP)

Use this when the plugin configuration shows a file path or expects input from a file (`FilePath` / `*Files`).

Example: MAP plugin reads from:
`/var/www/download/MAP/MAP_9709_UPER.txt`

```bash
python3 tests/integration/injectors/inject_file.py \
  --payload tests/integration/inputs/MAP/mapupermsg.txt \
  --service v2xhub \
  --dest /var/www/download/MAP/MAP_9709_UPER.txt \
  --restart
```

Notes:

- --dest must match the file path shown in the plugin configuration.
- --restart is used when the plugin loads the file only at startup.

### UDP-based injection (example: SPAT)

Use this when the plugin configuration shows a UDP port (Local_UDP_Port / Port).

SPAT listens on UDP port 6053 in the plugin configuration.

```bash
python3 tests/integration/injectors/inject_udp.py \
  --payload tests/integration/inputs/SPAT/spat_1721238398773.bin \
  --port 6053 \
  --repeat 10 \
  --delay 0.2
```
Expected result: SPAT plugin shows J2735 SPAT-P message count increasing.

### UDP-based injection (example: MessageReceiver)

MessageReceiver listens on UDP port 26789 in the plugin configuration.

```bash
python3 tests/integration/injectors/inject_udp.py \
  --payload tests/integration/inputs/MessageReceiver/sample_bsm.bin \
  --port 26789 \
  --repeat 20 \
  --delay 0.1
```
Expected result: MessageReceiver shows J2735 BSM message count increasing.