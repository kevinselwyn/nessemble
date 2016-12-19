#!/bin/bash

# test results
total=0
pass=0
fail=0

# flags
flag_silent="0"

# test flags
flags=""

# dir
old_dir=""

# IFS
OLDIFS=$IFS

# parse arguments
while [[ $# -gt 0 ]]
do
    key="$1"

    case $key in
        -s|--silent)
        flag_silent="1"
        shift
        ;;
        *)
        ;;
    esac
    shift
done

# timer start
start=$(python -c 'import time; print time.time()')

for asm in `find ./test -name '*.asm' | sort`
do
    rom="${asm%.*}"
    dir=$(dirname $rom)

    if [ "$flag_silent" != "1" ]
    then
        if [[ "$dir" != "$old_dir" ]]
        then
            printf "\n\033[0;4m%s/\033[0;0m (%d)\n" $dir $(find ./test -name '*.asm' | grep -i "$dir" | wc -l)
            old_dir=$dir
        fi
    fi

    if [ ! -e "$rom.rom" ]
    then
        if [ "$flag_silent" != "1" ]
        then
            printf "  \033[0;31m✗\033[0m %s \033[1;30m[missing ROM file]\033[0m\n" $(basename $asm)
        else
            printf "\033[0;31m✗\033[0m %s \033[1;30m[missing ROM file]\033[0m\n" $asm
        fi

        fail=$(echo "$fail + 1" | bc)
        total=$(echo "$total + 1" | bc)
        continue
    fi

    IFS=$'\n'

    flags=$(cat $asm | head -1 | sed 's/; Flags: //g')

    if [[ $flags != --* ]]
    then
        flags=""
    fi

    for testcase in $(cat $asm | grep "; Test: " | cut -b9-)
    do
        printf "     $testcase\n"
    done

    IFS=$OLDIFS

    diff "$rom.rom" <(./nessemble $asm --output - $flags) &>/dev/null

    if [ $? -eq 0 ]
    then
        if [ "$flag_silent" != "1" ]
        then
            printf "  \033[0;32m✔\033[0m %s\n" $(basename $asm)

            pass=$(echo "$pass + 1" | bc)
        fi
    else
        printf "\033[0;31m✗\033[0m %s\n" $asm
        printf "\n  Actual:\n    "
        ./nessemble $asm --output - $flags | hexdump -e '1/1 "%.2X "'
        printf "\n\n  Expected:\n    "
        cat "$rom.rom" | hexdump -e '1/1 "%.2X "'
        printf "\n\n"

        fail=$(echo "$fail + 1" | bc)
    fi

    total=$(echo "$total + 1" | bc)
done

# fail report
if [ "$fail" != "0" ]
then
    printf "\n\033[0;31m✗ %s of %s tests failed\033[0m\n" $fail $total
    exit 1
fi

# timer end
end=$(python -c 'import time; print time.time()')
duration=$(printf "%s - %s\n" $end $start | bc)

# pass report
printf "\033[0;32m✔ %s tests completed\033[0m \033[1;30m(%ss)\033[0m\n" $total $duration
