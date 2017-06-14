# coding=utf-8
# pylint: disable=C0103
"""Error routes"""

from flask import Blueprint
from ..utils.utils import bad_request_custom, conflict_custom
from ..utils.utils import internal_server_error_custom, method_not_allowed_custom
from ..utils.utils import not_found_custom, too_many_custom, unauthorized_custom
from ..utils.utils import unprocessable_custom

#----------------#
# Variables

errors_endpoint = Blueprint('errors_endpoint', __name__)

#----------------#
# Errors

@errors_endpoint.app_errorhandler(400)
def bad_request(_error):
    """Bad Request error handler"""

    return bad_request_custom()

@errors_endpoint.app_errorhandler(401)
def unauthorized(_error):
    """Unauthorized error handler"""

    return unauthorized_custom()

@errors_endpoint.app_errorhandler(404)
def not_found(_error):
    """Not Found error handler"""

    return not_found_custom()

@errors_endpoint.app_errorhandler(405)
def method_not_allowed(_error):
    """Method Not Allowed error handler"""

    return method_not_allowed_custom()

@errors_endpoint.app_errorhandler(409)
def conflict(_error):
    """Conflict error handler"""

    return conflict_custom()

@errors_endpoint.app_errorhandler(422)
def unprocessable(_error):
    """Unprocessable error handler"""

    return unprocessable_custom()

@errors_endpoint.app_errorhandler(429)
def too_many(_error):
    """Too Many Requests error handler"""

    return too_many_custom()

@errors_endpoint.app_errorhandler(500)
def internal_server_error(_error):
    """Internal Server Error error handler"""

    return internal_server_error_custom()
