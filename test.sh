#!/bin/bash

STRELAC=release/strelac
if [ $# -gt 0 ]; then
    if [ -d $1 ]; then
        DIR=$1
    else
        DIR=""
    fi
else
    DIR="tests/"
fi

if [ $DIR ]; then
    ok=0;
    err=0;
    for fn in `find $DIR -type f -name *.strela`; do
        TESTDIR=`dirname $fn`
        if [ "$CURDIR" != "$TESTDIR" ]; then
            echo -e "\n>>> $TESTDIR"
            CURDIR="$TESTDIR"
        fi

        if $0 $fn; then
            ok=$((ok+1))
        else
            err=$((err+1))
        fi
    done
    echo ''
    if [ $err -gt 0 ]; then
        printf "\033[41;30m"
    elif [ $err -eq 0 ]; then
        printf "\033[42;30m"
    fi
    echo -e "Ran $((err+ok)) tests. $ok ok, $err failed.\033[0m"
    exit $err
else
    MODNAME=`basename $1 .strela`
    DIRNAME=`dirname $1`
    printf "$1 "
    if output=$($STRELAC --search ./ --timeout 5 $1); then
        echo "$output" | diff -u --strip-trailing-cr $DIRNAME/$MODNAME.out - # &>/dev/null
        if [ $? == 0 ]; then
            echo -e "\033[32mOK\033[0m"
            exit 0
        else
            echo -e "\033[31mDiff\033[0m"
            exit 1
        fi
    else
        echo -e "\033[31mError\033[0m"
        exit -1
    fi

fi