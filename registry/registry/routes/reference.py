# coding=utf-8
# pylint: disable=C0103,C0301
"""Reference route"""

from flask import Blueprint, request
from ..config.cache import cache, cache_headers
from ..config.config import config as CONFIG
from ..utils.utils import parse_accept, registry_response
from ..views import reference_view

#----------------#
# Constants

CACHE_TIME = CONFIG.getint('registry', 'cache_time')

#----------------#
# Variables

reference_endpoint = Blueprint('reference_endpoint', __name__)

#----------------#
# Endpoints

@reference_endpoint.route('/reference', methods=['GET'])
@reference_endpoint.route('/reference/<path:path>', methods=['GET'])
@cache.cached(timeout=CACHE_TIME)
@cache_headers(CACHE_TIME)
def reference(path=''):
    """Reference endpoint"""

    accept, _version = parse_accept(request.headers.get('Accept'), ['text/plain'])

    paths = path.split('/')
    output = reference_view(paths=paths)

    return registry_response(output, mimetype=accept)
