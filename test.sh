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

# valgrind
has_valgrind=0
type valgrind &>/dev/null
if [ $? -eq 0 ]
then
    has_valgrind=1
fi

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
    # test start
    test_start=$(python -c 'import time; print time.time()')

    rom="${asm%.*}"
    dir=$(dirname $rom)

    # valgrind
    valgrind_fail=0

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

        if [ $has_valgrind -eq 1 ]
        then
            valgrind --leak-check=full --show-reachable=yes --show-leak-kinds=all --suppressions=suppressions.supp --error-exitcode=1 -q ./nessemble $rom.rom --disassemble &>/dev/null
            valgrind_fail_disassembly=$?

            if [ $valgrind_fail_disassembly -eq 1 ]
            then
                valgrind_type="disassembly"
                valgrind_fail=1
            fi
        fi
    fi

    # simulate
    simulation_fail=0
    ls $rom-simulated.txt &>/dev/null
    simulation_nonexist=$?

    if [ $simulation_nonexist -eq 0 ]
    then
        diff "$rom-simulated.txt" <(./nessemble --simulate $rom.rom --recipe $rom-recipe.txt) &>/dev/null
        simulation_fail=$?

        if [ $has_valgrind -eq 1 ]
        then
            valgrind --leak-check=full --show-reachable=yes --show-leak-kinds=all --suppressions=suppressions.supp --error-exitcode=1 -q ./nessemble --simulate $rom.rom --recipe $rom-recipe.txt &>/dev/null
            valgrind_fail_simulation=$?

            if [ $valgrind_fail_simulation -eq 1 ]
            then
                valgrind_type="simulation"
                valgrind_fail=1
            fi
        fi
    fi

    IFS=$OLDIFS

    # assembly
    diff "$rom.rom" <(./nessemble $asm --output - $flags) &>/dev/null
    diff_rc=$?

    if [ $has_valgrind -eq 1 ]
    then
        valgrind --leak-check=full --show-reachable=yes --show-leak-kinds=all --suppressions=suppressions.supp --error-exitcode=1 -q ./nessemble $asm --output - $flags &>/dev/null
        valgrind_fail_assembly=$?

        if [ $valgrind_fail_assembly -eq 1 ]
        then
            valgrind_type="assembly"
            valgrind_fail=1
        fi
    fi

    # if no assembly diff and no disassembly diff
    if [ $diff_rc -eq 0 ] && [ $disassembly_fail -eq 0 ] && [ $simulation_fail -eq 0 ] && [ $valgrind_fail -eq 0 ]
    then
        if [ "$flag_silent" != "1" ]
        then
            printf "  \033[0;32m✔\033[0m %s" $(basename $asm)

            if [ $disassembly_nonexist -eq 0 ]
            then
                printf " *"
            fi

            if [ $simulation_nonexist -eq 0 ]
            then
                printf " *"
            fi

            pass=$(echo "$pass + 1" | bc)

            test_end=$(python -c 'import time; print time.time()')
            test_duration=$(printf "%s - %s\n" $test_end $test_start | bc)

            printf " \033[1;30m(%ss)\033[0m\n" $test_duration
        fi
    else
        printf "\033[0;31m✗\033[0m %s" $asm

        # if disassembly diff
        if [ $disassembly_fail -ne 0 ]
        then
            printf " [disassembly fail]"
        fi

        # if simulation diff
        if [ $simulation_fail -ne 0 ]
        then
            printf " [simulation fail]"
        fi

        # if valgrind fail
        if [ $valgrind_fail -ne 0 ]
        then
            printf " [valgrind fail: %s]" $valgrind_type
        fi

        if [ $disassembly_fail -ne 0 ] || [ $simulation_fail -ne 0 ] || [ $valgrind_fail -ne 0 ]
        then
            printf "\n"
        fi

        # if assembly fail
        if [ $diff_rc -ne 0 ]
        then
            printf "\n\n  Actual:\n    "
            ./nessemble $asm --output - $flags | hexdump -n 128 -e '1/1 "%.2X "'
            printf "\n\n  Expected:\n    "
            cat "$rom.rom" | hexdump -n 128 -e '1/1 "%.2X "'
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
