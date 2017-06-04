#!/bin/bash

HOME="/home/ubuntu"
ROOT="/vagrant"

# parse args
TEMP=`getopt -o h:r: --long home:,root: -n 'provision.sh' -- "$@"`
eval set -- "$TEMP"

# parse args
while true
do
    case "$1" in
        -h|--home)
            HOME=$2 ; shift 2 ;;
        -r|--root)
            ROOT=$2 ; shift 2 ;;
        --) shift ; break ;;
        *) echo "Internal error!" ; exit 1 ;;
    esac
done

# funcs
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

# debug tools
sudo apt-get -y install bc gdb splint valgrind

# python
sudo apt-get -y install python-pip

# python modules
sudo pip install --upgrade pip

for reqs in `find $ROOT -name requirements.txt`
do
    sudo pip install -r $reqs
done

# mingw
sudo apt-get -y install gcc-mingw-w64-i686 gcc-mingw-w64-x86-64

# msitools
MSITOOLS_VERSION=0.96

sudo apt-get -y install intltool libglib2.0-dev libgsf-1-dev uuid-dev libgcab-dev libmsi-dev
rm -rf $HOME/msitools-$MSITOOLS_VERSION*
_retry curl -o $HOME/msitools-$MSITOOLS_VERSION.tar.xz "http://ftp.gnome.org/pub/GNOME/sources/msitools/$MSITOOLS_VERSION/msitools-$MSITOOLS_VERSION.tar.xz"
cd $HOME/ && tar xf msitools-$MSITOOLS_VERSION.tar.xz
cd $HOME/msitools-$MSITOOLS_VERSION && ./configure
cd $HOME/msitools-$MSITOOLS_VERSION && make && sudo make install
rm -rf $HOME/msitools-$MSITOOLS_VERSION*

# liblua
LIBLUA_VERSION=5.1.5

rm -rf $ROOT/src/third-party/lua-$LIBLUA_VERSION*
_retry curl -o $ROOT/src/third-party/lua-$LIBLUA_VERSION.tar.gz "https://www.lua.org/ftp/lua-$LIBLUA_VERSION.tar.gz"
cd $ROOT/src/third-party/ && tar xf lua-$LIBLUA_VERSION.tar.gz
rm -rf $ROOT/src/third-party/lua*.tar.gz

# libtinyscheme
LIBTINYSCHEME_VERSION=1.41

rm -rf $ROOT/src/third-party/tinyscheme*
_retry curl -o $ROOT/src/third-party/tinyscheme-$LIBTINYSCHEME_VERSION.tar.gz "https://svwh.dl.sourceforge.net/project/tinyscheme/tinyscheme/tinyscheme-$LIBTINYSCHEME_VERSION/tinyscheme-$LIBTINYSCHEME_VERSION.tar.gz"
cd $ROOT/src/third-party/ && tar xf tinyscheme-$LIBTINYSCHEME_VERSION.tar.gz
mv $ROOT/src/third-party/tinyscheme-$LIBTINYSCHEME_VERSION/scheme.h $ROOT/src/third-party/tinyscheme-$LIBTINYSCHEME_VERSION/scheme-old.h
cat $ROOT/src/third-party/tinyscheme-$LIBTINYSCHEME_VERSION/scheme-old.h | sed 's/# define STANDALONE 1/# define STANDALONE 0/' > $ROOT/src/third-party/tinyscheme-$LIBTINYSCHEME_VERSION/scheme.h
rm -rf $ROOT/src/third-party/tinyscheme*.tar.gz

# expose custom scripts
echo "PATH=\$PATH:$ROOT/provision/scripts" >> $HOME/.profile

# go to $ROOT at login
echo "cd $ROOT" >> $HOME/.profile
