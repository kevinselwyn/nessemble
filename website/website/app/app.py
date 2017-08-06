# coding=utf-8
# pylint: disable=C0103,C0301,C0330,C0326
"""Nessemble website server"""

import os
from flask import Flask, make_response, render_template, send_from_directory
from flask_caching import Cache
from ..config.config import config as CONFIG

#----------------#
# Constants

BASE       = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))
CACHE_TIME = CONFIG.getint('website', 'cache_time')

#--------------#
# Variables

tmpl_dir = os.path.join(BASE, 'templates')
static_dir = os.path.join(BASE, 'static')
app = Flask(__name__, template_folder=tmpl_dir, static_folder=static_dir)
cache = Cache(app, config={
    'CACHE_TYPE': 'simple',
    'CACHE_THRESHOLD': 256
})

#--------------#
# Functions

def custom_response(data, status=200, mimetype='text/html'):
    """Custom HTTP response"""

    response = make_response(data, status)

    response.headers.remove('Content-Type')
    response.headers.add('Content-Type', mimetype)

    return response

#--------------#
# Endpoints

@app.route('/', methods=['GET'])
@cache.cached(timeout=CACHE_TIME)
def index():
    """Index endpoint"""

    return custom_response(render_template('index.html',
        documentation=CONFIG.get('website', 'documentation'),
        analytics_id=CONFIG.get('website', 'analytics_id'),
        analytics_domain=CONFIG.get('website', 'analytics_domain')
    ))

@app.route('/favicon.ico')
def serve_favicon():
    """Serve favicon"""

    filename = os.path.join(BASE, 'static')

    return send_from_directory(filename, 'favicon.ico')

#--------------#
# Variables

__all__ = ['app']
