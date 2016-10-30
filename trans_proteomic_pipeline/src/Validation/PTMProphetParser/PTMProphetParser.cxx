#include "PTMProphetParser.h"

PTMProphetParser::PTMProphetParser(string& modstring, double tol) : Parser("ptmprophet") {
  em_ = false;
  update_mod_tags_ = false;
  mpx_ = false;

  
  string aa = "";
  double shift = 0.;
  
  int start = 0;
  int comma = modstring.find(",", start);
  
  while (comma != string::npos) {
    aa = modstring.substr(start, comma-start);

    start = comma + 1;
    comma = modstring.find(",", start);

    if (comma != string::npos) {
      shift = atof(modstring.substr(start, comma-start).c_str());
      start = comma + 1;
      aminoacids_.push_back(aa);
      massshift_.push_back(shift);
      comma = modstring.find(",", start);    
      
    }
    else {
      shift = atof(modstring.substr(start).c_str()); 
      aminoacids_.push_back(aa);
      massshift_.push_back(shift);
      break;
            
    }

  }
  modstring = "PTMProphet_"+modstring;
  ptm_model_ = new KDModel(modstring.c_str());  
  mzTol_ = tol;
  out_file_ = "";
}
 
void PTMProphetParser::setOutFile(string name) {
  out_file_ = name;
}

void PTMProphetParser::setEM(bool em) { 
  em_ = em; 
  if (mpx_) em_ = false; //set to false if mpx_
}

void PTMProphetParser::setMpx(bool mpx){ 
  mpx_ = mpx; 
  if (mpx_) em_ = false; //set to false if mpx_
}

void PTMProphetParser::setUpdate(bool up) { 
  update_mod_tags_ = up; 
}

void PTMProphetParser::run(const char* c, const char* opts) {
  opts_ = string(opts);
  init(c);
 }

void PTMProphetParser::parse(const char* c) {
  if (!mpx_) {
    if (em_) {
      parseRead(c);
      computePTMModel();
    }
    if (update_mod_tags_) {
      parseWriteUpdate(c);
    }
    else {
      parseWrite(c);
    }
  }
  else {
    if (em_) {
      parseReadMpx(c);
      computePTMModel(); //TODO: might need to update this function
    }
    if (update_mod_tags_) {
      parseWriteUpdateMpx(c);
    }
    else {
      parseWriteMpx(c);
    }


  }

}

void PTMProphetParser::parseRead(const char* c) {
  Array<Tag*>* tags = NULL;
  Tag* tag = NULL;

  char *nextline = new char[line_width_];
  char* data = NULL;

  bool evalSites = true;
  string spectrum_name = "";
  string exp_lbl = "";
  string charge = "";
  double prob = 0;
  Array<double>* allntt_prob=NULL ;
  double calcnmass = -1;
  double rt = -1;
  string pep_seq = "";
  string mod_pep = "";
 
  string mod_tags = "";

  bool in_mod= false;
  
  bool get_pep = false;
  int k=0;

  long scan = -1;
  vector<string> mtvec;
  SpectraSTLib* mtlib= NULL;
  //  SpectraSTCreateParams* mtparams=NULL;
  // SpectraSTPepXMLLibImporter*  specLib = new SpectraSTPepXMLLibImporter(mtvec, mtlib, *mtparams);
  SpectraSTLibEntry* entry = NULL;
  Peptide* specPep = NULL;
  PTMProphet* prophet = NULL ;
  cRamp* cramp = NULL;

  string dataFile = "";
  string dataExt = "";
  
  //  tmp_file_ = make_tmpfile_name(c);
  //  ofstream fout(tmp_outfile_.c_str());  
  //  if(! fout) {
  //    cerr << "cannot write output to file " << tmp_file_ << endl;
  //    exit(1);
  //  }

  // TODO  double allntt_prob[3] = {-100, -100, -100};
  // for(k = 0; k < input_files_->length(); k++) {
    pwiz::util::random_access_compressed_ifstream fin(c); // read possibly gzipped files
    if(! fin) {
      cerr << "fin: error opening " << c << endl;
      exit(1);
    }

    while(fin.getline(nextline, line_width_)) {
      data = strstr(nextline, "<");
      while(data != NULL) {
	tag = new Tag(data);
	//tag->write(fout);
	if (in_mod) {
	  mod_tags += data;
	}
	if (tag != NULL) {
	  
	  if (tag->isStart() && 
	      ! strcmp(tag->getName(), "msms_pipeline_analysis")) {
	    //(*input_files_)[k]->assign(tag->getAttributeValue("summary_xml"));
	    //inter_proph_->addSearch((*input_files_)[k]);
	    //	    fout << "<analysis_summary analysis=\"ptmprophet\" time=\"" << time_ << "\"/>" << endl;
	  }

	  else if (tag->isStart() && 
	      ! strcmp(tag->getName(), "msms_run_summary")) {
	    //(*input_files_)[k]->assign(tag->getAttributeValue("summary_xml"));
	    //inter_proph_->addSearch((*input_files_)[k]);
	    dataFile = tag->getAttributeValue("base_name");
	    dataExt = tag->getAttributeValue("raw_data");
	    if (dataExt[0] != '.') {
	      dataFile += ".";
	    }
	    dataFile += dataExt;
	    
	    if (cramp != NULL)
	      delete cramp;

	    cramp = new cRamp(dataFile.c_str());
	    if (!cramp->OK()) {
	      cerr << "ERROR: cannot read scan in data file " << dataFile << " exiting ..." << endl;
	      exit(1);
	    }
	    //stat_mods_hash_.clear();
	    //stat_prot_termods_hash_.clear();
	  }

	  else if (tag->isStart() && 
		   ! strcmp(tag->getName(), "spectrum_query")) {
	    //	    get_pep = true;
	    pep_seq = "";
	    mod_pep = "";
	    spectrum_name = "";
	    rt = -1;
	    if (tag->getAttributeValue("retention_time_sec") != NULL) 
	      rt = atof(tag->getAttributeValue("retention_time_sec"));

	    // store the spectrum name for later
	    int len = strlen(tag->getAttributeValue("spectrum"));
	    
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
	    //char* tmp = strrchr(spectrum_name, '.');
	    //tmp = '\0';
	    charge = tag->getAttributeValue("assumed_charge");
	    scan = atoi(tag->getAttributeValue("start_scan"));
	    
	  }
	  else if (tag->isStart() && 
		   ! strcmp(tag->getName(), "search_hit") && 
		   atoi(tag->getAttributeValue("hit_rank")) == 1) {
	    pep_seq = "";
	    pep_seq += tag->getAttributeValue("peptide");
	    calcnmass = atof(tag->getAttributeValue("calc_neutral_pep_mass"));
	    //get_pep = false;
	    
	  }
	  else if ( tag->isStart() && ! strcmp(tag->getName(), "peptideprophet_result" )  ) {
	    double pepprob = atof(tag->getAttributeValue("probability"));
	    if (pepprob >= 0.9 && prophet) {
	      string probstr = prophet->getPepProbString();
	      if (!probstr.empty()) {
		//  fout << "<analysis_result analysis=\"ptmprophet\">" << endl;
		//fout << "<ptmprophet_result ptm=\"Phospho\" ptm_peptide=\"" << probstr << "\">" << endl;
		
		for (int idx = 0; idx < pep_seq.length(); idx++) {
		  //fout.precision(3);
		  //fout.width(5);
		  //fout.setf(ios::fixed);
		  double prob = prophet->getProbAtPosition(idx);
		  
		  if (prob >= 0) {
		    ptm_model_->insert(prob, prob);
		    priors_.push_back(prophet->getModPrior());
		    //priors_.push_back(0.5);
		    //   fout << "<mod_aminoacid_probability position=\"" << idx+1  << "\" probability=\"" << prob << "\"/>" << endl;
		  }
		}
		
		//fout << "</ptmprophet_result>\n</analysis_result>" << endl;
	      }
	      
	      //	    prophet->printPTM();
	    }


	  }
	  else if (tag->isEnd() && //get_pep &&
		   ! strcmp(tag->getName(), "modification_info")) {   

	    //DDS: TODO IT HERE!!!!
	    // specLib->SpectraSTPepXMLLibImporter::addModificationsToPeptide(pep_seq, mod_tags);
	    if (prophet) {
	      delete prophet;
	      prophet = NULL;
	    }
	    prophet = new PTMProphet(mod_pep, atoi(charge.c_str()), cramp, scan, aminoacids_[0], massshift_[0]);
	    if (prophet && !prophet->evaluateModSites(mzTol_)) {
	      delete prophet;
	      prophet = NULL;
	    }
	    else {
	      mod_tags = "";
	      in_mod = false;
	    }
	  }
	  else if (tag->isStart() && //get_pep &&
		   ! strcmp(tag->getName(), "modification_info")) {      
	    in_mod = true;
       	    mod_tags = data; 
	    //	    get_pep = false;
	    mod_pep = "";
	    mod_pep += tag->getAttributeValue("modified_peptide");
	  }


	  else if (tag->isEnd() && 
		   ! strcmp(tag->getName(), "spectrum_query")) {
	    exp_lbl = "";
	    spectrum_name = "";
	    charge = "";
	    //	    get_pep = false;
	    pep_seq = "";
	    mod_pep = "";
	    prob = 0;
	    scan = -1;
	    if (prophet) {
	      delete prophet;
	      prophet = NULL;
	    }
	     // TODO  allntt_prob[0] = -100; allntt_prob[1] = -100; allntt_prob[2] = -100;
	  }
		 	
	}
	delete tag;
	data = strstr(data+1, "<");      
      }
    }
    fin.close();
    //    fout.close();

    //unlink(c); 
    //rename(tmp_file_.c_str(), c);
    
    
  
    delete [] nextline;
    //    inter_proph_->computeModels();
}



void PTMProphetParser::parseReadMpx(const char* c) {
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
 
  string mod_tags = "";

  bool in_mod= false;
  

  bool is_nterm_pep= false;
  bool is_cterm_pep= false;
   
  bool get_pep = false;
  int k=0;

  long scan = -1;
  vector<string> mtvec;
  SpectraSTLib* mtlib= NULL;
  //  SpectraSTCreateParams* mtparams=NULL;
  //SpectraSTPepXMLLibImporter*  specLib = new SpectraSTPepXMLLibImporter(mtvec, mtlib, *mtparams);
  SpectraSTLibEntry* entry = NULL;
  Peptide* specPep = NULL;
  PTMProphetMpx* prophet = NULL ;
  cRamp* cramp = NULL;

  string dataFile = "";
  string dataExt = "";
  
  //  tmp_file_ = make_tmpfile_name(c);
  //  ofstream fout(tmp_outfile_.c_str());  
  //  if(! fout) {
  //    cerr << "cannot write output to file " << tmp_file_ << endl;
  //    exit(1);
  //  }

  // TODO  double allntt_prob[3] = {-100, -100, -100};
  // for(k = 0; k < input_files_->length(); k++) {
    pwiz::util::random_access_compressed_ifstream fin(c); // read possibly gzipped files
    if(! fin) {
      cerr << "fin: error opening " << c << endl;
      exit(1);
    }
    cerr << "INFO: Reading file " << c << " ..." << endl;
    
    while(fin.getline(nextline, line_width_)) {
      data = strstr(nextline, "<");
      while(data != NULL) {
	tag = new Tag(data);
	//tag->write(fout);
	if (in_mod) {
	  mod_tags += data;
	}
	if (tag != NULL) {
	  
	  if (tag->isStart() && 
	      ! strcmp(tag->getName(), "msms_pipeline_analysis")) {
	    //(*input_files_)[k]->assign(tag->getAttributeValue("summary_xml"));
	    //inter_proph_->addSearch((*input_files_)[k]);
	    //	    fout << "<analysis_summary analysis=\"ptmprophet\" time=\"" << time_ << "\"/>" << endl;
	  }

	  else if (tag->isStart() && 
	      ! strcmp(tag->getName(), "msms_run_summary")) {
	    //(*input_files_)[k]->assign(tag->getAttributeValue("summary_xml"));
	    //inter_proph_->addSearch((*input_files_)[k]);
	    dataFile = tag->getAttributeValue("base_name");
	    dataExt = tag->getAttributeValue("raw_data");
	    if (dataExt[0] != '.') {
	      dataFile += ".";
	    }
	    dataFile += dataExt;
	   
	    if (cramp != NULL)
	      delete cramp;

	    cramp = new cRamp(dataFile.c_str());
	    if (!cramp->OK()) {
	      cerr << "ERROR: cannot read scan in data file " << dataFile << " exiting ..." << endl;
	      exit(1);
	    }
	    //stat_mods_hash_.clear();	    
	    //stat_prot_termods_hash_.clear();
	  }

	  else if (tag->isStart() && 
		   ! strcmp(tag->getName(), "spectrum_query")) {
	    //	    get_pep = true;
	    pep_seq = "";
	    mod_pep = "";
	    spectrum_name = "";
	    rt = -1;
	    if (tag->getAttributeValue("retention_time_sec") != NULL) 
	      rt = atof(tag->getAttributeValue("retention_time_sec"));

	    // store the spectrum name for later
	    int len = strlen(tag->getAttributeValue("spectrum"));
	    
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
	    //char* tmp = strrchr(spectrum_name, '.');
	    //tmp = '\0';
	    charge = tag->getAttributeValue("assumed_charge");
	    scan = atoi(tag->getAttributeValue("start_scan"));
	    
	  }
	  else if (tag->isStart() && 
		   ! strcmp(tag->getName(), "search_hit") && 
		   atoi(tag->getAttributeValue("hit_rank")) == 1) {
	    pep_seq = "";
	    pep_seq += tag->getAttributeValue("peptide");
	    calcnmass = atof(tag->getAttributeValue("calc_neutral_pep_mass"));
	    //get_pep = false;

	    is_nterm_pep = false;
	    is_cterm_pep = false;

	    if (! strcmp(tag->getAttributeValue("peptide_prev_aa") , "-")) {
	      is_nterm_pep = true;
	    }
	    if (! strcmp(tag->getAttributeValue("peptide_next_aa") , "-")) {
	      is_cterm_pep = true;
	    }
	    
	  }
	  else if ( tag->isStart() && ! strcmp(tag->getName(), "peptideprophet_result" )  ) {
	    double pepprob = atof(tag->getAttributeValue("probability"));
	    if (pepprob >= 0.9 && prophet) {
	      for (int type = 0 ; type < aminoacids_.size(); type++) {
		string probstr = prophet->getPepProbString(type);
		if (!probstr.empty()) {
		  //  fout << "<analysis_result analysis=\"ptmprophet\">" << endl;
		  //fout << "<ptmprophet_result ptm=\"Phospho\" ptm_peptide=\"" << probstr << "\">" << endl;
		  
		  for (int idx = 0; idx < pep_seq.length(); idx++) {
		    //fout.precision(3);
		    //fout.width(5);
		    //fout.setf(ios::fixed);
		    double prob = prophet->getProbAtPosition(type, idx);
		    
		    if (prob >= 0) {
		      
		      ptm_model_->insert(prob, prob);
		      priors_.push_back(prophet->getModPrior(type));
		      //priors_.push_back(0.5);
		      //   fout << "<mod_aminoacid_probability position=\"" << idx+1  << "\" probability=\"" << prob << "\"/>" << endl;
		    }
		  }
		}
		
		//fout << "</ptmprophet_result>\n</analysis_result>" << endl;
	      }
	      
	      //	    prophet->printPTM();
	    }


	  }
	  else if (tag->isEnd() && //get_pep &&
		   ! strcmp(tag->getName(), "modification_info")) {   

	    //DDS: TODO IT HERE!!!!
	    // specLib->SpectraSTPepXMLLibImporter::addModificationsToPeptide(pep_seq, mod_tags);
	    if (prophet) {
	      delete prophet;
	      prophet = NULL;
	    }
	    if (tag->isStart()) {
	      mod_pep = "";
	      mod_pep += tag->getAttributeValue("modified_peptide");
	    }
	    prophet = new PTMProphetMpx(mod_pep, atoi(charge.c_str()), calcnmass, cramp, scan, aminoacids_, massshift_, &stat_mods_hash_, &stat_prot_termods_hash_, is_nterm_pep, is_cterm_pep);
	    if (prophet->init()) {
	      prophet->setTolerance(mzTol_);
	      prophet->combinations();
	    }
	    else {
	      if (mod_pep != "") {
		cerr << "WARNING: Cannot initialize for sequence: " << mod_pep.c_str() 
		     << ", unknown mods may exist in spectrum " << spectrum_name.c_str() << endl;
	      }
	      delete prophet;
	      prophet = NULL;
	    }
	    //	    prophet->evaluateModSites(mzTol_);

	    mod_tags = "";
	    in_mod = false;
	  }
	  else if (tag->isStart() && !tag->isEnd() && //get_pep &&
		   ! strcmp(tag->getName(), "modification_info")) {      
	    in_mod = true;
       	    mod_tags = data; 
	    //	    get_pep = false;
	    mod_pep = "";
	    mod_pep += tag->getAttributeValue("modified_peptide");
	  }


	  else if (tag->isEnd() && 
		   ! strcmp(tag->getName(), "spectrum_query")) {
	    exp_lbl = "";
	    spectrum_name = "";
	    charge = "";
	    //	    get_pep = false;
	    pep_seq = "";
	    mod_pep = "";
	    prob = 0;
	    scan = -1;
	    if (prophet) {
	      delete prophet;
	      prophet = NULL;
	    }
	     // TODO  allntt_prob[0] = -100; allntt_prob[1] = -100; allntt_prob[2] = -100;
	  }
		 	
	}
	delete tag;
	data = strstr(data+1, "<");      
      }
    }
    fin.close();
    //    fout.close();

    //unlink(c); 
    //rename(tmp_file_.c_str(), c);
    
    
  
    delete [] nextline;
    //    inter_proph_->computeModels();
}

void PTMProphetParser::computePTMModel() {
  ptm_model_->makeReady(true, 5);
  
  double probsum = 0;
  double prob = 0;
  double val = 0;
  double lastprobsum = 0;
  
  double tpos = 0, tneg = 0;
  long i;
  bool ret = false;
  
  double posprior = 0.5;

  int iter = 1;
  double negprior = 1 - posprior;
  
  Array<double>* probarr = new Array<double>(ptm_model_->getValSize());
  Array<double>* valarr = new Array<double>(ptm_model_->getValSize());
  cerr << "Iterating PTM Model: ";
  while (iter < 100) {
   
    if (iter % 5 == 0) {
      cerr << iter;
    }
    else {
       cerr << ".";
    }
    cerr.flush();
    probsum = 0;
    if (ptm_model_->getValSize() < 1) {
      ret = true;
    }
    for (i =0; i<ptm_model_->getValSize(); i++) {
      val = ptm_model_->getValAtIndex(i);
      

      if (iter == 1) {
	posprior = priors_[i]; 
      }
      negprior = 1 - posprior;
      prob = posprior*ptm_model_->getPosProb(val) / 
	(posprior*ptm_model_->getPosProb(val) + negprior*ptm_model_->getNegProb(val));
      if (fabs(prob-(*probarr)[i])<0.001) {
	if (i == 0) {
	  ret = true;
	}
      }
      else {
	ret = false;
      }
      (*probarr)[i] = prob;

      (*valarr)[i] = val;
      probsum += prob;
    }
    posprior = probsum/ptm_model_->getValSize();negprior = 1 - posprior;
    

    if (ret) {
      cerr << "done" << endl;
      delete probarr;
      delete valarr;
      return;
    }
    else {
      ptm_model_->clear();
      ptm_model_->clearObs();
      for (i =0; i<probarr->size(); i++) {
	ptm_model_->insert((*probarr)[i], (*valarr)[i]);
      }
      ptm_model_->makeReady(true, 5);
      iter++;
    }
    lastprobsum = probsum;
    
  }
  
  delete probarr;
  delete valarr;

}

void PTMProphetParser::parseWrite(const char* c) {
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
 
  string mod_tags = "";


  bool in_mod= false;
  
  bool get_pep = false;
  int k=0;

  long scan = -1;
  vector<string> mtvec;
  SpectraSTLib* mtlib= NULL;
  //  SpectraSTCreateParams* mtparams=NULL;
  //  SpectraSTPepXMLLibImporter*  specLib = new SpectraSTPepXMLLibImporter(mtvec, mtlib, *mtparams);
  SpectraSTLibEntry* entry = NULL;
  Peptide* specPep = NULL;
  
  cRamp* cramp = NULL;

  string dataFile = "";
  string dataExt = "";

  bool is_nterm_pep = false;
  
  bool is_cterm_pep = false;

  if (out_file_.empty()) {
    tmp_file_ = make_tmpfile_name(c);
  }
  else {
    tmp_file_ = out_file_;
  }
  ofstream fout(tmp_file_.c_str());  
  if(! fout) {
    cerr << "cannot write output to file " << tmp_file_ << endl;
    exit(1);
  }

  // TODO  double allntt_prob[3] = {-100, -100, -100};
  // for(k = 0; k < input_files_->length(); k++) {
    pwiz::util::random_access_compressed_ifstream fin(c); // read possibly gzipped files
    if(! fin) {
      cerr << "fin: error opening " << c << endl;
      exit(1);
    }

    while(fin.getline(nextline, line_width_)) {
      data = strstr(nextline, "<");
      while(data != NULL) {
	tag = new Tag(data);
	tag->write(fout);
	if (in_mod) {
	  mod_tags += data;
	}
	if (tag != NULL) {
	  
	  if (tag->isStart() && 
	      ! strcmp(tag->getName(), "msms_pipeline_analysis")) {
	    //(*input_files_)[k]->assign(tag->getAttributeValue("summary_xml"));
	    //inter_proph_->addSearch((*input_files_)[k]);
	    fout << "<analysis_summary analysis=\"ptmprophet\" time=\"" << time_  << "\">" 
		 << "<ptmprophet_summary version=\"" << szTPPVersionInfo  << "\""
		 << " options=\"" << opts_.c_str() 
		 << "\">" << endl;

	    fout << "<inputfile name=\"" << c << "\"/>" << endl;

	    fout << "</ptmprophet_summary>" << endl;

	    fout << "</analysis_summary>" << endl;
	 
	  }
	  else if (tag->isStart() && 
	      ! strcmp(tag->getName(), "msms_run_summary")) {
	    //(*input_files_)[k]->assign(tag->getAttributeValue("summary_xml"));
	    //inter_proph_->addSearch((*input_files_)[k]);
	    dataFile = tag->getAttributeValue("base_name");
	    dataExt = tag->getAttributeValue("raw_data");
	    if (dataExt[0] != '.') {
	      dataFile += ".";
	    }
	    dataFile += dataExt;
	    
	    if (cramp != NULL)
	      delete cramp;

	    cramp = new cRamp(dataFile.c_str());
	    if (!cramp->OK()) {
	      cerr << "ERROR: cannot read scan in data file " << dataFile << " exiting ..." << endl;
	      exit(1);
	    }
	    //stat_mods_hash_.clear();
	    //stat_prot_termods_hash_.clear();
	  }

	  else if (tag->isStart() && 
		   ! strcmp(tag->getName(), "spectrum_query")) {
	    //	    get_pep = true;
	    pep_seq = "";
	    mod_pep = "";
	    spectrum_name = "";
	    rt = -1;
	    if (tag->getAttributeValue("retention_time_sec") != NULL) 
	      rt = atof(tag->getAttributeValue("retention_time_sec"));

	    // store the spectrum name for later
	    int len = strlen(tag->getAttributeValue("spectrum"));
	    
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
	    //char* tmp = strrchr(spectrum_name, '.');
	    //tmp = '\0';
	    charge = tag->getAttributeValue("assumed_charge");
	    scan = atoi(tag->getAttributeValue("start_scan"));
	    
	  }
	  else if (tag->isStart() && 
		   ! strcmp(tag->getName(), "search_hit") && 
		   atoi(tag->getAttributeValue("hit_rank")) == 1) {
	    pep_seq = "";
	    pep_seq += tag->getAttributeValue("peptide");
	    calcnmass = atof(tag->getAttributeValue("calc_neutral_pep_mass"));
	    
	  }
	  else if (tag->isEnd() && //get_pep &&
		   ! strcmp(tag->getName(), "modification_info")) {   

	    PTMProphet* prophet = new PTMProphet(mod_pep, atoi(charge.c_str()), cramp, scan, aminoacids_[0], massshift_[0]);
	    prophet->init();
	    prophet->evaluateModSites(mzTol_);
	    string probstr = prophet->getPepProbString();
	    if (!probstr.empty()) {
	      fout << "<analysis_result analysis=\"ptmprophet\">" << endl;
	      double prior = prophet->getModPrior();
	      double nmods = prophet->getNumMods();
	      double probsum = 0;
	      
	      for (int idx = 0; idx < pep_seq.length(); idx++) {
		double prob = prophet->getProbAtPosition(idx);
		
		if (em_) {
		  prob = prior*ptm_model_->getPosProb( prob) /
		    ( (1-prior)*ptm_model_->getNegProb( prob) + prior*ptm_model_->getPosProb( prob)); 
		}
		if (prob >= 0) {
		  probsum += prob;
		}
	      }
	      
	      fout << "<ptmprophet_result prior=\"" << prior << "\"" << " ptm=\"PTMProphet_" << aminoacids_[0] << massshift_[0] << "\" ptm_peptide=\"";
		for (int idx = 0; idx < pep_seq.length(); idx++) {
		  double prob = prophet->getProbAtPosition(idx);
		  if (em_) {
		    prob = prior*ptm_model_->getPosProb( prob) /
		      ( (1-prior)*ptm_model_->getNegProb( prob) + prior*ptm_model_->getPosProb( prob)); 
		  
		    if (prob >= 0 && probsum > 0) {
		      prob = nmods*prob/probsum;
		      if (prob > 1) {
			prob = 1;
		      }
		    }
		  }
		  fout.width(1);
		  fout << pep_seq.substr(idx,1).c_str();
		  if (prob >= 0) {
		    fout << "(";
		    fout.precision(3);
		    fout.width(5);
		    fout.setf(ios::fixed);
		    fout << prob;
		    fout.width(1);
		    fout << ")";
		  }
		}
		
		
		fout << "\">" << endl;
		
		for (int idx = 0; idx < pep_seq.length(); idx++) {
		  fout.precision(3);
		  fout.width(5);
		  fout.setf(ios::fixed);
		  //		double prob = prophet->getProbAtPosition(idx);
		  double prob = prophet->getProbAtPosition(idx);
	
		  if (em_) {
		    prob = prior*ptm_model_->getPosProb( prob) /
		      ( (1-prior)*ptm_model_->getNegProb( prob) + prior*ptm_model_->getPosProb( prob)); 

		  
		    if (prob >= 0 && probsum > 0) {
		      prob = nmods*prob/probsum;
		      if (prob > 1) {
			prob = 1;
		      }
		    }
		  }
		  
		  if (prob >= 0) {
		    fout << "<mod_aminoacid_probability position=\"" << idx+1  << "\" probability=\"" << prob << "\"/>" << endl;
		  }
		}
	      
	      fout << "</ptmprophet_result>\n</analysis_result>" << endl;
	    }
	    
	    //	    prophet->printPTM();
	    delete prophet;
	    mod_tags = "";
	    in_mod = false;
	  }
	  else if (tag->isStart() && !tag->isEnd() && //get_pep &&
		   ! strcmp(tag->getName(), "modification_info")) {      
	    in_mod = true;
       	    mod_tags = data; 
	    //	    get_pep = false;
	    mod_pep = "";
	    mod_pep += tag->getAttributeValue("modified_peptide");
	  }


	  else if (tag->isEnd() && 
		   ! strcmp(tag->getName(), "spectrum_query")) {
	    exp_lbl = "";
	    spectrum_name = "";
	    charge = "";
	    //	    get_pep = false;
	    pep_seq = "";
	    mod_pep = "";
	    prob = 0;
	    scan = -1;
	     // TODO  allntt_prob[0] = -100; allntt_prob[1] = -100; allntt_prob[2] = -100;
	  }
		 	
	}
	delete tag;
	data = strstr(data+1, "<");      
      }
    }
    fin.close();
    fout.close();

    if (out_file_.empty()) {
      unlink(c); 
      rename(tmp_file_.c_str(), c);
    }
    
  
    delete [] nextline;
    //    inter_proph_->computeModels();
}



void PTMProphetParser::parseWriteUpdate(const char* c) {
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
 
  // string mod_tags = "";
  Array<Tag*>* mod_tags = NULL;

  bool in_mod= false;
  
  bool get_pep = false;
  int k=0;

  long scan = -1;
  vector<string> mtvec;
  SpectraSTLib* mtlib= NULL;
  //SpectraSTCreateParams* mtparams=NULL;
  //SpectraSTPepXMLLibImporter*  specLib = new SpectraSTPepXMLLibImporter(mtvec, mtlib, *mtparams);
  SpectraSTLibEntry* entry = NULL;
  Peptide* specPep = NULL;
  
  cRamp* cramp = NULL;

  string dataFile = "";
  string dataExt = "";

  if (out_file_.empty()) {
    tmp_file_ = make_tmpfile_name(c);
  }
  else {
    tmp_file_ = out_file_;
  }

  ofstream fout(tmp_file_.c_str());  
  if(! fout) {
    cerr << "cannot write output to file " << tmp_file_ << endl;
    exit(1);
  }

  // TODO  double allntt_prob[3] = {-100, -100, -100};
  // for(k = 0; k < input_files_->length(); k++) {
    pwiz::util::random_access_compressed_ifstream fin(c); // read possibly gzipped files
    if(! fin) {
      cerr << "fin: error opening " << c << endl;
      exit(1);
    }

    while(fin.getline(nextline, line_width_)) {
      data = strstr(nextline, "<");
      while(data != NULL) {
	tag = new Tag(data);
	if (!in_mod && strcmp(tag->getName(), "modification_info")) {
	  tag->write(fout);
	}
 
	//	if (in_mod) {
	//	  mod_tags->insertAtEnd(tag);
	//	}

	if (tag != NULL) {
	  
	  if (tag->isStart() && 
	      ! strcmp(tag->getName(), "msms_pipeline_analysis")) {
	    //(*input_files_)[k]->assign(tag->getAttributeValue("summary_xml"));
	    //inter_proph_->addSearch((*input_files_)[k]);
	    fout << "<analysis_summary analysis=\"ptmprophet\" time=\"" << time_ << "\">" << endl
		 << "<ptmprophet_summary version=\"" << szTPPVersionInfo  << "\""
		 << " options=\"" << opts_.c_str() 
		 << "\">" << endl;

	    fout << "<inputfile name=\"" << c << "\"/>" << endl;

	    fout << "</ptmprophet_summary>" << endl;

	    fout << "</analysis_summary>" << endl;
	  }
	  else if (! strcmp(tag->getName(), "aminoacid_modification") && !strcmp(tag->getAttributeValue("variable"), "N")) {
	    if (stat_mods_hash_.find(*tag->getAttributeValue("aminoacid")) != stat_mods_hash_.end() 
		&& fabs(atof(tag->getAttributeValue("massdiff"))-stat_mods_hash_[*tag->getAttributeValue("aminoacid")])>0.001) {
	      cerr << "ERROR: multiple static mods with different masses found on aminoacid " << tag->getAttributeValue("aminoacid") << ", cannot be processed together with PTMProphet." << endl;
	      exit(1);
	    }
	    stat_mods_hash_[*tag->getAttributeValue("aminoacid")] = atof(tag->getAttributeValue("massdiff"));	    
	  }
	  else if (! strcmp(tag->getName(), "terminal_modification") && !strcmp(tag->getAttributeValue("variable"), "N") && !strcmp(tag->getAttributeValue("protein_terminus"), "N")) {
	    if (stat_mods_hash_.find(tolower(*tag->getAttributeValue("terminus"))) != stat_mods_hash_.end()
		&& fabs(atof(tag->getAttributeValue("massdiff"))-stat_mods_hash_[tolower(*tag->getAttributeValue("terminus"))])>0.001) {
	      cerr << "ERROR: multiple static mods with different masses found on terminus " << tag->getAttributeValue("terminus") << ", cannot be processed together with PTMProphet." << endl;
	      exit(1);
	    }
	    stat_mods_hash_[tolower(*tag->getAttributeValue("terminus"))] = atof(tag->getAttributeValue("massdiff"));
	  }
	  else if (! strcmp(tag->getName(), "terminal_modification") && !strcmp(tag->getAttributeValue("variable"), "N") && !strcmp(tag->getAttributeValue("protein_terminus"), "Y")) {
	    if (stat_prot_termods_hash_.find(tolower(*tag->getAttributeValue("terminus"))) != stat_prot_termods_hash_.end()
		&& fabs(atof(tag->getAttributeValue("massdiff"))-stat_prot_termods_hash_[tolower(*tag->getAttributeValue("terminus"))])>0.001) {
	      cerr << "ERROR: multiple static mods with different masses found on terminus " << tag->getAttributeValue("terminus") << ", cannot be processed together with PTMProphet." << endl;
	      exit(1);
	    }
	    stat_prot_termods_hash_[tolower(*tag->getAttributeValue("terminus"))] = atof(tag->getAttributeValue("massdiff"));
	  }
	  else if (tag->isStart() && 
	      ! strcmp(tag->getName(), "msms_run_summary")) {
	    //(*input_files_)[k]->assign(tag->getAttributeValue("summary_xml"));
	    //inter_proph_->addSearch((*input_files_)[k]);
	    dataFile = tag->getAttributeValue("base_name");
	    dataExt = tag->getAttributeValue("raw_data");
	    if (dataExt[0] != '.') {
	      dataFile += ".";
	    }
	    dataFile += dataExt;
	    
	    if (cramp != NULL)
	      delete cramp;

	    cramp = new cRamp(dataFile.c_str());
	    if (!cramp->OK()) {
	      cerr << "ERROR: cannot read scan in data file " << dataFile << " exiting ..." << endl;
	      exit(1);
	    }
	    //stat_mods_hash_.clear();
	    //stat_prot_termods_hash_.clear();
	  }

	  else if (tag->isStart() && 
		   ! strcmp(tag->getName(), "spectrum_query")) {
	    //	    get_pep = true;
	    pep_seq = "";
	    mod_pep = "";
	    spectrum_name = "";
	    rt = -1;
	    if (tag->getAttributeValue("retention_time_sec") != NULL) 
	      rt = atof(tag->getAttributeValue("retention_time_sec"));

	    // store the spectrum name for later
	    int len = strlen(tag->getAttributeValue("spectrum"));
	    
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
	    //char* tmp = strrchr(spectrum_name, '.');
	    //tmp = '\0';
	    charge = tag->getAttributeValue("assumed_charge");
	    scan = atoi(tag->getAttributeValue("start_scan"));
	    
	  }
	  else if (tag->isStart() && 
		   ! strcmp(tag->getName(), "search_hit") && 
		   atoi(tag->getAttributeValue("hit_rank")) == 1) {
	    pep_seq = "";
	    pep_seq += tag->getAttributeValue("peptide");
	    calcnmass = atof(tag->getAttributeValue("calc_neutral_pep_mass"));
	    
	  }
	  else if (tag->isEnd() && //get_pep &&
		   ! strcmp(tag->getName(), "modification_info")) {   

	    PTMProphet* prophet = new PTMProphet(mod_pep, atoi(charge.c_str()), cramp, scan, aminoacids_[0], massshift_[0]);
	    prophet->init();
	    prophet->evaluateModSites(mzTol_);
	    //  mod_tags->insertAtEnd(tag);
	    writeUpdatedModTags(prophet,fout, mod_pep);
	    //	    tag->write(fout)
	    string probstr = prophet->getPepProbString();
	    if (!probstr.empty()) {
	      fout << "<analysis_result analysis=\"ptmprophet\">" << endl;
	      double prior = prophet->getModPrior();
	      double nmods = prophet->getNumMods();
	      double probsum = 0;
	      
	      for (int idx = 0; idx < pep_seq.length(); idx++) {
		double prob = prophet->getProbAtPosition(idx);
		
		if (prob >= 0 && em_) {
		  prob = prior*ptm_model_->getPosProb( prob) /
		    ( (1-prior)*ptm_model_->getNegProb( prob) + prior*ptm_model_->getPosProb( prob)); 
		}
		if (prob >= 0) {
		  probsum += prob;
		}
	      }
	      
	      fout << "<ptmprophet_result prior=\"" << prior << "\"" << " ptm=\"PTMProphet_" << aminoacids_[0] << massshift_[0] << "\" ptm_peptide=\"";
		for (int idx = 0; idx < pep_seq.length(); idx++) {
		  double prob = prophet->getProbAtPosition(idx);
		  if (prob >= 0) {
		    if (em_) {
		      prob = prior*ptm_model_->getPosProb( prob) /
			( (1-prior)*ptm_model_->getNegProb( prob) + prior*ptm_model_->getPosProb( prob)); 
		      if (prob >= 0 && probsum > 0) {
			prob = nmods*prob/probsum;
			if (prob > 1) {
			  prob = 1;
			}
		      }
		    }
		  }
		  fout.width(1);
		  fout << pep_seq.substr(idx,1).c_str();
		  if (prob >= 0) {
		    fout << "(";
		    fout.precision(3);
		    fout.width(5);
		    fout.setf(ios::fixed);
		    fout << prob;
		    fout.width(1);
		    fout << ")";
		  }
		}
		
		
		fout << "\">" << endl;
		
		for (int idx = 0; idx < pep_seq.length(); idx++) {
		  fout.precision(3);
		  fout.width(5);
		  fout.setf(ios::fixed);
		  //		double prob = prophet->getProbAtPosition(idx);
		  double prob = prophet->getProbAtPosition(idx);
		  if (em_) {
		    prob = prior*ptm_model_->getPosProb( prob) /
		      ( (1-prior)*ptm_model_->getNegProb( prob) + prior*ptm_model_->getPosProb( prob)); 
		    
		    
		    if (prob >= 0 && probsum > 0) {
		      prob = nmods*prob/probsum;
		      if (prob > 1) {
			prob = 1;
		      }
		    }
		  }

		  if (prob >= 0) {
		    fout << "<mod_aminoacid_probability position=\"" << idx+1  << "\" probability=\"" << prob << "\"/>" << endl;
		  }
		}
	      
	      fout << "</ptmprophet_result>\n</analysis_result>" << endl;
	    }
	    
	    //	    prophet->printPTM();
	    delete prophet;
	    //mod_tags->clear();
	    in_mod = false;
	  }
	  else if (tag->isStart() && //get_pep &&
		   ! strcmp(tag->getName(), "modification_info")) {      
	    in_mod = true;
	    //       	    mod_tags->insertAtEnd(tag); 
	    //	    get_pep = false;
	    mod_pep = "";
	    mod_pep += tag->getAttributeValue("modified_peptide");
	  }


	  else if (tag->isEnd() && 
		   ! strcmp(tag->getName(), "spectrum_query")) {
	    exp_lbl = "";
	    spectrum_name = "";
	    charge = "";
	    //	    get_pep = false;
	    pep_seq = "";
	    mod_pep = "";
	    prob = 0;
	    scan = -1;
	     // TODO  allntt_prob[0] = -100; allntt_prob[1] = -100; allntt_prob[2] = -100;
	  }
		 	
	}
	delete tag;
	data = strstr(data+1, "<");      
      }
    }
    fin.close();
    fout.close();

    if (out_file_.empty()) {
      unlink(c); 
      rename(tmp_file_.c_str(), c);
    }
    
  
    delete [] nextline;
    //    inter_proph_->computeModels();
}

void PTMProphetParser::parseWriteMpx(const char* c) {
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
 
  string mod_tags = "";


  bool in_mod= false;
  bool is_nterm_pep= false;
  bool is_cterm_pep= false;  
  bool get_pep = false;
  int k=0;

  long scan = -1;
  vector<string> mtvec;
  SpectraSTLib* mtlib= NULL;
  //SpectraSTCreateParams* mtparams=NULL;
  //SpectraSTPepXMLLibImporter*  specLib = new SpectraSTPepXMLLibImporter(mtvec, mtlib, *mtparams);
  SpectraSTLibEntry* entry = NULL;
  Peptide* specPep = NULL;
  
  cRamp* cramp = NULL;

  bool skip_run = false;
  
  string dataFile = "";
  string dataExt = "";
  if (out_file_.empty()) {
    tmp_file_ = make_tmpfile_name(c);
  }
  else {
    tmp_file_ = out_file_;
  }

  ofstream fout(tmp_file_.c_str());  
  if(! fout) {
    cerr << "cannot write output to file " << tmp_file_ << endl;
    exit(1);
  }
  else {
    cerr << "INFO: Writing file " << tmp_file_ << " ..." << endl;
  }

  // TODO  double allntt_prob[3] = {-100, -100, -100};
  // for(k = 0; k < input_files_->length(); k++) {
    pwiz::util::random_access_compressed_ifstream fin(c); // read possibly gzipped files
    if(! fin) {
      cerr << "fin: error opening " << c << endl;
      exit(1);
    }
    

    while(fin.getline(nextline, line_width_)) {
      data = strstr(nextline, "<");
      while(data != NULL) {
	tag = new Tag(data);
	tag->write(fout);
	if (in_mod) {
	  mod_tags += data;
	}
	if (tag != NULL) {
	  if (tag->isStart() && !strcmp(tag->getName(), "search_summary") && !strcmp(tag->getAttributeValue("search_engine"), "SpectraST")) {
	    skip_run = true;

	  }
	  else if (tag->isStart() && !strcmp(tag->getName(), "search_summary")) {
	    skip_run = false;
	    
	  }
	  if (!skip_run && tag->isEnd() && !strcmp(tag->getName(), "search_summary")) {
	    Tag* ts_tag = new Tag("analysis_timestamp", True, True);
	    ts_tag->setAttributeValue("analysis", "ptmprophet");
	    ts_tag->setAttributeValue("time", time_);
	    ts_tag->setAttributeValue("id", "1");
	    ts_tag->write(fout);
	    delete ts_tag;
	  }

	  
	  if (!skip_run && tag->isStart() && 
	      ! strcmp(tag->getName(), "msms_pipeline_analysis")) {
	    //(*input_files_)[k]->assign(tag->getAttributeValue("summary_xml"));
	    //inter_proph_->addSearch((*input_files_)[k]);
	    fout << "<analysis_summary analysis=\"ptmprophet\" time=\"" << time_  << "\">" << endl
		 << "<ptmprophet_summary version=\"" << szTPPVersionInfo  << "\""
		 << " options=\"" << opts_.c_str() 
		 << "\">" << endl;
	    
	    fout << "<inputfile name=\"" << c << "\"/>" << endl;
	    
	    fout << "</ptmprophet_summary>" << endl;
	    
	    fout << "</analysis_summary>" << endl;
	 
	  }
	  else if (tag->isStart() && 
	      ! strcmp(tag->getName(), "msms_run_summary")) {
	    //(*input_files_)[k]->assign(tag->getAttributeValue("summary_xml"));
	    //inter_proph_->addSearch((*input_files_)[k]);
	    
	    dataFile = tag->getAttributeValue("base_name");
	    dataExt = tag->getAttributeValue("raw_data");
	    if (dataExt[0] != '.') {
	      dataFile += ".";
	    }
	    dataFile += dataExt;


	    if (cramp != NULL)
	      delete cramp;

	    cramp = new cRamp(dataFile.c_str());
	    if (!cramp->OK()) {
	      cerr << "ERROR: cannot read scan in data file " << dataFile << " exiting ..." << endl;
	      exit(1);
	    }
	    //stat_mods_hash_.clear();
	    //stat_prot_termods_hash_.clear();
	  }

	  else if (tag->isStart() && 
		   ! strcmp(tag->getName(), "spectrum_query")) {
	    //	    get_pep = true;
	    pep_seq = "";
	    mod_pep = "";
	    spectrum_name = "";
	    rt = -1;
	    if (tag->getAttributeValue("retention_time_sec") != NULL) 
	      rt = atof(tag->getAttributeValue("retention_time_sec"));

	    // store the spectrum name for later
	    int len = strlen(tag->getAttributeValue("spectrum"));
	    
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
	    //char* tmp = strrchr(spectrum_name, '.');
	    //tmp = '\0';
	    charge = tag->getAttributeValue("assumed_charge");
	    scan = atoi(tag->getAttributeValue("start_scan"));
	    
	  }
	  else if (tag->isStart() && 
		   ! strcmp(tag->getName(), "search_hit") && 
		   atoi(tag->getAttributeValue("hit_rank")) == 1) {
	    pep_seq = "";
	    pep_seq += tag->getAttributeValue("peptide");
	    calcnmass = atof(tag->getAttributeValue("calc_neutral_pep_mass"));
	    is_nterm_pep = false;
	    is_cterm_pep = false;

	    if (! strcmp(tag->getAttributeValue("peptide_prev_aa") , "-")) {
	      is_nterm_pep = true;
	    }
	    if (! strcmp(tag->getAttributeValue("peptide_next_aa") , "-")) {
	      is_cterm_pep = true;
	    }
	  }
	  else if (tag->isEnd() && //get_pep &&
		   ! strcmp(tag->getName(), "modification_info")) {   
	    if (tag->isStart()) {
	      mod_pep = "";
	      mod_pep += tag->getAttributeValue("modified_peptide");
	    }

	    if (!skip_run) {
	      PTMProphetMpx* prophet = new PTMProphetMpx(mod_pep, atoi(charge.c_str()), calcnmass, cramp, scan, aminoacids_, massshift_, &stat_mods_hash_, &stat_prot_termods_hash_, is_nterm_pep, is_cterm_pep);
	      if (prophet->init()) {
		prophet->setTolerance(mzTol_);
		prophet->combinations();
		
	      
		for (int type = 0 ; type < aminoacids_.size(); type++) {
		  
		  
		  string probstr = prophet->getPepProbString(type);
		  if (!probstr.empty()) {
		    fout << "<analysis_result analysis=\"ptmprophet\">" << endl;
		    double prior = prophet->getModPrior(type);
		    double nmods = prophet->getNumMods(type);
		    double probsum = 0;
		    int ntermod = prophet->nTermMod();
		    int ctermod = prophet->cTermMod();
		    int adj = prophet->nTermMod() ? 0 : 1;
		    for (int idx = 0; idx < pep_seq.length()+ntermod+ctermod; idx++) {
		      double prob = prophet->getProbAtPosition(type, idx);
		      
		      if (prob >= 0 && em_) {
			prob = prior*ptm_model_->getPosProb( prob) /
			  ( (1-prior)*ptm_model_->getNegProb( prob) + prior*ptm_model_->getPosProb( prob)); 
		      }
		      if (prob >= 0) {
			probsum += prob;
		      }
		    }
		    
		    fout << "<ptmprophet_result prior=\"" << prior << "\"" << " ptm=\"PTMProphet_" 
			 << aminoacids_[type] << massshift_[type] << "\" ptm_peptide=\"";
		    
		    
		    if (prophet->hasNTermMod()) { //(prophet->nTermMod(type)) {
		      ntermod = 1;
		      double prob = prophet->getProbAtPosition(type,0);
		      char aa = 'n';
		      fout.width(1);
		      fout << aa;
		      if (prob >= 0) {
			fout << "(";
			fout.precision(3);
			fout.width(5);
			fout.setf(ios::fixed);
			fout << prob;
			fout.width(1);
			fout << ")";
		      }
		      
		    }
		    
		    for (int idx = ntermod; idx < pep_seq.length()+ntermod; idx++) {
		      double prob = prophet->getProbAtPosition(type,idx-adj);
		      if (prob >= 0) {
			if (em_) {
			  prob = prior*ptm_model_->getPosProb( prob) /
			    ( (1-prior)*ptm_model_->getNegProb( prob) + prior*ptm_model_->getPosProb( prob)); 
			  
			  if (prob >= 0 && probsum > 0) {
			    prob = nmods*prob/probsum;
			    if (prob > 1) {
			      prob = 1;
			    }
			  }
			}
		      }
		      fout.width(1);
		      fout << pep_seq.substr(idx-ntermod,1).c_str();
		      if (prob >= 0) {
			fout << "(";
			fout.precision(3);
			fout.width(5);
			fout.setf(ios::fixed);
			fout << prob;
			fout.width(1);
			fout << ")";
		      }
		    }
		    if (prophet->cTermMod(type)) {
		      double prob = prophet->getProbAtPosition(type,pep_seq.length()+ntermod+ctermod-1);
		      char aa = 'c';
		      fout.width(1);
		      fout << aa;
		      if (prob >= 0) {
			fout << "(";
			fout.precision(3);
			fout.width(5);
			fout.setf(ios::fixed);
			fout << prob;
			fout.width(1);
			fout << ")";
		      }
		      
		    }
		    
		    
		    fout << "\">" << endl;
		    
		    
		    if (prophet->hasNTermMod()) { //(prophet->nTermMod(type)) {if (prophet->nTermMod(type)) {
		      ntermod = 1;
		      
		      double prob = prophet->getProbAtPosition(type,0);
		      char aa = 'n';
		      fout.width(1);
		      fout << aa;
		      if (prob >= 0) {
			fout << "<mod_aminoacid_probability position=\"" << 0  << "\" probability=\"" << prob << "\"/>" << endl;
		      }
		      
		    }
		    
		    for (int idx = ntermod; idx < pep_seq.length()+ntermod; idx++) {
		      fout.precision(3);
		      fout.width(5);
		      fout.setf(ios::fixed);
		      //		double prob = prophet->getProbAtPosition(idx);
		      double prob = prophet->getProbAtPosition(type, idx);
		      if (prob >= 0 && em_) {
			prob = prior*ptm_model_->getPosProb( prob) /
			  ( (1-prior)*ptm_model_->getNegProb( prob) + prior*ptm_model_->getPosProb( prob)); 
			
			
			if (prob >= 0 && probsum > 0) {
			  prob = nmods*prob/probsum;
			  if (prob > 1) {
			    prob = 1;
			  }
			}
		      }
		      
		      if (prob >= 0) {
			fout << "<mod_aminoacid_probability position=\"" << idx+1-ntermod  << "\" probability=\"" << prob << "\"/>" << endl;
		      }
		    }
		    
		    if (prophet->cTermMod(type)) {
		      double prob = prophet->getProbAtPosition(type,pep_seq.length()+ntermod+ctermod-1);
		      char aa = 'c';
		      fout.width(1);
		      fout << aa;
		      if (prob >= 0) {
		        fout << "<mod_aminoacid_probability position=\"" << pep_seq.length()+ctermod  << "\" probability=\"" << prob << "\"/>" << endl;
			fout << ")";
		      }
		      
		    }
		    
		    
		    
		    fout << "</ptmprophet_result>\n</analysis_result>" << endl;
		  }
		}
		
		//	    prophet->printPTM();
		delete prophet;
		prophet = NULL;
		mod_tags = "";
		in_mod = false;
	      }
	      else {
		if (mod_pep != "") {
		  cerr << "WARNING: Cannot initialize for sequence: " << mod_pep.c_str() 
		       << ", unknown mods may exist in spectrum " << spectrum_name.c_str() << endl;
		}
		delete prophet;
		//fout << mod_tags;
		in_mod = false;
		prophet = NULL;
	      }
	    }
	  }
	  else if (tag->isStart() && //get_pep &&
		   ! strcmp(tag->getName(), "modification_info")) {      
	    in_mod = true;
       	    mod_tags = data; 
	    //	    get_pep = false;
	    mod_pep = "";
	    mod_pep += tag->getAttributeValue("modified_peptide");
	  }


	  else if (tag->isEnd() && 
		   ! strcmp(tag->getName(), "spectrum_query")) {
	    exp_lbl = "";
	    spectrum_name = "";
	    charge = "";
	    //	    get_pep = false;
	    pep_seq = "";
	    mod_pep = "";
	    prob = 0;
	    scan = -1;
	     // TODO  allntt_prob[0] = -100; allntt_prob[1] = -100; allntt_prob[2] = -100;
	  }
		 	
	}
	delete tag;
	data = strstr(data+1, "<");      
      }
    }
    fin.close();
    fout.close();
    if (out_file_.empty()) {
      unlink(c); 
      rename(tmp_file_.c_str(), c);
      cerr << "INFO: Renaming output file to " << c << " ..." << endl;
    }
    
  
    delete [] nextline;
    //    inter_proph_->computeModels();
}



void PTMProphetParser::parseWriteUpdateMpx(const char* c) {
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
 
  // string mod_tags = "";
  Array<Tag*>* mod_tags = new Array<Tag*>();//NULL;

  bool in_mod= false;
  
  bool get_pep = false;
  int k=0;

  long scan = -1;
  vector<string> mtvec;
  SpectraSTLib* mtlib= NULL;
  //SpectraSTCreateParams* mtparams=new SpectraSTCreateParams();
  // SpectraSTPepXMLLibImporter*  specLib = new SpectraSTPepXMLLibImporter(mtvec, mtlib, *mtparams);
  SpectraSTLibEntry* entry = NULL;
  Peptide* specPep = NULL;
  
  cRamp* cramp = NULL;

  string dataFile = "";
  string dataExt = "";

  bool skip_run = false;

  bool is_nterm_pep = false;
  
  bool is_cterm_pep = false;

  if (out_file_.empty()) {
    tmp_file_ = make_tmpfile_name(c);
  }
  else {
    tmp_file_ = out_file_;
  }

  ofstream fout(tmp_file_.c_str());  
  if(! fout) {
    cerr << "cannot write output to file " << tmp_file_ << endl;
    exit(1);
  }
  else {
    cerr << "INFO: Writing file " << tmp_file_ << " ..." << endl;
  }

  // TODO 

  // TODO  double allntt_prob[3] = {-100, -100, -100};
  // for(k = 0; k < input_files_->length(); k++) {
    pwiz::util::random_access_compressed_ifstream fin(c); // read possibly gzipped files
    if(! fin) {
      cerr << "fin: error opening " << c << endl;
      exit(1);
    }
    cerr << "INFO: Reading file " << c << " ..." << endl;

    while(fin.getline(nextline, line_width_)) {
      data = strstr(nextline, "<");
      while(data != NULL) {
	tag = new Tag(data);
	if (!in_mod && strcmp(tag->getName(), "modification_info")) {
	  tag->write(fout);
	}
 
	if (in_mod) {
	  mod_tags->insertAtEnd(tag);
	}

	if (tag != NULL) {
	  if (tag->isStart() && !strcmp(tag->getName(), "search_summary") && !strcmp(tag->getAttributeValue("search_engine"), "SpectraST")) {
	    skip_run = true;

	  }
	  else if (tag->isStart() && !strcmp(tag->getName(), "search_summary")) {
	    skip_run = false;
	    
	  }
	  if (!skip_run && tag->isEnd() && !strcmp(tag->getName(), "search_summary")) {
	    Tag* ts_tag = new Tag("analysis_timestamp", True, True);
	    ts_tag->setAttributeValue("analysis", "ptmprophet");
	    ts_tag->setAttributeValue("time", time_);
	    ts_tag->setAttributeValue("id", "1");
	    ts_tag->write(fout);
	    delete ts_tag;

	  }
	  
	  if (!skip_run && tag->isStart() && 
	      ! strcmp(tag->getName(), "msms_pipeline_analysis")) {
	    //(*input_files_)[k]->assign(tag->getAttributeValue("summary_xml"));
	    //inter_proph_->addSearch((*input_files_)[k]);
	    fout << "<analysis_summary analysis=\"ptmprophet\" time=\"" << time_   << "\">" << endl
		 << "<ptmprophet_summary version=\"" << szTPPVersionInfo  << "\""
		 << " options=\"" << opts_.c_str() 
		 << "\">" << endl;

	    fout << "<inputfile name=\"" << c << "\"/>" << endl;

	    fout << "</ptmprophet_summary>" << endl;

	    fout << "</analysis_summary>" << endl;
	  }
	  else if (! strcmp(tag->getName(), "aminoacid_modification") && !strcmp(tag->getAttributeValue("variable"), "N")) {
	    if (stat_mods_hash_.find(*tag->getAttributeValue("aminoacid")) != stat_mods_hash_.end() 
		&& fabs(atof(tag->getAttributeValue("massdiff"))-stat_mods_hash_[*tag->getAttributeValue("aminoacid")])>0.001) {
	      cerr << "ERROR: multiple static mods with different masses found on aminoacid " << tag->getAttributeValue("aminoacid") << ", cannot be processed together with PTMProphet." << endl;
	      exit(1);
	    }
	    stat_mods_hash_[*tag->getAttributeValue("aminoacid")] = atof(tag->getAttributeValue("massdiff"));
	  }
	  else if (! strcmp(tag->getName(), "terminal_modification") && !strcmp(tag->getAttributeValue("variable"), "N") && !strcmp(tag->getAttributeValue("protein_terminus"), "N")) {
	    if (stat_mods_hash_.find(tolower(*tag->getAttributeValue("terminus"))) != stat_mods_hash_.end()
		&& fabs(atof(tag->getAttributeValue("massdiff"))-stat_mods_hash_[tolower(*tag->getAttributeValue("terminus"))])>0.001) {
	      cerr << "ERROR: multiple static mods with different masses found on terminus " << tag->getAttributeValue("terminus") << ", cannot be processed together with PTMProphet." << endl;
	      exit(1);
	    }
	    stat_mods_hash_[tolower(tolower(*tag->getAttributeValue("terminus")))] = atof(tag->getAttributeValue("massdiff"));
	  }
	  else if (! strcmp(tag->getName(), "terminal_modification") && !strcmp(tag->getAttributeValue("variable"), "N") && !strcmp(tag->getAttributeValue("protein_terminus"), "Y")) {
	    if (stat_prot_termods_hash_.find(tolower(*tag->getAttributeValue("terminus"))) != stat_prot_termods_hash_.end()
		&& fabs(atof(tag->getAttributeValue("massdiff"))-stat_prot_termods_hash_[tolower(*tag->getAttributeValue("terminus"))])>0.001) {
	      cerr << "ERROR: multiple static mods with different masses found on terminus " << tag->getAttributeValue("terminus") << ", cannot be processed together with PTMProphet." << endl;
	      exit(1);
	    }
	    stat_prot_termods_hash_[tolower(*tag->getAttributeValue("terminus"))] = atof(tag->getAttributeValue("massdiff"));
	  }
	  else if (tag->isStart() && 
	      ! strcmp(tag->getName(), "msms_run_summary")) {
	    //(*input_files_)[k]->assign(tag->getAttributeValue("summary_xml"));
	    //inter_proph_->addSearch((*input_files_)[k]);
	    dataFile = tag->getAttributeValue("base_name");
	    dataExt = tag->getAttributeValue("raw_data");
	    if (dataExt[0] != '.') {
	      dataFile += ".";
	    }
	    dataFile += dataExt;
	    
	    
	    if (cramp != NULL)
	      delete cramp;

	    cramp = new cRamp(dataFile.c_str());
	    if (!cramp->OK()) {
	      cerr << "ERROR: cannot read scan in data file " << dataFile << " exiting ..." << endl;
	      exit(1);
	    }
	    //stat_mods_hash_.clear();
	    //stat_prot_termods_hash_.clear();
	  }

	  else if (tag->isStart() && 
		   ! strcmp(tag->getName(), "spectrum_query")) {
	    //	    get_pep = true;
	    pep_seq = "";
	    mod_pep = "";
	    spectrum_name = "";
	    rt = -1;
	    if (tag->getAttributeValue("retention_time_sec") != NULL) 
	      rt = atof(tag->getAttributeValue("retention_time_sec"));

	    // store the spectrum name for later
	    int len = strlen(tag->getAttributeValue("spectrum"));
	    
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
	    //char* tmp = strrchr(spectrum_name, '.');
	    //tmp = '\0';
	    charge = tag->getAttributeValue("assumed_charge");
	    scan = atoi(tag->getAttributeValue("start_scan"));
	    
	  }
	  else if (tag->isStart() && 
		   ! strcmp(tag->getName(), "search_hit") && 
		   atoi(tag->getAttributeValue("hit_rank")) == 1) {
	    pep_seq = "";
	    pep_seq += tag->getAttributeValue("peptide");

	    calcnmass = atof(tag->getAttributeValue("calc_neutral_pep_mass"));

	    is_nterm_pep = false;
	    is_cterm_pep = false;

	    if (! strcmp(tag->getAttributeValue("peptide_prev_aa") , "-")) {
	      is_nterm_pep = true;
	    }
	    if (! strcmp(tag->getAttributeValue("peptide_next_aa") , "-")) {
	      is_cterm_pep = true;
	    }
	  }
	  else if (tag->isEnd() && //get_pep &&
		   ! strcmp(tag->getName(), "modification_info")) {   
	    if (tag->isStart()) {
	      mod_pep = "";
	      mod_pep += tag->getAttributeValue("modified_peptide");
	    }
	    
	    if (!skip_run) {

	      PTMProphetMpx* prophet = new PTMProphetMpx(mod_pep, atoi(charge.c_str()),   calcnmass, cramp,
							 scan, aminoacids_, massshift_, &stat_mods_hash_, &stat_prot_termods_hash_, is_nterm_pep, is_cterm_pep);
	      if (prophet->init()) {
		prophet->setTolerance(mzTol_);
		prophet->combinations();
		//  mod_tags->insertAtEnd(tag);
		writeUpdatedModTags(prophet,fout, mod_pep);
		//	    tag->write(fout)
		for (int type = 0 ; type < aminoacids_.size(); type++) {
		  string probstr = prophet->getPepProbString(type);
		  if (!probstr.empty()) {
		    fout << "<analysis_result analysis=\"ptmprophet\">" << endl;
		    double prior = prophet->getModPrior(type);
		    double nmods = prophet->getNumMods(type);
		    double probsum = 0;
		    int ntermod = prophet->nTermMod();
		    int ctermod = prophet->cTermMod();
		    int adj = prophet->nTermMod() ? 0 : 1;
		    for (int idx = 0; idx < pep_seq.length()+ntermod+ctermod; idx++) {
		      double prob = prophet->getProbAtPosition(type, idx);
		    
		      if (prob >= 0 && em_) {
			prob = prior*ptm_model_->getPosProb( prob) /
			  ( (1-prior)*ptm_model_->getNegProb( prob) + prior*ptm_model_->getPosProb( prob)); 
		      }
		      if (prob >= 0) {
			probsum += prob;
		      }
		    }
		  
		    fout << "<ptmprophet_result prior=\"" << prior << "\"" << " ptm=\"PTMProphet_" 
			 << aminoacids_[type] << massshift_[type] << "\" ptm_peptide=\"";
		  
		  
		    if (prophet->hasNTermMod() && aminoacids_[type].find('n') != string::npos) { //(prophet->nTermMod(type)) {
		      ntermod = 1;
		      double prob = prophet->getProbAtPosition(type,0);
		      char aa = 'n';
		      fout.width(1);
		      fout << aa;
		      if (prob >= 0) {
			fout << "(";
			fout.precision(3);
			fout.width(5);
			fout.setf(ios::fixed);
			fout << prob;
			fout.width(1);
			fout << ")";
		      }
		  
		    }

		    for (int idx = ntermod; idx < pep_seq.length()+ntermod; idx++) {
		      double prob = prophet->getProbAtPosition(type,idx);
		      if (prob >= 0) {
			if (em_) {
			  prob = prior*ptm_model_->getPosProb( prob) /
			    ( (1-prior)*ptm_model_->getNegProb( prob) + prior*ptm_model_->getPosProb( prob)); 
			
			  if (prob >= 0 && probsum > 0) {
			    prob = nmods*prob/probsum;
			    if (prob > 1) {
			      prob = 1;
			    }
			  }
			}
		      }
		      fout.width(1);
		      fout << pep_seq.substr(idx-ntermod,1).c_str();
		      if (prob >= 0) {
			fout << "(";
			fout.precision(3);
			fout.width(5);
			fout.setf(ios::fixed);
			fout << prob;
			fout.width(1);
			fout << ")";
		      }
		    }
		    if (prophet->cTermMod(type)) {
		      double prob = prophet->getProbAtPosition(type,pep_seq.length()+ntermod+ctermod-1);
		      char aa = 'c';
		      fout.width(1);
		      fout << aa;
		      if (prob >= 0) {
			fout << "(";
			fout.precision(3);
			fout.width(5);
			fout.setf(ios::fixed);
			fout << prob;
			fout.width(1);
			fout << ")";
		      }
		  
		    }
		  
		  
		    fout << "\">" << endl;
		  
	  
		    if (prophet->hasNTermMod()) { //(prophet->nTermMod(type)) {
		      ntermod = 1;
		      double prob = prophet->getProbAtPosition(type,0);
		      char aa = 'n';
		      fout.width(1);
		      fout << aa;
		      if (prob >= 0) {
			fout << "<mod_aminoacid_probability position=\"" << 0  << "\" probability=\"" << prob << "\"/>" << endl;
		      }
		  
		    }
		  
		    for (int idx = ntermod; idx < pep_seq.length()+ntermod; idx++) {
		      fout.precision(3);
		      fout.width(5);
		      fout.setf(ios::fixed);
		      //		double prob = prophet->getProbAtPosition(idx);
		      double prob = prophet->getProbAtPosition(type, idx);
		      if (prob >= 0 && em_) {
			prob = prior*ptm_model_->getPosProb( prob) /
			  ( (1-prior)*ptm_model_->getNegProb( prob) + prior*ptm_model_->getPosProb( prob)); 
		      
		    
			if (prob >= 0 && probsum > 0) {
			  prob = nmods*prob/probsum;
			  if (prob > 1) {
			    prob = 1;
			  }
			}
		      }
		    
		      if (prob >= 0) {
			fout << "<mod_aminoacid_probability position=\"" << idx+1-ntermod  << "\" probability=\"" << prob << "\"/>" << endl;
		      }
		    }
		  
		    if (prophet->cTermMod(type)) {
		      double prob = prophet->getProbAtPosition(type,pep_seq.length()+ntermod+ctermod-1);
		      char aa = 'c';
		      fout.width(1);
		      fout << aa;
		      if (prob >= 0) {
		        fout << "<mod_aminoacid_probability position=\"" << pep_seq.length()+ctermod  << "\" probability=\"" << prob << "\"/>" << endl;
			fout << ")";
		      }
		  
		    }
		  
		  
		  
		    fout << "</ptmprophet_result>\n</analysis_result>" << endl;
		  }
		}
		//	    prophet->printPTM();
		delete prophet;
		prophet = NULL;
		//for (int m = 0; m < mod_tags->size(); m++) {
		//delete (*mod_tags)[m];
		//}
		mod_tags->clear();
		in_mod = false;
	      
	      }
	      else {  
		if (mod_pep != "") {
		  cerr << "WARNING: Cannot initialize for sequence: " << mod_pep.c_str() 
		       << ", unknown mods may exist in spectrum " << spectrum_name.c_str() << endl;
		}
		
		delete prophet;
		prophet = NULL;

		//mod_tags->insertAtEnd(tag);
		for (int m = 0; m < mod_tags->size(); m++) {
		  (*mod_tags)[m]->write(fout);
		  //delete (*mod_tags)[m];
		}
		//tag->write(fout);
		in_mod = false;

		mod_tags->clear();
	
	      }
	    }

	    //mod_tags->insertAtEnd(tag);
	    for (int m = 0; m < mod_tags->size(); m++) {
	      (*mod_tags)[m]->write(fout);
	      //delete (*mod_tags)[m];
	    }
	    //tag->write(fout);
	    in_mod = false;
	    
	    mod_tags->clear();
	  }
	  else if (tag->isStart() && //get_pep &&
		   ! strcmp(tag->getName(), "modification_info")) {      
	    in_mod = true;
	    mod_tags->insertAtEnd(tag); 
	    //	    get_pep = false;
	    mod_pep = "";
	    mod_pep += tag->getAttributeValue("modified_peptide");
	  }


	  else if (tag->isEnd() && 
		   ! strcmp(tag->getName(), "spectrum_query")) {
	    exp_lbl = "";
	    spectrum_name = "";
	    charge = "";
	    //	    get_pep = false;
	    pep_seq = "";
	    mod_pep = "";
	    prob = 0;
	    scan = -1;
	     // TODO  allntt_prob[0] = -100; allntt_prob[1] = -100; allntt_prob[2] = -100;
	  }
		 	
	}
	if (!in_mod) {
	  delete tag;
	}

	data = strstr(data+1, "<");      
      }
    }
    fin.close();
    fout.close();
    
    if (out_file_.empty()) {
      unlink(c); 
      rename(tmp_file_.c_str(), c);
      cerr << "INFO: Renaming output file to " << c << " ..." << endl;
    }
    
  
    delete [] nextline;
    //    inter_proph_->computeModels();
}



bool compare_probs (ProbPos* i,ProbPos* j) { 
  return (i->prob_>j->prob_); 
}

void PTMProphetParser::writeUpdatedModTags(PTMProphet* proph, ofstream& fout, string& mod_pep_str) {
  double prior = proph->getModPrior();
  double prob = -1;
  string new_mpep = "";
  double poten_sites = proph->getNumModSites();
  double mod_sites = proph->getNumMods();

  Peptide* mpep = proph->getPeptide();

  Tag* tag;

  vector<ProbPos*> prob_vector;
  TPP_HASHMAP_T<int, int> pos_rank_hash;

  TPP_HASHMAP_T<int, int> pos_newpos_hash;

  int new_mods = 0;
  int idx = 0;
  //  int adj =  mpep->nTermMod.empty() ? 1 : 0;
  for (idx = mpep->nTermMod.empty() ? 0 : 1; idx < mpep->NAA(); idx++) {
    if ((prob = proph->getProbAtPosition(idx)) >= 0) {
      if (em_) {
	prob = prior*ptm_model_->getPosProb( prob) /
	  ( (1-prior)*ptm_model_->getNegProb( prob) + prior*ptm_model_->getPosProb( prob)); 
      }
      prob_vector.push_back(new ProbPos(prob, idx));  

    }
  }

  sort(prob_vector.begin(), prob_vector.end(), compare_probs);
  
  for (int i=0; i<prob_vector.size(); i++) {
    pos_rank_hash[prob_vector[i]->pos_] = i; //i is rank

  }

  string::size_type mpep_idx = 0;
  string tok = "";
  int pos = 0;
  ostringstream omass;
  ostringstream opos;

  string nterm_mod = "";
  string cterm_mod = "";
  
  while (mpep_idx!= string::npos) {
    tok = mpep->nextAAToken(mod_pep_str, mpep_idx, mpep_idx);
    if (tok[0] == 'n' || tok[0] == 'c') {
      
       nterm_mod = (tok[0] == 'n') ? tok : "" ;

       cterm_mod = (tok[0] == 'c') ? tok : "" ;


      new_mpep += tok;
    }
    else if (pos >= 0 && (prob = proph->getProbAtPosition(pos)) < 0 || pos_rank_hash[pos] < mod_sites) {
      new_mpep += tok;
      if (prob >= 0 && tok.length() == 1) {

	omass.str("");
	omass.precision(3);
	omass << "[" << mpep->getAATokenMonoisotopicMass(tok) + massshift_[0] << "]";
	new_mpep += omass.str().c_str();
      }  
   
      pos++;
    }
    else if (pos > 0 && pos <  mpep->NAA()) {
      new_mpep += tok.substr(0,1);
      pos++;
    }
    else {
      pos++;
    }
  }

  tag = new Tag("modification_info", true, false);
  tag->setAttributeValue("modified_peptide", new_mpep.c_str());
  if (nterm_mod != "") {
    tok = nterm_mod;
    omass.str("");
    opos.str("");
    double mass = mpep->getAATokenMonoisotopicMass(tok);
    omass <<  mass;
    tag->setAttributeValue("mod_nterm_mass", omass.str().c_str());
  }


  if (cterm_mod != "") {
    tok = cterm_mod;
    omass.str("");
    opos.str("");
    double mass = mpep->getAATokenMonoisotopicMass(tok);
    omass <<  mass;
    tag->setAttributeValue("mod_cterm_mass", omass.str().c_str());
  }
   


  tag->write(fout);
  delete tag;
  mpep_idx = 0;
  pos = 0;
  bool have_smod = false;
  double stat_mod = 0;

  double stat_nterm_mass = 0;
  double stat_cterm_mass = 0;
  int rank = 0;

  while (mpep_idx!= string::npos) {
    tok = mpep->nextAAToken(new_mpep, mpep_idx, mpep_idx);
    omass.str("");
    opos.str("");

    stat_mod = 0;
    have_smod = false;
    if (stat_mods_hash_.find(*(tok.substr(0,1).c_str())) != stat_mods_hash_.end()) {
       stat_mod = stat_mods_hash_[*(tok.substr(0,1).c_str())];
       have_smod = true;
    }
    
    if (tok.find('n') != string::npos || tok.find('c') != string::npos) {
      continue;
    }
    else if (pos >= 0) {
      prob = proph->getProbAtPosition(pos);
      int rank = pos_rank_hash[pos];
      if (prob >= 0 && rank < mod_sites) {
	  opos << pos+1;
	  tag = new Tag("mod_aminoacid_mass", true, true);
	  tag->setAttributeValue("position", opos.str().c_str());
	  //new_mpep += omass.str().c_str();
	  omass.precision(7);
	  
	  double mass = mpep->getAATokenMonoisotopicMass(tok);//  mpep->getAATokenMonoisotopicMass(tok.substr(0,1)) + massshift_ + stat_mod;
	  omass <<  mass;
	  tag->setAttributeValue("mass", omass.str().c_str());
	  tag->write(fout);
	  delete tag; 
      }
      else if (tok.length() > 1){
	  opos << pos+1;
	  tag = new Tag("mod_aminoacid_mass", true, true);
	  tag->setAttributeValue("position", opos.str().c_str());
	  omass.precision(7);
	  double mass = mpep->getAATokenMonoisotopicMass(tok);
	  omass <<  mass;
	  tag->setAttributeValue("mass", omass.str().c_str());
	  tag->write(fout);
	  delete tag;
	  
      }
      else if (have_smod) {
	opos << pos+1;
	tag = new Tag("mod_aminoacid_mass", true, true);
	tag->setAttributeValue("position", opos.str().c_str());
	omass.precision(7);
	double mass = mpep->getAATokenMonoisotopicMass(tok)+stat_mod;
	omass <<  mass;
	tag->setAttributeValue("mass", omass.str().c_str());
	tag->write(fout);
	delete tag;
	
      }

      pos++;
      
    }
  
   
    
  }
  
  tag = new Tag("modification_info",  false, true);
  tag->write(fout);
  delete tag;


  for (int i=0; i<prob_vector.size(); i++) {
    delete prob_vector[i];
  }

}


void PTMProphetParser::writeUpdatedModTags(PTMProphetMpx* proph, ofstream& fout, string& mod_pep_str) {

  TPP_HASHMAP_T<int, TPP_HASHMAP_T<int, int>* >* pos_rank_hash = new TPP_HASHMAP_T<int, TPP_HASHMAP_T<int, int>* >();
  TPP_HASHMAP_T<int, vector<int>* >* pos_types_hash = new TPP_HASHMAP_T<int, vector<int>* >();
  TPP_HASHMAP_T<int, int> * mod_positions = new TPP_HASHMAP_T<int, int> ();
  double mod_sites;
  double poten_sites;
  Peptide  * mpep = proph->getPeptide();
  
  vector<ProbPos*> prob_vector;
  Tag* tag;

  
  if (proph->hasNTermMod() && mod_pep_str.substr(0,1) != "n") {
    mod_pep_str.insert(0, "n");
  }


  for (int type = 0 ; type < aminoacids_.size(); type++) {
    (*pos_rank_hash)[type] = new TPP_HASHMAP_T<int, int>();
    double prior = proph->getModPrior(type);
    double prob = -1;
   
    poten_sites = proph->getNumModSites(type);
    mod_sites = proph->getNumMods(type);
    
    
    //TPP_HASHMAP_T<int, int> pos_rank_hash;
    
    //    TPP_HASHMAP_T<int, int> pos_newpos_hash;
    
    int new_mods = 0;
    int idx = 0;
    //    int nterm_mod = mpep->nTermMod.empty() ? 0 : 1;

    int nterm_mod = proph->hasNTermMod() ? 1 : 0;
    for (idx = 0; idx <= mpep->NAA()+nterm_mod; idx++) {
      if ((prob = proph->getProbAtPosition(type, idx)) >= 0) {
	if (em_) {
	  prob = prior*ptm_model_->getPosProb( prob) /
	    ( (1-prior)*ptm_model_->getNegProb( prob) + prior*ptm_model_->getPosProb( prob)); 
	}
	prob_vector.push_back(new ProbPos(prob, idx));  
	
      }
    }
    
    sort(prob_vector.begin(), prob_vector.end(), compare_probs);
  

    for (int i=0; i<prob_vector.size(); i++) {
      (*(*pos_rank_hash)[type])[prob_vector[i]->pos_] = i; //i is rank

      if (pos_types_hash->find(prob_vector[i]->pos_) == pos_types_hash->end()) {
	(*pos_types_hash)[prob_vector[i]->pos_] = new vector<int>();
      }

      (*pos_types_hash)[prob_vector[i]->pos_]->push_back(type);
      
    }
					 
    
    for (int i=0; i<prob_vector.size(); i++) {
      delete prob_vector[i];
    }
    prob_vector.clear();
    
  }

    
  string new_mpep = "";

  string::size_type mpep_idx = 0;
  string tok = "";
  int pos = 0;
  ostringstream omass;
  ostringstream opos;
  

  bool have_smod = false;
  double stat_mod = 0;

  string nterm_mod = "";
  string cterm_mod = "";
  int nterm_flag = 0;
  double prob;
  int type=-1;
  while (mpep_idx!= string::npos) {
    
    tok = mpep->nextAAToken(mod_pep_str, mpep_idx, mpep_idx);

    
    stat_mod = 0;
    have_smod = false;
    if (stat_mods_hash_.find(*(tok.substr(0,1).c_str())) != stat_mods_hash_.end()) {
      stat_mod = stat_mods_hash_[*(tok.substr(0,1).c_str())];
      have_smod = true;
    }
    
    double stat_tok_mass = mpep->getAATokenMonoisotopicMass(tok.substr(0,1))+stat_mod;
    if (0 && (tok[0] == 'n' || tok[0] == 'c') ) {
      
      if (tok[0] == 'n') {
	//pos -= 1;
	nterm_mod =tok;
	nterm_flag = 1;
      }
      else {
	nterm_mod = "" ;
      }
	
      
      cterm_mod = (tok[0] == 'c') ? tok : "" ;
      
      
      new_mpep += tok;
    }
    else {
      if (tok.length() > 1 || (tok[0] != 'n' && tok[0] != 'c'))  {
	new_mpep += tok.substr(0,1);
      }
      
  
      double maxProb = -1;
      for (int i=0; pos_types_hash->find(pos) != pos_types_hash->end() && 
	     i < (*(*pos_types_hash)[pos]).size(); i++) {
	type = (*(*pos_types_hash)[pos])[i];
	if ( (prob = proph->getProbAtPosition(type, pos)) >= 0 && 
	     (*(*pos_rank_hash)[type])[pos] < (mod_sites = proph->getNumMods(type))) {
	  if (prob > maxProb && ( (*mod_positions).find(pos) == (*mod_positions).end() || (*mod_positions)[pos] == type )) {
	    maxProb = prob;
	  }
	}

      }

      if (maxProb >= 0) {

	for (int i=0; pos_types_hash->find(pos) != pos_types_hash->end()
	       && i < (*(*pos_types_hash)[pos]).size(); i++) {
	  type = (*(*pos_types_hash)[pos])[i];
	  if ( (prob = proph->getProbAtPosition(type, pos)) >= 0 && 
	       (*(*pos_rank_hash)[type])[pos] < (mod_sites = proph->getNumMods(type))) {
	    
	    if (prob == maxProb && (*mod_positions).find(pos) == (*mod_positions).end()  ) {
	      omass.str("");
	      if ( stat_tok_mass + massshift_[type] >=  1 && stat_tok_mass + massshift_[type] < 10) {
		omass.precision(1);
	      }  
	      else if ( stat_tok_mass + massshift_[type] >=  10 && stat_tok_mass + massshift_[type] < 100) {
		omass.precision(2);
	      }  
	      else if ( stat_tok_mass + massshift_[type] >=  100 && stat_tok_mass + massshift_[type] < 1000) {
		omass.precision(3);
	      }  
	      else if ( stat_tok_mass + massshift_[type] >=  1000 && stat_tok_mass + massshift_[type] < 10000) {
		omass.precision(4);
	      }  

	      omass << "[" << stat_tok_mass + massshift_[type] << "]";
	      new_mpep += omass.str().c_str();
	      if (tok[0] == 'n') {
		//pos -= 1;
		nterm_mod =new_mpep;
		nterm_flag = 1;
	      }
	      if (tok[0] == 'c') {
		//pos -= 1;
		cterm_mod =new_mpep.substr(new_mpep.find("c"));
		//cterm_flag = 1;
	      }

	      (*mod_positions).insert(make_pair(pos, type));
	      
	      for (int tp = 0 ; tp != type && tp < aminoacids_.size(); tp++) {
		if ( aminoacids_[tp].find(tok.substr(0,1)) != string::npos) {
		  for (int x = 0 ; x < (*pos_rank_hash)[tp]->size(); x++) {
		    (*(*pos_rank_hash)[tp])[x]--;
		  }
		}
	      }
	      break;
	    }
	    
	  }
	}
      }
      
    }
    
    
    pos++;
    
  }
  
  tag = new Tag("modification_info", true, false);
  tag->setAttributeValue("modified_peptide", new_mpep.c_str());
  if (nterm_mod != "") {
    tok = nterm_mod;
    omass.str("");
    opos.str("");
    double mass = mpep->getAATokenMonoisotopicMass(tok);
    omass.precision(7);
    omass <<  mass;
    tag->setAttributeValue("mod_nterm_mass", omass.str().c_str());
  }
  else if (stat_mods_hash_.find('n') != stat_mods_hash_.end()) {
    omass.str("");
    opos.str("");
    double mass = mpep->getAATokenMonoisotopicMass("n")+stat_mods_hash_['n'];
    omass.precision(7);
    omass <<  mass;
    tag->setAttributeValue("mod_nterm_mass", omass.str().c_str());

  }
  else if (proph->isNtermPep() && stat_prot_termods_hash_.find('n') != stat_prot_termods_hash_.end()) {
    omass.str("");
    opos.str("");
    double mass = mpep->getAATokenMonoisotopicMass("n")+stat_prot_termods_hash_['n'];
    omass.precision(7);
    omass <<  mass;
    tag->setAttributeValue("mod_nterm_mass", omass.str().c_str());
  }

  
  if (cterm_mod != "") {
    tok = cterm_mod;
    omass.str("");
    opos.str("");
    double mass = mpep->getAATokenMonoisotopicMass(tok);
    omass.precision(7);
    omass <<  mass;
    tag->setAttributeValue("mod_cterm_mass", omass.str().c_str());
  }
  else if (stat_mods_hash_.find('c') != stat_mods_hash_.end()) {
    omass.str("");
    opos.str("");
    double mass = mpep->getAATokenMonoisotopicMass("c")+stat_mods_hash_['c'];
    omass.precision(7);
    omass <<  mass;
    tag->setAttributeValue("mod_cterm_mass", omass.str().c_str());
  }
  else if (proph->isCtermPep() && stat_prot_termods_hash_.find('c') != stat_prot_termods_hash_.end()) {
    omass.str("");
    opos.str("");
    double mass = mpep->getAATokenMonoisotopicMass("c")+stat_prot_termods_hash_['c'];
    omass <<  mass;
    tag->setAttributeValue("mod_cterm_mass", omass.str().c_str());
  }

  tag->write(fout);
  delete tag;
  mpep_idx = 0;
  pos = 0;

  int rank = 0;
  
  while (mpep_idx!= string::npos) {
    tok = mpep->nextAAToken(new_mpep, mpep_idx, mpep_idx);
    omass.str("");
    opos.str("");
    
     stat_mod = 0;
     have_smod = false;
     if (stat_mods_hash_.find(*(tok.substr(0,1).c_str())) != stat_mods_hash_.end()) {
       stat_mod = stat_mods_hash_[*(tok.substr(0,1).c_str())];
       have_smod = true;
     }
    
    if (tok.find('n') != string::npos || tok.find('c') != string::npos) {
      continue;
    }
    else {
      if (tok.size() > 1 || have_smod) {
	if ( type > -1 && 
	     (prob = proph->getProbAtPosition(type, pos-nterm_flag)) >= 0 && 
	     (rank = (*(*pos_rank_hash)[type])[pos-nterm_flag]) < mod_sites) {
	  opos << pos+1;
	  tag = new Tag("mod_aminoacid_mass", true, true);
	  tag->setAttributeValue("position", opos.str().c_str());
	  //new_mpep += omass.str().c_str();
	  omass.precision(7);
	  double mass = mpep->getAATokenMonoisotopicMass(tok);//  stat_tok_mass + massshift_ + stat_mod;

	  if (tok.size() == 1  &&  have_smod) {
	    mass += stat_mod;
	  }
	  
	  omass <<  mass;
	  tag->setAttributeValue("mass", omass.str().c_str());
	  tag->write(fout);
	  delete tag; 
	}
	else if (tok.length() > 1){
	  opos << pos+1;
	  tag = new Tag("mod_aminoacid_mass", true, true);
	  tag->setAttributeValue("position", opos.str().c_str());
	  omass.precision(7);
	  double mass = mpep->getAATokenMonoisotopicMass(tok);
	  omass <<  mass;
	  tag->setAttributeValue("mass", omass.str().c_str());
	  tag->write(fout);
	  delete tag;
	  
	}
	else if (have_smod) {
	  opos << pos+1;
	  tag = new Tag("mod_aminoacid_mass", true, true);
	  tag->setAttributeValue("position", opos.str().c_str());
	  omass.precision(7);
	  double mass = mpep->getAATokenMonoisotopicMass(tok)+stat_mod;
	  omass <<  mass;
	  tag->setAttributeValue("mass", omass.str().c_str());
	  tag->write(fout);
	  delete tag;
	  
	}
      }

      pos++;
      
    }
    
    
    
  }
  
  tag = new Tag("modification_info",  false, true);
  tag->write(fout);
  delete tag;
  //CLEAN UP  
  for (int type = 0 ; type < aminoacids_.size(); type++) {
    
    delete (*pos_rank_hash)[type];
  }

  for (TPP_HASHMAP_T<int, vector<int>* >::iterator itr = pos_types_hash->begin();
       itr != pos_types_hash->end(); itr++) {
    if ((*pos_types_hash)[(*itr).first]) 
      delete (*pos_types_hash)[(*itr).first];
  }

  delete pos_types_hash;
  delete pos_rank_hash;
  delete mod_positions;

  for (int i=0; i<prob_vector.size(); i++) {
    delete prob_vector[i];
  }
  

  

}

 


