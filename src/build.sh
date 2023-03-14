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

RELEASE_BUILD=0
if [ "$1" = "release" ]; then
    RELEASE_BUILD=1
fi
DEBUG_BUILD=0
if [ "$1" = "debug" ]; then
    DEBUG_BUILD=1
fi

if [ $DEBUG_BUILD -eq 1 ]; then
    BUILD_TYPE="Debug"
elif [ $RELEASE_BUILD -eq 0 ]; then
    COVERAGE_FLAGS="-g --coverage -fprofile-arcs -ftest-coverage"
    BUILD_TYPE="Debug"
fi

pushd tmx
cmake -Bbuild -DCMAKE_PREFIX_PATH=/usr/local/share/tmx\;\/opt/carma/cmake\;\ -DCMAKE_CXX_FLAGS="${COVERAGE_FLAGS}" -DCMAKE_C_FLAGS="${COVERAGE_FLAGS}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" .
pushd build
make
make install
popd
popd

pushd v2i-hub
cmake -Bbuild  -DCMAKE_PREFIX_PATH=/usr/local/share/tmx\;\/opt/carma/cmake\;\ -DqserverPedestrian_DIR=/usr/local/share/qserverPedestrian/cmake -Dv2xhubWebAPI_DIR=/usr/local/share/v2xhubWebAPI/cmake/ -DCMAKE_CXX_FLAGS="${COVERAGE_FLAGS}" -DCMAKE_C_FLAGS="${COVERAGE_FLAGS}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" .
pushd build
make install
popd
popd
