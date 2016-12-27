#!/bin/bash

files="y.tab.c lex.yy.c assemble.c pseudo.c opcodes.c instructions.c disassemble.c simulate.c png.c utils.c usage.c"
flags="-ll -lpng"

for file in check_*.c
do
    name=`echo "$file" | sed -r 's/check_//g' | sed -r 's/\.c//g'`
    utils=""

    gcc -o suite_$name $file $files $flags `pkg-config --cflags --libs check`
done

for suite in suite_*
do
    ./$suite
done
