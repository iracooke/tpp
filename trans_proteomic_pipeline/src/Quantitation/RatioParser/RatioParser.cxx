#include "RatioParser.h"

RatioParser::RatioParser(const char* xmlfile) : Parser(NULL) { 
  init(xmlfile);
}

void RatioParser::parse(const char* xmlfile) {
  //open file and pass along
  char* data = NULL;
  Tag* tag;

  ifstream fin(xmlfile);
  if(! fin) {
    cerr << "error opening " << xmlfile << endl;
    exit(1);
  }


  Array<Tag*>* tags = NULL;
  Array<char*>* peps = NULL;
  double next_prot_prob = 2.0;
  double MIN_PROB = 0.2;
  Boolean done = False;
  Boolean old_summary = False;

  TagFilter* summary_filter = new TagFilter("ASAP_prot_analysis", 1);
  TagFilter* ratio_filter = new TagFilter("ASAPRatio");
  TagFilter* bofinfo_filter = new TagFilter("ASAP_Seq");
  Array<char*>* inputfiles = new Array<char*>;
  Array<char*>* peptide_boffiles = new Array<char*>;
  Array<char*>* orig_inputfiles = new Array<char*>;

  char *nextline = new char[line_width_];
  while(fin.getline(nextline, line_width_)) {
    data = strstr(nextline, "<");
    while(data != NULL) {
      tag = new Tag(data);
      setFilter(tag);

      // replace date for analysis info
      if(tag->isStart() && ! strcmp(tag->getName(), "ASAP_prot_analysis")) 
	old_summary = True;

      if(! old_summary && strcmp(tag->getName(), "ASAPRatio"))
	; //tag->print();

      if(! tag->isStart() && ! strcmp(tag->getName(), "ASAP_prot_analysis")) 
	old_summary = False;

      //if(! tag->isStart() || strcmp(tag->getName(), "ASAP_prot_analysis")) // filter out prev analysis
      //tag->print();
      //cout << "starting with " << tag->getName() << endl;

      // parse input file names (and infer peptide bof names accordingly)
      if(tag->isStart() && ! strcmp(tag->getName(), "protein_summary_header")) {
	char* files = tag->getAttributeValue("source_files_alt");
	// parse through
	int i = 0;
	int last_i;
	char* nextfile;
	while(i < strlen(files)) {
	  i++;
	  if(files[i] == '+' || i == strlen(files)) {
	    nextfile = new char[i - last_i + 1];
	    for(int z = last_i; z < i; z++)
	      nextfile[z-last_i] = files[z];
	    nextfile[i - last_i] = 0;
	    inputfiles->insertAtEnd(nextfile);
	    peptide_boffiles->insertAtEnd(getBofFile(nextfile));
	    orig_inputfiles->insertAtEnd(getOrigFile(nextfile));
	  }
	} // while
      } // if protein summary header
      if(tag->isStart() && ! strcmp(tag->getName(), "protein_summary")) {
	Tag* summary = new Tag("ASAP_prot_analysis", True, True);
	summary->setAttributeValue("date", "yesterday");
	summary->print();
	delete summary;
      }
      if(! done && tag->isStart() && ! strcmp(tag->getName(), "protein_group")) {
	char* next = tag->getAttributeValue("probability");
	done = (next == NULL || atof(next) < MIN_PROB);
      }
      if(! done && tag->isStart() && ! strcmp(tag->getName(), "protein")) {
	char* next = tag->getAttributeValue("probability");
	next_prot_prob = next == NULL ? 0.0 : atof(next);
      }

      // filter out entries below min prob, and exclude all previous ASAPRatio calculations
      if(! done && filter_ && next_prot_prob >= MIN_PROB) {
	// new protein
	//tag->print();

	//cout << "in here with " << tag->getName() << endl;

	if(tags == NULL)
	  tags = new Array<Tag*>;

	if(! ratio_filter->filter(tag) && ! bofinfo_filter->filter(tag))
	  tags->insertAtEnd(tag);

	if(peps == NULL)
	  peps = new Array<char*>;

	if(tag->isStart() && ! strcmp(tag->getName(), "peptide")) {
	  // add here the additional asap tags and subtags....
	  Tag* nextseqtag = new Tag("ASAP_Seq", True, False);
	  nextseqtag->setAttributeValue("status", "1");

	  Tag* nextpeaktag = new Tag("ASAP_Peak", True, False);
	  nextpeaktag->setAttributeValue("status", "0");
	  
	  Tag* nextdtatag = new Tag("ASAP_Dta", True, True);
	  nextdtatag->setAttributeValue("peptide_index", "500");
	  nextdtatag->setAttributeValue("include", "2");

	  tags->insertAtEnd(nextseqtag);
	  tags->insertAtEnd(nextpeaktag);
	  tags->insertAtEnd(nextdtatag);
	  tags->insertAtEnd(new Tag("ASAP_Peak", False, True));
	  tags->insertAtEnd(new Tag("ASAP_Seq", False, True));


	    
	  
	  peps->insertAtEnd(tag->getAttributeValue("peptide_sequence"));
	  //cout << (*peps)[peps->length()-1] << endl;
	}

	else if(filter_memory_ && tags != NULL && peps != NULL) {

	  //cout << "out there with " << tag->getName() << endl;

	  // add the asap pro
	  Tag* protag = new Tag("ASAPRatio", True, True);



	  protag->setAttributeValue("ratio_mean", "1.0");
	  protag->setAttributeValue("ratio_standard_dev", "1.0");
	  protag->setAttributeValue("ratio_number_peptides", "1.0");
	  protag->setAttributeValue("index", "2");
	  

	  //cout << "here" << endl; exit(1);
	  for(int k = 0; k < tags->length(); k++) {
	    (*tags)[k]->print();
	    if(k == 0) {
	      protag->print();
	      delete protag;
	    }
	  }
	  for(int k = 0; k < peps->length(); k++)
	    cout << (*peps)[k] << endl;

	  for(int k = 0; k < tags->length(); k++)
	    if((*tags)[k] != NULL)
	      delete (*tags)[k];

	  if(tags != NULL) {
	    delete tags;
	    tags = NULL;
	  }
	  if(peps != NULL) {
	    delete peps;
	    peps = NULL;
	  }
	}
      }
      else {
	// add after firset tag the additional asap_pro tag...
	//cout << "this tag: " << tag->getName() << endl;
	  if(! summary_filter->filter(tag))
	    tag->print();

	delete tag;
      }
      data = strstr(data+1, "<");
    }

  }
  fin.close();

  delete [] nextline;
}

void RatioParser::setFilter(Tag* tag) {
  if(tag == NULL)
    return;

  if(filter_memory_) {
    filter_memory_ = False;
    filter_ = False;
  }

  if(! strcmp(tag->getName(), "protein")) 
    if(tag->isStart()) {
      //tag->print();
      filter_ = True;
    }
    else
      filter_memory_ = True;


}

