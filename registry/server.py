# coding=utf-8
# pylint: disable=C0103,C0301,C0326
"""Nessemble registry server"""

import json
import tarfile
import tempfile
import os
import time
import datetime
import random
import md5
import sqlite3
import csv
import argparse
from flask import Flask, abort, g, make_response, request
from models.users import User, Users_Session

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

def get_auth(headers):
    """Get auth token"""

    if not 'X-Auth-Token' in headers:
        abort(401)

    token = headers['X-Auth-Token']

    if not token:
        abort(401)

    # start session

    session = Users_Session()

    # check if user exists

    result = session.query(User).filter(User.login_token == token).all()
    user = result[0]

    if not user:
        return False

    diff = datetime.datetime.now() - user.date_login

    if diff.days >= 1:
        session.query(User).filter(User.id == user.id).update({
            'login_token': None
        })
        session.commit()

        return False

    return token

def missing_fields(data, fields, field_name='field'):
    """Check for missing fields"""

    missing = []

    if not data:
        missing = fields
    else:
        for field in fields:
            if not field in data:
                missing.append(field)

    if len(missing) == 0:
        return False

    return registry_response({
        'status': 400,
        'error': 'Missing %s: `%s`' % (field_name if len(missing) == 1 else ('%ss' % field_name), '`, `'.join(missing))
    }, 400)

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

@app.errorhandler(400)
def bad_request(error):
    """Bad Request error handler"""

    return registry_response({
        'status': int(str(error)[:3]),
        'error': 'Bad Request'
    }, status=400)

def bad_request_custom(message=False):
    """Bad Request custom error handler"""

    return registry_response({
        'status': 400,
        'error': 'Bad Request' if not message else message
    }, status=400)

@app.errorhandler(401)
def unauthorized(error):
    """Unauthorized error handler"""

    return registry_response({
        'status': int(str(error)[:3]),
        'error': 'Unauthorized'
    }, status=401)

def unauthorized_custom(message=False):
    """Unauthorized custom error handler"""

    return registry_response({
        'status': 401,
        'error': 'Unauthorized' if not message else message
    }, status=401)

@app.errorhandler(404)
def not_found(error):
    """Not Found error handler"""

    return registry_response({
        'status': int(str(error)[:3]),
        'error': 'Not Found'
    }, status=404)

@app.errorhandler(409)
def conflict(error):
    """Conflict error handler"""

    return registry_response({
        'status': int(str(error)[:3]),
        'error': 'Conflict'
    }, status=409)

def conflict_custom(message=False):
    """Conflict custom error handler"""

    return registry_response({
        'status': 409,
        'error': 'Conflict' if not message else message
    }, status=409)

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

    result["readme"] = "%s%s.md" % (request.url_root, package)
    result["resource"] = "%s%s.tar.gz" % (request.url_root, package)

    return registry_response(result)

@app.route('/<string:package>.md', methods=['GET'])
def get_readme(package):
    """Get package README endpoint"""

    readme = ''
    path = '%s/libs/%s/' % (os.path.dirname(os.path.abspath(__file__)), package)

    try:
        with open('%s/README.md' % (path), 'r') as f:
            readme = f.read()
    except IOError:
        abort(404)

    return registry_response(readme, mimetype='text/plain')

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

@app.route('/user/create', methods=['POST'])
def user_create():
    """Create new user"""

    user = request.get_json()

    missing = missing_fields(user, ['name', 'email', 'password'])
    if missing:
        return missing

    # start session

    session = Users_Session()

    # check if user exists

    result = session.query(User).filter(User.email == user['email']).all()

    if len(result):
        return conflict_custom('User already exists')

    # create user

    session.add(User(
        name=user['name'],
        email=user['email'],
        password=md5.new(user['password']).hexdigest(),
        date_created=datetime.datetime.now()))
    session.commit()

    return registry_response({})

@app.route('/user/login', methods=['POST'])
def user_login():
    """User login"""

    auth = request.authorization

    missing = missing_fields(auth, ['username', 'password'], 'authorization')
    if missing:
        return missing

    # start session

    session = Users_Session()

    # check if user exists

    result = session.query(User).filter(User.email == auth['username']).all()

    if not len(result):
        return unauthorized_custom('User does not exist')

    user = result[0]

    if user.password != md5.new(auth['password']).hexdigest():
        return unauthorized_custom('Incorrect password')

    # create login token

    login_token = '%X' % (random.getrandbits(128))

    session.query(User).filter(User.id == user.id).update({
        'date_login': datetime.datetime.now(),
        'login_token': login_token
    })
    session.commit()

    return registry_response({
        'token': login_token
    })

@app.route('/user/logout', methods=['GET', 'POST'])
def user_logout():
    """User logout"""

    token = get_auth(request.headers)

    if not token:
        return unauthorized_custom('User is not logged in')

    # start session

    session = Users_Session()

    # check if user exists

    result = session.query(User).filter(User.login_token == token).all()
    user = result[0]

    # log out

    session.query(User).filter(User.id == user.id).update({
        'login_token': None
    })
    session.commit()

    return registry_response({})

#----------------#
# Import/Export

def db_import(filename=None):
    """Import tables"""

    if not filename:
        return

    basename = os.path.splitext(filename)[0]
    tablename = '%s.db' % (basename)

    os.remove(tablename)

    infile = open(filename, 'rb')
    incsv = csv.reader(infile)

    fields = next(incsv)

    print fields

    for row in incsv:
        print row

def db_export():
    """Export tables"""

    con = sqlite3.connect('users.db')
    outfile = open('users.csv', 'wb')
    outcsv = csv.writer(outfile)

    cursor = con.execute('select * from users')

    outcsv.writerow([x[0] for x in cursor.description])
    outcsv.writerows(cursor.fetchall())

    outfile.close()

#----------------#
# Main

def main():
    """Main function"""

    parser = argparse.ArgumentParser(description='Nessemble registry')
    parser.add_argument('--host', '-H', dest='host', type=str, default=HOSTNAME, required=False, help='Host')
    parser.add_argument('--port', '-P', dest='port', type=int, default=PORT, required=False, help='Port')
    parser.add_argument('--debug', '-D', dest='debug', action='store_true', required=False, help='Debug mode')
    parser.add_argument('--import', '-i', dest='db_import', type=str, help='Import CSV')
    parser.add_argument('--export', '-e', dest='db_export', action='store_true', help='Export CSV')

    args = parser.parse_args()

    if args.db_import:
        db_import(args.db_import)
        return

    if args.db_export:
        db_export()
        return

    app.run(host=args.host, port=args.port, debug=args.debug)

if __name__ == '__main__':
    main()
