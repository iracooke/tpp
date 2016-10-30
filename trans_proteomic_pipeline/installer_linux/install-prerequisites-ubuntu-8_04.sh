#!/bin/bash

# install prerequisites for building TPP on Ubuntu 8.04

# tested on Amazon EC2 AMI image ami-ef48af86, which is an Ubuntu 8.04 linux image, EC2 "small" size (alestic/ubuntu-8.04-hardy-base-20090418.manifest.xml)

# (libboost*1.35* is now included and build in the TPP)

if [ "$(id -u)" = "0" ] ; then # we're already root
   SUDO=""  
else
   SUDO="sudo"
 fi

$SUDO apt-get update

$SUDO apt-get -y install \
    libcurl4-openssl-dev \
    libfuse-dev \
    libxml2-dev \
    libc6-dev \
    gcc \
    subversion \
    make \
    expat \
    g++ \
    libexpat1-dev \
    zlib1g-dev \
    libbz2-dev \
    mcrypt \
    gnuplot \
    xsltproc \
    swig \
    libgd2-noxpm \
    libgd2-noxpm-dev

