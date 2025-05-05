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
  echo "Usage: $0 <BUILD_TYPE> --j2735-version <version>"
  echo ""
  echo "Required positional arguments:"
  echo "  BUILD_TYPE            The build type (e.g., debug, release, coverage)"
  echo ""
  echo "Required options:"
  echo "  --j2735-version INT   Specify the J2735 version as an integer (e.g., 2016, 2020, 2025)"
  echo ""
  echo "Optional flags:"
  echo "  -h, --help            Show this help message and exit"
}

# Default values
J2735_VERSION="2016"

# Initialize variables
BUILD_TYPE="debug"

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
    -h|--help)
      show_help
      exit 0
      ;;
    --*)
      echo "Unknown option: $1"
      exit 1
      ;;
    *)  # Assume positional BUILD_TYPE
      if [[ -z "$BUILD_TYPE" ]]; then
        BUILD_TYPE="$1"
      else
        echo "Error: Unexpected extra positional argument: $1"
        exit 1
      fi
      shift
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
    # Flag to enable global placeholders for boost::bind and avoid deprecation warnings
    CMAKE_CXX_FLAGS="-DBOOST_BIND_GLOBAL_PLACEHOLDERS"
elif [ "$BUILD_TYPE" = "debug" ]; then
    BUILD_TYPE="Debug"
    # Flag to enable global placeholders for boost::bind and avoid deprecation warnings
    CMAKE_CXX_FLAGS="-DBOOST_BIND_GLOBAL_PLACEHOLDERS"
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
cmake -Bbuild -DCMAKE_PREFIX_PATH=\"/usr/local/share/tmx\;/opt/carma/cmake\;\" -DqserverPedestrian_DIR=/usr/local/share/qserverPedestrian/cmake -Dv2xhubWebAPI_DIR=/usr/local/share/v2xhubWebAPI/cmake/ -DCMAKE_CXX_FLAGS="${CMAKE_CXX_FLAGS}" -DCMAKE_C_FLAGS="${CMAKE_CXX_FLAGS}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" -DSAEJ2735_SPEC_VERSION="${J2735_VERSION}" .
cmake --build build -j "${numCPU}"
cmake --install build
popd
