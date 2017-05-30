#!/usr/bin/env python
# coding=utf-8
# pylint: disable=C0301,C0326
"""Nessemble website server"""

import argparse
from website.app import app

#--------------#
# Constants

HOSTNAME = '0.0.0.0'
PORT     = 5000

#--------------#
# Main

def main():
    """Main function"""

    parser = argparse.ArgumentParser(description='Nessemble website')
    parser.add_argument('--host', '-H', dest='host', type=str, default=HOSTNAME, required=False, help='Host')
    parser.add_argument('--port', '-P', dest='port', type=int, default=PORT, required=False, help='Port')
    parser.add_argument('--debug', '-D', dest='debug', action='store_true', required=False, help='Debug mode')

    args = parser.parse_args()

    app.run(host=args.host, port=args.port, debug=args.debug)

if __name__ == '__main__':
    main()
