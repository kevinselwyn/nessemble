#!/bin/bash

_retry() {
    for i in {1..10}; do
        "$@" && return 0 || sleep 5;
    done
    return 1
}

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
sudo pip install --upgrade pip
sudo pip install -r /vagrant/requirements.txt
sudo pip install -r /vagrant/registry/requirements.txt

# mingw
sudo apt-get -y install gcc-mingw-w64-i686 gcc-mingw-w64-x86-64

# msitools
sudo apt-get -y install intltool libglib2.0-dev libgsf-1-dev uuid-dev libgcab-dev libmsi-dev
rm -rf /home/ubuntu/msitools-0.96*
_retry curl -o /home/ubuntu/msitools-0.96.tar.xz "http://ftp.gnome.org/pub/GNOME/sources/msitools/0.96/msitools-0.96.tar.xz"
cd /home/ubuntu/ && tar xf msitools-0.96.tar.xz
cd /home/ubuntu/msitools-0.96 && ./configure
cd /home/ubuntu/msitools-0.96 && make && sudo make install
rm -rf /home/ubuntu/msitools-0.96*

# tinyscheme
rm -rf /vagrant/src/third-party/tinyscheme*
_retry curl -o /vagrant/src/third-party/tinyscheme-1.41.tar.gz "https://svwh.dl.sourceforge.net/project/tinyscheme/tinyscheme/tinyscheme-1.41/tinyscheme-1.41.tar.gz"
cd /vagrant/src/third-party/ && tar xf tinyscheme-1.41.tar.gz
mv /vagrant/src/third-party/tinyscheme-1.41/scheme.h /vagrant/src/third-party/tinyscheme-1.41/scheme-old.h
cat /vagrant/src/third-party/tinyscheme-1.41/scheme-old.h | sed 's/# define STANDALONE 1/# define STANDALONE 0/' > /vagrant/src/third-party/tinyscheme-1.41/scheme.h
rm -rf /vagrant/src/third-party/tinyscheme*.tar.gz

# expose custom scripts
echo "PATH=\$PATH:/vagrant/provision/scripts" >> /home/ubuntu/.profile

# go to /vagrant at login
echo "cd /vagrant" >> /home/ubuntu/.profile
