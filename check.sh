#!/bin/bash

for file in check_*.c
do
    name=`echo "$file" | sed -r 's/check_//g' | sed -r 's/\.c//g'`
    gcc -o suite_$name $file $name.c `pkg-config --cflags --libs check`
done

for suite in suite_*
do
    ./$suite
done
