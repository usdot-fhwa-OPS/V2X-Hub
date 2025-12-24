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
  echo "  --skip-plugins STRING Space-separated list of plugin directory names to skip  (case-sensitive, default empty = build all)"
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
      if [[ $# -gt 1 ]]; then
        SKIP_PLUGINS="$2"
        shift 2
      else
        SKIP_PLUGINS=""
        shift 1
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


if ! [[ "$J2735_VERSION" =~ ^[0-9]+$ ]]; then
  echo "Error: --j2735-version must be an integer."
  exit 1
fi

# Set CPP CMake flags and capitalize build type for CMake
if [ "$BUILD_TYPE" = "release" ]; then
    BUILD_TYPE="Release"
    # -DBOOST_BIND_GLOBAL_PLACEHOLDERS : Flag to enable global placeholders for boost::bind and avoid deprecation warnings
    # -O3 : Flag to enable optimization levels for release builds (https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html)
    CMAKE_CXX_FLAGS="-DBOOST_BIND_GLOBAL_PLACEHOLDERS -O3 "
elif [ "$BUILD_TYPE" = "debug" ]; then
    BUILD_TYPE="Debug"
    # -DBOOST_BIND_GLOBAL_PLACEHOLDERS : Flag to enable global placeholders for boost::bind and avoid deprecation warnings
    # -fsanitize=address : Instruments code at compile time to detect memory access errors
    # -Og : Enables all -O1 optimizations except those known to greatly interfere with debugging (https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html)
    # -g : Includes debugging symbols in output binary for use with debugger.
    # -Wall : Enables all the warnings about bad C++ practices (https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html)
    # -Wextra : Enables extra warnings flags that are not enabled by -Wall (https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html)
    CMAKE_CXX_FLAGS="-DBOOST_BIND_GLOBAL_PLACEHOLDERS -fsanitize=address -Og -g -Wall -Wextra "
elif [ "$BUILD_TYPE" = "coverage" ]; then
    # Coverage flags plus flag to enable global placeholders for boost::bind and avoid 
    # deprecation warnings
    CMAKE_CXX_FLAGS="-g --coverage -fprofile-arcs -ftest-coverage -DBOOST_BIND_GLOBAL_PLACEHOLDERS"
    BUILD_TYPE="Debug"
else 
    echo "Error: Unsupported BUILD_TYPE ${BUILD_TYPE}. Supported BUILD_TYPE : release, coverage, debug."
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
