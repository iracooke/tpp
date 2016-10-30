#define COMMAND_LEN 1000000

/*
Program       : xinteract
Author        : Andrew Keller <akeller@systemsbiology.org>
Date          : 11.27.02
Revision      : $Id: xinteract.cxx 6590 2014-08-22 20:21:52Z slagelwa $

Run tools pipeline

Copyright (C) 2003 Andrew Keller

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Andrew Keller
Institute for Systems Biology
401 Terry Avenue North 
Seattle, WA  98109  USA
akeller@systemsbiology.org 
*/

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <string>

#include <sys/stat.h>
#include "common/constants.h"
using namespace std;

#include "common/TPPVersion.h" // contains version number, name, revision

#ifdef WINDOWS_NATIVE // MSVC or MinGW
#include "common/wglob.h"	//glob for windows
#else
#include <glob.h>		//glob for real
#endif

#include "common/Array.h"
#include "common/util.h"
#include <time.h>
#include "Parsers/Parser/TagListComparator.h" // for REGRESSION_TEST_CMDLINE_ARG defn

const char* getPeptideProphetOption(char o);
const char* getInterProphetOption(char o);
const char* getPTMProphetOption(char o);
const char* getProteinProphetOption(char o);
const char* getSampleEnzyme(char e);
void cleanup_and_move_tmpfile(const char *output_fname,const char *final_output_fname);

bool isValidCommand(const char *command) {
  bool result = true;	// default to trusting PATH to do its job
  if (getIsInteractiveMode()) { // TPP vs LabKey usage style
    // we don't quite trust the PATH in TPP (has poor installers)...
    struct stat statbuf;
    result= (0==stat(command,&statbuf));
    if (!result) {
      char *test = new char[strlen(command)+5];
      struct stat statbuf;
      bool endq = false;

      if (command[0] == '"' || command[0] == '\'') {
	endq = (command[1] && (command[strlen(command)-1] != command[0]));
	command++;
      }
      strcpy(test,command);
      if (endq)
	test[strlen(test) - 1] = '\0';
#ifdef WINDOWS_NATIVE
      result = (0 == stat(test,&statbuf));
      if (!result)
	strcat(test,".exe");
#endif
      if (!result)
	result = (0 == stat(test,&statbuf));
      delete[] test;
    }
  }
  return result;
}

// handy string for writing a log
std::string glob_logString;

// tell the user what we're going to do, then do it
void doCommand(const char *command, Boolean force) {
  cout << endl << "running: \"" << command << "\"" << endl;
  time_t then;
  time(&then);
  int result = tpplib_system(command); // like system(), but handles multiple commands for win32
  if (result) {
    cout << endl << "command \"" << command << "\" exited with non-zero exit code: " << result << endl;
    if (force) {
      cout << "WARNING - the job might be incomplete" << endl;
    }
    else {
      cout << "QUIT - the job is incomplete" << endl;
      exit(WEXITSTATUS(result));
    }
  }
  time_t now;
  time(&now);

  cout << "command completed in " << difftime(now,then) <<" sec " << endl;
  glob_logString += std::string(command);
  char buf[256];
  snprintf(buf,sizeof(buf)," %d sec\n",(int)difftime(now,then));
  glob_logString += std::string(buf);
}

// tell the user what we're going to do, then do it
FILE *doCommandPipe(const char *command) {
  cout << endl << "running: \"" << command << "\"" << endl;
  glob_logString += std::string(command);
  glob_logString += "\n";
  return tpplib_popen(command,"r");
}

// concatenate appropriate parameter quoting
void paramqcat(char* command) {
  strcat(command, getCmdlineQuoteChar());
}

void runConverter(char* fileArg, char* command, 
		  char* sequest2xml, char* mascot2xml, char* comet2xml,
		  char* sample_enzyme, char* maldi, 
		  char* masstype, char* conversion_param, 
		  char* conversion_database, Boolean mascot_pI, const char *testArg) {
  if(strlen(fileArg) > 5 && ! strcmp(fileArg+strlen(fileArg)-5, ".html")) {
    // call Sequest2XML
    if(conversion_param == NULL) {
      if(strlen(sample_enzyme) > 0)
	sprintf(command, "%s %s -E%s %s %s", sequest2xml, fileArg, sample_enzyme, maldi, masstype);
      else
	sprintf(command, "%s %s %s %s", sequest2xml, fileArg, maldi, masstype);
    }
    else { // use specified sequest.params file
      if(strlen(sample_enzyme) > 0)
	sprintf(command, "%s %s %s -E%s %s %s", sequest2xml, fileArg, conversion_param, sample_enzyme, maldi, masstype);
      else
	sprintf(command, "%s %s %s %s %s", sequest2xml, fileArg, conversion_param, maldi, masstype);
    }
    doCommand(command, false);
    command[0] = 0;
  }
  else if(strlen(fileArg) > 4 && ! strcmp(fileArg+strlen(fileArg)-4, ".dat")) {
    // call Mascot2XML
    if(conversion_database == NULL) {
      cout << "error, no database specified for Mascot2XML conversion" << endl;
      exit(1);
    }
    if(strlen(sample_enzyme) > 0)
      sprintf(command, "%s %s -D%s -E%s%s", mascot2xml, fileArg, conversion_database, sample_enzyme, mascot_pI?" -pI":"");
    else 
      sprintf(command, "%s %s -D%s%s", mascot2xml, fileArg, conversion_database, mascot_pI?" -pI":"");
    if (testArg) { // regression test stuff
      strcat(command," ");
      strcat(command,testArg);
    }
    doCommand(command, false);
    strcpy(strrchr(fileArg,'.'),get_pepxml_dot_ext()); // replace filename.ext
    command[0] = 0;
  }
  else if(strlen(fileArg) > 11 && ! strcmp(fileArg+strlen(fileArg)-11, ".cmt.tar.gz")) {
    // call Comet2XML
    if(strlen(sample_enzyme) > 0)
      sprintf(command, "%s -E%s %s", comet2xml, sample_enzyme, fileArg);
    else 
      sprintf(command, "%s %s", comet2xml, fileArg);
    doCommand(command, false);
    command[0] = 0;
  }
  else if(!hasValidPepXMLFilenameExt(fileArg)) {
    cout << "error, cannot parse input file " << fileArg << endl;
    exit(1);
  }
}

static void index_pepXML_file(const char *fname) {
  char* fileToIndex =  makeFullPath(fname);
  std::string indexCmd(CGI_FULL_BIN);
  indexCmd+= "PepXMLViewer.cgi -I ";
  indexCmd+=fileToIndex;
  doCommand(indexCmd.c_str(), false);
  free(fileToIndex);
}

static void run_TPP_models(const char *fname) {
  char *command = new char[COMMAND_LEN];
  sprintf(command, "%s%s", LOCAL_BIN, "tpp_models.pl");

  if(!isValidCommand(command)) {
    cout << "error: file " << command << " does not exist" << endl;
    return;
  }
  strcat(command, " ");
  paramqcat(command);
  strcat(command, fname);
  paramqcat(command);

  doCommand(command, false);

  free(command);
}


int main(int argc, char** argv) {
  hooks_tpp handler(argc,argv); // installdir issues etc
  if(argc < 2) {
    const char *ext=get_pepxml_dot_ext();
    cout << " " << argv[0] << " (" << szTPPVersionInfo << ")" << endl;
    cout << " usage: xinteract (generaloptions) (-Oprophetoptions) (-iiprophetoptions) (-Mptmprophetoptions) (-Xxpressoptions) (-Aasapoptions) (-L<conditionfile>libraoptions) xmlfile1 xmlfile2 ...." << endl << endl;
    cout << " generaloptions:" << endl;
    cout << "            For developers:" << endl;
    cout << "                 -t  [run regression test against a previously derived result]" << endl;
    cout << "                 -t! [learn results for regression test]" << endl;
    cout << "                 -t# [run regression test, do not stop on test failure]" << endl << endl;
    cout << "            For users:" << endl;
    cout << "                 -Nmyfile" << ext << " [write output to file 'myfile" << ext << "']" << endl;
    cout << "                 -R fix protein names in OMSSA data" << endl;
    cout << "                 -G record collision energy in pepXML" << endl;
    cout << "                 -V record compensation voltage (FAIMS) in pepXML" << endl;
    cout << "                 -PREC record precursor intensity in pepXML" << endl;
    cout << "                 -nI [do not run Interact (convert to pepXML only)]" << endl;
    cout << "                 -nP [do not run PeptideProphet]" << endl;
    cout << "                 -nR [do not run get all proteins corresponding to degenerate peptides from database]" << endl;
    cout << "                 -p0 [do not discard search results with PeptideProphet probabilities below 0.05]" << endl;    
    cout << "                 -x<num> [number of extra PeptideProphet interations; default <num>=20]" << endl;    
    cout << "                 -I<num> [ignore charge <num>+]" << endl;    
    cout << "                 -d<tag> [use decoy hits to pin down the negative distribution." << endl
         << "                          the decoy protein names must begin with <tag> (whitespace is not allowed)]" << endl;    
    cout << "                 -D<database_path> [specify path to database]" << endl;
    cout << "                 -c<conservative_level> [specify how conservative the model is to be in number of standard deviations from negative mean " << endl
	 << "                                         to allow positive model to cover (default 0, higher is more conservative)]" << endl;
    cout << "                 -PPM [use PPM instead of daltons in Accurate Mass Model]" << endl;
    cout << "                 -E<experiment_label> [used to commonly label all spectra belonging to one experiment (required by iProphet)]" << endl;    
    cout << "                 -l<num> [minimum peptide length considered in the analysis (default 7)]" << endl;    
    cout << "                 -T<database type> [specify 'AA' for amino acid, 'NA' for nucleic acid (default 'AA')]" << endl;    
    cout << "                 -a<data_path> [specify absolute path to data directory]" << endl;    
    cout << "                 -p<num> [filter results below PeptideProphet probability <num>; default <num>=0.05]" << endl;
    cout << "                 -mw [calculate protein molecular weights]" << endl;
    cout << "                 -MONO [calculate monoisotopic peptide masses during conversion to pepXML]" << endl;
    cout << "                 -AVE [calculate average peptide masses during conversion to pepXML]" << endl;
    cout << "                 -THREADS=<num> [specify maximum number of threads to use]" << endl;

    cout << "                 -eX [specify sample enzyme]" << endl;
    cout << "                     -eT [specify sample enzyme = Trypsin]" << endl;
    cout << "                     -eS [specify sample enzyme = StrictTrypsin]" << endl;
    cout << "                     -eC [specify sample enzyme = Chymotrypsin]" << endl;
    cout << "                     -eR [specify sample enzyme = RalphTrypsin]" << endl;
    cout << "                     -eA [specify sample enzyme = AspN]" << endl;
    cout << "                     -eG [specify sample enzyme = GluC]" << endl;
    cout << "                     -eB [specify sample enzyme = GluC Bicarb]" << endl;
    cout << "                     -eM [specify sample enzyme = CNBr]" << endl;
    cout << "                     -eD [specify sample enzyme = Trypsin/CNBr]" << endl;
    cout << "                     -e3 [specify sample enzyme = Chymotrypsin/AspN/Trypsin]" << endl;
    cout << "                     -eE [specify sample enzyme = Elastase]" << endl;
    cout << "                     -eK [specify sample enzyme = LysC / Trypsin_K (cuts after K not before P)]" << endl;
    cout << "                     -eL [specify sample enzyme = LysN (cuts before K)]" << endl;
    cout << "                     -eP [specify sample enzyme = LysN Promisc (cuts before KASR)]" << endl;
    cout << "                     -eN [specify sample enzyme = Nonspecific or None]" << endl;
    cout << endl;
    
    cout << " PeptideProphet options [following the '-O']:" << endl;
    cout << "                 i [use icat information in PeptideProphet]" << endl;
    cout << "                 f [do not use icat information in PeptideProphet]" << endl;
    cout << "                 g [use N-glyc motif information in PeptideProphet]" << endl;
    cout << "                 H [use Phospho information in PeptideProphet]" << endl;
    cout << "                 m [maldi data]" << endl;
    cout << "                 I [use pI information in PeptideProphet]" << endl;
    cout << "                 R [use Hydrophobicity / RT information in PeptideProphet]" << endl;
    cout << "                 F [force the fitting of the mixture model, bypass automatic mixture model checks]" << endl;
    cout << "                 A [use accurate mass binning in PeptideProphet]" << endl;
    cout << "                 w [warning instead of exit with error if instrument types between runs is different]" << endl;
    cout << "                 x [exclude all entries with asterisked score values in PeptideProphet]" << endl;
    cout << "                 l [leave alone all entries with asterisked score values in PeptideProphet]" << endl;
    cout << "                 n [use hardcoded default initialization parameters of the distributions]" << endl;
    cout << "                 P [use Non-parametric model, can only be used with decoy option]" << endl;
    cout << "                 N [do not use the NTT model]" << endl;
    cout << "                 M [do not use the NMC model]" << endl;
    cout << "                 k [do not use the mass model]" << endl;
    cout << "                 o [optimize f-value function f(dot,delta) using PCA (applies only to SpectraST)]" << endl;
    cout << "                 G [use Gamma Distribution to model the Negatives (applies only to X!Tandem data)]" << endl;
    cout << "                 E [only use Expect Score as the Discriminant(applies only to X!Tandem data, " << endl 
	 << "                    helpful for data with homologous top hits e.g. phospho or glyco)]" << endl;
    cout << "                 d [report decoy hits with a computed probability based on the model learned]" << endl;
    cout << "                 p [run ProteinProphet afterwards]" << endl;
    cout << "                 t [do not create png data plot]" << endl;
    cout << "                 u [do not assemble protein groups in ProteinProphet analysis]" << endl;
    cout << "                 s [do not use Occam's Razor in ProteinProphet analysis to " << endl;
    cout << "                    derive the simplest protein list to explain observed peptides]" << endl << endl;

    cout << " iProphet options [run iProphet on the PeptideProphet result with options that follow the '-i']:" << endl;
    cout << "                 p [run ProteinProphet on the iProphet results]" << endl;
    cout << "                 P [do not use number of sibling peptides model]" << endl;
    cout << "                 R [do not use number replicate spectra model]" << endl;
    cout << "                 I [do not use number sibling ions model]" << endl;
    cout << "                 M [do not use number sibling mods model]" << endl;
    cout << "                 S [do not use numbe of sibling searches model]" << endl;
    cout << "                 E [do not use number of sibling MS/MS runs model]" << endl << endl;

    cout << " PTMProphet options [run PTMProphet on the iProphet result with options that follow the '-M' (e.g. -M-STY,79.9663-MZTOL=0.4)]:" << endl;
    cout << "                 -{<amino acids, n, or c>,<mass_shift>,...}  [specify mod masses (e.g. -STY,79.9663,K,114.0429,M,15.9949)] " << endl;
    cout << "                 -MZTOL=<number>                             [Use specified +/- mz tolerance on site specific ions (default=0.1 dalton)] " << endl;
    cout << "                 -NOUPDATE                                   [Don't update modification_info tags in pepXML] " << endl << endl;

    cout << " xpressoptions [will run XPRESS analysis with any specified options that follow the '-X']: " << endl;
    cout << "                 -m<num>        change XPRESS mass tolerance (default=1.0)" << endl;
    cout << "                 -a             tolerance specified by -m is in ppm (default=Daltons)" << endl;
    cout << "                 -n<str>,<num>  change XPRESS residue mass difference for <str> to <num> (default=9.0)" << endl;
    cout << "                 -b             heavy labeled peptide elutes before light labeled partner" << endl;
    cout << "                 -F<num>        fix elution peak area as +-<num> scans (<num> optional, default=5) from peak apex" << endl;
    cout << "                 -c<num>        change minimum number of chromatogram points needed for quantitation (default=5)" << endl;
    cout << "                 -p<num>        number of 13C isotopic peaks to add to precursor chromatogram (default=1)" << endl;
    cout << "                 -L             for ratio, set/fix light to 1, vary heavy" << endl;
    cout << "                 -H             for ratio, set/fix heavy to 1, vary light" << endl;
    cout << "                 -M             for 15N metabolic labeling; ignore all other parameters, assume" << endl;
    cout << "                                IDs are normal and quantify w/corresponding 15N heavy pair" << endl;
    cout << "                 -N             for 15N metabolic labeling; ignore all other parameters, assume" << endl;
    cout << "                                IDs are 15N heavy and quantify w/corresponding 14N light pair" << endl;
    cout << "                 -O             for 13C metabolic labeling; ignore all other parameters, assume" << endl;
    cout << "                                IDs are normal and quantify w/corresponding 13C heavy pair" << endl;
    cout << "                 -P             for 13C metabolic labeling; ignore all other parameters, assume" << endl;
    cout << "                                IDs are 13C heavy and quantify w/corresponding 12C light pair" << endl;
    cout << "                 -i             also export intensities and intensity based ratio" << endl;
    cout << "                 -l             label free mode: stats on precursor ions only, no ratios" << endl;
    cout << "                                only relevant label-free parameters are -m, -c, and -p" << endl << endl;

    cout << " asapoptions [will run ASAPRatio analysis with any specified options that follow the '-A']: " << endl;
    cout << "                 -l<str>    change labeled residues (default='C')" << endl;
    cout << "                 -b         heavy labeled peptide elutes before light labeled partner" << endl;
    cout << "                 -r<num>    range around precusor m/z to search for peak (default 0.5)" << endl;
    cout << "                 -f<num>    areaFlag set to num (ratio display option)" << endl;
    cout << "                 -S         static modification quantification (i.e. each run is either" << endl;
    cout << "                            all light or all heavy)" << endl;
    cout << "                 -F         use fixed scan range for light and heavy" << endl;
    cout << "                 -C         quantitate only the charge state where the CID was made" << endl;
    cout << "                 -B         return a ratio even if the background is high" << endl;
    cout << "                 -Z         set all background to zero" << endl;
    cout << "                 -m<str>    specified label masses (e.g. M74.325Y125.864), only relevant for " << endl;
    cout << "                            static modification quantification " << endl << endl;

    cout << " libraoptions [will run Libra Quantitation analysis with any specified options that follow the '-L']: " << endl;
//  cout << "                 -<num>    normalization channel (for protein level quantitation)" << endl << endl;

    cout << " refreshparser options (disabled by -nR switch)" << endl;
    cout << "                 -PREV_AA_LEN=<length>   set the number of previous AAs recorded for a peptide hit (default 1)" << endl;
    cout << "                 -NEXT_AA_LEN=<length>   set the number of following AAs recorded for a peptide hit (default 1)" << endl;
    cout << "                 -RESTORE_NONEXISTENT_IF_PREFIX=<str>  for proteins which starts with <str> and not found in refresh database," << endl;
    cout << "                                                       keep original protein names instead of NON_EXISTENT" << endl << endl;

    cout << " examples: " << endl;
    cout << " xinteract *" << ext << " [combines together data in all pepXML files into 'interact" << ext << "', then runs PeptideProphet]" << endl;
    cout << " xinteract -Ndata" << ext << " *" << ext << " [same as above, but results are written to 'data" << ext << "']" << endl;
    cout << " xinteract -Ndata" << ext << " -X -Op *" << ext << " [same as above, but run XPRESS analysis in its default mode, then" << endl;
    cout << "      ProteinProphet]" << endl;
    cout << " xinteract -X -A file1" << ext << " file2" << ext << " [combines together data in file1" << ext << " and file2" << ext << " into 'interact" << ext << "'" << endl;
    cout << "      and then runs XPRESS (in its default mode) and ASAPRatio (in its default mode)]" << endl;
    cout << " xinteract -X-nC,6.0 -A file1" << ext << " file2" << ext << " [same as above, but specifies that cysteine label has a heavy/light" << endl;
    cout << "      mass difference of 6.0]" << endl;

    cout << " xinteract -X -A-lDE-S file1" << ext << " file2" << ext << " [sampe as above, but specifies for ASAP to run in static mode " << endl;
    cout << "                                            with labeled residues D and E]" << endl;
    cout << " xinteract -Lmyconditionfile.xml -Op file1" << ext << " file2" << ext << " [run libra quantitiation after PeptideProphet using myconditionfile.xml" << endl;

    cout << "------------------------------------------------------------------------------------------------------------" << endl;
    exit(1);
  } else {
    cout << endl << argv[0] << " ("<<szTPPVersionInfo<<")"<<endl;
  }

  int debug = 0;
  time_t then;
  time(&then);

  char *interact = new char[1000];
  sprintf(interact, "%s%s", LOCAL_BIN, "InteractParser");
  char *refresh = new char[1000];
  sprintf(refresh, "%s%s", LOCAL_BIN, "RefreshParser");
  char *database = new char[1000];
  sprintf(database, "%s%s", LOCAL_BIN, "DatabaseParser");
  char *pepproph = new char[1000];
  sprintf(pepproph, "%s%s", LOCAL_BIN, "PeptideProphetParser");
  char *iproph = new char[1000];
  sprintf(iproph, "%s%s", LOCAL_BIN, "InterProphetParser");
  char *ptmproph = new char[1000];
  sprintf(ptmproph, "%s%s", LOCAL_BIN, "PTMProphetParser");
  char *pepprophmods = new char[1000];
  sprintf(pepprophmods, "%s%s", LOCAL_BIN, "ProphetModels.pl -i");
  char *protprophmods = new char[1000];
  sprintf(protprophmods, "%s%s", LOCAL_BIN, "ProtProphModels.pl -i");
  char *xpress_pep = new char[1000];
  sprintf(xpress_pep, "%s%s", LOCAL_BIN, "XPressPeptideParser");
  char *asap_pep = new char[1000];
  sprintf(asap_pep, "%s%s", LOCAL_BIN, "ASAPRatioPeptideParser");
  char *protproph = new char[1000];
  sprintf(protproph, "%s%s", LOCAL_BIN, "ProteinProphet");
  char *xpress_prot = new char[1000];
  sprintf(xpress_prot, "%s%s", LOCAL_BIN, "XPressProteinRatioParser");
  char *asap_prot = new char[1000];
  sprintf(asap_prot, "%s%s", LOCAL_BIN, "ASAPRatioProteinRatioParser");
  char *asap_pvalue = new char[1000];
  sprintf(asap_pvalue, "%s%s", LOCAL_BIN, "ASAPRatioPvalueParser");
  char *sequest2xml = new char[1000];
  sprintf(sequest2xml, "%s%s", LOCAL_BIN, "Sequest2XML");
  char *mascot2xml = new char[1000];
  sprintf(mascot2xml, "%s%s", LOCAL_BIN, "Mascot2XML");
  char *comet2xml = new char[1000];
  sprintf(comet2xml, "%s%s", LOCAL_BIN, "Comet2XML");
  char *libra_pep = new char[1000];
  sprintf(libra_pep, "%s%s", LOCAL_BIN, "LibraPeptideParser");
  char *libra_prot = new char[1000];
  sprintf(libra_prot, "%s%s", LOCAL_BIN, "LibraProteinRatioParser");

  char *command = new char[COMMAND_LEN];

  std::string default_output_pep("interact");
  default_output_pep += get_pepxml_dot_ext();
  std::string default_output_ipep("interact.ipro");
  default_output_ipep += get_pepxml_dot_ext();
  std::string default_output_ptmpep("interact.ptm.ipro");
  default_output_ptmpep += get_pepxml_dot_ext();
  std::string default_output_prot("interact");
  default_output_prot += get_protxml_dot_ext();
  std::string default_output_iprot("interact.ipro");
  default_output_iprot += get_protxml_dot_ext();
  std::string default_output_ptmprot("interact.ptm.ipro");
  default_output_iprot += get_protxml_dot_ext();

  char *peptide_prophet_options = new char[COMMAND_LEN];
  peptide_prophet_options[0] = 0;
  char *inter_prophet_options = new char[COMMAND_LEN];
  inter_prophet_options[0] = 0;
  char *protein_prophet_options = new char[COMMAND_LEN];
  protein_prophet_options[0] = 0;
  char *protein_iprophet_options = new char[COMMAND_LEN];
  protein_iprophet_options[0] = 0;
  char *asap_options = new char[COMMAND_LEN];
  char *ptm_options = new char[COMMAND_LEN];
  asap_options[0] = 0;
  ptm_options[0] = 0;
  char *xpress_options = new char[COMMAND_LEN];
  xpress_options[0] = 0;
  char *xpress_prot_opts = new char[COMMAND_LEN];
  xpress_prot_opts[0] = 0;
  char *cmd_pep = NULL;
  char *cmd_prot = NULL;

  int peptide_prophet = 1;
  int inter_prophet = 0;
  int ptm_prophet = 0;
  int protein_prophet = 0;
  int protein_iprophet = 0;

  int zero = 0;
  int xpress = 0;
  int asap = 0;
  int cmd1 = 0;
  int cmd2 = 0;
  char maldi[10];
  maldi[0] = 0;
  char masstype[10];
  masstype[0] = 0;

  int k, libra = 0;
  char *libra_params = new char[1000];
  libra_params[0] = 0;
//char libra_norm_channel[100]; // protein level
//libra_norm_channel[0] = 0;
  int no_interact = 0;
  int delude = 0; // don't refresh 

  int prot_wt = 0;

  char sample_enzyme[100];
  sample_enzyme[0] = 0;

  if(! peptide_prophet) {
    protein_prophet = 0;
    protein_iprophet = 0;
    inter_prophet = 0;
  }

  Boolean mascot_pI = false;

  char *prev_aa_len_arg = NULL;
  char *next_aa_len_arg = NULL;

  std::string restore_nonexistent_prefix("");

  char *output_pep = new char[COMMAND_LEN];
  char *output_prot = new char[COMMAND_LEN];
  char *output_ipep = new char[COMMAND_LEN];
  char *output_iprot = new char[COMMAND_LEN];
  char *output_ptmpep = new char[COMMAND_LEN];
  char *output_ptmprot = new char[COMMAND_LEN];

  // we may do our work in a tmpdir, these are the
  // final destinations for the results
  std::string final_output_pep;
  std::string final_output_prot;
  std::string final_output_ipep;
  std::string final_output_iprot;
  std::string final_output_ptmpep;
  std::string final_output_ptmprot;

  const int line_width = 2000;
  char dbase[line_width];

  char* conversion_param = NULL;
  char* conversion_database = NULL;

  char* exp_lbl = NULL;
  Boolean prec_intens = false;
  Boolean fix_prot_names = false;
  Boolean collision_eng = false;
  Boolean comp_volt = false;

  int min_peplen = 7;
  char* buf = NULL;
  char* conversion_dbtype = NULL;
  char* interact_datapath = NULL;
  char *testArg = NULL;
  FILE* pipe = NULL;

  strcpy(output_pep, default_output_pep.c_str());
  strcpy(output_prot, default_output_prot.c_str());
  strcpy(output_ipep, default_output_ipep.c_str());
  strcpy(output_iprot, default_output_iprot.c_str());
  strcpy(output_ptmpep, default_output_ptmpep.c_str());
  strcpy(output_ptmprot, default_output_ptmprot.c_str());

  Array<char*>* infiles = new Array<char*>;
  char* next = NULL;

  const char* nextoption;
  
  std::string decoy_tag("");

  // check for test args first
  for(k = 1; k < argc; k++) {
    if(! strncmp(argv[k], REGRESSION_TEST_CMDLINE_ARG,
		 strlen(REGRESSION_TEST_CMDLINE_ARG))) { // regression test
      int testargbaselen=strlen(REGRESSION_TEST_CMDLINE_ARG);
      if (strchr(testArg=argv[k],REGRESSION_TEST_CMDLINE_ARG_MOD_LEARN)) {
	cout << " learning regression test..." << endl;
	testargbaselen++;
      } else {
	cout << " performing regression test";
	if (strchr(testArg,REGRESSION_TEST_CMDLINE_ARG_MOD_FORCE)) {
	  cout << " (" << REGRESSION_TEST_CMDLINE_ARG_MOD_FORCE << " option in use, will continue in case of error)";
	  testargbaselen++; // skip past the "force continuation" marker
	}
	cout << "..." << endl;
      }
      if ((int)strlen(testArg)==testargbaselen) {
	// no test name given, make one from commandline args
	std::string args(testArg);
	for (int i=1;i<argc;i++) {
	  if (i!=k) {
	    args += std::string(argv[i]);
	    args += std::string(" ");
	  }
	}
	testArg = strdup(args.c_str());
	for (char *cp=testArg+testargbaselen;*cp;cp++) {
	  // remove any cygdrive stuff for best crossplatform checking
	  char *cygdrive;
	  while ( (cygdrive=strstr(cp,"/cygdrive/")) ) {
	    char *bump=cygdrive+strlen("/cygdrive/"); // copy from here
	    cygdrive[0] = *bump; // drive letter
	    cygdrive[1] = ':'; // make it look windowsy
	    memmove(cygdrive+2,bump+1,strlen(bump));
	  }
	  if (strchr("*?:;+/\\[] \t",*cp)) { // valid filename char?
	    *cp = (*(cp+1))?'_':0; // no trailing _
	  }
	}
      } else {
	testArg = strdup(argv[k]); // use test name as given
      }
    }
  }
  // check out arguments
  for(k = 1; k < argc; k++) {
    if(strlen(argv[k]) > 1 && argv[k][0] == '-') { // have an option

      if(! strcmp(argv[k]+1, "nP"))
	peptide_prophet = 0;
      else if(! strncmp(argv[k]+1, "PREV_AA_LEN=",12)) {
	prev_aa_len_arg =argv[k]+1;
      } else if(! strncmp(argv[k]+1, "NEXT_AA_LEN=",12)) {
	next_aa_len_arg =argv[k]+1;
      } else if(! strncmp(argv[k]+1, "RESTORE_NONEXISTENT_IF_PREFIX=",30)) {
	restore_nonexistent_prefix = "RESTORE_NONEXISTENT_IF_PREFIX=";
	restore_nonexistent_prefix += (argv[k] + 31);
      }
      else if(! strcmp(argv[k]+1, "nI"))
	no_interact = 1;
      else if(! strcmp(argv[k]+1, "nR"))
	delude = 1;
      else if(! strcmp(argv[k]+1, "P0")) 
	zero = 1;
      else if(! strcmp(argv[k]+1, "mw")) 
	prot_wt = 1;
      else if(strlen(argv[k]+1) > 1 && argv[k][1] == 'e')  // get enzyme
	strcpy(sample_enzyme, getSampleEnzyme(argv[k][2]));
      else if(strlen(argv[k]+1) > 1 && argv[k][1] == 'p') { // extract minprob, or mascot pI arg
	if ('I'==argv[k][2]) {
	  mascot_pI = true;
	} else {
	  if(strlen(peptide_prophet_options) > 0)
	    strcat(peptide_prophet_options, " ");
	  strcat(peptide_prophet_options, "MINPROB=");
	  strcat(peptide_prophet_options, argv[k]+2);
	}
      }
      else if(strlen(argv[k]+1) > 1 && argv[k][1] == 'c') { 
	if(strlen(peptide_prophet_options) > 0)
	  strcat(peptide_prophet_options, " ");
	strcat(peptide_prophet_options, "CLEVEL=");
	strcat(peptide_prophet_options, argv[k]+2);
      }// extract extra iterations
      else if(strlen(argv[k]+1) > 1 && argv[k][1] == 'x') { // extract extra iterations
	if(strlen(peptide_prophet_options) > 0)
	  strcat(peptide_prophet_options, " ");
	strcat(peptide_prophet_options, "EXTRAITRS=");
	strcat(peptide_prophet_options, argv[k]+2);
      }
      else if(strlen(argv[k]+1) > 1 && argv[k][1] == 'I') { // extract ignore charge
	if(strlen(peptide_prophet_options) > 0)
	  strcat(peptide_prophet_options, " ");
	strcat(peptide_prophet_options, "IGNORECHG=");
	strcat(peptide_prophet_options, argv[k]+2);
      }
      else if(strlen(argv[k]+1) > 1 && argv[k][1] == 'd') { // extract decoy tag
	if(strlen(peptide_prophet_options) > 0)
	  strcat(peptide_prophet_options, " ");
	strcat(peptide_prophet_options, "DECOY=");
	strcat(peptide_prophet_options, argv[k]+2);
	decoy_tag = argv[k]+2 ;
	decoy_tag = "\"" + decoy_tag + "\"";
      }
      else if(argv[k][1] == 'O') { // peptideprophet options
	for(int j = 2; j < (int) strlen(argv[k]); j++)
	  if(argv[k][j] == 'p')
	    protein_prophet = 1;
	  else { // peptideprophet or proteinprophet option
	    nextoption = getPeptideProphetOption(argv[k][j]);
	    if(nextoption != NULL) {
	      if(strlen(peptide_prophet_options) > 0)
		strcat(peptide_prophet_options, " ");
	      strcat(peptide_prophet_options, nextoption);
	    } // pepproph option
	    else { // prot
	      nextoption = getProteinProphetOption(argv[k][j]);
	      if(nextoption != NULL) {
		if(strlen(protein_prophet_options) > 0)
		  strcat(protein_prophet_options, " ");
		strcat(protein_prophet_options, nextoption);
	      }
	    }
	  } // pep or proph
      }
      else if(argv[k][1] == 'i') { // iprophet options
	inter_prophet = 1;
	for(int j = 2; j < (int) strlen(argv[k]); j++) {
	  if(argv[k][j] == 'p') {
	    protein_iprophet = 1;
	  }
	  nextoption = getInterProphetOption(argv[k][j]);
	  if(nextoption != NULL) {
	    if(strlen(inter_prophet_options) > 0)
	      strcat(inter_prophet_options, " ");
	    strcat(inter_prophet_options, nextoption);
	  } // pepproph option
	  else { // prot
	    nextoption = getProteinProphetOption(argv[k][j]);
	    if(nextoption != NULL) {
	      if(strlen(protein_iprophet_options) > 0)
		strcat(protein_iprophet_options, " ");
	      strcat(protein_iprophet_options, nextoption);
	    }
	  }
	} // pep or proph
      }
      else if(argv[k][1] == 'L') { // libra
	libra = 1;
	int index = 2;
	int libra_index = 0;
	while(index < (int) strlen(argv[k])) {

	  if(argv[k][index] == '-') { // get channel
//	    strcpy(libra_norm_channel, argv[k] + index + 1);
	    index = strlen(argv[k]); // done
	  }
	  else 
	    libra_params[libra_index++] = argv[k][index++];

	} // while
	libra_params[libra_index] = 0;
      }
      else if(argv[k][1] == 'X') { // xpress options
	xpress = 1;
	// get additional options here....
	// must add spaces before each -
	int index = 2;
	int xpress_ind = 0;
	while(index < (int) strlen(argv[k])) {	
	  if(xpress_ind > 0 && argv[k][index] == '-' &&  argv[k][index-1] != ',')
	    xpress_options[xpress_ind++] = ' ';
	  if (argv[k][index] == '~') {
	    xpress_options[xpress_ind++] = '-';
	  }
	  else {
	    xpress_options[xpress_ind++] = argv[k][index];
	  }
	  index++;
	}
	xpress_options[xpress_ind] = 0;
      }
      else if(argv[k][1] == 'A') { // asap options
	asap = 1;
	// get additional options here....
	// must add spaces before each -
	int index = 2;
	int asap_ind = 0;
	while(index < (int) strlen(argv[k])) {
	  if(asap_ind > 0 && argv[k][index] == '-')
	    asap_options[asap_ind++] = ' ';
	  asap_options[asap_ind++] = argv[k][index++];
	}
	asap_options[asap_ind] = 0;
      }

      else if(argv[k][1] == 'C') { // command
        // custom command
	if (argv[k][2] == '1') {
	  cmd1 = 1;
          cmd_pep = new char[strlen(argv[k] + 3) + 1];
          strcpy(cmd_pep, argv[k] + 3);
        } else if (argv[k][2] == '2') {
	  cmd2 = 1;
          cmd_prot = new char[strlen(argv[k] + 3) + 1];
          strcpy(cmd_prot, argv[k] + 3);
        }
      }
      else if(argv[k][1] == 'N') { // rename output
	strcpy(output_pep, argv[k]+2); // must end in PEPXML_FILENAME_DOTEXT or .xml (backward compatible)
	strcpy(output_ipep, argv[k]+2); // must end in PEPXML_FILENAME_DOTEXT or .xml (backward compatible)
	const char* dotext = hasValidPepXMLFilenameExt(output_pep);
	// now derive a protxml filename
	if(dotext != NULL) { // has an OK pepxml .ext, copy and replace with protxml .ext
	  strcpy(output_prot, output_pep);
	  strcpy(output_prot+(strlen(output_pep)-strlen(dotext)), get_protxml_dot_ext());
	  strcpy(output_ipep, output_pep);
	  strcpy(output_ipep+(strlen(output_ipep)-strlen(dotext)), ".ipro");
	  strcat(output_ipep, dotext);
	  strcpy(output_iprot, output_pep);
	  strcpy(output_iprot+(strlen(output_pep)-strlen(dotext)), ".ipro");
	  strcat(output_iprot, get_protxml_dot_ext());
	  strcpy(output_ptmpep, output_pep);
	  strcpy(output_ptmpep+(strlen(output_ptmpep)-strlen(dotext)), ".ptm.ipro");
	  strcat(output_ptmpep, dotext);
	  strcpy(output_ptmprot, output_pep);
	  strcpy(output_ptmprot+(strlen(output_pep)-strlen(dotext)), ".ptm.ipro");
	  strcat(output_ptmprot, get_protxml_dot_ext());

	  if (isDotGZ(output_pep)) {  // was output file spec'd as a gzip file?
	    if (!isDotGZ(output_prot)) {
	      strcat(output_prot,".gz"); // then gzip the protxml as well
	    }
	    if (!isDotGZ(output_iprot)) {
	      strcat(output_iprot,".gz"); // then gzip the protxml as well
	    }
	    if (!isDotGZ(output_ptmprot)) {
	      strcat(output_ptmprot,".gz"); // then gzip the protxml as well
	    }
	  } else {
	    if (isDotGZ(output_prot)) {
	      *strrchr(output_prot,'.')=0; // do not gzip the protxml
	    }
	    if (isDotGZ(output_iprot)) {
	      *strrchr(output_iprot,'.') = 0; //  do not gzip the protxml
	    }
	    if (isDotGZ(output_ptmprot)) {
	      *strrchr(output_ptmprot,'.') = 0; //  do not gzip the protxml
	    }
	  }
	}
	else {
	  strcpy(output_pep, "interact-");
	  strcat(output_pep, argv[k]+2); // must end in PEPXML_FILENAME_DOTEXT

	  strcpy(output_prot, output_pep);
	  strcpy(output_iprot, output_pep);
	  strcpy(output_ptmprot, output_pep);

	  strcat(output_pep, get_pepxml_dot_ext());
	  strcat(output_prot, get_protxml_dot_ext());

	  strcat(output_iprot, ".ipro"); // must end in PEPXML_FILENAME_DOTEXT
	  strcat(output_iprot, get_protxml_dot_ext());

	  strcat(output_ptmprot, ".ptm.ipro"); // must end in PEPXML_FILENAME_DOTEXT
	  strcat(output_ptmprot, get_protxml_dot_ext());

	  strcpy(output_ipep, "interact-");
	  strcat(output_ipep, argv[k]+2); // must end in PEPXML_FILENAME_DOTEXT
	  strcat(output_ipep, ".ipro"); // must end in PEPXML_FILENAME_DOTEXT
	  strcat(output_ipep, get_pepxml_dot_ext());

	  strcpy(output_ptmpep, "interact-");
	  strcat(output_ptmpep, argv[k]+2); // must end in PEPXML_FILENAME_DOTEXT
	  strcat(output_ptmpep, ".ptm.ipro"); // must end in PEPXML_FILENAME_DOTEXT
	  strcat(output_ptmpep, get_pepxml_dot_ext());

	  cout << " naming output file " << output_pep << endl;
	}
      } // rename
      else if(! strcmp(argv[k]+1, "PREC")) { 
	prec_intens = true;
      }
      else if(! strcmp(argv[k]+1, "PPM")) { // conversion paramter
	cout << " PPM mode in Accurate Mass Model ..." << endl;
	if(strlen(peptide_prophet_options) > 0)
	  strcat(peptide_prophet_options, " ");
	strcat(peptide_prophet_options, "PPM");
      }
      else if(strstr(argv[k]+1, "THREADS=")!=NULL) { // conversion paramter
	cout << " using " << argv[k]+1 << " for iProphet..." << endl;
	if(strlen(inter_prophet_options) > 0)
	  strcat(inter_prophet_options, " ");
	strcat(inter_prophet_options, argv[k]+1);
      }
      else if(argv[k][1] == 'P') { // conversion paramter
	conversion_param = new char[strlen(argv[k])+1];
	strcpy(conversion_param, argv[k]);
      }
      else if(argv[k][1] == 'G') { // conversion paramter
	collision_eng = true;
      }
      else if(argv[k][1] == 'V') { // conversion paramter
	comp_volt = true;
      }
      else if(argv[k][1] == 'D') { // 
	conversion_database = new char[strlen(argv[k])+1];
	strcpy(conversion_database, &argv[k][2]);
      }
      else if(argv[k][1] == 'E') { // 
	exp_lbl = new char[strlen(argv[k])+1];
	strcpy(exp_lbl, &argv[k][2]);
      }
      else if(argv[k][1] == 'R') { // 
	fix_prot_names = true;;
      }
      else if(argv[k][1] == 'l') { // 
	buf = new char[strlen(argv[k])+1];
	strcpy(buf, &argv[k][2]);
	min_peplen = atoi(buf);
	delete [] buf;
      }
      else if(argv[k][1] == 'T') { // 
	conversion_dbtype = new char[strlen(argv[k])+1];
	strcpy(conversion_dbtype, &argv[k][2]);
      }
      else if(argv[k][1] == 'a') { // interact absolute path to data directory
	interact_datapath = new char[strlen(argv[k])+1];
	strcpy(interact_datapath, &argv[k][2]);
      }
      else if(! strcmp(argv[k]+1, "MALDI")) { // conversion paramter
	cout << " maldi mode..." << endl;
	strcpy(maldi, "-M");
      }
      else if(! strcmp(argv[k]+1, "MONO")) { // conversion paramter
	cout << " monoisotopic peptide mass mode..." << endl;
	strcpy(masstype, "-m");
      }
      else if(! strcmp(argv[k]+1, "AVE")) { // conversion paramter
	cout << " average peptide mass mode..." << endl;
	strcpy(masstype, "-a");
      }
      else if(argv[k][1] == 'M') { // asap options
	ptm_prophet = 1;
	// get additional options here....
	// must add spaces before each -
	int index = 2;
	int ptm_ind = 0;
	while(index < (int) strlen(argv[k])) {
	  if(ptm_ind > 0 && argv[k][index+1] == '-') {
	    ptm_options[ptm_ind++] = ' ';
	    index++;
	  }
	  else if (ptm_ind == 0 && argv[k][index] == '-') {
	    ptm_options[ptm_ind++] = argv[k][++index];
	  }
	  else {
	    ptm_options[ptm_ind++] = argv[k][++index];
	  }
	}
	ptm_options[ptm_ind] = 0;
      }
      else if(! strncmp(argv[k], REGRESSION_TEST_CMDLINE_ARG,
			strlen(REGRESSION_TEST_CMDLINE_ARG))) { // regression test
	// already handled it
      }
      else {
	cout << "nothing found for " << argv[k] << endl;
      }
    }
    else { // argv[k][0] != '-', take it as an input filename
      char *fileArg = new char[strlen(argv[k])+strlen(get_pepxml_dot_ext())+1001];

      if (strchr(argv[k], '*')!=NULL || strchr(argv[k], '?')!=NULL || 
	  strchr(argv[k], '[')!=NULL || strchr(argv[k], '{')!=NULL) {

	glob_t g;
	glob(argv[k], 0, NULL, &g);

	for (int i = 0; i < g.gl_pathc; i++) {
	  runConverter((g.gl_pathv)[i], command, 
		       sequest2xml, mascot2xml, comet2xml, 
		       sample_enzyme, maldi, 
		       masstype, conversion_param, 
		       conversion_database, mascot_pI, testArg);
	}

      }
      else {
	strcpy(fileArg, argv[k]);
	runConverter(fileArg, command, 
		     sequest2xml, mascot2xml, comet2xml,
		     sample_enzyme, maldi, 
		     masstype, conversion_param, 
		     conversion_database, mascot_pI, testArg);
      }
      strcpy(fileArg,argv[k]);

      if(strlen(fileArg) > 5 && ! strcmp(fileArg+strlen(fileArg)-5, ".html")) {
	strcpy(strrchr(fileArg,'.'),get_pepxml_dot_ext()); // replace filename.ext
      }
      else if(strlen(fileArg) > 4 && ! strcmp(fileArg+strlen(fileArg)-4, ".dat")) {
	strcpy(strrchr(fileArg,'.'),get_pepxml_dot_ext()); // replace filename.ext
      }
      else if(strlen(fileArg) > 11 && ! strcmp(fileArg+strlen(fileArg)-11, ".cmt.tar.gz")) {
	strcpy(strstr(fileArg,".cmt.tar.gz"),get_pepxml_dot_ext()); // replace filename.ext
      }
      else if(!hasValidPepXMLFilenameExt(fileArg)) {
	cout << "error, cannot parse input file " << fileArg << endl;
	exit(1);
      }

      infiles->insertAtEnd(fileArg);
    }
  }

  // have we been told to do our work in a temp dir?
  // save the actual output names in case we have
  final_output_pep = output_pep;
  final_output_prot = output_prot;
  final_output_ipep = output_ipep;
  final_output_iprot = output_iprot;
  final_output_ptmpep = output_ptmpep;
  final_output_ptmprot = output_ptmprot;

  std::string tmpdir;
  if (getWebserverTmpPath() && !testArg) { // do this in temp dir if so configured, unless in regression test mode
    // make a temp subdir, at the end we'll clean up everything in it for path refs etc
    tmpdir = getWebserverTmpPath();
    tmpdir += "XXXXXX";
    safe_fclose(FILE_mkstemp(tmpdir)); // create then close a uniquely named file
    verified_unlink(tmpdir.c_str()); // it's actually a file at this point, kill to replace
    pushWebserverTmpPath(tmpdir.c_str()); // so child programs share same idea of tmp
    // create a subdirectory tree below tmpdir as a cue to programs that want to drop
    // image files and the like in the eventual permanent directory
    std::string outpath(output_pep);
    if (findRightmostPathSeperator(outpath)<0) {
      char cwd[512];
      safepath_getcwd(cwd,sizeof(cwd));
      outpath = cwd;
      outpath += "/";
    }
    if (!getWebserverRoot()) {
      cout << "WEBSERVER_ROOT environment variable is not set\n";
      cout << "This will cause problems for many TPP components, even commandline mode.\n";
      cout << "Please set WEBSERVER_ROOT=<some_writeable_directory> then try again.\n";
      exit(1);
    }
    if (!strncasecmp(outpath.c_str(),getWebserverRoot(),strlen(getWebserverRoot()))) {
      // construct a tmpdir name using the part of output path that's below WEBSERVER_ROOT
      std::string subdir(outpath);
      int len = (int)strlen(getWebserverRoot());
      subdir = subdir.substr(len,findRightmostPathSeperator(subdir)-len);
      tmpdir += "/";
      tmpdir += subdir;
    }
    std::string cmd("mkdir -p ");
    cmd += getCmdlineQuoteChar();
    cmd += tmpdir;
    cmd += getCmdlineQuoteChar();
    if (tpplib_system(cmd.c_str())) {
      cout << "could not create temp dir " << tmpdir << ", working in data directory" << endl;
      tmpdir.erase();
    } else {
      std::string cmd("chmod a+wr ");
      cmd += getCmdlineQuoteChar();
      cmd += tmpdir;
      cmd += getCmdlineQuoteChar();
      if (tpplib_system(cmd.c_str())) {
	cout << "could not set permissions on temp dir " << tmpdir << ", working in data directory" << endl;
	tmpdir.erase();
      } else {
	tmpdir+="/";
	replace_path(output_pep,COMMAND_LEN,tmpdir.c_str());
	replace_path(output_prot,COMMAND_LEN,tmpdir.c_str());
	replace_path(output_ipep,COMMAND_LEN,tmpdir.c_str());
	replace_path(output_iprot,COMMAND_LEN,tmpdir.c_str());
	replace_path(output_ptmpep,COMMAND_LEN,tmpdir.c_str());
	replace_path(output_ptmprot,COMMAND_LEN,tmpdir.c_str());
      }
    }
  }

  if(no_interact) {
    if (prev_aa_len_arg || next_aa_len_arg) {
      cout << "conflicting options: \"convert only\" option prevents RefreshParser from setting prev_aa and/or next_aa lengths" << endl;
    }
    cerr << "INFO: -nI option enabled InteractParser will not be utilized." << endl;

    //return 0; // done
  }
  else {
    if (tmpdir.length()) {
      pushWebserverRoot(tmpdir.c_str()); // fool children into thinking tmpdir is in root path
    }

    //
    // run interact, possibly as regression test
    //
    strcpy(command, interact);

    if(! debug && ! isValidCommand(command)) {
      cout << "error: file " << command << " does not exist" << endl;
      exit(1);
    }
    if (testArg) {
      strcat(command, " ");
      strcat(command, testArg);
    }
    strcat(command, " ");
    paramqcat(command);
    strcat(command, output_pep);
    paramqcat(command);

    for(k = 0; k < infiles->length(); k++) {
      strcat(command, " ");
      paramqcat(command);
      strcat(command, (*infiles)[k]);
      delete[] (*infiles)[k];
      paramqcat(command);
    }
    delete infiles;

    if (interact_datapath != NULL) {
      strcat(command, " -a");
      paramqcat(command);
      strcat(command, interact_datapath);
      paramqcat(command);
    }
    if (conversion_database != NULL) {
      strcat(command, " -D");
      paramqcat(command);
      strcat(command, conversion_database);
      paramqcat(command);
    }
    if (exp_lbl != NULL) {
      strcat(command, " -X");
      paramqcat(command);
      strcat(command, exp_lbl);
      paramqcat(command);
    }
    if (fix_prot_names) {
      strcat(command, " -P");
    }
    if (collision_eng) {
      strcat(command, " -G");
    }
    if (prec_intens) {
      strcat(command, " -I");
    }
    if (comp_volt) {
      strcat(command, " -V");
    }

    strcat(command, " -L");
    paramqcat(command);
    // sprintf(command, "%s%d", command, min_peplen);  not a good idea! which "command" wins?
    sprintf(command+strlen(command), "%d", min_peplen);
    paramqcat(command);

    if (conversion_dbtype != NULL) {
      strcat(command, " -T");
      paramqcat(command);
      strcat(command, conversion_dbtype);
      paramqcat(command);
    }
    if (strlen(sample_enzyme) > 0) {
      strcat(command, " -E");
      paramqcat(command);
      strcat(command, sample_enzyme);
      paramqcat(command);
    }

    if(debug)
      cout << command << endl;
    else
      doCommand(command, false);
  }

  // Run DatabaseParser, followed by RefreshParser
  if( delude) {
    if (prev_aa_len_arg || next_aa_len_arg) {
      cout << "conflicting options: \"delude\" option prevents RefreshParser from setting prev_aa and/or next_aa lengths" << endl;
    }
  } else {
    // now refresh
    // HENRY: if database already given by -D option, don't run DatabaseParser
    if (conversion_database) {
      cout << "\nUse database specified by -D option: " << conversion_database << ". Skip DatabaseParser step." << endl;
      strcpy(dbase, conversion_database);

    } else {
      // Run DatabaseParser
      strcpy(command, database);
      if(! debug && ! isValidCommand(command)) {
	cout << "error: file " << command << " does not exist" << endl;
	exit(1);
      }
      strcat(command, " ");
      paramqcat(command);
      strcat(command, output_pep);
      paramqcat(command);
      if(debug) {
	cout << command << endl;
      } else {
	time_t startPipe;
	time(&startPipe);
	if ( (pipe=doCommandPipe(command))==NULL || ! fgets(dbase, line_width, pipe)) {
	  cout << "error, could not execute: " << command << endl;
	  exit(1);
	}
	pclose(pipe);
	time_t endPipe;
	time(&endPipe);
	cout << "command completed in " << difftime(endPipe,startPipe) <<" sec " << endl;
      }

      command[0] = 0;
      dbase[strlen(dbase)-1] = 0; // chop off last character
      if (dbase[0]) { // can't run refreshparser without a database!
	// now make sure database is singular
	for(int k = 0; k < (int)strlen(dbase); k++) {
	  if(dbase[k] == ',') {
	    cout << "Error: multiple databases referenced by input files: " << dbase << endl << "Only ONE database must be specified" << endl;
	    dbase[0] = '\0';
	    break;
	  }
	}
      }

    } // end if (conversion_database) else

    //
    // run RefreshParser, possibly as regression test
    //
    if (!(dbase[0])) {
      cout << "\nNo single database specified (by -D option or in .pep.xml file). RefreshParser skipped." << endl;
    } else {
      strcpy(command, refresh);

      if(! debug && ! isValidCommand(command)) {
        cout << "error: file " << command << " does not exist" << endl;
        exit(1);
      }
      strcat(command, " ");
      paramqcat(command);
      strcat(command, output_pep);
      paramqcat(command);
      strcat(command, " ");
      paramqcat(command);
      memcpy(command+strlen(command)*sizeof(char), dbase, strlen(dbase)+1);
      paramqcat(command);
      //    strcat(command, dbase);
      if(prot_wt) {
        strcat(command, " ");
        strcat(command, "PROT_WT");
      }

      // extend prev_aa and/or next_aa lengths?
      if (prev_aa_len_arg) {
        strcat(command, " ");
        strcat(command, prev_aa_len_arg);
      }
      if (next_aa_len_arg) {
	strcat(command, " ");
	strcat(command, next_aa_len_arg);
      }
      if (!restore_nonexistent_prefix.empty()) {
	strcat(command, " ");
	strcat(command, restore_nonexistent_prefix.c_str());
      }

      if (testArg) { // regression test stuff - bpratt Insilicos LLC, Nov 2005
	strcat(command, " ");
	strcat(command, testArg);
      }
      if(debug) {
	cout << command << endl;
      } else {
	doCommand(command, false);
      }

    } // if database available to refresh
    command[0] = 0;
  } // if !delude (i.e. no -nR option)

  //
  // run PeptideProphet, possibly as regression test
  //
  command[0] = 0;
  if(peptide_prophet) {
    strcpy(command, pepproph);

    if(! debug && ! isValidCommand(command)) {
      cout << "error: file " << command << " does not exist" << endl;
      exit(1);
    }
    strcat(command, " ");
    paramqcat(command);
    strcat(command, output_pep);
    paramqcat(command);
    if(strlen(peptide_prophet_options) > 0) {
      strcat(command, " ");
      strcat(command, peptide_prophet_options);
    }
    if(zero) {
      strcat(command, " ");
      strcat(command, "ZERO");
    }
    if (testArg) { // regression test stuff - bpratt Insilicos LLC, Nov 2005
      strcat(command, " ");
      strcat(command, testArg);
    }

    if(debug)
      cout << command << endl;
    else
      doCommand(command, false);
    command[0] = 0;

    if (getIsInteractiveMode()) {
      strcat(command, pepprophmods);
      strcat(command, " ");
      strcat(command, output_pep);

      // HENRY: pass decoy tag to ProphetModels.pl too
      if (!decoy_tag.empty()) {
	strcat(command, " -d ");
	strcat(command, decoy_tag.c_str());
      }

      // HENRY - MY OWN USE: add protein plot, don't delete intermediate files
      // strcat(command, " -P -k ");

      if(debug)
	cout << command << endl;
      else
	doCommand(command, false);
      command[0] = 0;
    }

  } // if peptide prophet

  //
  // run quantitation, possibly as regression test
  //
  if(libra) {
    strcpy(command, libra_pep);
    if(! debug && ! isValidCommand(command)) {
      cout << "error: file " << command << " does not exist" << endl;
      exit(1);
    }
    strcat(command, " ");
    paramqcat(command);
    strcat(command, output_pep);
    paramqcat(command);
    strcat(command, " -c");
    strcat(command, libra_params);
    if (testArg) { // regression test stuff - bpratt Insilicos LLC, Nov 2005
      strcat(command, " ");
      strcat(command, testArg);
    }

    if(debug)
      cout << command << endl;
    else
      doCommand(command, false);
    command[0] = 0;
  } // libra

  if(xpress) {
    strcpy(command, xpress_pep);

    if(! debug && ! isValidCommand(command)) {
      cout << "error: file " << command << " does not exist" << endl;
      exit(1);
    }
    strcat(command, " ");
    paramqcat(command);
    strcat(command, output_pep);
    paramqcat(command);
    if(strlen(xpress_options) > 0) {
      strcat(command, " ");
      strcat(command, xpress_options);
    }	
    if (testArg) { // regression test stuff - bpratt Insilicos LLC, Nov 2005
      strcat(command, " ");
      strcat(command, testArg);
    }

    if(debug)
      cout << command << endl;
    else
      doCommand(command, false);
    command[0] = 0;
  } // xpress

  if(asap) {
    strcpy(command, asap_pep);

    if(! debug && ! isValidCommand(command)) {
      cout << "error: file " << command << " does not exist" << endl;
      exit(1);
    }
    strcat(command, " ");
    paramqcat(command);
    strcat(command, output_pep);
    paramqcat(command);
    if(strlen(asap_options) > 0) {
      strcat(command, " ");
      strcat(command, asap_options);
    }	
    if (testArg) { // regression test stuff - bpratt Insilicos LLC, Nov 2005
      strcat(command, " ");
      strcat(command, testArg);
    }

    if(debug)
      cout << command << endl;
    else
      doCommand(command, false);
    command[0] = 0;
  } // asap

  // Run an arbitrary outside tool (e.g. CPL's Q3 quantitation tool)
  if(cmd1) {
    strcpy(command, cmd_pep);

    strcat(command, " ");
    paramqcat(command);
    strcat(command, output_pep);
    paramqcat(command);
    // if (testArg) { // regression test stuff - bpratt Insilicos LLC, Nov 2005
    //  strcat(command, " ");
    //  strcat(command, testArg);
    //}

    if(debug)
      cout << command << endl;
    else
      doCommand(command, false);
    command[0] = 0;
  }
  
  // done with PeptideProphet execution and any other PepProphet-level analyses
  // (quantitation, etc.)
  // !except! iprophet; see below

  if (getIsInteractiveMode() && !tmpdir.length()) {
    // index the pepXML file if user is likely to view it, unless it's in a tmpdir and will be run through sed
    index_pepXML_file(output_pep);
  }

  if(protein_prophet) {
    strcat(command, protproph);

    strcat(command, " ");
    paramqcat(command);
    strcat(command, output_pep);
    paramqcat(command);
    strcat(command, " ");
    paramqcat(command);
    strcat(command, output_prot);
    paramqcat(command);
    strcat(command, " XML");
    if(strlen(protein_prophet_options) > 0) {
      strcat(command, " ");
      strcat(command, protein_prophet_options);
    }

    if (testArg) { // regression test stuff - bpratt Insilicos LLC, Nov 2005
      strcat(command, " ");
      strcat(command, testArg);
    }

    if(debug)
      cout << command << endl;
    else
      doCommand(command, false);
    command[0] = 0;

    if (getIsInteractiveMode()) {
      strcat(command, protprophmods);
      strcat(command, " ");
      strcat(command, output_prot);
    
      if(debug)
	cout << command << endl;
      else
	doCommand(command, false);
      command[0] = 0;
    }
  
    // from here on, output_prot must refrence the xml
    char* result = (char *)hasValidProtXMLFilenameExt(output_prot);
    if(result == NULL) { // must modify
      result = strstr(output_prot, ".shtml");
      if(result != NULL && strlen(result) == 6) {
	strcpy(result,get_protxml_dot_ext());
      }
      else {
	result = strstr(output_prot, ".htm");
	if(result != NULL && strlen(result) == 4) {
	  strcpy(result,get_protxml_dot_ext());
	}
	else {
	  cout << "do not recognize filename extension of protein output: " << output_prot << endl;
	  exit(1);
	}
      }
    }

    if(libra) {
      strcpy(command, libra_prot);
      if(! debug && ! isValidCommand(command)) {
	cout << "error: file " << command << " does not exist" << endl;
	exit(1);
      }
      strcat(command, " ");
      paramqcat(command);
      strcat(command, output_prot);
      paramqcat(command);
      strcat(command, " -c");
      strcat(command, libra_params);
      if (testArg) { // regression test stuff - bpratt Insilicos LLC, Nov 2005
	strcat(command, " ");
	strcat(command, testArg);
      }

      if(debug)
	cout << command << endl;
      else
	doCommand(command, false);
      command[0] = 0;
    } // libra

    if(xpress) {
      strcpy(command, xpress_prot);
      // check whether binary exists
      if(! debug && ! isValidCommand(command)) {
	cout << "error: file " << command << " does not exist" << endl;
	exit(1);
      }
      strcat(command, " ");
      paramqcat(command);
      strcat(command, output_prot);
      paramqcat(command);
      if (testArg) { // regression test stuff - bpratt Insilicos LLC, Nov 2005
	strcat(command, " ");
	strcat(command, testArg);
      }

      if(debug)
	cout << command << endl;
      else
	doCommand(command, false);
      command[0] = 0;
    } // xpress

    if(asap) {
      strcpy(command, asap_prot);
      if(! debug && ! isValidCommand(command)) {
	cout << "error: file " << command << " does not exist" << endl;
	exit(1);
      }
      strcat(command, " ");
      paramqcat(command);
      strcat(command, output_prot);
      paramqcat(command);
      if (testArg) { // regression test stuff - bpratt Insilicos LLC, Nov 2005
	strcat(command, " ");
	strcat(command, testArg);
      }
      if(debug)
	cout << command << endl;
      else
	doCommand(command, false);
      command[0] = 0;
      strcpy(command, asap_pvalue);
      if(! debug && ! isValidCommand(command)) {
	cout << "error: file " << command << " does not exist" << endl;
	exit(1);
      }
      strcat(command, " ");
      paramqcat(command);
      strcat(command, output_prot);
      paramqcat(command);

      if (testArg) { // regression test stuff - bpratt Insilicos LLC, Nov 2005
	strcat(command, " ");
	strcat(command, testArg);
      }
      if(debug)
	cout << command << endl;
      else
	doCommand(command, false);
      command[0] = 0;
    } // asap

    if (cmd2) {
      // custom protein processing command.
      strcpy(command, cmd_prot);
      strcat(command, " ");
      paramqcat(command);
      strcat(command, output_prot);
      paramqcat(command);
      //if (testArg) { // regression test stuff - bpratt Insilicos LLC, Nov 2005
      //   strcat(command, " ");
      //   strcat(command, testArg);
      //}
      if(debug)
	cout << command << endl;
      else
	doCommand(command, false);
      command[0] = 0;
    } // cmd_prot

  } // proteinprophet
 
  //IPROPHET
  if (inter_prophet) {
    strcat(command, iproph);
    if (testArg) { // regression test stuff - bpratt Insilicos LLC
      strcat(command, " ");
      strcat(command, testArg);
    }

    if(strlen(inter_prophet_options) > 0) {
      strcat(command, " ");
      strcat(command, inter_prophet_options);
    }
    strcat(command, " ");
    strcat(command, output_pep);
    strcat(command, " ");
    strcat(command, output_ipep);

    if(debug)
      cout << command << endl;
    else
      doCommand(command, false);
    command[0] = 0;

    if (getIsInteractiveMode()) {
      strcat(command, pepprophmods);
      strcat(command, " ");
      strcat(command, output_ipep);

      // HENRY: pass decoy tag to ProphetModels.pl too
      if (!decoy_tag.empty()) {
	strcat(command, " -d ");
	strcat(command, decoy_tag.c_str());
      }
    
      // HENRY - MY OWN USE: add protein plot, don't delete intermediate files
      // strcat(command, " -P -k ");

      if(debug)
	cout << command << endl;
      else
	doCommand(command, false);
      command[0] = 0;
    }

    if (getIsInteractiveMode() && !tmpdir.length()) {
      // index the pepXML file if user is likely to view it, unless it's in a tmpdir and will be run through sed
      index_pepXML_file(output_ipep);
    }
  }
  else if (ptm_prophet) {
    protein_iprophet = 0;
    ptm_prophet = 0;
    cerr << "WARNING: iProphet not used, PTMProphet will not run ..." << endl;
  }

  if (ptm_prophet) {
    strcat(command, ptmproph);
    if(! debug && ! isValidCommand(command)) {
      cout << "error: file " << command << " does not exist" << endl;
      exit(1);
    }
    if(strlen(ptm_options) > 0) {
      strcat(command, " ");
      strcat(command, ptm_options);
    }
    strcat(command, " ");
    strcat(command, output_ipep);
    strcat(command, " ");
    strcat(command, output_ptmpep);

    if(debug)
      cout << command << endl;
    else
      doCommand(command, false);

    if (getIsInteractiveMode() && !tmpdir.length()) {
      // index the pepXML file if user is likely to view it, unless it's in a tmpdir and will be run through sed
      index_pepXML_file(output_ptmpep);
    }
  }

  //PROTEINPROPHET IPROPHET
  if(protein_iprophet) {
    command[0] = 0;
    strcat(command, protproph);

    if(! debug && ! isValidCommand(command)) {
      cout << "error: file " << command << " does not exist" << endl;
      exit(1);
    }

    strcat(command, " ");
    paramqcat(command);
    if (!ptm_prophet) {
      strcat(command, output_ipep);
      paramqcat(command);
      strcat(command, " ");
      paramqcat(command);
      strcat(command, output_iprot);
      paramqcat(command);
    }
    else {
      strcat(command, output_ptmpep);
      paramqcat(command);
      strcat(command, " ");
      paramqcat(command);
      strcat(command, output_ptmprot);
      paramqcat(command);
    }

    strcat(command, " XML");
    if(strlen(protein_iprophet_options) > 0) {
      strcat(command, " ");
      strcat(command, protein_iprophet_options);
    }

    if (testArg) { // regression test stuff - bpratt Insilicos LLC, Nov 2005
      strcat(command, " ");
      strcat(command, testArg);
    }

    if(debug)
      cout << command << endl;
    else
      doCommand(command, false);

    command[0] = 0;

    if (getIsInteractiveMode()) {
      strcat(command, protprophmods);
      strcat(command, " ");
      if (!ptm_prophet) {    
	strcat(command, output_iprot);
      }
      else {
	strcat(command, output_ptmprot);
      }
      if(debug)
	cout << command << endl;
      else
	doCommand(command, false);
      command[0] = 0;
    }

    // from here on, output_iprot must refrence the xml
    if(!hasValidProtXMLFilenameExt(output_iprot)) { // must modify
      char* result = strstr(output_iprot, ".shtml");
      if(result != NULL && strlen(result) == 6) {
	strcpy(result,get_protxml_dot_ext());
      }
      else {
	char* result = strstr(output_iprot, ".htm");
	if(result != NULL && strlen(result) == 4) {
	  strcpy(result,get_protxml_dot_ext());
	}
	else {
	  cout << "do not recognize filename extension of protein output: " << output_iprot << endl;
	  exit(1);
	}
      }
    }

    if(libra) {
      strcpy(command, libra_prot);
      if(! debug && ! isValidCommand(command)) {
	cout << "error: file " << command << " does not exist" << endl;
	exit(1);
      }
      strcat(command, " ");
      paramqcat(command);

      if (ptm_prophet) 
	strcat(command, output_ptmprot);
      else
	strcat(command, output_iprot);

      paramqcat(command);
      strcat(command, " -c");
      strcat(command, libra_params);
      if (testArg) { // regression test stuff - bpratt Insilicos LLC, Nov 2005
	strcat(command, " ");
	strcat(command, testArg);
      }
      if(debug)
	cout << command << endl;
      else
	doCommand(command, false);
      command[0] = 0;
    } // libra

    if(xpress) {
      strcpy(command, xpress_prot);
      // check whether binary exists
      if(! debug && ! isValidCommand(command)) {
	cout << "error: file " << command << " does not exist" << endl;
	exit(1);
      }
      strcat(command, " ");
      paramqcat(command);

      if (ptm_prophet) 
	strcat(command, output_ptmprot);
      else
	strcat(command, output_iprot);

      paramqcat(command);
      if (testArg) { // regression test stuff - bpratt Insilicos LLC, Nov 2005
	strcat(command, " ");
	strcat(command, testArg);
      }
      if(debug)
	cout << command << endl;
      else
	doCommand(command, false);
      command[0] = 0;
    } // xpress

    if(asap) {
      strcpy(command, asap_prot);
      if(! debug && ! isValidCommand(command)) {
	cout << "error: file " << command << " does not exist" << endl;
	exit(1);
      }
      strcat(command, " ");
      paramqcat(command);

      if (ptm_prophet) 
	strcat(command, output_ptmprot);
      else
	strcat(command, output_iprot);

      paramqcat(command);
      if (testArg) { // regression test stuff - bpratt Insilicos LLC, Nov 2005
	strcat(command, " ");
	strcat(command, testArg);
      }
      if(debug)
	cout << command << endl;
      else
	doCommand(command, false);

      command[0] = 0;
      strcpy(command, asap_pvalue);
      if(! debug && ! isValidCommand(command)) {
	cout << "error: file " << command << " does not exist" << endl;
	exit(1);
      }
      strcat(command, " ");
      paramqcat(command);

      if (ptm_prophet) 
	strcat(command, output_ptmprot);
      else
	strcat(command, output_iprot);

      paramqcat(command);
      if (testArg) { // regression test stuff - bpratt Insilicos LLC, Nov 2005
	strcat(command, " ");
	strcat(command, testArg);
      }
      if(debug)
	cout << command << endl;
      else
	doCommand(command, false);

      command[0] = 0;
    } // asap

    if (cmd2) {
      // custom protein processing command.
      strcpy(command, cmd_prot);
      strcat(command, " ");
      paramqcat(command);

      if (ptm_prophet) 
	strcat(command, output_ptmprot);
      else
	strcat(command, output_iprot);

      paramqcat(command);
      //if (testArg) { // regression test stuff - bpratt Insilicos LLC, Nov 2005
      //   strcat(command, " ");
      //   strcat(command, testArg);
      //}
      if(debug)
	cout << command << endl;
      else
	doCommand(command, false);
      command[0] = 0;
    } // cmd_prot

  } // protein iprophet
 
  free(testArg);

  if (tmpdir.length()) { // we used tmpfiles, tidy up and copy to final destination
    popWebserverRoot(); // we fooled children into thinking tmpdir is in root path
    // for each file in tmpdir, move it to final location and fix up any path refs in file
    glob_t g;
    struct stat s;
    std::string tmpdirall = tmpdir + "*";
    glob(tmpdirall.c_str(), 0, NULL, &g);
    // get target directory
    char *finaldir = strdup(final_output_pep.c_str());
    char *cp=findRightmostPathSeperator(finaldir);
    if (cp) {
      *(cp+1) = 0;
    } else {
      *finaldir = 0;
    }
    for (int i = 0; i < g.gl_pathc; i++) {
      // glob should not return full path info, but be paranoid
      const char *fname = findRightmostPathSeperator((g.gl_pathv)[i]);
      fname = fname?(fname+1):((g.gl_pathv)[i]);

      std::string target(finaldir);
      target += fname;
      std::string source(tmpdir);
      source += fname;

      if (!stat(source.c_str(), &s) && !(S_IFDIR & s.st_mode)) { // avoid . and ..
	// rename and move tempfile, cleaning up any paths referenced in the file
	// also performs gzip compress on files named *.gz
	cleanup_and_move_tmpfile(source.c_str(),target.c_str());
      }
    }
    tmpdir = tmpdir.substr(0,tmpdir.length()-1); // drop that trailing slash
    verified_rmdir(tmpdir.c_str()); // delete the tmpdir
    free(finaldir);

  } else { // no tmpdir, but did user request .gz?
    if (isDotGZ(final_output_pep.c_str())) { // .gz output was requested
      do_gzip(final_output_pep); // compress
    }
  }

  if (getIsInteractiveMode()) { // generate models pages, post-tmp file shenanigans
    char *tmp =  makeFullPath(final_output_pep.c_str());
    run_TPP_models(tmp);
    free(tmp);

    if (inter_prophet) {
      char *tmp =  makeFullPath(final_output_ipep.c_str());
      run_TPP_models(tmp);
      free(tmp);
    }

    if (ptm_prophet) {
      char *tmp =  makeFullPath(final_output_ptmpep.c_str());
      run_TPP_models(tmp);
      free(tmp);
    }

    if (protein_prophet) {
      char *tmp = makeFullPath(final_output_prot.c_str());
      run_TPP_models(tmp);
      free(tmp);
    }

    if (protein_iprophet) {
      char *tmp;
      if (ptm_prophet) 
	tmp = makeFullPath(final_output_ptmprot.c_str());
      else
	tmp = makeFullPath(final_output_iprot.c_str());
      run_TPP_models(tmp);
      free(tmp);
    }
  }

  time_t now;
  time(&now);
  cout << glob_logString.c_str();
  cout << "job completed in " << difftime(now, then) <<" sec " << endl;

  delete[] command;
  delete[] peptide_prophet_options;
  delete[] inter_prophet_options;
  delete[] protein_prophet_options;
  delete[] protein_iprophet_options;
  delete[] asap_options;
  delete[] xpress_options;
  delete[] xpress_prot_opts;
  delete[] output_pep;
  delete[] output_prot;
  delete[] output_ipep;
  delete[] output_iprot;
  delete[] output_ptmpep;
  delete[] output_ptmprot;

  delete[] interact;
  delete[] refresh;
  delete[] database;
  delete[] pepproph;
  delete[] iproph;
  delete[] pepprophmods;
  delete[] protprophmods;
  delete[] xpress_pep;
  delete[] asap_pep;
  delete[] protproph;
  delete[] xpress_prot;
  delete[] asap_prot;
  delete[] asap_pvalue;
  delete[] sequest2xml;
  delete[] mascot2xml;
  delete[] comet2xml;
  delete[] libra_pep;
  delete[] libra_prot;
  delete[] libra_params;

  return 0;
}

const char* getPeptideProphetOption(char o) {
  switch(o) {
  case 'i': return "ICAT";
  case 'f': return "NOICAT";
  case 'g': return "GLYC";
  case 'H': return "PHOSPHO";
  case 'm': return "MALDI";
  case 'x': return "EXCLUDE";
  case 'l': return "LEAVE";
  case 'I': return "PI";
  case 'R': return "RT";
  case 'A': return "ACCMASS";
  case 'w': return "INSTRWARN";
  // HENRY
  case 'B': return "PERFECTLIB";
  // case 'D': return "DELTA";
  case 'o': return "OPTIMIZEFVAL";
  // END HENRY

  case 'n': return "NONEGINIT";
  case 'N': return "NONTT";
  case 'M': return "NONMC";
  case 'P': return "NONPARAM";  
  case 'E': return "EXPECTSCORE";  
  case 'F': return "FORCEDISTR";
  case 'G': return "NEGGAMMA";
  case 'd': return "DECOYPROBS";
  case 'k': return "NOMASS";

  default: return NULL;
  } // switch
}

const char* getInterProphetOption(char o) {
  switch(o) {
  case 'P': return "NONSP";
  case 'S': return "NONSS";
  case 'E': return "NONSE";
  case 'R': return "NONRS";
  case 'I': return "NONSI";
  case 'M': return "NONSM";

  default: return NULL;
  } // switch
}

const char* getProteinProphetOption(char o) {
  switch(o) {
  case 'u': return "NOGROUPS";
  case 'd': return "DELUDE";
  case 's': return "NOOCCAM";
  case 't': return "NOPLOT";
  case 'i': return "ICAT";
  case 'p': return "IPROPHET";
  case 'o': return "NORMPROTLEN";
  case 'W': return "PROTMW";
  case 'L': return "PROTLEN";

  default: return NULL;
  } // switch
}

const char* getSampleEnzyme(char e) {
  switch(e) {
  case 'T': return "trypsin";
  case 'S': return "stricttrypsin";
  case 'C': return "chymotrypsin";
  case 'A': return "aspn";
  case 'G': return "gluc";
  case 'B': return "gluc_bicarb";
  case 'M': return "cnbr";
  case 'D': return "trypsin/cnbr";
  case '3': return "tca";
  case 'E': return "elastase";
  case 'K': return "trypsin_k";
  case 'L': return "lysn";
  case 'P': return "lysn_promisc";
  case 'R': return "ralphtrypsin";
  case 'N': return "nonspecific";

  default: return "";
  } // switch
}

static void append_escaped(std::string &cmd,const std::string &add) {
  for (const char *c=add.c_str();*c;c++) {
    if (strchr("/.",*c)) { // gotta escape regexp stuff
      cmd += '\\';
    }
    cmd += *c;
  }
}

// helper func to move a tempfile to permanent home, after cleaning up any internal
// path references
void cleanup_and_move_tmpfile(const char *output_fname,const char *final_output_fname) {
  printf("moving tempfile %s to %s\n",output_fname,final_output_fname);

  std::string tmp_path(output_fname);
  std::string final_path(final_output_fname);
  makeFullPath(final_path);
  const char *quot=getCmdlineQuoteChar(); // get a system-appropriate quote char
  // remove the leading similarities (presumbably tmpdir is under webserver_root)
  while (tolower(tmp_path[0]) == tolower(final_path[0])) {
    tmp_path = tmp_path.substr(1);
    final_path = final_path.substr(1);
  }
  // and remove the trailing similarities, leaving just the path difference
  while (tmp_path.length() && final_path.length() &&
	 (tolower(tmp_path[tmp_path.length()-1]) == tolower(final_path[final_path.length()-1]))) {
    tmp_path = tmp_path.substr(0,tmp_path.length()-1);
    final_path = final_path.substr(0,final_path.length()-1);
  }	
  std::string cmd;
  if (strstri(final_output_fname,".png")) { // binary file
    safe_rename(output_fname,final_output_fname);
  } else {
    // we'll use sed to copy tmpfile to tmpfile, clearing up paths in the file
    std::string tmp2_fname(output_fname);
    tmp2_fname += ".XXXXXX";
    safe_fclose(FILE_mkstemp(tmp2_fname)); // create then close a uniquely named file
    verified_unlink(tmp2_fname); // kill it
    cmd = "";
    bool zipped=false;
    if (isDotGZ(output_fname)) { // decompress, pipe to sed
      zipped=true;
      cmd += "gzip -d -c ";
      cmd += " ";
      cmd += quot;
      cmd += output_fname;
      cmd += quot;
      cmd += " | ";
    }
    cmd += "sed ";
    cmd += quot;
    cmd += "s/";
    append_escaped(cmd,tmp_path);
    cmd += '/';
    append_escaped(cmd,final_path);
    cmd += "/g";
    cmd += quot;
    if (zipped) {
      cmd += " | gzip -9 "; // recompress to final file
    } else {
      cmd += " ";
      cmd += quot;
      cmd += output_fname;
      cmd += quot;
    }
    cmd += " > ";
    cmd += quot;
    cmd += tmp2_fname;
    cmd += quot;
    verified_system(cmd.c_str());
    // is this a pepXML file in need of indexing now that it contains proper paths? 
    if (getIsInteractiveMode() && hasValidPepXMLFilenameExt(final_output_fname)) {
      std::string tmpindex = tmp2_fname + ".index";
      std::string finalindex(final_output_fname);
      finalindex += ".index";
      index_pepXML_file(tmp2_fname.c_str());
      // now clean up the path info in the index (o what a tangled web we weave...)
      cmd = "sed ";
      cmd += quot;
      cmd += "s/";
      append_escaped(cmd,tmp_path);
      cmd += '/';
      append_escaped(cmd,final_path);
      cmd += "/g";
      cmd += quot;
      cmd += " ";
      cmd += quot;
      cmd += tmpindex;
      cmd += quot;
      cmd += " > ";
      cmd += quot;
      cmd += finalindex.c_str();
      cmd += quot;
      verified_system(cmd.c_str());
      verified_unlink(tmpindex);
    }
    safe_rename(tmp2_fname.c_str(),final_output_fname);
    verified_unlink(output_fname);
  }
}
