#!/bin/bash

#  Copyright (C) 2018-2021 LEIDOS.
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

# This script is meant to generate .gcov files from source code built using code coverage compiler flags
# such that .gcno and .gcda files have been generated
# WARNING this script will remove any .gcov files which currently exist under the output directory argument 2. default=./coverage_reports/gcov/
#
# Parameters
# $1 The execution directory which this script will be run from. Generally this should be the catkin workspace with all source code and binaries below this directory
# $2 The output directory which the .gcov files will be written to
#
#


execution_dir="."
output_dir="./coverage_reports/gcov"

echo "Running gcovr with execution directory $1"
if [ -z "$1" ]; then
	echo "No execution root provided. Executing from ${execution_dir}"
else
	execution_dir=$1;
	cd ${execution_dir} # cd to execution directory
fi

if [ -z "$2" ]; then
	echo "No output directory provided. Output will be in ${output_dir}"
else
	output_dir=$2;
fi


echo "Ensuring output directory exists"
mkdir -p ${output_dir}


echo "Deleting old files"
find "${output_dir}" -iname "*\.gcda" -o -iname "*\.gcna" -o -iname "*\.gcov" -type f -exec rm -fv {} \;


# Grab resulting gcov files and move them to the ${output_dir} directory
echo "Moving new files"
cd "${output_dir}"
find /home/V2X-Hub -iname "*\.gcda" -o -iname "\.gcna" | xargs gcov

echo "Generating coverage.xml"
gcovr --sonarqube coverage.xml -k -r . # Run gcovr with -k to ensure generated .gcov files are preserved -r . makes it run in the current directory

exit 0
