#!/bin/bash

# paths
HOME="/home/ubuntu"
ROOT="/vagrant"

# start time
START=$(date +%s)

# constants
MSITOOLS_VERSION=0.96
LIBLUA_VERSION=5.1.5
LIBTINYSCHEME_VERSION=1.41

# urls
MSITOOLS_URL="http://ftp.gnome.org/pub/GNOME/sources/msitools/$MSITOOLS_VERSION/msitools-$MSITOOLS_VERSION.tar.xz"
LIBLUA_URL="https://www.lua.org/ftp/lua-$LIBLUA_VERSION.tar.gz"
LIBTINYSCHEME_URL="https://svwh.dl.sourceforge.net/project/tinyscheme/tinyscheme/tinyscheme-$LIBTINYSCHEME_VERSION/tinyscheme-$LIBTINYSCHEME_VERSION.tar.gz"
EMSCRIPTEN_URL="https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz"

# funcs
_retry() {
    for i in {1..10}; do
        "$@" && return 0 || sleep 5;
    done
    return 1
}

_download() {
    if [ ! -f $2 ]
    then
        _retry curl -s -o $2 "$1"
    fi

    if [ ! -f $2 ]
    then
        printf "Could not download %s" $1
        exit 1
    fi
}

# cache
CACHE="0"

if [ "$1" == "cache" ]
then
    CACHE="1"
    shift
fi

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
        *) printf "Internal error!\n" ; exit 1 ;;
    esac
done

# cache
if [ "$CACHE" == "1" ]
then
    rm -rf $HOME/cache
fi

mkdir -p $HOME/cache

if [ "$CACHE" == "1" ]
then
    _download $MSITOOLS_URL $HOME/cache/msitools-$MSITOOLS_VERSION.tar.xz
    _download $LIBLUA_URL $HOME/cache/lua-$LIBLUA_VERSION.tar.gz
    _download $LIBTINYSCHEME_URL $HOME/cache/tinyscheme-$LIBTINYSCHEME_VERSION.tar.gz
    _download $EMSCRIPTEN_URL $HOME/cache/emsdk-portable.tar.gz
fi

# update
sudo apt-get -y update

# build tools
sudo apt-get -y install git make curl bison flex

# debug tools
sudo apt-get -y install bc gdb splint valgrind

# install nvm
curl -o- https://raw.githubusercontent.com/creationix/nvm/v0.33.2/install.sh | bash
export NVM_DIR="$HOME/.nvm"
[ -s $NVM_DIR/nvm.sh ] && . "$NVM_DIR/nvm.sh"

# install node
nvm install 8.0.0
nvm use 8.0.0

# npm fix
sudo chown -R ubuntu:$(id -gn ubuntu) $HOME/.config

# install yarn
npm install -g yarn

# node modules
yarn global add uglify-js uglifycss typescript
cd $ROOT/docs && yarn install

# python
sudo apt-get -y install python-pip

# python modules
sudo -H pip install --upgrade pip

for reqs in `find $ROOT -name requirements.txt`
do
    sudo -H pip install -r $reqs
done

# mingw
sudo apt-get -y install gcc-mingw-w64-i686 gcc-mingw-w64-x86-64

# msitools
sudo apt-get -y install intltool libglib2.0-dev libgsf-1-dev uuid-dev libgcab-dev libmsi-dev
rm -rf $HOME/msitools-$MSITOOLS_VERSION*
_download $MSITOOLS_URL $HOME/cache/msitools-$MSITOOLS_VERSION.tar.xz
cp $HOME/cache/msitools-$MSITOOLS_VERSION.tar.xz $HOME/msitools-$MSITOOLS_VERSION.tar.xz
cd $HOME/ && tar xf msitools-$MSITOOLS_VERSION.tar.xz
cd $HOME/msitools-$MSITOOLS_VERSION && ./configure
cd $HOME/msitools-$MSITOOLS_VERSION && make && sudo make install
rm -rf $HOME/msitools-$MSITOOLS_VERSION*

# liblua
rm -rf $ROOT/src/third-party/lua-$LIBLUA_VERSION*
_download $LIBLUA_URL $HOME/cache/lua-$LIBLUA_VERSION.tar.gz
cp $HOME/cache/lua-$LIBLUA_VERSION.tar.gz $ROOT/src/third-party/lua-$LIBLUA_VERSION.tar.gz
cd $ROOT/src/third-party/ && tar xf lua-$LIBLUA_VERSION.tar.gz
rm -rf $ROOT/src/third-party/lua*.tar.gz

# libtinyscheme
rm -rf $ROOT/src/third-party/tinyscheme*
_download $LIBTINYSCHEME_URL $HOME/cache/tinyscheme-$LIBTINYSCHEME_VERSION.tar.gz
cp $HOME/cache/tinyscheme-$LIBTINYSCHEME_VERSION.tar.gz $ROOT/src/third-party/tinyscheme-$LIBTINYSCHEME_VERSION.tar.gz
cd $ROOT/src/third-party/ && tar xf tinyscheme-$LIBTINYSCHEME_VERSION.tar.gz
mv $ROOT/src/third-party/tinyscheme-$LIBTINYSCHEME_VERSION/scheme.h $ROOT/src/third-party/tinyscheme-$LIBTINYSCHEME_VERSION/scheme-old.h
cat $ROOT/src/third-party/tinyscheme-$LIBTINYSCHEME_VERSION/scheme-old.h | sed 's/# define STANDALONE 1/# define STANDALONE 0/' > $ROOT/src/third-party/tinyscheme-$LIBTINYSCHEME_VERSION/scheme.h
rm -rf $ROOT/src/third-party/tinyscheme*.tar.gz

# emscripten
rm -rf $HOME/emsdk-portable*
_download $EMSCRIPTEN_URL $HOME/cache/emsdk-portable.tar.gz
cp $HOME/cache/emsdk-portable.tar.gz $HOME/emsdk-portable.tar.gz
cd $HOME && tar xf emsdk-portable.tar.gz
cd $HOME/emsdk-portable
./emsdk update
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh

# heroku
wget -qO- https://cli-assets.heroku.com/install-ubuntu.sh | sh

# scripts
if [ -d "/vagrant" ]
then
    # expose custom scripts
    printf "PATH=\"\$PATH:%s/provision/scripts\"\n" $ROOT >> $HOME/.profile

    # go to $ROOT at login
    printf "\n#login directory\ncd %s\n" $ROOT >> $HOME/.profile
fi

END=$(date +%s)

printf "Provisioning completed in %s seconds\n" $((END-START))
printf "Run \"vagrant ssh\" to connect to VM\n"
