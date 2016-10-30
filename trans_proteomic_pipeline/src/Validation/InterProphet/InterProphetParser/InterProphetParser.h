#ifndef INTER_PROPH_PARSER_H
#define INTER_PROPH_PARSER_H

/*

Program       : InterProphet                                                       
Author        : David Shteynberg <dshteynb  AT systemsbiology.org>                                                       
Date          : 12.12.07

Primary data object holding all mixture distributions for each precursor ion charge

Copyright (C) 2007 David Shteynberg

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

David Shteynberg
Institute for Systems Biology
401 Terry Avenue North 
Seattle, WA  98109  USA

*/


#include "common/sysdepend.h"
#include "common/TPPVersion.h"
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string>
#include "Parsers/Parser/Parser.h"
#include "common/Array.h"
#include "Parsers/Parser/Tag.h"
#include "InterProphet.h"
#include "Parsers/Parser/TagListComparator.h" // regression test stuff - bpratt Insilicos LLC, Nov 2005
#include "pwiz/utility/misc/random_access_compressed_ifstream.hpp" // for potentially reading pep.xml.gz
#include "gzstream.h" // for producing .gz files if indicated by output filename


using namespace std;

class InterProphetParser : public Parser {

 public:

  InterProphetParser(bool nss_flag, bool nrs_flag, bool nse_flag, bool nsw_flag, bool nsi_flag, bool nsm_flag, bool nsp_flag, bool use_fpkm, bool use_length, string* catfile, string* decoyTag, int max_threads);
  void addFile(const char* filename);
  ~InterProphetParser();
  //void setFilter(Tag* tag);
  bool setOutFile(const char* c);

  void run();
  void parse(const char* c);
  void parse_catfile();
  void writePepXML();
  void writePepSHTML();
  void writePepXMLFast();
  void writePepXMLFast(double minProb, string* opts = NULL);
  void printResult();

 protected:

  //void displayOptions(char* eng);

  InterProphet* inter_proph_;
  
  Array<string*>* input_files_;
  Array<Tag*>* anal_summs_;


  int_hash* ms_runs_;

  string* outfile_;
  Boolean use_nss_;
  Boolean use_nrs_;
  Boolean use_nse_;
  Boolean use_nsi_;
  Boolean use_nsm_;
  Boolean use_nsp_;
 
  string* catfile_;
  Boolean use_cat_;

  string* decoyTag_;
  Boolean use_decoy_;

  int max_threads_;

};











#endif
