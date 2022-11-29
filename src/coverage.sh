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

# script to run tests, generate test-coverage, and store coverage reports in a place
# easily accessible to sonar. Test names should follow convention run<pluginName>Tests

cd /home/V2X-Hub
mkdir test_results

cd /home/V2X-Hub/src/tmx/TmxUtils/build/
./TmxUtils_test --gtest_output=xml:../../../../test_results/
cd /home/V2X-Hub/src/tmx/TmxUtils/
mkdir coverage
cd /home/V2X-Hub/src/tmx/
gcovr --sonarqube TmxUtils/coverage/coverage.xml -s -f TmxUtils/ -r .

cd /home/V2X-Hub/src/v2i-hub/PedestrianPlugin/build/
./PedestrianPlugin_test --gtest_output=xml:../../../../test_results/
cd /home/V2X-Hub/src/v2i-hub/PedestrianPlugin/
mkdir coverage
cd /home/V2X-Hub/src/v2i-hub/
gcovr --sonarqube PedestrianPlugin/coverage/coverage.xml -s -f PedestrianPlugin/ -r .

cd /home/V2X-Hub/src/v2i-hub/PreemptionPlugin/build/
./PreemptionPlugin_test --gtest_output=xml:../../../../test_results/
cd /home/V2X-Hub/src/v2i-hub/PreemptionPlugin/
mkdir coverage
cd /home/V2X-Hub/src/v2i-hub/
gcovr --sonarqube PreemptionPlugin/coverage/coverage.xml -s -f PreemptionPlugin/ -r .

cd /home/V2X-Hub/src/v2i-hub/SpatPlugin/build/
./SpatPlugin_test --gtest_output=xml:../../../../test_results/
cd /home/V2X-Hub/src/v2i-hub/SpatPlugin/
mkdir coverage
cd /home/V2X-Hub/src/v2i-hub/
gcovr --sonarqube SpatPlugin/coverage/coverage.xml -s -f SpatPlugin/ -r .
