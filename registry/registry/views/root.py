# coding=utf-8
"""Root view"""

from collections import OrderedDict
from flask import request
from ..models.libs import Lib
from ..utils.utils import Session

def root_view(page=1, per_page=10, sort_by='title', order='asc'):
    """View"""

    url_root = request.url_root
    session = Session()
    results = OrderedDict([
        ('packages', []),
        ('pagination', None)
    ])

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

    url_prev = '%s?page=%d&per_page=%d' % (url_root, page - 1, per_page)
    url_next = '%s?page=%d&per_page=%d' % (url_root, page + 1, per_page)

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
            ('url', '%spackage/%s' % (url_root, lib.title))
        ]))

    return results
