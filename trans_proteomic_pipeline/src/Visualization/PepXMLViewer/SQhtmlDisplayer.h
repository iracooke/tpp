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

#ifndef _INCLUDED_SQHTMLDISPLAYER_H_
#define _INCLUDED_SQHTMLDISPLAYER_H_

/** @file 
    @brief
*/

#include <string>
#include <iostream>

#include "PepXOptions.h"
#include "PipelineAnalysis.h"
#include "PepXNode.h"
#include "SQHandler.h"

using std::string;
using std::cout;
using std::endl;


/**
   implements SQHandler
 */
class SQhtmlDisplayer : public SQHandler {
 public:

  PipelineAnalysis* pipeline_;
  Options* options_;


  int rowNum;
  string oldProtein;
  int proteinGrp;
  string style;
  string oldPeptide;
  int peptideGrp;
  

  SQhtmlDisplayer(PipelineAnalysis& pipeline,
			 Options& options) {
    pipeline_ = &pipeline;
    options_ = &options;
    oldProtein="-";
    proteinGrp = 0;
    oldPeptide="-";
    peptideGrp = 0;
    rowNum = 0;

  }





  virtual void acceptSQ(XMLNodePtr sqNode) {
    // display html data table row

    style = "";

    // color table rows, alternating or protein grouping


    cout << "<tr "; //<< i << " of " << mrs->children_.size() << " ";

    if (options_->sortField.getFieldParamName() == "Gprotein") {
      // options_->sortField is "protein"

      string protein;
      protein = options_->sortField.getText(Field::value, sqNode, *pipeline_);
      if (protein == oldProtein) {
	//cout << "match" << endl;
      } 
      else {
	//cout << "no match" << endl;
	++proteinGrp;
	oldProtein = protein;
      }
      if ( (proteinGrp % 2) == 0) {
	cout << "class=\"even\"";
      }
      else {
	cout << "class=\"odd\"";
      }
    }
    if (options_->sortField.getFieldParamName() == "Gpeptide") {
      // options_->sortField is "peptide"

      string peptide;
      peptide = options_->sortField.getText(Field::value, sqNode, *pipeline_);
      if (peptide == oldPeptide) {
	//cout << "match" << endl;
      } 
      else {
	//cout << "no match" << endl;
	++peptideGrp;
	oldPeptide = peptide;
      }
      if ( (peptideGrp % 2) == 0) {
	cout << "class=\"even\"";
      }
      else {
	cout << "class=\"odd\"";
      }
    }
    else {
      // standard 'every-other-row' highlighting
      if ( (rowNum % 2) == 0) {
	cout << "class=\"even\"";
      }
      else {
	cout << "class=\"odd\"";
      }
      
    }
    cout << " >" << endl << endl;




    // spin through columns
    for (unsigned int j=0; j<options_->columns.size();j++) {

      if (options_->columns[j].fieldCode_ == Field::ptmProphet) {
	if ((options_->columns[j].fieldName_ == "ptm_peptide")) {
	  style = "class=\"peptide\" style=\"text-align: left;\"";
	}
      }
      else if (options_->columns[j].fieldCode_ == Field::searchScore
	  ||
	  options_->columns[j].fieldCode_ == Field::peptideProphet
	  ||
	  options_->columns[j].fieldCode_ == Field::quantitation) {
	style = "style=\"text-align: center;\"";
      }
      else if ((options_->columns[j].fieldCode_ == Field::general)) {
	if ((options_->columns[j].fieldName_ == "peptide")) {
	  style = "class=\"peptide\" style=\"text-align: left;\"";
	}
	else if (  options_->columns[j].fieldName_ == "protein"
		   ||
		   options_->columns[j].fieldName_ == "protein_descr"
		   || 
		   options_->columns[j].fieldName_ == "spectrum"
		 ) {
	  style = "style=\"text-align: left;\"";
	}
	else {
	  style = "style=\"text-align: center;\"";
	}
      }
      cout << "<td " << style << " >" << endl;
      
      
      cout << options_->columns[j].getText(Field::html, 
						sqNode, 
						*pipeline_,
						options_->highlightedPeptideText,
						options_->highlightedProteinText,
						options_->highlightedSpectrumText,
						options_->includeHighlightedPepTextMods,
						options_->libraAbsoluteValues);
      cout << "</td>" << endl;
    } // end td loop
    cout << "</tr>" << endl;
    rowNum++;
  }


  

};


#endif // header guards
