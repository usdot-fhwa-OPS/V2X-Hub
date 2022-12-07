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

set -x

top_dir="$PWD"

for d in tmx/* v2i-hub/*; do
    echo ""
    echo $d
    sub_dir="$top_dir"/"$d"
    if [[ -d "$sub_dir" ]]; then
        if ls $sub_dir | grep -E "[a-zA-Z]*_test|run[a-zA-Z]*Tests"; then
            TESTS="./$(ls $sub_dir | grep -E "[a-zA-Z]*_test|run[a-zA-Z]*Tests")"
            echo "$TESTS built"
            cd "$sub_dir"
            $TESTS
            cd ../../..
            gcovr --sonarqube src/"$d"/coverage.xml -s -k -f src/"$d"/ -r .
            mkdir -p "$sub_dir"/coverage
            mv "$sub_dir"/*.gcov "$sub_dir"/coverage/
        else
            echo "no tests built"
        fi
    fi
done
