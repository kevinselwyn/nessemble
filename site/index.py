#!/usr/bin/env python
# coding=utf-8
# pylint: disable=C0103,C0301,C0326
"""Nessemble website"""

import argparse
from flask import Flask, make_response, render_template

#--------------#
# Constants

HOSTNAME = '0.0.0.0'
PORT     = 8000

#--------------#
# Variables

app = Flask(__name__)

#--------------#
# Functions

def custom_response(data, status=200, mimetype='text/html'):
    """Custom HTTP response"""

    response = make_response(data, status)

    response.headers.remove('Content-Type')
    response.headers.add('Content-Type', mimetype)

    return response

#--------------#
# Endpoints

@app.route('/', methods=['GET'])
def index():
    """Index endpoint"""

    return custom_response(render_template('index.html'))

#--------------#
# Variables

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
