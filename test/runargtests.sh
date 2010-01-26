#! /bin/bash

echo `dirname $0`

for file in `dirname $0`/arg/*.test; do

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
