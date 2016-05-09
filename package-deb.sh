#!/bin/bash
set -ex

#Install Pre-req
apt-get update
apt-get install rubygems -y

gem install fpm
export DIR=${PWD}
export PACKAGE="openrov-geomuxpp"

ARCH=`uname -m`
if [ ${ARCH} = "armv7l" ]
then
  ARCH="armhf"
fi

rm -rf output/*

./build.sh

export PACKAGE_VERSION=1.0.0-1~${BUILD_NUMBER}

#package
cd $DIR

fpm -f -m info@openrov.com -s dir -t deb -a $ARCH \
	-n ${PACKAGE} \
	-v ${PACKAGE_VERSION} \
	--description "OpenROV Geomux Interface Daemon" \
	-C ${DIR}/output ./

rm -rf output/*
