#!/bin/bash
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
            cd ../..
        else
            echo "no tests built"
        fi
    fi
done

cd v2i-hub
pwd
gcovr -k ./P*
mv *.gcov ../../coverage_reports/gcov