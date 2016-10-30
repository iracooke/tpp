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



#ifndef _INCLUDED_PEPXFIELD_H_
#define _INCLUDED_PEPXFIELD_H_

/** @file field.h
    @brief 
*/


#include <string>

#include <boost/shared_ptr.hpp>

// include headers that implement a archive in simple text format
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

#include "XMLNode.h"
#include "PMap.h"

using std::string;
using boost::shared_ptr;


/**
   A field here refers to a specific displayed data category.  This
   may or may not directly correspond to a specific attribute-name of
   a spectrum_query (sub) element.  Example: "index" corresponds to an
   attribute of the top "spectrum_query" element.  "ions" corresponds
   to a combination of attributes from the search_hit element.  Most
   fields do, however, directly correspond to a specific attribute.

   The field name is displayed as column headers in the display.

   Different fields require different handling elsewhere in the
   program (eg. filtering.)  The program design requires that a field
   provide information as to a "field code", which identifies the type
   of field ('general', 'search score', 'protein prophet', etc.)

   In situations where a unique name for a field is required, the
   unique name is composed of a single-character field code, followed
   by the field name: Gindex, Sdeltacn, etc.
  
   This class provides methods to

   o convert a single-character field code to an enumerated type

   o return either an html or plain-text formatted string (eg, a
     peptide sequence with link and highlighting markup, or just a
     plain peptide string); these methods require the entire
     spectrum_query XMLNode.

   o return a longer descriptive string describing the field (for
     help, etc.)

 */


class Field;
typedef shared_ptr<Field> FieldPtr;

class Field {
protected:


public:

  enum {
    unknown = 0,
    general,
    ptmProphet,
    peptideProphet,
    interProphet,
    searchScore,
    quantitation
  }; // field codes


  enum {
    value,
    plainText,
    html
  }; // display options


  int fieldCode_; // Field::General
  string fieldName_; // spectrum

  // the associated node-name; if this field were filtered, which node
  // should be evaluated?  search score and parameter node names are
  // the rewritten versions (eg: "search_score[@name=xcorr]")
  string nodeName_; // spectrum_query

  bool isValueNode_; // true for parameter, search_score nodes of form
		     // <nodeName_ name=attrName value="double to compare" />

		     // false

  friend class boost::serialization::access;
  // When the class Archive corresponds to an output archive, the
  // & operator is defined similar to <<.  Likewise, when the class Archive
  // is a type of input archive the & operator is defined similar to >>.
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & fieldCode_;
    ar & fieldName_;
    ar & nodeName_;
    ar & isValueNode_;
  }

  void write(std::ostream& out) {
    out << fieldCode_ << " ";
    out << fieldName_ << " ";
    out << nodeName_  << " ";
    out << isValueNode_ << " ";
  }

  void read(std::istream& in) {
    in >> fieldCode_;
    in >> fieldName_;
    in >> nodeName_ ;
    in >> isValueNode_;
  }
  
public:

  Field();
  Field(const Field& rhs) {
    fieldCode_ = rhs.fieldCode_;
    fieldName_ = rhs.fieldName_;
    nodeName_ = rhs.nodeName_;
    isValueNode_ = rhs.isValueNode_;
  }

  const Field& operator=(const Field & rhs) {
    fieldCode_ = rhs.fieldCode_;
    fieldName_ = rhs.fieldName_;
    nodeName_ = rhs.nodeName_;
    isValueNode_ = rhs.isValueNode_;
    return *this;
  }

  // given "Gindex", "Sxcorr"
  Field(const string & name);

  // given ("index", "G")
  Field(const string & name, char fieldCode);


  void set(const string & name, char fieldCode);

  // sets the associated node name: eg ions => search_hit
  void nodeNameLookup(void);

  // returns enum code
  int fieldCodeLookup(char code);

  // returns "Gions", "Sxcorr", etc
  string getFieldParamName(void) const;

  bool operator==(const Field& rhs);

  string getAbbreviation(void);

  string getDescription(const string& searchEngine);


  string getText(int mode, 
		 const XMLNodePtr& node, 
		 PMap& info,
		 const string& highlightedPeptideText="",
		 const string& highlightedProteinText="",
		 const string& highlightedSpectrumText="",
		 const bool includeHighlightedPepTextMods=false,
		 const bool libraAbsoluteValues=true);


  // helper method: 
  // given a search_hit node, search the peptide text
  static bool searchPeptideText(XMLNodePtr node,
				string searchText,
				bool includeModText,
				bool searchOnly, // then no text generated
				string& returnText,
				bool returnHtml=false, // or plain text (for spreadsheet, etc)
				bool returnBrackets=true // use [] notation?
				);

  string getPTMProbsPlot(XMLNodePtr ptm_node, int pepsize);  

  string getModPeptide(XMLNodePtr mod_node, string pep);  

};


#endif // header guards
