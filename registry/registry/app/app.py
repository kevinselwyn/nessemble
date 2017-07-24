# coding=utf-8
# pylint: disable=C0103,C0301,C0326
"""Nessemble registry server"""

import os
import sqlite3
import time
from hashlib import sha1
from flask import Flask, g
from flask_limiter import Limiter
from flask_limiter.util import get_remote_address
from ..config.cache import cache
from ..config.config import config as CONFIG
from ..models.libs import Lib
from ..routes import root_endpoint, status_endpoint, errors_endpoint, package_endpoint, reference_endpoint, search_endpoint, user_endpoint
from ..utils.utils import get_package_zip
from ..utils.utils import Session

#----------------#
# Constants

BASE       = os.path.normpath(os.path.join(os.path.dirname(__file__), '..', '..'))
DB_PATH    = os.path.join(BASE, CONFIG.get('registry', 'db'))
SQL_PATH   = os.path.join(BASE, CONFIG.get('registry', 'sql'))
CACHE_TIME = CONFIG.getint('registry', 'cache_time')

#----------------#
# Variables

tmpl_dir = os.path.join(BASE, 'templates')

app = Flask(__name__, template_folder=tmpl_dir)

cache.init_app(app, config={
    'CACHE_TYPE': 'simple',
    'CACHE_THRESHOLD': 256
})

app.register_blueprint(root_endpoint)
app.register_blueprint(status_endpoint)
app.register_blueprint(errors_endpoint)
app.register_blueprint(package_endpoint)
app.register_blueprint(reference_endpoint)
app.register_blueprint(search_endpoint)
app.register_blueprint(user_endpoint)

limiter = Limiter(
    app,
    key_func=get_remote_address,
    default_limits=CONFIG.get('registry', 'limits').split(','),
    headers_enabled=True
)

#----------------#
# Utilities

@app.before_request
def before_request():
    """Before request"""

    g.request_start_time = time.time()
    g.request_time = lambda: '%.5s' % (time.time() - g.request_start_time)

#----------------#
# Import/Export

def db_import(filename=None):
    """Import database"""

    os.unlink(DB_PATH)

    con = sqlite3.connect(DB_PATH)

    with open(filename, 'r') as f:
        sql = f.read()
        con.executescript(sql)

    # update shasums

    session = Session()

    results = session.query(Lib) \
                    .order_by(Lib.title, Lib.id) \
                    .all()

    for result in results:
        data = get_package_zip(result.title, result.version)

        session.query(Lib) \
               .filter(Lib.id == result.id) \
               .update({
                   'shasum': sha1(data).hexdigest()
               })
        session.commit()

def db_export():
    """Export database"""

    con = sqlite3.connect(DB_PATH)

    with open(SQL_PATH, 'w') as f:
        for line in con.iterdump():
            f.write('%s\n' % line)

#--------------#
# Variables

__all__ = ['app', 'db_import', 'db_export']
