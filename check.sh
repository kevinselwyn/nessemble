#!/bin/bash

files="y.tab.c lex.yy.c assemble.c disassemble.c error.c instructions.c macro.c opcodes.c png.c `ls pseudo/*.c` simulate.c `ls simulate/*.c` usage.c utils.c wav.c"
flags="-ll -lpng"

for file in check/*.c
do
    name=`echo "$file" | sed -r 's/check\///g' | sed -r 's/\.c//g'`
    utils=""

    gcc -o check/suite_$name $file $files $flags `pkg-config --cflags --libs check`
done

for suite in check/suite_*
do
    ./$suite
done
