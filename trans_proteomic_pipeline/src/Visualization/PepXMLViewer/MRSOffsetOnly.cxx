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

#include <string>

#include "PepXUtility.h"
#include "XMLNode.h"

#include "MRSOffsetOnly.h"

using std::string;

MRSOffsetOnly::MRSOffsetOnly() : 
  inMRS(false),
  MRSCount(0)
{
}

MRSOffsetOnly::~MRSOffsetOnly() {
}


void
MRSOffsetOnly::init(const string& xmlFileName,
	  PipelineAnalysis& pipeline) {
  setFileName(xmlFileName.c_str());
  pipeline_ = &pipeline;
}



void
MRSOffsetOnly::offsetParse(void) {
  for (unsigned int i=0; i<pipeline_->mrsOffsets_.size(); i++) {
    inMRS = false;
    debug(debugout << "trying MRS offset parse" << i);
    int offset = pipeline_->mrsOffsets_[i];
    resetParser();
    jumpParse(offset);
    debug(debugout << "end MRS offset parse" << i);
  }
}
  




void
MRSOffsetOnly::startElement(const XML_Char *elementName, 
			  const XML_Char **attr) {
  string name = elementName;

  if (!inMRS) {
    if (name != "msms_run_summary" ) {
      return;
    }
    else {
      inMRS = true;
      //cout << "<found mrs start>" << endl;
    }
  }
    
  // in MRS: either start tag or inside
  if (name == "spectrum_query") {
    //extract MRS info from tree;
    inMRS = false;
    //cout << "///////////////" << endl;
    //cout << "finished MRS " << MRSCount << endl;
    //cout << "tree top name is " << tree.top()->name_ << endl;
    XMLNodePtr curMRS = tree.top();
    //cout << "current MRS start offset: " << curMRS->startOffset_ << endl;
    //tree.getRootNode()->print();
    //cout << "///////////////" << endl <<endl;
    pipeline_->extractMRSInfo(curMRS, MRSCount);
    MRSCount++;
    tree.popStack();
    tree.getRootNode()->removeChild(curMRS);
      
    debug(debugout << endl << "*** stopping jumparse *** ");
      
    stopParsing();
    return;
  }
  else {
    //cout << name << "!= spectrum_query " << endl;
    XMLNodePtr np = XMLNode::create(elementName, attr);
    np->startOffset_ = getByteOffset();
    tree.addNode(np);
  }
}



  
void
MRSOffsetOnly::endElement(const XML_Char *el) {
  if (inMRS) {
    //normal end element handling;
    tree.top()->endOffset_ = getByteEndOffset();
    tree.popStack();
  }
  // otherwise do nothing

  return;
}

