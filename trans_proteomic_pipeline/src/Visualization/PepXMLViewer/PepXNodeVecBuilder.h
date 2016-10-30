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



#ifndef _INCLUDED_PEPXNODEVECBUILDER_H_
#define _INCLUDED_PEPXNODEVECBUILDER_H_

/** @file 
    @brief
*/

#include <vector>
#include <string>

#include "PepXNode.h"
#include "PepXField.h"
#include "PipelineAnalysis.h"
#include "SQHandler.h"

using std::string;
using std::vector;




/**
   implements SQHandler
 */
class PepXNodeVecBuilder : public SQHandler {
 public:

  PepXNodeVec* pepXNodeVec_;
  PipelineAnalysis* pipeline_;
  Options* options_;
  int numAcceptedSQ_;
  int numSQ_;


  vector<string> peptideList_;
  vector<string> strippedPeptideList_;

  vector<string> proteinList_;

  int numSingleHitProteins_;

  PepXNodeVecBuilder(PipelineAnalysis& pipeline,
		     Options& options);


  /**
     When we don't have an index file, we use a PepXSAXHandler parser to
     parse the entire PepXML file.  During this parsing, this function
     is called when we encouter a good (filtered) SQ node.  In this mode
     of action, we want to extract the vital info from this node and
     store it in a PepXNode object; this object is stored in our PepXNode
     list and will be used later for parsing the specific nodes, and
     writing out the index file.
  */

  virtual void acceptSQ(XMLNodePtr sqNode);


};
    
  
  
#endif // header guards
