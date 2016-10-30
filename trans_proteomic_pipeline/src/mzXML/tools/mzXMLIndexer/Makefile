####################################################
#     Program: indexmzXML Makefile
#     Description: inspect and correct mzXML indexing
#     
#     Copyright (C) ISB
#
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License as published by the Free Software Foundation; either
#     version 2.1 of the License, or (at your option) any later version.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#
#
########

# get $(ARCH) value
SRC_ROOT= ../../../
include $(SRC_ROOT)Makefile.incl

APP = ${OBJ_ARCH}/indexmzXML

DEBUGFLAGS = -D NDEBUG

CXXFLAGS  = 
PROFILE  = #-pg

LDLIBS =    

VPATH = ../../common
IFLAGS += -I$(SRC_ROOT) $(BOOST_INCL) -I$(SRC_ROOT)../extern/gzstream $(ZLIB_INCL) -I$(TPP_MSDATA_READER_INCL)

LDFLAGS = $(PROFILE)

OFLAGS = -O2

MV  := mv -f
RM  := rm -f
SED := sed

CXXSOURCES = $(wildcard *.cpp)

CXXOBJECTS = $(patsubst %.cpp,${OBJ_ARCH}/%.o,$(CXXSOURCES))


CC = g++ ${CXXFLAGS}

.SUFFIXES: .cpp

${OBJ_ARCH}/%.o : %.cpp	
	$(CC)   -c -I$(SRC_ROOT)Parsers/ramp $(DEBUGFLAGS) $(OFLAGS) $(PROFILE) $(IFLAGS) -o $@ $<

all: ${OBJ_ARCH} $(APP)

$(APP) : $(COBJECTS) $(CXXOBJECTS) 
	$(CXX) $(OFLAGS) $(LDFLAGS) $^ $(OBJ_ARCH)/libtpp.a $(LDLIBS) $(SYSLIBS) -o $@



.PHONY: clean
clean:
	rm -rf $(CXXOBJECTS) $(COBJECTS) $(APP) gmon.out *~


