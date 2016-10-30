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


#ifndef _INCLUDED_PEPXPARSER_H_
#define _INCLUDED_PEPXPARSER_H_

/** @file 
    @brief
*/

#include <vector>
#include <string>

#include "PepXSAXHandler.h"
#include "PepXOptions.h"
#include "PepXIndexFile.h"
#include "PipelineAnalysis.h"
#include "SQHandler.h"

// parsing with index file:
#include "HeaderMRSSQ.h"

// parsing without index file:
#include "MRSOffsetOnly.h"
#include "SQOffsetOnly.h"
#include "HeaderOnly.h"

using std::string;
using std::vector;



/**


   ???implements SQHandler

*/
class PepXParser { //: SQHandler {
protected:


  /* *** various parsing objects *** */

  // without index file
  HeaderMRSSQ headerMRSSQ_; // no index file: parse the header, MRS sections, and SQ nodes


  // with index file
  HeaderOnly headerOnly_; // with index file: parse the header only

  MRSOffsetOnly mrsOffsetOnly_; // with index file: parse the MRS
				// sections given their start offsets
				// from the index file

  SQOffsetOnly sqOffsetOnly_; // with a list of prefiltered nodes from
			      // the index file, use that list of
			      // offsets to jump into the pepxml file
			      // and only parse the selected SQ
			      // elements



  
  bool readIndex(Options& options);
  IndexFile indexFile_;
  vector<IndexSortData*>* indexSortData_;

public:  
  PipelineAnalysis pipeline_;


public:
  PepXParser();
  void reset(void);
  ~PepXParser();
  void parseSQNodes(const std::string& xmlFileName, 
		    SQHandler& sqAcceptor,
		    int page,
		    int perPage);
  void parsePepXMLEssentials(const std::string &xmlFileName,
			     Options& options);
};



#endif // header guards


