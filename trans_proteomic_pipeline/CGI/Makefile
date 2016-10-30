#
# note this assumes the perl_paths target in the parent makefile has been run,
# which puts copies of the perl scripts in the right build dir and does the path
# translations as needed (it's a normal part of "make all" so you needn't worry).
#
# if this seems a bit convoluted and tacked on, it is - such is life in a mature codebase.
# but it's better than what we did before, which was alter the files right there in the source
# tree, which made source code change control a nightmare.
#

# get ARCH
SRC_ROOT= ../src/
include $(SRC_ROOT)Makefile.incl

all:
	echo Perl scripts, nothing to build.

install:
	cp -f $(OBJ_ARCH)/show_tmp_pngfile.pl ${CGI_DIR}
	cp -f $(OBJ_ARCH)/findsubsets.pl ${CGI_DIR}
	cp -f $(OBJ_ARCH)/mascotout.pl ${CGI_DIR}
	cp -f $(OBJ_ARCH)/more_anal.pl ${CGI_DIR}
	cp -f $(OBJ_ARCH)/more_pepanal.pl ${CGI_DIR}
	cp -f $(OBJ_ARCH)/peptidexml_html2.pl ${CGI_DIR}
	cp -f $(OBJ_ARCH)/pepxml2html.pl ${CGI_DIR}
	cp -f $(OBJ_ARCH)/pepxml2html.pl ${BIN_DIR}
	cp -f $(OBJ_ARCH)/prot_wt_xml.pl ${CGI_DIR}
	cp -f $(OBJ_ARCH)/protxml2html.pl ${CGI_DIR}
	cp -f $(OBJ_ARCH)/protxml2html.pl ${BIN_DIR}
	cp -f $(OBJ_ARCH)/ProtXMLViewer.pl ${CGI_DIR}
	cp -f $(OBJ_ARCH)/show_dataset_derivation.pl ${CGI_DIR}
	cp -f $(OBJ_ARCH)/show_help.pl ${CGI_DIR}
	cp -f $(OBJ_ARCH)/show_nspbin.pl ${CGI_DIR}
	cp -f $(OBJ_ARCH)/show_pepdataset_derivation.pl ${CGI_DIR}
	cp -f $(OBJ_ARCH)/show_pipeline_help.pl ${CGI_DIR}
	cp -f $(OBJ_ARCH)/show_search_params.pl ${CGI_DIR}
	cp -f $(OBJ_ARCH)/show_sens_err.pl ${CGI_DIR}
	cp -f $(OBJ_ARCH)/check_env.pl ${CGI_DIR}
	cp -f $(OBJ_ARCH)/tpp_gui.pl ${CGI_DIR}
	if [ ! -e ${CGI_DIR}/tpp_gui_config.pl ]; then cp -f tpp_gui/tpp_gui_config.pl ${CGI_DIR} ; fi;
	cp -f tpp_gui/tpp_gui.css ${CGI_DIR}
	mkdir -p ${CGI_DIR}/images
	cp -f tpp_gui/*.png ${CGI_DIR}/images
	cp -f tpp_gui/*.jpg ${CGI_DIR}/images
	cp -f tpp_gui/*.gif ${CGI_DIR}/images
	cp -f ../installer_win32/*.ico ${CGI_DIR}/images
	mkdir -p ${CGI_USERS_DIR}
	cp -rf tpp_gui/users/* ${CGI_USERS_DIR}
	mkdir -p ${HTML_DIR}js
	chmod a+x ${HTML_DIR}js
	cp -f tpp_gui/js/*.js ${HTML_DIR}js