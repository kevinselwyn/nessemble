# coding=utf-8
# pylint: disable=C0103,C0301
"""Nessemble cache"""

import datetime
import time
from functools import wraps
from flask_caching import Cache
from flask import make_response

cache = Cache()

def cache_headers(expires=None):
    """@cache_headers decorator"""

    def cache_decorator(view):
        """Cache decorator"""

        @wraps(view)
        def cache_func(*args, **kwargs):
            """Cache func"""

            now = datetime.datetime.now()

            response = make_response(view(*args, **kwargs))

            if expires is None:
                response.headers['Cache-Control'] = 'no-store, no-cache, must-revalidate, post-check=0, pre-check=0, max-age=0'
                response.headers['Expires'] = '-1'
            else:
                expires_time = now + datetime.timedelta(seconds=expires)
                expires_time = expires_time.replace(second=0, microsecond=0)

                response.headers['Cache-Control'] = 'max-age=%d' % (expires)
                response.headers['Expires'] = int(time.mktime(expires_time.timetuple()))

            return response

        return cache_func

    return cache_decorator
