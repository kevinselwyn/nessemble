# coding=utf-8
# pylint: disable=C0103,C0301,C0326
"""Nessemble CDN server"""

import os
from flask import Flask, send_from_directory

#----------------#
# Constants

BASE       = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))

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

@app.route('/assembler.css')
def serve_assembler_css():
    """Serve assembler.css"""

    path = os.path.normpath(os.path.join(BASE, '..', 'docs', 'pages', 'css'))

    return send_from_directory(path, 'assembler.css')

@app.route('/assembler.min.css')
def serve_assembler_min_css():
    """Serve assembler.min.css"""

    path = os.path.normpath(os.path.join(BASE, '..', 'docs', 'pages', 'css'))

    return send_from_directory(path, 'assembler.min.css')

@app.route('/assemblers.js')
def serve_assemblers_js():
    """Serve assemblers.js"""

    path = os.path.normpath(os.path.join(BASE, '..', 'docs', 'pages', 'js'))

    return send_from_directory(path, 'assemblers.js')

@app.route('/assemblers.min.js')
def serve_assemblers_min_js():
    """Serve assemblers.min.js"""

    path = os.path.normpath(os.path.join(BASE, '..', 'docs', 'pages', 'js'))

    return send_from_directory(path, 'assemblers.min.js')

#--------------#
# Variables

__all__ = ['app']
