#!/bin/bash


# TPP build prerequisites for Centos 5.2



# tested on AMI image ami-cd52b6a4, rightscale-us/CentOS5_2_X86_64_V4_1_10.manifest.xml

# x86_64 packages are automatically selected and installed

# helpful: yum search

# todo: libfuse-dev for S3 access

# ? todo (referenced in Ubuntu install notes): libgd2-noxpm-dev libc6-dev

# note: xsltproc is provided in libxslt

sudo yum -y install \
    curl-devel \
    libxml2-devel \
    make \
    gcc \
    subversion \
    expat-devel \
    gd \
    gd-devel \
    expat-devel \
    zlib-devel \
    bzip2-devel \
    mcrypt \
    gnuplot \
    libxslt \
    swig \
    perl-XML-Parser \
    gcc-c++

