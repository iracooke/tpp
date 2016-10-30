#include "PeptideProphetParser.h"

/*

Program       : PeptideProphet                                                       
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

// regression test stuff - bpratt Insilicos LLC, Nov 2005
#include "Parsers/Parser/TagListComparator.h" 
class PeptideProphetTagListComparator : public TagListComparator {
public:
   PeptideProphetTagListComparator(const char *progname,eTagListFilePurpose why,const Array<Tag*> &lhs,const char *rhsFilename) :
   TagListComparator(progname,why,lhs,rhsFilename) { // read from file then compare
   }

   int unacceptable_difference(const char *attribute, // return 1 if these values are too different for this attribute
                                                                const char *value1, const char *value2) {
      if (TagListComparator::unacceptable_difference(attribute,value1,value2)) {
         // check for local exceptions
         if ((!strcmp(attribute,"num_corr")) ||
             (!strcmp(attribute,"num_incorr")))
         { // these get written at 0 digits precision
            double r1=atof(value1);
            double r2=atof(value2);
            if (fabs(r2-r1) <= 1) {
               return 0; // within a reasonable tolerance
            }
         } 
         return 1; // bad match
      }
      return 0; // OK
   }
};

PeptideProphetParser::PeptideProphetParser() : Parser("peptideprophet") { 
   testMode_ = NULL;
}

PeptideProphetParser::PeptideProphetParser(const char* xmlfile, const char* options, const char *testMode) : Parser("peptideprophet") {
  model_ = NULL;
  testMode_ = testMode?strdup(testMode):NULL; // regression test stuff - bpratt Insilicos LLC, Nov 2005

  //cout << "|" << options << "|" << endl;

  modelOpts_.massspec_ = new InstrumentStruct();
  scoreOpts_.inputfile_[0] = 0;

  // default settings
  modelOpts_.icat_ = ICAT_UNKNOWN;
  modelOpts_.glyc_ = False;
  modelOpts_.phospho_ = False;
  modelOpts_.minprob_ = 0.0; //0.05;
  modelOpts_.extraitrs_ = EXTRAITRS;
  modelOpts_.no_neg_init_ = False;
  modelOpts_.no_ntt_ = False;
  modelOpts_.no_nmc_ = False;
  modelOpts_.use_decoy_ = False;
  modelOpts_.output_decoy_probs_ = False;
  modelOpts_.pI_ = False;
  modelOpts_.accMass_ = False;
  modelOpts_.ppm_ = False;
  modelOpts_.conservative_ = 0;
  modelOpts_.no_mass_ = False;
  modelOpts_.min_pI_ntt_ = 2;
  modelOpts_.min_pI_prob_ = 0.9;
  modelOpts_.RT_ = False;
  modelOpts_.rtcat_file_ = False;
  modelOpts_.min_RT_ntt_ = 2;
  modelOpts_.min_RT_prob_ = 0.9;
  modelOpts_.instrwarn_ = False;
  modelOpts_.minprob_ = 0.05;
  modelOpts_.enzyme_[0] = 0;
  modelOpts_.engine_[0] = 0;
  modelOpts_.decoy_label_[0] = 0;
  modelOpts_.use_expect_ = False;
  modelOpts_.nonparam_ = False;

  // HENRY: ICAT type for SpectraST
  modelOpts_.spectrast_icat_ = 0;
  // modelOpts_.spectrast_delta_ = false;
  modelOpts_.multiply_by_spectrast_lib_probs_ = True;
  modelOpts_.optimize_fval_ = false;
  // END HENRY

  for (int i=0; i<MAX_CHARGE; i++) {
    modelOpts_.use_chg_[i] = True;
  }

  // default settings here
  scoreOpts_.deltastar_ = DELTACN_ZERO; //DELTACN_EXCLUDE; //ZERO;
  scoreOpts_.mascotstar_ = MASCOTSTAR_PENALIZE; //MASCOTSTAR_LEAE; //ZERO;
  scoreOpts_.cometstar_ = COMETSTAR_ZERO;

  if(strstr(options, " EXCLUDE ") != NULL) {
    scoreOpts_.deltastar_ = DELTACN_EXCLUDE; //ZERO;
    scoreOpts_.mascotstar_ = MASCOTSTAR_EXCLUDE; // MASCOTSTAR_PENALIZE; //MASCOTSTAR_LEAVE; //ZERO;
    scoreOpts_.cometstar_ = COMETSTAR_EXCLUDE; //ZERO;
  }
  else if(strstr(options, " LEAVE ") != NULL) {
    scoreOpts_.deltastar_ = DELTACN_LEAVE; //ZERO;
    scoreOpts_.mascotstar_ = MASCOTSTAR_LEAVE; //ZERO;
    scoreOpts_.cometstar_ = COMETSTAR_LEAVE; //ZERO;
  }
  // HENRY
  if(strstr(options, " PERFECTLIB ") != NULL) {
    modelOpts_.multiply_by_spectrast_lib_probs_ = False;
  }
  // if (strstr(options, " DELTA ") != NULL) {
  //   modelOpts_.spectrast_delta_ = true;
  // }
  if(strstr(options, " OPTIMIZEFVAL ") != NULL) {
    cout << "Optimize F-value function by PCA." << endl;
    modelOpts_.optimize_fval_ = True;
  } 
  // END HENRY

  if(strstr(options, " ICAT ") != NULL) {
    modelOpts_.icat_ = ICAT_ON;
  }
  else if(strstr(options, " NOICAT ") != NULL) {
    modelOpts_.icat_ = ICAT_OFF;
  }
  if(strstr(options, " ZERO ") != NULL) {
    modelOpts_.minprob_ = 0.0;
  }

  if(strstr(options, " ACCMASS ") != NULL) {
     cout << "using Accurate Mass Bins" << std::endl;
    modelOpts_.accMass_ = True;
  }

  if(strstr(options, " CLEVEL=") != NULL) { // extract out
    const char* matchresult = strstr(options, "CLEVEL=");
    sscanf(matchresult + strlen("CLEVEL="), "%f", &modelOpts_.conservative_);
  }

  if(strstr(options, " PPM ") != NULL) {
     cout << "using PPM mass difference" << std::endl;
    modelOpts_.ppm_ = True;
  }

  if(strstr(options, " NOMASS ") != NULL) {
     cout << "Not using Mass Difference Model" << std::endl;
    modelOpts_.no_mass_ = True;
  }

  if(strstr(options, " PI ") != NULL) {
    cout << "using pI" << endl;
    modelOpts_.pI_ = True;
  }

  if(strstr(options, " RT ") != NULL) {
    cout << "using RT" << endl;
    modelOpts_.RT_ = True;
  }

  if(strstr(options, " GLYC ") != NULL) {
    cout << "using N-glyc info" << endl;
    modelOpts_.glyc_ = True;
  }

  if(strstr(options, " PHOSPHO ") != NULL) {
    cout << "using phospho info" << endl;
    modelOpts_.phospho_ = True;
  }

  if(strstr(options, " MALDI ") != NULL) {
    cout << "maldi mode" << endl;
    modelOpts_.maldi_ = True;
  }
  if(strstr(options, " INSTRWARN ") != NULL) {
    cout << "Using no error on different instrument types." << endl;
    modelOpts_.instrwarn_ = True;
  }
  if(strstr(options, " MINPROB=") != NULL) { // extract out
    const char* matchresult = strstr(options, "MINPROB=");
    sscanf(matchresult + strlen("MINPROB="), "%lf", &modelOpts_.minprob_);
  }

  if(strstr(options, " DECOY=") != NULL) { // extract out
    const char* matchresult = strstr(options, "DECOY=");
    sscanf(matchresult + strlen("DECOY="), "%s", modelOpts_.decoy_label_);
    cout << "Using Decoy Label \"" << modelOpts_.decoy_label_ << "\"." << endl;
    modelOpts_.use_decoy_ = True;
  }

  if(strstr(options, " RTCAT=") != NULL) { // extract out
    const char* matchresult = strstr(options, "RTCAT=");
    modelOpts_.rtcat_file_ = new char[strlen(matchresult)+10];
    sscanf(matchresult + strlen("RTCAT="), "%s", modelOpts_.rtcat_file_);
    cout << "Using RTCatalog \"" << modelOpts_.rtcat_file_ << "\"." << endl;
    modelOpts_.RT_ = True;
  }


  if(modelOpts_.use_decoy_  && strstr(options, " DECOYPROBS ") != NULL) { // extract out
    modelOpts_.output_decoy_probs_ = True;
    cout << "Decoy Probabilities will be reported." << endl;
  }

  if(strstr(options, " EXTRAITRS=") != NULL) { // extract out
    const char* matchresult = strstr(options, "EXTRAITRS=");
    sscanf(matchresult + strlen("EXTRAITRS="), "%d", &modelOpts_.extraitrs_);
  }
  const char *ign = options;
  while(strstr(ign, " IGNORECHG=") != NULL) { // extract out
    const char* matchresult = strstr(ign, "IGNORECHG=");
    int tmp = -1;
    sscanf(matchresult + strlen("IGNORECHG="), "%d", &tmp);
    ign = matchresult + strlen("IGNORECHG=");
    if (tmp < 1 || tmp > MAX_CHARGE) {
      cerr << "WARNING: IGNORECHG=" << tmp 
	   << " option specifies an invalid charge ... skipping ... " << endl;
    }
    else {
      cout << "Ignoring charge " << tmp << "+ spectra." << endl;
      modelOpts_.use_chg_[tmp-1] = False;
    }
  }
  if(strstr(options, " MINPINTT=") != NULL) { // extract out
    const char* matchresult = strstr(options, "MINPINTT=");
    sscanf(matchresult + strlen("MINPINTT="), "%d", &modelOpts_.min_pI_ntt_);
  }
  if(strstr(options, " MINPIPROB=") != NULL) { // extract out
    const char* matchresult = strstr(options, "MINPIPROB=");
    sscanf(matchresult + strlen("MINPIPROB="), "%lf", &modelOpts_.min_pI_prob_);
  }
  if(strstr(options, " MINRTNTT=") != NULL) { // extract out
    const char* matchresult = strstr(options, "MINRTNTT=");
    sscanf(matchresult + strlen("MINRTNTT="), "%d", &modelOpts_.min_RT_ntt_);
  }
  if(strstr(options, " MINRTPROB=") != NULL) { // extract out
    const char* matchresult = strstr(options, "MINRTPROB=");
    sscanf(matchresult + strlen("MINRTPROB="), "%lf", &modelOpts_.min_RT_prob_);
  }
  if(strstr(options, " NONEGINIT ") != NULL) {
    cout << "Using hardcoded values to initialize the distributions." << endl;
    modelOpts_.no_neg_init_ = True;
  }
  if(strstr(options, " NONTT ") != NULL) {
    cout << "Not using ntt model" << endl;
    modelOpts_.no_ntt_ = True;
  }
  if(strstr(options, " NONMC ") != NULL) {
    cout << "Not using nmc model" << endl;
    modelOpts_.no_nmc_ = True;
  }
  if(strstr(options, " FORCEDISTR ") != NULL) {
    cout << "Forcing output of mixture model" << endl;
    modelOpts_.forcedistr_ = True;
  }
  if(strstr(options, " NEGGAMMA ") != NULL) {
    cout << "Using Gamma Distribution to model the negative hits." << endl;
    modelOpts_.neg_gamma_ = True;
  }
  if(strstr(options, " EXPECTSCORE ") != NULL) {
   
    scoreOpts_.mascotstar_ = MASCOT_EXPECT;
    modelOpts_.use_expect_ = True;
  }
  if(strstr(options, " NONPARAM ") != NULL) {
    if (modelOpts_.use_decoy_) {
      cout << "Using non-parametric distributions" << endl;
      modelOpts_.nonparam_ = True;
    }
    else {
      cout << "WARNING: Cannot non-parametric distributions without decoys!" 
	   << endl;
      modelOpts_.nonparam_ = False;
    }
  }

  strcpy(scoreOpts_.inputfile_, xmlfile);
  options_ = new char[strlen(options)+1];
  strcpy(options_, options);

  icat_ = False;
  force_ = False;


  init(xmlfile);
}

PeptideProphetParser::~PeptideProphetParser() {
   delete [] options_;
   delete modelOpts_.massspec_;
   delete model_;
   free(testMode_);
}

void PeptideProphetParser::parse(const char* xmlfile) {
  char* engine = NULL;
  char* enzyme = NULL;
  Array<Tag*>* tags = NULL;
  Tag* tag = NULL;

  //  int line_width = 10000;
  char *nextline = new char[line_width_];
  char* data = NULL;
  Array<char*>* inputfiles = new Array<char*>;


  //
  // regression test stuff - bpratt Insilicos LLC, Nov 2005
  //
  eTagListFilePurpose testType;
  Array<Tag *> test_tags;
  checkRegressionTestArgs(testMode_,testType);
  char *testFileName = NULL;
  if (testType!=NO_TEST) {
     testFileName = constructTagListFilename(xmlfile, // input file
        testMode_, // program args
        "peptideProphet", // program name
        testType); // user info output
  }
#define RECORD(tag) {(tag)->write(fout);if (testType!=NO_TEST) {test_tags.insertAtEnd(new Tag(*(tag)));}}

  Boolean exclude_negative_probs = False; // whether or not to remove -1,-2,and -3 probability results from output

  double MIN_PROB =  modelOpts_.minprob_; //0.0; //0.05; // for now

  // initialize InstrumentStruct
  char instrument_default[] = "UNKNOWN";
  char manufacturer[] = "msManufacturer";
  strcpy(modelOpts_.massspec_->manufacturer, instrument_default);
  char model[] = "msModel";
  strcpy(modelOpts_.massspec_->model, instrument_default);
  char ionisation[] = "msIonization";
  strcpy(modelOpts_.massspec_->ionisation, instrument_default);
  char analyzer[] = "msMassAnalyzer";
  strcpy(modelOpts_.massspec_->analyzer, instrument_default);
  char detector[] = "msDetector";
  strcpy(modelOpts_.massspec_->detector, instrument_default);


  // must do first run to determine whether icat or not, prior to initializing MixtureModel with options...
  Boolean assessPeptideProperties = modelOpts_.icat_ == ICAT_UNKNOWN;

  bool is_rejected = false;

  if(assessPeptideProperties) {
    pwiz::util::random_access_compressed_ifstream fin(xmlfile); // can read gzipped xml
    if(! fin) {
      cerr << "fin: error opening " << xmlfile << endl;
      exit(1);
    }
    int num_unmod = 0;
    int num_mod = 0;
    int num_neg = 0;
    int max_tot = 2000;
    int tot = 0;

#ifdef USE_STD_MODS
    double error = 0.5;
    double light_icat_masses[] = {545.23, 555.23}; // cleavable, not
    double heavy_icat_masses[] = {555.23, 575.34}; // old fashioned
#endif

    while(tot < max_tot && fin.getline(nextline, line_width_)) {
      data = strstr(nextline, "<");
      while(data != NULL) {
	tag = new Tag(data);

	//tag->write(cout);

#ifdef USE_STD_MODS
	if(tag->isStart() && ! strcmp(tag->getName(), "aminoacid_modification") && 
	   ! strcmp(tag->getAttributeValue("aminoacid"), "C")) {
	  double nextmass = atof(tag->getAttributeValue("mass"));
	  for(int j = 0; j < ICATMixtureDistr::num_icat_; j++) {

	    if(nextmass - ICATMixtureDistr::light_icat_masses_[j] <= error && 
	       ICATMixtureDistr::light_icat_masses_[j] - nextmass <= error) {
	      modelOpts_.icat_ = ICAT_ON;
	      tot = max_tot + 1;
	    }
	    else if(nextmass - ICATMixtureDistr::heavy_icat_masses_[j] <= error && 
		    ICATMixtureDistr::heavy_icat_masses_[j] - nextmass <= error) {
	      modelOpts_.icat_ = ICAT_ON;
	      tot = max_tot + 1;
	    }
	  } // next icat label
	} // if aminoacid mod to C
	else if(tag->isEnd() && ! strcmp(tag->getName(), "search_summary")) { // done
	  tot = max_tot + 1;
	}
#endif
#ifndef USE_STD_MODS
	if(tag->isStart() && ! strcmp(tag->getName(), "search_hit") && ! strcmp(tag->getAttributeValue("hit_rank"), "1") ) {
	  if (tag->getAttributeValue("is_rejected") != NULL && !strcmp(tag->getAttributeValue("is_rejected"), "1"))  {
	    is_rejected = true;
	  }
	  else {
	    is_rejected = false;
	  }
	  strcpy(nextpep, tag->getAttributeValue("peptide"));
	  for(int z = 0; z < strlen(nextpep); z++) {
	    if(nextpep[z] == 'C') {
	      if(z == strlen(nextpep) - 1 || nextpep[z+1] < 'A' || nextpep[z+1] > 'Z') // followed by any non-alpha
		num_mod++;
	      else
		num_unmod++;
	    }
	  } // next position in peptyide
	  if(strchr(nextpep, 'C') == NULL)
	    num_neg++;
	  tot++;
	} // if peptide
#endif
	if(tag != NULL) {
	  delete tag;
	  tag = NULL;
	}


	data = strstr(data+1, "<");
      } // next tag
    

    } // next line
    fin.close();

    double pct_mod = 0; 
    double pct_pos = 0; 

    if(num_mod + num_unmod > 0) {
      pct_mod = double(num_mod)/(num_mod + num_unmod);
      pct_pos = double(num_mod + num_unmod)/(num_mod + num_unmod + num_neg);
      if(pct_pos > 0.5 && pct_mod > 0)
	modelOpts_.icat_ = ICAT_ON; //icat_ = True;
      else if(pct_pos > 0.1 && pct_mod > 0.2 && pct_mod < 0.8)
	modelOpts_.icat_ = ICAT_ON; //icat_ = True;
      else
	modelOpts_.icat_ = ICAT_OFF; //icat_ = True;
    }
  } // if assess

  Boolean collected = False;
  pwiz::util::random_access_compressed_ifstream fin(xmlfile); // can read gzipped xml
  if(! fin) {
    cerr << "fin(2): error opening " << xmlfile << endl;
    exit(1);
  }
  int run_idx = -1; 
  string* run_name = NULL;
  int search_res = 0;
  bool protein_semi = false;
  bool refine = false;
  bool top_hit = false;
  while(fin.getline(nextline, line_width_)) {
    //    cout << "next: " << nextline << endl;
    
    data = strstr(nextline, "<");
    while(data != NULL) {
      tag = new Tag(data);
      setFilter(tag);
      //tag->write(cout);
      collected = False;    

      if(tag->isStart() && ! strcmp(tag->getName(), "spectrum_query") ) {
	top_hit = false;
      }


      if(!top_hit && tag->isStart() && ! strcmp(tag->getName(), "search_hit") && ! strcmp(tag->getAttributeValue("hit_rank"), "1") ) {
	top_hit = true;
	if (tag->getAttributeValue("is_rejected") != NULL && !strcmp(tag->getAttributeValue("is_rejected"), "1"))  {
	  is_rejected = true;
	}
	else {
	  is_rejected = false;
	}
      }
  
      if(tag->isStart() && ! strcmp(tag->getName(), "parameter") && !strcmp(tag->getAttributeValue("name"), "protein, cleavage semi") && !strcmp(tag->getAttributeValue("value"), "yes") ) {
	protein_semi = true;
    delete tag;
    tag = NULL;

      }
      else if(tag->isStart() && ! strcmp(tag->getName(), "parameter") && !strcmp(tag->getAttributeValue("name"), "refine") && !strcmp(tag->getAttributeValue("value"), "yes") ) {
	refine = true;
      }
      else if(!protein_semi && refine && tag->isStart() && ! strcmp(tag->getName(), "parameter") && 
	      !strcmp(tag->getAttributeValue("name"), "refine, cleavage semi") && !strcmp(tag->getAttributeValue("value"), "yes") ) {
	refine = true;
	cerr << "WARNING: Found semi-cleavage in refinement mode and semi-cleavage not used in the first pass. Turning off NTT model." << endl;
	modelOpts_.no_ntt_ = True;
      }
      else if(refine && tag->isStart() && ! strcmp(tag->getName(), "parameter") && 
	      !strcmp(tag->getAttributeValue("name"), "refine, unanticipated cleavage") && !strcmp(tag->getAttributeValue("value"), "yes") ) {
	refine = true;
	cerr << "WARNING: Found unanticipated cleavage in refinement mode.  Turning off NTT model." << endl;
	modelOpts_.no_ntt_ = True;
      }
      else if(tag->isStart() && ! strcmp(tag->getName(), "msms_run_summary")) {
	run_name = new string(tag->getAttributeValue("base_name"));
	const char* nextfield = tag->getAttributeValue(manufacturer);
	if(nextfield != NULL) {
	  if(strcmp(modelOpts_.massspec_->manufacturer, instrument_default)) {
	    if(strcmp(modelOpts_.massspec_->manufacturer, nextfield)) {
	      if (modelOpts_.instrwarn_) {
		cout << "WARNING: detected atleast two instrument manufacturers: " << modelOpts_.massspec_->manufacturer << " and " 
		     << nextfield << ": This could be a problem for learning an accurate model by PeptideProphet." << endl;
	      }
	      else {
		cout << "ERROR: two instrument manufacturers: " << modelOpts_.massspec_->manufacturer << " and " << nextfield << endl;
		exit(1);
	      }
	    }
	  }
	  else 
	    strcpy(modelOpts_.massspec_->manufacturer, nextfield);
	}
	nextfield = tag->getAttributeValue(model);
	if(nextfield != NULL) {
	  if(strcmp(modelOpts_.massspec_->model, instrument_default)) {
	    if(strcmp(modelOpts_.massspec_->model, nextfield)) {
	      if (modelOpts_.instrwarn_) {
		cout << "WARNING: detected atleast two instrument models: " << modelOpts_.massspec_->model << " and " 
		     << nextfield << ": This could be a problem for learning an accurate model by PeptideProphet." << endl;
	      }
	      else {
		cout << "ERROR: two instrument models: " << modelOpts_.massspec_->model << " and " << nextfield << endl;
		exit(1);
	      }
	    }
	  }
	  else
	    strcpy(modelOpts_.massspec_->model, nextfield);
	}
	nextfield = tag->getAttributeValue(ionisation);
	if(nextfield != NULL) {
	  if(strcmp(modelOpts_.massspec_->ionisation, instrument_default)) {
	    if(strcmp(modelOpts_.massspec_->ionisation, nextfield)) {
	      if (modelOpts_.instrwarn_) {
		cout << "WARNING: detected atleast two instrument ionisations: " << modelOpts_.massspec_->ionisation << " and " 
		     << nextfield << ": This could be a problem for learning an accurate model by PeptideProphet." << endl;
	      }
	      else {
		cout << "ERROR: two instrument ionisations: " << modelOpts_.massspec_->ionisation << " and " << nextfield << endl;
		exit(1);
	      }
	    }
	  }
	  else
	    strcpy(modelOpts_.massspec_->ionisation, nextfield);
	}
	nextfield = tag->getAttributeValue(analyzer);
	if(nextfield != NULL) {
	  if(strcmp(modelOpts_.massspec_->analyzer, instrument_default)) {
	    if(strcmp(modelOpts_.massspec_->analyzer, nextfield)) {
	      if (modelOpts_.instrwarn_) {
		cout << "WARNING: detected atleast two instrument analyzers: " << modelOpts_.massspec_->analyzer << " and " 
		     << nextfield << ": This could be a problem for learning an accurate model by PeptideProphet." << endl;
	      }
	      else {
		cout << "ERROR: two instrument analyzers: " << modelOpts_.massspec_->analyzer << " and " << nextfield << endl;
		exit(1);
	      }
	    }
	  }
	  else
	    strcpy(modelOpts_.massspec_->analyzer, nextfield);
	}
	nextfield = tag->getAttributeValue(detector);
	if(nextfield != NULL) {
	  if(strcmp(modelOpts_.massspec_->detector, instrument_default)) {
	    if(strcmp(modelOpts_.massspec_->detector, nextfield)) {
	      if (modelOpts_.instrwarn_) {
		cout << "WARNING: detected atleast two instrument detectors: " << modelOpts_.massspec_->detector << " and " 
		     << nextfield << ": This could be a problem for learning an accurate model by PeptideProphet." << endl;
	      }
	      else {
		cout << "ERROR: two instrument detectors: " << modelOpts_.massspec_->detector << " and " << nextfield << endl;
		exit(1);
	      }
	    }
	  }
	  else {
	    strcpy(modelOpts_.massspec_->detector, nextfield);
	  }
	}

	// store the input file
	char* nextfile = new char[strlen(tag->getAttributeValue("base_name"))+strlen(get_pepxml_dot_ext())+1]; //pad it for pepXML extension
	strcpy(nextfile, tag->getAttributeValue("base_name"));
	inputfiles->insertAtEnd(nextfile);
    delete tag;
    tag = NULL;
      } // msms_summary 
      else if(tag->isStart() && ! strcmp(tag->getName(), "search_summary")) {
	run_idx++;
        // Check for case of X!Tandem running COMET scoring.
	if(!strcasecmp("X!Comet", tag->getAttributeValue("search_engine")) ||
        !strcasecmp("X! Tandem (comet)", tag->getAttributeValue("search_engine")))
            tag->setAttributeValue("search_engine", "COMET");
	if(engine == NULL) {
	  //	  cout << "1a here and ready with " << tag->getAttributeValue("search_engine") << endl;
	  engine = new char[strlen(tag->getAttributeValue("search_engine"))+1];
	  strcpy(engine, tag->getAttributeValue("search_engine"));

	  displayOptions(engine);
	  strcpy(modelOpts_.engine_, engine);
	}
	else if(strcmp(engine, tag->getAttributeValue("search_engine"))) {
	    //error
	  cout << "two different search engines specified: " << engine << " and " << tag->getAttributeValue("search_engine") << endl;
	  exit(1);
	}

	// HENRY
	// Do not initialize model here -- too early. Need to wait for parameters to be parsed to get all modelOpts
	// Instead, it is initialized below right before model->enterData.
	// END HENRY
    delete tag;
    tag = NULL;
      } // if search_summary
      else if(tag->isStart() && ! strcmp(tag->getName(), "sample_enzyme")) {
	if(enzyme == NULL) {
	  enzyme = new char[strlen(tag->getAttributeValue("name"))+1];
	  strcpy(enzyme, tag->getAttributeValue("name"));
	  char* tmpEnzName = ProteolyticEnzyme::getStandardEnzymeName(enzyme);
	  strcpy(modelOpts_.enzyme_, tmpEnzName);
	  delete [] tmpEnzName;
	}
	else if(strcmp(enzyme, tag->getAttributeValue("name"))) {


	  // HENRY
          // Since I moved the initialize model code to later, one will never get model != NULL here
          // To imitate the behavior of setting nonspecific enzyme when more than one enzyme is named,
          // we first use ProteolyticEnzyme::getStandardEnzymeName to standardize the enzyme names 
	  // (for fear that the same enzyme is called different names), then do the comparison.
	  // If in fact we have more than one enzyme, we set modelOpts_.enzyme_ to "", essentially
	  // doing what the old code is doing.
          
          char* newEnzyme = new char[strlen(tag->getAttributeValue("name"))+1];
	  strcpy(newEnzyme, tag->getAttributeValue("name"));
	  char* tmpNewEnzName = ProteolyticEnzyme::getStandardEnzymeName(newEnzyme);
	  if (strcmp(tmpNewEnzName, modelOpts_.enzyme_) != 0) {
            enzyme[0] = '\0';             // set enzyme to empty string
	    strcpy(modelOpts_.enzyme_, enzyme); // set modelOpts_.enzyme_ to empty string
	  }
          delete[] newEnzyme;
	  delete[] tmpNewEnzName;

	  //	  if(model_ != NULL)
	  //  model_->setEnzyme(""); // make nonspecific
	  // else {
	  //   cerr << "mult enz's named, and no model" << endl;
	  //  //error
	  //  exit(1);
	  // }

	  // END HENRY
	}
    delete tag;
    tag = NULL;
      } // sample enzyme
      // HENRY - parse out search parameters to set ICAT
      else if(tag->isStart() && ! strcmp(tag->getName(), "parameter")) {
	if(engine && model && !strcasecmp(engine, "SPECTRAST")) {
	  if (!strcmp(tag->getAttributeValue("name"), "icat_type")) {
	    const char* tmpIcatType = tag->getAttributeValue("value");

	    if (!assessPeptideProperties && modelOpts_.icat_ == ICAT_OFF) {
	      modelOpts_.spectrast_icat_ = 0;
	    } else {
	      if (!strcmp(tmpIcatType, "cl")) {
		modelOpts_.spectrast_icat_ = 1;
	      } else if (!strcmp(tmpIcatType, "uc")) {
		modelOpts_.spectrast_icat_ = 2;
	      } else {
		modelOpts_.spectrast_icat_ = 0;
	      }
	    }
	  }
	}
    delete tag;
    tag = NULL;
      } // parameter -- SpectraST needs to parse out some search parameters
      // END HENRY
      else if(filter_) {
	if(tags == NULL)
	  tags = new Array<Tag*>;
	tags->insertAtEnd(tag);
    tag = NULL;
	collected = True;
      }
      else {
	if(tag != NULL) {
	  delete tag;
	  tag = NULL;
	}
      }

      if(filter_memory_) { // process </search_result>

	if (enzyme == NULL) {
	  // Unknown enzyme 
	  // Got here because the pepXML files don't even have the sample_enzyme element -- possible for pepXML files from other search engine
	  // Just make it nonspecific
	  enzyme = new char[1];
	  enzyme[0] = '\0';
	  modelOpts_.enzyme_[0] = '\0';

	}

	// HENRY - initialize model here, so that all modelOpts are set properly before initializing the model
	if(model_ == NULL && engine != NULL && enzyme != NULL) {
	  model_ = new MixtureModel(modelOpts_, scoreOpts_, 499);
	  
	  cout << "init with " << engine << " " << enzyme << " " << endl;
	}
	if(model_ == NULL) {
	  if (enzyme == NULL) cout << "ERROR: Unknown enzyme" << endl;
	  if (engine == NULL) cout << "ERROR: Unknown search engine" << endl;
	  cout << "ERROR: Cannot initialize model." << endl;
	  // error
	  exit(1);
	}
	// END HENRY

	if(model_ != NULL && tags != NULL && !is_rejected)
	  model_->enterData(tags, run_idx, run_name);


	if(tags != NULL) {
	  for(int k = 0; k < tags->size(); k++) {
	    if((*tags)[k] != NULL) {
	      delete (*tags)[k];
          (*tags)[k] = NULL;

	    }
	  }
	  delete tags;
	  tags = NULL;
	}

      }
      data = strstr(data+1, "<");
    } // next tag

  } // next line
  fin.close();




  cout << "MS Instrument info: Manufacturer: " << modelOpts_.massspec_->manufacturer << ", Model: " << modelOpts_.massspec_->model << ", Ionization: " << modelOpts_.massspec_->ionisation << ", Analyzer: " << modelOpts_.massspec_->analyzer << ", Detector: " << modelOpts_.massspec_->detector << endl << endl; 

  if(model_ != NULL) {
    model_->initResultsInOrder();
    model_->process();
    model_->setResultsInOrder();
    if (modelOpts_.pI_) { 
      char suffix[] = ".pIstats";
      char *pIfile =  new char[strlen(xmlfile)+strlen(suffix)+1];
      strcpy(pIfile, xmlfile);
      strcat(pIfile, suffix);
      ofstream fout(pIfile,ios::binary);
      if(! fout) {
	cerr << "cannot write output to file " << pIfile << endl;
	exit(1);
      }
      fout << "Min NTT Used to Compute Model: " <<  modelOpts_.min_pI_ntt_ << ", Min Prob Used to Compute Model: " << modelOpts_.min_pI_prob_ << endl;
      model_->write_pIstats(fout);
      fout.close();
      delete [] pIfile;
    }
    if (modelOpts_.RT_) { 
      char suffix[] = ".RTstats";
      char *RTfile =  new char[strlen(xmlfile)+strlen(suffix)+1];
      strcpy(RTfile, xmlfile);
      strcat(RTfile, suffix);
      ofstream fout(RTfile,ios::binary);
      if(! fout) {
	cerr << "cannot write output to file " << RTfile << endl;
	exit(1);
      }
      //      fout << "Min NTT Used to Compute Model: " <<  modelOpts_.min_pI_ntt_ << ", Min Prob Used to Compute Model: " << modelOpts_.min_pI_prob_ << endl;
      model_->write_RTstats(fout);
      fout.close();
      delete [] RTfile;
    }
    if (modelOpts_.RT_) { 
      char suffix[] = ".RTcoeff";
      char *RTfile =  new char[strlen(xmlfile)+strlen(suffix)+1];
      strcpy(RTfile, xmlfile);
      strcat(RTfile, suffix);
      ofstream fout(RTfile,ios::binary);
      if(! fout) {
	cerr << "cannot write output to file " << RTfile << endl;
	exit(1);
      }
      //      fout << "Min NTT Used to Compute Model: " <<  modelOpts_.min_pI_ntt_ << ", Min Prob Used to Compute Model: " << modelOpts_.min_pI_prob_ << endl;
      model_->write_RTcoeff(fout);
      fout.close();
      delete [] RTfile;
    }
  }



  // now write the results



//  Tag* timestamp = new Tag("peptideprophet_timestamp", True, True);
  // get time info
//  timestamp->setAttributeValue("time", time_);



  int index = 0;

  char nextspec[5000];
  double nextprob = -1.0;


  Array<Tag*>* unprocessed = new Array<Tag*>;
  unprocessed->insertAtEnd(new Tag("analysis_result", True, False));
  unprocessed->insertAtEnd(new Tag("peptideprophet_result", True, True));
  unprocessed->insertAtEnd(new Tag("analysis_result", False, True));
  (*unprocessed)[0]->setAttributeValue("analysis", "peptideprophet");
  (*unprocessed)[1]->setAttributeValue("probability", "0");
  (*unprocessed)[1]->setAttributeValue("all_ntt_prob", "(0,0,0)"); 
  (*unprocessed)[1]->setAttributeValue("analysis", "none"); 

  // construct a tmpfile name based on xmlfile
  std::string outfile = make_tmpfile_name(xmlfile);
  //cerr << "writing data to " << outfile << endl;
  ofstream fout(outfile.c_str(),ios::binary);
  if(! fout) {
    cerr << "cannot write output to file " << outfile << endl;
    exit(1);
  }


  if(tags != NULL) {
    delete tags;
    tags = NULL;
  }
  TagFilter* pep_proph_score_filter = new TagFilter("search_score_summary");


  TagFilter* summary_filter = new TagFilter("analysis_summary", 1);
  summary_filter->enterRequiredAttributeVal("analysis", getName());
  TagFilter* pepproph_filter = new TagFilter("analysis_result");
  pepproph_filter->enterRequiredAttributeVal("analysis", getName());
  TagFilter* pep_proph_time_filter = new TagFilter("analysis_timestamp");
  pep_proph_time_filter->enterRequiredAttributeVal("analysis", getName());


  int result_index = 1;
  char search_result[] = "spectrum_query";
  char attr_name[] = "index";
  int nextcharge = -1;

  Tag* last_tag = NULL;

  //  bool top_hit = false;

  pwiz::util::random_access_compressed_ifstream fin2(xmlfile); // can read gzipped xml
  if(! fin2) {
    cerr << "fin2: error opening " << xmlfile << endl;
    exit(1);
  }

  while(fin2.getline(nextline, line_width_)) {
    
    data = strstr(nextline, "<");
    while(data != NULL) {
      tag = new Tag(data);

      setFilter(tag);
      //tag->write(cout);

      if(tag->isStart() && ! strcmp(tag->getName(), "spectrum_query")) {
	top_hit = false;
      }
	 
      if(!top_hit && tag->isStart() && ! strcmp(tag->getName(), "search_hit") && ! strcmp(tag->getAttributeValue("hit_rank"), "1")) {
	top_hit = true;
	if (tag->getAttributeValue("is_rejected") != NULL && !strcmp(tag->getAttributeValue("is_rejected"), "1"))  {
	  is_rejected = true;
	}
	else {
	  is_rejected = false;
	}
      }
  
      if(tag->isStart() && ! strcmp(tag->getName(), "msms_pipeline_analysis")) {
	//char* nextfile = new char[strlen(xmlfile)+1];
        //strcpy(nextfile, xmlfile);
	//	inputfiles->insertAtEnd(nextfile);	     
	RECORD(tag);
	Array<Tag*>* summary_tags = model_->getModelSummaryTags(getName(), time_, inputfiles, MIN_PROB, options_);
	if(summary_tags != NULL) {
      // now write
	  for(int k = 0; k < summary_tags->size(); k++) 
	    if((*summary_tags)[k] != NULL) {
	      RECORD((*summary_tags)[k]);
	      delete (*summary_tags)[k];
	    }
	  delete summary_tags;
	}
	// done with inputfiles
	if(inputfiles != NULL) {
	  for(int k = 0; k < inputfiles->size(); k++)
	    if((*inputfiles)[k] != NULL)
	      delete [] (*inputfiles)[k];
	  delete inputfiles;
	}
    if(tag != NULL){
	  delete tag;
      tag = NULL;
    }
      }
      else if(tag->isStart() && ! strcmp(tag->getName(), "msms_run_summary")) {
	RECORD(tag);
    if(tag != NULL){
	  delete tag;
      tag = NULL;
    }
      }
      else if(tag->isEnd() && ! strcmp(tag->getName(), "search_summary")) {
	RECORD(tag);

	Tag* nexttag = new Tag("analysis_timestamp", True, True);
	nexttag->setAttributeValue("analysis", getName());
	nexttag->setAttributeValue("time", time_);
	nexttag->setAttributeValue("id", "1");
	RECORD(nexttag);
	delete nexttag;
    if(tag != NULL){
	  delete tag;
      tag = NULL;
    }
      }
      else if(filter_) {
	if(tags == NULL)
	  tags = new Array<Tag*>;
	if(! summary_filter->filter(tag) &&
	   ! pepproph_filter->filter(tag) &&
	   ! pep_proph_time_filter->filter(tag) &&
	   ! pep_proph_score_filter->filter(tag)) 
	  tags->insertAtEnd(tag);
	else {
        if (tag != NULL) {
	    delete tag;
	  tag = NULL;
        }
	}

	if(tag != NULL && tag->isStart() &&  !tag->isEnd() && ! strcmp(tag->getName(), "spectrum_query")) {
	  strcpy(nextspec, tag->getAttributeValue("spectrum"));
	  nextcharge = atoi(tag->getAttributeValue("assumed_charge"));
	}

      }
      else {
	if(! summary_filter->filter(tag) &&
	   ! pepproph_filter->filter(tag) &&
	   ! pep_proph_time_filter->filter(tag) &&
	   ! pep_proph_score_filter->filter(tag)) 
	  RECORD(tag);
    if(tag != NULL){
	  delete tag;
      tag = NULL;
    }
      }

      if(filter_memory_) { // process
	Boolean verbose = False; 


	if(model_->isOrderedSpectrum(nextspec, nextcharge, index)) { 
	  if(verbose)
	    cout << nextspec << " is ordered spectrum with index: " << index << endl;
	  nextprob = model_->getOrderedProb(index);
	  if(verbose)
	    cout << " and with prob: " << nextprob << endl;
	  if( (! exclude_negative_probs && nextprob < 0.0) || (nextprob >= MIN_PROB)) { // then print it

	    Array<Tag*>* prob_tags = model_->getOrderedProbTags(getName(), index); 
	    if(verbose && prob_tags == NULL) 
	      cout << "NULL orderedtags" << endl;

	    else if(0 && verbose) {
	      cout << "all ok" << endl;
	      for(int kk = 0; kk < prob_tags->size(); kk++) 
		if((*prob_tags)[kk] != NULL) {
		  (*prob_tags)[kk]->write(cout);
		}
	    }
	    if(verbose)
	      cout << "starting process..." << endl;

	    for(int k = 0; k < tags->size(); k++) 
	      if((*tags)[k] != NULL) {

		setTagValue((*tags)[k], search_result, attr_name, &result_index);

		if(verbose) {
		  cout << "tag: " << k+1 << endl;
		  (*tags)[k]->write(cout);
		  if(prob_tags == NULL)
		    cout << " with null prob tags" << endl;
		}

		if ( (*tags)[k]->isStart() && ! strcmp((*tags)[k]->getName(), "search_hit") ) {
		  
		  if (!strcmp((*tags)[k]->getAttributeValue("hit_rank"), "1")) {		      
		    top_hit = true;
		  }
		  else {
		    top_hit = false;

		  }
		  if ((*tags)[k]->getAttributeValue("is_rejected") != NULL && !strcmp((*tags)[k]->getAttributeValue("is_rejected"), "1"))  {
		    is_rejected = true;
		  }
		  else {
		    is_rejected = false;
		  }
		}

		if(verbose && (*tags)[k]->isEnd() && ! strcmp((*tags)[k]->getName(), "search_hit")) 
		  cout << " here we go..." << endl;
	
		if(top_hit  && !is_rejected && prob_tags != NULL && (*tags)[k]->isEnd() && ! strcmp((*tags)[k]->getName(), "search_hit")) { // write probs before...
		  if(verbose)
		    cout << " and here..." << endl;
		  for(int kk = 0; kk < prob_tags->size(); kk++) 
		    if((*prob_tags)[kk] != NULL) {
		      RECORD((*prob_tags)[kk]);
		      delete (*prob_tags)[kk];
		    }
		  delete prob_tags;
		  prob_tags = NULL;
		}
		RECORD((*tags)[k]);
		delete (*tags)[k];
	      }
	    delete tags;
	    tags = NULL;
	  } // if above next prob
	  //if (!is_rejected)
	    index++; // advance

	} // if next ordered
	else if (tags != NULL) {
	  nextprob = 0;
	  if(verbose)
	    cout << nextspec << " is NOT ordered spectrum with index: " << index << endl;
	  if(nextprob >= MIN_PROB) { // then print it
	    bool t_hit = false;
	    int k;
	    for(k = 0; k < tags->size(); k++) {

	      if( (*tags)[k] != NULL) {
		//(*tags)[k]->write(cout);
		if ( (*tags)[k]->isStart() && ! strcmp((*tags)[k]->getName(), "search_hit") && !strcmp((*tags)[k]->getAttributeValue("hit_rank"), "1")) {		      
		  t_hit = true;
		}
		else if ( (*tags)[k]->isStart() && ! strcmp((*tags)[k]->getName(), "search_hit") && strcmp((*tags)[k]->getAttributeValue("hit_rank"), "1")) {		      
		
		  t_hit = false;
		}

		setTagValue((*tags)[k], search_result, attr_name, &result_index);
		if(t_hit && (*tags)[k]->isEnd() && ! strcmp((*tags)[k]->getName(), "search_hit")) { // write probs before...
		  for(int l=0; l<unprocessed->size(); l++) {
		    RECORD((*unprocessed)[l]);
		  }
		}
		RECORD((*tags)[k]);
		if (t_hit && (*tags)[k]->isEnd() && ! strcmp((*tags)[k]->getName(), "search_hit")) {
		  delete (*tags)[k];
		  (*tags)[k] = NULL;
		  k++;
		  break; // short circuit the loop
		}
		delete (*tags)[k];
        (*tags)[k]=NULL;
	      }
	      
	    }
	    //Write cached tags
	    for(; k < tags->size(); k++) {
	      if( (*tags)[k] != NULL) {
		setTagValue((*tags)[k], search_result, attr_name, &result_index);
		RECORD((*tags)[k]);
		delete (*tags)[k];
		
	      }
	    }

	    delete tags;
	    tags = NULL;
	  } // print zero

	} // if not analyzed
	if(tags != NULL) {
	  for(int k = 0; k < tags->size(); k++)
	    if((*tags)[k] != NULL)
	      delete (*tags)[k];
	  delete tags;
	  tags = NULL;
	}

      } // if end of search result
      data = strstr(data+1, "<");
    } // next tag


  } // next line
  delete summary_filter;
  delete pepproph_filter;
  delete pep_proph_time_filter;
  delete pep_proph_score_filter;
  fin2.close();
  fout.close();

  if (testType!=NO_TEST) {
     //
     // regression test stuff - bpratt Insilicos LLC, Nov 2005
     //
     PeptideProphetTagListComparator("PeptideProphet",testType,test_tags,testFileName);
     delete[] testFileName;
     for(int k = test_tags.length(); k--;) {
        delete test_tags[k];
     }
  }
  
  if(! overwrite(xmlfile, outfile.c_str(), "</msms_pipeline_analysis>")) {
    cerr << "error: no peptideprophet data written to file " << xmlfile << endl;
  }
   for(int l=0; l<unprocessed->size(); l++) {
     delete (*unprocessed)[l];
   }
   delete unprocessed;
   delete[] nextline;
   delete[] enzyme;
}


void PeptideProphetParser::setFilter(Tag* tag) {
  if(tag == NULL)
    return;

  if(filter_memory_) {
    filter_memory_ = False;
    filter_ = False;
  }

  if(! strcmp(tag->getName(), "spectrum_query")) {
    if(tag->isStart()&&!tag->isEnd()) {
      //tag->print();
      filter_ = True;
    }
    else if(tag->isStart()&&tag->isEnd()) {
      //tag->print();
      filter_ = False;
      filter_memory_ = False;
    }
    else {
      filter_memory_ = True;
    }
  }


}

void PeptideProphetParser::displayOptions(const char* eng) {

  cout << " (" << eng << ")";

  if(modelOpts_.icat_ == ICAT_ON)
    cout << " (icat)";


  if(! strcasecmp(eng, "SEQUEST")) {
    if(scoreOpts_.deltastar_ == DELTACN_EXCLUDE)
      cout << " (excluding deltacn* entries)";
    else if(scoreOpts_.deltastar_ == DELTACN_LEAVE)
      cout << " (leaving deltacn* entries)";

    if (modelOpts_.use_expect_ == True) {
      cout << " (using Sequest's expectation score for modeling)" << endl;
    }

  }
  else if(! strcasecmp(eng, "MASCOT")) {
    if(scoreOpts_.mascotstar_ == MASCOTSTAR_EXCLUDE)
      cout << " (excluding ionscore* entries)";
    else if(scoreOpts_.mascotstar_ == MASCOTSTAR_LEAVE)
      cout << " (leaving ionscore* entries)";
    else if(scoreOpts_.mascotstar_ == MASCOT_EXPECT)
      cout << " (using Mascot expect score)";
  }
  else if(! strcasecmp(eng, "X! Tandem")) {
    if (modelOpts_.use_expect_ == True) {
      cout << " (using Tandem's expectation score for modeling)" << endl;
    }
  }
  else if(! strcasecmp(eng, "COMET")) {
    if(scoreOpts_.cometstar_ == COMETSTAR_EXCLUDE)
      cout << " (excluding delta* entries)";
    else if(scoreOpts_.cometstar_ == COMETSTAR_LEAVE)
      cout << " (leaving delta* entries)";

     if (modelOpts_.use_expect_ == True) {
      cout << " (using Comet's expectation score for modeling)" << endl;
    }
  }
  if(modelOpts_.minprob_ == 0.0)
    cout << " (minprob 0)";
  
  cout << endl;

}
