# coding=utf-8
# pylint: disable=C0103
"""Search route"""

from flask import Blueprint, request
from ..config.cache import cache, cache_headers
from ..config.config import config as CONFIG
from ..utils.utils import parse_accept, registry_response
from ..views import search_view

#----------------#
# Constants

CACHE_TIME = CONFIG.getint('registry', 'cache_time')

#----------------#
# Variables

search_endpoint = Blueprint('search_endpoint', __name__)

#----------------#
# Endpoints

@search_endpoint.route('/search', methods=['GET'])
@search_endpoint.route('/search/<string:term>', methods=['GET'])
@cache.cached(timeout=CACHE_TIME)
@cache_headers(CACHE_TIME)
def search_packages(term=None):
    """Search packages endpoint"""

    accept, _version = parse_accept(request.headers.get('Accept'), ['application/json'])
    args = {
        'term': term
    }

    results = search_view(**args)

    return registry_response(results, mimetype=accept)
