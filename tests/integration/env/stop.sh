#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
cd "$ROOT_DIR"

IT_ENV="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/.env.it"
if [[ -f "$IT_ENV" ]]; then
  source "$IT_ENV"
fi

docker compose -p "$PROJECT" \
  -f "$BASE_COMPOSE" \
  -f "$OVERRIDE_COMPOSE" \
  --env-file "$ENV_FILE" \
  down -v --remove-orphans
