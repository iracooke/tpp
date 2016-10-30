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


#include <vector>
#include <string>

#include "PepXUtility.h"

#include "PepXNodeVecBuilder.h"


using namespace std;

PepXNodeVecBuilder::PepXNodeVecBuilder(PipelineAnalysis& pipeline,
				       Options& options) {
  pipeline_ = &pipeline;
  options_ =  &options;
  pepXNodeVec_ = &(pipeline_->pepXNodeVec_);
  numAcceptedSQ_ = 0;
  numSQ_ = 0;

  numSingleHitProteins_ = 0;


}



/**
   When we don't have an index file, we use a PepXSAXHandler parser to
   parse the entire PepXML file.  During this parsing, this function
   is called when we encouter a good (filtered) SQ node.  In this mode
   of action, we want to extract the vital info from this node and
   store it in a PepXNode object; this object is stored in our PepXNode
   list and will be used later for parsing the specific nodes, and
   writing out the index file.
*/

void 
PepXNodeVecBuilder::acceptSQ(XMLNodePtr sqNode) {
  
  ++numAcceptedSQ_;
  // yes, it passed filters

  // based on current fields (set from header and MRS), extract data
  // from the XMLNode and store it in the PepXNode.

  // store the vital data in the filteredNodeList
  PepXNodePtr px(new PepXNode());

  toInt(sqNode->getAttrValue("index"), px->index);
  px->runSummaryNumber = pipeline_->getCurrentRunSummaryNum();
  px->startOffset = sqNode->startOffset_;
  px->endOffset = sqNode->endOffset_;
  // current run summary: set already?

  //sqNode->print();
  for (unsigned int e=0; 
       e< (pipeline_->fields_.size()); 
       e++) {
    
    //std::cerr << "field: " << pipeline_->fields_[e].fieldName_ << std::endl;

    px->sortData.push_back(pipeline_->fields_[e].getText(Field::value,
							 sqNode,
							 *pipeline_,
							 "",
							 "",
							 "",
							 options_->includeHighlightedPepTextMods,
							 options_->libraAbsoluteValues));
    //px->print(std::cout);
	
  }

  pepXNodeVec_->push_back(px);



  /* 
     peptide and protein tallying
     
     keep protein and peptide info for bookkeeping.


  */
  proteinList_.push_back(sqNode->
			findChild("search_result")->
			findChild("search_hit", "hit_rank", "1")->getAttrValue("protein"));
  if (sqNode->
      findChild("search_result")->
      findChild("search_hit", "hit_rank", "1")->
      findChildren("alternative_protein")->size() == 0) {
    ++numSingleHitProteins_;
  }

  string peptideSequence;


  peptideSequence = sqNode->
    findChild("search_result")->
    findChild("search_hit", "hit_rank", "1")->getAttrValue("peptide");

  // save as stripped 
  strippedPeptideList_.push_back(peptideSequence);


  // each vector element is the text for one AA
  vector<string> peptideAAVec;
  for (unsigned int i=0; i<peptideSequence.size(); i++) {
    peptideAAVec.push_back(peptideSequence.substr(i,1));
  }

	
  // add modifications, if any
  if (! 
      sqNode->
      findChild("search_result")->
      findChild("search_hit", "hit_rank", "1")->findChild("modification_info")->isEmpty_) {
	  
    // yes, there are modifications

    int numMods = (int)sqNode->
      findChild("search_result")->
      findChild("search_hit", "hit_rank", "1")->findChild("modification_info")->children_.size();

    int c; 
    for (c=0; c<numMods; c++) {
      int modPos;
      toInt(sqNode->
	    findChild("search_result")
	    ->
	    findChild("search_hit", "hit_rank", "1")
	    ->findChild("modification_info")
	    ->children_[c]->getAttrValue("position"),
	    modPos);

      string modVal = 
	(sqNode->
	 findChild("search_result")
	 ->
	 findChild("search_hit", "hit_rank", "1")
	 ->findChild("modification_info")
	 ->children_[c]->getAttrValue("mass"));
      char t[255];
      double d;
      toDouble(modVal, d);
      sprintf(t, "%.2f", d);
	    
      // string offset starts from zero
      modPos--;
      peptideAAVec[modPos] += "[";
      peptideAAVec[modPos] += t;
      peptideAAVec[modPos] += "]";
    }
  }

	
  // reassemble into one string.
  string modPeptideString;
  for (unsigned int j=0; j<peptideAAVec.size(); j++) {
    modPeptideString += peptideAAVec[j];
  }

  peptideList_.push_back(modPeptideString);




}

























