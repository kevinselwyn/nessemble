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
    reset_date = Column(DateTime)

    def __repr__(self):
        return "<User(id='%s', name='%s', email='%s', password='%s', date_created='%s', date_login='%s', login_token='%s')>" % (self.id, self.name, self.email, self.password, self.date_created, self.date_login, self.login_token)
