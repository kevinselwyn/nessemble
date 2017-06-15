#!/usr/bin/python
# coding=utf-8
# pylint: disable=C0103,C0301,C0413,E0401,R0912,R0914,R0915,W0702
"""GET /package/<string:package>"""

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
    package = 'rle'
    url = '/'.join([api_root, 'package', package])

    # make request
    try:
        req = requests.get(url)
    except requests.exceptions.ConnectionError:
        print 'No connection'
        exit(1)

    # check status code
    if req.status_code != 200:
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
        json = req.json()
    except:
        json = {}

    validator = Validator({
        'title': {
            'type': 'string',
            'minlength': 1,
            'required': True
        },
        'description': {
            'type': 'string',
            'minlength': 1,
            'required': True
        },
        'version': {
            'type': 'string',
            'minlength': 1,
            'required': True
        },
        'versions': {
            'type': 'dict',
            'schema': {
                '1.0.1': {
                    'type': 'string',
                    'minlength': 1,
                    'required': True
                }
            }
        },
        'author': {
            'type': 'string',
            'minlength': 1,
            'required': True
        },
        'license': {
            'type': 'string',
            'minlength': 1,
            'required': True
        },
        'tags': {
            'type': 'list',
            'minlength': 1,
            'required': True,
            'schema': {
                'type': 'string',
                'minlength': 1
            }
        },
        'date': {
            'type': 'string',
            'minlength': 1,
            'required': True
        },
        'readme': {
            'type': 'string',
            'minlength': 1,
            'required': True
        },
        'resource': {
            'type': 'string',
            'minlength': 1,
            'required': True
        },
        'shasum': {
            'type': 'string',
            'minlength': 1,
            'required': True
        }
    })

    if not validator.validate(json):
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
