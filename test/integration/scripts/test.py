#!/usr/bin/python
# coding=utf-8
# pylint: disable=C0103,C0301
"""scripts test"""

import os
import shutil
import subprocess

def which(program):
    """Find program path"""

    def is_exe(fpath):
        """Detect executable"""

        return os.path.isfile(fpath) and os.access(fpath, os.X_OK)

    fpath, _fname = os.path.split(program)

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

def heartbeat():
    """Regular heartbeat"""

    # remove .nessemble directory
    home_dir = os.path.expanduser('~')
    shutil.rmtree('%s%s.nessemble' % (home_dir, os.sep), True)

    return True

def main():
    """Main function"""

    root = os.getcwd()
    flags = ['scripts']

    valgrind = which('valgrind')

    if not heartbeat():
        exit(1)

    # normal
    arguments = ['.%snessemble' % (os.sep)]
    if flags:
        arguments.extend(flags)

    child = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    child.communicate()
    rc = child.returncode

    # cmp
    if rc != 0:
        print 'failed'
        print ' '.join(arguments)
        exit(1)

    # valgrind
    if valgrind:
        if not heartbeat():
            exit(1)

        arguments = [valgrind, '--leak-check=full', '--show-reachable=yes', '--show-leak-kinds=all', '--suppressions=%s%s%s%ssuppressions.supp' % (root, os.sep, 'test', os.sep), '--error-exitcode=2', '-q', '%s%snessemble' % (root, os.sep)]
        if flags:
            arguments.extend(flags)

        child = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        child.communicate()
        rc = child.returncode

        if rc == 2:
            print 'failed memory leak'
            print ' '.join(arguments)
            exit(1)

if __name__ == '__main__':
    main()
