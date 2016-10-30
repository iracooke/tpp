INSPECTDIR := $(dir $(shell which inspect 2>/dev/null))

help :
	@echo "Ensure the inspect program is in your path and"
	@echo "then run 'make install'. To remove the patch"
	@echo "simply run 'make remove'."

install :
	@if [ ! -d "$(INSPECTDIR)" ]; then				\
	   echo "Error: inspect program not found in your path";	\
	   exit 1;							\
	fi
	@echo "Installing patch in $(INSPECTDIR)"
	mv $(INSPECTDIR)InspectToPepXML.py $(INSPECTDIR)InspectToPepXML.py.orig
	cp InspectToPepXML.py $(INSPECTDIR)

remove :
	@if [ ! -d "$(INSPECTDIR)" ]; then				\
	   echo "Error: inspect program not found in your path";	\
	   exit 1;							\
	fi
	@echo "Removing patch in $(INSPECTDIR)"
	mv $(INSPECTDIR)InspectToPepXML.py.orig $(INSPECTDIR)InspectToPepXML.py
