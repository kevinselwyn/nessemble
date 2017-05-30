#!/usr/bin/env python
# coding=utf-8
# pylint: disable=C0103,C0301,C0326,C0413
"""Nessemble registry server"""

import os
import sys
import argparse

parent_dir = os.path.dirname(os.path.dirname(os.path.realpath(__file__)))
sys.path.append(parent_dir)

from registry.app import app, db_import, db_export

#----------------#
# Constants

HOSTNAME   = '0.0.0.0'
PORT       = 5000

#----------------#
# Main

def main():
    """Main function"""

    parser = argparse.ArgumentParser(description='Nessemble registry')
    parser.add_argument('--host', '-H', dest='host', type=str, default=HOSTNAME, required=False, help='Host')
    parser.add_argument('--port', '-P', dest='port', type=int, default=PORT, required=False, help='Port')
    parser.add_argument('--debug', '-D', dest='debug', action='store_true', required=False, help='Debug mode')
    parser.add_argument('--import', '-i', dest='db_import', type=str, help='Import SQL')
    parser.add_argument('--export', '-e', dest='db_export', action='store_true', help='Export SQL')

    args = parser.parse_args()

    if args.db_import:
        db_import(args.db_import)

        if not args.port and not args.debug:
            return

    if args.db_export:
        db_export()
        return

    app.run(host=args.host, port=args.port, debug=args.debug)

if __name__ == '__main__':
    main()
