# V2X-Hub Integration Test Environment

This folder provides a repeatable **integration test environment** for V2X-Hub using Docker Compose.

It is meant to be used:
- locally (developer machine)
- in CI later
- environment only (start/stop + optional plugin selection)

---

## What this gives you

- One command to start the V2X-Hub stack for integration testing
- One command to stop/cleanup the stack
- Enable selected plugins (ex: MAP + SPAT) via a separate script.
- Optional extra containers using Compose profiles (example: kafka). They’re usually runtime dependency containers that V2X-Hub might need for certain plugin test cases.

---

## Files

### `env/.env.it`
Dedicated env file for integration testing. This file is used by:
- the run/stop/plugin-selection scripts (via `source`)
- `docker compose` (via `--env-file`)

Key settings:
- `V2XHUB_IT_PLUGINS=MAP,SPAT` → enable only these plugins (optional)
- `COMPOSE_PROFILES=kafka` → start optional profile services (optional)

### `env/run.sh`
Starts the integration stack using:
- `configuration/docker-compose.yml` (base)
- `tests/integration/env/docker-compose.it.override.yml` (integration override)

Then:
- waits until UI is reachable: `https://127.0.0.1/admin/admin.html`
- runs `plugin_selection.sh` if present (to enable only requested plugins)

### `env/stop.sh`
Stops containers and removes volumes for a clean slate.

### `env/plugin_selection.sh`
Optional seed step to enable only requested plugins.

Behavior:
- waits for DB + plugin tables to be populated
- verifies requested plugin tokens exist
- disables all plugins
- enables only requested plugins
- restarts `v2xhub` so the enabled set is applied

If `V2XHUB_IT_PLUGINS` is empty or `ALL`, it skips.

### `env/docker-compose.it.override.yml`
Integration override file.
Keeps base compose as source of truth and adds integration-specific tweaks.
Also contains optional services controlled by profiles (example: kafka, debug container).

---

## Prereqs

- Docker
- Docker Compose v2
  - Verify: `docker compose version`
  - Recommended: v2.20+ (any v2 should work)
- curl

---

## Quick start (local)

### Start the environment
```bash
./tests/integration/env/run.sh