# coding=utf-8
# pylint: disable=C0103,C0301,C0326
"""Root route"""

from collections import OrderedDict
from flask import Blueprint, request
from ..config.cache import cache, cache_headers
from ..config.config import config as CONFIG
from ..models.libs import Lib
from ..utils.utils import make_cache_key, parse_accept, registry_response, Session

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

    session = Session()
    results = OrderedDict([
        ('packages', []),
        ('pagination', None)
    ])

    # pagination vars

    page = int(request.args.get('page', 1))
    per_page = int(request.args.get('per_page', 10))
    sort_by = request.args.get('sort_by', 'title').lower()
    order = request.args.get('order', 'asc').lower()
    total = 0

    if page <= 0:
        page = 1

    if sort_by not in ['title', 'description']:
        sort_by = 'title'

    if order not in ['asc', 'desc']:
        order = 'asc'

    # query total

    total = session.query(Lib).count()

    # query

    result = session.query(Lib) \
                    .group_by(Lib.title)

    if sort_by == 'title':
        if order == 'desc':
            result = result.order_by(Lib.title.desc(), Lib.id)
        else:
            result = result.order_by(Lib.title, Lib.id)
    elif sort_by == 'description':
        if order == 'desc':
            result = result.order_by(Lib.description.desc(), Lib.id)
        else:
            result = result.order_by(Lib.description, Lib.id)

    result = result.offset((page - 1) * per_page) \
                   .limit(per_page) \
                   .all()

    # pagination

    url_prev = '%s?page=%d&per_page=%d' % (request.url_root, page - 1, per_page)
    url_next = '%s?page=%d&per_page=%d' % (request.url_root, page + 1, per_page)

    if page == 1:
        url_prev = None

    if (page * per_page) >= total:
        url_next = None

    results['pagination'] = OrderedDict([
        ('page', page),
        ('per_page', per_page),
        ('total', total),
        ('prev', url_prev),
        ('next', url_next)
    ])

    # packages

    for lib in result:
        results['packages'].append(OrderedDict([
            ('title', lib.title),
            ('description', lib.description),
            ('tags', lib.tags.split(',')),
            ('url', '%spackage/%s' % (request.url_root, lib.title))
        ]))

    return registry_response(results, mimetype=accept)
