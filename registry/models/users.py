# coding=utf-8
# pylint: disable=C0103,C0301,R0903
"""User model"""

from sqlalchemy import Column, create_engine, DateTime, Integer, String
from sqlalchemy.orm import sessionmaker
from sqlalchemy.ext.declarative import declarative_base

Base = declarative_base()
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

    def __repr__(self):
        return "<User(id='%s', name='%s', email='%s', password='%s', date_created='%s', date_login='%s', login_token='%s')>" % (self.id, self.name, self.email, self.password, self.date_created, self.date_login, self.login_token)

users_db = create_engine('sqlite:///users.db')
Base.metadata.create_all(users_db)
Users_Session = sessionmaker(bind=users_db)
