#!/usr/bin/env python
# coding=utf-8
# pylint: disable=C0103,C0301,C0326
"""Nessemble website server"""

import os
import argparse
from website.app import app
from website.config import config as CONFIG

#--------------#
# Constants

BASE     = os.path.normpath(os.path.join(os.path.dirname(__file__), '..'))
NAME     = CONFIG.get('website', 'name')
HOSTNAME = CONFIG.get('website', 'host')
PORT     = CONFIG.getint('website', 'port')

#--------------#
# Main

def main():
    """Main function"""

    parser = argparse.ArgumentParser(description=NAME)
    parser.add_argument('--host', '-H', dest='host', type=str, default=HOSTNAME, required=False, help='Host')
    parser.add_argument('--port', '-P', dest='port', type=int, default=PORT, required=False, help='Port')
    parser.add_argument('--debug', '-D', dest='debug', action='store_true', required=False, help='Debug mode')

    args = parser.parse_args()

    app.run(host=args.host, port=args.port, debug=args.debug, threaded=True)

if __name__ == '__main__':
    main()
