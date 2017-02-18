# coding=utf-8
# pylint: disable=C0103,C0301
"""Nessemble registry server"""

import json
import tarfile
import tempfile
import os
import time
import argparse
from flask import Flask, abort, g, make_response, request

#----------------#
# Constants

HOSTNAME = '0.0.0.0'
PORT     = 5000

#----------------#
# Variables

app = Flask(__name__)

#----------------#
# Utilities

def registry_response(data, status=200, mimetype='application/json'):
    """Registry response"""

    if mimetype == 'application/json':
        response = make_response(json.dumps(data, indent=4), status)
    else:
        response = make_response(data, status)

    response.headers.remove('Content-Type')

    response.headers.add('Content-Type', mimetype)
    response.headers.add('Access-Control-Allow-Origin', '*')
    response.headers.add('Server', 'Nessemble')
    response.headers.add('X-Response-Time', g.request_time())

    return response

def get_packages():
    """Get all packages"""

    data = ''
    path = '%s/libs' % (os.path.dirname(os.path.abspath(__file__)))
    result = []
    readme = {}

    for package in os.listdir(path):
        try:
            with open('%s/%s/package.json' % (path, package), 'r') as f:
                data = f.read()
        except IOError:
            continue

        try:
            readme = json.loads(data)
        except ValueError:
            continue

        result.append({
            'name': readme['name'],
            'description': readme['description'],
            'tags': readme['tags'],
            'url': '%s%s.json' % (request.url_root, readme['name'])
        })

    return result

@app.before_request
def before_request():
    """Before request"""

    g.request_start_time = time.time()
    g.request_time = lambda: '%.5s' % (time.time() - g.request_start_time)

#----------------#
# Errors

@app.errorhandler(404)
def not_found(error):
    """Not Found error handler"""

    return registry_response({
        'status': int(str(error)[:3]),
        'error': 'Not Found'
    }, status=404)

@app.errorhandler(500)
def internal_server_error(error):
    """Internal Server Error error handler"""

    return registry_response({
        'status': int(str(error)[:3]),
        'error': 'Internal Server Error'
    }, status=500)

#----------------#
# Endpoints

@app.route('/', methods=['GET'])
def list_packages():
    """List packages endpoint"""

    result = {
        'libraries': get_packages()
    }

    return registry_response(result)

@app.route('/search/<string:term>', methods=['GET'])
def search_packages(term):
    """Search packages endpoint"""

    packages = get_packages()
    results = {
        'results': []
    }

    term = term.lower()

    for package in packages:
        match_name = package['name'].lower().find(term) != -1
        match_description = package['description'].lower().find(term) != -1
        match_tags = any(tag.lower().find(term) != -1 for tag in package['tags'])

        if match_name or match_description or match_tags:
            results['results'].append(package)

    return registry_response(results)

@app.route('/<string:package>.json', methods=['GET'])
def get_package(package):
    """Get package endpoint"""

    data = ''
    path = '%s/libs/%s/' % (os.path.dirname(os.path.abspath(__file__)), package)
    result = {}

    # get package.json
    try:
        with open('%s/package.json' % (path), 'r') as f:
            data = f.read()
    except IOError:
        abort(404)

    # parse package.json
    try:
        result = json.loads(data)
    except ValueError:
        abort(500)

    # include README.md
    try:
        with open('%s/README.md' % (path), 'r') as f:
            result['readme'] = f.read()
    except IOError:
        pass

    result["resource"] = "%s%s.tar.gz" % (request.url_root, package)

    return registry_response(result)

@app.route('/<string:package>.tar.gz', methods=['GET'])
def get_gz(package):
    """Get package zip endpoint"""

    data = ''
    path = '%s/libs/%s/' % (os.path.dirname(os.path.abspath(__file__)), package)
    temp = tempfile.NamedTemporaryFile()

    tar = tarfile.open(temp.name, 'w:gz')

    for filename in ['lib.asm', 'package.json', 'README.md']:
        tar.add("%s/%s" % (path, filename), arcname=filename)

    tar.close()

    with open(temp.name, 'r') as f:
        data = f.read()

    temp.close()

    return registry_response(data, mimetype='application/tar+gzip')

#----------------#
# Main

def main():
    """Main function"""

    parser = argparse.ArgumentParser(description='Nessemble registry')
    parser.add_argument('--host', '-H', dest='host', type=str, default=HOSTNAME, required=False, help='Host')
    parser.add_argument('--port', '-P', dest='port', type=int, default=PORT, required=False, help='Port')
    parser.add_argument('--debug', '-D', dest='debug', action='store_true', required=False, help='Debug mode')

    args = parser.parse_args()

    app.run(host=args.host, port=args.port, debug=args.debug)

if __name__ == '__main__':
    main()
