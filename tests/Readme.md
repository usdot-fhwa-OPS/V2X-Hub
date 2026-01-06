# V2X-Hub Integration Test Environment

This folder provides a repeatable **integration test environment** for V2X-Hub using Docker Compose.
It is meant to be used both:
- locally (developer machine)
- in CI later
- environment only (start/stop + optional plugin selection).
---

## What this gives you

- One command to start the V2X-Hub stack for integration testing
- One command to stop/cleanup the stack
- Enable selected plugins (ex: MAP + SPAT) via a separate script.
- Optional extra containers using V2XHUB_IT_PROFILES
- log viewing helpers

---

## Files

### `env/run_v2xhub_integration.sh`
Starts the integration stack using:
- `configuration/docker-compose.yml` (base)
- `tests/integration/env/docker-compose.it.override.yml` (integration override)

Then:
- Waits until UI is reachable (https://127.0.0.1/admin/admin.html)
- runs `apply_integration_seed.sh` if present (plugin selection)
Inputs (env vars):
- V2XHUB_IT_PLUGINS="MAP,SPAT" → enable only these plugins (optional)
- V2XHUB_IT_PROFILES="debug" → start optional profile services (optional)

### `env/stop_v2xhub_integration.sh`
Stops containers and removes volumes for a clean slate.

### `env/apply_integration_seed.sh`
Optional seed step to enable only requested plugins.
Controlled by:
- `V2XHUB_IT_PLUGINS="MAP,SPAT"`

Behavior:
- Waits for DB tables to exist/populate
- Disables all plugins
- Enables only the requested plugin names
- Restarts v2xhub so the enabled set is applied
Notes:
- Plugin names must match what exists in the DB (example: MAP, SPAT)
- This is meant to support future automated tests where we want the environment deterministic.

If empty or `ALL`, it skips.

### `env/docker-compose.it.override.yml`
Integration override file.
Keeps base compose as source of truth and adds integration-specific tweaks.
Also contains optional services controlled by profiles (example: debug container).

### `env/logs.sh`
Convenience script to tail logs:
- all services, or one service only

---

## Prereqs

- Docker
- Docker Compose v2 (`docker compose`)
- curl

---

## Quick start (local)

### Start the environment
Enable MAP + SPAT for this run:

```bash
V2XHUB_IT_PLUGINS="MAP,SPAT" ./tests/integration/env/run_v2xhub_integration.sh
```
### Stop + cleanup
```bash
./tests/integration/env/stop_v2xhub_integration.sh
