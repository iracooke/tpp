#include "InterProphetParser.h"

/*

Program       : InterProphet                                                       
Author        : David Shteynberg <dshteynb  AT systemsbiology.org>                                                       
Date          : 12.12.07

Primary data object holding all mixture distributions for each precursor ion charge

Copyright (C) 2007 David Shteynberg

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

David Shteynberg
Institute for Systems Biology
401 Terry Avenue North 
Seattle, WA  98109  USA
akeller@systemsbiology.org

*/

#pragma  warning(disable: 4800)

InterProphetParser::InterProphetParser(bool nss_flag, bool nrs_flag, bool nse_flag, bool sharp_nse, bool nsi_flag, bool nsm_flag, bool nsp_flag, bool use_fpkm, bool use_length, string* catfile, string* decoyTag, int max_threads) : Parser("interprophet") { 
  //testMode_ = NULL;
  max_threads_ = max_threads;
  use_cat_ = false;
  catfile_ = NULL;
  use_decoy_ = false;
  if (catfile != NULL) {
    catfile_ = new string(*catfile);
    use_cat_ = true;
  }
  if (decoyTag != NULL) {
    decoyTag_ = new string(*decoyTag);
    use_decoy_ = true;
  }
  inter_proph_ = new InterProphet(nss_flag, nrs_flag, nse_flag, sharp_nse, nsi_flag, nsm_flag,  nsp_flag, use_fpkm, use_length, use_cat_!=False, max_threads);
  input_files_ = new Array<string*>();
  anal_summs_ = new Array<Tag*>();
  ms_runs_ = new int_hash();
  outfile_ = new string();
}
InterProphetParser::~InterProphetParser() {
  //TODO implement me
}

bool InterProphetParser::setOutFile(const char* c) {
  int dirsize = 5000;
  char* curr_dir = new char[dirsize];
  outfile_->assign(c);
  if(outfile_->length() > 1 && !isAbsolutePath(*outfile_)) {
    char *ret = getcwd(curr_dir, dirsize); // gcc 4.3 insists we look at return value
    outfile_ ->assign(ret?curr_dir:"???");
    outfile_->append("/");
    outfile_->append(c);
  }
  delete [] curr_dir;
  return true;
}

void InterProphetParser::addFile(const char* filename) {
  string* name = new string(filename);
  //cerr << "DDS DEBUG: inserting file " << filename << endl;
  input_files_->insertAtEnd(name);
}

//void InterProphetParser::printResult() {
//  inter_proph_->printAdjProbs();
//  inter_proph_->reportModels(cout);
//}

void InterProphetParser::parse_catfile() {
  if (use_cat_) {
    pwiz::util::random_access_compressed_ifstream fin(catfile_->c_str()); // read possibly gzipped files
    if(! fin) {
      cerr << "fin: error opening " << catfile_ << endl;
      exit(1);
    }
    string pep;
    string cat;
    while(fin >> pep) {
      fin >> cat;
      inter_proph_->addPeptideCategory(pep, cat);
    }

  }
}

void InterProphetParser::run() {
  init(NULL);
}

void InterProphetParser::parse(const char* c) {

  Array<Tag*>* tags = NULL;
  Tag* tag = NULL;

  char *nextline = new char[line_width_];
  char* data = NULL;


  string spectrum_name = "";
  string exp_lbl = "";
  string charge = "";
  double prob = 0;
  Array<double>* allntt_prob=NULL ;
  double calcnmass = -1;
  double rt = -1;
  string pep_seq = "";
  string mod_pep = "";
  string swath_assay = "";

  Array<string*>* prots = NULL;
  string* prot = NULL;
  double maxFPKM = 0l;
  double emp_irt = -999999999;
  int swath_window = -1;
  int alt_swath = -1;
  bool is_decoy = false;
  bool get_pep = false;
  bool peppro_go = false;

  int k=0;
  // TODO  double allntt_prob[3] = {-100, -100, -100};
  for(k = 0; k < input_files_->length(); k++) {
    pwiz::util::random_access_compressed_ifstream fin((*input_files_)[k]->c_str()); // read possibly gzipped files
    if(! fin) {
      cerr << "fin: error opening " << (*input_files_)[k]->c_str() << endl;
      exit(1);
    }

    while(fin.getline(nextline, line_width_)) {
      data = strstr(nextline, "<");
      while(data != NULL) {
	tag = new Tag(data);
	
	if (tag != NULL) {
	  
	      
	  if (tag->isStart() && 
	      ! strcmp(tag->getName(), "analysis_summary") && 
	      !strcmp(tag->getAttributeValue("analysis"), "peptideprophet") ) {
	    Tag* newtag = new Tag(*tag);
	    newtag->setEnd();
	    anal_summs_->insertAtEnd(newtag);	  
	  }
	      

	  if (tag->isStart() && 
	      ! strcmp(tag->getName(), "msms_pipeline_analysis")) {
	    //(*input_files_)[k]->assign(tag->getAttributeValue("summary_xml"));
	    inter_proph_->addSearch((*input_files_)[k]);
	  }
	  else if (tag->isStart() && 
	      ! strcmp(tag->getName(), "msms_run_summary")) {
	    string* msrun = new string(tag->getAttributeValue("base_name"));
	    int_hash::iterator it = ms_runs_->find(*msrun);
	    if (it == ms_runs_->end()) {
	      ms_runs_->insert(make_pair(*msrun, 1));
	    }
	    else {
	      delete msrun;
	    }
	  }
	  else if (tag->isStart() && 
		   ! strcmp(tag->getName(), "spectrum_query")) {
	    is_decoy = false;
	    get_pep = true;
	    emp_irt = -99999999999;
	    swath_window = -1;
	    pep_seq = "";
	    mod_pep = "";
	    swath_assay = "";
	    spectrum_name = "";
	    rt = -1;
	    if (tag->getAttributeValue("retention_time_sec") != NULL) 
	      rt = atof(tag->getAttributeValue("retention_time_sec"));

	    // store the spectrum name for later
	    int len = strlen(tag->getAttributeValue("spectrum"));
	    
	    spectrum_name += tag->getAttributeValue("spectrum");
	    
	    if (tag->getAttributeValue("swath_assay") != NULL) {
	      swath_assay = tag->getAttributeValue("swath_assay");
	    }

	    if (tag->getAttributeValue("experiment_label") != NULL) {
	      len = strlen(tag->getAttributeValue("experiment_label"));
	    }
	    else {
	      len = 0;
	    }
	    exp_lbl = "";
	    if (len > 0)
	      exp_lbl += tag->getAttributeValue("experiment_label");
	    //char* tmp = strrchr(spectrum_name, '.');
	    //tmp = '\0';
	    charge = tag->getAttributeValue("assumed_charge");
	    
	  }
	  else if (tag->isStart() && 
		   ! strcmp(tag->getName(), "parameter") && 
		   tag->getAttributeValue("name") != NULL && !strcmp(tag->getAttributeValue("name"), "empir_irt") ) {
	    emp_irt = atof(tag->getAttributeValue("value"));
	    
	  }
	  else if (tag->isStart() && 
		   ! strcmp(tag->getName(), "parameter") && 
		   tag->getAttributeValue("name") != NULL && !strcmp(tag->getAttributeValue("name"), "swath_window") ) {
	    swath_window = atoi(tag->getAttributeValue("value"));
	    
	  }
	  else if (tag->isStart() && 
		   ! strcmp(tag->getName(), "parameter") && 
		   tag->getAttributeValue("name") != NULL && !strcmp(tag->getAttributeValue("name"), "alt_swath") ) {
	    alt_swath = atoi(tag->getAttributeValue("value"));
	    
	  }
	  else if (tag->isStart() && 
		   ! strcmp(tag->getName(), "search_hit") && 
		   atoi(tag->getAttributeValue("hit_rank")) == 1) {
	    pep_seq = "";
	    pep_seq += tag->getAttributeValue("peptide");
	    calcnmass = atof(tag->getAttributeValue("calc_neutral_pep_mass"));
	    maxFPKM = 0l;
	    prots = new Array<string*>();
	    prot = new string();
	    
	    if (tag->getAttributeValue("protein_descr")!=NULL) {
	      *prot = "" + string(tag->getAttributeValue("protein_descr"));
	    }

	    size_t start = prot->find("FPKM=");
	    size_t end=-1;
	    
	    if (start != string::npos) {
	      start+=5;
	      end = prot->find_first_of(" \t\f\v\n\r", start);
	      maxFPKM = atof(prot->substr(start, end-start).c_str());
	    }
	    
	    *prot = "" + string(tag->getAttributeValue("protein"));
	    prots->insertAtEnd(prot);
	    if (use_decoy_ && prot->find(*decoyTag_) == 0) {
	      is_decoy = true;
	    }
	    //get_pep = false;
	    
	  }
	  else if (tag->isStart() && 
		   ! strcmp(tag->getName(), "search_hit")) {
	    prot = NULL;
	    
	  }
	  else if (tag->isStart() && 
		   ! strcmp(tag->getName(), "alternative_protein") && prot != NULL) {
	    prot = new string();
	    if (tag->getAttributeValue("protein_descr")!=NULL) {
	      *prot = "" + string(tag->getAttributeValue("protein_descr"));
	    }
	    size_t start = prot->find("FPKM=");
	    size_t end=-1;
	    double nextFPKM = 0l;
	    if (start != string::npos) {
	      start+=5;
	      end = prot->find_first_of(" \t\f\v\n\r", start);
	      nextFPKM = atof(prot->substr(start, end-start).c_str());
	    }
	    
	    if (maxFPKM < nextFPKM) {
	      maxFPKM = nextFPKM;
	    }

	    *prot = "" + string(tag->getAttributeValue("protein"));
	    if (use_decoy_ && prot->find(*decoyTag_) == 0) {
	      is_decoy &= true;
	    }
	    else {
	      is_decoy = false;
	    }
	    prots->insertAtEnd(prot);
	  }
	  else if (tag->isStart() && get_pep &&
		   ! strcmp(tag->getName(), "modification_info")) {      
	    get_pep = false;
	    mod_pep = "";

	    if (!tag->getAttributeValue("modified_peptide")) {
	      cerr << "ERROR: modified_peptide sequences are not written in the pep.xml file, please run InteractParser on the file first!" << endl;
	      exit(1);
	    }

	    mod_pep += tag->getAttributeValue("modified_peptide");
	  }

	  else if ( ! strcmp(tag->getName(), "peptideprophet_result")) {

	    // got the spectrum name and probability
	    if (tag->isStart() ) {
	      peppro_go = false;
	    

	      prob = atof( tag->getAttributeValue("probability") );
	      if (mod_pep == "") {
		mod_pep = pep_seq;
	      }
	      allntt_prob = new Array<double>(3);
	      //TODO: This parsing is fragile, perhaps move to boost regex
	      const char* nttprob =  tag->getAttributeValue("all_ntt_prob");
	      char* buf = new char[strlen(nttprob)];
	      strcpy(buf, nttprob+1);
	      char* c = strchr(buf, ',');
	      *c = '\0';
	      (*allntt_prob)[0] = atof(buf);
	      int len = strlen(buf)+2;
	      strcpy(buf, nttprob+len);
	      c = strchr(buf, ',');
	      *c = '\0';
	      (*allntt_prob)[1] = atof(buf);
	      strcpy(buf, strrchr(nttprob,',')+1);
	      c = strchr(buf, ')');
	      *c = '\0';
	      (*allntt_prob)[2] = atof(buf);
	      delete [] buf;
	    }

	    if (tag->isStart() && (tag->getAttributeValue("analysis") == NULL || ( strcmp(tag->getAttributeValue("analysis"), "none") && strcmp(tag->getAttributeValue("analysis"), "incomplete"))))  {
	      peppro_go = true;
	    }
		
	    if (tag->isEnd() && peppro_go ) {
	      inter_proph_->insertResult(k, spectrum_name, prob, allntt_prob, pep_seq, mod_pep, swath_assay, calcnmass, exp_lbl, charge, prots, is_decoy, maxFPKM, emp_irt, swath_window, alt_swath);
	    }

	  }
	  else if (tag->isEnd() && 
		   ! strcmp(tag->getName(), "spectrum_query")) {
	    exp_lbl = "";
	    spectrum_name = "";
	    charge = "";
	    get_pep = false;
	    pep_seq = "";
	    swath_assay = "";
	    mod_pep = "";
	    prob = 0;
	     // TODO  allntt_prob[0] = -100; allntt_prob[1] = -100; allntt_prob[2] = -100;
	  }
		 	
	}


	delete tag;
       
	data = strstr(data+1, "<");      
      }
    }
    fin.close();

  }
  delete [] nextline;
  if (max_threads_ < 7) {
    inter_proph_->computeModels();
  }
  else {
    inter_proph_->computeModelsThreaded();
  }


}


void InterProphetParser::writePepSHTML() {

  char command[5000];
  char szBuf[SIZE_BUF];
  char xslmaker[1000];
  char *szWebserverRoot = new char[400]; 
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
    szWebserverRoot[i] = tolower(szWebserverRoot[i]);
  }
  if (!isPathSeperator(szWebserverRoot[tmpLen-1])) {
    szWebserverRoot[tmpLen++] = '/';
  }
  szWebserverRoot[tmpLen] = 0;

  fixPath(szWebserverRoot,0); // tidy up path seperators etc


#else
  // output xsl name relative to webserver root
  const char *wsr=getWebserverRoot();
  int wsrlen = wsr?strlen(wsr):0;
  if (wsrlen && isPathSeperator(wsr[wsrlen-1])) {
     wsrlen--; // leave the trailing / alone (so it becomes leading)
  }


#endif // end if not win32
  delete [] szCommand;

  
#if defined(USING_RELATIVE_WEBSERVER_PATH)
  sprintf(xslmaker, "%s%s", getCGIFullBin(), "pepxml2html.pl -file");
#else
  sprintf(xslmaker, "%s%s", LOCAL_BIN, "pepxml2html.pl -file");
#endif
  delete[] szWebserverRoot;
  
  strcpy(command, xslmaker);
  strcat(command, " ");
  strcat(command, outfile_->c_str());
  strcat(command, " 1"); // for now
  FILE* pipe;
  if ( (pipe=tpplib_popen(command, "r"))==NULL)
    {
      printf(" Error - cannot open input file %s\n\n", command);
      exit(0);
    }
  
  while(fgets(szBuf, SIZE_BUF, pipe)) 
    printf("%s\n", szBuf);
  
  pclose(pipe);
  
}

void InterProphetParser::writePepXML() {
  Array<Tag*>* tags = NULL;
  
  Tag* tag = NULL;

  char *nextline = new char[line_width_];
  char* data = NULL;


  string spectrum_name = "";
  string exp_lbl = "";
  double prob = 0;
  double calcnmass = -1;
  string pep_seq = "";
  string mod_pep = "";
  string swath_assay = "";
  bool get_pep = false;
  bool pepproph = false;
  int k=0;
  int_hash::iterator itr = ms_runs_->begin();
  ofstream fout(outfile_->c_str());
  // TODO  double allntt_prob[3] = {-100, -100, -100};
  
  fout << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;

  //TODO add stylesheet and dynamic schema references
  fout << "<msms_pipeline_analysis date=\"" <<time_ 
       << "\" xmlns=\"http://regis-web.systemsbiology.net/pepXML\" "
       << "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
       << "xsi:schemaLocation=\"http://regis-web.systemsbiology.net/pepXML "
       << "/tools/bin/TPP/tpp/schema/pepXML_v18.xsd\" summary_xml=\"" 
       << outfile_->c_str() << "\">" << endl;
  
  fout << "<analysis_summary analysis=\"interprophet\" time=\"" <<time_ << "\">" << endl;

  for(k = 0; k < input_files_->length(); k++) {
    fout << "<inputfile name=\"" << (*input_files_)[k]->c_str() << "\"/>" << endl;
  }

  Array<Tag*>* roc_tags = inter_proph_->getRocDataPointTags();
  
  for (k = 0; k < roc_tags->length(); k++) {
    (*roc_tags)[k]->write(fout);
  }
  fout << endl;
  inter_proph_->reportModels(fout);

  fout << "</analysis_summary>" << endl;



  bool output = false;
  bool done = false;
  bool found = false;
  for (itr = ms_runs_->begin(); itr!= ms_runs_->end(); itr++) {
    const string* ms_run = &itr->first;

    for(k = 0; k < input_files_->length(); k++) {
      output = false;
      done = false;
      found = false;
      pwiz::util::random_access_compressed_ifstream fin((*input_files_)[k]->c_str());
      if(! fin) {
	cerr << "fin: error opening " << (*input_files_)[k]->c_str() << endl;
	exit(1);
      }
      while(fin.getline(nextline, line_width_)) {
	if (done) {
	  break;
	}
	
	data = strstr(nextline, "<");
	
	if (output) {
	  fout << nextline << endl;
	}
	
	while(data != NULL) {
	  tag = new Tag(data);
	  
	  if (tag != NULL) {
	    if (tag->isStart() && 
		! strcmp(tag->getName(), "msms_run_summary")) {
	      string* tmp = new string(tag->getAttributeValue("base_name"));
	      if (*ms_run == *tmp) {
		found = true;
		output = true;
		tag->write(fout);
	      }
	      delete tmp;
	    }
	    else if (found && tag->isEnd() && !strcmp(tag->getName(), "search_summary")) {
	      output = false;
	    }
	    else if (!strcmp(tag->getName(), "analysis_summary")) {
	      // pass through any analysis summary that isn't peptideprophet
	      
	      if (tag->isStart() &&
		  strcmp(tag->getAttributeValue("analysis"),"peptideprophet")) {
		  tag->write(fout); // and copy the current	line
		  output = !tag->isEnd();
	      }
	      else { 
		output = false; // stop copying
	      }
	    }
	    else if (found && tag->isStart() && tag->isEnd() && !strcmp(tag->getName(), "analysis_timestamp")) {
	      output = false; 
	      tag->write(fout);
	    }
	    else if (found && tag->isStart() && !strcmp(tag->getName(), "analysis_timestamp")) {
	      output = true;
	      tag->write(fout);
	    }
	    else if (found && tag->isEnd() && !strcmp(tag->getName(), "analysis_timestamp")) {
	      output = false;
	    }
	    else if (found && tag->isEnd() && 
		     ! strcmp(tag->getName(), "msms_run_summary")) {
	      tag->write(fout);
	      done = true;
	    }
	    
	    else if (found && tag->isStart() && 
		! strcmp(tag->getName(), "spectrum_query")) {
	      get_pep = false;
	      pep_seq = ""; mod_pep = "";
	      // store the spectrum name for later
	      int len = strlen(tag->getAttributeValue("spectrum"));
	      spectrum_name = "";
	      spectrum_name += tag->getAttributeValue("spectrum");
	      
	      if (tag->getAttributeValue("experiment_label") != NULL) {
		len = strlen(tag->getAttributeValue("experiment_label"));
	      }
	      else {
		len = 0;
	      } 
	      exp_lbl = "";
	      if (len > 0)
		exp_lbl += tag->getAttributeValue("experiment_label");
	      
	      int best = inter_proph_->getBestMatch(exp_lbl, spectrum_name, swath_assay);
	      if ( best == k ) {
		output = true;
		tag->write(fout);
	      }
	      else {
		output = false;
	      }
	      
	    }
	    else if (found && tag->isStart() && 
		     ! strcmp(tag->getName(), "search_hit") && 
		     atoi(tag->getAttributeValue("hit_rank")) == 1) {
	      pep_seq += tag->getAttributeValue("peptide");
	      calcnmass = atof(tag->getAttributeValue("calc_neutral_pep_mass"));
	      //get_pep = false;
	      
	    }
	    else if (found && tag->isStart() && get_pep &&
		     ! strcmp(tag->getName(), "modification_info")) {
	      
	      get_pep = false;
	      mod_pep = "";
	      mod_pep += tag->getAttributeValue("modified_peptide");
	      
	    }
	    else if (found && tag->isStart() && 
		     ! strcmp(tag->getName(), "peptideprophet_result") && 
		     (tag->getAttributeValue("analysis") == NULL || 
		      (strcmp(tag->getAttributeValue("analysis"), "none") &&
		       strcmp(tag->getAttributeValue("analysis"), "incomplete")) ) ) {
		       
	      // got the spectrum name and probability
	      prob = atof( tag->getAttributeValue("probability") );
	      //inter_proph_->insertResult(k, spectrum_name, prob, pep_seq, calcnmass);
	      
	      
	    }
	    else if (found && tag->isEnd() && 
		     ! strcmp(tag->getName(), "spectrum_query")) {
	      output = false;
	      spectrum_name = "";
	      exp_lbl = "";
	      get_pep = false;
	      pep_seq = "";
	      prob = 0;
	      // TODO  allntt_prob[0] = -100; allntt_prob[1] = -100; allntt_prob[2] = -100;
	    }
	    else if (found && tag->isStart() && ! strcmp(tag->getName(), "analysis_result") &&! strcmp(tag->getAttributeValue("analysis"), "peptideprophet")) {
	      pepproph = true;
	    }
	    else if (pepproph && found && tag->isStart() &&  
		     ! strcmp(tag->getName(), "peptideprophet_result") && 
		     tag->getAttributeValue("analysis") != NULL && 
		     !( strcmp(tag->getAttributeValue("analysis"), "none") &&
		        strcmp(tag->getAttributeValue("analysis"), "incomplete")) ) {
	      pepproph = false;
	      output = true;
	    }
	    else if (found && pepproph && tag->isEnd() && ! strcmp(tag->getName(), "analysis_result")) {
	      pepproph = false;
	      if (output) {
		fout << "<analysis_result analysis=\"interprophet\">" << endl;
		//TODO Add all ntt probs here
		fout << "<interprophet_result probability=\"" << inter_proph_->getAdjProb(exp_lbl, spectrum_name, swath_assay) 
		     << "\" all_ntt_prob=\"("  
		     << inter_proph_->getNTTAdjProb(exp_lbl, spectrum_name, swath_assay, 0) << ","
		     << inter_proph_->getNTTAdjProb(exp_lbl, spectrum_name, swath_assay, 1) << ","
		     << inter_proph_->getNTTAdjProb(exp_lbl, spectrum_name, swath_assay, 2) << ")\">" << endl; 
		fout << "<search_score_summary>" << endl;

		if ( inter_proph_->useNSSModel()) 
		  fout << "<parameter name=\"nss\"" << " value=\"" << inter_proph_->getNSSValue(k, exp_lbl, spectrum_name, swath_assay) << "\"/>" << endl;
		//		fout << "<parameter name=\"nss_adj_prob\"" << " value=\"" << inter_proph_->getNSSAdjProb(k, exp_lbl, spectrum_name) << "\"/>" << endl;
		if ( inter_proph_->useNRSModel()) 
		  fout << "<parameter name=\"nrs\"" << " value=\"" << inter_proph_->getNRSValue(k, exp_lbl, spectrum_name, swath_assay) << "\"/>" << endl;
		//		fout << "<parameter name=\"nrs_adj_prob\"" << " value=\"" << inter_proph_->getNRSAdjProb(k, exp_lbl, spectrum_name) << "\"/>" << endl;
		if ( inter_proph_->useNSEModel()) 
		  fout << "<parameter name=\"nse\"" << " value=\"" << inter_proph_->getNSEValue(k, exp_lbl, spectrum_name, swath_assay) << "\"/>" << endl;
		//		fout << "<parameter name=\"nse_adj_prob\"" << " value=\"" << inter_proph_->getNSEAdjProb(k, exp_lbl, spectrum_name) << "\"/>" << endl;
		if ( inter_proph_->useNSIModel()) 
		  fout << "<parameter name=\"nsi\"" << " value=\"" << inter_proph_->getNSIValue(k, exp_lbl, spectrum_name, swath_assay) << "\"/>" << endl;
		//		fout << "<parameter name=\"nsi_adj_prob\"" << " value=\"" << inter_proph_->getNSIAdjProb(k, exp_lbl, spectrum_name) << "\"/>" << endl;
		if ( inter_proph_->useNSMModel()) 
		  fout << "<parameter name=\"nsm\"" << " value=\"" << inter_proph_->getNSMValue(k, exp_lbl, spectrum_name, swath_assay) << "\"/>" << endl;

		if ( inter_proph_->useNSPModel()) 
		  fout << "<parameter name=\"nsp\"" << " value=\"" << inter_proph_->getNSPValue(k, exp_lbl, spectrum_name, swath_assay) << "\"/>" << endl;

		if ( inter_proph_->useFPKMModel()) 
		  fout << "<parameter name=\"fpkm\"" << " value=\"" << inter_proph_->getFPKMValue(k, exp_lbl, spectrum_name, swath_assay) << "\"/>" << endl;

		if ( inter_proph_->useCatModel()) 
		  fout << "<parameter name=\"top_cat\"" << " value=\"" << inter_proph_->getCatValue(k, exp_lbl, spectrum_name, swath_assay) << "\"/>" << endl;
		//		fout << "<parameter name=\"nsm_adj_prob\"" << " value=\"" << inter_proph_->getNSMAdjProb(k, exp_lbl, spectrum_name) << "\"/>" << endl;
		//TODO Add more search scores here
		fout << "</search_score_summary>" << endl;
		fout << "</interprophet_result>" << endl;
		fout << "</analysis_result>" << endl;
	      }
	    }
	  }
	  delete tag;
	  data = strstr(data+1, "<");      
	}
      }
      fin.close();
    }
  }
  fout << "</msms_pipeline_analysis>" << endl;
  fout.close();
  delete [] nextline;
}

void InterProphetParser::writePepXMLFast() {
  writePepXMLFast(-100, NULL);
}

void InterProphetParser::writePepXMLFast(double minProb, string* opts) {
  Array<Tag*>* tags = NULL;
  
  Tag* tag = NULL;

  char *nextline = new char[line_width_];
  char* data = NULL;


  string spectrum_name = "";
  string exp_lbl = "";
  double prob = 0;
  double calcnmass = -1;
  string pep_seq = "";
  string mod_pep = "";
  string swath_assay = "";

  bool get_pep = false;
  bool pepproph = false;
  int k=0;
  int_hash::iterator itr = ms_runs_->begin();
  ogzstream* fout = new ogzstream(outfile_->c_str());  // looks at filename, does gzip compression if ends in .gz
  //ofstream* fout = new ofstream(outfile_->c_str());  // looks at filename, does gzip compression if ends in .gz
  // TODO  double allntt_prob[3] = {-100, -100, -100};
  
  (*fout) << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;

  //TODO add stylesheet and dynamic schema references
  (*fout) << "<msms_pipeline_analysis date=\"" << time_
       << "\" xmlns=\"http://regis-web.systemsbiology.net/pepXML\" "
       << "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
       << "xsi:schemaLocation=\"http://regis-web.systemsbiology.net/pepXML "
       << "/tools/bin/TPP/tpp/schema/pepXML_v18.xsd\" summary_xml=\"" 
       << outfile_->c_str() << "\">" << endl;

  if (!opts) {
    opts = new string("");
  }
  
  (*fout) << "<analysis_summary analysis=\"interprophet\" time=\"" << time_ << "\">" << endl
	  << "<interprophet_summary version=\"" << szTPPVersionInfo  << "\""
	  << " options=\"" << *opts << "\"  est_tot_num_correct_psm=\"" << inter_proph_->getEstNumCorrectPSM() <<  "\"  est_tot_num_correct_pep=\"" << inter_proph_->getEstNumCorrectPep() << "\">" << endl;

  for(k = 0; k < input_files_->length(); k++) {
    (*fout) << "<inputfile name=\"" << (*input_files_)[k]->c_str() << "\"/>" << endl;
  }

  Array<Tag*>* roc_tags = inter_proph_->getRocDataPointTags();
  
  for (k = 0; k < roc_tags->length(); k++) {
    (*roc_tags)[k]->write((*fout));
  }
  (*fout) << endl;
  inter_proph_->reportModels((*fout));

  (*fout) << "</interprophet_summary>" << endl;
  (*fout) << "</analysis_summary>" << endl;


  for (int a=0; a<anal_summs_->size(); a++) {
    (*anal_summs_)[a]->write(*fout);
    
  }


  bool output = false;
  //  bool done = false;
  bool found = false;
  
  bool ipro_filter = false;

  unsigned long index = 1;
  
  bool open_summ = false;
  
  char text[500];
  
  //for (itr = ms_runs_->begin(); itr!= ms_runs_->end(); itr++) {
  //  const string* ms_run = &itr->first;

    for(k = 0; k < input_files_->length(); k++) {
      output = false;
      //done = false;
      found = false;
      pwiz::util::random_access_compressed_ifstream fin((*input_files_)[k]->c_str());
      if(! fin) {
	cerr << "fin: error opening " << (*input_files_)[k]->c_str() << endl;
	exit(1);
      }
      while(fin.getline(nextline, line_width_)) {
	//if (done) {
	//  break;
	//}
	
	data = strstr(nextline, "<");
	
	while(data != NULL) {
	  tag = new Tag(data);
	  
	  if (tag != NULL) {
	    if (tag->isStart() && 
		! strcmp(tag->getName(), "analysis_result") && 
		tag->getAttributeValue("analysis") != NULL &&
		! strcmp(tag->getAttributeValue("analysis"), "interprophet")) {
	      //string* tmp = new string(tag->getAttributeValue("base_name"));
	      //	      if (*ms_run == *tmp) {
	      ipro_filter = true;
		//}
		//delete tmp;
	    }
	    else if (tag->isStart() && 
		     ! strcmp(tag->getName(), "msms_run_summary")) {
	      //string* tmp = new string(tag->getAttributeValue("base_name"));
	      //	      if (*ms_run == *tmp) {
	      found = true;
	      output = true;
	      //tag->write((*fout));
		//}
		//delete tmp;
	    }
	    else if (!strcmp(tag->getName(), "analysis_summary")) {

	      // pass through any analysis summary that isn't interprophet or peptideprophet
	      
	      if (tag->isStart() && !tag->isEnd() && 
		  strcmp(tag->getAttributeValue("analysis"),"peptideprophet") &&
		  strcmp(tag->getAttributeValue("analysis"),"interprophet") ) {
		
		//tag->write((*fout)); // and copy the current	line
		output = true;
		open_summ = true;
	      }
	      else if (tag->isStart() && tag->isEnd() &&
		       strcmp(tag->getAttributeValue("analysis"),"peptideprophet") &&
		       strcmp(tag->getAttributeValue("analysis"),"interprophet") ) {
		//tag->write((*fout)); // and copy the current	line
		output = false;
		tag->write((*fout));	     
	      }
	      else if (tag->isEnd() && open_summ) {
		output = false; // stop copying
		tag->write((*fout));
		open_summ = false;
	      }
	    }
	    else if (tag->isEnd() && !strcmp(tag->getName(), "search_summary")) {
	      if (found) {
		output = false;
		tag->write((*fout));
	      }
	      Tag* ts_tag = new Tag("analysis_timestamp", True, True);
	      ts_tag->setAttributeValue("analysis", "interprophet");
	      ts_tag->setAttributeValue("time",time_);
	      ts_tag->setAttributeValue("id", "1");
	      ts_tag->write(*fout);
	      delete ts_tag;
	    }
	    else if (found && tag->isStart() && tag->isEnd() && !strcmp(tag->getName(), "analysis_timestamp")) {
	      output = false; 
	      tag->write((*fout));
	    }
	    else if (found && tag->isStart() && !strcmp(tag->getName(), "analysis_timestamp")) {
	      output = true;
	      //tag->write((*fout));
	    }
	    else if (found && tag->isEnd() && !strcmp(tag->getName(), "analysis_timestamp")) {
	      output = false;
	      tag->write((*fout));
	    }
	    else if (found && tag->isEnd() && 
		     ! strcmp(tag->getName(), "msms_run_summary")) {
	      tag->write((*fout));
	      //done = true;
	    }
	    
	    else if (found && tag->isStart() && !tag->isEnd() && 
		! strcmp(tag->getName(), "spectrum_query")) {
	      get_pep = false;
	      pep_seq = ""; mod_pep = "";swath_assay = "";
	      // store the spectrum name for later
	      int len = strlen(tag->getAttributeValue("spectrum"));
	      spectrum_name = "";
	      spectrum_name += tag->getAttributeValue("spectrum");
	      
	      if (tag->getAttributeValue("experiment_label") != NULL) {
		len = strlen(tag->getAttributeValue("experiment_label"));
	      }
	      else {
		len = 0;
	      } 
	      exp_lbl = "";
	      if (len > 0)
		exp_lbl += tag->getAttributeValue("experiment_label");
	      
	      if (tag->getAttributeValue("swath_assay") != NULL) {
		swath_assay = tag->getAttributeValue("swath_assay");
		swath_assay = swath_assay.substr(0, swath_assay.find(":"));
	      }
	      if (inter_proph_->getBestMatch(exp_lbl, spectrum_name, swath_assay) == k && inter_proph_->getAdjProb(exp_lbl, spectrum_name, swath_assay) >= minProb) {
			output = true;
			sprintf(text, "%lu", index);
			tag->setAttributeValue("index", text);
			index++;
			tag->write((*fout));
	      }
	      else {
			output = false;
	      }
	      
	    }
	    else if (found && tag->isStart() && 
		     ! strcmp(tag->getName(), "search_hit") && 
		     atoi(tag->getAttributeValue("hit_rank")) == 1) {
	      pep_seq += tag->getAttributeValue("peptide");
	      calcnmass = atof(tag->getAttributeValue("calc_neutral_pep_mass"));
	      //get_pep = false;
	      
	    }
	    else if (found && tag->isStart() && get_pep &&
		     ! strcmp(tag->getName(), "modification_info")) {
	      
	      get_pep = false;
	      mod_pep = "";
	      mod_pep += tag->getAttributeValue("modified_peptide");
	      
	    }
	    else if (found && tag->isStart() && 
		     ! strcmp(tag->getName(), "peptideprophet_result")) {
	      
	      // got the spectrum name and probability
	      prob = atof( tag->getAttributeValue("probability") );
	      //inter_proph_->insertResult(k, spectrum_name, prob, pep_seq, calcnmass);
	      
	      
	    }
	    else if (found && tag->isEnd() && 
		     ! strcmp(tag->getName(), "spectrum_query")) {
	      if (output)
		tag->write((*fout));
	      output = false;
	      spectrum_name = "";
	      exp_lbl = "";
	      get_pep = false;
	      pep_seq = "";
	      swath_assay = "";
	      prob = 0;
	      // TODO  allntt_prob[0] = -100; allntt_prob[1] = -100; allntt_prob[2] = -100;
	    }
	    else if (found && tag->isStart() && ! strcmp(tag->getName(), "analysis_result") &&! strcmp(tag->getAttributeValue("analysis"), "peptideprophet")) {
	      pepproph = true;
	    }

	    if (!ipro_filter && output && strcmp(tag->getName(), "spectrum_query")) {
	      (*fout) << nextline << endl;
	    }
	    
	    if (found && pepproph && tag->isEnd() && ! strcmp(tag->getName(), "analysis_result")) {
	      pepproph = false;
	      if (output) {
		(*fout) << "<analysis_result analysis=\"interprophet\">" << endl;
		//TODO Add all ntt probs here
		(*fout) << "<interprophet_result probability=\"" << inter_proph_->getAdjProb(exp_lbl, spectrum_name, swath_assay) 
		     << "\" all_ntt_prob=\"("  
		     << inter_proph_->getNTTAdjProb(exp_lbl, spectrum_name, swath_assay, 0) << ","
		     << inter_proph_->getNTTAdjProb(exp_lbl, spectrum_name, swath_assay, 1) << ","
		     << inter_proph_->getNTTAdjProb(exp_lbl, spectrum_name, swath_assay, 2) << ")\">" << endl; 
		(*fout) << "<search_score_summary>" << endl;
		if ( inter_proph_->useNSSModel()) 
		  (*fout) << "<parameter name=\"nss\"" << " value=\"" << inter_proph_->getNSSValue(k, exp_lbl, spectrum_name, swath_assay) << "\"/>" << endl;
		//		(*fout) << "<parameter name=\"nss_adj_prob\"" << " value=\"" << inter_proph_->getNSSAdjProb(k, exp_lbl, spectrum_name) << "\"/>" << endl;
		if ( inter_proph_->useNRSModel()) 
		  (*fout) << "<parameter name=\"nrs\"" << " value=\"" << inter_proph_->getNRSValue(k, exp_lbl, spectrum_name, swath_assay) << "\"/>" << endl;
		//		(*fout) << "<parameter name=\"nrs_adj_prob\"" << " value=\"" << inter_proph_->getNRSAdjProb(k, exp_lbl, spectrum_name) << "\"/>" << endl;
		if ( inter_proph_->useNSEModel()) 
		  (*fout) << "<parameter name=\"nse\"" << " value=\"" << inter_proph_->getNSEValue(k, exp_lbl, spectrum_name, swath_assay) << "\"/>" << endl;
		//		(*fout) << "<parameter name=\"nse_adj_prob\"" << " value=\"" << inter_proph_->getNSEAdjProb(k, exp_lbl, spectrum_name) << "\"/>" << endl;
		if ( inter_proph_->useNSIModel()) 
		  (*fout) << "<parameter name=\"nsi\"" << " value=\"" << inter_proph_->getNSIValue(k, exp_lbl, spectrum_name, swath_assay) << "\"/>" << endl;
		//		(*fout) << "<parameter name=\"nsi_adj_prob\"" << " value=\"" << inter_proph_->getNSIAdjProb(k, exp_lbl, spectrum_name) << "\"/>" << endl;
		if ( inter_proph_->useNSMModel()) 
		  (*fout) << "<parameter name=\"nsm\"" << " value=\"" << inter_proph_->getNSMValue(k, exp_lbl, spectrum_name, swath_assay) << "\"/>" << endl;
		if ( inter_proph_->useNSPModel()) 
		  (*fout) << "<parameter name=\"nsp\"" << " value=\"" << inter_proph_->getNSPValue(k, exp_lbl, spectrum_name, swath_assay) << "\"/>" << endl;

		if ( inter_proph_->useFPKMModel()) 
		  (*fout) << "<parameter name=\"fpkm\"" << " value=\"" << inter_proph_->getFPKMValue(k, exp_lbl, spectrum_name, swath_assay) << "\"/>" << endl;

		if ( inter_proph_->useCatModel()) 
		  (*fout) << "<parameter name=\"top_cat\"" << " value=\"" << inter_proph_->getCatValue(k, exp_lbl, spectrum_name, swath_assay) << "\"/>" << endl;
		//		(*fout) << "<parameter name=\"nsm_adj_prob\"" << " value=\"" << inter_proph_->getNSMAdjProb(k, exp_lbl, spectrum_name) << "\"/>" << endl;
		//TODO Add more search scores here
		(*fout) << "</search_score_summary>" << endl;
		(*fout) << "</interprophet_result>" << endl;
		(*fout) << "</analysis_result>" << endl;
	      }
	    }
	    


	    if (ipro_filter && tag->isEnd() && ! strcmp(tag->getName(), "analysis_result")) {
	      ipro_filter = false;
	    }

	  }
	  delete tag;
	  data = strstr(data+1, "<");      
	}
      }
      fin.close();
    }

    (*fout) << "</msms_pipeline_analysis>" << endl;
    (*fout).close();
    delete fout;
    delete [] nextline;
}



