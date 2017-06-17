# coding=utf-8
# pylint: disable=C0103,C0301,C0326,R0914
"""Utils"""

import base64
import datetime
import hmac
import json
import os
import re
import StringIO
import struct
import tarfile
import tempfile
import time
from collections import OrderedDict
from hashlib import sha1
from cerberus import Validator
import pyotp
import semver
from flask import abort, g, make_response, request
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from ..config.config import config as CONFIG
from ..models.base import Base
from ..models.libs import Lib
from ..models.users import User

#----------------#
# Constants

BASE    = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))
DB_PATH = os.path.join(BASE, CONFIG.get('registry', 'db'))

#----------------#
# Variables

db = create_engine('sqlite:///%s' % (DB_PATH))
Base.metadata.create_all(db)
Session = sessionmaker(bind=db)

#----------------#
# Error Functions

def bad_request_custom(message=False):
    """Bad Request custom error handler"""

    return registry_response({
        'status': 400,
        'error': 'Bad Request' if not message else message
    }, status=400, mimetype='application/json')

def unauthorized_custom(message=False):
    """Unauthorized custom error handler"""

    return registry_response({
        'status': 401,
        'error': 'Unauthorized' if not message else message
    }, status=401, mimetype='application/json')

def not_found_custom(message=False):
    """Not Found custom error handler"""

    return registry_response({
        'status': 404,
        'error': 'Not Found' if not message else message
    }, status=404, mimetype='application/json')

def method_not_allowed_custom(message=False):
    """Method Not Allowed custom error handler"""

    return registry_response({
        'status': 405,
        'error': 'Method Not Allowed' if not message else message
    }, status=405, mimetype='application/json')

def conflict_custom(message=False):
    """Conflict custom error handler"""

    return registry_response({
        'status': 409,
        'error': 'Conflict' if not message else message
    }, status=409, mimetype='application/json')

def unprocessable_custom(message=False):
    """Unprocessable custom error handler"""

    return registry_response({
        'status': 422,
        'error': 'Unprocessable Entity' if not message else message
    }, status=422, mimetype='application/json')

def too_many_custom(message=False):
    """Too Many Requests custom error handler"""

    return registry_response({
        'status': 429,
        'error': 'Too Many Requests' if not message else message
    }, status=429, mimetype='application/json')

def internal_server_error_custom(message=False):
    """Internal Server Error custom error handler"""

    return registry_response({
        'status': 500,
        'error': 'Internal Server Error' if not message else message
    }, status=500, mimetype='appliction/json')

#----------------#
# Functions

def get_auth(headers, method, route, token_type):
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
                    .filter(User.username == auth_username) \
                    .all()

    if not result:
        return False

    user = result[0]

    if not user:
        return False

    # check if hash matches

    token = user[token_type]

    if not token:
        return False

    hashed = hmac.new(str(token), '%s+%s' % (method, route), sha1)

    if not hmac.compare_digest(hashed.hexdigest(), auth_hash):
        totp = pyotp.TOTP(base64.b32encode(token))
        hashed = hmac.new(str(totp.now()), '%s+%s' % (method, route), sha1)

        if not hmac.compare_digest(hashed.hexdigest(), auth_hash):
            return False

    # check if login token has expired

    diff = datetime.datetime.now() - user['date_%s' % (token_type.replace('_token', ''))]

    if diff.days >= 1:
        update_obj = {}
        update_obj[token_type] = None

        session.query(User) \
               .filter(User.id == user.id) \
               .update(update_obj)
        session.commit()

        return False

    return user.id

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
        ('author', user.username),
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

def make_cache_key(*_args, **_kwargs):
    """Make cache key"""

    method = request.method
    path = request.path
    args = []

    for arg in request.args.lists():
        key, val = arg

        args.append('%s=%s' % (key, ''.join(val)))

    key = ("%s+%s?%s" % (method, path, '&'.join(args))).encode('utf-8')

    return sha1(key)

def missing_fields(data, fields, field_name='field'):
    """Check for missing fields"""

    missing = []

    if not data:
        missing = fields
    else:
        for field in fields:
            if not field in data or not data[field]:
                missing.append(field)

    if not missing:
        return False

    return bad_request_custom('Missing %s: `%s`' % (field_name if len(missing) == 1 else ('%ss' % field_name), '`, `'.join(missing)))

def parse_accept(accept_header=None, default_accept=None):
    """Parse accept header"""

    if not default_accept:
        default_accept = ['application/json']

    accept = default_accept[0]
    version = 1

    if not accept_header:
        return accept, version

    parts = accept_header.split(';')

    # accept

    accept = str(parts[0]).split(',')[0].strip()

    if not accept in default_accept:
        accept = default_accept[0]

    # version

    version_parts = ['version', '1']

    for part in parts[1:]:
        if 'version' in part.lower():
            version_parts = part.split('=')

    if len(version_parts) > 1:
        version = int(version_parts[1])

    return accept, version

def registry_response(data, status=200, mimetype='application/json', headers=None):
    """Registry response"""

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

    return response

def validate_email(value):
    """Validate email string"""

    if not re.match(r'^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$', value):
        return False

    return True

def validate_semver(field, value, error):
    """Validate semver string"""

    try:
        semver.parse(value)
    except ValueError:
        error(field, 'Must be a valid semver string')

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
            'type': 'string',
            'empty': False
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
