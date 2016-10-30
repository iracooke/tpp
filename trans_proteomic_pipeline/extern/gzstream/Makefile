# NOTE: this copy of the gzstream code has been altered by bpratt of Insilicos LLC: 
# a couple of tweaks for VC8 and to support different compression levels, including 
# none at all (write a regular file instead of a gzip file).  
# In particular this Makefile has tweaks to make it compatible with the TPP build system.
# Oct 2008

# ============================================================================
# gzstream, C++ iostream classes wrapping the zlib compression library.
# Copyright (C) 2001  Deepak Bandyopadhyay, Lutz Kettner
#
#some tweaks by Brian Pratt for TPP usage, esp. multiplatform builds
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
# ============================================================================
# 
# File          : Makefile
# Revision      : $Revision: 1.3 $
# Revision_date : $Date: 2001/10/04 15:09:28 $
# Author(s)     : Deepak Bandyopadhyay, Lutz Kettner
# 
# ============================================================================

# ----------------------------------------------------------------------------
# adapt these settings to your need:
# add '-DGZSTREAM_NAMESPACE=name' to CPPFLAGS to place the classes
# in its own namespace. Note, this macro needs to be set while creating
# the library as well while compiling applications based on it.
# As an alternative, gzstream.cpp and gzstream.h can be edited.
# ----------------------------------------------------------------------------

# CXX      = CC -n32 -LANG:std   # for SGI Irix 6.5, MIPSpro CC version 7.30
CXX      = g++   # for Linux RedHat 6.1, g++ version 2.95.2

CPPFLAGS = -I. -O $(ZLIB_INCL)
LDFLAGS  = -L. -lgzstream  $(ZLIB_LIB) $(STATIC_LIBSTDCPP)
AR       = ar cr

# ----------------------------------------------------------------------------
# plain simple rules to make and cleanup the library:
# make default;   compiles the library
# make test;      compiles and executes test. O.K. message marks success.
# make clean;     removes temporary files
# make cleanall;  removes temporary files, the library, and programs
# ----------------------------------------------------------------------------

default: $(BUILD_DIR)/libgzstream.a

test:    test_gzip test_gunzip
	$(BUILD_DIR)/test_gzip COPYING.LIB gz.tmp.gz
	gunzip gz.tmp.gz
	diff COPYING.LIB gz.tmp
	gzip gz.tmp
	$(BUILD_DIR)//test_gunzip gz.tmp.gz gz.tmp
	diff COPYING.LIB gz.tmp
	rm gz.tmp.gz gz.tmp
	# *** O.K. Test finished successfully. ***

$(ARCH)gzstream.o : gzstream.cpp gzstream.h
	${CXX} ${CPPFLAGS} -c -o $(ARCH)gzstream.o gzstream.cpp

$(ARCH)test_gzip.o : test_gzip.cpp gzstream.h
	${CXX} ${CPPFLAGS} -c -o $(ARCH)test_gzip.o test_gzip.C

$(ARCH)test_gunzip.o : test_gunzip.cpp gzstream.h
	${CXX} ${CPPFLAGS} -c -o $(ARCH)test_gunzip.o test_gunzip.C

$(BUILD_DIR)/libgzstream.a : $(ARCH)gzstream.o
	${AR} $(BUILD_DIR)/libgzstream.a $(ARCH)gzstream.o 

$(BUILD_DIR)/test_gzip : $(ARCH)test_gzip.o $(BUILD_DIR)/libgzstream.a
	${CXX} -o $(BUILD_DIR)/test_gzip $(ARCH)test_gzip.o ${LDFLAGS}

$(BUILD_DIR)/test_gunzip : $(ARCH)test_gunzip.o $(BUILD_DIR)/libgzstream.a
	${CXX} -o $(BUILD_DIR)/test_gunzip $(ARCH)test_gunzip.o ${LDFLAGS}

clean :
	rm $(ARCH)*.o

cleanall :
	rm $(ARCH)*.o $(BUILD_DIR)/libgzstream.a $(BUILD_DIR)/test_gzip $(BUILD_DIR)/test_gunzip

# ============================================================================
# EOF

