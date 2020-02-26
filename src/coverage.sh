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
            gcovr -k .
            mkdir coverage
            PLUGIN=`echo $d | cut -d "/" -f 2`
            mv $(ls | grep [a-zA-Z0-9#-]*$PLUGIN | grep -v test#  | grep gcov) coverage
            cd ../..
        else
            echo "no tests built"
        fi
    fi
done