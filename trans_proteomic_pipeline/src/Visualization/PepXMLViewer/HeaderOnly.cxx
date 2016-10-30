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



#include "PepXDebug.h"
#include "XMLNode.h"
#include "HeaderOnly.h"
#include "PepXUtility.h"



HeaderOnly::HeaderOnly() : 
  inHeader(false)
{
}

HeaderOnly::~HeaderOnly() {
}


void
HeaderOnly::init(const string& xmlFileName,
		 PipelineAnalysis& pipeline) {
  setFileName(xmlFileName.c_str());
  pipeline_ = &pipeline;
}


  
void
HeaderOnly::startElement(const XML_Char *elementName, 
			 const XML_Char **attr) {
  string name = elementName;

  if (!inHeader) {
    if (name != "msms_pipeline_analysis") {
      return;
    }
    else {
      inHeader = true;
      //cout << "<found header start>" << endl;
    }
  }
    
  // in Header: this is either the start tag or a subnode;/

  // stop if we see the first MRS
  if (name == "msms_run_summary") {
    //extract header info from tree;
    inHeader = false;
    //cout << "///////////////" << endl;
    //cout << "finished header " << endl;
    //cout << "tree top name is " << tree.top()->name_ << endl;
    XMLNodePtr headerNode = tree.top();

    //cout << "header start offset: " << headerNode->startOffset_ << endl;
    //tree.getRootNode()->print();
    //cout << "///////////////" << endl <<endl;
    pipeline_->extractHeaderInfo(headerNode);
    tree.popStack();
    tree.getRootNode()->removeChild(headerNode);

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
HeaderOnly::endElement(const XML_Char *el) {
  if (inHeader) {
    //normal end element handling;
    tree.popStack();
  }
  // otherwise do nothing

  return;
}
