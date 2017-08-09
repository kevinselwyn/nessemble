# coding=utf-8
# pylint: disable=C0103,R0201,R0903
"""Schema"""

import base64
import graphene
from ..views import root_view, status_view, package_view, reference_view, search_view

# PACKAGES

class PackagesPackage(graphene.ObjectType):
    """Packages package root"""

    title = graphene.String()
    description = graphene.String()
    tags = graphene.List(graphene.String)
    url = graphene.String()

class PackagesPagination(graphene.ObjectType):
    """Packages pagination root"""

    page = graphene.Int()
    perPage = graphene.Int()
    total = graphene.Int()
    prev = graphene.String()
    next = graphene.String()

class Packages(graphene.ObjectType):
    """Packages root"""

    package = graphene.List(PackagesPackage)
    pagination = graphene.Field(PackagesPagination)

def packages_resolver(_root, args, _context, _info):
    """Packages resolver"""

    opts = {
        'page': args['page'],
        'per_page': args['perPage'],
        'sort_by': args['sortBy'],
        'order': args['order']
    }

    data = dict(root_view(**opts))

    for i in range(0, len(data['packages'])):
        data['packages'][i] = PackagesPackage(**(data['packages'][i]))

    pagination_opts = {
        'page': data['pagination']['page'],
        'perPage': data['pagination']['per_page'],
        'prev': data['pagination']['prev'],
        'next': data['pagination']['next']
    }

    data['pagination'] = PackagesPagination(**pagination_opts)

    return Packages(package=data['packages'], pagination=data['pagination'])

# STATUS

class Status(graphene.String):
    """Status root"""

def status_resolver(_root, _args, _context, _info):
    """Status resolver"""

    return status_view()

# PACKAGE

def package_versions_resolver(_root, args, _context, _info):
    """Package versions resolver"""

    versions = list(_root.versions)

    if versions:
        if args['last']:
            versions = versions[:1]
        elif args['first']:
            versions = versions[-1:]

    return versions

class Package(graphene.ObjectType):
    """Package root"""

    title = graphene.String()
    description = graphene.String()
    version = graphene.String()
    versions = graphene.Field(graphene.List(graphene.String),
                              last=graphene.Argument(
                                  graphene.Boolean,
                                  default_value=False,
                                  required=False
                              ),
                              first=graphene.Argument(
                                  graphene.Boolean,
                                  default_value=False,
                                  required=False
                              ),
                              resolver=package_versions_resolver
                             )
    author = graphene.String()
    license = graphene.String()
    tags = graphene.List(graphene.String)
    date = graphene.String()
    readme = graphene.String()
    resource = graphene.String()
    shasum = graphene.String()

def package_resolver(_root, args, _context, _info):
    """Package resolver"""

    output = None
    opts = {
        'package': args['title'],
        'version': args['version'] if args['version'] != '' else None
    }

    output = dict(package_view.package(**opts))
    output['readme'] = package_view.readme(**opts)
    output['resource'] = base64.b64encode(package_view.data(**opts))

    return Package(**output)

# REFERENCE

class Reference(graphene.String):
    """Reference root"""

def reference_resolver(_root, args, _context, _info):
    """Reference resolver"""

    opts = {
        'paths': args['terms']
    }

    return reference_view(**opts)

# SEARCH

class SearchResult(graphene.ObjectType):
    """Search result root"""

    title = graphene.String()
    description = graphene.String()
    tags = graphene.List(graphene.String)
    url = graphene.String()

class Search(graphene.ObjectType):
    """Search root"""

    term = graphene.String()
    results = graphene.List(SearchResult)

def search_resolver(_root, args, _context, _info):
    """Search resolver"""

    data = dict(search_view(**args))

    for i in range(0, len(data['results'])):
        data['results'][i] = SearchResult(**(data['results'][i]))

    return Search(term=data['term'], results=data['results'])

# QUERY

class Query(graphene.ObjectType):
    """Query root"""

    packages = graphene.Field(Packages,
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

    status = graphene.Field(Status,
                            resolver=status_resolver
                           )

    package = graphene.Field(Package,
                             title=graphene.Argument(
                                 graphene.String,
                                 required=True
                             ),
                             version=graphene.Argument(
                                 graphene.String,
                                 default_value='',
                                 required=False
                             ),
                             resolver=package_resolver
                            )

    reference = graphene.Field(Reference,
                               terms=graphene.Argument(
                                   graphene.List(graphene.String),
                                   default_value=[],
                                   required=False
                               ),
                               resolver=reference_resolver
                              )

    search = graphene.Field(Search,
                            term=graphene.Argument(
                                graphene.String,
                                default_value=''
                            ),
                            resolver=search_resolver
                           )

schema = graphene.Schema(query=Query)
