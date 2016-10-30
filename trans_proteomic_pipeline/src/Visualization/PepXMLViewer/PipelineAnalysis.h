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



#ifndef _INCLUDED_PIPELINEANALYSIS_H_
#define _INCLUDED_PIPELINEANALYSIS_H_

/** @file 
    @brief
*/


#include <vector>
#include <map>
#include <string>
#include <iostream>

#include <boost/shared_ptr.hpp>

#include "XMLNode.h"
#include "PMap.h"
#include "PepXField.h"
#include "PepXOptions.h"
#include "PepXNode.h"

using std::string;
using std::map;
using std::vector;
using std::ostream;
using boost::shared_ptr;



typedef shared_ptr< map<string, string> > mapPtr;


class PipelineAnalysis : public PMap {
protected:
  int currentRunSummary_;
  int numRunSummaries_;
  vector<mapPtr> runSummaryVec_;
  string peptideProphetOpts_;
  // header info flags
  bool interact_;
  bool peptideProphet_;
  bool ptmProphet_;
  bool interProphet_;
  bool asapratio_;
  bool xpress_;
  bool libra_;
  bool prepedFields_;

public:

  bool rewroteIndex_;

  string xmlFileName_;
  bool haveIndexFile_;


  int totalNumSQ_;
  int acceptedSQ_;

  int numUniquePeptides_;
  int numUniqueStrippedPeptides_;
  int numUniqueProteins_;
  int numSingleHitProteins_;


  vector<int> libraChannelsMZ_;

  map<int, AAModInfoVec> modificationInfo_; // key is MRS#

  PepXNodeVec pepXNodeVec_; // list of objects summarizing SQ info
			  //(including their byte offsets)  

  vector<long> mrsOffsets_; // filepointer locations of MRS sections




  vector<Field> fields_; // was 'allColumns'

  // used for html display
  map<string, bool> conditions_;
  map<string, string> generalInfo_;


  void extractHeaderInfo(XMLNodePtr node);
  void extractMRSInfo(XMLNodePtr node, int mrsNum);
  void prepareFields(void);

 public: // functions
  PipelineAnalysis();
  void reset(void); // clear everything
  ~PipelineAnalysis();
  

  void prepareParameters(Options& options);
  



  void setCurrentRunSummary(int runSumNum);
  int getNumRunSummaries(void);
  int getCurrentRunSummaryNum(void);
  void newRunSummary(void);



  // --PMap interface--
  /**
    
    return the string if it's in the general map
    otherwise, return it if it's in the current run summary map
    otherwise, return empty string
  */

  // lookup only!
  virtual string operator[](const string& from);

  virtual AAModInfoVec* getModVec(void) {
    return &(modificationInfo_[currentRunSummary_]);
  }

  string& runSummaryParameter(const string& from);

  void print(ostream &out);
  void print(void); // useful for gdb enthusiasts

  
};


#endif // header guards

