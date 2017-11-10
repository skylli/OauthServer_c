#!/bin/sh
mkdir  -p package 
cd packeage

# Install libjwt
# libtool and autoconf may be required, install them with 'sudo apt-get install libtool autoconf'
git clone https://github.com/benmcollins/libjwt.git
cd libjwt
autoreconf -i
./configure # use ./configure --without-openssl to use gnutls instead, you must have gnutls 3.5.8 minimum
make
sudo make install
cd ..

# Install Orcania
git clone https://github.com/babelouest/orcania.git
cd orcania
make
sudo make install
cd ..

# Install Yder
git clone https://github.com/babelouest/yder.git
cd yder/src
make
sudo make install
cd ..

# Install Ulfius
git clone https://github.com/babelouest/ulfius.git
cd ulfius/src
make
sudo make install
cd ..

# Install Hoel
git clone https://github.com/babelouest/hoel.git
cd hoel/src
make
sudo make install
cd ..

# Install Glewlwyd
git clone https://github.com/babelouest/glewlwyd.git
cd glewlwyd/src
make 
sudo make install
cd ..