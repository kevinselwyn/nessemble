#!/usr/bin/env python
# coding=utf-8
# pylint: disable=C0103,C0301,C0326,C0413
"""Nessemble CDN server"""

import os
import argparse
from cdn.app import app
from cdn.config import config as CONFIG

#--------------#
# Constants

BASE     = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
NAME     = CONFIG.get('cdn', 'name')
HOSTNAME = CONFIG.get('cdn', 'host')
PORT     = CONFIG.getint('cdn', 'port')

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
