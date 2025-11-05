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
  echo "Usage: $0 [BUILD_TYPE] [--j2735-version <version>] [--plugins 'list or all']"
  echo ""
  echo "Optional positional argument:"
  echo "  BUILD_TYPE            The build type (e.g., debug, release, coverage)"
  echo ""
  echo "Optional options:"
  echo "  --j2735-version INT   Specify the J2735 version as an integer (e.g., 2016, 2020, 2024)"
  echo "  --plugins STRING      Specify plugins to build (space-separated, case-sensitive) or 'all' (non-interactive mode)"
  echo ""
  echo "Optional flags:"
  echo "  -h, --help            Show this help message and exit"
  echo ""
  echo "If arguments are not provided, the script will prompt interactively."
}

# Initialize variables
BUILD_TYPE=""
J2735_VERSION=""

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
    --plugins)
      if [[ -n "$2" && "$2" != --* ]]; then
        PLUGIN_INPUT="$2"
        shift 2
      else
        echo "Error: --plugins requires a string value (e.g., 'All' or 'MapPlugin SpatPlugin')"
        exit 1
      fi
      ;;
    -h|--help)
      show_help
      exit 0
      ;;
    --*)
      echo "Unknown option: $1"
      show_help
      exit 1
      ;;
    *)  # Positional argument: BUILD_TYPE
      if [[ -z "$BUILD_TYPE" ]]; then
        BUILD_TYPE="$1"
        shift
      else
        echo "Error: Unexpected extra positional argument: $1"
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
  echo "Enter J2735 version (e.g., 2016, 2020, 2024):"
  read -p "> " J2735_VERSION
fi

if ! [[ "$J2735_VERSION" =~ ^[0-9]+$ ]]; then
  echo "Error: J2735 version must be an integer."
  exit 1
fi

# Set CPP CMake flags and capitalize build type for CMake
if [ "$BUILD_TYPE" = "release" ]; then
    BUILD_TYPE="Release"
    CMAKE_CXX_FLAGS="-DBOOST_BIND_GLOBAL_PLACEHOLDERS"
elif [ "$BUILD_TYPE" = "debug" ]; then
    BUILD_TYPE="Debug"
    CMAKE_CXX_FLAGS="-DBOOST_BIND_GLOBAL_PLACEHOLDERS"
elif [ "$BUILD_TYPE" = "coverage" ]; then
    CMAKE_CXX_FLAGS="-g --coverage -fprofile-arcs -ftest-coverage -DBOOST_BIND_GLOBAL_PLACEHOLDERS"
    BUILD_TYPE="Debug"
else 
    echo "Error: Unsupported BUILD_TYPE ${BUILD_TYPE}. Supported: release, coverage, debug."
    exit 1
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

# Plugin selection: Use --plugins arg if provided, else prompt
if [[ -z "$PLUGIN_INPUT" ]]; then
  echo "Available plugins:"
  for p in "${plugin_dirs[@]}"; do
    echo "  - $p"
  done
  echo ""
  echo "Enter plugins to build (space-separated, case-sensitive), or 'all' to build everything."
  echo "Example: 'MapPlugin SpatPlugin' or 'all' (no quotes needed)"
  read -p "> " PLUGIN_INPUT
fi

# Process plugin input
cmake_flags=""
if [[ "${PLUGIN_INPUT,,}" = "all" ]]; then  # Case-insensitive check (converts to lowercase)
  cmake_flags="$cmake_flags -DBUILD_ALL_PLUGINS=ON"
else
  cmake_flags="$cmake_flags -DBUILD_ALL_PLUGINS=OFF"
  IFS=' ' read -r -a selected_plugins <<< "$PLUGIN_INPUT"
  for plugin in "${selected_plugins[@]}"; do
    if [[ " ${plugin_dirs[*]} " =~ " ${plugin} " ]]; then
      cmake_flags="$cmake_flags -DBUILD_PLUGIN_${plugin}=ON"
    else
      echo "Warning: Invalid plugin '$plugin' - skipping."
    fi
  done
fi

# Run CMake with the new flags
cmake -Bbuild -DCMAKE_PREFIX_PATH=\"/usr/local/share/tmx\;/opt/carma/cmake\;\" -DqserverPedestrian_DIR=/usr/local/share/qserverPedestrian/cmake -Dv2xhubWebAPI_DIR=/usr/local/share/v2xhubWebAPI/cmake/ -DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS}" -DCMAKE_C_FLAGS="${CMAKE_CXX_FLAGS}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" -DSAEJ2735_SPEC_VERSION="${J2735_VERSION}" ${cmake_flags} .
cmake --build build -j "${numCPU}"
cmake --install build
popd