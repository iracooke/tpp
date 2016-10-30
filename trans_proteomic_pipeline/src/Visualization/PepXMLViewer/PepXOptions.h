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



#ifndef _INCLUDED_PEPXOPTIONS_H_
#define _INCLUDED_PEPXOPTIONS_H_

/** @file PepXOptions.h
    @brief sort/filter/etc options
*/


#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <istream>

// include headers that implement a archive in simple text format
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

#include "PepXField.h"
#include "PepXFilter.h"

using std::string;
using std::vector;
using std::map;
using std::ostream;
using std::cout;


/**     
    parse and store program options, either from CGI or command line.
*/


class Options {
 public: // members

  friend class boost::serialization::access;
  // When the class Archive corresponds to an output archive, the
  // & operator is defined similar to <<.  Likewise, when the class Archive
  // is a type of input archive the & operator is defined similar to >>.
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & queryString_;
    ar & xmlFileName;
    ar & exportSpreadsheet;
    ar & sortField;
    ar & sortDir;
    ar & page;
    ar & perPage;
    ar & columns;

    ar & excludeCharge1;
    ar & excludeCharge2;
    ar & excludeCharge3;
    ar & excludeChargeOthers;
    ar & excludeCharges;

    ar & filterMap_;

    ar & requireIonscoreGTidentityscore;
    ar & requireIonscoreGThomologyscore;
    ar & minimizeTableHeaders;

    ar & requireAA;
    ar & requiredAA;
    ar & requireGlyc;
    ar & includeBinGlycMotif;
    ar & requirePeptideText;
    ar & includeRequiredPepTextMods;
    ar & requiredPeptideText;
    ar & requireProteinText;
    ar & requiredProteinText;
    ar & requireSpectrumText;
    ar & requiredSpectrumText;
    
    ar & highlightPeptideText;
    ar & highlightedPeptideText;
    ar & includeHighlightedPepTextMods;
    ar & highlightProteinText;
    ar & highlightedProteinText;
    ar & highlightSpectrumText;
    ar & highlightedSpectrumText;

    ar & expandProteinList;
    
    ar & requireASAP;
    ar & requireXPRESS;
    ar & requireXPRESSMinArea;
    ar & requiredXPRESSMinArea;
    ar & requireASAPConsistent;
    ar & lowToHighASAP;

    ar & libraAbsoluteValues;
   
  }
  
  void write(std::ostream& out=cout);

  void read(std::istream& in);
  
  string queryString_; // keep this

  string xmlFileName;
  bool exportSpreadsheet;


  // sorting
  Field sortField;
  enum {ascending, descending};
  int sortDir;


  // paging
  int page;
  int perPage;
  
  // column selection
  // ordered list
  vector<Field> columns;


  string displayState;

  // filtering


  bool excludeCharge1;
  bool excludeCharge2;
  bool excludeCharge3;
  bool excludeChargeOthers;
  bool excludeCharges;
  
  

  map< string, FilterVec > filterMap_;


  bool requireIonscoreGTidentityscore;
  bool requireIonscoreGThomologyscore;

  bool minimizeTableHeaders; // visually
  

  bool requireAA;
  string requiredAA;
  bool requireGlyc; // NxS/T motif
  bool includeBinGlycMotif; // should we also consider B as well as N
			    // in the glyc motif?
  bool requirePeptideText;
  bool includeRequiredPepTextMods;
  string requiredPeptideText;
  bool requireProteinText;
  string requiredProteinText;
  bool requireSpectrumText;
  string requiredSpectrumText;

  bool highlightPeptideText;
  bool includeHighlightedPepTextMods;
  string highlightedPeptideText;
  bool highlightProteinText;
  string highlightedProteinText;
  bool highlightSpectrumText;
  string highlightedSpectrumText;

  bool expandProteinList;


  // quantitation
  bool requireASAP;
  bool requireXPRESS;
  bool requireXPRESSMinArea;
  double requiredXPRESSMinArea;

  bool requireASAPConsistent;
  bool lowToHighASAP; // how to display ratio

  bool libraAbsoluteValues; // true: absolue; false: normalized

  // keep bools for what kind of searches we need,
  // so we can pull out the SearchScore node once, etc

 public: // methods
	
  Options();
  Options(const Options&);
  void reset(void);
  ~Options(); // destroy filter datas
  
  const Options& operator=(Options & rhs);

  // true if filter options are the same;
  // ignores display-only flags
  bool filterEquals(Options& rhs);

  // return a vector of required columns for sorting
  //void generateMinimumSortInfo(string& sortInfo);
  
  // return a vector of required columns for sorting and filtering
  void generateMinimumSortFilterInfo(vector<string>& sortFilterInfo);


  void parseColumnString(const string& columnString);
  void addColumns(const string& columnString);


  string getColumnString(void) const;
  string getAllColumnString(void) const;
  string getUnusedColumnString(void) const;


  void parseQueryString(const string& queryString, bool encoded);
  bool parseCommandLineArgs(int argc, char** argv);

	
  static string lookupNode(const string& fieldCode, 
				const string& field);

  void print (ostream &out=cout) const;
  void print(void) const; // useful for gdb enthusiasts


};


#endif // header guard
