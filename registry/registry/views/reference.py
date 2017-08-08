# coding=utf-8
# pylint: disable=C0301
"""Reference view"""

from flask import abort
from ..utils.utils import Session

def reference_view(paths=None):
    """View"""

    output = ''
    sql = []
    sql_and = []
    sql_data = {}

    if not paths:
        paths = [None]

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

    return output
