#!/bin/bash

# update
sudo apt-get -y update

# build tools
sudo apt-get -y install git make bison flex

# libraries
sudo apt-get -y install liblua5.1-0-dev python-dev

# debug tools
sudo apt-get -y install bc gdb splint valgrind

# python
sudo apt-get -y install python-pip

# python modules
sudo pip install flask sqlalchemy

# run server
python /vagrant/registry/server.py --debug &> /dev/null &

# mingw
sudo apt-get -y install gcc-mingw-w64-i686 gcc-mingw-w64-x86-64
