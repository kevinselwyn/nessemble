#!/usr/bin/python
# coding=utf-8
# pylint: disable=C0103,C0301,C0413,E0401,R0912,R0914,R0915,W0702
"""POST /user/login"""

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
    url = '/'.join([api_root, 'user', 'login'])

    # login
    try:
        req = requests.post(url, auth=('kevinselwyn', 'pass'))
    except requests.exceptions.ConnectionError:
        print 'No connection'
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

    validator = Validator({
        'token': {
            'type': 'string',
            'minlength': 1,
            'required': True
        }
    })

    if not validator.validate(json_data):
        for key, value in validator.errors.iteritems():
            if isinstance(value[0], dict):
                for key2, value2 in value[0].iteritems():
                    print 'JSON field `%s.%s` %s' % (key, key2, value2[0])
                    exit(1)
            else:
                print 'JSON field `%s` %s' % (key, value[0])
                exit(1)

    exit(0)

if __name__ == '__main__':
    main()
