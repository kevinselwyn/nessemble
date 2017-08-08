# coding=utf-8
# pylint: disable=C0103,C0301,C0326
"""Root route"""

from flask import Blueprint, request
from ..config.cache import cache, cache_headers
from ..config.config import config as CONFIG
from ..utils.utils import make_cache_key, parse_accept, registry_response
from ..views import root_view

#----------------#
# Constants

CACHE_TIME    = CONFIG.getint('registry', 'cache_time')
ALLOWED_ORDER = ['asc', 'desc']
ALLOWED_SORT  = ['title', 'description']

#----------------#
# Variables

root_endpoint = Blueprint('root_endpoint', __name__)

#----------------#
# Endpoints

@root_endpoint.route('/', methods=['GET'])
@cache.cached(timeout=CACHE_TIME, key_prefix=make_cache_key)
@cache_headers(CACHE_TIME)
def list_packages():
    """List packages endpoint"""

    accept, _version = parse_accept(request.headers.get('Accept'), ['application/json'])
    args = {
        'page': int(request.args.get('page', 1)),
        'per_page': int(request.args.get('per_page', 10)),
        'sort_by': request.args.get('sort_by', 'title').lower(),
        'order': request.args.get('order', 'asc').lower()
    }

    results = root_view(**args)

    return registry_response(results, mimetype=accept)
