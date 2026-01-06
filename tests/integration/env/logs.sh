#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
cd "$ROOT_DIR"

SERVICE="${1:-}"

if [ -n "$SERVICE" ]; then
  docker compose -p v2xhub_it \
    -f configuration/docker-compose.yml \
    -f tests/integration/env/docker-compose.it.override.yml \
    --env-file configuration/.env \
    logs -f --tail=200 "$SERVICE"
else
  docker compose -p v2xhub_it \
    -f configuration/docker-compose.yml \
    -f tests/integration/env/docker-compose.it.override.yml \
    --env-file configuration/.env \
    logs -f --tail=200
fi


#usage:

# ./tests/integration/env/logs.sh php
# ./tests/integration/env/logs.sh v2xhub
# ./tests/integration/env/logs.sh mysql