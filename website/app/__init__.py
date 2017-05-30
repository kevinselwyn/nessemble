# coding=utf-8
# pylint: disable=C0103
"""Nessemble website server"""

import os
from flask import Flask, make_response, render_template

#--------------#
# Variables

tmpl_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'templates')
static_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'static')
app = Flask(__name__, template_folder=tmpl_dir, static_folder=static_dir)

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
def index():
    """Index endpoint"""

    return custom_response(render_template('index.html'))

#--------------#
# Variables

__all__ = ['app']
