# coding=utf-8
# pylint: disable=C0103,C0301,R0903
"""Lib model"""

from sqlalchemy import Column, DateTime, Integer, String, Text
from .base import Base

class Lib(Base):
    """Lib model"""

    __tablename__ = 'libs'

    id = Column(Integer, primary_key=True)
    user_id = Column(Integer)
    readme = Column(Text)
    lib = Column(Text)
    title = Column(String(128))
    description = Column(Text)
    version = Column(String(32))
    license = Column(String(128))
    tags = Column(Text)
    date = Column(DateTime)
    shasum = Column(String(40))

    def __repr__(self):
        return "<Lib(id='%s', user_id='%s', title='%s', description='%s', version='%s', license='%s', tags='%s', date='%s', shasum='%s')>" % (self.id, self.user_id, self.title, self.description, self.version, self.license, self.tags, self.date, self.shasum)
