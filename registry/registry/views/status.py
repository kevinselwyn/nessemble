# coding=utf-8
"""Status view"""

from flask import render_template

def status_view():
    """View"""

    data = render_template('status.html')

    return data
