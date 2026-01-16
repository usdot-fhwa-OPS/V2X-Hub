# V2X-Hub Integration Test Input Injectors

This folder provides small **input injection tools** for a running V2X-Hub integration environment.

It is meant to be used:
- locally
- in CI later
- inputs only (no output validation in this story)

---

## What this gives you

- A reusable way to inject inputs into V2X-Hub for integration testing
- Support for two common input types:
  - file-based inputs (copy payload to a configured path that Plugins expect)
  - UDP-based inputs (send bytes to a plugin UDP port)
  - Any new input types can me added for injection for other plugins in future.

These scripts only inject inputs.
Output validation and pass/fail checks come later in future implemetation stories.

---

## Target plugin

These injectors are generic. You choose the target plugin by using:

- the plugin’s configured file path (`--dest`) for file-based inputs, or the plugin’s UDP port (`--port`) for UDP-based inputs.

## _common.py

- _common.py provides shared helper functions (find_repo_root, load_env, sh) used by both inject scripts.
- This avoids repeating common logic for environment loading, Docker Compose setup, and command execution.

## toggle_plugin.py

- toggle_plugin.py is a small utility used by inject_file.py to force plugin reloads.
- It disables and re-enables the specified plugin in the integration database (e.g., MAP) with a short wait between actions so that the plugin fully restarts and reloads its configuration or file inputs.

## Prereqs

- Integration environment is running (OMDO-90):

```bash
./tests/integration/env/run.sh
```