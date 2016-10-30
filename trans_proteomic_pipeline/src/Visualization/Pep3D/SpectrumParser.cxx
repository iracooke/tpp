#include "SpectrumParser.h"



SpectrumParser::SpectrumParser(const char* xmlfile, const char* basename, int startscan, int endscan, int charge) : Parser(NULL) {

  xmlfiles_ = new Array<char*>;

  char* input = new char[strlen(xmlfile)+1];
  strcpy(input, xmlfile);
  xmlfiles_->insertAtEnd(input);

  basename_ = new char[strlen(basename)+1];
  strcpy(basename_, basename);

  start_scan_ = startscan;
  end_scan_ = endscan;
  charge_ = charge;

  databases_ = new Array<char*>;
  basenames_ = new Array<char*>;
  pepproph_timestamps_ = new Array<char*>;
  iproph_timestamps_ = new Array<char*>;
  asap_timestamps_ = new Array<char*>;
  asap_quantHighBGs_ = new Array<Boolean>;
  asap_zeroBGs_ = new Array<Boolean>;
  asap_mzBounds_ = new Array<double>;
  asap_wavelets_ = new Array<bool>;
#ifndef USE_STD_MODS
  aa_modifications_ = new Array<char*>;
  term_modifications_ = new Array<char*>;
#endif
  misc_run_conditions_ = new Array<char*>;

  result_index_ = 0;
  pepdata_index_ = 0;
  current_index_ = 0;

  prodata_component_ = new ProDataComponent(0, 0, 0, 0, 0, 0, False);

  factory_ = NULL;
  found_ = False;

  init(NULL);


}


SpectrumParser::~SpectrumParser() {
  cleanup(databases_);
  cleanup(basenames_);
  cleanup(pepproph_timestamps_);
  cleanup(iproph_timestamps_);
  cleanup(asap_timestamps_);
  delete asap_quantHighBGs_;
  delete asap_zeroBGs_;
  delete asap_mzBounds_;
  delete asap_wavelets_;
  cleanup(xmlfiles_);
  if(prodata_component_ != NULL)
    delete prodata_component_;
}



void SpectrumParser::cleanup(Array<char*>* data) {
  if(data != NULL) {
    for(int k = 0; k < data->length(); k++)
      if((*data)[k] != NULL)
	delete (*data)[k];
    delete data;
  }
}

void SpectrumParser::parse(const char* xmlfile) {


  char* data = NULL;


  Boolean analyze = False;
  Boolean collection = False;

  double probability = -4.0;
  Boolean adjusted_prob = False;
  Boolean incomplete_prob = False;

  Array<Tag*>* tags = NULL;
  long scan = -1;
  int precursor_charge;
  int index = -1;
  Array<Tag*>* search_result_tags = NULL;


  SearchResult* nextresult = NULL;

  prodatacomponent_struct comp_data;

  int asap_index = 0;

  comp_data.xpressratio[0] = 0;

  comp_data.lightfirstscan = -1;
  comp_data.lightlastscan = -1;
  comp_data.heavyfirstscan = -1;
  comp_data.heavylastscan = -1;
  comp_data.lightmass = -1.0;
  comp_data.heavymass = -1.0;
  comp_data.masstol = -1.0;
  comp_data.xpresslight = -1;


  comp_data.asap_mean = -3.0;
  comp_data.asap_err = -1.0;
  comp_data.asapratio_index = -1;
  comp_data.score_summary[0] = 0;
  char search_engine[300];
  search_engine[0] = 0;

  // match to basename
  char base_match[2000];

  sprintf(base_match, " base_name=\"%s\"", basename_);

  char result_match[2000];

  char startscan[30];
  char endscan[30];

  // these are deprecated, since start scan and end scan
  // are no longer allowed to start with 0's
  if(1 || start_scan_ >= 1000)
    sprintf(startscan, "%d", start_scan_);
  else if(start_scan_ >= 100)
    sprintf(startscan, "0%d", start_scan_);
  else if(start_scan_ >= 10)
    sprintf(startscan, "00%d", start_scan_);
  else 
    sprintf(startscan, "000%d", start_scan_);
  if(1 || end_scan_ >= 1000)
    sprintf(endscan, "%d", end_scan_);
  else if(end_scan_ >= 100)
    sprintf(endscan, "0%d", end_scan_);
  else if(end_scan_ >= 10)
    sprintf(endscan, "00%d", end_scan_);
  else 
    sprintf(endscan, "000%d", end_scan_);

	    

  sprintf(result_match, "start_scan=\"%s\" end_scan=\"%s\"", startscan, endscan);

  char asap_match[] = "asapratio_timestamp";
  char xpress_match[] = "xpressratio_timestamp";
  char search_summ_match[] = "search_summary";
  char misc_run_match[1000];
  char impossible_match[] = "^^^^^^^^^^^^^";
  strcpy(misc_run_match, impossible_match);
#ifndef USE_STD_MODS
  char modification_match[] = "_modification";
  char aa_modification[4000];
  char term_modification[4000];
#endif
  char comet_md5_match[] = "parameter name=\"md5_check_sum\"";

  char *nextline = new char[line_width_];

  Boolean filterParams = False;
  Boolean quantHighBG = False;
  Boolean zeroBG = False;
  double mzBound = 0.5;
  double wavelet = false;

  for(int k = 0; k < xmlfiles_->length(); k++) {

    Boolean base_found = False;
    Boolean result_found = False;
    Boolean search_score_found = False;
  
    pwiz::util::random_access_compressed_ifstream fin((*xmlfiles_)[k]); // can read gzipped xml
    if(! fin) {
      cout << "SpectrumParser: error opening " << (*xmlfiles_)[k] << endl;
      exit(1);
    }

    while(fin.getline(nextline, line_width_)) {

      if(strstr(nextline, base_match) != NULL || strstr(nextline, "xpressratio_summary") != NULL ||  strstr(nextline, "analysis_summary") != NULL ||
	 strstr(nextline, "mzBound") != NULL || strstr(nextline, "zeroBG") != NULL || strstr(nextline, "quantHighBG") != NULL ||
	 (base_found && (strstr(nextline, result_match) != NULL || strstr(nextline, "analysis_") != NULL ||
			 strstr(nextline, asap_match) != NULL || strstr(nextline, "search_database") != NULL ||
			 strstr(nextline, search_summ_match) != NULL ||
			 strstr(nextline, misc_run_match) != NULL || 
#ifndef USE_STD_MODS
			 strstr(nextline, modification_match) != NULL ||
#endif
			 strstr(nextline, comet_md5_match) != NULL ||
			 strstr(nextline, "peptideprophet_timestamp") != NULL)) ||
	 result_found) {

	if(strstr(nextline, base_match) != NULL) {
	  base_found = True;
	}
	else if(base_found && strstr(nextline, result_match) != NULL)
	  result_found = True;
	

	analyze = True;

      }
      else
	analyze = False;

      if(analyze) {
	data = strstr(nextline, "<");
	while(data != NULL) {
	  Tag *tag = new Tag(data);
#define DELNULL(t) {delete t;t=NULL;}
	  if(tag != NULL) {

	    if(tag->isStart() && ! strcmp(tag->getName(), "xpressratio_summary")) {
	      comp_data.xpresslight = atoi(tag->getAttributeValue("xpress_light"));
	    }
	    else if(base_found && tag->isStart() && ! strcmp(tag->getName(), "search_database")) {
	      char* next = new char[strlen(tag->getAttributeValue("local_path"))+1];
	      strcpy(next, tag->getAttributeValue("local_path"));	      
	      databases_->insertAtEnd(next);
	    }
	    else if(base_found && tag->isStart() && ! strcmp(tag->getName(), "search_summary")) {
	      int masstype = ! strcmp(tag->getAttributeValue("precursor_mass_type"), "monoisotopic") ? 1 : 0;
	      int fragmasstype = ! strcmp(tag->getAttributeValue("fragment_mass_type"), "monoisotopic") ? 1 : 0;
	      if(masstype || fragmasstype)
		prodata_component_->setMassType(masstype, fragmasstype);
	      strcpy(search_engine, tag->getAttributeValue("search_engine"));
	      if(! strcasecmp(search_engine, "COMET"))
		strcpy(misc_run_match, comet_md5_match);
	      else
		strcpy(misc_run_match, impossible_match); // impossible to match
	      
	      strcpy(modOpts_.engine_, search_engine);
	      factory_ = new MixtureDistrFactory(modOpts_, scoreOpts_);
#ifndef USE_STD_MODS
	      // reset
	      aa_modification[0] = 0;
	      term_modification[0] = 0;
#endif
	    }
	    else if(base_found && tag->isStart() && ! strcmp(tag->getName(), "parameter") && ! strcmp(tag->getAttributeValue("name"), "md5_check_sum")) { // comet case
	      char* nextmisc = new char[strlen(tag->getAttributeValue("value"))+1];
	      strcpy(nextmisc, tag->getAttributeValue("value"));
	      misc_run_conditions_->insertAtEnd(nextmisc);
	    }
#ifndef USE_STD_MODS
	    else if(base_found && tag->isStart() && ! strcmp(tag->getName(), "aminoacid_modification")) {
	      strcat(aa_modification, tag->getAttributeValue("aminoacid"));
	      if(tag->getAttributeValue("symbol") != NULL)
		strcat(aa_modification, tag->getAttributeValue("symbol"));
	      strcat(aa_modification, "-");
	      strcat(aa_modification, tag->getAttributeValue("mass"));
	      strcat(aa_modification, ":");
	    }
	    else if(base_found && tag->isStart() && ! strcmp(tag->getName(), "terminal_modification")) {
	      strcat(term_modification, tag->getAttributeValue("terminus"));
	      if(tag->getAttributeValue("symbol") != NULL)
		strcat(term_modification, tag->getAttributeValue("symbol"));
	      strcat(term_modification, "-");
	      strcat(term_modification, tag->getAttributeValue("mass"));
	      strcat(term_modification, ":");
	    }
	    else if(base_found && tag->isEnd() && ! strcmp(tag->getName(), "search_summary")) {
	      char* next_aamod = new char[strlen(aa_modification)+1];
	      strcpy(next_aamod, aa_modification);
	      aa_modifications_->insertAtEnd(next_aamod);
	      char* next_termmod = new char[strlen(term_modification)+1];
	      strcpy(next_termmod, term_modification);
	      term_modifications_->insertAtEnd(next_termmod);
	    }
#endif

	    else if(base_found && tag->isStart() && ! strcmp(tag->getName(), "analysis_timestamp") &&
		    ! strcmp(tag->getAttributeValue("analysis"), "peptideprophet")) {
	    //	    else if(base_found && tag->isStart() && ! strcmp(tag->getName(), "peptideprophet_timestamp")) {
	      char* next = new char[strlen(tag->getAttributeValue("time"))+1];
	      strcpy(next, tag->getAttributeValue("time"));
	      pepproph_timestamps_->insertAtEnd(next);
	    }
	    else if(base_found && tag->isStart() && ! strcmp(tag->getName(), "analysis_timestamp") &&
		    ! strcmp(tag->getAttributeValue("analysis"), "interprophet")) {
	    //	    else if(base_found && tag->isStart() && ! strcmp(tag->getName(), "peptideprophet_timestamp")) {
	      char* next = new char[strlen(tag->getAttributeValue("time"))+1];
	      strcpy(next, tag->getAttributeValue("time"));
	      iproph_timestamps_->insertAtEnd(next);
	    }
	    //	    else if(base_found && tag->isStart() && ! strcmp(tag->getName(), "asapratio_timestamp")) {
	    else if(tag->isStart() && //! strcmp(tag->getName(), "analysis_timestamp") &&
		    ! strcmp(tag->getName(), "analysis_summary") &&
		    ! strcmp(tag->getAttributeValue("analysis"), "asapratio")) {
	      char* next = new char[strlen(tag->getAttributeValue("time"))+1];
	      strcpy(next, tag->getAttributeValue("time"));
	      asap_timestamps_->insertAtEnd(next);
	      quantHighBG = False;
	      zeroBG = False;
	      mzBound = 0.5;
	      wavelet = false;
	      filterParams = True;
	    }

	    if (filterParams && ! strcmp(tag->getName(), "parameter")) {
	      if (! strcmp(tag->getAttributeValue("name"), "quantHighBG") && 
		  ! strcmp(tag->getAttributeValue("value"), "True")) {
		quantHighBG = True;
	      }
	      else if (! strcmp(tag->getAttributeValue("name"), "zeroBG") && 
		       ! strcmp(tag->getAttributeValue("value"), "True")) {
		zeroBG = True;
	      }
	      else if (! strcmp(tag->getAttributeValue("name"), "wavelet") && 
		       ! strcmp(tag->getAttributeValue("value"), "True")) {
		wavelet = true;
	      }
	      else if (! strcmp(tag->getAttributeValue("name"), "mzBound")) {
		mzBound = atof(tag->getAttributeValue("value"));
		if (mzBound <= 0 || mzBound >= 1) {
		  mzBound = 0.5;
		}
	      }
	      
	    }
	    if (filterParams && tag->isEnd() && ! strcmp(tag->getName(), "analysis_summary")) {
	      filterParams = False;
	      asap_quantHighBGs_->insertAtEnd(quantHighBG);
	      asap_zeroBGs_->insertAtEnd(zeroBG);
	      asap_wavelets_->insertAtEnd(wavelet);
	      asap_mzBounds_->insertAtEnd(mzBound);
	    }


	    if(base_found && tag->isStart() && ! strcmp(tag->getName(), "msms_run_summary")) {
	      char* next = new char[strlen(tag->getAttributeValue("base_name"))+1];
	      strcpy(next, tag->getAttributeValue("base_name"));
	      basenames_->insertAtEnd(next);
	    }
	    else if(result_found) {
	      if(tag->isStart() && ! strcmp(tag->getName(), "spectrum_query")) {

	      precursor_charge = atoi(tag->getAttributeValue("assumed_charge"));
	      scan = (long)(atoi(tag->getAttributeValue("start_scan")));
	      index = atoi(tag->getAttributeValue("index"));
	      search_result_tags = new Array<Tag*>;
	      search_result_tags->insertAtEnd(tag);
	      comp_data.asapratio_index = atoi(tag->getAttributeValue("index"));
         tag = NULL; // don't delete, we've saved it
	      }
	      else if(tag->isStart() && ! strcmp(tag->getName(), "asapratio_result")) {
		comp_data.asap_mean = atof(tag->getAttributeValue("mean"));
		comp_data.asap_err = atof(tag->getAttributeValue("error"));
	      }
	      else if(tag->isStart() && ! strcmp(tag->getName(), "peptideprophet_result")) {
		probability = atof(tag->getAttributeValue("probability"));
		if(tag->getAttributeValue("analysis") != NULL) {
		  adjusted_prob = ! strcmp(tag->getAttributeValue("analysis"), "adjusted");
		  incomplete_prob = ! strcmp(tag->getAttributeValue("analysis"), "incomplete");
		}
		else {
		  adjusted_prob = False;
		  incomplete_prob = False;
		}
		DELNULL( tag );
	      }
	      else if(tag->isStart() && ! strcmp(tag->getName(), "search_score_summary")) {
		search_score_found = True;
	      }
	      else if(search_score_found && tag->isStart() && ! strcmp(tag->getName(), "parameter")) {
		if(strlen(comp_data.score_summary) == 0) { // first one
		  strcpy(comp_data.score_summary, tag->getAttributeValue("name"));
		}
		else {
		  strcat(comp_data.score_summary, " ");
		  strcat(comp_data.score_summary, tag->getAttributeValue("name"));
		}
		strcat(comp_data.score_summary, ":");
		strcat(comp_data.score_summary, tag->getAttributeValue("value"));

	      }
	      else if(search_score_found && tag->isEnd() && ! strcmp(tag->getName(), "search_score_summary")) 
		search_score_found = False;

	      else if(tag->isStart() && ! strcmp(tag->getName(), "xpressratio_result")) {
		strcpy(comp_data.xpressratio, tag->getAttributeValue("ratio"));
		comp_data.lightfirstscan = atoi(tag->getAttributeValue("light_firstscan"));
		comp_data.lightlastscan = atoi(tag->getAttributeValue("light_lastscan"));
		comp_data.heavyfirstscan = atoi(tag->getAttributeValue("heavy_firstscan"));
		comp_data.heavylastscan = atoi(tag->getAttributeValue("heavy_lastscan"));
		comp_data.lightmass = atof(tag->getAttributeValue("light_mass"));
		comp_data.heavymass = atof(tag->getAttributeValue("heavy_mass"));
		comp_data.masstol = atof(tag->getAttributeValue("mass_tol"));
		DELNULL ( tag );
	      }
	      else {
		if(tag->isEnd() && ! strcmp(tag->getName(), "search_hit")) {

		  // here must process everything and reset
		  //nextresult = getSearchResult(search_result_tags, search_engine);
		  if (factory_ != NULL)
		      nextresult = factory_->getSearchResultWithAppliedOpts(search_result_tags);

		  if(nextresult != NULL && asap_index >= 0) {
		    nextresult->probability_ = probability;
		    nextresult->adjusted_prob_ = adjusted_prob;
		    nextresult->incomplete_prob_ = incomplete_prob;

		    prodata_component_->enter(data_, nextresult, basenames_->length()-1, pepproph_timestamps_->length()-1, iproph_timestamps_->length()-1, databases_->length()-1, asap_timestamps_->length()-1, comp_data, scan);



		    found_ = True;
		    if(search_result_tags != NULL) {
		      for(int k = 0; k < search_result_tags->length(); k++)
			if((*search_result_tags)[k] != NULL)
			  delete (*search_result_tags)[k];
		      delete search_result_tags;
		      search_result_tags = NULL;
		    }
		    fin.close();
          delete[] nextline;
		    return; 
		  }
		  else {
		    cout << "error" << endl;
		    if(nextresult == NULL)
		      cout << "null result" << endl;
		    if(asap_index == 0)
		      cout << "0 index" << endl;
		    exit(1);
		  }

		}
		else {
		  search_result_tags->insertAtEnd(tag);
		  tag = NULL; // don't delete, we've saved it
		}
	      } // if process (but not collection)
	    } // if result found

	  } //  if not null
	  data = strstr(data+1, "<");
     DELNULL(tag);
	} // next tag
      } // if analyze

    } // next line
    //cout << "closing " << (*xmlfiles_)[k] << endl;
    fin.close();
  } // next inputfile
  delete [] nextline;
  if(search_result_tags != NULL) {
     for(int k = 0; k < search_result_tags->length(); k++) {
        if((*search_result_tags)[k] != NULL) {
           delete (*search_result_tags)[k];
        }
     }
	  delete search_result_tags;
  }
}



SearchResult* SpectrumParser::getSearchResult(Array<Tag*>* tags, char* engine) {
  if(! strcasecmp(engine, "SEQUEST"))
    return new SequestResult(tags);
  if(! strcasecmp(engine, "Mascot"))
    return new MascotResult(tags);
  if(! strcasecmp(engine, "COMET"))
    return new CometResult(tags);

  return NULL;
}

Boolean SpectrumParser::found() {
  return found_;
}

void SpectrumParser::write(ostream& os) {
  if(prodata_component_ != NULL)
#ifdef USE_STD_MODS
    prodata_component_->writeStandardFormat(os, xmlfiles_, basenames_, pepproph_timestamps_, 
					    iproph_timestamps_, databases_, asap_timestamps_, 
					    asap_quantHighBGs_, asap_zeroBGs_, asap_mzBounds_, asap_wavelets_, 
					    True, misc_run_conditions_, NULL);
#endif
#ifndef USE_STD_MODS
    prodata_component_->writeStandardFormat(os, xmlfiles_, basenames_, pepproph_timestamps_, iproph_timestamps_, 
					    databases_, asap_timestamps_, asap_quantHighBGs_, asap_zeroBGs_, 
					    asap_mzBounds_, asap_wavelets_, True, aa_modifications_, term_modifications_, misc_run_conditions_);
#endif
}

void SpectrumParser::setFilter(Tag* tag) {
  if(tag == NULL)
    return;

  if(filter_memory_) {
    filter_memory_ = False;
    filter_ = False;
  }

  if(! strcmp(tag->getName(), "search_hit")) {
    if(tag->isStart() && ! strcmp(tag->getAttributeValue("hit_rank"), "1")) {
      //tag->print();
      filter_ = True;
    }else{
      if(filter_ && tag->isEnd())
        filter_memory_ = True;
    }
  }

}


