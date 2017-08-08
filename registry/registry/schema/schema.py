# coding=utf-8
# pylint: disable=C0103,R0201,R0903
"""Schema"""

import graphene
from graphene.types.generic import GenericScalar
from ..views import root_view, status_view, package_view, reference_view, search_view

def packages_resolver(_root, args, _context, _info):
    """packages resolver"""

    opts = {
        'page': args['page'],
        'per_page': args['perPage'],
        'sort_by': args['sortBy'],
        'order': args['order']
    }

    return root_view(**opts)

def status_resolver(_root, _args, _context, _info):
    """status resolver"""

    return status_view()

def package_resolver(_root, args, _context, _info):
    """package_resolver"""

    output = None
    opts = {
        'package': args['title'],
        'version': args['version'] if args['version'] != '' else None
    }

    if args['target'] == 'package':
        output = package_view.package(**opts)
    elif args['target'] == 'readme':
        output = {
            'readme': package_view.readme(**opts)
        }
    elif args['target'] == 'data':
        output = {
            'data': ''.join([('\\x%02X' % (ord(i))) for i in package_view.data(**opts)])
        }

    return output

def reference_resolver(_root, args, _context, _info):
    """reference resolver"""

    opts = {
        'paths': args['terms']
    }

    return reference_view(**opts)

def search_resolver(_root, args, _context, _info):
    """search resolver"""

    return search_view(**args)

class Query(graphene.ObjectType):
    """Query root"""

    packages = GenericScalar(
        page=graphene.Argument(
            graphene.Int,
            default_value=1,
            required=False
        ),
        perPage=graphene.Argument(
            graphene.Int,
            default_value=10,
            required=False
        ),
        sortBy=graphene.Argument(
            graphene.String,
            default_value='title',
            required=False
        ),
        order=graphene.Argument(
            graphene.String,
            default_value='asc',
            required=False
        ),
        resolver=packages_resolver
    )

    status = GenericScalar(
        resolver=status_resolver
    )

    package = GenericScalar(
        title=graphene.Argument(
            graphene.String,
            required=True
        ),
        version=graphene.Argument(
            graphene.String,
            default_value='',
            required=False
        ),
        target=graphene.Argument(
            graphene.String,
            default_value='package',
            required=False
        ),
        resolver=package_resolver
    )

    reference = GenericScalar(
        terms=graphene.Argument(
            graphene.List(graphene.String),
            default_value=[],
            required=False
        ),
        resolver=reference_resolver
    )

    search = GenericScalar(
        term=graphene.Argument(
            graphene.String,
            default_value=''
        ),
        resolver=search_resolver
    )

schema = graphene.Schema(query=Query)
