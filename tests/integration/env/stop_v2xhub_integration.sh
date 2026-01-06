#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
cd "$ROOT_DIR"

docker compose -p v2xhub_it \
  -f configuration/docker-compose.yml \
  -f tests/integration/env/docker-compose.it.override.yml \
  --env-file configuration/.env \
  down -v --remove-orphans
