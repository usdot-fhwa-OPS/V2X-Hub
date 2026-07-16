#!/bin/bash
# This script installs the dependencies required to run the mock-srm.py script. 

set -e

# Dependencies
dependencies="python3"

# Install preliminary dependencies
sudo apt-get update
sudo apt-get install -y $dependencies

# Declare local variables for paths and commands
REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
VENV_DIR=${VENV_DIR:-"$REPO_ROOT/.venv"}
SYSTEM_PYTHON=${PYTHON:-python3}

# Create virtual environment if it doesn't exist
if [ ! -x "$VENV_DIR/bin/python" ]; then
  echo "Creating virtual environment at $VENV_DIR..."
  "$SYSTEM_PYTHON" -m venv "$VENV_DIR"
fi

# Activate virtual environment
# shellcheck disable=SC1091
source "$VENV_DIR/bin/activate"

# Upgrade pip in the venv
PYTHON_BIN="$VENV_DIR/bin/python"
"$PYTHON_BIN" -m pip install -U pip >/dev/null 2>&1 || true

# Ensure j2735_202409 package is present; clone/install only if missing
echo "Checking for Python module 'j2735_202409'…"
if ! "$PYTHON_BIN" -c "import importlib; importlib.import_module('j2735_202409')" >/dev/null 2>&1; then
  echo "'j2735_202409' not found; attempting to clone and install…"
  J2735_REPO_URL=${J2735_REPO_URL:-https://github.com/usdot-fhwa-stol/j2735_202409.git}
  git clone "$J2735_REPO_URL" j2735_202409
  pushd j2735_202409 >/dev/null
  "$PYTHON_BIN" -m pip install -U pip >/dev/null 2>&1 || true
  "$PYTHON_BIN" -m pip install dist/j2735_202409-0.1.0-py3-none-any.whl
  popd >/dev/null
  rm -rf j2735_202409

  # Verify import after install
  if "$PYTHON_BIN" -c "import importlib; importlib.import_module('j2735_202409')" >/dev/null 2>&1; then
    echo "Installed and verified 'j2735_202409'."
  else
    echo "Warning: 'j2735_202409' still not importable after install attempt." >&2
  fi
else
  echo "'j2735_202409' already available; skipping install."
fi

# Install other Python dependencies from requirements.txt
REQ_FILE="$REPO_ROOT/scripts/requirements.txt"
if [[ -f "$REQ_FILE" ]]; then
  "$PYTHON_BIN" -m pip install -r "$REQ_FILE"
else
  echo "Warning: requirements.txt not found at $REQ_FILE; skipping Python deps install." >&2
fi

echo "Virtual environment ready at $VENV_DIR"
echo "Activate with: source $VENV_DIR/bin/activate"
