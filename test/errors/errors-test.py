#!/usr/bin/python
# coding=utf-8
# pylint: disable=C0103, C0301, R0912, R0915, W0612
"""Errors test"""

import filecmp
import os
import subprocess
import sys
import tempfile

def which(program):
    """Find program path"""

    def is_exe(fpath):
        """Detect executable"""

        return os.path.isfile(fpath) and os.access(fpath, os.X_OK)

    fpath, fname = os.path.split(program)

    if fpath:
        if is_exe(program):
            return program
    else:
        for path in os.environ["PATH"].split(os.pathsep):
            path = path.strip('"')
            exe_file = os.path.join(path, program)

            if is_exe(exe_file):
                return exe_file

    return None

def main():
    """Main function"""

    root = os.getcwd()
    dirname = os.path.dirname(os.path.realpath(__file__))

    try:
        err_name = sys.argv[1]
    except IndexError:
        exit(1)

    try:
        flag = sys.argv[2]
    except IndexError:
        flag = None

    valgrind = which('valgrind')

    if not err_name:
        exit(1)

    # assemble
    if os.path.exists('%s/%s/%s.rom' % (dirname, err_name, err_name)):
        arguments = ['%s/nessemble' % (root), '%s/%s/%s.asm' % (dirname, err_name, err_name), '--output', '-']
        if flag:
            arguments.append(flag)

        child = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stds = child.communicate()
        rc = child.returncode

        data = ''

        if stds[1]:
            data = stds[1]

        data += stds[0]

        tmp = tempfile.NamedTemporaryFile()
        tmp.write(data)
        tmp.seek(0)

        if not filecmp.cmp('%s/%s/%s.rom' % (dirname, err_name, err_name), tmp.name):
            tmp.close()
            print 'failed assembly'
            exit(1)

        tmp.close()

        # assemble (valgrind)
        if valgrind:
            arguments = [valgrind, '--leak-check=full', '--show-reachable=yes', '--show-leak-kinds=all', '--suppressions=%s/suppressions.supp' % (root), '--error-exitcode=2', '-q', '%s/nessemble' % (root), '%s/%s/%s.asm' % (dirname, err_name, err_name), '--output', '-']
            if flag:
                arguments.append(flag)

            child = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            data = child.communicate()[0]
            rc = child.returncode

            if rc != 0:
                print 'memory leak assembly'
                exit(rc)

    # disassemble
    if os.path.exists('%s/%s/%s-disassembled.txt' % (dirname, err_name, err_name)):
        arguments = ['%s/nessemble' % (root), '%s/%s/%s.rom' % (dirname, err_name, err_name), '--disassemble', '--output', '-']
        if flag:
            arguments.append(flag)

        child = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stds = child.communicate()
        rc = child.returncode

        data = ''

        if stds[1]:
            data = stds[1]

        data += stds[0]

        tmp = tempfile.NamedTemporaryFile()
        tmp.write(data)
        tmp.seek(0)

        if not filecmp.cmp('%s/%s/%s-disassembled.txt' % (dirname, err_name, err_name), tmp.name):
            tmp.close()
            print 'failed disassembly'
            exit(1)

        tmp.close()

        # disassemble (valgrind)
        if valgrind:
            arguments = [valgrind, '--leak-check=full', '--show-reachable=yes', '--show-leak-kinds=all', '--suppressions=%s/suppressions.supp' % (root), '--error-exitcode=2', '-q', '%s/nessemble' % (root), '%s/%s/%s.rom' % (dirname, err_name, err_name), '--disassemble', '--output', '-']
            if flag:
                arguments.append(flag)

            child = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            data = child.communicate()[0]
            rc = child.returncode

            if rc != 0:
                print 'memory leak disassembly'
                exit(rc)

    # simulate
    if os.path.exists('%s/%s/%s-simulated.txt' % (dirname, err_name, err_name)):
        arguments = ['%s/nessemble' % (root), '--simulate', '%s/%s/%s.rom' % (dirname, err_name, err_name), '--recipe', '%s/%s/%s-recipe.txt' % (dirname, err_name, err_name), '--output', '-']
        if flag:
            arguments.append(flag)

        child = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stds = child.communicate()
        rc = child.returncode

        data = ''

        if stds[1]:
            data = stds[1]

        data += stds[0]

        tmp = tempfile.NamedTemporaryFile()
        tmp.write(data)
        tmp.seek(0)

        if not filecmp.cmp('%s/%s/%s-simulated.txt' % (dirname, err_name, err_name), tmp.name):
            tmp.close()
            print 'failed simulation'
            exit(1)

        tmp.close()

        # simulate (valgrind)
        if valgrind:
            arguments = [valgrind, '--leak-check=full', '--show-reachable=yes', '--show-leak-kinds=all', '--suppressions=%s/suppressions.supp' % (root), '--error-exitcode=2', '-q', '%s/nessemble' % (root), '--simulate', '%s/%s/%s.rom' % (dirname, err_name, err_name), '--recipe', '%s/%s/%s-recipe.txt' % (dirname, err_name, err_name), '--output', '-']
            if flag:
                arguments.append(flag)

            child = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            data = child.communicate()[0]
            rc = child.returncode

            if rc != 0:
                print 'memory leak simulation'
                exit(rc)

if __name__ == '__main__':
    main()
