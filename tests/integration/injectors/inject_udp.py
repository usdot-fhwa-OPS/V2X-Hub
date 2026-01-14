#!/usr/bin/env python3
import argparse, subprocess, time
from pathlib import Path

def env(k, d):
    p = Path("tests/integration/env/.env.it")
    if not p.exists(): return d
    for line in p.read_text().splitlines():
        line = line.strip()
        if line.startswith(k + "="):
            return line.split("=", 1)[1].strip().strip('"').strip("'")
    return d

def sh(cmd): subprocess.run(cmd, check=True)

def main():
    a = argparse.ArgumentParser()
    a.add_argument("--payload", required=True)
    a.add_argument("--port", type=int, required=True)
    a.add_argument("--host", default="127.0.0.1")
    a.add_argument("--service", default="v2xhub")
    a.add_argument("--repeat", type=int, default=1)
    a.add_argument("--delay", type=float, default=0.0)
    args = a.parse_args()

    if not Path(args.payload).exists():
        raise SystemExit(f"payload not found: {args.payload}")

    project = env("PROJECT", "v2xhub_it")
    base = env("BASE_COMPOSE", "configuration/docker-compose.yml")
    over = env("OVERRIDE_COMPOSE", "tests/integration/env/docker-compose.it.override.yml")
    envf = "tests/integration/env/.env.it"
    projdir = str(Path(base).parent)

    dc = ["docker", "compose", "--project-directory", projdir, "-p", project, "-f", base, "-f", over, "--env-file", envf]
    tmp = "/tmp/inject_udp.bin"

    sh(dc + ["cp", args.payload, f"{args.service}:{tmp}"])
    for i in range(args.repeat):
        sh(dc + ["exec", "-T", args.service, "bash", "-lc", f"cat {tmp} > /dev/udp/{args.host}/{args.port}"])
        if args.delay > 0 and i < args.repeat - 1:
            time.sleep(args.delay)

    print("done")

if __name__ == "__main__":
    main()
