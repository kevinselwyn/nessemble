# coding=utf-8
# pylint: disable=C0103,C0301,C0326
"""Nessemble docs server"""

import os
from ConfigParser import ConfigParser
from flask import Flask, send_from_directory

#----------------#
# Constants

BASE       = os.path.normpath(os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', '..'))
ROOT       = os.path.normpath(os.path.join(BASE, 'docs', 'site'))

CONFIG     = ConfigParser()
CONFIG.readfp(open(os.path.join(BASE, 'settings.cfg')))

#----------------#
# Variables

app = Flask(__name__)

#----------------#
# Errors

@app.errorhandler(404)
def not_found(_error):
    """404 error handler"""

    return '404: Not Found', 404

#----------------#
# Endpoints

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
    filename = ROOT

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
