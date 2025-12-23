#!/bin/sh

# abort if script fails
set -e

# update package library
apk update

# all dependencies needed for the project.
PACKAGES="
python3
build-base
linux-headers
make
cmake
git
glib-dev
zlib-dev
pkgconf
jansson-dev
libconfig-dev
libnice-dev
openssl-dev
libsrtp-dev
libusrsctp-dev
libmicrohttpd-dev
libwebsockets-dev
autoconf-archive
automake
m4
libtool
"

echo "installing dependencies"
apk add $PACKAGES
echo "dependencies installed"

echo "checking dependencies"
for pkg in $PACKAGES; do
    apk info -e "$pkg"
done
echo "all packages installed correctly"