#
# Makefile for building TPP's variant of X!Tandem
#
# $ Id: $
#

# Location for binary/object files
# TODO: Remove loading all of Makefile.incl here and make it so it can be run
#       standalone as opposed only through TPP's Makefile
ifeq ($(BUILD_DIR),)
   BUILD_DIR := ../bin
   OBJDIR    := $(BUILD_DIR)
else
   OBJDIR    := $(BUILD_DIR)/tandem-bin
   # for TPP install we want these files as well
   SUPPORTFILES= $(BUILD_DIR)/tandem_params.xml $(BUILD_DIR)/isb_default_input_kscore.xml $(BUILD_DIR)/isb_default_input_native.xml $(BUILD_DIR)/taxonomy.xml
   include $(SRC_ROOT)Makefile.incl
endif

EXECUTABLE = $(BUILD_DIR)/tandem

CXX = g++
CXXFLAGS = $(if $(DEBUG),$(DEBUG),-O2) -DGCC4 -DPLUGGABLE_SCORING $(OSFLAGS) $(ZLIB_INCL) -I $(EXPAT_INCL) -I $(HDF5_INCL) -I $(MZPARSER_INCL)
LINKCC = $(CXX)

LDFLAGS = -lm
ifneq ($(MSYSTEM),)
   LDFLAGS += -static -static-libstdc++ 
   # MinGW - Xtandem uses the define "MSVC" to differentiate which thread 
   # library to use so we can't easily choose to use mthreads without using all
   # of MSVC, therefore just use pthreads under Windows+MinGW.  Oh, and don't be
   # fooled by the static link options as pthreads is dynamically linked 
   # regardless.
   LDFLAGS += -lpthread
   # Actually as of pthreads-w32-2.9.1 you can statically link but you need to
   # compile with PTW32_STATIC_LIB defined. If you don't use this option the
   # gcc linker will also complain unless you link against the import library,
   # pthreads.dll
   CXXFLAGS += -DPTW32_STATIC_LIB
else
   LDFLAGS += -lpthread
endif

ifneq ($(ARCH),darwin)
   # on os x 10.7, make will balk at "-lz" as a build dependency
   LDFLAGS += $(ZLIB_LIB)
endif
LDFLAGS += $(EXPAT_LIB) $(HDF5_LIB) $(MZPARSER_LIB)


# -- Build rules.

.SUFFIXES: .o .cpp

SRCS := $(wildcard *.cpp)
OBJS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(wildcard *.cpp))
DEPS := $(patsubst %.o,%.d,$(OBJS))

all: $(EXECUTABLE) $(SUPPORTFILES)

$(OBJDIR) :
	mkdir -p $(OBJDIR)

#
# Compile each source file, generating the dependencies if necessary using gcc
# instead of the old fashioned way suggested in the GNU Makefile manual See 
# http://stackoverflow.com/questions/8025766/makefile-auto-dependency-generation
# and the GNU gcc manual for more information.
#
$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(if $(MAKEDEPS), \
	   $(CXX) -MM -MT $(OBJDIR)/$*.o $(CXXFLAGS) $< > $(OBJDIR)/$*.d \
	)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Link program
$(EXECUTABLE): $(OBJS)
	$(LINKCC) $(CXXFLAGS) -o $(EXECUTABLE) $(OBJS) $(MZPARSER_LIB) $(HDF5_LIB) $(LDFLAGS) $(ZLIB_LIB) $(USER_OBJS)

# Generate TPP support files
$(BUILD_DIR)/tandem_params.xml: ../bin/tandem_params.xml
	cp $< $@
	sed 's?_DEFAULT_INPUT_LOCATION_/?$(subst \,\\,$(TANDEM_PARAMETERS_INSTALL_DIR))?g' $@ > ${@}.updated
	mv ${@}.updated $@

$(BUILD_DIR)/isb_default_input_kscore.xml: ../bin/isb_default_input_kscore.xml
	cp $< $@

$(BUILD_DIR)/isb_default_input_native.xml: ../bin/isb_default_input_native.xml
	cp $< $@

$(BUILD_DIR)/taxonomy.xml: ../bin/taxonomy.xml
	cp $< $@


#
# -- Deletes all files that are normally created by the build.
#
.PHONY: clean
clean:
	rm -f $(OBJS) $(EXECUTABLE) $(DEPS) $(SUPPORTFILES)


#
# -- Output information about targets
#
.PHONY: explain
explain:
	@echo "The following info represents the program:"
	@echo "Final exec name: $(EXECUTABLE)"
	@echo "Source files:       $(SRCS)"
	@echo "Object files:       $(OBJS)"
	@echo "Dep files:          $(DEPS)"



# Include any pre-made source code header dependencies
$(if $(MAKEDEPS),-include $(DEPS))

# -*- makefile -*-
# vim:set ts=8 sw=8 sts=8 noet:# Include premade dependency files if they exist
