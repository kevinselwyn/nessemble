# coding=utf-8
# pylint: disable=C0103,C0301,C0326
"""Nessemble CDN server"""

import os
from flask import abort, Flask, send_from_directory

#----------------#
# Constants

BASE       = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))

#----------------#
# Variables

app = Flask(__name__)
files = [
    ('../docs/pages/css', 'assembler.css',     'text/css'),
    ('../docs/pages/css', 'assembler.min.css', 'text/css'),
    ('../docs/pages/js',  'assemblers.js',     'application/javascript'),
    ('../docs/pages/js',  'assemblers.min.js', 'application/javascript'),
    ('../docs/pages/js',  'nessemble.js',      'application/javascript'),
    ('../docs/pages/js',  'nessemble.min.js',  'application/javascript')
]

#----------------#
# Errors

@app.errorhandler(404)
def not_found(_error):
    """404 error handler"""

    return '404: Not Found', 404

#----------------#
# Endpoints

@app.route('/<path:filename>')
def serve(filename=''):
    """Serve file"""

    for serve_file in files:
        if serve_file[1] == filename:
            path = os.path.normpath(os.path.join(*([BASE] + serve_file[0].split('/'))))

            return send_from_directory(path, serve_file[1], mimetype=serve_file[2])

    abort(404)

#--------------#
# Variables

__all__ = ['app']
