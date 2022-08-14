if [ "$1" = "-clean" ]
then
    make clean
fi

#!/bin/sh
make -j 

if [ "$1" = "-w" ] 
then
    $SHELL
fi
