# coding=utf-8
# pylint: disable=C0103,C0301,R0903
"""Lib model"""

from .base import Base
from sqlalchemy import Column, Integer, String, Text

class Lib(Base):
    """Lib model"""

    __tablename__ = 'libs'

    id = Column(Integer, primary_key=True)
    user_id = Column(Integer)
    readme = Column(Text)
    lib = Column(Text)
    name = Column(String(128))
    description = Column(Text)
    version = Column(String(32))
    license = Column(String(128))
    tags = Column(Text)

    def __repr__(self):
        return "<Lib(id='%s', user_id='%s', name='%s', description='%s', version='%s', license='%s', tags='%s')>" % (self.id, self.user_id, self.name, self.description, self.version, self.license, self.tags)
