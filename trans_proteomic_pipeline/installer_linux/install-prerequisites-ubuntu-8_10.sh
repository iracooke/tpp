#!/bin/bash

# install prerequisites for building TPP on Ubuntu 8.10

# tested on Amazon EC2 AMI image ami-7806e211, which is an Ubuntu 8.10 linux image, EC2 "small" size (alestic/ubuntu-8.10-intrepid-base-20081030.manifest.xml)

# (libboost*1.35* is now included and build in the TPP)

sudo apt-get update

sudo apt-get -y install \
    libcurl4-openssl-dev \
    libfuse-dev \
    libxml2-dev \
    libc6-dev \
    gcc \
    subversion \
    make \
    expat \
    libgd2-xpm \
    libgd2-xpm-dev \
    libgd-tools \
    g++ \
    libexpat1-dev \
    libz-dev \
    libbz2-dev \
    mcrypt \
    gnuplot \
    xsltproc \
    swig

