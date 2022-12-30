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

set -ex

top_dir="$PWD"

COMPONENT_DIRS="tmx v2i-hub"
for component_dir in ${COMPONENT_DIRS}; do
for d in ${component_dir}/* ; do
    echo ""
    echo $d
    sub_dir=${top_dir}/${d}
    build_dir=${top_dir}/${component_dir}/build/$(basename ${d})
    if [ -d ${sub_dir} -a -d ${build_dir} ]; then
        if ls $build_dir | grep -E "[a-zA-Z]*_test|run[a-zA-Z]*Tests"; then
            TESTS="./$(ls $build_dir | grep -E "[a-zA-Z]*_test|run[a-zA-Z]*Tests")"
            echo "$TESTS built"
            pushd "$build_dir"
            $TESTS
            popd
            # generate a JSON file to be combined per https://gcovr.com/en/master/guide/merging.html
            gcovr -k --json ${component_dir}/$(basename ${d})-coverage.json -s -f ${sub_dir} -r .
            #gcovr -k --sonarqube ${build_dir}/coverage.xml -s -f ${sub_dir} -r .
        else
            echo "no tests built"
        fi
    fi
done
# combine all the JSON files for a component
gcovr --add-tracefile "${component_dir}/*-coverage.json" --sonarqube ${component_dir}/coverage.xml
done