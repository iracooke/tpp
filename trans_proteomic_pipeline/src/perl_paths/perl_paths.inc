# makefile snippet for tweaking perl scripts
# intended for include in perl_paths.mak (win32) and perl_paths.makefile (*nix)
# the main purpose of the wrapper makefiles is to specify XFORM 
# and define QT if needed (" for win32)

OUTDIR= $(OBJ_ARCH)


PL_TARGETS= $(QT)$(OUTDIR)/tpplib_perl.pm$(QT) $(QT)$(OUTDIR)/fileDownloader.pl$(QT) $(QT)$(OUTDIR)/run_marimba.pl$(QT)  $(QT)$(OUTDIR)/filterMRM.pl$(QT) $(QT)$(OUTDIR)/renamedat.pl$(QT) $(QT)$(OUTDIR)/min_sort.pl$(QT) $(QT)$(OUTDIR)/replaceall.pl$(QT) $(QT)$(OUTDIR)/updatepaths.pl$(QT) $(QT)$(OUTDIR)/ProphetModels.pl$(QT) $(QT)$(OUTDIR)/ProtProphModels.pl$(QT) $(QT)$(OUTDIR)/tpp_models.pl$(QT) $(QT)$(OUTDIR)/SSRCalc3.pl$(QT) $(QT)$(OUTDIR)/compareProts.pl$(QT) $(QT)$(OUTDIR)/compareProts_ClusterHM.pl$(QT) $(QT)$(OUTDIR)/get_prots.pl$(QT) $(QT)$(OUTDIR)/pepxmlcheck.pl$(QT) $(QT)$(OUTDIR)/compareProts3.pl$(QT) $(QT)$(OUTDIR)/check_env.pl$(QT) $(QT)$(OUTDIR)/findsubsets.pl$(QT) $(QT)$(OUTDIR)/mascotout.pl$(QT) $(QT)$(OUTDIR)/more_anal.pl$(QT) $(QT)$(OUTDIR)/more_pepanal.pl$(QT) $(QT)$(OUTDIR)/peptidexml_html2.pl$(QT) $(QT)$(OUTDIR)/pepxml2html.pl$(QT) $(QT)$(OUTDIR)/protxml2html.pl$(QT) $(QT)$(OUTDIR)/ProtXMLViewer.pl$(QT) $(QT)$(OUTDIR)/prot_wt_xml.pl$(QT) $(QT)$(OUTDIR)/show_dataset_derivation.pl$(QT) $(QT)$(OUTDIR)/show_help.pl$(QT) $(QT)$(OUTDIR)/show_nspbin.pl$(QT) $(QT)$(OUTDIR)/show_pepdataset_derivation.pl$(QT) $(QT)$(OUTDIR)/show_pipeline_help.pl$(QT) $(QT)$(OUTDIR)/show_search_params.pl$(QT) $(QT)$(OUTDIR)/show_sens_err.pl$(QT) $(QT)$(OUTDIR)/pepXML_std.xsl$(QT) $(QT)$(OUTDIR)/tpp_gui.pl$(QT) $(QT)$(OUTDIR)/tpp_gui_config.pl$(QT) $(QT)$(OUTDIR)/tandem-style.xsl$(QT) $(QT)$(OUTDIR)/show_tmp_pngfile.pl$(QT) $(QT)$(OUTDIR)/StatUtilities.pm$(QT) $(QT)$(OUTDIR)/calctppstat.pl$(QT) $(QT)$(OUTDIR)/exporTPP.pl$(QT) $(QT)$(OUTDIR)/createChargeFile.pl$(QT) $(QT)$(OUTDIR)/mergeCharges.pl$(QT) $(QT)$(OUTDIR)/fileDownloader.pl$(QT) $(QT)$(OUTDIR)/decoyFastaGenerator.pl$(QT) $(QT)$(OUTDIR)/Mayu.pl$(QT) 




all : $(PL_TARGETS)
	$(SET_X)

Scripts : all

clean :
	rm $(PL_TARGETS)

# kill the xformed copy of anything that fails syntax check
.DELETE_ON_ERROR :

$(QT)$(OUTDIR)/tpp_gui_config.pl$(QT) :$(QT)$(SRC_ROOT)../CGI/tpp_gui/tpp_gui_config.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../CGI/tpp_gui/tpp_gui_config.pl$(QT) > $(QT)$(OUTDIR)/tpp_gui_config.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/tpp_gui_config.pl$(QT)

$(QT)$(OUTDIR)/tpp_gui.pl$(QT) : $(QT)$(SRC_ROOT)../CGI/tpp_gui/tpp_gui.pl$(QT) $(QT)$(OUTDIR)/tpp_gui_config.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../CGI/tpp_gui/tpp_gui.pl$(QT) > $(QT)$(OUTDIR)/tpp_gui.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/tpp_gui.pl$(QT)

$(QT)$(OUTDIR)/pepxmlcheck.pl$(QT) : $(QT)$(SRC_ROOT)../CGI/tpp_gui/pepxmlcheck.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../CGI/tpp_gui/pepxmlcheck.pl$(QT) > $(QT)$(OUTDIR)/pepxmlcheck.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/pepxmlcheck.pl$(QT)

$(QT)$(OUTDIR)/check_env.pl$(QT) : $(QT)$(SRC_ROOT)../CGI/check_env.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../CGI/check_env.pl$(QT) > $(QT)$(OUTDIR)/check_env.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/check_env.pl$(QT)

$(QT)$(OUTDIR)/findsubsets.pl$(QT) : $(QT)$(SRC_ROOT)../CGI/findsubsets.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../CGI/findsubsets.pl$(QT) > $(QT)$(OUTDIR)/findsubsets.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/findsubsets.pl$(QT)

$(QT)$(OUTDIR)/mascotout.pl$(QT) : $(QT)$(SRC_ROOT)../CGI/mascotout.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../CGI/mascotout.pl$(QT) > $(QT)$(OUTDIR)/mascotout.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/mascotout.pl$(QT)

$(QT)$(OUTDIR)/more_anal.pl$(QT) : $(QT)$(SRC_ROOT)../CGI/more_anal.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../CGI/more_anal.pl$(QT) > $(QT)$(OUTDIR)/more_anal.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/more_anal.pl$(QT)

$(QT)$(OUTDIR)/more_pepanal.pl$(QT) : $(QT)$(SRC_ROOT)../CGI/more_pepanal.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../CGI/more_pepanal.pl$(QT) > $(QT)$(OUTDIR)/more_pepanal.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/more_pepanal.pl$(QT)

$(QT)$(OUTDIR)/peptidexml_html2.pl$(QT) : $(QT)$(SRC_ROOT)../CGI/peptidexml_html2.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../CGI/peptidexml_html2.pl$(QT) > $(QT)$(OUTDIR)/peptidexml_html2.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/peptidexml_html2.pl$(QT)

$(QT)$(OUTDIR)/pepxml2html.pl$(QT) : $(QT)$(SRC_ROOT)../CGI/pepxml2html.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../CGI/pepxml2html.pl$(QT) > $(QT)$(OUTDIR)/pepxml2html.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/pepxml2html.pl$(QT)

$(QT)$(OUTDIR)/protxml2html.pl$(QT) : $(QT)$(SRC_ROOT)../CGI/protxml2html.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../CGI/protxml2html.pl$(QT) > $(QT)$(OUTDIR)/protxml2html.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/protxml2html.pl$(QT)

$(QT)$(OUTDIR)/ProtXMLViewer.pl$(QT) : $(QT)$(SRC_ROOT)../CGI/ProtXMLViewer.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../CGI/ProtXMLViewer.pl$(QT) > $(QT)$(OUTDIR)/ProtXMLViewer.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/ProtXMLViewer.pl$(QT)

$(QT)$(OUTDIR)/prot_wt_xml.pl$(QT) : $(QT)$(SRC_ROOT)../CGI/prot_wt_xml.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../CGI/prot_wt_xml.pl$(QT) > $(QT)$(OUTDIR)/prot_wt_xml.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/prot_wt_xml.pl$(QT)

$(QT)$(OUTDIR)/show_dataset_derivation.pl$(QT) : $(QT)$(SRC_ROOT)../CGI/show_dataset_derivation.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../CGI/show_dataset_derivation.pl$(QT) > $(QT)$(OUTDIR)/show_dataset_derivation.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/show_dataset_derivation.pl$(QT)

$(QT)$(OUTDIR)/show_help.pl$(QT) : $(QT)$(SRC_ROOT)../CGI/show_help.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../CGI/show_help.pl$(QT) > $(QT)$(OUTDIR)/show_help.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/show_help.pl$(QT)

$(QT)$(OUTDIR)/show_nspbin.pl$(QT) : $(QT)$(SRC_ROOT)../CGI/show_nspbin.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../CGI/show_nspbin.pl$(QT) > $(QT)$(OUTDIR)/show_nspbin.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/show_nspbin.pl$(QT)

$(QT)$(OUTDIR)/show_pepdataset_derivation.pl$(QT) : $(QT)$(SRC_ROOT)../CGI/show_pepdataset_derivation.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../CGI/show_pepdataset_derivation.pl$(QT) > $(QT)$(OUTDIR)/show_pepdataset_derivation.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/show_pepdataset_derivation.pl$(QT)

$(QT)$(OUTDIR)/show_pipeline_help.pl$(QT) : $(QT)$(SRC_ROOT)../CGI/show_pipeline_help.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../CGI/show_pipeline_help.pl$(QT) > $(QT)$(OUTDIR)/show_pipeline_help.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/show_pipeline_help.pl$(QT)

$(QT)$(OUTDIR)/show_search_params.pl$(QT) : $(QT)$(SRC_ROOT)../CGI/show_search_params.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../CGI/show_search_params.pl$(QT) > $(QT)$(OUTDIR)/show_search_params.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/show_search_params.pl$(QT)

$(QT)$(OUTDIR)/show_sens_err.pl$(QT) : $(QT)$(SRC_ROOT)../CGI/show_sens_err.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../CGI/show_sens_err.pl$(QT) > $(QT)$(OUTDIR)/show_sens_err.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/show_sens_err.pl$(QT)

$(QT)$(OUTDIR)/show_tmp_pngfile.pl$(QT): $(QT)$(SRC_ROOT)../CGI/show_tmp_pngfile.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../CGI/show_tmp_pngfile.pl$(QT) > $(QT)$(OUTDIR)/show_tmp_pngfile.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/show_tmp_pngfile.pl$(QT)

$(QT)$(OUTDIR)/SSRCalc3.pl$(QT) : $(QT)$(SRC_ROOT)../perl/SSRCalc3.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../perl/SSRCalc3.pl$(QT) > $(QT)$(OUTDIR)/SSRCalc3.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/SSRCalc3.pl$(QT)

$(QT)$(OUTDIR)/run_marimba.pl$(QT) : $(QT)$(SRC_ROOT)../perl/run_marimba.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../perl/run_marimba.pl$(QT) > $(QT)$(OUTDIR)/run_marimba.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/run_marimba.pl$(QT)

$(QT)$(OUTDIR)/filterMRM.pl$(QT) : $(QT)$(SRC_ROOT)../perl/filterMRM.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../perl/filterMRM.pl$(QT) > $(QT)$(OUTDIR)/filterMRM.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/filterMRM.pl$(QT)

$(QT)$(OUTDIR)/renamedat.pl$(QT) : $(QT)$(SRC_ROOT)../perl/renamedat.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../perl/renamedat.pl$(QT) > $(QT)$(OUTDIR)/renamedat.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/renamedat.pl$(QT)

$(QT)$(OUTDIR)/min_sort.pl$(QT) : $(QT)$(SRC_ROOT)../perl/min_sort.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../perl/min_sort.pl$(QT) > $(QT)$(OUTDIR)/min_sort.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/min_sort.pl$(QT)

$(QT)$(OUTDIR)/replaceall.pl$(QT) : $(QT)$(SRC_ROOT)../perl/replaceall.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../perl/replaceall.pl$(QT) > $(QT)$(OUTDIR)/replaceall.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/replaceall.pl$(QT)

$(QT)$(OUTDIR)/tpplib_perl.pm$(QT) : $(QT)$(SRC_ROOT)../perl/tpplib_perl.pm$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../perl/tpplib_perl.pm$(QT) > $(QT)$(OUTDIR)/tpplib_perl.pm$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/tpplib_perl.pm $(QT)

$(QT)$(OUTDIR)/updatepaths.pl$(QT) : $(QT)$(SRC_ROOT)../perl/updatepaths.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../perl/updatepaths.pl$(QT) > $(QT)$(OUTDIR)/updatepaths.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/updatepaths.pl$(QT)

$(QT)$(OUTDIR)/ProphetModels.pl$(QT) : $(QT)$(SRC_ROOT)../perl/ProphetModels.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../perl/ProphetModels.pl$(QT) > $(QT)$(OUTDIR)/ProphetModels.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/ProphetModels.pl$(QT)

$(QT)$(OUTDIR)/decoyFastaGenerator.pl$(QT) : $(QT)$(SRC_ROOT)../perl/decoyFastaGenerator.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../perl/decoyFastaGenerator.pl$(QT) > $(QT)$(OUTDIR)/decoyFastaGenerator.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/decoyFastaGenerator.pl$(QT)


$(QT)$(OUTDIR)/ProtProphModels.pl$(QT) : $(QT)$(SRC_ROOT)../perl/ProtProphModels.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../perl/ProtProphModels.pl$(QT) > $(QT)$(OUTDIR)/ProtProphModels.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/ProtProphModels.pl$(QT)

$(QT)$(OUTDIR)/tpp_models.pl$(QT) : $(QT)$(SRC_ROOT)../perl/tpp_models.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../perl/tpp_models.pl$(QT) > $(QT)$(OUTDIR)/tpp_models.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/tpp_models.pl$(QT)

$(QT)$(OUTDIR)/compareProts.pl$(QT) : $(QT)$(SRC_ROOT)../perl/compareProts.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../perl/compareProts.pl$(QT) > $(QT)$(OUTDIR)/compareProts.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/compareProts.pl$(QT)

$(QT)$(OUTDIR)/compareProts_ClusterHM.pl$(QT) : $(QT)$(SRC_ROOT)../perl/compareProts_ClusterHM.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../perl/compareProts_ClusterHM.pl$(QT) > $(QT)$(OUTDIR)/compareProts_ClusterHM.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/compareProts_ClusterHM.pl$(QT)

$(QT)$(OUTDIR)/compareProts3.pl$(QT) : $(QT)$(SRC_ROOT)../perl/compareProts3.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../perl/compareProts3.pl$(QT) > $(QT)$(OUTDIR)/compareProts3.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/compareProts3.pl$(QT)

$(QT)$(OUTDIR)/pepXML_std.xsl$(QT) : $(QT)$(SRC_ROOT)../schema/pepXML_std.xsl$(QT) 
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../schema/pepXML_std.xsl$(QT) > $(QT)$(OUTDIR)/pepXML_std.xsl$(QT) 

$(QT)$(OUTDIR)/tandem-style.xsl$(QT): $(QT)$(SRC_ROOT)../schema/tandem-style.xsl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../schema/tandem-style.xsl$(QT) > $(QT)$(OUTDIR)/tandem-style.xsl$(QT)

$(QT)$(OUTDIR)/StatUtilities.pm$(QT) : $(QT)$(SRC_ROOT)../testing/StatUtilities.pm$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../testing/StatUtilities.pm$(QT) > $(QT)$(OUTDIR)/StatUtilities.pm$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/StatUtilities.pm$(QT)

$(QT)$(OUTDIR)/calctppstat.pl$(QT) : $(QT)$(OUTDIR)/StatUtilities.pm$(QT) $(QT)$(SRC_ROOT)../testing/calctppstat.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../testing/calctppstat.pl$(QT) > $(QT)$(OUTDIR)/calctppstat.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/calctppstat.pl$(QT)

$(QT)$(OUTDIR)/exporTPP.pl$(QT) : $(QT)$(SRC_ROOT)../perl/exporTPP.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../perl/exporTPP.pl$(QT) > $(QT)$(OUTDIR)/exporTPP.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/exporTPP.pl$(QT)

$(QT)$(OUTDIR)/get_prots.pl$(QT) : $(QT)$(SRC_ROOT)../perl/get_prots.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../perl/get_prots.pl$(QT) > $(QT)$(OUTDIR)/get_prots.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/get_prots.pl$(QT)

$(QT)$(OUTDIR)/createChargeFile.pl$(QT) : $(QT)$(SRC_ROOT)../perl/createChargeFile.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../perl/createChargeFile.pl$(QT) > $(QT)$(OUTDIR)/createChargeFile.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/createChargeFile.pl$(QT)

$(QT)$(OUTDIR)/mergeCharges.pl$(QT) : $(QT)$(SRC_ROOT)../perl/mergeCharges.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../perl/mergeCharges.pl$(QT) > $(QT)$(OUTDIR)/mergeCharges.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/mergeCharges.pl$(QT)

$(QT)$(OUTDIR)/fileDownloader.pl$(QT) : $(QT)$(SRC_ROOT)../perl/fileDownloader.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../perl/fileDownloader.pl$(QT) > $(QT)$(OUTDIR)/fileDownloader.pl$(QT)
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/fileDownloader.pl$(QT)

$(QT)$(OUTDIR)/Mayu.pl$(QT) : $(QT)$(SRC_ROOT)../extern/Mayu/Mayu.pl$(QT)
	$(PERL) -pe $(XFORM) < $(QT)$(SRC_ROOT)../extern/Mayu/Mayu.pl$(QT) > $(QT)$(OUTDIR)/Mayu.pl$(QT)
	mkdir -p $(QT)$(OUTDIR)/lib/Mayu/R
	cp $(SRC_ROOT)../extern/Mayu/r_lib/* $(QT)$(OUTDIR)/lib/Mayu/R
	cp $(SRC_ROOT)../extern/Mayu/lib/* $(QT)$(OUTDIR)/lib/Mayu 
	$(PERL) -c  -I  $(OUTDIR) $(QT)$(OUTDIR)/Mayu.pl$(QT)

