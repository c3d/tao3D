#!/bin/bash
INST_DIR="../install/Tao\ Presentations.app/Contents"
TAO="$INST_DIR/MacOS/Tao\ Presentations -nogit "
MODE=$1

function findLastRun()
{
    testName=$1
    list = `ls -dt1 $testName/run-* | head -n 1`
    return $list;
}
function runOneTest()
{
    testName=$1
    if [[ -f $testName/$testName.ddd-orig ]] ;
    then
        cp -f  $testName/$testName.ddd-orig $testName/$testName.ddd
    fi
    #echo "$testName/$testName.ddd"
    eval "$TAO $testName/$testName.ddd"
    res=$?
    if [[ -f $testName/$testName.ddd-orig ]] ;
    then
        lastRun = findLastRun $testName
        mv  $testName/$testName.ddd $testName/$lastRun
    fi

}

function listTests()
{
    eval "./rc.expect $MODE &"
    expectPID=$!

    for aFile in $@ ; do
        if [[ -d $aFile ]];
        then
            runOneTest $aFile
        fi ;
    done
    kill $expectPID

}


if [[ $# > 1 ]];
then
    shift
    listTests $@

else
    listTests `/bin/ls $PWD`
fi

