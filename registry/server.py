# coding=utf-8
# pylint: disable=C0103,C0301,C0326,R0914
"""Nessemble registry server"""

import datetime
import json
import md5
import os
import random
import sqlite3
import StringIO
import tarfile
import tempfile
import time
import argparse
from flask import Flask, abort, g, make_response, request
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from models.base import Base
from models.libs import Lib
from models.users import User

#----------------#
# Constants

HOSTNAME = '0.0.0.0'
PORT     = 5000

#----------------#
# Variables

app = Flask(__name__)

#----------------#
# Databases

db = create_engine('sqlite:///registry.db')
Base.metadata.create_all(db)
Session = sessionmaker(bind=db)

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

    session = Session()

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

def get_package_json(package='', full=True, string=False):
    """Get package JSON"""

    session = Session()

    result = session.query(Lib, User) \
                .filter(Lib.name == package) \
                .filter(Lib.user_id == User.id) \
                .all()

    if not len(result):
        return False

    lib, user = result[0]

    output = {
        'name': lib.name,
        'description': lib.description,
        'version': lib.version,
        'author': {
            'name': user.name,
            'email': user.email
        },
        'license': lib.license,
        'tags': lib.tags.split(',')
    }

    if full:
        output['readme'] = '%spackage/%s/README' % (request.url_root, package)
        output['resource'] = '%spackage/%s/data' % (request.url_root, package)

    if string:
        return json.dumps(output, indent=4)

    return output

def get_package_readme(package=''):
    """Get package README"""

    session = Session()

    result = session.query(Lib, User) \
                .filter(Lib.name == package) \
                .filter(Lib.user_id == User.id) \
                .all()

    if not len(result):
        return False

    lib, user = result[0]

    output = lib.readme.replace('\\n', '\n')

    return output

def get_package_zip(package=''):
    """Get package zip"""

    temp = tempfile.NamedTemporaryFile()
    tar = tarfile.open(temp.name, 'w:gz')
    session = Session()

    result = session.query(Lib, User) \
                .filter(Lib.name == package) \
                .filter(Lib.user_id == User.id) \
                .all()

    if not len(result):
        return False

    lib, user = result[0]

    string_lib = StringIO.StringIO()
    string_lib.write(lib.lib.replace('\\n', '\n'))
    string_lib.seek(0)
    string_lib_info = tarfile.TarInfo(name='lib.asm')
    string_lib_info.size = len(string_lib.buf)
    tar.addfile(fileobj=string_lib, tarinfo=string_lib_info)

    string_json = StringIO.StringIO()
    string_json.write(get_package_json(package, full=False, string=True))
    string_json.seek(0)
    string_json_info = tarfile.TarInfo(name='package.json')
    string_json_info.size = len(string_json.buf)
    tar.addfile(fileobj=string_json, tarinfo=string_json_info)

    string_readme = StringIO.StringIO()
    string_readme.write(lib.readme.replace('\\n', '\n'))
    string_readme.seek(0)
    string_readme_info = tarfile.TarInfo(name='README.md')
    string_readme_info.size = len(string_readme.buf)
    tar.addfile(fileobj=string_readme, tarinfo=string_readme_info)

    tar.close()

    with open(temp.name, 'r') as f:
        data = f.read()

    temp.close()

    return data

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

    session = Session()
    results = {
        'libraries': []
    }

    result = session.query(Lib) \
                .order_by(Lib.name) \
                .all()

    for lib in result:
        results['libraries'].append({
            'name': lib.name,
            'description': lib.description,
            'tags': lib.tags.split(','),
            'url': '%spackage/%s' % (request.url_root, lib.name)
        })

    return registry_response(results)

@app.route('/search/<string:term>', methods=['GET'])
def search_packages(term):
    """Search packages endpoint"""

    term = '%%%s%%' % (term.lower())
    session = Session()
    results = {
        'results': []
    }

    result = session.query(Lib) \
                .filter(Lib.name.like(term)) \
                .order_by(Lib.name) \
                .all()

    for lib in result:
        results['results'].append({
            'name': lib.name,
            'description': lib.description,
            'tags': lib.tags.split(','),
            'url': '%spackage/%s' % (request.url_root, lib.name)
        })

    return registry_response(results)

@app.route('/package/<string:package>', methods=['GET'])
def get_package(package):
    """Get package endpoint"""

    output = get_package_json(package)

    if not output:
        abort(404)

    return registry_response(output)

@app.route('/package/<string:package>/README', methods=['GET'])
def get_readme(package):
    """Get package README endpoint"""

    readme = get_package_readme(package)

    if not readme:
        abort(404)

    return registry_response(readme, mimetype='text/plain')

@app.route('/package/<string:package>/data', methods=['GET'])
def get_gz(package):
    """Get package zip endpoint"""

    data = get_package_zip(package)

    if not data:
        abort(404)

    return registry_response(data, mimetype='application/tar+gzip')

@app.route('/user/create', methods=['POST'])
def user_create():
    """Create new user"""

    user = request.get_json()

    missing = missing_fields(user, ['name', 'email', 'password'])
    if missing:
        return missing

    # start session

    session = Session()

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

    session = Session()

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

    session = Session()

    # check if user exists

    result = session.query(User).filter(User.login_token == token).all()
    user = result[0]

    # log out

    session.query(User).filter(User.id == user.id).update({
        'login_token': None
    })
    session.commit()

    return registry_response({})

@app.route('/reference', methods=['GET'])
@app.route('/reference/<path:path>', methods=['GET'])
def reference(path=''):
    paths = path.split('/')
    output = ''
    sql = []
    sql_and = []
    sql_data = {}

    session = Session()

    if not len(paths[0]):
        result = session.execute('SELECT * FROM reference WHERE parent_id = :parent_id', {
            'parent_id': 0
        })

        for row in result:
            output += '  %s\n' % (row[2])
    else:
        sql.append('SELECT r%d.* FROM reference as r1' % (len(paths) + 1))

        for i in range(0, len(paths)):
            sql.append('INNER JOIN reference AS r%d on r%d.parent_id = r%d.id' % (i + 2, i + 2, i + 1))

        for i in range(0, len(paths)):
            sql_and.append('r%d.term = :term%d' % (i + 1, i + 1))
            sql_data['term%d' % (i + 1)] = paths[i]

        sql.append('WHERE %s' % (' AND '.join(sql_and)))
        result = session.execute(' '.join(sql), sql_data)

        for row in result:
            if not row[2]:
                output = row[3].replace('\\n', '\n')
            else:
                output += '  %s\n' % (row[2])

    if not len(output):
        abort(404)

    return registry_response(output, mimetype='text/plain')

#----------------#
# Import/Export

def db_import(filename=None):
    """Import database"""

    os.unlink('registry.db')

    con = sqlite3.connect('registry.db')

    with open(filename, 'r') as f:
        sql = f.read()
        con.executescript(sql)

def db_export():
    """Export database"""

    con = sqlite3.connect('registry.db')

    with open('registry.sql', 'w') as f:
        for line in con.iterdump():
            f.write('%s\n' % line)

#----------------#
# Main

def main():
    """Main function"""

    parser = argparse.ArgumentParser(description='Nessemble registry')
    parser.add_argument('--host', '-H', dest='host', type=str, default=HOSTNAME, required=False, help='Host')
    parser.add_argument('--port', '-P', dest='port', type=int, default=PORT, required=False, help='Port')
    parser.add_argument('--debug', '-D', dest='debug', action='store_true', required=False, help='Debug mode')
    parser.add_argument('--import', '-i', dest='db_import', type=str, help='Import SQL')
    parser.add_argument('--export', '-e', dest='db_export', action='store_true', help='Export SQL')

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
