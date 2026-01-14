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

## Prereqs

- Integration environment is running (OMDO-90):
```bash
./tests/integration/env/run.sh
```