#include "RespectParser.h"


#ifdef __MINGW__
#define MSVC
#endif 

using namespace std;
typedef struct str_thdata
{
  int thread_no;
  int baseName_index;
  RespectParser* rp;
} thdata;


#ifdef MSVC
DWORD WINAPI FilterThread(LPVOID ptr) {
#else
void* FilterThread(void* ptr) {
#endif
  int step = 0;
  int tot = 0;
  int offset = -1;
  thdata *data;            
  int inc = 1;
  int b = 0;



  data = (thdata *) ptr; 
  inc = data->rp->max_threads_;
  offset = data->thread_no;
  b = data->baseName_index;  
 
  data->rp->parseReadFilter(data->rp->pepx_file_.c_str(), offset, b);

#ifdef MSVC
   ExitThread(0);
#else
   pthread_exit(0); 
#endif


}

 RespectParser::RespectParser( double tol, double isotol , int threads,  double minProb, bool keepChg, bool mzML) : Parser("respect") {
 
  mzTol_ = tol;
  isoTol_ = isotol;
  //out_dir_ = ".";
  max_threads_ = threads;
  
  minProb_ = minProb;
  //minChg_ = minChg;
  //maxChg_ = maxChg;


  keepChg_ = keepChg;

  mzML_ = mzML;
  
  total_spectra_ = 0;
  read_spectra_ = 0;

  baseNames_ = new int_hash();
  baseOffsets_ = new stream_vec_vec(); 
 
  filters_byname_ = new filter_hash();
}
 


void RespectParser::run(const char* c, const char* opts) {
  opts_ = string(opts);
  init(c);
 }

void RespectParser::parse(const char* c) {
  parseBaseNames(c);
  pepx_file_ = c;
 #ifdef MSVC
    DWORD *pId = new DWORD[max_threads_];
    HANDLE *pHandle = new HANDLE[max_threads_];
#else
    int *pId = new int[max_threads_];
    int *pHandle = new int[max_threads_];
    pthread_t pThreads[max_threads_];
#endif

      thdata data[max_threads_];
    
      int a = 0; //thread index
      int b = 0; //basename index
      int live_threads = 0;
      while (b < baseNames_->size()) {
	a = 0;
	live_threads = 0;
	while (a < max_threads_ && b < baseNames_->size()) {
	  data[a].thread_no = a;
	  data[a].rp = this;
	  data[a].baseName_index = b;
	  

	  
#ifdef MSVC
	  pHandle[a] = CreateThread(NULL,0,FilterThread,(void*) &data[a],0, NULL);
#else
	  pthread_create(&pThreads[a],NULL,FilterThread, (void*) &data[a]);
#endif
	  live_threads++;
	  a++;
	  b++;
	  
	}
	
	a = 0;
	while(a < live_threads)  {
	  void * ignore = 0;
#ifdef MSVC
	  WaitForSingleObject(pHandle[a],INFINITE);
#else
	  pthread_join(pThreads[a],&ignore);
#endif
	  a++;
	}
	

      }

    delete [] pId;
    delete [] pHandle;
  
}



void RespectParser::parseBaseNames(const char* c) {
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
  RespectFilter* filter = NULL ;
  cRamp* cramp = NULL;

  string dataFile = "";
  string dataExt = "";
  double parentMass = 0;

  int index = 0;
  //  tmp_file_ = make_tmpfile_name
  pepx_file_ = c;
  double pepprob = 0;
  streampos lastpos;
    pwiz::util::random_access_compressed_ifstream fin(c); // read possibly gzipped files
    if(! fin) {
      cerr << "fin: error opening " << c << endl;
      exit(1);
    }
    cerr << "INFO: Reading file " << c << " ..." << endl;
    lastpos = fin.tellg();
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
	      ! strcmp(tag->getName(), "msms_run_summary")) {
	    string * val = new string(tag->getAttributeValue("base_name"));
	    if (baseNames_->find(*val) == baseNames_->end()) {
	      baseNames_->insert(make_pair(string(*val), index));
	      baseOffsets_->push_back(new std::vector<streampos>());
	      index++;
	    }
	    int temp_i = (*baseNames_)[*val];
	    (*baseOffsets_)[temp_i]->push_back(lastpos);
	    

	  }
	  else if (tag->isEnd() && 
		   ! strcmp(tag->getName(), "spectrum_query")&& pepprob > minProb_) {
	    total_spectra_++;
	    pepprob = 0;
	  }

	  else if (tag->isStart() && //get_pep &&
		   ! strcmp(tag->getName(), "peptideprophet_result")) {   
	     pepprob = atof(tag->getAttributeValue("probability"));
	  }
	  else if (tag->isStart() && //get_pep &&
		   ! strcmp(tag->getName(), "interprophet_result")) {   
	     pepprob = atof(tag->getAttributeValue("probability"));

	  }
	}
	delete tag;
	data = strstr(data+1, "<");      
      }
      lastpos = fin.tellg();
    }
      delete [] nextline;
}

//void  RespectParser::setOutDir(string dir) {
//  out_dir_ = dir;
  
//}


 void RespectParser::parseReadFilter(const char* c, int t, int b) {
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
  RespectFilter* filter = NULL ;
  cRamp* cramp = NULL;

  string dataFile = "";
  string dataExt = "";
  double pepprob = 0;
  double parentMass = 0;
  int BaseIndex = -1;
  string BaseName = "";

  bool nextoffset = false;
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
    
  for (int X=0; X< (*baseOffsets_)[b]->size(); X++) {
    fin.seekg((*(*baseOffsets_)[b])[X]);
    nextoffset = false;
    while(fin.getline(nextline, line_width_)) {
      if (nextoffset) {
	break;

      }

      data = strstr(nextline, "<");

      while(data != NULL) {
	if (tag) {
	  delete tag;
	  tag = NULL;
	}
	
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
	    //	    fout << "<analysis_summary analysis=\"respect\" time=\"" << time_ << "\"/>" << endl;
	  }
	  
	  else if (! strcmp(tag->getName(), "aminoacid_modification") && !strcmp(tag->getAttributeValue("variable"), "N")) {
	    if (stat_mods_hash_.find(*tag->getAttributeValue("aminoacid")) != stat_mods_hash_.end() 
		&& fabs(atof(tag->getAttributeValue("massdiff"))-stat_mods_hash_[*tag->getAttributeValue("aminoacid")])>0.001) {
	      cerr << "ERROR: multiple static mods with different masses found on aminoacid " << tag->getAttributeValue("aminoacid") << ", cannot be processed together with reSpect." << endl;
	      exit(1);
	    }
	    stat_mods_hash_[*tag->getAttributeValue("aminoacid")] = atof(tag->getAttributeValue("massdiff"));	    
	  }
	  else if (! strcmp(tag->getName(), "terminal_modification") && !strcmp(tag->getAttributeValue("variable"), "N") && !strcmp(tag->getAttributeValue("protein_terminus"), "N")) {
	    if (stat_mods_hash_.find(tolower(*tag->getAttributeValue("terminus"))) != stat_mods_hash_.end()
		&& fabs(atof(tag->getAttributeValue("massdiff"))-stat_mods_hash_[tolower(*tag->getAttributeValue("terminus"))])>0.001) {
	      cerr << "ERROR: multiple static mods with different masses found on terminus " << tag->getAttributeValue("terminus") << ", cannot be processed together with reSpect." << endl;
	      exit(1);
	    }
	    stat_mods_hash_[tolower(*tag->getAttributeValue("terminus"))] = atof(tag->getAttributeValue("massdiff"));
	  }
	  else if (! strcmp(tag->getName(), "terminal_modification") && !strcmp(tag->getAttributeValue("variable"), "N") && !strcmp(tag->getAttributeValue("protein_terminus"), "Y")) {
	    if (stat_prot_termods_hash_.find(tolower(*tag->getAttributeValue("terminus"))) != stat_prot_termods_hash_.end()
		&& fabs(atof(tag->getAttributeValue("massdiff"))-stat_prot_termods_hash_[tolower(*tag->getAttributeValue("terminus"))])>0.001) {
	      cerr << "ERROR: multiple static mods with different masses found on terminus " << tag->getAttributeValue("terminus") << ", cannot be processed together with reSpect." << endl;
	      exit(1);
	    }
	    stat_prot_termods_hash_[tolower(*tag->getAttributeValue("terminus"))] = atof(tag->getAttributeValue("massdiff"));
	  }
	  else if (tag->isStart() && 
	      ! strcmp(tag->getName(), "msms_run_summary")) {
	    //(*input_files_)[k]->assign(tag->getAttributeValue("summary_xml"));
	    //inter_proph_->addSearch((*input_files_)[k]);
	    dataFile = tag->getAttributeValue("base_name");
	
	    BaseName = string(dataFile);
	    BaseIndex = (*baseNames_)[BaseName];
	   
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
	  else if (tag->isEnd() && 
		    ! strcmp(tag->getName(), "msms_run_summary")) {
	    nextoffset = true;
	 

	    break;
	  }

	  if (BaseIndex == b) {
	    if (tag->isStart() && 
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
	    parentMass = atof(tag->getAttributeValue("precursor_neutral_mass"));
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
	      mod_pep = pep_seq;
	    }
	    
	    else if (tag->isStart() && //get_pep &&
		     ! strcmp(tag->getName(), "peptideprophet_result")) {   
	      pepprob = atof(tag->getAttributeValue("probability"));
	    }
	    else if (tag->isStart() && //get_pep &&
		     ! strcmp(tag->getName(), "interprophet_result")) {   
	      pepprob = atof(tag->getAttributeValue("probability"));
	      
	    }
	    else if (tag->isStart() && //get_pep &&
		     ! strcmp(tag->getName(), "modification_info")) {   
	      if (tag->isStart()) {
		mod_pep = "";
		mod_pep += tag->getAttributeValue("modified_peptide");
	      }
	    }
	    else if (tag->isEnd() && //get_pep &&
		     ! strcmp(tag->getName(), "spectrum_query") &&  BaseIndex == b && 
		     BaseIndex % max_threads_ == t && pepprob > minProb_) {   
	      
	      //DDS: TODO IT HERE!!!!
	      // specLib->SpectraSTPepXMLLibImporter::addModificationsToPeptide(pep_seq, mod_tags);
	      
	      read_spectra_++;
	      
	      if (read_spectra_ % 1000 == 0) {
		cerr << "INFO: processed " << read_spectra_ << " / " << total_spectra_ << endl;
	      }
	      //if (filter) {
	      //  delete filter;
	      //  filter = NULL;
	      //}
	      
	      
	      
	      
	      if (!filter) { // && filters_byname_->find(BaseName) == filters_byname_->end() ) {
	
		string out_file = getOutFileName(dataFile);


		struct stat s;
	
		if ( !stat(out_file.c_str(), &s)) {
				cerr << "WARNING: Output file: " << out_file 
				<< " already exists, delete to rewrite." << endl;
				nextoffset = true;
		        break;
		
		}

// 		pwiz::msdata::MSDataFile* msd;
// 		try {
// 		  msd = new  pwiz::msdata::MSDataFile(out_file);
// 		  cerr << "WARNING: Output file: " << out_file 
// 		       << " already exists, delete to rewrite." << endl;
// 		  nextoffset = true;
// 		  delete msd;
// 		  break;
// 		  //outmsd_ = new  pwiz::msdata::MSDataFile(in_file_);
// 		}
// 		catch (...) {
		  

// 		}


		if (nextoffset) {
		  break;
		}


		filter = new RespectFilter(out_file, dataFile, mzML_);

		
		filters_byname_->insert(make_pair(BaseName, 
						  filter));
		
		
	      }
	      else {
	      	filter = (*filters_byname_)[BaseName];	      
	      }
	      
	      filter->init( spectrum_name, mod_pep, atoi(charge.c_str()), pepprob, 
			    parentMass, cramp, scan, 
			    &stat_mods_hash_, &stat_prot_termods_hash_, is_nterm_pep, 
			    is_cterm_pep, mzTol_, isoTol_, keepChg_);
	      
	      if (filter->set()) {
		filter->run();
	      }
	      else {
		
		//	      delete filter;
		//filter = NULL;
	      }
	      
	      mod_tags = "";
	      in_mod = false;
	    }
	    else if (tag->isStart() && !tag->isEnd() && //get_pep &&
		     ! strcmp(tag->getName(), "modification_info")) {      
	      in_mod = true;
	      mod_tags = data; 
	      
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
	      if (filter) {
		//delete filter;
		//filter = NULL;
	      }
	      // TODO  allntt_prob[0] = -100; allntt_prob[1] = -100; allntt_prob[2] = -100;
	    }
	  } 	
	}
	delete tag;
	tag = NULL;
	data = strstr(data+1, "<");      
      }
    }
    if (tag) {
      delete tag;
      tag = NULL;
    }

    if (cramp != NULL) {
      delete cramp;
      cramp = NULL;
    }
  }
  fin.close();
    //    fout.close();

    //unlink(c); 
    //rename(tmp_file_.c_str(), c);
    
    
  
    delete [] nextline;

    //    for (filter_hash::iterator itr = filters_byname_->begin(); itr != filters_byname_->end(); itr++) {
    
    if (filter) {
      filter->write();
      delete filter;
      filter = NULL;      
    }
    //}


    //    inter_proph_->computeModels();
}


 string RespectParser::getOutFileName(string& in_file) {
   
   std::string::size_type pos = 0;
   string out_file;
   if ((pos = in_file.find_last_of("\\/")) == std::string::npos) {
     pos = 0;
   }
   
   if ((pos = in_file.find_first_of('.', pos)) != std::string::npos) {
     
     out_file = in_file.substr(0,pos)+"_rs";
     out_file += in_file.substr(pos);
   }
   out_file = in_file.substr(0,pos)+"_rs"+in_file.substr(pos);
   return out_file;
   
 }
