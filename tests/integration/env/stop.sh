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

dc down -v --remove-orphans
