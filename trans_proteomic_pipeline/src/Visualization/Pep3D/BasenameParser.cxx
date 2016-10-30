#include "BasenameParser.h"
#include "mzParser.h"

BasenameParser::BasenameParser(const char* xmlfile) : Parser(NULL) {
  // default settings
  mzXmlfiles_ = new Array<char*>;

  init(xmlfile);
}

void BasenameParser::parse(const char* xmlfile) {
  Array<Tag*>* tags = NULL;
  Tag* tag = NULL;

  char run_summary_tag[] = "msms_run_summary";
  char* data = NULL;

  pwiz::util::random_access_compressed_ifstream fin(xmlfile); // can read gzipped xml
  if(! fin) {
    cerr << "BasenameParser: error opening " << xmlfile << endl;
    exit(1);
  }
  char *nextline = new char[line_width_];
  while(fin.getline(nextline, line_width_)) {
    //cout << "next: " << nextline << endl;
    
    if(strstr(nextline, run_summary_tag) != NULL) {

      data = strstr(nextline, "<");
      while(data != NULL) {
	tag = new Tag(data);
	//setFilter(tag);
	//tag->write(cout);
	if(tag->isStart() && ! strcmp(tag->getName(), "msms_run_summary")) {
     int len;

	  char* next = new char[len = (int)strlen(tag->getAttributeValue("base_name")) + 11];

     rampConstructInputFileName(next,len,tag->getAttributeValue("base_name"));

	  mzXmlfiles_->insertAtEnd(next);

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


void BasenameParser::setFilter(Tag* tag) {
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

Array<char*>* BasenameParser::getMzXMLFiles() {
  Array<char*>* output = new Array<char*>;
  for(int k = 0; k < mzXmlfiles_->length(); k++) {
    char* next = new char[strlen((*mzXmlfiles_)[k])+1];
    strcpy(next, (*mzXmlfiles_)[k]);
    output->insertAtEnd(next);

    //output[k] = new char[strlen((*mzXmlfiles_)[k])+1];
    //strcpy(output[k], (*mzXmlfiles_)[k]);
  }
  //output[mzXmlfiles_->length()+1] = new char[1];
  //strcpy(output[mzXmlfiles_->length()+1], "");

  return output;
}
