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
for d in v2i-hub/*
do
    echo ""
    echo $d
    if [[ -d $d ]]; then
        if ls $d | grep run[a-zA-Z]*Tests ; then
            TESTS="./`ls $d | grep run[a-zA-Z]*Tests`"
            echo "$TESTS built"
            cd $d
            $TESTS
            gcovr --sonarqube coverage.xml -k -r . # Run gcovr with -k to ensure generated .gcov files are preserved -r . makes it run in the current directory
            cd ../..
        else
            echo "no tests built"
        fi
    fi
done
for d in tmx/*
do
    echo ""
    echo $d
    if [[ -d $d ]]; then
        if ls $d | grep [a-zA-Z]*_test ; then
            TESTS="./`ls $d | grep [a-zA-Z]*_test`"
            echo "$TESTS built"
            cd $d
            $TESTS
            gcovr --sonarqube coverage.xml -k -r . # Run gcovr with -k to ensure generated .gcov files are preserved -r . makes it run in the current directory
            cd ../..
        else
            echo "no tests built"
        fi
    fi
done
