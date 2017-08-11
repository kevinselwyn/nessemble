# coding=utf-8
# pylint: disable=C0103,C0326
"""Status route"""

from flask import Blueprint, request
from ..config.cache import cache, cache_headers
from ..config.config import config as CONFIG
from ..utils.utils import make_cache_key, parse_accept, registry_response
from ..views import status_view

#----------------#
# Constants

CACHE_TIME    = CONFIG.getint('registry', 'cache_time')

#----------------#
# Variables

status_endpoint = Blueprint('status_endpoint', __name__)

#----------------#
# Endpoints

@status_endpoint.route('/status', methods=['GET'])
@cache.cached(timeout=CACHE_TIME, key_prefix=make_cache_key)
@cache_headers(CACHE_TIME)
def status():
    """Status endpoint"""

    accept, _version = parse_accept(request.headers.get('Accept'), ['text/html'])

    data = status_view()

    return registry_response(data, status=200, mimetype=accept)
