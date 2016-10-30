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



#ifndef _HEADERMRSSQ_H_
#define _HEADERMRSSQ_H_

//#define PRINT 0

#include <string>

#include "PepXSAXHandler.h"
#include "XMLNode.h"
#include "XMLTree.h"
#include "PepXOptions.h"
#include "PipelineAnalysis.h"
#include "SQHandler.h"

using std::string;


class HeaderMRSSQ : public PepXSAXHandler {
public:
  bool inMRS, inSQ, inHeader;
  int MRSCount;
  XMLTree tree;
  int SQCount;
  int acceptedSQCount;
  bool skipping;

  XMLNodePtr curSQ;

  SQHandler* sqAcceptor_;
  PipelineAnalysis* pipeline_;
  Options* options_;

  int mode; // enum
  enum {
    lookingForHeader,
    lookingForMRS,
    lookingForSQ
  };
    
  HeaderMRSSQ();


  ~HeaderMRSSQ();



  void startSQ(const XML_Char *elementName, 
	       const XML_Char **attr);



  void endElementInSQ(const std::string& name);
  bool checkSQNode(XMLNodePtr sqNode);


  void startMRS(const XML_Char *elementName, 
		const XML_Char **attr);


  
  virtual void startElement(const XML_Char *elementName, 
			    const XML_Char **attr);
  
  virtual void endElement(const XML_Char *elementName);

  
  void init(const std::string& xmlFileName,
	    Options& options,
	    PipelineAnalysis& pipeline,
	    SQHandler& sqAcceptor);
	    

};


#endif // header guards
