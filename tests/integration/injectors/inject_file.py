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
Copy a local payload file into a running V2X-Hub integration environment.

Use this for plugins that read inputs from a configured file path (example: MAP).
Optionally restart the target service or toggle a plugin to force it to reload the file.
"""

import argparse
import subprocess
import sys
from pathlib import Path

# Allow importing sibling _common.py when running as a standalone script.
sys.path.insert(0, str(Path(__file__).resolve().parent))

from _common import find_repo_root, load_env, sh

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--payload", required=True, help="Local payload file to copy into the running integration environment.")
    ap.add_argument("--service", default="v2xhub", help="Docker Compose service name to copy into (default: v2xhub).")
    ap.add_argument("--dest", required=True, help="Destination path inside the container. This should match the plugin config FilePath.")
    ap.add_argument("--restart", action="store_true", help="Restart the target service after copying the file. Use when a plugin loads the file only at startup.")
    ap.add_argument("--toggle-plugin", help="Disable and re-enable a plugin after copying the file (example: MAP).")
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

    dest_dir = str(Path(args.dest).parent)

    dc = [
        "docker", "compose",
        "--project-directory", str(proj_dir),
        "-p", project,
        "-f", str(repo / base_compose),
        "-f", str(repo / override_compose),
        "--env-file", str(env_path),
    ]

    # make sure dest folder exists
    sh(dc + ["exec", "-T", "-u", "root", args.service, "mkdir", "-p", dest_dir])

    # copy file
    sh(dc + ["cp", str(payload), f"{args.service}:{args.dest}"])

    if args.toggle_plugin:
        sh([
            "python3",
            str(repo / "tests/integration/injectors/toggle_plugin.py"),
            "--plugin", args.toggle_plugin,
            "--env-file", args.env_file,
            "--wait", "3.0"
       ])

    if args.restart:
        sh(dc + ["restart", args.service])

    # quick verify
    sh(dc + ["exec", "-T", args.service, "ls", "-lah", args.dest])
    print("[inject] done")

if __name__ == "__main__":
    main()
