#!/usr/bin/python
# coding=utf-8
# pylint: disable=C0103,C0301
"""help test"""

import filecmp
import os
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

def cleanup(rc):
    """Cleanup"""

    root = os.getcwd()

    try:
        os.unlink('%s%sexample.txt' % (root, os.sep))
    except OSError:
        pass

    exit(rc)

def main():
    """Main function"""

    root = os.getcwd()
    dirname = os.path.dirname(os.path.realpath(__file__))
    example = 'init'
    flags = ['init', '%s%sexample.txt' % (root, os.sep), '1', '1', '0', '0']

    valgrind = which('valgrind')

    if not example:
        cleanup(1)

    # normal
    if os.path.exists('%s%s%s.txt' % (dirname, os.sep, example)):
        arguments = ['.%snessemble' % (os.sep)]
        if flags:
            arguments.extend(flags)

        child = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        child.communicate()
        rc = child.returncode

        if not filecmp.cmp('%s%s%s.txt' % (dirname, os.sep, example), '%s%sexample.txt' % (root, os.sep)):
            print 'failed'
            print ' '.join(arguments)
            cleanup(1)

        os.unlink('%s%sexample.txt' % (root, os.sep))

        # valgrind
        if valgrind:
            arguments = [valgrind, '--leak-check=full', '--show-reachable=yes', '--show-leak-kinds=all', '--suppressions=%s%s%s%ssuppressions.supp' % (root, os.sep, 'test', os.sep), '--error-exitcode=2', '-q', '%s%snessemble' % (root, os.sep)]
            if flags:
                arguments.extend(flags)

            child = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            child.communicate()
            rc = child.returncode

            if rc == 2:
                print 'failed memory leak'
                print ' '.join(arguments)
                cleanup(rc)

    cleanup(0)

if __name__ == '__main__':
    main()
