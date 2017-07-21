#!/usr/bin/env python
# pylint: disable=C0103,C0301,C0326,W0603,W0702
"""Deploy nessemble"""

import argparse
import json
import netrc
import sys
import requests

DOMAIN   = 'api.heroku.com'
API_ROOT = 'https://%s/apps/' % (DOMAIN)

def get_auth():
    """Get auth"""

    try:
        get = netrc.netrc()
    except IOError:
        print 'You must login with Heroku first'
        sys.exit(1)

    authenticators = get.authenticators(DOMAIN)

    if not authenticators:
        print 'You must login with Heroku first'
        sys.exit(1)

    return (authenticators[0], authenticators[2])

def get_sources():
    """Get Heroku sources"""

    url = '%s/sources' % (API_ROOT)
    headers = {
        'Accept': 'application/vnd.heroku+json; version=3'
    }
    auth = get_auth()

    r = requests.post(url, headers=headers, auth=auth)

    if not r.status_code in [200, 201]:
        print 'Sources failed'
        sys.exit(1)

    sources = {}

    try:
        sources = r.json()
    except:
        print 'Could not parse response'
        sys.exit(1)

    return sources

def upload_tarball(source=None, tar=''):
    """Upload tarball"""

    url = ''
    data = ''

    if not source:
        print 'No source found'
        sys.exit(1)

    try:
        url = source['source_blob']['put_url']
    except KeyError:
        print 'No source found'
        sys.exit(1)

    with open(tar, 'r') as f:
        data = f.read()

    r = requests.put(url, data=data)

    if not r.status_code == 200:
        print 'Upload failed'
        sys.exit(1)

    return

def do_build(source=None):
    """Do build"""

    url = '%s/builds' % (API_ROOT)
    headers = {
        'Accept': 'application/vnd.heroku+json; version=3',
        'Content-Type': 'application/json'
    }
    auth = get_auth()
    data = {}

    if not source:
        print 'No source found'
        sys.exit(1)

    get_url = ''

    try:
        get_url = source['source_blob']['get_url']
    except KeyError:
        print 'No source found'
        sys.exit(1)

    data = {
        'source_blob': {
            'url': get_url,
            'version': '1.0.1'
        }
    }

    r = requests.post(url, data=json.dumps(data), headers=headers, auth=auth)

    if not r.status_code in [200, 201]:
        print 'Build failed'
        sys.exit(1)

    return

def main():
    """Main function"""

    global API_ROOT

    parser = argparse.ArgumentParser(description='Deploys nessemble to Heroku')
    parser.add_argument('--app', '-a', dest='app', type=str, default='nessemble', required=False, help='App name')
    parser.add_argument('--tar', '-t', dest='tar', type=str, required=True, help='Tarball')
    args = parser.parse_args()

    API_ROOT += args.app

    print 'Getting source...'
    source = get_sources()

    print 'Uploading tarball...'
    upload_tarball(source, args.tar)

    print 'Building...'
    do_build(source)

    print 'Open: http://%s.herokuapp.com' % (args.app)

if __name__ == '__main__':
    main()
