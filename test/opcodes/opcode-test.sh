#!/bin/bash

root=`pwd`
dir=`dirname $0`
opcode=$1

if [ -z "$opcode" ]
then
    exit 1
fi

# assemble
diff "$dir/$opcode/$opcode.rom" <($root/nessemble $dir/$opcode/$opcode.asm --output - 2>&1) &>/dev/null
diff_fail=$?

if [ $diff_fail -ne 0 ]
then
    printf "failed assembly %d"
    exit 1
fi

# assemble valgrind
valgrind --leak-check=full --show-reachable=yes --show-leak-kinds=all --suppressions=suppressions.supp --error-exitcode=2 -q $root/nessemble $dir/$opcode/$opcode.asm --output - &>/dev/null
valgrind_fail=$?

if [ $valgrind_fail -ne 0 ]
then
    printf "memory leak disassembly"
    exit 1
fi

# disassemble
diff "$dir/$opcode/$opcode-disassembled.txt" <($root/nessemble $dir/$opcode/$opcode.rom --disassemble 2>&1) &>/dev/null
diff_fail=$?

if [ $diff_fail -ne 0 ]
then
    printf "failed disassembly"
    exit 1
fi

# disassemble valgrind
valgrind --leak-check=full --show-reachable=yes --show-leak-kinds=all --suppressions=suppressions.supp --error-exitcode=2 -q $root/nessemble $dir/$opcode/$opcode.rom --disassemble &>/dev/null
valgrind_fail=$?

if [ $valgrind_fail -ne 0 ]
then
    printf "memory leak disassembly"
    exit 1
fi

# simulate
diff "$dir/$opcode/$opcode-simulated.txt" <($root/nessemble --simulate $dir/$opcode/$opcode.rom --recipe $dir/$opcode/$opcode-recipe.txt 2>&1) &>/dev/null
diff_fail=$?

if [ $diff_fail -ne 0 ]
then
    printf "failed simulation"
    exit 1
fi

# simulate valgrind
valgrind --leak-check=full --show-reachable=yes --show-leak-kinds=all --suppressions=suppressions.supp --error-exitcode=2 -q $root/nessemble --simulate $dir/$opcode/$opcode.rom --recipe $dir/$opcode/$opcode-recipe.txt &>/dev/null
valgrind_fail=$?

if [ $valgrind_fail -ne 0 ]
then
    printf "memory leak simulation"
    exit 1
fi

exit 0
