#!/bin/bash
#
# Program: TPP HPC Tools
# Author:  Joe Slagel
#
# Copyright (C) 2012 by Joseph Slagel
# 
# This library is free software; you can redistribute it and/or             
# modify it under the terms of the GNU Lesser General Public                
# License as published by the Free Software Foundation; either              
# version 2.1 of the License, or (at your option) any later version.        
#                                                                           
# This library is distributed in the hope that it will be useful,           
# but WITHOUT ANY WARRANTY; without even the implied warranty of            
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         
# General Public License for more details.                                  
#                                                                           
# You should have received a copy of the GNU Lesser General Public          
# License along with this library; if not, write to the Free Software       
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
# 
# Institute for Systems Biology
# 1441 North 34th St.
# Seattle, WA  98103  USA
# jslagel@systemsbiology.org
#
# $Id: $
#

#
# usage: startpvm.sh <pe_hostfile>
#
# Invoke using the start proc field of a parallel enviroment
# configuration.  Use $pe_host as an allocation rule to 
# contain the slot allocation to a single host. 
#

set -e          # Tell bash to exit if any statement fails

export PVM_ROOT=/tools/pvm3/
export PVM_ARCH=LINUX

PVM=/tools/pvm3/lib/pvm

PROG=$(basename $0)

# Is PVM daemon already running?
PVMID=$(pgrep -u $USER pvmd3) || PVMID=""     # use of || to avoid trap of err
if [ "$PVMID" != "" ]; then
   echo "$PROG: error PVM appears to be already running $PVMID" >&2
   exit 100
fi

# Cleanup any leftover pid files
find /tmp -maxdepth 1 -user $USER -name 'pvm*' -print | xargs rm -vf

# What resources where assigned by Sun Grid Engine?
PE_HOSTFILE=$1
PE_NSLOTS=$2
if [ ! -r "$PE_HOSTFILE" ]; then
   echo "$PROG: missing/unable to read pe_hostfile $PE_HOSTFILE" >&2
   exit 100
fi
if [ -z "$PE_NSLOTS" ]; then
   echo "$PROG: missing number of pe slots" >&2
   exit 100
fi

HOSTS=$(cut -f1 -d ' ' $PE_HOSTFILE)
echo "$PROG: pe assigned hosts = $HOSTS"
echo "$PROG: pe assigned $PE_NSLOTS slots"

# Write out a host file for pvm.  Our version of sequest determines the
# number of slaves to run using the "speed" variable in pvm so set it
# in the hosts file
#
# NOTE: until rhosts file is fixed the pvm server only starts on
#       the node I'm on
HOSTFILE=$(mktemp)
echo "* sp=8" >> $HOSTFILE
for HOST in $HOSTS; do 
   echo "$HOST sp=$PE_NSLOTS" >> $HOSTFILE
done

# Start pvm
echo "$PROG: starting pvm"
$PVM $HOSTFILE <<CMDS > /dev/null
   conf
   quit
CMDS
echo "PVM hostfile $HOSTFILE"
cat $HOSTFILE
rm -f $HOSTFILE
exit 0
