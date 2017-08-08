# coding=utf-8
# pylint: disable=C0103,C0301,R0911,R0912,R0914,R0915
"""Package routes"""

import datetime
import tarfile
import tempfile
from hashlib import sha1
from flask import abort, Blueprint, request
import semver
from ..config.cache import cache, cache_headers
from ..config.config import config as CONFIG
from ..models.libs import Lib
from ..models.users import User
from ..utils.utils import unauthorized_custom, unprocessable_custom
from ..utils.utils import get_auth, get_package_json, get_package_readme
from ..utils.utils import get_package_zip, parse_accept, registry_response
from ..utils.utils import validate_package
from ..utils.utils import Session
from ..views import package_view

#----------------#
# Constants

CACHE_TIME = CONFIG.getint('registry', 'cache_time')

#----------------#
# Variables

package_endpoint = Blueprint('package_endpoint', __name__)

#----------------#
# Endpoints

@package_endpoint.route('/package/<string:package>', methods=['GET'])
@cache.cached(timeout=CACHE_TIME)
@cache_headers(CACHE_TIME)
def get_package(package):
    """Get package endpoint"""

    accept, _version = parse_accept(request.headers.get('Accept'), ['application/json'])

    output = package_view.package(package)

    return registry_response(output, mimetype=accept)

@package_endpoint.route('/package/<string:package>/<string:version>', methods=['GET'])
@cache.cached(timeout=CACHE_TIME)
@cache_headers(CACHE_TIME)
def get_package_version(package, version):
    """Get package endpoint by version"""

    accept, _version = parse_accept(request.headers.get('Accept'), ['application/json'])

    output = package_view.package(package, version)

    return registry_response(output, mimetype=accept)

@package_endpoint.route('/package/<string:package>/README', methods=['GET'])
@cache.cached(timeout=CACHE_TIME)
@cache_headers(CACHE_TIME)
def get_readme(package):
    """Get package README endpoint"""

    accept, _version = parse_accept(request.headers.get('Accept'), ['text/plain'])

    output = package_view.readme(package)

    return registry_response(output, mimetype=accept)

@package_endpoint.route('/package/<string:package>/<string:version>/README', methods=['GET'])
@cache.cached(timeout=CACHE_TIME)
@cache_headers(CACHE_TIME)
def get_readme_version(package, version):
    """Get package README endpoint by version"""

    accept, _version = parse_accept(request.headers.get('Accept'), ['text/plain'])

    output = package_view.readme(package, version)

    return registry_response(output, mimetype=accept)

@package_endpoint.route('/package/<string:package>/data', methods=['GET'])
@cache.cached(timeout=CACHE_TIME)
@cache_headers(CACHE_TIME)
def get_gz(package):
    """Get package zip endpoint"""

    accept, _version = parse_accept(request.headers.get('Accept'), ['application/tar+gzip'])

    data = package_view.data(package)
    lib_data = get_package_json(package)

    headers = [
        ('Content-Disposition', 'attachment; filename="%s-%s.tar.gz"' % (lib_data['title'], lib_data['version'])),
        ('X-Integrity', lib_data['shasum'])
    ]

    return registry_response(data, mimetype=accept, headers=headers)

@package_endpoint.route('/package/<string:package>/<string:version>/data', methods=['GET'])
@cache.cached(timeout=CACHE_TIME)
@cache_headers(CACHE_TIME)
def get_gz_version(package, version):
    """Get package zip endpoint by version"""

    accept, _version = parse_accept(request.headers.get('Accept'), ['application/tar+gzip'])

    data = package_view.data(package, version)
    lib_data = get_package_json(package)

    headers = [
        ('Content-Disposition', 'attachment; filename="%s-%s.tar.gz"' % (lib_data['title'], lib_data['version'])),
        ('X-Integrity', lib_data['shasum'])
    ]

    return registry_response(data, mimetype=accept, headers=headers)

@package_endpoint.route('/package/publish', methods=['POST', 'PUT'])
@cache_headers()
def post_gz():
    """Post package zip endpoint"""

    accept, _version = parse_accept(request.headers.get('Accept'), ['application/json'])

    # get auth

    user_id = get_auth(request.headers, request.method, request.url_rule, 'login_token')

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

    if not result:
        return unauthorized_custom('User does not exist')

    user = result[0]

    if not user:
        return unauthorized_custom('User is not logged in')

    # make sure usernames match

    if user.username != json_info['author']:
        return unprocessable_custom('Author username mismatch')

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

        if request.method != 'PUT':
            abort(405)

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

    # clear cache

    cache.clear()

    return registry_response(json_info, status=201, mimetype=accept)
