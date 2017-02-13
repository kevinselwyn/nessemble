#!/bin/bash

# update
sudo apt-get -y update

# build tools
sudo apt-get -y install make bison flex

# libraries
sudo apt-get -y install libpng-dev libjansson-dev libcurl4-gnutls-dev

# debug tools
sudo apt-get -y install bc gdb splint valgrind

# python
sudo apt-get -y install python-pip

# python modules
sudo pip install flask
