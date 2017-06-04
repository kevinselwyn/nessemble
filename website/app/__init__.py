# coding=utf-8
# pylint: disable=C0103,C0301,C0326
"""Nessemble website server"""

import os
from ConfigParser import ConfigParser
from flask import Flask, make_response, render_template
from flask_caching import Cache

#----------------#
# Constants

BASE       = os.path.normpath(os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', '..'))
ROOT       = os.path.normpath(os.path.join(BASE, 'website'))

CONFIG     = ConfigParser()
CONFIG.readfp(open(os.path.join(BASE, 'settings.cfg')))

CACHE_TIME = CONFIG.getint('website', 'cache_time')

#--------------#
# Variables

tmpl_dir = os.path.join(ROOT, 'templates')
static_dir = os.path.join(ROOT, 'static')
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

    return custom_response(render_template('index.html'))

#--------------#
# Variables

__all__ = ['app']
