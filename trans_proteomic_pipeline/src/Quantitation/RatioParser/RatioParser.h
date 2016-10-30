#ifndef RATIOPARSER_H
#define RATIOPARSER_H

#include "Parsers/Parser/Parser.h"
#include "Parsers/Parser/TagFilter.h"

class RatioParser : public Parser {

 public:

  RatioParser(const char* xmlfile);

 protected:

  void parse(const char* xmlfile);
  void setFilter(Tag* tag);
  //  char* getBofFile(char* file);
  //  char* getOrigFile(char* file);


};











#endif
