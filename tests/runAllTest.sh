#!/bin/bash
set -a DYLD_LIBRARY_PATH
INST_DIR=$PWD"/../install/Tao presentations.app/Contents"
if [[ -f $INST_DIR"/MacOS/Tao presentations" ]];
then
  DYLD_LIBRARY_PATH=$INST_DIR"/Frameworks"
  TAO="cd \"$INST_DIR/MacOS\"; ./Tao\ presentations -norepo "
else
  echo $INST_DIR"/MacOS/Tao presentations not found."
  exit -1
fi

runTests="run_all_tests -> "$'\n'
listTests="\"Tests that will run :\""$'\n'
importTests=""

function generateOneTest()
{
    testName=$1
#    testDir=$PWD/$testName;
    testDir=$testName;
    testCmdName=$testDir/$testName"_test.ddd"
    testFileName=$testDir/$testName".ddd"
    testOrigName=$testFileName"-orig"

    if [[ -f $testOrigName ]];
    then
        echo "copiing $testOrigName into $testFileName"
        cp -f $testOrigName $testFileName
    fi
    if [[ -f $testCmdName && -f $testFileName ]];
    then
        echo "testName : $testName"
        runTests=$runTests"    goto_page \"${testName}\""$'\n'
        runTests=$runTests"    load \"${testCmdName}\""$'\n'
        runTests=$runTests"    ${testName}_test"$'\n'
        runTests=$runTests"    test_replay"$'\n\n'

        listTests=$listTests"        * \"${testName}\""$'\n'
        importTests=$importTests"import \"${testFileName}\""$'\n'
    fi
}

function listTests()
{
    dirname=$1
    for aFile in `/bin/ls $dirname` ; do
        if [[ -d $aFile ]];
        then
            generateOneTest $aFile
        fi ;
    done
    runTests=$runTests"    goto_page \"results\""$'\n'
}



function generateCore()
{
    cat > $PWD/runTests.ddd << EOF

import TaoTester 1.0

toolbar_test
show_source false

* X:text ->
    factor -> 20.0
    margins 40, 0
    paragraph_break
    anchor
        color "Accent", 5
        circle factor * -75%, factor * 30%, factor * 20%
    text X

$runTests

page "intro",
    active_widget
        color 0.592157, 0.772549, 0.109804, 1.0
        push_button -50, 440, 240, 50, "button_1", "Run all tests",
            run_all_tests
    text_box 0, 0, 300, 600,
        text $listTests

$importTests

page "results",
    import "results.ddd"

EOF
}

function prepareResult()
{
cat >  $PWD/results.ddd <<EOF

load "result_style.xl"

text_box 0,0,800,1500,
EOF

}

listTests $PWD

generateCore

prepareResult

eval "$TAO $PWD/runTests.ddd"
