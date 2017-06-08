# coding=utf-8
# pylint: disable=C0103,C0326
"""Nessemble config"""

import os
from ConfigParser import ConfigParser

BASE       = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))

config     = ConfigParser()
config.readfp(open(os.path.join(BASE, 'settings.cfg')))

#--------------#
# Variables

__all__ = ['config']
