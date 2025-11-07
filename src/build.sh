#!/bin/bash
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

set -e

# script executes all tmx and v2i build and coverage steps so that they can be singularly
# wrapped by the sonarcloud build-wrapper

numCPU=$(nproc)

show_help() {
  echo "Usage: $0 [BUILD_TYPE] [--j2735-version <version>] [--skip-plugins 'list']"
  echo ""
  echo "Required positional arguments:"
  echo "  BUILD_TYPE            The build type (e.g., debug, release, coverage)"
  echo ""
  echo "Required options:"
  echo "  --j2735-version INT   Specify the J2735 version as an integer (e.g., 2016, 2020, 2024)"
  echo ""
  echo "Optional options:"
  echo "  --skip-plugins STRING Space-separated list of plugins to skip (case-sensitive, default empty = build all)"
  echo "  -h, --help            Show this help message and exit"
  echo ""
  echo "If arguments are not provided, the script will prompt interactively."
}

# Initialize variables
BUILD_TYPE=""
J2735_VERSION=""
SKIP_PLUGINS=""

# Parse arguments
while [[ $# -gt 0 ]]; do
  case "$1" in
    --j2735-version)
      if [[ -n "$2" && "$2" != --* ]]; then
        J2735_VERSION="$2"
        shift 2
      else
        echo "Error: --j2735-version requires an integer value"
        exit 1
      fi
      ;;
    --skip-plugins)
      if [[ -n "$2" && "$2" != --* ]]; then
        SKIP_PLUGINS="$2"
        shift 2
      else
        echo "Error: --skip-plugins requires a string value (e.g., 'MapPlugin SpatPlugin')"
        exit 1
      fi
      ;;
    -h|--help)
      show_help
      exit 0
      ;;
    --*)
      echo "Unknown option: $1"
      echo ""
      show_help
      exit 1
      ;;
    *)  # Positional argument: BUILD_TYPE
      if [[ -z "$BUILD_TYPE" ]]; then
        BUILD_TYPE="$1"
        shift
      else
        echo "Error: Unexpected extra positional argument: $1"
        echo ""
        show_help
        exit 1
      fi
      ;;
  esac
done

# Interactive prompts if values are missing
if [[ -z "$BUILD_TYPE" ]]; then
  echo "Enter build type (debug, release, coverage):"
  read -p "> " BUILD_TYPE
fi

if [[ -z "$J2735_VERSION" ]]; then
  echo "Enter --j2735-version (e.g., 2016, 2020, 2024):"
  read -p "> " J2735_VERSION
fi

if ! [[ "$J2735_VERSION" =~ ^[0-9]+$ ]]; then
  echo "Error: J2735 version must be an integer."
  exit 1
fi

# Interactive prompt for SKIP_PLUGINS if missing (and interactive terminal)
if [[ -z "$SKIP_PLUGINS" && -t 0 ]]; then
  echo "Enter plugins to skip (space-separated, case-sensitive) or press Enter for none (build all):"
  read -p "> " SKIP_PLUGINS
fi

# Output results
echo "Build Type: $BUILD_TYPE"
echo "J2735 Version: $J2735_VERSION"

pushd tmx
cmake -Bbuild -DCMAKE_PREFIX_PATH=\"/usr/local/share/tmx\;/opt/carma/cmake\;\" -DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS}" -DCMAKE_C_FLAGS="${CMAKE_CXX_FLAGS}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" -DSAEJ2735_SPEC_VERSION="${J2735_VERSION}" .
cmake --build build -j "${numCPU}"
cmake --install build
popd

pushd v2i-hub

# Dynamically list plugin directories (those with CMakeLists.txt, excluding main dir)
plugin_dirs=()
for f in */CMakeLists.txt; do
  dir=$(dirname "$f")
  if [ "$dir" != "." ] && [ "$dir" != "build" ]; then
    plugin_dirs+=("$dir")
  fi
done

# Process skip plugins input
cmake_flags=""
if [[ -n "$SKIP_PLUGINS" ]]; then
  IFS=' ' read -r -a skipped_plugins <<< "$SKIP_PLUGINS"
  for plugin in "${skipped_plugins[@]}"; do
    if [[ " ${plugin_dirs[*]} " =~ " ${plugin} " ]]; then
      cmake_flags="$cmake_flags -DSKIP_${plugin}=ON"
    else
      echo "Warning: Invalid plugin to skip '$plugin' - ignoring."
    fi
  done
  echo "Skipping specified plugins: ${skipped_plugins[*]}"
else
  echo "No plugins skipped (building all)"
fi

# Run CMake with the new flags
cmake -Bbuild -DCMAKE_PREFIX_PATH=\"/usr/local/share/tmx\;/opt/carma/cmake\;\" -DqserverPedestrian_DIR=/usr/local/share/qserverPedestrian/cmake -Dv2xhubWebAPI_DIR=/usr/local/share/v2xhubWebAPI/cmake/ -DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS}" -DCMAKE_C_FLAGS="${CMAKE_CXX_FLAGS}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" -DSAEJ2735_SPEC_VERSION="${J2735_VERSION}" ${cmake_flags} .
cmake --build build -j "${numCPU}"
cmake --install build
popd