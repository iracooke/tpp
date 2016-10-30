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

#ifndef _INCLUDED_SQSPEADSHEETDISPLAYER_H_
#define _INCLUDED_SQSPEADSHEETDISPLAYER_H_

/** @file 
    @brief
*/

#include <string>
#include <iostream>

#include "PepXNode.h"
#include "SQHandler.h"
#include "PipelineAnalysis.h"
#include "PepXOptions.h"

using std::string;
using std::ostream;
using std::endl;


/**
   implements SQHandler
 */
class SQspreadsheetDisplayer : public SQHandler {
 public:

  PipelineAnalysis* pipeline_;
  Options* options_;
  ostream* fout_;

  SQspreadsheetDisplayer(PipelineAnalysis& pipeline,
			 Options& options,
			 ostream& fout) {
    pipeline_ = &pipeline;
    options_ = &options;
    fout_ = &fout;
  }



  void writeHeaders(void) {
    ostream& fout = *fout_;
    // print headers
    string colHeader;
    for (unsigned int i=0; i<options_->columns.size();i++) {
      if (i>0) fout << "\t";
      colHeader = options_->columns[i].fieldName_;
      // distinguish prop from interProphet prob

      if (options_->columns[i].fieldCode_ == Field::interProphet &&
	  colHeader == "probability") {
	colHeader = "iP probability";
      }


      // asapratio actually prints as TWO tab-delim columns: mean and error
      if (colHeader == "asapratio") {
	fout << "asap mean" << "\t" << "asap error";
      } else {
	fout << options_->columns[i].fieldName_;
      }
    }
    fout << endl;

  }


  virtual void acceptSQ(XMLNodePtr sqNode) {
    ostream& fout = *fout_;

    // display spreadsheet data row
    for (unsigned int j=0; j<options_->columns.size();j++) {
      if (j>0) fout << "\t";
      fout << options_->columns[j].getText(Field::plainText, 
					   sqNode, 
					   *pipeline_,
					   "",
					   "",
					   "",
					   options_->includeHighlightedPepTextMods,
					   options_->libraAbsoluteValues);
    }
    fout << endl;

  }



};


#endif // header guards
