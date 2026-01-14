#!/usr/bin/env python3
import argparse
import subprocess
from pathlib import Path

def load_env(env_path: Path) -> dict:
    env = {}
    if not env_path.exists():
        return env
    for line in env_path.read_text().splitlines():
        line = line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        k, v = line.split("=", 1)
        v = v.strip().strip("\r").strip('"').strip("'")
        env[k.strip()] = v
    return env

def sh(cmd: list[str]) -> None:
    subprocess.run(cmd, check=True)

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--payload", required=True)
    ap.add_argument("--service", default="v2xhub")
    ap.add_argument("--dest", required=True)
    ap.add_argument("--restart", action="store_true")
    args = ap.parse_args()

    root = Path(__file__).resolve().parents[3]  # repo root
    env_path = root / "tests/integration/env/.env.it"
    env = load_env(env_path)

    project = env.get("PROJECT", "v2xhub_it")
    base_compose = env.get("BASE_COMPOSE", "configuration/docker-compose.yml")
    override_compose = env.get("OVERRIDE_COMPOSE", "tests/integration/env/docker-compose.it.override.yml")
    proj_dir = root / Path(base_compose).parent  # configuration/

    payload = (root / args.payload).resolve()
    if not Path(args.payload).exists():
        raise SystemExit(f"payload not found: {args.payload}")


    dest_dir = str(Path(args.dest).parent)

    dc = [
        "docker", "compose",
        "--project-directory", str(proj_dir),
        "-p", project,
        "-f", str(root / base_compose),
        "-f", str(root / override_compose),
        "--env-file", str(env_path),
    ]

    # make sure dest folder exists
    sh(dc + ["exec", "-T", "-u", "root", args.service, "mkdir", "-p", dest_dir])

    # copy file
    sh(dc + ["cp", str(payload), f"{args.service}:{args.dest}"])
    
    if args.restart:
        sh(dc + ["restart", args.service])

    # quick verify
    sh(dc + ["exec", "-T", args.service, "ls", "-lah", args.dest])
    print("[inject] done")

if __name__ == "__main__":
    main()
