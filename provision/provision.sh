#!/bin/bash

# update
sudo apt-get -y update

# build tools
sudo apt-get -y install git make curl bison flex

# libraries
sudo apt-get -y install liblua5.1-0-dev

# debug tools
sudo apt-get -y install bc gdb splint valgrind

# python
sudo apt-get -y install python-pip

# python modules
sudo pip install -r /vagrant/requirements.txt
sudo pip install -r /vagrant/registry/requirements.txt

# mingw
sudo apt-get -y install gcc-mingw-w64-i686 gcc-mingw-w64-x86-64

# msitools
sudo apt-get -y install intltool libglib2.0-dev libgsf-1-dev uuid-dev libgcab-dev libmsi-dev
curl -O "http://ftp.gnome.org/pub/GNOME/sources/msitools/0.96/msitools-0.96.tar.xz"
tar xf msitools-0.96.tar.xz && cd msitools-0.96
./configure
make && sudo make install
rm -rf msitools-0.96 msitools-0.96.tar.xz
