# coding=utf-8
# pylint: disable=C0103,C0301
"""Reference route"""

from flask import abort, Blueprint, request
from ..config.cache import cache, cache_headers
from ..config.config import config as CONFIG
from ..utils.utils import parse_accept, registry_response, Session

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
        abort(404)

    return registry_response(output, mimetype=accept)
