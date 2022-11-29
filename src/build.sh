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

# script executes all tmx and v2i build and coverage steps so that they can be singularly
# wrapped by the sonarcloud build-wrapper
set -e

COVERAGE_FLAGS="-g --coverage -fprofile-arcs -ftest-coverage"

cd /home/V2X-Hub/src/tmx/TmxUtils
mkdir build
cd /home/V2X-Hub/src/tmx/TmxUtils/build
cmake -DCMAKE_CXX_FLAGS="${COVERAGE_FLAGS}" -DCMAKE_C_FLAGS="${COVERAGE_FLAGS}" -DCMAKE_BUILD_TYPE="Debug" ..
make

cd /home/V2X-Hub/src/v2i-hub/PedestrianPlugin
mkdir build
cd /home/V2X-Hub/src/v2i-hub/PedestrianPlugin
cmake -DCMAKE_CXX_FLAGS="${COVERAGE_FLAGS}" -DCMAKE_C_FLAGS="${COVERAGE_FLAGS}" -DCMAKE_BUILD_TYPE="Debug" ..
make

cd /home/V2X-Hub/src/v2i-hub/PedestrianPlugin
mkdir build
cd /home/V2X-Hub/src/v2i-hub/PedestrianPlugin
cmake -DCMAKE_CXX_FLAGS="${COVERAGE_FLAGS}" -DCMAKE_C_FLAGS="${COVERAGE_FLAGS}" -DCMAKE_BUILD_TYPE="Debug" ..
make

cd /home/V2X-Hub/src/v2i-hub/PreemptionPlugin
mkdir build
cd /home/V2X-Hub/src/v2i-hub/PreemptionPlugin
cmake -DCMAKE_CXX_FLAGS="${COVERAGE_FLAGS}" -DCMAKE_C_FLAGS="${COVERAGE_FLAGS}" -DCMAKE_BUILD_TYPE="Debug" ..
make

cd /home/V2X-Hub/src/v2i-hub/SpatPlugin
mkdir build
cd /home/V2X-Hub/src/v2i-hub/SpatPlugin
cmake -DCMAKE_CXX_FLAGS="${COVERAGE_FLAGS}" -DCMAKE_C_FLAGS="${COVERAGE_FLAGS}" -DCMAKE_BUILD_TYPE="Debug" ..
make
