# coding=utf-8
# pylint: disable=C0103,C0301,R0914,W0702
"""User routes"""

import base64
import datetime
import io
import md5
import random
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
import qrcode
from flask import abort, Blueprint, render_template, request
from ..config.config import config as CONFIG
from ..models.users import User
from ..utils.utils import bad_request_custom, conflict_custom, unauthorized_custom
from ..utils.utils import get_auth, missing_fields, parse_accept, registry_response, validate_email
from ..utils.utils import Session

#----------------#
# Constants

CACHE_TIME = CONFIG.getint('registry', 'cache_time')

#----------------#
# Variables

user_endpoint = Blueprint('user_endpoint', __name__)

#----------------#
# Endpoints

@user_endpoint.route('/user/create', methods=['POST'])
def user_create():
    """Create new user"""

    accept, _version = parse_accept(request.headers.get('Accept'), ['application/json'])

    user = request.get_json()

    missing = missing_fields(user, ['name', 'username', 'email', 'password'])
    if missing:
        return missing

    # start session

    session = Session()

    # check if user exists

    result = session.query(User) \
                    .filter(User.username == user['username']) \
                    .all()

    if result:
        return conflict_custom('User already exists')

    # validate email
    if not validate_email(user['email']):
        return bad_request_custom('Invalid email address')

    # create user

    session.add(User(
        name=user['name'],
        username=user['username'],
        email=user['email'],
        password=md5.new(user['password']).hexdigest(),
        date_created=datetime.datetime.now()))
    session.commit()

    return registry_response({}, mimetype=accept)

@user_endpoint.route('/user/login', methods=['POST'])
def user_login():
    """User login"""

    accept, _version = parse_accept(request.headers.get('Accept'), ['application/json'])

    auth = request.authorization

    missing = missing_fields(auth, ['username', 'password'], 'authorization')
    if missing:
        return missing

    # start session

    session = Session()

    # check if user exists

    result = session.query(User) \
                    .filter(User.username == auth['username']) \
                    .all()

    if not result:
        return unauthorized_custom('User does not exist')

    user = result[0]

    if user.password != md5.new(auth['password']).hexdigest():
        return unauthorized_custom('Incorrect password')

    # create login token

    login_token = '%X' % (random.getrandbits(128))

    session.query(User) \
           .filter(User.id == user.id) \
           .update({
               'date_login': datetime.datetime.now(),
               'login_token': login_token
           })
    session.commit()

    return registry_response({
        'token': login_token
    }, mimetype=accept)

@user_endpoint.route('/user/logout', methods=['GET', 'POST'])
def user_logout():
    """User logout"""

    accept, _version = parse_accept(request.headers.get('Accept'), ['application/json'])

    # get auth

    user_id = get_auth(request.headers, request.method, request.url_rule, 'login_token')

    if not user_id:
        return unauthorized_custom('User is not logged in')

    # start session

    session = Session()

    # check if user exists

    result = session.query(User) \
                    .filter(User.id == user_id) \
                    .all()

    if not result:
        return unauthorized_custom('User does not exist')

    user = result[0]

    if not user:
        return unauthorized_custom('User does not exist')

    # log out

    session.query(User) \
           .filter(User.id == user_id) \
           .update({
               'login_token': None
           })
    session.commit()

    return registry_response({}, mimetype=accept)

@user_endpoint.route('/user/forgotpassword', methods=['POST'])
def user_forgotpassword():
    """User forgot password"""

    accept, _version = parse_accept(request.headers.get('Accept'), ['application/json'])

    user = request.get_json()

    missing = missing_fields(user, ['username'])
    if missing:
        return missing

    # start session

    session = Session()

    # check if user exists

    result = session.query(User) \
                    .filter(User.username == user['username']) \
                    .all()

    if not result:
        return conflict_custom('User does not exist')

    user = result[0]

    if not user:
        return conflict_custom('User does not exist')

    # generate reset token

    forgot_password_email = CONFIG.get('registry', 'forgot_password_email')
    forgot_password_subject = CONFIG.get('registry', 'forgot_password_subject')
    forgot_password_timeout = CONFIG.getint('registry', 'forgot_password_timeout')

    reset_token = '%X' % (random.getrandbits(128))

    session.query(User) \
           .filter(User.id == user.id) \
           .update({
               'reset_token': reset_token,
               'date_reset': datetime.datetime.now() + datetime.timedelta(seconds=forgot_password_timeout)
           })
    session.commit()

    # render email message

    message = MIMEMultipart('alternative')
    message['Subject'] = forgot_password_subject
    message['From'] = forgot_password_email
    message['To'] = user.email

    message_text = render_template('forgot-password.txt', \
        name=user.name, \
        reset_token=user.reset_token, \
        reset_time=(forgot_password_timeout / 60) \
    )
    message_html = render_template('forgot-password.html', \
        name=user.name, \
        reset_token=user.reset_token, \
        reset_time=(forgot_password_timeout / 60) \
    )

    message.attach(MIMEText(message_text, 'plain'))
    message.attach(MIMEText(message_html, 'html'))

    # send email

    smtp_user = CONFIG.get('registry', 'smtp_user')
    smtp_pass = CONFIG.get('registry', 'smtp_pass')
    smtp_domain = CONFIG.get('registry', 'smtp_domain')

    try:
        smtp_port = CONFIG.getint('registry', 'smtp_port')
    except:
        smtp_port = 0

    email = False

    try:
        server = smtplib.SMTP_SSL(smtp_domain, smtp_port)
        server.ehlo()
        server.login(smtp_user, smtp_pass)
        server.sendmail(forgot_password_email, [user.email], message.as_string())
        server.close()

        email = True
    except:
        pass

    reset_id = '%X' % (random.getrandbits(128))

    session.query(User) \
           .filter(User.id == user.id) \
           .update({
               'reset_id': reset_id
           })
    session.commit()

    return registry_response({
        'email': email,
        'url': '%suser/2FA/%s' % (request.url_root, reset_id)
    }, mimetype=accept)

@user_endpoint.route('/user/2FA/<string:reset_id>', methods=['GET'])
def user_2fa(reset_id=None):
    """User 2FA"""

    accept, _version = parse_accept(request.headers.get('Accept'), ['image/png'])

    if not reset_id:
        abort(401)

    # start session

    session = Session()

    # check if user exists

    result = session.query(User) \
                    .filter(User.reset_id == reset_id) \
                    .all()

    if not result:
        abort(401)

    user = result[0]

    if not user:
        abort(401)

    # check if reset token

    if not user.reset_token or not user.reset_id:
        abort(401)

    # generate qr code

    uri = 'otpauth://totp/%s:%s?secret=%s&issuer=%s' % ('Nessemble', user.username, base64.b32encode(user.reset_token), 'Nessemble')
    qr = qrcode.make(uri)

    img = io.BytesIO()
    qr.save(img, 'PNG')
    img.seek(0)

    # clear reset_id

    session.query(User) \
           .filter(User.id == user.id) \
           .update({
               'reset_id': None
           })
    session.commit()

    return registry_response(img.read(), mimetype=accept)

@user_endpoint.route('/user/resetpassword', methods=['POST'])
def user_resetpassword():
    """User reset password"""

    accept, _version = parse_accept(request.headers.get('Accept'), ['application/json'])

    # get user data

    user_data = request.get_json()

    missing = missing_fields(user_data, ['username', 'password'])
    if missing:
        return missing

    # get auth

    user_id = get_auth(request.headers, request.method, request.url_rule, 'reset_token')

    if not user_id:
        return unauthorized_custom('Invalid reset token')

    # start session

    session = Session()

    # check if user exists

    result = session.query(User) \
                    .filter(User.id == user_id) \
                    .all()

    if not result:
        return unauthorized_custom('User does not exist')

    user = result[0]

    if not user:
        return unauthorized_custom('User does not exist')

    if user.username != user_data['username']:
        return unauthorized_custom('Username mismatch')

    # update

    session.query(User) \
           .filter(User.id == user_id) \
           .update({
               'password': md5.new(user_data['password']).hexdigest(),
               'reset_token': None
           })
    session.commit()

    return registry_response({}, mimetype=accept)
