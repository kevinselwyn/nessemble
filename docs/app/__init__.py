# coding=utf-8
# pylint: disable=C0103
"""Nessemble docs server"""

import os
from flask import Flask, send_from_directory
from flask_caching import Cache

#----------------#
# Constants

CACHE_TIME = 60 * 60

#----------------#
# Variables

app = Flask(__name__)
cache = Cache(app, config={
    'CACHE_TYPE': 'simple',
    'CACHE_THRESHOLD': 256
})

#----------------#
# Errors

@app.errorhandler(404)
def not_found(_error):
    """404 error handler"""

    return '404: Not Found', 404

#----------------#
# Endpoints

@app.route('/')
@cache.cached(timeout=CACHE_TIME)
def serve_root():
    """Serve root"""

    return serve_path('index.html')

@app.route('/favicon.ico')
@cache.cached(timeout=CACHE_TIME)
def serve_favicon():
    """Serve favicon"""

    return serve_path(os.path.join('img', 'favicon.ico'))

@app.route('/search.html')
@cache.cached(timeout=CACHE_TIME)
def serve_search():
    """Serve search"""

    return serve_path('search.html')

@app.route('/sitemap.xml')
@cache.cached(timeout=CACHE_TIME)
def serve_sitemap():
    """Serve search"""

    return serve_path('sitemap.xml')

@app.route('/<path:path>')
@cache.cached(timeout=CACHE_TIME)
def serve_path(path='index.html'):
    """Serve path"""

    directory = None
    filename = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'site')

    if path:
        directory = path.split(os.sep)[0]

    if directory in ['css', 'js', 'img', 'fonts', 'mkdocs']:
        return send_from_directory(filename, path)
    else:
        if os.path.splitext(path)[1] in ['.html', '.xml']:
            return send_from_directory(filename, path)

        filename = os.path.join(filename, path)

    return send_from_directory(filename, 'index.html')

#--------------#
# Variables

__all__ = ['app']
