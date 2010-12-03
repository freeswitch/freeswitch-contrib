#!/bin/bash

. settings.sh
echo Start accepted tests

# echo $ACCEPTED_TESTS

for test in $ACCEPTED_TESTS
do
    echo $test
    $test
done



echo Start contrib tests
for test in $CONTRIB_TESTS
do
    $test
    LAST_EXIT=$?
    if [ $LAST_EXIT -eq "0" ]
    then
	echo "$test passed";
    else
	echo "$test failed with exit code $LAST_EXIT";
    fi
    
done
