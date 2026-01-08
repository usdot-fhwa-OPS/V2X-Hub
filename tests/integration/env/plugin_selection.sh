#!/usr/bin/env bash
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


PLUGINS="${V2XHUB_IT_PLUGINS:-}"
if [[ -z "$PLUGINS" || "$PLUGINS" == "ALL" ]]; then
  echo "[seed] V2XHUB_IT_PLUGINS is empty or ALL -> skipping plugin selection"
  exit 0
fi

read_env() {
  local key="$1"
  local val
  val="$(grep -E "^${key}=" "$ENV_FILE" | head -n1 | cut -d= -f2- | tr -d '\r')"
  val="${val%\"}"; val="${val#\"}"
  val="${val%\'}"; val="${val#\'}"
  echo "$val"
}

MYSQL_DATABASE="$(read_env MYSQL_DATABASE)"; MYSQL_DATABASE="${MYSQL_DATABASE:-IVP}"
MYSQL_USER="$(read_env MYSQL_USER)"; MYSQL_USER="${MYSQL_USER:-IVP}"
MYSQL_PASSWORD="$(read_env MYSQL_PASSWORD)"

if [[ -z "$MYSQL_PASSWORD" ]]; then
  echo "[seed] ERROR: MYSQL_PASSWORD is empty in $ENV_FILE"
  exit 1
fi

mysql_exec() {
  local sql="$1"
  dc exec -T -e MYSQL_PWD="$MYSQL_PASSWORD" db \
    mysql -u"$MYSQL_USER" "$MYSQL_DATABASE" -e "$sql"
}

mysql_scalar() {
  local sql="$1"
  dc exec -T -e MYSQL_PWD="$MYSQL_PASSWORD" db \
    mysql -u"$MYSQL_USER" "$MYSQL_DATABASE" -N -B -e "$sql" 2>/dev/null | tr -d '\r' || true
}

echo "[seed] Waiting for DB..."
ready=0
for _ in {1..120}; do
  if mysql_exec "SELECT 1;" >/dev/null 2>&1; then
    ready=1
    break
  fi
  sleep 1
done

if [[ "$ready" != "1" ]]; then
  echo "[seed] ERROR: DB never became ready."
  echo "[seed] Last mysql logs:"
  dc logs --tail=80 db || true
  exit 1
fi

echo "[seed] Waiting for plugin table to be populated..."
for _ in {1..180}; do
  c="$(mysql_scalar "SELECT COUNT(*) FROM plugin;")"
  [[ "${c:-0}" =~ ^[0-9]+$ ]] && [[ "${c:-0}" -gt 0 ]] && break
  sleep 1
done

echo "[seed] Waiting for installedPlugin table to be populated..."
for _ in {1..180}; do
  c="$(mysql_scalar "SELECT COUNT(*) FROM installedPlugin;")"
  [[ "${c:-0}" =~ ^[0-9]+$ ]] && [[ "${c:-0}" -gt 0 ]] && break
  sleep 1
done

echo "[seed] Checking requested plugins exist..."
IFS=',' read -ra ARR <<< "$PLUGINS"
for tok in "${ARR[@]}"; do
  tok="$(echo "$tok" | xargs)"
  [[ -z "$tok" ]] && continue
  tok_esc="$(printf "%s" "$tok" | sed "s/'/''/g")"
  ok="$(mysql_scalar "
    SELECT EXISTS(
      SELECT 1
      FROM installedPlugin ip
      JOIN plugin p ON p.id = ip.pluginId
      WHERE LOWER(p.name) LIKE LOWER('%${tok_esc}%')
    );
  ")"
  if [[ "${ok:-0}" != "1" ]]; then
    echo "[seed] ERROR: plugin token not found in installedPlugin: $tok"
    exit 1
  fi
done

echo "[seed] Disabling all plugins..."
mysql_exec "UPDATE installedPlugin SET enabled=0;"

echo "[seed] Enabling only: $PLUGINS"
IFS=',' read -ra ARR <<< "$PLUGINS"
for tok in "${ARR[@]}"; do
  tok="$(echo "$tok" | xargs)"
  [[ -z "$tok" ]] && continue
  mysql_exec "
    UPDATE installedPlugin ip
    JOIN plugin p ON p.id = ip.pluginId
    SET ip.enabled=1
    WHERE LOWER(p.name) LIKE LOWER('%${tok}%');
  "
done

echo "[seed] Enabled plugins now:"
mysql_exec "
  SELECT p.name, ip.enabled
  FROM installedPlugin ip
  JOIN plugin p ON p.id = ip.pluginId
  WHERE ip.enabled=1
  ORDER BY p.name;
"

echo "[seed] Restarting v2xhub to apply enabled set..."
dc restart v2xhub >/dev/null

echo "[seed] Done"
