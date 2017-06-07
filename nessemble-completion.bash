#!/bin/bash

if [[ "$SHELL" == *"zsh" ]]
then
    autoload bashcompinit
    bashcompinit
fi

_nessemble()
{
    local cur prev first opts
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    first="${COMP_WORDS[1]}"

    long_flags="--output --format --empty --undocumented --list --pseudo --check --coverage --disassemble --reassemble --simulate --recipe --version --license --help"
    short_flags="-o -f -e -u -l -p -c -C -d -R -s -r -v -L -h"
    flags="$long_flags $short_flags"

    cmds="init scripts reference config registry install uninstall publish info ls search adduser login logout forgotpassword resetpassword"

    all_opts="$flags $cmds"

    if [[ ${cur} == -* ]] ; then
        COMPREPLY=($(compgen -W "${all_opts}" -- ${cur}))
        return 0
    fi

    case "${prev}" in
        -o|--output)
            COMPREPLY=($(compgen -f ${cur}))
            return 0
            ;;
        -f|--format)
            format_opts="NES RAW"
            COMPREPLY=($(compgen -W "${format_opts}" -- ${cur}))
            return 0
            ;;
        -e|--empty)
            empty_opts="FF 00"
            COMPREPLY=($(compgen -W "${empty_opts}" -- ${cur}))
            return 0
            ;;
        -l|--list)
            COMPREPLY=($(compgen -f ${cur}))
            return 0
            ;;
        -p|--pseudo)
            COMPREPLY=($(compgen -f ${cur}))
            return 0
            ;;
        -s|--simulate)
            COMPREPLY=($(compgen -f ${cur}))
            return 0
            ;;
        -r|--recipe)
            COMPREPLY=($(compgen -f ${cur}))
            return 0
            ;;
        init|scripts|reference|registry|install|info|ls|search|adduser|login|logout|forgotpassword|resetpassword)
            COMPREPLY=($(compgen -W "" - ${cur}))
            return 0
            ;;
        config)
            config_opts="registry"
            COMPREPLY=($(compgen -W "${config_opts}" - ${cur}))
            return 0
            ;;
        uninstall)
            uninstall_opts=$(ls ~/.nessemble/packages)
            COMPREPLY=($(compgen -W "${uninstall_opts}" - ${cur}))
            return 0
            ;;
        publish)
            COMPREPLY=($(compgen -f ${cur}))
            return 0
            ;;
        *)
        ;;
    esac

    case "${first}" in
        uninstall)
            uninstall_opts=$(ls ~/.nessemble/packages)
            COMPREPLY=($(compgen -W "${uninstall_opts}" - ${cur}))
            return 0
            ;;
        *)
        ;;
    esac

    COMPREPLY=($(compgen -W "${all_opts}" -- ${cur}))
}

complete -F _nessemble nessemble
