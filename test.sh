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

# loop through .asm files
for asm in `find ./test -name '*.asm' | sort`
do
    rom="${asm%.*}"
    dir=$(dirname $rom)

    # display directory
    if [ "$flag_silent" != "1" ]
    then
        if [[ "$dir" != "$old_dir" ]]
        then
            printf "\n\033[0;4m%s/\033[0;0m (%d)\n" $dir $(find ./test -name '*.asm' | grep -i "$dir" | wc -l)
            old_dir=$dir
        fi
    fi

    # if no rom
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

    # get comment flags
    flags=$(cat $asm | head -1 | sed 's/; Flags: //g')

    if [[ $flags != --* ]]
    then
        flags=""
    fi

    # disassembly
    disassembly_fail=0
    ls $rom-disassembled.txt &>/dev/null
    disassembly_nonexist=$?

    if [ $disassembly_nonexist -eq 0 ]
    then
        diff "$rom-disassembled.txt" <(./nessemble $rom.rom --disassemble) &>/dev/null
        disassembly_fail=$?
    fi

    IFS=$OLDIFS

    # assembly
    diff "$rom.rom" <(./nessemble $asm --output - $flags) &>/dev/null
    diff_rc=$?

    # if no assembly diff and no disassembly diff
    if [ $diff_rc -eq 0 ] && [ $disassembly_fail -eq 0 ]
    then
        if [ "$flag_silent" != "1" ]
        then
            printf "  \033[0;32m✔\033[0m %s" $(basename $asm)

            if [ $disassembly_nonexist -eq 0 ]
            then
                printf " *"
            fi

            printf "\n"

            pass=$(echo "$pass + 1" | bc)
        fi
    else
        printf "\033[0;31m✗\033[0m %s" $asm

        # if disassembly diff
        if [ $disassembly_fail -ne 0 ]
        then
            printf " [disassembly fail]\n"
        fi

        # if assembly fail
        if [ $diff_rc -ne 0 ]
        then
            printf "\n\n  Actual:\n    "
            ./nessemble $asm --output - $flags | hexdump -e '1/1 "%.2X "'
            printf "\n\n  Expected:\n    "
            cat "$rom.rom" | hexdump -e '1/1 "%.2X "'
            printf "\n\n"
        fi

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
