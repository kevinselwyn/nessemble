# coding=utf-8
# pylint: disable=C0103,C0301,R0903
"""User model"""

from sqlalchemy import Column, DateTime, Integer, String
from .base import Base

class User(Base):
    """User model"""

    __tablename__ = 'users'

    id = Column(Integer, primary_key=True)
    name = Column(String(128))
    username = Column(String(128))
    email = Column(String(128))
    password = Column(String(128))
    date_created = Column(DateTime)
    date_login = Column(DateTime)
    login_token = Column(String(128))
    reset_token = Column(String(128))
    date_reset = Column(DateTime)
    reset_id = Column(String(128))

    def __repr__(self):
        return ("<User(id='%s', name='%s', username='%s', email='%s', "
                "password='%s', date_created='%s', date_login='%s', "
                "login_token='%s', reset_token='%s', date_reset='%s', "
                "reset_id='%s')>") % (
                    self.id,
                    self.name,
                    self.username,
                    self.email,
                    self.password,
                    self.date_created,
                    self.date_login,
                    self.login_token,
                    self.reset_token,
                    self.date_reset,
                    self.reset_id
                )

    def __getitem__(self, key):
        switcher = {
            'id': self.id,
            'name': self.name,
            'username': self.username,
            'email': self.email,
            'password': self.password,
            'date_created': self.date_created,
            'date_login': self.date_login,
            'login_token': self.login_token,
            'reset_token': self.reset_token,
            'date_reset': self.date_reset,
            'reset_id': self.reset_id
        }

        return switcher.get(key, lambda: None)
