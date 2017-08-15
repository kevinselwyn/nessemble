#!/usr/bin/python
# coding=utf-8
# pylint: disable=C0103, C0301,W0612
"""Unit tests"""

import os
import subprocess
import unittest

tests = []

class TestSequence(unittest.TestCase):
    """Test suite"""

    pass

def test_generator(filename):
    """Test generator"""

    def test(self):
        """Test"""

        child = subprocess.Popen(['python', filename], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        data = child.communicate()[0]
        rc = child.returncode

        self.assertEqual(rc, 0)

    return test

def main():
    """Main function"""

    dirname = os.path.dirname(os.path.realpath(__file__))

    for root, dirnames, filenames in os.walk(dirname):
        for filename in filenames:
            if filename == 'test.py':
                path = os.path.join(root, filename)
                name = os.sep.join(os.path.dirname(path).split(os.sep)[-2:])

                tests.append([name, path])

    for t in tests:
        test_name = 'test_%s' % (t[0])
        test = test_generator(t[1])
        setattr(TestSequence, test_name, test)

    unittest.main()

if __name__ == '__main__':
    main()
