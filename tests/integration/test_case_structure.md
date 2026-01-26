# Integration Test Case Structure (V2X-Hub)

## Goal
This document is to provide a consistent way to define automated integration test cases for V2X-Hub repository.

This document mainly defines:
- where test cases live
- what files and folders a test case contains
- how a test case declares inputs (file / PCAP; UDP)
- how a test case declares validations (logs / message counts/ additional validation types can be added when needed)
- how tests are executed consistently (pytest + testcontainers)

---

## Test Case Guidelines
1) **Repeatable and self-contained**
   - Inputs live inside the test case folder.

2) **No custom SQL in tests**
   - Tests should not enable/disable plugins by invoking MySQL commands directly.
   - Plugin state should come from supported mechanisms (for example, startup scripts like `configuration/service.sh`, or a saved DB state restored at container startup).

3) **UI is not required for pass/fail**
   - CI pass/fail should validate via logs, output files, or message counts.
   - UI checks can help with local debugging.

4) **Prefer PCAP replay for packet-based inputs**
   - Single-packet sending is not the preferred abstraction.
   - PCAP is the standard format for replaying a collection of packets.

5) **Execution should standardize on pytest + testcontainers**
   - Minimize start/stop shell scripts.
   - Shared runner utilities should be callable directly from pytest.

---

## Where test cases live
All integration test cases live under:

`tests/integration/cases/`

Each test case is a subfolder:

`tests/integration/cases/<test_case_name>/`

Examples:
- `tests/integration/cases/map_sample_test/`
- `tests/integration/cases/spat_sample_test/`
- `tests/integration/cases/message_receiver_bsm/`

---

## Standard test case structure

```
tests/integration/cases/<test_case_name>/
  test_case.yaml
  test_<test_case_name>.py
  inputs/
    ... input files (text/bin/pcap/db dump/etc.)
  expected/
    ... expected outputs for comparison (can be empty)
  README.md
    ... test case documentation
```

### Required files
**1) `test_case.yaml`**  
Test case config describing:
- environment requirements (compose files, plugin state source)
- inputs to inject/replay
- validations to perform
This is the single source of truth for the test case.

**2) `test_<test_case_name>.py`**  
Pytest entry file that:
- loads `test_case.yaml` and checks:
  - required fields exist
  - referenced files exist under `inputs/`
- and uses shared utilities to:
  - start containers
  - inject/replay inputs
  - validate outputs
  - tear down cleanly
This keeps tests consistent with pytest (`test_*.py`) and avoids one-off shell scripts per test.

### Required folders

**`inputs/`**  All input artifacts referenced by `test_case.yaml` must live here (PCAPs, MAP files, DB dumps, etc.).  
Keeping inputs inside the test case folder makes the test self-contained and repeatable.

**`expected/`**  
Expected is only for tests that need to compare against a known output artifact (for example: expected decoded output file, expected JSON/text snapshot produced by a receiver, etc.).

This folder can be ignored (and remain empty) because most tests don’t compare files. They pass/fail using:
- logs containing a message
- message count from message receiver
- a file exists in a container

---

## Test case config: `test_case.yaml`

### YAML config structure

```yaml
name: <string>
description: <string>

environment:
  compose_files:
    - <path>
    - <path>

# Use plugin_state only for test cases that require a specific plugin enable/disable state at startup. If a test does not specify plugin_state, the environment starts with the default startup behavior (for example, service.sh).
  plugin_state:
    type: db_dump
    src: inputs/db/v2xhub_state.sql

inputs:
  - name: <string>
    type: file | pcap | udp
    src: <relative path under this test case folder>
    # per-type fields shown below

validations:
  - name: <string>
    type: log_contains | message_count | file_exists
    # per-type fields shown below
```

Notes:
- All `src` paths are relative to the test case directory.
- `compose_files` may include overrides (compose extend/include style).

## Input types

### 1) `file`
Use when a plugin reads from a configured file path.

```yaml
- name: map_file
  type: file
  src: inputs/files/MAP_9709_UPER.txt
  target:
    service: v2xhub
    path: /var/www/download/MAP/MAP_9709_UPER.txt
  reload:
    # Optional. Use only if required for the plugin to re-read the file.
    # Avoid custom SQL.
    strategy: service_restart
    service: v2xhub
```

What the fields mean:
- `src`: file path inside the test case folder
- `target.service`: which compose service/container receives the file
- `target.path`: destination path inside that container
- `reload` (if used): a supported way to re-read the file (example: restart the service)

Optional:
- `reload.strategy`
- `reload.service`

---

### 2) `pcap`
Use to replay multiple packets as captured.

```yaml
- name: spat_replay
  type: pcap
  src: inputs/pcap/spat_trace.pcap
  replay:
    tool: tcpreplay
    target:
      host: v2xhub
      port: 6053
    rate: "1.0"
```

What the fields mean:
- `src`: PCAP file path inside the test case folder
- `replay.tool`: replay mechanism (example: `tcpreplay`)
- `replay.target.host` / `replay.target.port`: where the packets should be replayed
- `rate`: optional replay speed multiplier

---

### 3) `udp` (allowed, not preferred long-term)
Allowed for early/simple cases. Prefer PCAP for long-term packet replay.

```yaml
- name: spat_udp
  type: udp
  src: inputs/bin/spat_sample.bin
  send:
    host: v2xhub
    port: 6053
    repeat: 10
    delay_seconds: 0.2
```

What the fields mean:
- `src`: payload file path inside the test case folder
- `send.host` / `send.port`: where to send the UDP payload
- `repeat` / `delay_seconds`: optional loop controls

---

## Validation types

### 1) `log_contains`
Check that a service/container log contains one or more strings.

```yaml
- name: v2xhub_processed_map
  type: log_contains
  target:
    service: v2xhub
  match:
    - "MAP"
    - "Loaded"
```

### 2) `message_count`
Count messages observed by an output receiver. CI pass/fail should not depend on UI.

```yaml
- name: receiver_saw_spat
  type: message_count
  target:
    service: message-receiver
  message:
    kind: SPAT
  expected_min: 1
```

### 3) `file_exists`
Check that a file exists inside a target service/container.

```yaml
- name: map_file_present
  type: file_exists
  target:
    service: v2xhub
    path: /var/www/download/MAP/MAP_9709_UPER.txt
```

## How tests are executed

Run pytest to execute the integration tests. The test runner will start the environment, apply inputs, and evaluate validations as defined in test_case.yaml.

```bash
pytest -q tests/integration/cases
```

### Execution flow (high level)
Each `test_<test_case_name>.py` follows the same steps:
1. Load `test_case.yaml`
2. Start the environment using `compose_files`
3. Seed plugin state using `plugin_state` (saved DB state), when used
4. Process each `inputs[]` item:
   - `file`: copy into container at `target.path` (and reload if needed)
   - `pcap`: replay the capture to the configured target host/port
   - `udp`: send payload to the configured host/port
5. Run each `validations[]` check
6. Bring down environment
---

### Target direction 
Converge on **pytest + Python testcontainers** so pytest can:
- start docker compose for the test case
- apply plugin state from a saved DB state (no custom SQL scripts in tests)
- inject files into containers (copy or bind mount)
- replay PCAP / send UDP
- evaluate validations (logs/message count/file existence)
- bring down cleanly

---

## Example skeleton test case

```
tests/integration/cases/map_sample_test/
  test_case.yaml
  test_map_sample_test.py
  inputs/
    files/
      MAP_9709_UPER.txt
  expected/
```
