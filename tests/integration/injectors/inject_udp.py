#!/usr/bin/env python3
#  Copyright (C) 2018-2026 LEIDOS.
# 
#  Licensed under the Apache License, Version 2.0 (the "License"); you may not
#  use this file except in compliance with the License. You may obtain a copy of
#  the License at
# 
#  http://www.apache.org/licenses/LICENSE-2.0
# 
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#  License for the specific language governing permissions and limitations under
#  the License.

"""
Send a binary payload over UDP to a running V2X-Hub integration environment.

Use this for plugins that listen on a UDP port for inputs (example: SPAT, MessageReceiver).
This script only sends bytes; validation is handled in later integration test stories.
"""
import argparse
import time
import sys
from pathlib import Path

# Allow importing sibling _common.py when running as a standalone script.
sys.path.insert(0, str(Path(__file__).resolve().parent))

from _common import find_repo_root, load_env, sh

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--payload", required=True, help="Local payload file to send over UDP.")
    ap.add_argument("--port", type=int, required=True, help="Destination UDP port (plugin port).")
    ap.add_argument("--host", default="127.0.0.1", help="Destination host (default: 127.0.0.1).")
    ap.add_argument("--service", default="v2xhub", help="Docker Compose service to execute UDP send from (default: v2xhub).")
    ap.add_argument("--repeat", type=int, default=1, help="How many times to send the payload (default: 1).")
    ap.add_argument("--delay", type=float, default=0.0, help="Delay between repeats in seconds (default: 0.0).")
    ap.add_argument("--env-file", default="tests/integration/env/.env.it", help="Path to integration env file (default: tests/integration/env/.env.it).")
    args = ap.parse_args()

    root = Path(__file__).resolve()
    repo = find_repo_root(root)
    env_path = (repo / args.env_file).resolve()


    env = load_env(env_path)

    project = env.get("PROJECT", "v2xhub_it")
    base_compose = env.get("BASE_COMPOSE", "configuration/docker-compose.yml")
    override_compose = env.get("OVERRIDE_COMPOSE", "tests/integration/env/docker-compose.it.override.yml")
    proj_dir = repo / Path(base_compose).parent

    payload = (repo / args.payload).resolve()
    if not payload.exists():
        raise SystemExit(f"payload not found: {args.payload}")

    dc = [
        "docker", "compose",
        "--project-directory", str(proj_dir),
        "-p", project,
        "-f", str(repo / base_compose),
        "-f", str(repo / override_compose),
        "--env-file", str(env_path),
    ]

    tmp = "/tmp/inject_udp.bin"
    sh(dc + ["cp", str(payload), f"{args.service}:{tmp}"])

    for i in range(args.repeat):
        sh(dc + ["exec", "-T", args.service, "bash", "-lc", f"cat {tmp} > /dev/udp/{args.host}/{args.port}"])
        if args.delay > 0 and i < args.repeat - 1:
            time.sleep(args.delay)

    print("[inject] done")

if __name__ == "__main__":
    main()
