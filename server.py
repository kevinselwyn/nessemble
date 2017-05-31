#!/usr/bin/env python
# coding=utf-8
# pylint: disable=C0103,C0301,C0326
"""Server"""

import os
import argparse
from flask import Flask
from werkzeug.wsgi import DispatcherMiddleware
from website.app import app as website
from registry.app import app as registry, db_import
from docs.app import app as docs

#--------------#
# Constants

HOSTNAME = '0.0.0.0'
PORT     = 5000

#--------------#
# Variables

app = Flask(__name__)

app.wsgi_app = DispatcherMiddleware(website, {
    '/registry': registry,
    '/documentation': docs
})

#--------------#
# Main

def main():
    """Main function"""

    parser = argparse.ArgumentParser(description='Nessemble website')
    parser.add_argument('--host', '-H', dest='host', type=str, default=HOSTNAME, required=False, help='Host')
    parser.add_argument('--port', '-P', dest='port', type=int, default=PORT, required=False, help='Port')
    parser.add_argument('--debug', '-D', dest='debug', action='store_true', required=False, help='Debug mode')

    args = parser.parse_args()

    db = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'registry', 'registry.sql')
    db_import(db)

    app.run(host=args.host, port=args.port, debug=args.debug)

if __name__ == '__main__':
    main()
