# coding=utf-8
# pylint: disable=C0103
"""Search route"""

from collections import OrderedDict
from flask import Blueprint, request
from ..config.cache import cache
from ..config.config import config as CONFIG
from ..models.libs import Lib
from ..utils.utils import parse_accept, registry_response, Session

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
def search_packages(term=None):
    """Search packages endpoint"""

    accept, _version = parse_accept(request.headers.get('Accept'), ['application/json'])

    session = Session()
    results = OrderedDict([
        ('term', term),
        ('results', [])
    ])

    if term:
        term = '%%%s%%' % (term.lower())

    result = session.query(Lib) \
                    .group_by(Lib.title) \
                    .order_by(Lib.title, Lib.id)

    if term:
        result = result.filter(Lib.title.like(term))

    result = result.all()

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

    return registry_response(results, mimetype=accept)
