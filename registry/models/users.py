# coding=utf-8
# pylint: disable=C0103,C0301,R0903
"""User model"""

from .base import Base
from sqlalchemy import Column, DateTime, Integer, String

class User(Base):
    """User model"""

    __tablename__ = 'users'

    id = Column(Integer, primary_key=True)
    name = Column(String(128))
    email = Column(String(128))
    password = Column(String(128))
    date_created = Column(DateTime)
    date_login = Column(DateTime)
    login_token = Column(String(128))
    reset_token = Column(String(128))
    date_reset = Column(DateTime)

    def __repr__(self):
        return "<User(id='%s', name='%s', email='%s', password='%s', date_created='%s', date_login='%s', login_token='%s')>" % (self.id, self.name, self.email, self.password, self.date_created, self.date_login, self.login_token)

    def __getitem__(self, key):
        switcher = {
            'id': self.id,
            'name': self.name,
            'email': self.email,
            'password': self.password,
            'date_created': self.date_created,
            'date_login': self.date_login,
            'login_token': self.login_token,
            'reset_token': self.reset_token,
            'date_reset': self.date_reset
        }

        return switcher.get(key, lambda: None)
