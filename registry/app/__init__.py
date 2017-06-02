#!/usr/bin/env python
# coding=utf-8
# pylint: disable=C0103,C0301,R0911,R0912,R0914,R0915,W0603
"""Nessemble registry server"""

import base64
import datetime
import hmac
import json
import md5
import os
import random
import re
import sqlite3
import StringIO
import struct
import tarfile
import tempfile
import time
from collections import OrderedDict
from ConfigParser import ConfigParser
from hashlib import sha1
import semver
from cerberus import Validator
from flask import Flask, abort, g, make_response, request
from flask_caching import Cache
from flask_limiter import Limiter
from flask_limiter.util import get_remote_address
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from ..models.base import Base
from ..models.libs import Lib
from ..models.users import User

#----------------#
# Constants

BASE       = os.path.normpath(os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', '..'))
ROOT       = os.path.normpath(os.path.join(BASE, 'registry'))

CONFIG     = ConfigParser()
CONFIG.readfp(open(os.path.join(BASE, 'settings.cfg')))

DB_PATH    = os.path.join(ROOT, CONFIG.get('registry', 'db'))
SQL_PATH   = os.path.join(ROOT, CONFIG.get('registry', 'sql'))
CACHE_TIME = CONFIG.getint('registry', 'cache_time')

#----------------#
# Variables

app = Flask(__name__)
cache = Cache(app, config={
    'CACHE_TYPE': 'simple',
    'CACHE_THRESHOLD': 256
})
limiter = Limiter(
    app,
    key_func=get_remote_address,
    default_limits=CONFIG.get('registry', 'limits').split(','),
    headers_enabled=True
)
abort_mimetype = 'application/json'

#----------------#
# Databases

db = create_engine('sqlite:///%s' % (DB_PATH))
Base.metadata.create_all(db)
Session = sessionmaker(bind=db)

#----------------#
# Utilities

def registry_response(data, status=200, mimetype='application/json', headers=None):
    """Registry response"""

    global abort_mimetype

    if mimetype == 'application/json':
        response = make_response(json.dumps(data, indent=4), status)
    else:
        if isinstance(data, dict):
            data = json.dumps(data, indent=4)

        response = make_response(data, status)

    response.headers.remove('Content-Type')

    response.headers.add('Content-Type', mimetype)
    response.headers.add('Access-Control-Allow-Origin', '*')
    response.headers.add('Server', 'Nessemble')

    try:
        response.headers.add('X-Response-Time', g.request_time())
    except AttributeError:
        response.headers.add('X-Response-Time', 0)

    if headers:
        for header in headers:
            response.headers.add(header[0], header[1])

    abort_mimetype = 'application/json'

    return response

def get_auth(headers, method, route):
    """Get user_id if authorized"""

    if not 'Authorization' in headers:
        abort(401)

    authorization = headers['Authorization']

    if not authorization:
        abort(401)

    # parse authorization

    try:
        auth_type, auth_credentials_base64 = authorization.split(' ')
    except ValueError:
        abort(401)

    if auth_type != 'HMAC-SHA1' or not auth_credentials_base64:
        abort(401)

    auth_credentials = base64.b64decode(auth_credentials_base64)

    try:
        auth_username, auth_hash = auth_credentials.split(':')
    except ValueError:
        abort(401)

    # start session

    session = Session()

    # check if user exists

    result = session.query(User) \
                    .filter(User.email == auth_username) \
                    .all()

    if not result:
        return False

    user = result[0]

    if not user:
        return False

    # check if hash matches

    hashed = hmac.new(str(user.login_token), '%s+%s' % (method, route), sha1)

    if not hmac.compare_digest(hashed.hexdigest(), auth_hash):
        return False

    # check if login token has expired

    diff = datetime.datetime.now() - user.date_login

    if diff.days >= 1:
        session.query(User) \
               .filter(User.id == user.id) \
               .update({
                   'login_token': None
               })
        session.commit()

        return False

    return user.id

def missing_fields(data, fields, field_name='field'):
    """Check for missing fields"""

    missing = []

    if not data:
        missing = fields
    else:
        for field in fields:
            if not field in data:
                missing.append(field)

    if not missing:
        return False

    return registry_response({
        'status': 400,
        'error': 'Missing %s: `%s`' % (field_name if len(missing) == 1 else ('%ss' % field_name), '`, `'.join(missing))
    }, 400)

def get_package_json(package='', version=None, full=True, string=False):
    """Get package JSON"""

    session = Session()

    result = session.query(Lib, User) \
                    .group_by(Lib.title) \
                    .order_by(Lib.title, Lib.id) \
                    .filter(Lib.title == package) \
                    .filter(Lib.user_id == User.id)

    if version:
        result = result.filter(Lib.version == version)

    result = result.all()

    if not result:
        return False

    lib, user = result[0]

    output = OrderedDict([
        ('title', lib.title),
        ('description', lib.description),
        ('version', lib.version)
    ])

    if full:
        versions = OrderedDict()
        results = session.query(Lib, User) \
                         .order_by(Lib.title, Lib.id.desc()) \
                         .filter(Lib.title == package) \
                         .filter(Lib.user_id == User.id) \
                         .all()

        for result in results:
            version_lib = result[0]
            versions.update([
                (version_lib.version, '%sZ' % (version_lib.date.strftime('%Y-%m-%dT%H:%M:%S.%f')[:-3].ljust(23, '0')))
            ])

        output.update([
            ('versions', versions)
        ])

    output.update([
        ('author', {
            'name': user.name,
            'email': user.email
        }),
        ('license', lib.license),
        ('tags', lib.tags.split(','))
    ])

    if full:
        package_version = package

        if version:
            package_version = '%s/%s' % (package, version)

        output.update([
            ('date', '%sZ' % (lib.date.strftime('%Y-%m-%dT%H:%M:%S.%f')[:-3].ljust(23, '0'))),
            ('readme', '%spackage/%s/README' % (request.url_root, package_version)),
            ('resource', '%spackage/%s/data' % (request.url_root, package_version)),
            ('shasum', lib.shasum)
        ])

    if string:
        return json.dumps(output, indent=4)

    return output

def get_package_readme(package='', version=None):
    """Get package README"""

    session = Session()

    result = session.query(Lib, User) \
                    .group_by(Lib.title) \
                    .order_by(Lib.title, Lib.id) \
                    .filter(Lib.title == package) \
                    .filter(Lib.user_id == User.id)

    if version:
        result = result.filter(Lib.version == version)

    result = result.all()

    if not result:
        return False

    lib, _user = result[0]

    output = lib.readme.replace('\\n', '\n')

    return output

def get_package_zip(package='', version=None):
    """Get package zip"""

    session = Session()

    result = session.query(Lib, User) \
                    .group_by(Lib.title) \
                    .order_by(Lib.title, Lib.id) \
                    .filter(Lib.title == package) \
                    .filter(Lib.user_id == User.id)

    if version:
        result = result.filter(Lib.version == version)

    result = result.all()

    if not result:
        return False

    lib, _user = result[0]

    temp_name = '%s%s%s.tar.gz' % (tempfile.gettempdir(), os.sep, package)
    temp_time = time.mktime(lib.date.timetuple())

    tar = tarfile.open(temp_name, 'w:gz')

    string_lib = StringIO.StringIO()
    string_lib.write(lib.lib.replace('\\n', '\n'))
    string_lib.seek(0)
    string_lib_info = tarfile.TarInfo(name='lib.asm')
    string_lib_info.size = len(string_lib.buf)
    string_lib_info.mtime = temp_time
    tar.addfile(fileobj=string_lib, tarinfo=string_lib_info)

    string_json = StringIO.StringIO()
    string_json.write(get_package_json(package, version, full=False, string=True))
    string_json.seek(0)
    string_json_info = tarfile.TarInfo(name='package.json')
    string_json_info.size = len(string_json.buf)
    string_json_info.mtime = temp_time
    tar.addfile(fileobj=string_json, tarinfo=string_json_info)

    string_readme = StringIO.StringIO()
    string_readme.write(lib.readme.replace('\\n', '\n'))
    string_readme.seek(0)
    string_readme_info = tarfile.TarInfo(name='README.md')
    string_readme_info.size = len(string_readme.buf)
    string_readme_info.mtime = temp_time
    tar.addfile(fileobj=string_readme, tarinfo=string_readme_info)

    tar.close()

    with open(temp_name, 'r') as f:
        data = f.read()

    time_str = struct.pack("<L", long(temp_time))

    for i in range(0, 4):
        if ord(time_str[i]) == 0x00:
            time_str = time_str[:i] + chr(0x01) + time_str[i+1:]

    output = '%s%s%s' % (data[0:4], time_str, data[8:])

    return output

def validate_semver(field, value, error):
    """Validate semver string"""

    try:
        semver.parse(value)
    except ValueError:
        error(field, 'Must be a valid semver string')

def validate_email(field, value, error):
    """Validate email string"""

    if not re.match(r'^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$', value):
        error(field, 'Must be a valid email address')

def validate_package(data):
    """Validate package.json"""

    try:
        package = json.loads(data)
    except ValueError:
        raise ValueError('Invalid package.json')

    validator = Validator({
        'title': {
            'type': 'string',
            'empty': False,
            'forbidden': ['publish']
        },
        'description': {
            'type': 'string',
            'empty': False
        },
        'version': {
            'type': 'string',
            'empty': False,
            'validator': validate_semver
        },
        'license': {
            'type': 'string',
            'empty': False
        },
        'author': {
            'type': 'dict',
            'schema': {
                'name': {
                    'type': 'string',
                    'empty': False
                },
                'email': {
                    'type': 'string',
                    'empty': False,
                    'validator': validate_email
                }
            }
        },
        'tags': {
            'type': 'list',
            'empty': False
        }
    })

    if not validator.validate(package):
        for key, value in validator.errors.iteritems():
            if isinstance(value[0], dict):
                for key2, value2 in value[0].iteritems():
                    raise ValueError('package.json field `%s.%s` %s' % (key, key2, value2[0].lower()))
            else:
                raise ValueError('package.json field `%s` %s' % (key, value[0].lower()))

    return package

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
    }, status=400, mimetype=abort_mimetype)

def bad_request_custom(message=False):
    """Bad Request custom error handler"""

    return registry_response({
        'status': 400,
        'error': 'Bad Request' if not message else message
    }, status=400, mimetype=abort_mimetype)

@app.errorhandler(401)
def unauthorized(error):
    """Unauthorized error handler"""

    return registry_response({
        'status': int(str(error)[:3]),
        'error': 'Unauthorized'
    }, status=401, mimetype=abort_mimetype)

def unauthorized_custom(message=False):
    """Unauthorized custom error handler"""

    return registry_response({
        'status': 401,
        'error': 'Unauthorized' if not message else message
    }, status=401, mimetype=abort_mimetype)

@app.errorhandler(404)
def not_found(error):
    """Not Found error handler"""

    return registry_response({
        'status': int(str(error)[:3]),
        'error': 'Not Found'
    }, status=404, mimetype=abort_mimetype)

@app.errorhandler(409)
def conflict(error):
    """Conflict error handler"""

    return registry_response({
        'status': int(str(error)[:3]),
        'error': 'Conflict'
    }, status=409, mimetype=abort_mimetype)

def conflict_custom(message=False):
    """Conflict custom error handler"""

    return registry_response({
        'status': 409,
        'error': 'Conflict' if not message else message
    }, status=409, mimetype=abort_mimetype)

@app.errorhandler(422)
def unprocessable(error):
    """Unprocessable error handler"""

    return registry_response({
        'status': int(str(error)[:3]),
        'error': 'Unprocessable Entity'
    }, status=422, mimetype=abort_mimetype)

def unprocessable_custom(message=False):
    """Unprocessable custom error handler"""

    return registry_response({
        'status': 422,
        'error': 'Unprocessable Entity' if not message else message
    }, status=422, mimetype=abort_mimetype)

@app.errorhandler(429)
def too_many(error):
    """Too many requests error handler"""

    return registry_response({
        'status': int(str(error)[:3]),
        'error': 'Too Many Requests'
    }, status=429, mimetype=abort_mimetype)

@app.errorhandler(500)
def internal_server_error(error):
    """Internal Server Error error handler"""

    return registry_response({
        'status': int(str(error)[:3]),
        'error': 'Internal Server Error'
    }, status=500, mimetype=abort_mimetype)

#----------------#
# Endpoints

@app.route('/', methods=['GET'])
@cache.cached(timeout=CACHE_TIME)
def list_packages():
    """List packages endpoint"""

    session = Session()
    results = OrderedDict([
        ('packages', [])
    ])

    result = session.query(Lib) \
                    .group_by(Lib.title) \
                    .order_by(Lib.title, Lib.id) \
                    .all()

    for lib in result:
        results['packages'].append(OrderedDict([
            ('title', lib.title),
            ('description', lib.description),
            ('tags', lib.tags.split(',')),
            ('url', '%spackage/%s' % (request.url_root, lib.title))
        ]))

    return registry_response(results)

@app.route('/search/<string:term>', methods=['GET'])
@cache.cached(timeout=CACHE_TIME)
def search_packages(term):
    """Search packages endpoint"""

    session = Session()
    results = OrderedDict([
        ('term', term),
        ('results', [])
    ])

    term = '%%%s%%' % (term.lower())

    result = session.query(Lib) \
                    .group_by(Lib.title) \
                    .order_by(Lib.title, Lib.id) \
                    .filter(Lib.title.like(term)) \
                    .all()

    result.extend(session.query(Lib) \
                         .group_by(Lib.title) \
                         .order_by(Lib.title, Lib.id) \
                         .filter(Lib.tags.like(term)) \
                         .all())

    # get unique results

    result = list(OrderedDict.fromkeys(result))

    # sort results

    result = sorted(result, key=lambda res: res.title)

    for lib in result:
        results['results'].append(OrderedDict([
            ('title', lib.title),
            ('description', lib.description),
            ('tags', lib.tags.split(',')),
            ('url', '%spackage/%s' % (request.url_root, lib.title))
        ]))

    return registry_response(results)

@app.route('/package/<string:package>', methods=['GET'])
@cache.cached(timeout=CACHE_TIME)
def get_package(package):
    """Get package endpoint"""

    output = get_package_json(package)

    if not output:
        abort(404)

    return registry_response(output)

@app.route('/package/<string:package>/<string:version>', methods=['GET'])
@cache.cached(timeout=CACHE_TIME)
def get_package_version(package, version):
    """Get package endpoint by version"""

    output = get_package_json(package, version)

    if not output:
        abort(404)

    return registry_response(output)

@app.route('/package/<string:package>/README', methods=['GET'])
@cache.cached(timeout=CACHE_TIME)
def get_readme(package):
    """Get package README endpoint"""

    readme = get_package_readme(package)

    if not readme:
        abort(404)

    return registry_response(readme, mimetype='text/plain')

@app.route('/package/<string:package>/<string:version>/README', methods=['GET'])
@cache.cached(timeout=CACHE_TIME)
def get_readme_version(package, version):
    """Get package README endpoint by version"""

    readme = get_package_readme(package, version)

    if not readme:
        abort(404)

    return registry_response(readme, mimetype='text/plain')

@app.route('/package/<string:package>/data', methods=['GET'])
@cache.cached(timeout=CACHE_TIME)
def get_gz(package):
    """Get package zip endpoint"""

    data = get_package_zip(package)

    if not data:
        abort(404)

    lib_data = get_package_json(package)

    headers = [
        ('Content-Disposition', 'attachment; filename="%s-%s.tar.gz"' % (lib_data['title'], lib_data['version'])),
        ('X-Integrity', lib_data['shasum'])
    ]

    return registry_response(data, mimetype='application/tar+gzip', headers=headers)

@app.route('/package/<string:package>/<string:version>/data', methods=['GET'])
@cache.cached(timeout=CACHE_TIME)
def get_gz_version(package, version):
    """Get package zip endpoint by version"""

    data = get_package_zip(package, version)

    if not data:
        abort(404)

    lib_data = get_package_json(package)

    headers = [
        ('Content-Disposition', 'attachment; filename="%s-%s.tar.gz"' % (lib_data['title'], lib_data['version'])),
        ('X-Integrity', lib_data['shasum'])
    ]

    return registry_response(data, mimetype='application/tar+gzip', headers=headers)

@app.route('/package/publish', methods=['POST'])
def post_gz():
    """Post package zip endpoint"""

    global abort_mimetype

    abort_mimetype = 'application/tar+gzip'

    # get auth

    user_id = get_auth(request.headers, request.method, request.url_rule)

    if not user_id:
        return unauthorized_custom('User is not logged in')

    # get zip data

    package_zip = request.data

    if not package_zip:
        abort(404)

    # open zip file

    temp = tempfile.NamedTemporaryFile()
    with open(temp.name, 'w') as f:
        f.write(package_zip)

    try:
        tar = tarfile.open(temp.name, 'r:gz')
    except TypeError:
        return unprocessable_custom('Invalid tarball')

    # check that all files are included

    file_lib = None
    file_readme = None
    file_info = None

    for member in tar.getmembers():
        if member.isfile():
            if member.name[-7:] == 'lib.asm':
                file_lib = tar.extractfile(member)

            if member.name[-9:] == 'README.md':
                file_readme = tar.extractfile(member)

            if member.name[-12:] == 'package.json':
                file_info = tar.extractfile(member)

    if not file_lib or not file_readme or not file_info:
        missing = []

        if not file_lib:
            missing.append('lib.asm')

        if not file_readme:
            missing.append('README.md')

        if not file_info:
            missing.append('package.json')

        return unprocessable_custom('Missing: ' + ', '.join(missing))

    # read files

    data_lib = file_lib.read()
    data_readme = file_readme.read()
    data_info = file_info.read()
    json_info = None

    # validate JSON

    try:
        json_info = validate_package(data_info)
    except ValueError as error:
        return unprocessable_custom(error[0])

    session = Session()

    # check if user is logged in

    result = session.query(User) \
                    .filter(User.id == user_id) \
                    .all()
    user = result[0]

    if not user:
        return unauthorized_custom('User is not logged in')

    # make sure emails match

    if user.email != json_info['author']['email']:
        return unprocessable_custom('Author email mismatch')

    # check that lib doesn't already exist

    result = session.query(Lib) \
                    .group_by(Lib.title) \
                    .order_by(Lib.title, Lib.id) \
                    .filter(Lib.title == json_info['title']) \
                    .all()

    if result:
        lib = result[0]

        if lib.user_id != user.id:
            return unprocessable_custom('Package `%s` already exists' % (json_info['title']))

        if semver.compare(json_info['version'], lib.version) != 1:
            return unprocessable_custom('Cannot upgrade to version %s (currently at %s)' % (json_info['version'], lib.version))

    # add new lib

    lib = Lib(
        user_id=user.id,
        readme=data_readme,
        lib=data_lib,
        title=json_info['title'],
        description=json_info['description'],
        version=json_info['version'],
        license=json_info['license'],
        tags=','.join(json_info['tags']),
        date=datetime.datetime.now(),
        shasum=''
    )

    session.add(lib)
    session.commit()

    # update shasum

    package_data = get_package_zip(lib.title)

    session.query(Lib) \
           .filter(Lib.id == lib.id) \
           .update({
               'shasum': sha1(package_data).hexdigest()
           })
    session.commit()

    return registry_response(json_info, mimetype='application/tar+gzip')

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

    result = session.query(User) \
                    .filter(User.email == user['email']) \
                    .all()

    if result:
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

    result = session.query(User) \
                    .filter(User.email == auth['username']) \
                    .all()

    if not result:
        return unauthorized_custom('User does not exist')

    user = result[0]

    if user.password != md5.new(auth['password']).hexdigest():
        return unauthorized_custom('Incorrect password')

    # create login token

    login_token = '%X' % (random.getrandbits(128))

    session.query(User) \
           .filter(User.id == user.id) \
           .update({
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

    # get auth

    user_id = get_auth(request.headers, request.method, request.url_rule)

    if not user_id:
        return unauthorized_custom('User is not logged in')

    # start session

    session = Session()

    # check if user exists

    result = session.query(User) \
                    .filter(User.id == user_id) \
                    .all()
    user = result[0]

    if not user:
        return unauthorized_custom('User does not exist')

    # log out

    session.query(User) \
           .filter(User.id == user_id) \
           .update({
               'login_token': None
           })
    session.commit()

    return registry_response({})

@app.route('/reference', methods=['GET'])
@app.route('/reference/<path:path>', methods=['GET'])
@cache.cached(timeout=CACHE_TIME)
def reference(path=''):
    """Reference endpoint"""

    global abort_mimetype

    paths = path.split('/')
    output = ''
    sql = []
    sql_and = []
    sql_data = {}

    session = Session()

    if not paths[0]:
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

    if not output:
        abort_mimetype = 'text/plain'
        abort(404)

    return registry_response(output, mimetype='text/plain')

#----------------#
# Import/Export

def db_import(filename=None):
    """Import database"""

    os.unlink(DB_PATH)

    con = sqlite3.connect(DB_PATH)

    with open(filename, 'r') as f:
        sql = f.read()
        con.executescript(sql)

    # update shasums

    session = Session()

    results = session.query(Lib) \
                    .order_by(Lib.title, Lib.id) \
                    .all()

    for result in results:
        data = get_package_zip(result.title, result.version)

        session.query(Lib) \
               .filter(Lib.id == result.id) \
               .update({
                   'shasum': sha1(data).hexdigest()
               })
        session.commit()

def db_export():
    """Export database"""

    con = sqlite3.connect(DB_PATH)

    with open(SQL_PATH, 'w') as f:
        for line in con.iterdump():
            f.write('%s\n' % line)

#--------------#
# Variables

__all__ = ['app', 'db_import', 'db_export']
