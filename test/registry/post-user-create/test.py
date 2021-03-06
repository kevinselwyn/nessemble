#!/usr/bin/python
# coding=utf-8
# pylint: disable=C0103,C0301,C0413,E0401,R0912,R0914,R0915,W0702
"""POST /user/create"""

import os
import sys
import requests
from cerberus import Validator

ROOT = os.path.abspath(os.path.join(os.path.abspath(os.path.dirname(__file__)), '..'))
sys.path.append(ROOT)

import config as config

def main():
    """Main function"""

    # assemble url
    api_root = config.config['api_root']
    url = '/'.join([api_root, 'user', 'create'])

    # assemble user
    user = {
        'name': 'Joe Somebody',
        'username': 'joe.somebody',
        'email': 'joe.somebody@email.com',
        'password': 'pass'
    }

    # login
    try:
        req = requests.post(url, json=user)
    except requests.exceptions.ConnectionError:
        print 'No connection'
        exit(1)

    # check status code
    if req.status_code != 201:
        print 'Status code: %d' % (req.status_code)
        exit(1)

    # check headers
    exist_headers = [
        'Content-Length',
        'X-Response-Time',
        'X-RateLimit-Limit',
        'X-RateLimit-Remaining',
        'X-RateLimit-Reset',
        'Retry-After',
        'Date'
    ]

    for exist_header in exist_headers:
        if not exist_header in req.headers:
            print 'Missing header `%s`' % (exist_header)
            exit(1)

    # compare headers
    response_headers_expect = {
        'Content-Type': 'application/json',
        'Access-Control-Allow-Origin': '*',
        'Server': 'Nessemble'
    }

    response_headers_actual = {
        'Content-Type': req.headers['Content-Type'],
        'Access-Control-Allow-Origin': req.headers['Access-Control-Allow-Origin'],
        'Server': req.headers['Server']
    }

    for k, _v in response_headers_expect.iteritems():
        if response_headers_expect[k] != response_headers_actual[k]:
            print 'Mismatch header:\n  Expected: `%s`\n  Actual:   `%s`' % (response_headers_expect[k], response_headers_actual[k])
            exit(1)

    # validate content
    try:
        json_data = req.json()
    except:
        json_data = {}

    if json_data != {}:
        print 'JSON mismatch'
        exit(1)

    exit(0)

if __name__ == '__main__':
    main()
