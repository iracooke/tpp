#ifndef PEP3D_PARSER_H
#define PEP3D_PARSER_H

#include <stdio.h>
#include <math.h>
#include <time.h>

#include "Parsers/Parser/Parser.h"
#include "Parsers/Parser/TagFilter.h"

#include "common/constants.h"



class Pep3DParser : public Parser {

 public:

  Pep3DParser(const char* xmlfile, const char** mzXmlfiles, int numfiles);
  void setFilter(Tag* tag);
  int getNumDataEntries();
  Pep3D_dataStrct* getPep3DDataStrct();
  ~Pep3DParser();

 protected:

  void parse(const char* xmlfile);
  Boolean mzXmlfileOnList(const char* mzxmlfile);

  char mzXMLfile_[500];

  Array<const char*>* mzXmlfiles_;
  Array<Pep3D_dataStrct>* data_;

};




#endif
