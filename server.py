#!/usr/bin/env python
# coding=utf-8
# pylint: disable=C0103,C0301,C0326
"""Nessemble server"""

import os
import argparse
from ConfigParser import ConfigParser
from flask import Flask
from werkzeug.wsgi import DispatcherMiddleware
from website.website import app as website
from registry.registry import app as registry
from docs.docs import app as docs

#--------------#
# Constants

BASE     = os.path.normpath(os.path.dirname(__file__))

CONFIG   = ConfigParser()
CONFIG.readfp(open(os.path.join(BASE, 'settings.cfg')))

NAME     = CONFIG.get('main', 'name')
HOSTNAME = CONFIG.get('main', 'host')
PORT     = CONFIG.getint('main', 'port')

SQL_PATH = os.path.join(BASE, 'registry', 'registry.sql')

#--------------#
# Variables

app = Flask(__name__)

app.wsgi_app = DispatcherMiddleware(website.app, {
    '/registry': registry.app,
    '/documentation': docs.app
})

#--------------#
# Main

def main():
    """Main function"""

    parser = argparse.ArgumentParser(description=NAME)
    parser.add_argument('--host', '-H', dest='host', type=str, default=HOSTNAME, required=False, help='Host')
    parser.add_argument('--port', '-P', dest='port', type=int, default=PORT, required=False, help='Port')
    parser.add_argument('--debug', '-D', dest='debug', action='store_true', required=False, help='Debug mode')

    args = parser.parse_args()

    registry.db_import(SQL_PATH)

    app.run(host=args.host, port=args.port, debug=args.debug)

if __name__ == '__main__':
    main()
