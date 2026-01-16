import subprocess
from pathlib import Path


def find_repo_root(start: Path) -> Path:
    """
    Find the repo root directory starting from a file path.

    We treat the repo root as the first parent directory (including itself)
    that contains a "tests" folder. If not found, fall back to a fixed parent
    depth to preserve current behavior.
    """
    start = start.resolve()
    return next(
        (p for p in [start, *start.parents] if (p / "tests").exists()),
        start.parents[3],
    )


def load_env(env_path: Path) -> dict:
    """
    Load a simple KEY=VALUE env file into a dict.

    - Ignores blank lines and comments (# ...)
    - Strips quotes and Windows CRs
    - Does not do variable expansion (keeps it simple/portable)
    """
    env = {}
    if not env_path.exists():
        return env

    for line in env_path.read_text().splitlines():
        line = line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue

        k, v = line.split("=", 1)
        env[k.strip()] = v.strip().strip("\r").strip('"').strip("'")

    return env


def sh(cmd: list[str]) -> None:
    """
    Run a shell command and fail fast if it returns non-zero.
    """
    subprocess.run(cmd, check=True)
