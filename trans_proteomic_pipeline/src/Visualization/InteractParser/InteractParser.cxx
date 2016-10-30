/*

Program       : Interact                                                    
Author        : Andrew Keller <akeller@systemsbiology.org>                                                    
Date          : 11.27.02 

Primary data object holding all mixture distributions for each precursor ion charge

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

#include "InteractParser.h"
#include "common/util.h"
#include <errno.h>
#include <pwiz/utility/misc/random_access_compressed_ifstream.hpp>


InteractParser::InteractParser(Array<char*>* inputfiles, char* outfile, char* database, char* datapath, char* dbtype, char* enz, bool prot_name, bool update_chg, char* exp_lbl, int min_peplen, int max_rank, bool collision_eng, bool fix_pyro_mods, bool comp_volt, bool prec_intens, bool get_rt, bool write_ref, bool check_pepproph) : Parser("interact") {
  // default settings

  check_pepproph_ = check_pepproph;
  modelOpts_.icat_ = ICAT_UNKNOWN;
  modelOpts_.glyc_ = False;
  modelOpts_.minprob_ = 0.05;
  scoreOpts_.deltastar_ = DELTACN_ZERO;

  msd_ = NULL;
  
  write_ref_ = write_ref;

  if (!write_ref_ && inputfiles->size() > 1) {
    cerr << "WARNING: -N (no reference) option used with more than 1 input files, references will be written and -N option ignored." << endl;

  }

  dirsize_ = 5000;
  curr_dir_ = new char[dirsize_];
  prot_name_ = prot_name;
  update_chg_ = update_chg;
  database_ = NULL;
  if (database != NULL) {
    database_ = new char[strlen(database)+1];
    strcpy(database_, database);
  }
  enz_ = NULL;
  if (enz != NULL) {
    enz_ = new char[strlen(enz)+1];
    strcpy(enz_, enz);
    enzyme_ = (new ProteolyticEnzymeFactory())->getProteolyticEnzyme(enz_); 
  } else {
    enzyme_ = NULL;
  }

  datapath_ = NULL;
  if (datapath != NULL) {
    datapath_ = new char[strlen(datapath)+2];
    strcpy(datapath_, datapath);
    if (datapath_[strlen(datapath_)-1] != '/') {
      strcat(datapath_, "/");
    }
  }

  dbtype_ = new char[3];
  strcpy(dbtype_, "AA");
  if (dbtype != NULL) {
    strcpy(dbtype_, dbtype);
  }

  // get full path for outfile
  if(!isAbsolutePath(outfile)) {
    safepath_getcwd(curr_dir_, dirsize_);
    outfile_ = new char[strlen(curr_dir_)+strlen(outfile)+2];
    strcpy(outfile_, curr_dir_);
    strcat(outfile_, "/");
    strcat(outfile_, outfile);
  }
  else {
    outfile_ = new char[strlen(outfile)+1];
    strcpy(outfile_, outfile);
	curr_dir_[0] = 0;
  }
  inputfiles_ = inputfiles;

  options_ = NULL;
  xmlfile_ = NULL;

  exp_lbl_ = exp_lbl;
  min_peplen_ = min_peplen;
  max_rank_ = max_rank;
  collision_eng_ = collision_eng;

  comp_volt_ = comp_volt;

  get_rt_ = get_rt;

  fix_pyro_mods_ = fix_pyro_mods;
  prec_intens_ = prec_intens;
  init(xmlfile_);
}

// make base name a full path
static void check_basename(Tag *tag) {
   const char *basename = tag->getAttributeValue("base_name");
   if(basename && !isAbsolutePath(basename)) {
      char curr_dir[1024];
      safepath_getcwd(curr_dir, sizeof(curr_dir));
      char *bfile = new char[strlen(curr_dir)+strlen(basename)+2];
      strcpy(bfile, curr_dir);
      strcat(bfile, "/");
      strcat(bfile, basename);
      tag->setAttributeValue("base_name",bfile);
   }
}

void InteractParser::verify_and_correct_path(Tag *tag) {
  
  string base = string(tag->getAttributeValue("base_name"));
  string ext =  string(tag->getAttributeValue("raw_data"));
  string mzFile = base;
  size_t pos;
  RAMPFILE *pFI = NULL;


  if (( pos=base.find('.') ) != string::npos &&
      ( base.substr(pos) == ".mzML" || base.substr(pos) == ".mzXML" || 
	base.substr(pos) == ".pepXML" || base.substr(pos) == ".pep.xml" ) ) {
    cerr << "WARNING: base_name " << base 
	 << " in msms_run_summary tag contains extenstion...removing..." << endl;

    base = base.substr(0,pos);
    
  }

  if (ext.empty()) {
    cerr << "WARNING: empty raw_data in msms_run_summary tag ... trying mzML ..." << endl;
    ext = ".mzML";
  }
  else if (ext.substr(0,1) != ".") {
    ext = "."+ext;
  }

  mzFile = base + ext;

  if ((pFI = rampOpenFile(mzFile.c_str())) == NULL) {
    cerr << "WARNING: cannot open data file " << mzFile <<  " in msms_run_summary tag ..." ;

  
    if (ext == ".mzXML") {
      cerr << "... trying .mzML ..." << endl;
      ext = ".mzML";


    }
    else if (ext == ".mzML") {
      cerr << "... trying .mzXML ..." << endl;
      ext = ".mzXML";
    }

    mzFile = base + ext;
    if ((pFI = rampOpenFile(mzFile.c_str())) == NULL) {
      cerr << "WARNING: cannot CORRECT data file " << mzFile <<  " in msms_run_summary tag ..." << endl;
      return;
    }

  }
  cerr << "SUCCESS: CORRECTED data file " << mzFile <<  " in msms_run_summary tag ..." << endl;
  tag->setAttributeValue("base_name", base.c_str());
  tag->setAttributeValue("raw_data", ext.c_str());
  if (msd_) {
    delete msd_;
    msd_ = NULL;
  }
  if (pFI != NULL) {
    rampCloseFile(pFI);
    try {
       msd_ = new pwiz::msdata::MSDataFile(mzFile);

    }
    catch (...) {
      cerr << "WARNING: Unable to open file: " << mzFile << " cannot correct scan numbers!" <<  endl;
      msd_ = NULL;
      return;
    }
    
  }
  using namespace pwiz::msdata;
  if (!msd_->run.spectrumListPtr.get()) {
       throw runtime_error("[InteractParser] Null spectrumListPtr.");
       delete msd_;
       msd_ = NULL;
  }
  else {
    sl_ = msd_->run.spectrumListPtr;
  }
}

InteractParser::~InteractParser() {
  if (exp_lbl_ != NULL) 
    delete[] exp_lbl_;
  if(curr_dir_ != NULL)
    delete[] curr_dir_;
}

void InteractParser::parse(const char* xmlfile) {

  if(xmlfile != NULL) {
    cerr << "InteractParser: internal error, " << xmlfile << " not NULL" << endl;
    exit(1);
  }

  RAMPFILE *pFI = NULL;
  ramp_fileoffset_t *pScanIndex;
  int iLastScan;
  struct ScanHeaderStruct scanHeader;
  struct RunHeaderStruct runHeader;
  ramp_fileoffset_t indexOffset;



  char* engine = NULL;
  char* enzyme = NULL;
  char* massspec = NULL;
  Array<Tag*>* tags = NULL;
  Tag* tag = NULL;

  //  int line_width = 10000;
  char *nextline = new char[line_width_];
  char* data = NULL;

  double MIN_PROB =  modelOpts_.minprob_; //0.0; //0.05; // for now

  int k, charge = -1;
  char peptide[500];
  char *szWebserverRoot = new char[400]; 

#define SIZE_BUFF 5000
  char szBuf[SIZE_BUFF];
  int len = (int)strlen(outfile_)+(int)strlen(PEPXML_STD_XSL)+1;
  char *tmpXsl = new char[len];
  char *xslfile = new char[len];
  strcpy(xslfile, outfile_);
  char *cp = strrchr(xslfile,'.');
  if (cp) {
     strcpy(cp,".xsl");
  }
  if(strstr(xslfile, ".xsl") == NULL) {
    cout << "error: xsl " << xslfile << " must end in .xsl" << endl;
    exit(1);
  }
  char *szCommand = new char[2000];
#if defined(USING_RELATIVE_WEBSERVER_PATH)
  const char *pStr1=NULL;
  pStr1=getWebserverRoot();
  if (pStr1==NULL)
  {
    printf("<PRE> Environment variable WEBSERVER_ROOT does not exist.\n\n");
    printf(" For Windows users, you can set this environment variable\n");
    printf(" through the Advanced tab under System Properties when you\n");
    printf(" right-mouse-click on your My Computer icon.\n\n");
    
    printf(" Set this environment variable to your webserver's document\n");
    printf(" root directory such as c:\\inetpub\\wwwroot for IIS or\n");
    printf(" c:\\website\\htdocs or WebSite Pro.\n\n");
    printf(" Exiting.\n");
    exit(0);
  }
#ifdef WINDOWS_NATIVE
  strcpy(szWebserverRoot, pStr1);
#else
    // must first pass to cygpath program
  sprintf(szCommand, "cygpath -u '%s'", pStr1);
  FILE *fp;
    if((fp = popen(szCommand, "r")) == NULL)
    {
      printf("cygpath error, exiting\n");
      exit(1);
    }
      fgets(szBuf, SIZE_BUF, fp);
      pclose(fp);
      szBuf[strlen(szBuf)-1] = 0;
      strcpy(szWebserverRoot, szBuf);
#endif

  //TODO: need to save the old unchanged string in case WEBSERVER_ROOT is not the prefix
  //lowerCase the first part
  int tmpLen=(int)strlen(szWebserverRoot);
  for (int i=0; i<tmpLen; i++) {
    xslfile[i] = tolower(xslfile[i]);
    szWebserverRoot[i] = tolower(szWebserverRoot[i]);
  }
  if (!isPathSeperator(szWebserverRoot[tmpLen-1])) {
    szWebserverRoot[tmpLen++] = '/';
  }
  szWebserverRoot[tmpLen] = 0;

  fixPath(szWebserverRoot,0); // tidy up path seperators etc
  fixPath(xslfile,0); // tidy up path seperators etc
  fixPath(outfile_,0); // tidy up path seperators etc

  if (getIsInteractiveMode() &&
	  pathcmp(xslfile, szWebserverRoot)) { // compare the paths, ignoring filename
    cout << "warning: xsl file " << xslfile << " must begin with the WEBSERVER_ROOT path for use with a webserver." << endl;
  }

  char *pStr2 = xslfile+strlen(szWebserverRoot);
  sprintf(tmpXsl, "%s%s", PEPXML_STD_XSL, pStr2);
  strcpy(xslfile, tmpXsl);
  delete [] tmpXsl;
#else
  // output xsl name relative to webserver root
  const char *wsr=getWebserverRoot();
  int wsrlen = wsr?strlen(wsr):0;
  if (wsrlen && isPathSeperator(wsr[wsrlen-1])) {
     wsrlen--; // leave the trailing / alone (so it becomes leading)
  }
  if (wsrlen && !strncmp(wsr,xslfile,wsrlen)) {
     memmove(xslfile,xslfile+wsrlen,strlen(xslfile+wsrlen)+1);
  }

#endif // end if not win32
  delete [] szCommand;


  Tag* derivation_tag = new Tag("dataset_derivation", True, True);
  derivation_tag->setAttributeValue("generation_no", "0");

  TagFilter* xml_filter = new TagFilter("xml");
  TagFilter* xmlstyle_filter = new TagFilter("xml-stylesheet");
  TagFilter* doc_filter = new TagFilter("!DOCTYPE", -1);
  TagFilter* deriv_filter = new TagFilter("dataset_derivation");

  // construct a tempfile name, possibly in the tmp dir if so configured
  std::string tmp_outfile = make_tmpfile_name(outfile_);
  ofstream fout(tmp_outfile.c_str());
  if(! fout) {
    cerr << "cannot write output to file " << tmp_outfile << endl;
    exit(1);
  }
  Boolean first = False;
  Boolean last = False;
  char pipeline_analysis[] = "msms_pipeline_analysis";
  int result_index = 1;
  char search_result[] = "spectrum_query";
  char attr_name[] = "index";

  Boolean USE_XML_SCHEMA = True; // instead of DTD


  OuterTag* outertag = NULL;


  TagFilter* datagen_filt = new TagFilter("dataset_derivation");
  TagFilter* interact_filt = new TagFilter("analysis_summary");
  interact_filt->enterRequiredAttributeVal("analysis", getName());

  Array<Tag*>* oldsummaries = new Array<Tag*>; // store all the old summaryies
  Boolean collect = False;

#ifdef USE_STD_MODS
  Boolean USE_STD_MODIFICATIONS = True; //False;
  Boolean EXCLUDE_CONST_STATICS = True;
#endif
#ifndef USE_STD_MODS
  Boolean USE_STD_MODIFICATIONS = False; //False;
  Boolean EXCLUDE_CONST_STATICS = False;
#endif


  Array<Tag*>* modifications = NULL;
  Boolean mod_on = False;
  peptide[0] = 0;
  ModificationInfo* mod_info = NULL;
  Array<StaticModificationCount>* static_consts = NULL;
  StaticModificationCount next_static_count;
  int static_tots = 0;
  if(EXCLUDE_CONST_STATICS) {
    static_consts = new Array<StaticModificationCount>;
  }

 
  char*  cpydata = new char[line_width_];
  for(k = 0; k < inputfiles_->length(); k++) {
    Boolean skip_file = False;
    char* dirsep = findRightmostPathSeperator(outfile_);
    char* outbase = strstr(outfile_, (*inputfiles_)[k]);
    if (outbase != NULL && *(outbase-sizeof(char)) == *dirsep && !strcmp(outbase,(*inputfiles_)[k])) { 
      //If the input file and the output file are the same
      cerr << "Skipping file " << (*inputfiles_)[k] << ", which has the same name as the output file ..." << endl;
      skip_file =  True;
    }
    else {
      pwiz::util::random_access_compressed_ifstream ftest((*inputfiles_)[k]); // can read gzipped xml
      if(! ftest) {
	cerr << "error opening " << (*inputfiles_)[k] << endl;
	exit(1);
      }
      
      while(ftest.getline(nextline, line_width_)) {
	// Collapse tag spread over many lines into a single string
	strcpy(cpydata, nextline);
	data = strstr(cpydata, ">");
	
	while(data == NULL && ftest.getline(nextline, line_width_)) {
	  strcat(cpydata, " ");
	  strcat(cpydata, nextline);
	  data = strstr(cpydata, ">");
	}
	
	data = strstr(cpydata, "<");
	if (data != NULL) {
	  if (tag) delete tag;
	  tag = new Tag(data);
	  if(tag != NULL && ! datagen_filt->filter(tag) && ! interact_filt->filter(tag)) {
	    if(tag->isStart()) {
	      if(! strcmp(tag->getName(), "protein_summary")) {
		cerr << "Skipping file " << (*inputfiles_)[k] << ", which is not a pepXML file ..." << endl;
		delete tag;
		tag = NULL;
		skip_file = True;
		break;
	      }
	      if(check_pepproph_ && ! strcmp(tag->getName(), "analysis_summary") && ! strcmp(tag->getAttributeValue("analysis"), "peptideprophet")) {
		cerr << "Skipping file " << (*inputfiles_)[k] << ", which already contains peptideprophet results ..." << endl;
		delete tag;
		tag = NULL;
		skip_file = True;
		break;
	      }
	      else if(! strcmp(tag->getName(), "analysis_summary") && ! strcmp(tag->getAttributeValue("analysis"), "peptideprophet")) {
		cerr << "WARNING: File " << (*inputfiles_)[k] << ", already contains peptideprophet results ..." << endl;

	      }
	    }
	  }
	}
	if (tag != NULL)
	  delete tag;
	tag = NULL;
      }
      ftest.close();
    }
    if (skip_file) {
      inputfiles_->remove(k--);
    }
  }
  Array<Tag*>* tag_cache = new Array<Tag*>;
  Array<Tag*>* summary_tags = new Array<Tag*>;
  Tag* summary_tag = new Tag("analysis_summary", True, False);
  summary_tag->setAttributeValue("analysis", getName());
  summary_tag->setAttributeValue("time", time_);
  summary_tags->insertAtEnd(summary_tag);

  summary_tag = new Tag("interact_summary", True, False);
  summary_tag->setAttributeValue("filename", outfile_);
  summary_tag->setAttributeValue("directory", curr_dir_);
  // now for the inputfiles

  summary_tags->insertAtEnd(summary_tag);


  if (write_ref_) {
    for(k = 0; k < inputfiles_->length(); k++) {
      summary_tag = new Tag("inputfile", True, True);
      summary_tag->setAttributeValue("name", (*inputfiles_)[k]);
      if (!isAbsolutePath((*inputfiles_)[k])) {
	char curr_dir[1024];
	safepath_getcwd(curr_dir, sizeof(curr_dir));
	summary_tag->setAttributeValue("directory", curr_dir);
      }
      summary_tags->insertAtEnd(summary_tag);
    }
  }

 
  summary_tag = new Tag("interact_summary", False, True);
  summary_tags->insertAtEnd(summary_tag);

  summary_tag = new Tag("analysis_summary", False, True);
  summary_tags->insertAtEnd(summary_tag);
  Boolean skip_hit = False;

  for(k = 0; k < inputfiles_->length(); k++) {
    first = k == 0;
    last = k == inputfiles_->length()-1;

    Boolean run_header = True;
    Boolean dboutput = False;
    Boolean skip_tag = False;
    Boolean skip_spec = False;
    char * basename;
    pwiz::util::random_access_compressed_ifstream fin((*inputfiles_)[k]); // can read gzipped xml
    if(! fin) {
      FILE *f=fopen((*inputfiles_)[k],"r"); // just to set errno
      cerr << "InteractParser: error opening interact file " << (*inputfiles_)[k] << ": " << strerror(errno) << endl;
      exit(1);
    }
    cout << " file " << (k+1) << ": " << (*inputfiles_)[k] << endl;
    
    while(fin.getline(nextline, line_width_)) {
      //      cout << "next: " << nextline << endl;
    
      
      // Collapse tag spread over many lines into a single string
      strcpy(cpydata, nextline);
      data = strstr(cpydata, ">");

      while(data == NULL && fin.getline(nextline, line_width_)) {
	strcat(cpydata, " ");
	strcat(cpydata, nextline);
	data = strstr(cpydata, ">");
      }

      data = strstr(cpydata, "<");
      while (data != NULL) { // handle multiple tags on a line
	skip_tag = False;
      if (data != NULL) {
	if (!tag_cache->size() && tag) {
	  delete tag;
	  tag = NULL;
	}
	tag = new Tag(data);
        //tag->write(cout);


	if(tag != NULL && ! datagen_filt->filter(tag) && ! interact_filt->filter(tag)) {
	  Boolean output = ! collect;
	  if(run_header && strstr(tag->getName(), "msms_run_summary") != NULL) {
	    if(tag->isEnd()) {
	      if(! strcmp(tag->getName(), "msms_run_summary")) {
		run_header = True;
	      }
	    }
	    if(tag->isStart()) {
	      if(! strcmp(tag->getName(), "msms_run_summary")) {

		if (datapath_ != NULL) {
		  string base = string(tag->getAttributeValue("base_name"));
		  unsigned found = base.find_last_of("/\\");
		  tag->setAttributeValue("base_name",base.substr(found+1).c_str());
		  
		  basename = new char[strlen(datapath_) + strlen(tag->getAttributeValue("base_name")) + 2];
		  strcpy(basename, datapath_);
		  strcat(basename, tag->getAttributeValue("base_name"));
		  tag->setAttributeValue("base_name", basename);
		  delete [] basename;
		}

		run_header = False;
		// make base name a full path
		check_basename(tag);
		
		verify_and_correct_path(tag);
		
		//DDS: Read Collision Energies from mzFile
		
		if (collision_eng_ || comp_volt_ || prec_intens_ || get_rt_) {
		  
		  string mzFile = string(tag->getAttributeValue("base_name"));
		  string ext =  string(tag->getAttributeValue("raw_data"));

		  
		  
		  if (ext.empty()) {
		    ext = ".mzML";
		    mzFile = string(tag->getAttributeValue("base_name")) + ext;
		  }
		  else if ((pFI = rampOpenFile(mzFile.c_str())) == NULL) {	
		    ext = ".mzML";
		    mzFile = string(tag->getAttributeValue("base_name")) + ext;

		    if ((pFI = rampOpenFile(mzFile.c_str())) == NULL) {		    	       
		      ext = ".mzXML";
		      mzFile = string(tag->getAttributeValue("base_name")) + ext;
		      
		      if ((pFI = rampOpenFile(mzFile.c_str())) == NULL)
			{
			  printf ("Could not open data file (tried extensions .mzML and .mzXML) %s\n",
				  mzFile.c_str());
			  fflush(stdout);
			  return;
			}
		    }

		  }

		  if (pFI == NULL && (pFI = rampOpenFile(mzFile.c_str())) == NULL) {		    	       
		    ext = ".mzXML";
		    mzFile = string(tag->getAttributeValue("base_name")) + ext;

		    if ((pFI = rampOpenFile(mzFile.c_str())) == NULL)
		      {
			printf ("Could not open data file (tried extensions .mzML and .mzXML) %s\n",
				mzFile.c_str());
			fflush(stdout);
			return;
		      }
		  }
		  indexOffset = getIndexOffset (pFI);
		  pScanIndex = readIndex (pFI, indexOffset, &iLastScan);
		  readRunHeader(pFI, pScanIndex, &runHeader, iLastScan);		 
		  
		}
		
	      }
	      else {
		collect = True;
		output = False;
	      }
	    }
	    else if(collect && tag->isEnd())
	      collect = False; // but still no output
	    
	  }
	  
	  if (tag->isStart() && 
	      !strcmp(tag->getName(), "sample_enzyme")) {
	    if (enzyme_ == NULL) {
	      if ((enzyme_ = (new ProteolyticEnzymeFactory())->getProteolyticEnzyme(tag->getAttributeValue("name")))  == NULL) {
		cerr << "ERROR: Unrecognized enzyme " << tag->getAttributeValue("name") << " - please specify enzyme with -E option" << endl;
		exit(1);
	      }
	      else if (enz_ == NULL) {
		enz_ = new char[strlen(tag->getAttributeValue("name"))+1];
		strcpy(enz_,   tag->getAttributeValue("name"));
	      }
	      
	    }
	    enzyme_->writePepXMLTags(fout);
	    skip_tag = True;
	  }
	  else if (!strcmp(tag->getName(), "sample_enzyme") ||
		   !strcmp(tag->getName(), "specificity")) {
	      skip_tag = True;
	  }
	  

	  if (!strcmp(tag->getName(), "search_database")) {
	    dboutput = True;
	  }
	
	  if(! strcmp(tag->getName(), "search_result") || ! strcmp(tag->getName(), "spectrum_query")) {
	    if (tag->isStart()) {
	      tag_cache->insertAtEnd(tag);
	      skip_spec = True;
	      output = False;
	    }
	    else if (skip_spec) {
	      tag_cache->clear();
	      output = False;
	    }
	    else {
	      output = True;
	    }
	    skip_hit = False;
	  }

	  // DCT
	  // Always stop skipping if we hit an msms_run_summary clsing tag.
	  // Fix for issue where end of file is not written if last search_result
	  // is empty, as observed on a Comet output file
	  if(! strcmp(tag->getName(), "msms_run_summary") || ! strcmp(tag->getName(), "msms_pipeline_analysis")  ) {
	    if (tag->isEnd()) {
	      tag_cache->clear();
	      output = True;
	      skip_hit = False;
	      skip_spec = False;
	    }
	  }
	  
	  if(USE_STD_MODIFICATIONS && tag->isStart() && ! strcmp(tag->getName(), "search_hit") 
		  && atoi(tag->getAttributeValue("hit_rank")) > max_rank_) {
	    skip_hit = True;
	  }

	  if(USE_STD_MODIFICATIONS && tag->isStart() && ! strcmp(tag->getName(), "search_hit") 
		  && atoi(tag->getAttributeValue("hit_rank")) <= max_rank_) {
	    skip_spec = False;
	    for (int c=0; c<tag_cache->size(); c++) {
	      (*tag_cache)[c]->write(fout);
	      //delete (*tag_cache)[c];
	    }
	    tag_cache->clear();
	    skip_hit = False;
	  }
	  if( !skip_hit && prot_name_ && !strcmp(tag->getName(), "alternative_protein") ) {
	    const char *protd = tag->getAttributeValue("protein_descr");
	    
	    if (protd) {
	      char* prot = new char [strlen(protd)+1];
	      strcpy(prot, protd);
	      char* sep = strchr(prot, ' ');
	      if (sep != NULL) {
		*sep = '\0';
	      }
	      tag->setAttributeValue("protein", prot);
	      delete [] prot;
	    }
	  }
	 
	  if (tag->isStart() && !strcmp(tag->getName(), "search_summary") && 
	      datapath_ != NULL) {                                              

	    string base = string(tag->getAttributeValue("base_name"));
	    unsigned found = base.find_last_of("/\\");
	    tag->setAttributeValue("base_name",base.substr(found+1).c_str());

	    basename = new char[strlen(datapath_) + strlen(tag->getAttributeValue("base_name")) + 2];
	    strcpy(basename, datapath_);
	    strcat(basename, tag->getAttributeValue("base_name"));
	    tag->setAttributeValue("base_name", basename);
	    delete [] basename;
	    dboutput = False;	      
	  }
	  
	  if (tag->isStart() && !strcmp(tag->getName(), "search_summary") && !strcmp(tag->getAttributeValue("search_engine"), "OMSSA")) {
	    cerr << "INFO:  OMSSA detected, pyro-mods and protein names will be autocorrected." << endl;
	    prot_name_ = True;
	    fix_pyro_mods_ = True;
	  }
	    
	 	    //DDS: Read Collision Energies from mzFile
	  if (tag->isStart() && !strcmp(tag->getName(), "msms_run_summary")){

	    if (datapath_ != NULL) {
	      string base = string(tag->getAttributeValue("base_name"));
	      unsigned found = base.find_last_of("/\\");
	      tag->setAttributeValue("base_name",base.substr(found+1).c_str());
	      
	      basename = new char[strlen(datapath_) + strlen(tag->getAttributeValue("base_name")) + 2];
	      strcpy(basename, datapath_);
	      strcat(basename, tag->getAttributeValue("base_name"));
	      tag->setAttributeValue("base_name", basename);
	      delete [] basename;
	    }
	    //verify_and_correct_path(tag);
	    if (collision_eng_ || comp_volt_ || prec_intens_ || get_rt_) {
	      
	      string mzFile = string(tag->getAttributeValue("base_name")) + 
		string(tag->getAttributeValue("raw_data"));
	     
	      if (pFI != NULL) {
		rampCloseFile(pFI);
		free (pScanIndex);
		pFI=NULL;
	      }
	      if ((pFI = rampOpenFile(mzFile.c_str())) == NULL)
		{
		  printf ("Could not open data file to read collision energies or compensation voltages%s\n",
			  mzFile.c_str());
		  fflush(stdout);
		  return;
		}
	      
	      indexOffset = getIndexOffset (pFI);
	      pScanIndex = readIndex (pFI, indexOffset, &iLastScan);
	      readRunHeader(pFI, pScanIndex, &runHeader, iLastScan);		 
	      
	    }
	  }

	  
	  if(setTagValue(tag, search_result, attr_name, &result_index)) {
	    char* spec_new ;
	    if (update_chg_) {
	      const char *spec = tag->getAttributeValue("spectrum");	   
	      const char *chg = tag->getAttributeValue("assumed_charge");
	      spec_new = new char [strlen(spec)+11];
	      strcpy(spec_new, spec);
	       //	      spec_new[strlen(spec)-1] = chg[0];
	      tag->setAttributeValue("assumed_charge", &spec_new[strlen(spec)-1]);
	
	    }
	    else {
	      const char *spec  = tag->getAttributeValue("spectrum");	 
	      spec_new = new char [strlen(spec)+32];
	    }

	    //Rewrite spectrum name
	    size_t pos, end;	    
	    string spec_str = tag->getAttributeValue("spectrum");

	    if ( ( pos = spec_str.find_first_of('.') )!= string::npos ) {
	      spec_str = spec_str.substr(0, pos);
	    }
	    
	    
	    string base = spec_str;

	    string start_scan = tag->getAttributeValue("start_scan") ? tag->getAttributeValue("start_scan") : "";
	    string end_scan = tag->getAttributeValue("end_scan") ?  tag->getAttributeValue("end_scan") : "";
	    string charge =  tag->getAttributeValue("assumed_charge") ? tag->getAttributeValue("assumed_charge") : "";

		
		if ((pos = spec_str.find_last_of("\\/")) != string::npos ) {
	        spec_str =  spec_str.substr(pos+1);
	    }


	    if ( charge == "" && (pos = spec_str.find_last_of('.')) != string::npos ) {
	      charge = spec_str.substr(pos+1);
	      spec_str =  spec_str.substr(0, pos);
	    }
	    
	    if ( end_scan == "" && (pos = spec_str.find_last_of('.')) != string::npos ) {
	      end_scan = spec_str.substr(pos+1);
	      spec_str =  spec_str.substr(0, pos);
	    }

	    if ( start_scan == "" && (pos = spec_str.find_last_of('.')) != string::npos ) {
	      start_scan = spec_str.substr(pos+1);
	      spec_str =  spec_str.substr(0, pos);
	    }
	    
	    if (tag->getAttributeValue("spectrumNativeID")!=NULL) {
	      start_scan = tag->getAttributeValue("spectrumNativeID");
	      
	      int sscan = -1, escan = -1;
	      
	      if (msd_ && (sscan = sl_->find(start_scan)+1) <= sl_->size()) {
		escan = sscan;
	      }
	      size_t pos_s;
	      size_t pos_e;
	      std::string whitespaces (" \t\f\v\n\r");
	      
	      if ( (pos_s=start_scan.find("scan=")) != string::npos ) {
		pos_s = pos_s + 5;
		pos_e = start_scan.find_first_of(whitespaces,pos_s);
		sscan = atoi(start_scan.substr(pos_s, pos_e-pos_s).c_str());
		escan = sscan;
	      }

	      if (sscan > 0 && escan > 0) 
		sprintf(spec_new, "%s.%05u.%05u.%u", 
			spec_str.c_str(), 
			sscan,
			escan,
			atoi(charge.c_str()));
	      stringstream sscan_ss, escan_ss;
	      sscan_ss << sscan;
	      tag->setAttributeValue("start_scan",  sscan_ss.str().c_str());

	      escan_ss << escan;
	      tag->setAttributeValue("end_scan",    escan_ss.str().c_str());
	    }
	    else {
	    
	    
	      sprintf(spec_new, "%s.%05u.%05u.%u", 
		      spec_str.c_str(), 
		      atoi(start_scan.c_str()),
		      atoi(end_scan.c_str()),
		      atoi(charge.c_str()));
	      tag->setAttributeValue("start_scan",  start_scan.c_str());
	      tag->setAttributeValue("end_scan",    end_scan.c_str());
	    }
	    
	    tag->setAttributeValue("spectrum", spec_new);
	    
	    delete [] spec_new;
	    
	    
	    if (exp_lbl_ != NULL) {
	      tag->setAttributeValue("experiment_label", exp_lbl_);
	    }
	    
	    if (collision_eng_ || comp_volt_ || prec_intens_ || get_rt_) {

	      long scan = atoi(tag->getAttributeValue("start_scan"));
	      readHeader (pFI, pScanIndex[scan], &scanHeader);
	      
	      ostringstream cE;
	      ostringstream cV;
	      ostringstream pI;
	      ostringstream rt;
	      if (collision_eng_) {
		cE << scanHeader.collisionEnergy;
		tag->setAttributeValue("collision_energy", cE.str().c_str());
	      }

	      if (comp_volt_) {
		cV << scanHeader.compensationVoltage;
		tag->setAttributeValue("compensation_voltage", cV.str().c_str());
	      }

	      if (prec_intens_) {
		pI << scanHeader.precursorIntensity;
		tag->setAttributeValue("precursor_intensity", pI.str().c_str());
	      }

	      if (get_rt_) {
		rt << scanHeader.retentionTime;
		tag->setAttributeValue("retention_time_sec", rt.str().c_str());
	      }

	      


	    }

	    //tag->write(fout);
	  }
	  else if (!skip_hit && fix_pyro_mods_ && tag->isStart() && ! strcmp(tag->getName(), "modification_info") && tag->getAttributeValue("mod_nterm_mass")) {

	    const char* mod_pep = tag->getAttributeValue("modified_peptide");
	    const char* mod_nterm_mass_str = tag->getAttributeValue("mod_nterm_mass");
	    double mod_nterm_mass = 0.0;
	    if (mod_nterm_mass_str) mod_nterm_mass = atof(mod_nterm_mass_str);
	    
	    if (strstr(mod_pep, "Q[111]") == mod_pep || strstr(mod_pep, "E[111]") == mod_pep || strstr(mod_pep, "C[143]") == mod_pep) {
	      Tag* new_tag = new Tag("modification_info", True, False);
	      new_tag->setAttributeValue("modified_peptide", mod_pep);
	      Tag* mod_aa_tag = new Tag("mod_aminoacid_mass", True, True);
	      mod_aa_tag->setAttributeValue("position","1");
	      mod_aa_tag->setAttributeValue("mass",tag->getAttributeValue("mod_nterm_mass"));
	      new_tag->write(fout);
	      delete new_tag;
	      mod_aa_tag->write(fout);
	      delete mod_aa_tag;
	      if (tag->isEnd()) {
		new_tag = new Tag("modification_info", False, True);
		new_tag->write(fout);
		delete new_tag;
	      }
	      
	    } else if ((mod_nterm_mass > 0.0001 || mod_nterm_mass < -0.0001) && strlen(mod_pep) > 1 && mod_pep[0] != 'n' && mod_pep[1] == '[') {
	      // looks like it puts the nterm mod on the amino acid. bad
	      mod_nterm_mass -= (ResidueMass::getMass(mod_pep[0], True) - ResidueMass::getMass('n', True)); // just subtract the AA mass
	      char buf[32];
	      sprintf(buf, "%.4f", mod_nterm_mass);
	      Tag* new_tag = new Tag("modification_info", True, False);
	      new_tag->setAttributeValue("mod_nterm_mass", buf); // didn't change the modified_peptide, let TPP deal with it
	      new_tag->write(fout);
	      if (tag->isEnd()) {
		new_tag = new Tag("modification_info", False, True);
		new_tag->write(fout);
		delete new_tag;
	      }
	    } else {
	      tag->write(fout);
	    }
	    
	    
	  }
	  else if(EXCLUDE_CONST_STATICS && tag->isStart() && ! strcmp(tag->getName(), "terminal_modification") &&
		  ! strcmp(tag->getAttributeValue("variable"), "N")) { // static terminus
	    // check if already seen

	    char nextmod = (tag->getAttributeValue("terminus"))[0];
	    double nextmass = atof(tag->getAttributeValue("mass"));
	    Boolean found = False;
	    for(int k = 0; k < static_consts->length(); k++) {
	      if((*static_consts)[k].mod == nextmod && (*static_consts)[k].mass - nextmass <= MOD_ERROR &&
		 nextmass - (*static_consts)[k].mass <= MOD_ERROR) {
		(*static_consts)[k].num++;
		found = True;;
	      }
	    }
	    if(! found) {
	      next_static_count.mod = nextmod;
	      next_static_count.mass = nextmass;
	      next_static_count.num = 1;
	      static_consts->insertAtEnd(next_static_count);
	    }
	    tag->write(fout);
	  }
	  else if(EXCLUDE_CONST_STATICS && tag->isStart() && ! strcmp(tag->getName(), "aminoacid_modification") &&
		  ! strcmp(tag->getAttributeValue("variable"), "N")) { // static aa
	    // check if already seen

	    char nextmod = (tag->getAttributeValue("aminoacid"))[0];
	    double nextmass = atof(tag->getAttributeValue("mass"));
	    Boolean found = False;
	    for(int k = 0; k < static_consts->length(); k++) {
	      if((*static_consts)[k].mod == nextmod && (*static_consts)[k].mass - nextmass <= MOD_ERROR &&
		 nextmass - (*static_consts)[k].mass <= MOD_ERROR) {
		(*static_consts)[k].num++;
		found = True;;
	      }
	    }
	    if(! found) {
	      next_static_count.mod = nextmod;
	      next_static_count.mass = nextmass;
	      next_static_count.num = 1;
	      static_consts->insertAtEnd(next_static_count);
	    }
	    tag->write(fout);
	  }
	  else if(EXCLUDE_CONST_STATICS && tag->isEnd() && ! strcmp(tag->getName(), "search_summary")) {
	    check_basename(tag); // make sure basename has entire path
	    static_tots++;
	    if (database_ != NULL && !dboutput) {
	      Tag *dbtag = new Tag("search_database", True, True);
	      dbtag->setAttributeValue("local_path", database_);
	      dbtag->setAttributeValue("type", dbtype_);
	      dbtag->write(fout);
	      delete dbtag;
	    }
	    tag->write(fout);
	  }
	  
	  else if(first) {
	    
	    if(tag->isStart() && ! strcmp(tag->getName(), pipeline_analysis)) {
	      skip_hit = False;
	      if(outertag == NULL)
		outertag = new OuterTag(tag);
	      
	      string schemaLoc = string(PEPXML_NAMESPACE) + " " + string(DEFAULT_PEPXML_STD_XSL) + string(PEPXML_SCHEMA);
	      outertag->setAttributeValue("xsi:schemaLocation", schemaLoc.c_str());
	      outertag->setAttributeValue("summary_xml", outfile_);
	      //outertag->write(cout);
	      
	      if(USE_XML_SCHEMA)
		fout << "<?xml-stylesheet type=\"text/xsl\" href=\"" << xslfile << "\"?>" << endl;
	      
	      tag->setAttributeValue("summary_xml", outfile_);
	      
	      tag->setAttributeValue("xsi:schemaLocation", schemaLoc.c_str());
	      tag->write(fout);
	      for(int k = 0; k < summary_tags->length(); k++)
		if((*summary_tags)[k] != NULL)
		  (*summary_tags)[k]->write(fout);
	      derivation_tag->write(fout); // derivation set to generation 0
	    }
	    
	    else if(!skip_tag && !skip_hit && doc_filter->filter(tag)) {
	      if(! USE_XML_SCHEMA) {
		tag->write(fout);
		// add new tag... for xsl file
		fout << "<?xml-stylesheet type=\"text/xsl\" href=\"" << xslfile << "\"?>" << endl;
	      }
	      
	    }
	    else if( !skip_spec && !skip_tag && !skip_hit &&  (! tag->isEnd() || strcmp(tag->getName(), pipeline_analysis)) && ! xmlstyle_filter->filter(tag)) {
	      if (output)
		tag->write(fout);
	      else
		oldsummaries->insertAtEnd(tag);
	    }
	    else if( !skip_spec && !skip_tag && !skip_hit && last && tag->isEnd() && ( ! strcmp(tag->getName(), pipeline_analysis) || ! strcmp(tag->getName(), "msms_run_summary"))) { // only 1 input file
	      tag->write(fout);
	    }
	  }

	  
	
	  else if(tag->isStart() && ! strcmp(tag->getName(), pipeline_analysis) && outertag != NULL) {
	    OuterTag* nextouter = new OuterTag(tag);
	    //nextouter->write(cout);
	    if(nextouter != NULL) {
	      outertag->enterRefs(nextouter);
	      delete nextouter;
	    }
	  }
	
	  
	  
	  else if(!skip_spec && !skip_tag && !skip_hit && ! xml_filter->filter(tag) && ! xmlstyle_filter->filter(tag) &&
		  ! doc_filter->filter(tag) && ! deriv_filter->filter(tag)) {
	    if(strcmp(tag->getName(), pipeline_analysis)) {
	      if(output)
		tag->write(fout);
	      else
		oldsummaries->insertAtEnd(tag);
	      //tag->write(fout);
	      
	    }
	    else if(!skip_spec && !skip_tag && !skip_hit && tag->isEnd() && last)
	      tag->write(fout);
	  }
	  
	  if(output || skip_hit){
	    delete tag;
	    tag = NULL;
	  }
	} // if not null
	
	
	data = strstr(data+1, "<");
      } // next tag
      } // handle multiple tags on a line
      
    } // next line
    fin.close();
    if (collision_eng_||comp_volt_||prec_intens_||get_rt_) {
      rampCloseFile(pFI);
      free (pScanIndex);
    }
  } // next inputfile
  delete [] cpydata;
  fout.close();

  cout << " processed altogether " << result_index-1 << " results" << endl;
  if(EXCLUDE_CONST_STATICS) {
    for(int k = 0; k < static_consts->length(); k++)
      if((*static_consts)[k].num == static_tots)
	; //cout << "mod: " << (*static_consts)[k].mod << " mass: " << (*static_consts)[k].mass << endl;
      else 
	static_consts->remove(k--);
  } // if USE


  unlink(outfile_); // sometimes old cygwin files won't reopen for write
  ofstream fout2(outfile_);
  if(! fout2) {
    FILE *f=fopen(outfile_,"w"); // just to set errno
    cerr << "cannot write output to file " << outfile_ << ": " << strerror(errno) << endl;
    exit(1);
  }
  ifstream fin2(tmp_outfile.c_str());
  skip_hit = False;

  // look for first 
  while(fin2.getline(nextline, line_width_)) {
    //cout << "next: " << nextline << endl;
    if(strstr(nextline, "dataset_derivation") != NULL || strstr(nextline, pipeline_analysis) != NULL ||
       (USE_STD_MODIFICATIONS && 
	(mod_on || strstr(nextline, "search_hit") != NULL ||  strstr(nextline, "modification_info") != NULL ))) {

      data = strstr(nextline, "<");
      while(data != NULL) {
	tag = new Tag(data);
	Boolean output = True;
	if(tag != NULL) {

	  if(tag->isStart() && ! strcmp(tag->getName(), pipeline_analysis) && outertag != NULL) {
	    outertag->setAttributeValue("date", getDateTime());
	    //outertag->write(cout);
	    //if (!skip_hit) 
	      outertag->write(fout2);
	    delete outertag;
	    outertag = NULL;
	  }
	  else  if(datagen_filt->filter(tag)) {
	    // now the summary
	    for(int k = 0; k < oldsummaries->length(); k++)
	      if((*oldsummaries)[k] != NULL) {
		(*oldsummaries)[k]->write(fout2);
		delete (*oldsummaries)[k];
		(*oldsummaries)[k] = NULL;
	      }
	    //if (!skip_hit) 
	      tag->write(fout2);
	  }
	  else if(! strcmp(tag->getName(), "search_result") || ! strcmp(tag->getName(), "spectrum_query")) {
	    skip_hit = False;
	  }
	  else if(USE_STD_MODIFICATIONS && tag->isStart() && ! strcmp(tag->getName(), "search_hit") 
		  && atoi(tag->getAttributeValue("hit_rank")) > max_rank_) {
	    skip_hit = True;
	  }
	  else if(USE_STD_MODIFICATIONS && tag->isStart() && ! strcmp(tag->getName(), "search_hit") 
	    && atoi(tag->getAttributeValue("hit_rank")) <= max_rank_) { // TODO: DDS: Modify here to allow the lower down hits 
	    skip_hit = False;
	    if ((int)strlen(tag->getAttributeValue("peptide")) < min_peplen_) {
	      tag->setAttributeValue("is_rejected", "1");
	    }
	    
	    if (strpbrk(tag->getAttributeValue("peptide"), "Xx") != NULL) {
	      tag->setAttributeValue("is_rejected", "1");
	    }

	    if (prot_name_) {
	      const char *protd = tag->getAttributeValue("protein_descr");	      
	      char* prot = new char [strlen(protd)+1];
	      strcpy(prot, protd);
	      char* sep = strchr(prot, ' ');
	      if (sep != NULL) {
		*sep = '\0';
	      }
	      tag->setAttributeValue("protein", prot);
	      delete [] prot;
	    }

	    const char *pep = tag->getAttributeValue("peptide");
	    const char *paa = tag->getAttributeValue("peptide_prev_aa");
	    const char *naa = tag->getAttributeValue("peptide_next_aa");
	    

	    if (enz_ != NULL) {
	      if (paa == NULL || paa[0] == '\0') {
		tag->setAttributeValue("peptide_prev_aa", "-");
	      }
	      
	      if (naa == NULL || naa[0] == '\0') {
		tag->setAttributeValue("peptide_next_aa", "-");
	      }
	      char* buf = new char[2];
	      sprintf(buf, "%d", enzyme_->getNumTolTerm(tag->getAttributeValue("peptide_prev_aa")[0], tag->getAttributeValue("peptide"), tag->getAttributeValue("peptide_next_aa")[0]));
	      tag->setAttributeValue("num_tol_term", buf);
	      char * pepWithEnds = new char [strlen(pep)+5];

	      sprintf(pepWithEnds, "%s.%s.%s", tag->getAttributeValue("peptide_prev_aa"), tag->getAttributeValue("peptide"), tag->getAttributeValue("peptide_next_aa"));
	      sprintf(buf, "%d", enzyme_->getNumMissedCleavages(pepWithEnds));
	      tag->setAttributeValue("num_missed_cleavages", buf);
	      delete [] pepWithEnds;
	      delete [] buf;
	    }

	    if (pep) {
	      strcpy(peptide, pep);
	      if (!skip_hit) 
		tag->write(fout2);
	    } else {
	      std::cerr << "warning: skipping search_hit element without any peptide member" << endl;
	    }
	  }
	  else if(USE_STD_MODIFICATIONS && tag->isStart() && ! strcmp(tag->getName(), "modification_info")) {
	    modifications = new Array<Tag*>;
	    modifications->insertAtEnd(tag);
	    mod_on = True;
	    output = False;

	    if(tag->isEnd()) {
	      output = False;
	      // process now
	      // get rid of all static modifications (marking them)
	      mod_info = new ModificationInfo(modifications);
	      if(mod_info != NULL && strlen(peptide) > 0 && modifications->length() > 0 && (*modifications)[0]->isStart() &&
		 ! strcmp((*modifications)[0]->getName(), "modification_info")) {
		char* stdpep = NULL;
		if(EXCLUDE_CONST_STATICS) 
		  stdpep = mod_info->getStandardModifiedPeptide(peptide, static_consts, MOD_ERROR);
		else
		  stdpep = mod_info->getStandardModifiedPeptide(peptide, NULL, 0.0);
		if(stdpep != NULL) {
		  (*modifications)[0]->setAttributeValue("modified_peptide", stdpep);
		  delete[] stdpep;
		}
		delete mod_info;
	      }

	      // now write out
	      for(int k = 0; k < modifications->length(); k++)
		if((*modifications)[k] != NULL  && !skip_hit) {
		  (*modifications)[k]->write(fout2);
		  delete (*modifications)[k];
		}
	      delete modifications;
	      mod_on = False;
	      peptide[0] = 0;
	    } // if end modification tag


	  }
	  else if(USE_STD_MODIFICATIONS && tag->isEnd() && ! strcmp(tag->getName(), "modification_info") ) {
	    modifications->insertAtEnd(tag);
	    output = False;
	    // process now
	    // get rid of all static modifications (marking them)
	    mod_info = new ModificationInfo(modifications);
	    if(mod_info != NULL && strlen(peptide) > 0 && modifications->length() > 0 && (*modifications)[0]->isStart() &&
	       ! strcmp((*modifications)[0]->getName(), "modification_info")) {
	      char* stdpep = NULL;
	      if(EXCLUDE_CONST_STATICS) 
		stdpep = mod_info->getStandardModifiedPeptide(peptide, static_consts, MOD_ERROR);
	      else
		stdpep = mod_info->getStandardModifiedPeptide(peptide, NULL, 0.0);
	      if(stdpep != NULL) {
		(*modifications)[0]->setAttributeValue("modified_peptide", stdpep);
		delete stdpep;
	      }
	      delete mod_info;
	    }

	    // now write out
	    for(int k = 0; k < modifications->length(); k++)
	      if((*modifications)[k] != NULL && !skip_hit) {
		(*modifications)[k]->write(fout2);
		delete (*modifications)[k];
	      }
	    delete modifications;
	    mod_on = False;
	    peptide[0] = 0;
	  }
	  else if(USE_STD_MODIFICATIONS && mod_on ) {
	    modifications->insertAtEnd(tag);
	    output = False;
	  }
	  else {
	    if (!skip_hit) 
	      tag->write(fout2);
	  }

	  if(output || skip_hit) {
	    delete tag;
	    tag = NULL;
	  }

	} // if make tag

	data = strstr(data+1, "<");
      } // next tag

  
    } // if have dataset derivation
    else { 
      if (!skip_hit) 
	fout2 << nextline << endl;
    }
  } // next line
  fout2.close();
  fin2.close();

  // verify that the tag was successfully written to the end of outfile
  if(! tag_is_at_tail( outfile_, "</msms_pipeline_analysis>")) {
    cerr << "ERROR: output file created, " << outfile_ << ", appears incomplete. Please check the input files for completeness. " << endl;
    //exit(1);
  }
  // do gzip compression on result if filename indicates it
  if (isDotGZ(outfile_)) {
	  std::string outf(outfile_);
	  do_gzip(outf);
  }
  
  verified_unlink(tmp_outfile);

  
  if (false) { // if (getIsInteractiveMode()) { // TPP (web oriented) vs LabKey (headless)

      char command[5000];
      char xslmaker[1000];
      
#if defined(USING_RELATIVE_WEBSERVER_PATH)
      sprintf(xslmaker, "%s%s", getCGIFullBin(), "pepxml2html.pl -file");
#else
      sprintf(xslmaker, "%s%s", LOCAL_BIN, "pepxml2html.pl -file");
#endif
      delete[] szWebserverRoot;
      
      strcpy(command, xslmaker);
      strcat(command, " ");
      strcat(command, outfile_);
      strcat(command, " 1"); // for now
      FILE* pipe;
      if ( (pipe=tpplib_popen(command, "r"))==NULL)
	{
	  printf(" Error - cannot open input file %s\n\n", command);
	  exit(0);
	}
      
      while(fgets(szBuf, SIZE_BUFF, pipe)) 
	printf("%s\n", szBuf);
      
      pclose(pipe);
    }
    else {
      cerr << "INFO: Results written to file: " << outfile_ << endl;
    }
  delete[] nextline;
}


void InteractParser::setFilter(Tag* tag) {
  if(tag == NULL)
    return;

  if(filter_memory_) {
    filter_memory_ = False;
    filter_ = False;
  }

  if(! strcmp(tag->getName(), "search_result")){  
    if(tag->isStart()) {
      filter_ = True;
    }else{
      filter_memory_ = True;
    }
  }

}
