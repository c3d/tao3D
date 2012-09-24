#!/bin/bash


function cleanRunOneTest()
{
    testName=$1
    echo " cleaning $testName/run-*"
    `/bin/rm -rf $testName/run-*`
}


function cleanRefOneTest()
{
    testName=$1
    echo " cleaning $testName/*.png"
    `/bin/rm -f $testName/*.png`
}


function listTests()
{
    dirname=$1
    what=$2
    for aFile in `/bin/ls $dirname` ; do
        if [[ -d $aFile ]];
        then
            if [[ $what -eq "RUN" || $what -eq "ALL" ]];
            then
                cleanRunOneTest $aFile
            fi;
            if [[ $what -eq "REF" || $what -eq "ALL" ]];
            then
                cleanRefOneTest $aFile
            fi;
        fi ;
    done

}

listTests $PWD $1

