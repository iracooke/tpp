#ifndef BASEN_PARSER_H
#define BASEN_PARSER_H

#include <stdio.h>
#include <math.h>
#include <time.h>

#include "Parsers/Parser/Parser.h"
#include "Parsers/Parser/TagFilter.h"

#include "common/constants.h"



class BasenameParser : public Parser {

 public:

  BasenameParser(const char* xmlfile);
  void setFilter(Tag* tag);
  Array<char*>* getMzXMLFiles();

 protected:

  void parse(const char* xmlfile);

  Array<char*>* mzXmlfiles_;

};




#endif
