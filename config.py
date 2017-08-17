#!/usr/bin/env python
# pylint: disable=C0103
"""gunicorn config"""

import multiprocessing
import os
from subprocess import call

#--------------#
# Variables

bind = '0.0.0.0:%s' % (os.getenv('PORT', '8000'))
workers = multiprocessing.cpu_count() * 2 + 1

#--------------#
# Functions

def on_starting(_server):
    """On Starting func"""

    call(['python', '-c', '"from server import db_import; db_import()"'])
