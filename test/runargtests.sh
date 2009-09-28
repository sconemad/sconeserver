#! /bin/bash

for file in arg/*.test; do

    echo ""
    echo "### RUNNING $file"
    echo ""
    
    ./argtest < $file
    
    if [ $? != 0 ]; then
	echo ""
	echo "<<< FAILED $file >>> "
	echo ""
	exit 1;
    fi
    
done
