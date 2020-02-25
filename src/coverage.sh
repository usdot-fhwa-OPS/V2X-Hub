#!/bin/bash

for d in v2i-hub/*
do
    echo ""
    echo $d
    if [[ -d $d ]]; then
        TESTS=`ls $d | grep run[a-zA-Z]*Tests`
        echo $TESTS
        cd $d
        if $TESTS; then
            ./$TESTS
        fi
        gcovr -k -r .
        mv *.gcov ../../../coverage_reports/gcov
        cd ../..
    fi
done