#!/bin/bash
#
# a little script in support of getting tandem onto and off of S3
#

if [ -e /etc/lsb-release ]; then
    # we might be on ubuntu
    . /etc/lsb-release
    # DISTRIB_ID and DISTRIB_RELEASE should now be set
elif [ -e /etc/redhat-release ]; then
    grep "CentOS" /etc/redhat-release > /dev/null
    if [ $? -eq 0 ]; then
		DISTRIB_ID="CentOS"
    else
		DISTRIB_ID="RedHat"
    fi
elif [ -e /etc/debian_version ]; then
	# Debian - just pretend its ubuntu
	DISTRIB_ID="Debian"
elif [ -e 'c:/Program Files' ]; then
	DISTRIB_ID="mingw"
else
    DISTRIB_ID="unknown"
fi

if [ "mpi" = "$1" ]; then
	OPEN_MPI=`mpiexec 2>&1 | grep open-mpi`
	if [ "" = "$OPEN_MPI" ]; then
		OPEN_MPI=""
	else
		OPEN_MPI=/open_mpi
	fi
else
	OPEN_MPI=""
fi

# ${$DISTRIB_ID,,} lowercases the distrib id for a nicely formed uri
S3DIR=$(echo $DISTRIB_ID | awk '{print tolower($0)}')
TANDEM_ARCH=$S3DIR/$(uname -m)$OPEN_MPI
echo $TANDEM_ARCH
