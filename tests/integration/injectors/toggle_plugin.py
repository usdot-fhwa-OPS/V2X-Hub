#  Copyright (C) 2018-2020 LEIDOS.
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

#!/usr/bin/env python3
"""
Disable and re-enable a V2X-Hub plugin in the integration environment.

This is a simple way to force a plugin to reload configuration or file inputs
without restarting the full container stack.
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
    ap.add_argument("--plugin", required=True, help="Plugin name or token (example: MAP).")
    ap.add_argument("--wait", type=float, default=2.0, help="Seconds to wait between disable and enable (default: 2.0).")
    ap.add_argument("--env-file", default="tests/integration/env/.env.it", help="Path to integration env file.")
    args = ap.parse_args()

    root = Path(__file__).resolve()
    repo = find_repo_root(root)
    env_path = (repo / args.env_file).resolve()
    env = load_env(env_path)

    project = env.get("PROJECT", "v2xhub_it")
    base_compose = env.get("BASE_COMPOSE", "configuration/docker-compose.yml")
    override_compose = env.get("OVERRIDE_COMPOSE", "tests/integration/env/docker-compose.it.override.yml")
    proj_dir = repo / Path(base_compose).parent

    mysql_pwd = env.get("MYSQL_PASSWORD", "")
    if not mysql_pwd:
        raise SystemExit(f"MYSQL_PASSWORD not found in {env_path}")

    mysql_user = env.get("MYSQL_USER", "IVP")
    mysql_db = env.get("MYSQL_DATABASE", "IVP")

    tok = args.plugin.strip().replace("'", "''")

    dc = [
        "docker", "compose",
        "--project-directory", str(proj_dir),
        "-p", project,
        "-f", str(repo / base_compose),
        "-f", str(repo / override_compose),
        "--env-file", str(env_path),
    ]

    disable_sql = f"""
        UPDATE installedPlugin ip
        JOIN plugin p ON p.id = ip.pluginId
        SET ip.enabled = 0
        WHERE p.name = '{tok}' OR p.name LIKE '%{tok}%';
        SELECT ROW_COUNT() AS rows_disabled;
    """

    enable_sql = f"""
        UPDATE installedPlugin ip
        JOIN plugin p ON p.id = ip.pluginId
        SET ip.enabled = 1
        WHERE p.name = '{tok}' OR p.name LIKE '%{tok}%';
        SELECT ROW_COUNT() AS rows_enabled;
    """

    sh(dc + ["exec", "-T", "-e", f"MYSQL_PWD={mysql_pwd}", "db",
             "mysql", f"-u{mysql_user}", mysql_db, "-e", disable_sql])

    time.sleep(args.wait)

    sh(dc + ["exec", "-T", "-e", f"MYSQL_PWD={mysql_pwd}", "db",
             "mysql", f"-u{mysql_user}", mysql_db, "-e", enable_sql])

    print(f"[toggle] toggled plugin: {args.plugin} (wait={args.wait}s)")


if __name__ == "__main__":
    main()
