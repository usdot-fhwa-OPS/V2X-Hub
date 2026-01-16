#!/usr/bin/env bash
"""
# Start the V2X-Hub Docker Compose stack for integration testing.
# Waits for the UI to be reachable and then applies optional plugin selection.
"""
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
cd "$ROOT_DIR"

IT_ENV="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/.env.it"
if [[ -f "$IT_ENV" ]]; then
  source "$IT_ENV"
fi

ENV_FILE="$IT_ENV"

dc() {
  local proj_dir="$ROOT_DIR/$(dirname "$BASE_COMPOSE")"
  docker compose --project-directory "$proj_dir" \
    -p "$PROJECT" \
    -f "$ROOT_DIR/$BASE_COMPOSE" \
    -f "$ROOT_DIR/$OVERRIDE_COMPOSE" \
    --env-file "$ENV_FILE" "$@"
}

dc up -d --remove-orphans

echo ""
echo "[integration] COMPOSE_PROFILES=${COMPOSE_PROFILES:-<none>}"
dc ps

# Ready check (use the UI page that is actually reachable from host)
READY_URL="https://127.0.0.1/admin/admin.html"
echo ""
echo "[integration] Waiting for UI to be reachable..."
for i in {1..120}; do
  if curl -kfsS "$READY_URL" >/dev/null 2>&1; then
    echo "[integration] UI is reachable: $READY_URL"
    break
  fi
  sleep 1
done

# If still not reachable, fail fast
if ! curl -kfsS "$READY_URL" >/dev/null 2>&1; then
  echo "[integration] ERROR: UI not reachable after waiting: $READY_URL"
  exit 1
fi

# Plugin selection (Step 4) — run if script exists
SEED_SCRIPT="$ROOT_DIR/tests/integration/env/plugin_selection.sh"
echo ""
if [[ -f "$SEED_SCRIPT" ]]; then
  echo "[integration] Applying plugin selection (V2XHUB_IT_PLUGINS=${V2XHUB_IT_PLUGINS:-<none>})"
  bash "$SEED_SCRIPT"
else
  echo "[integration] plugin_selection.sh not found (ok)"
fi

echo ""
echo "[integration] UI:"
echo "  https://127.0.0.1:19760/   (accept cert warning)"
echo "  https://127.0.0.1/admin/admin.html"
