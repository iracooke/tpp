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



#include <boost/regex.hpp>

#include "PepXUtility.h"
#include "XMLNode.h"

#include "SQOffsetOnly.h"

using namespace std;
using namespace boost;




SQOffsetOnly::SQOffsetOnly() : 
  inSQ(false),
  SQCount(0),
  mode(SQOffsetOnly::lookingForSQ)
{
}

SQOffsetOnly::~SQOffsetOnly() {
}


void
SQOffsetOnly::init(const std::string& xmlFileName,
		   PipelineAnalysis& pipeline,
		   SQHandler& sqAcceptor) {
  setFileName(xmlFileName.c_str());
  pipeline_ = &pipeline;
  sqAcceptor_ = &sqAcceptor;
  tree.reset();
}


void
SQOffsetOnly::startSQ(const XML_Char *elementName, 
		      const XML_Char **attr) {
  inSQ = true;
  XMLNodePtr np = XMLNode::create(elementName, attr);
  np->startOffset_ = getByteOffset();
  tree.addNode(np);
}


  

  
void
SQOffsetOnly::startElement(const XML_Char *elementName, 
			   const XML_Char **attr) {
  string name = elementName;

  if (mode == SQOffsetOnly::lookingForSQ) {
    if (!inSQ) {
      if (name != "spectrum_query" ) {
	return;
      }
      else {
	inSQ = true;
#ifdef PRINT
	cout << "<found SQ start>" << endl;
#endif
	startSQ(elementName, attr);
	return;
      }
    }
      
    // in SQ: either start tag or inside

    // when do we stop? at end of msms_run_summary
      
    XMLNodePtr np = XMLNode::create(elementName, attr);
    np->startOffset_ = getByteOffset();
            
    // modify the names for our conventions
    if ( (name == "search_score") ||
	 (name == "parameter") ) {
      // these special value nodes; ***rename the actual node-name*** to
      // include attr name
      name += "[@" +  np->attrs_[string("name")] + "]";
      np->name_ = name;
      //cout << "renamed node to: " << name_ << endl;
    }


    tree.addNode(np);
  }
}
      





  
void
SQOffsetOnly::endElement(const XML_Char *elementName) {
  string name = elementName;
  if (mode == SQOffsetOnly::lookingForSQ) {
    XMLNodePtr np;
    if (inSQ && name == "spectrum_query") {
      ++SQCount;
#ifdef PRINT
      cout << SQCount << " sqs found" << endl;
#endif
      inSQ = false;
      np = tree.top();
      np->endOffset_ = getByteEndOffset();

	
      // do whatever needs to be done
      sqAcceptor_->acceptSQ(np);

      tree.popStack();
#ifdef PRINT
      cout << "before remove child: np, top: " << np->name_;
      cout << ", " << tree.top()->name_ << endl;
      tree.getRootNode()->print();
#endif
	
      tree.top()->removeChild(np);
#ifdef PRINT
      cout << "||| finished SQ |||" << endl;
      np->print();
      cout << "||| |||" << endl << endl;
#endif
      //np->print();
      //stopParsing();
      return;
    }
    else {
      // normal
      //cout << "top: " << tree.top()->name_ << endl;
      tree.popStack();
      return;
    }
  }
}

