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

export PVM=/tools/pvm3/lib/pvm
export PVM_ROOT=/tools/pvm3/
export PVM_ARCH=LINUX

PROG=$(basename $0)

echo "$PROG: stopping pvm"

`echo "halt" | $PVM > /dev/null 2>&1`

exit 0
