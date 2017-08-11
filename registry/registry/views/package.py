# coding=utf-8
# pylint: disable=C0103
"""Package view"""

from flask import abort
from ..utils.utils import get_package_json, get_package_readme, get_package_zip

class package_view(object):
    """View"""

    @staticmethod
    def package(package, version=None):
        """package view"""

        output = get_package_json(package, version)

        if not output:
            abort(404)

        return output

    @staticmethod
    def readme(package, version=None):
        """readme view"""

        output = get_package_readme(package, version)

        if not output:
            abort(404)

        return output

    @staticmethod
    def data(package, version=None):
        """data view"""

        output = get_package_zip(package, version)

        if not output:
            abort(404)

        return output
