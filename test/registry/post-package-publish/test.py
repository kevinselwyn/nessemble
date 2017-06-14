#!/usr/bin/python
# coding=utf-8
# pylint: disable=C0103,C0301,C0413,E0401,R0912,R0914,R0915,W0702
"""POST /package/publish"""

import base64
import hmac
import io
import json
import os
import StringIO
import sys
import tarfile
from hashlib import sha1
import requests
from cerberus import Validator

ROOT = os.path.abspath(os.path.join(os.path.abspath(os.path.dirname(__file__)), '..'))
sys.path.append(ROOT)

import config as config

def main():
    """Main function"""

    # assemble url
    api_root = config.config['api_root']
    url = '/'.join([api_root, 'package', 'publish'])
    login_url = '/'.join([api_root, 'user', 'login'])

    # login
    try:
        login_req = requests.post(login_url, auth=('kevinselwyn', 'pass'))
    except requests.exceptions.ConnectionError:
        print 'No connection'
        exit(1)

    try:
        login_json = login_req.json()
        token = login_json['token']
        hmac_hash = hmac.new(str(token), 'POST+/package/publish', sha1)
        hmac_sha1 = hmac_hash.hexdigest()
        base64_auth = base64.b64encode('%s:%s' % ('kevinselwyn', hmac_sha1))
    except:
        print 'Token error'
        exit(1)

    # assemble data
    json_expect = {
        'title': 'test',
        'description': 'Test',
        'version': '1.0.1',
        'author': 'kevinselwyn',
        'license': 'WTFPL',
        'tags': ['test']
    }

    infile = io.BytesIO()
    tar = tarfile.open(fileobj=infile, mode='w:gz')

    lib_asm_file = StringIO.StringIO()
    lib_asm_file.write('  LDA #$01')
    lib_asm_file.seek(0)
    lib_asm_info = tarfile.TarInfo(name='lib.asm')
    lib_asm_info.size = len(lib_asm_file.buf)
    tar.addfile(tarinfo=lib_asm_info, fileobj=lib_asm_file)

    readme_md_file = StringIO.StringIO()
    readme_md_file.write('# README')
    readme_md_file.seek(0)
    readme_md_info = tarfile.TarInfo(name='README.md')
    readme_md_info.size = len(readme_md_file.buf)
    tar.addfile(tarinfo=readme_md_info, fileobj=readme_md_file)

    package_json_file = StringIO.StringIO()
    package_json_file.write(json.dumps(json_expect))
    package_json_file.seek(0)
    package_json_info = tarfile.TarInfo(name='package.json')
    package_json_info.size = len(package_json_file.buf)
    tar.addfile(tarinfo=package_json_info, fileobj=package_json_file)

    tar.close()
    infile.seek(0)

    # make request
    try:
        req = requests.post(url, data=infile.read(), headers={
            'Authorization': 'HMAC-SHA1 %s' % (base64_auth)
        })
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
        json_data = req.json()
    except:
        json_data = {}

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

    if json_data != json_expect:
        print 'JSON mismatch'
        exit(1)

    exit(0)

if __name__ == '__main__':
    main()
