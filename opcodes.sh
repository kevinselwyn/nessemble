#!/bin/bash

MODES=()
METAS=()
LABELS=()
OLDIFS=$IFS
IFS=,
CSV=()
input=$1
output=$2
header=_
count=0
i=0

# functions
function join_by {
    local d=$1
    shift
    echo -n "$1"
    shift
    printf "%s" "${@/#/$d}"
}

# usage
if [ -z "$input" ] || [ -z "$output" ]
then
    printf "Usage: ./opcodes.sh <input.csv> <output.h>\n"
    IFS=$OLDIFS
    exit 1
fi

# save csv
while read line
do
    CSV+=("$line")
done < $input

# start file
header+=$(echo $output | awk '{print toupper($0)}')
header=${header/./_}

printf "#ifndef %s\n" $header > $output
printf "#define %s\n\n" $header >> $output

# generate mode #defines
for line in "${CSV[@]}"
do
    while read mnemonic mode opcode length timing meta
    do
        count=$(printf "%d+1\n" $count | bc)

        if [[ ! " ${MODES[@]} " =~ " ${mode} " ]]
        then
            MODES+=($mode)
        fi
    done <<< "$line"
done

i=0

for mode in "${MODES[@]}"
do
    printf "#define %-16s 0x%02X\n" $mode $i >> $output
    i=$(printf "%d+1\n" $i | bc)
done

# generate struct
printf "\nstruct opcode {\n" >> $output
printf "    char mnemonic[4];\n" >> $output
printf "    int mode;\n" >> $output
printf "    int opcode;\n" >> $output
printf "    int length;\n" >> $output
printf "    int timing;\n" >> $output
printf "    int meta;\n" >> $output
printf "};\n\n" >> $output

# meta info
METAS=("META_NONE" "META_BOUNDARY" "META_UNDOCUMENTED")

i=0

for meta in "${METAS[@]}"
do
    printf "#define %-17s 0x%02X\n" $meta $(printf "2 ^ (%d - 1)\n" $i | bc) >> $output
    i=$(printf "%d+1\n" $i | bc)
done

# generate opcodes
printf "\n#define OPCODE_COUNT %s\n\n" $count >> $output

printf "struct opcode opcodes[OPCODE_COUNT] = {\n" $count >> $output

for line in "${CSV[@]}"
do
    while read mnemonic mode opcode length timing meta
    do
        meta_arr=()
        meta_str=$meta

        if [[ "$meta" == "0" ]]
        then
            meta_str="META_NONE"
        else
            if ((($meta&1)>0))
            then
                meta_arr+=("META_BOUNDARY")
            fi

            if ((($meta&2)>0))
            then
                meta_arr+=("META_UNDOCUMENTED")
            fi
        fi

        if [[ "${meta_arr[@]}" > 0 ]]
        then
            meta_str=$(printf "%s" $(join_by " | " "${meta_arr[@]}"))
        fi

        printf "    { %-07s %-17s %s, %s, %s, %s },\n" "$mnemonic," "$mode," $opcode $length $timing $meta_str >> $output
    done <<< "$line"
done

printf "};\n\n" >> $output

# labels
LABELS=("SYMBOL_LABEL" "SYMBOL_CONSTANT" "SYMBOL_RS")

i=0

for label in "${LABELS[@]}"
do
    printf "#define %-15s 0x%02X\n" $label $i >> $output
    i=$(printf "%d+1\n" $i | bc)
done

printf "\nstruct symbol {\n" >> $output
printf "    char *name;\n" >> $output
printf "    int value;\n" >> $output
printf "    int type;\n" >> $output
printf "};\n\n" >> $output

printf "struct symbol symbols[1024];\n\n" >> $output

# finish
printf "#endif /* %s */\n" $header >> $output

IFS=$OLDIFS
