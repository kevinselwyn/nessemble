# coding=utf-8
# pylint: disable=C0103,C0301
"""Root route"""

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

root_endpoint = Blueprint('root_endpoint', __name__)

#----------------#
# Endpoints

@root_endpoint.route('/', methods=['GET'])
@cache.cached(timeout=CACHE_TIME)
def list_packages():
    """List packages endpoint"""

    accept, _version = parse_accept(request.headers.get('Accept'), ['application/json'])

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

    return registry_response(results, mimetype=accept)
