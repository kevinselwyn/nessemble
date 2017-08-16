#!/usr/bin/env python
# pylint: disable=C0103
"""gunicorn config"""

import multiprocessing
import os

bind = '0.0.0.0:%s' % (os.getenv('PORT', '8000'))
workers = multiprocessing.cpu_count() * 2 + 1
