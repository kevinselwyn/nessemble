#!/usr/bin/python
# coding=utf-8
# pylint: disable=C0103,C0301
"""pong1 test"""

import os
import subprocess

def main():
    """Main function"""

    dirname = os.path.dirname(os.path.realpath(__file__))
    example = 'pong1'

    child = subprocess.Popen(['python', '%s/../nerdy-test.py' % (dirname), example], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    data = child.communicate()[0]
    rc = child.returncode

    if rc != 0:
        print data

    exit(rc)

if __name__ == '__main__':
    main()
