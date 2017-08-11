# coding=utf-8
"""Search view"""

from collections import OrderedDict
from flask import request
from ..models.libs import Lib
from ..utils.utils import Session

def search_view(term=None):
    """View"""

    url_root = request.url_root
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
            ('url', '%spackage/%s' % (url_root, lib.title))
        ]))

    return results
