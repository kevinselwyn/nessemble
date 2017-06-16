#!/usr/bin/python
# coding=utf-8
"""reference test"""

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
    example = 'reference'
    flags = ['reference']

    valgrind = which('valgrind')

    if not example:
        exit(1)

    # normal
    if os.path.exists('%s%s%s.txt' % (dirname, os.sep, example)):
        arguments = ['.%snessemble' % (os.sep)]
        if flags:
            arguments.extend(flags)

        child = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        stds = child.communicate()
        rc = child.returncode

        data = ''

        data += stds[0]

        tmp = tempfile.NamedTemporaryFile()
        tmp.write(data)
        tmp.seek(0)

        if not filecmp.cmp('%s%s%s.txt' % (dirname, os.sep, example), tmp.name):
            tmp.close()
            print 'failed'
            print ' '.join(arguments)
            exit(1)

        tmp.close()

        # valgrind
        if valgrind:
            arguments = [valgrind, '--leak-check=full', '--show-reachable=yes', '--show-leak-kinds=all', '--suppressions=%s%ssuppressions.supp' % (root, os.sep), '--error-exitcode=2', '-q', '%s%snessemble' % (root, os.sep)]
            if flags:
                arguments.extend(flags)

            child = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            data = child.communicate()[0]
            rc = child.returncode

            if rc == 2:
                print 'failed memory leak'
                print ' '.join(arguments)
                exit(rc)

    exit(0)

if __name__ == '__main__':
    main()
