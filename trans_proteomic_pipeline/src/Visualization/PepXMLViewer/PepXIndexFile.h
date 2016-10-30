// -*- mode: c++ -*-



/*
    Program: PepXMLViewer.cgi
    Description: CGI program to display pepxml files in a web browser
    Date: July 31 2006

    Copyright (C) 2006  Natalie Tasman (original author), ISB Seattle

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    Natalie Tasman
    Institute for Systems Biology
    401 Terry Avenue North
    Seattle, WA  98109  USA
    
    email (remove underscores):
    n_tasman at systems_biology dot org
*/



#ifndef _INCLUDED_PEPXINDEXFILE_H_
#define _INCLUDED_PEPXINDEXFILE_H_

/** @file 
    @brief
*/

#include <vector>
#include <string>
#include <fstream>

#include "PepXField.h"
#include "PepXOptions.h"
#include "PipelineAnalysis.h"
#include "SQHandler.h"

using std::string;
using std::vector;
using std::ofstream;



struct IndexSortData {
  unsigned long long startOffset_;
  unsigned long long endOffset_;
  int MRS_;
  string data_;

  IndexSortData(unsigned long long start, unsigned long long end, 
		int mrs, const string & dataStr) : 
    startOffset_(start),
    endOffset_(end),
    MRS_(mrs),
    data_(dataStr)
  {}
};


class IndexDataSorter {
protected:
  Field field_;
  int sortDir_;
public:
  IndexDataSorter(Field field, const int direction) : field_(field),
    sortDir_(direction) {
  }
  
  ~IndexDataSorter() {}

  bool operator()( IndexSortData* const& A,  IndexSortData* const& B);
};




/**
   implements SQHandler
 */
class IndexFile : public SQHandler {
 public:

  PipelineAnalysis* pipeline_;
  Options* options_;
  int numAcceptedSQ_;
  int totalNumSQ_;
  ofstream fout_;
  ofstream foutTmp_;

  string tmpFileName_;
  string indexFileName_;
  

  vector<string> peptideList_;
  vector<string> strippedPeptideList_;

  vector<string> proteinList_;

  vector<Field> minimumSortFilterFields_;

  int numSingleHitProteins_;

  IndexFile(void);
  
  void reset(PipelineAnalysis& pipeline,
	    Options& options);

  void writeColumnHeader(void);
  void writeIndexHeader(void);

  bool readAndTestHeader(void);
  

  void
  generateFilteredListFromIndex(vector<IndexSortData*>& filteredList);

  /**
     When we don't have an index file, we use a PepXSAXHandler parser to
     parse the entire PepXML file.  During this parsing, this function
     is called when we encouter *any* SQ node.  In this mode of
     action, we want to extract the minimum required data for sorting
     and filtering from this node, and write it directly to the index
     file.
  */
  virtual void acceptSQ(XMLNodePtr sqNode);


};
    
  
  
#endif // header guards
