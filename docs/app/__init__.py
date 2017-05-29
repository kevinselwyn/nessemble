# coding=utf-8
# pylint: disable=C0103
"""Nessemble docs server"""

import os
from flask import Flask, send_from_directory

app = Flask(__name__)

@app.errorhandler(404)
def not_found(_error):
    """404 error handler"""

    return '404: Not Found', 404

@app.route('/')
def serve_root():
    """Serve root"""

    return serve_path('index.html')

@app.route('/favicon.ico')
def serve_favicon():
    """Serve favicon"""

    return serve_path(os.path.join('img', 'favicon.ico'))

@app.route('/search.html')
def serve_search():
    """Serve search"""

    return serve_path('search.html')

@app.route('/sitemap.xml')
def serve_sitemap():
    """Serve search"""

    return serve_path('sitemap.xml')

@app.route('/<path:path>')
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

__all__ = ['app']
