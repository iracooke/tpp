#include "Pep3DParser.h"
#include "mzParser.h"

Pep3DParser::Pep3DParser(const char* xmlfile, const char** mzXmlfiles, int num_mzXmls) : Parser(NULL) {
  // default settings
  name_ = NULL;
  time_ = NULL;
  mzXmlfiles_ = new Array<const char*>;
  for(int k = 0; k < num_mzXmls; k++) {
    mzXmlfiles_->insertAtEnd(mzXmlfiles[k]);
    //    cout << (*mzXmlfiles_)[k] << endl;
  }

  //  int index = 0;
  //  while(strlen(mzXmlfiles[index]) > 0)
  //    mzXmlfiles_->insertAtEnd(mzXmlfiles[index++]);

  mzXMLfile_[0] = 0;

  data_ = new Array<Pep3D_dataStrct>;

  init(xmlfile);
}

Pep3DParser::~Pep3DParser() {}

void Pep3DParser::parse(const char* xmlfile) {
  Array<Tag*>* tags = NULL;
  Tag* tag = NULL;

  /*
  time_t now;
  time(&now);

  char* conversion = ctime(&now);
  char* cleaned = new char[strlen(conversion)];
  strncpy(cleaned, conversion, strlen(conversion)-1);
  cleaned[strlen(conversion)-1] = 0;
  */

  char* data = NULL;


  Boolean analyze = False;
  Boolean top_hit = False;

  char engine[500];
  Pep3D_dataStrct pep3data;
  pep3data.startScan = 0;
  pep3data.endScan = 0;
  pep3data.charge = 0;
  pep3data.prob = -4.0;
  pep3data.score = 0;
  
  char run_summary_tag[] = "msms_run_summary";

  pwiz::util::random_access_compressed_ifstream fin(xmlfile); // can read gzipped xml
  if(! fin) {
    cerr << "Pep3DParser: error opening " << xmlfile << endl;
    exit(1);
  }
  char *nextline = new char[line_width_];
  while(fin.getline(nextline, line_width_)) {
    if(analyze || strstr(nextline, run_summary_tag) != NULL) {
      data = strstr(nextline, "<");
      while(data != NULL) {
	tag = new Tag(data);
	//setFilter(tag);
	//tag->write(cout);
	if(tag->isStart() && ! strcmp(tag->getName(), "msms_run_summary")) {

     rampConstructInputPath(mzXMLfile_,sizeof(mzXMLfile_),xmlfile,
        tag->getAttributeValue("base_name")); 

	  //	  strcpy(engine, tag->getAttributeValue("search_engine"));
	  analyze = mzXmlfileOnList(mzXMLfile_);
	  
	}
	else if(analyze) {
	  if(tag->isStart() && ! strcmp("search_summary", tag->getName())) {
	    strcpy(engine, tag->getAttributeValue("search_engine"));
	  }

	  else if(tag->isStart() && ! strcmp("spectrum_query", tag->getName())) {
	    pep3data.startScan = atoi(tag->getAttributeValue("start_scan"));
	    pep3data.endScan = atoi(tag->getAttributeValue("end_scan"));
	    pep3data.charge = atoi(tag->getAttributeValue("assumed_charge"));
	  }
	  else if(tag->isStart() && ! strcmp("search_hit", tag->getName()) && 
		  ! strcmp("1", tag->getAttributeValue("hit_rank"))) {
	    top_hit = True;
	  }
	  else if(top_hit && tag->isStart() && ! strcmp("peptideprophet_result", tag->getName()))
	    pep3data.prob = atof(tag->getAttributeValue("probability"));
	  else if(top_hit && tag->isStart() && ! strcmp("search_score", tag->getName())) {
	    if(! strcasecmp(engine, "SEQUEST") && ! strcmp(tag->getAttributeValue("name"), "xcorr"))
	      pep3data.score = atof(tag->getAttributeValue("value"));
	    else if(! strcasecmp(engine, "MASCOT") && ! strcmp(tag->getAttributeValue("name"), "ionscore"))
	      pep3data.score = atof(tag->getAttributeValue("value"));
	  }
	  else if(top_hit && tag->isEnd() && ! strcmp("search_hit", tag->getName())) {
	    if(pep3data.startScan && pep3data.endScan && pep3data.charge)
	      data_->insertAtEnd(pep3data);
	    pep3data.startScan = 0;
	    pep3data.endScan = 0;
	    pep3data.charge = 0;
	    pep3data.prob = -4.0;
	    pep3data.score = 0;
	    top_hit = False;
	  }
	} // if analyze
	if(tag != NULL)
	  delete tag;

	data = strstr(data+1, "<");
      } // next tag
    
    } // if analyze
  } // next line
  fin.close();
  
  delete [] nextline;
}


void Pep3DParser::setFilter(Tag* tag) {
  if(tag == NULL)
    return;

  if(filter_memory_) {
    filter_memory_ = False;
    filter_ = False;
  }

  if(! strcmp(tag->getName(), "search_result")) {
    if(tag->isStart()) {
      //tag->print();
      filter_ = True;
    }else{
      filter_memory_ = True;
    }
  }


}

int Pep3DParser::getNumDataEntries() {
  if(data_ == NULL)
    return 0;
  return data_->length();
}

Pep3D_dataStrct* Pep3DParser::getPep3DDataStrct() {
  Pep3D_dataStrct* output = new Pep3D_dataStrct[data_->length()];
  for(int k = 0; k < data_->length(); k++)
    output[k] = (*data_)[k];

  return output;
}

Boolean Pep3DParser::mzXmlfileOnList(const char* mzxmlfile) {
  for(int k = 0; k < mzXmlfiles_->length(); k++) {
    //cout << mzxmlfile << " vs " << (*mzXmlfiles_)[k] << endl;
    if(! strcmp((*mzXmlfiles_)[k], mzxmlfile))
      return True;
  }
  return False;

}
