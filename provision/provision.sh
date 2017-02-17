#!/bin/bash

# update
sudo apt-get -y update

# build tools
sudo apt-get -y install make bison flex

# libraries
sudo apt-get -y install libpng-dev libjson-c-dev libcurl4-gnutls-dev libarchive-dev

# debug tools
sudo apt-get -y install bc gdb splint valgrind

# python
sudo apt-get -y install python-pip

# python modules
sudo pip install flask

# run server
python /vagrant/registry/server.py --debug &> /dev/null &
