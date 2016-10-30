#!/bin/bash

# install prerequisites for building TPP on Ubuntu 9.04
# original author: A. Quandt

sudo apt-get update

sudo apt-get install \
    g++ \
    subversion \
    libbz2-dev \
    swig \
    expat \
    libpng12-dev \
    gnuplot \
    libperl-dev \
    build-essential \
    libgd2-xpm \
	libfuse-dev \
	libcurl4-openssl-dev \
	libxml2-dev \
    libgd2-xpm-dev

