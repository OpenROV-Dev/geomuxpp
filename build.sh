#!/bin/bash
set -e

# Install prereqs
mkdir -p deb
pushd deb
wget http://openrov-software-nightlies.s3-us-west-2.amazonaws.com/jessie/zmq/openrov-zmq_1.0.0-1~1_armhf.deb
wget http://openrov-software-nightlies.s3-us-west-2.amazonaws.com/jessie/ffmpeg/openrov-ffmpeg-lib_3.0.2~14_armhf.deb
wget http://openrov-software-nightlies.s3-us-west-2.amazonaws.com/jessie/geocamera-libs/openrov-geocamera-libs_1.0.0-1~35.16a26aa_armhf.deb
wget http://openrov-software-nightlies.s3-us-west-2.amazonaws.com/jessie/json/openrov-nlohmann-json_1.0.0-1~1_armhf.deb

dpkg -i openrov-zmq_1.0.0-1~1_armhf.deb
dpkg -i openrov-ffmpeg-lib_3.0.2~14_armhf.deb
dpkg -i openrov-geocamera-libs_1.0.0-1~35.16a26aa_armhf.deb
dpkg -i openrov-nlohmann-json_1.0.0-1~1_armhf.deb

popd
rm -rf deb

# Make output folder
mkdir -p output/usr/bin

# Cleanup prior build
make clean_all

# Build
make all

# Copy bin to output
cp bin/release/geomuxpp output/usr/bin

