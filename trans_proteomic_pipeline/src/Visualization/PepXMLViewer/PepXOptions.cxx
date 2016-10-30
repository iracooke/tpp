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


#include <cstring>

#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>

#include "PepXUtility.h"

#include "PepXOptions.h"

using namespace std;


/** 
    todo:

    convert requiredAA string to all caps

*/




/**
   parse and store program options, either from CGI or command line.
*/


Options::Options() {
  reset();
}

void Options::write(ostream& out)
 {
  if (queryString_.empty()) {
	   queryString_ = "xmlFileName=" + xmlFileName;
   }

   out << queryString_ << " ";
   out << xmlFileName << " ";
   out << exportSpreadsheet << " ";

   sortField.write(out);

   out << sortDir << " ";
   out << page << " ";
   out << perPage << " ";

   out << columns.size() << " ";

   for (int i =0; i < (int)columns.size(); i++)
	   columns[i].write(out);

   out << excludeCharge1 << " ";
   out << excludeCharge2 << " ";
   out << excludeCharge3 << " ";
   out << excludeChargeOthers << " ";
   out << excludeCharges << " ";

   out << filterMap_.size() << " ";

   for ( map< string, FilterVec >::iterator itr = filterMap_.begin(); itr != filterMap_.end(); itr++ ) {
	   out << (*itr).first << " ";
	   (*itr).second.write(out);
   }

   out << requireIonscoreGTidentityscore << " ";
   out << requireIonscoreGThomologyscore << " ";
   out << minimizeTableHeaders << " ";

   out << requireAA << " ";
   out << requiredAA << " ";
   out << requireGlyc << " ";
   out << includeBinGlycMotif << " ";
   out << requirePeptideText << " ";
   out << includeRequiredPepTextMods << " ";
   out << requiredPeptideText << " ";
   out << requireProteinText << " ";
   out << requiredProteinText << " ";
   out << requireSpectrumText << " ";
   out << requiredSpectrumText << " ";

   out << highlightPeptideText << " ";
   out << highlightedPeptideText << " ";
   out << includeHighlightedPepTextMods << " ";
   out << highlightProteinText << " ";
   out << highlightedProteinText << " ";
   out << highlightSpectrumText << " ";
   out << highlightedSpectrumText << " ";

   out << expandProteinList << " ";

   out << requireASAP << " ";
   out << requireXPRESS << " ";
   out << requireXPRESSMinArea << " ";
   out << requiredXPRESSMinArea << " ";
   out << requireASAPConsistent << " ";
   out << lowToHighASAP << " ";

   out << libraAbsoluteValues << " ";
   out << endl;
 }

 void Options::read(istream& in)
 {
   in >> skipws >>  queryString_;
   in >>  xmlFileName;
   in >>  exportSpreadsheet;
   sortField.read(in);
   in >>  sortDir;
   in >>  page;
   in >>  perPage;

   size_t size;

   in >> size;
   for (int i =0; i < (int)size; i++) {
	   Field* col = new Field();
	   (*col).read(in);
	   columns.push_back(*col);
   }

   in >>  excludeCharge1;
   in >>  excludeCharge2;
   in >>  excludeCharge3;
   in >>  excludeChargeOthers;
   in >>  excludeCharges;

   in >>  size;

   for (int i =0; i < (int)size; i++) {
	   string name;
	   in >> name;
	   FilterVec* filters = new FilterVec();
	   (*filters).read(in);
	   filterMap_.insert(make_pair(name, *filters));



   }


   in >>  requireIonscoreGTidentityscore;
   in >>  requireIonscoreGThomologyscore;
   in >>  minimizeTableHeaders;

   in >>  requireAA;
   if (requireAA) in >>  requiredAA;

   in >>  requireGlyc;
   in >>  includeBinGlycMotif;

   in >>  requirePeptideText;
   in >>  includeRequiredPepTextMods;
   if (requirePeptideText) in >>  requiredPeptideText;

   in >>  requireProteinText;
   if (requireProteinText) in >>  requiredProteinText;

   in >>  requireSpectrumText;
   if (requireSpectrumText) in >>  requiredSpectrumText;

   in >>  highlightPeptideText;
   if (highlightPeptideText) in >>  highlightedPeptideText;

   in >>  includeHighlightedPepTextMods;
   in >>  highlightProteinText;

   if (highlightProteinText) in >>  highlightedProteinText;
   in >>  highlightSpectrumText;

   if (highlightSpectrumText) in >>  highlightedSpectrumText;

   in >>  expandProteinList;

   in >>  requireASAP;
   in >>  requireXPRESS;
   in >>  requireXPRESSMinArea;
   in >>  requiredXPRESSMinArea;
   in >>  requireASAPConsistent;
   in >>  lowToHighASAP;

   in >>  libraAbsoluteValues;

 }

void
Options::reset(void) {  
  // set default options

  xmlFileName = "";
  
  exportSpreadsheet = false;

  // display

  //
  // sorting:
  //
  // see PipelineAnalysis::prepareParameters() for override of default
  // sort field, direction if PeptideProphet probability field is
  // present;
  //
  // default: sort by prob else sort by spectrum name
  //
  sortField.set("spectrum", 'G');
  sortDir = Options::ascending;

  page = 1;
  perPage = 50;


  displayState = "";

  // filtering

  // charges
  excludeCharge1 = false;
  excludeCharge2 = false;
  excludeCharge3 = false;
  excludeChargeOthers = false;
  excludeCharges = false; // TODO: check if we're even using this

  requireIonscoreGTidentityscore = false;
  requireIonscoreGThomologyscore = false;

  requireXPRESS = false;
  requireXPRESSMinArea = false;
  requiredXPRESSMinArea = 0;

  requireASAP = false;
  requireASAPConsistent = false;
  lowToHighASAP = false;

  libraAbsoluteValues = true;
  
  minimizeTableHeaders = true;

  expandProteinList = false;

  highlightPeptideText = false;
  highlightedPeptideText = "";
  includeHighlightedPepTextMods = false;
  highlightProteinText = false;
  highlightedProteinText = "";
  highlightSpectrumText = false;
  highlightedSpectrumText = "";

  requireAA = false;
  requiredAA = "";

  requireGlyc = false;
  includeBinGlycMotif = false;

  requirePeptideText = false;
  includeRequiredPepTextMods = false;
  requiredPeptideText = "";
  requireProteinText = false;
  requiredProteinText = "";
  requireSpectrumText = false;
  requiredSpectrumText = "";


}








Options::~Options() {
  // delete all FilterData objects; previously dynamically allocated
  map< std::string, FilterVec >::iterator i;
  //cout << "deleting filters" << endl;
  for (i=filterMap_.begin();i!=filterMap_.end();i++) {
    string nodeName = (*i).first;
    //cout << "destroying filters for " << nodeName;
    //cout << "(" << (*i).second.filterVec_.size() << " filters)" << endl;
    for (unsigned int j=0; j<(*i).second.filterVec_.size(); j++) {
      //cout << "destroying" << endl;
      //((*i).second.filterVec_[j])->print();
      delete ((*i).second.filterVec_[j]);
    }
  }
}







bool 
Options::filterEquals(Options& rhs) {
  if (xmlFileName != rhs.xmlFileName)
    return false;

  if (excludeCharge1 != rhs.excludeCharge1)
    return false;

  if (excludeCharge2 != rhs.excludeCharge2)
    return false;

  if (excludeCharge3 != rhs.excludeCharge3)
    return false;

  if (excludeChargeOthers != rhs.excludeChargeOthers)
    return false;

  if (excludeCharges != rhs.excludeCharges)
    return false;


  // filters: see below

  if (requireIonscoreGTidentityscore != rhs.requireIonscoreGTidentityscore)
    return false;

  if (requireIonscoreGThomologyscore != rhs.requireIonscoreGThomologyscore)
    return false;

  if (requireAA != rhs.requireAA)
    return false;
  
  if (requireAA && requiredAA != rhs.requiredAA)
    return false;

  if (requireGlyc != rhs.requireGlyc)
    return false;

  if (includeBinGlycMotif != rhs.includeBinGlycMotif)
    return false;

  if (requirePeptideText != rhs.requirePeptideText)
    return false;

  if (requirePeptideText && requiredPeptideText != rhs.requiredPeptideText)
    return false;

  if (includeRequiredPepTextMods != rhs.includeRequiredPepTextMods)
    return false;
  
  if (requireProteinText != rhs.requireProteinText)
    return false;

  if (requireProteinText && requiredProteinText != rhs.requiredProteinText)
    return false;

  if (requireSpectrumText != rhs.requireSpectrumText)
    return false;

  if (requireSpectrumText && requiredSpectrumText != rhs.requiredSpectrumText)
    return false;

  if (requireASAP != rhs.requireASAP)
    return false;

  if (requireXPRESS != rhs.requireXPRESS)
    return false;

  if (requireXPRESSMinArea != rhs.requireXPRESSMinArea)
    return false;

  if (requireXPRESS && requiredXPRESSMinArea != rhs.requiredXPRESSMinArea)
    return false;

  if (requireASAPConsistent != rhs.requireASAPConsistent)
    return false;


  if (libraAbsoluteValues != rhs.libraAbsoluteValues)
    return false;

  // check filters
  // filterMap_: lookup nodeName and find associated FilterVec;
  // a FilterVec is a list of filters for a given node
  map< std::string, FilterVec >::const_iterator it;
  if (filterMap_.size() == 0) {
    // spin through rhs, and make sure everything there is also empty
    map< std::string, FilterVec >::const_iterator rhsFMit;
    for (rhsFMit=rhs.filterMap_.begin();rhsFMit !=rhs.filterMap_.end();rhsFMit++) {
      if ( (*rhsFMit).second.isEmpty_ == false) {
	// we're totally empty, but rhs isn't: return false
	return false;
      }
    }
    return true;
  }
    


  for (it=filterMap_.begin();it !=filterMap_.end();it++) {
    string nodeName = (*it).first;
    map< std::string, FilterVec >::const_iterator rhsFMit 
      = (rhs.filterMap_).find(nodeName);
    if ( 
	( ! (*it).second.isEmpty_) 
	 ) { 
      // only proceed here if, for node nodeName, we have some filters
      if (rhsFMit == rhs.filterMap_.end()) {
	// no filters found for our node, which does have filters.
	// we can return false now
	//cerr << "eq: " << nodeName << " not in rhs" << endl;
	return false;
      }
      else {
	// we have 1+ filter for this node,
	// rhs has 1+ filter for this node
	if ( ! (
		(*it).second // FilterVec object
		==
		(*rhsFMit).second  // FilterVec object
		) ) {
	  //cout << "here" << endl;
	  return false;
	}
      }
    }
    else {
      // if here, our filtervec is empty for this node, but is the rhs's also empty?
      // false if not the case.
      if ( (rhsFMit != rhs.filterMap_.end())
	   && 
	   (! (*rhsFMit).second.isEmpty_) ) {
	return false;
      }
    }
  }


  // ok!
  return true;
}



// expects comma-separted list of encoded fields: eg
// "Gindex,Sxcorr,Pfval"
void Options::parseColumnString(const string & columnList) {
  columns.clear();

  // split string on comma
  boost::regex re(",");
  boost::sregex_token_iterator i(columnList.begin(), columnList.end(), re, -1);
  boost::sregex_token_iterator j;

  //cout << "in parse" << endl;

  while(i != j ) {
    //cout << "got col " << *i << endl;
    string t = *i;
    Field newField(t.substr(1), t[0]);
    columns.push_back(newField); // memory leak?
    i++;
  }
}








// returns comma-separted list of encoded fields: eg
// "Gindex,Sxcorr,Pfval"
string Options::getColumnString(void) const {
  string result="";
  for (unsigned int i=0; i<columns.size(); i++) {
    if (i!=0) {result += ",";}
    result += columns[i].getFieldParamName();
  }
  return result;
}









/**
   CGI input


*/
void
Options::parseQueryString(const string& queryString, bool encoded) {
  
  //cout << "qs: " <<queryString<< endl;
  queryString_ = queryString;

  string query = queryString;
  // we're given a url-encoded string


  // split string on ampersand
  boost::regex reAmp("&");

  boost::regex reEq("=");


  boost::sregex_token_iterator i(query.begin(), query.end(), reAmp, -1);
  boost::sregex_token_iterator j;


  char buf[8192];

  while(i != j ) {
    //cout << "pair:"<< *i<<"<br/>" << endl;
    //cout << "got pair " << *i << endl;
    
    // split on "="
    string pair = *i;
    boost::sregex_token_iterator k(pair.begin(), pair.end(), reEq, -1);
    
    // we should have a name=val pair

    // assert that len < bufsize

    // strncpy?

    // get the name
    string name = *k;
    if (encoded) {
      strcpy(buf, name.c_str());
      plustospace(buf);
      unescape_url(buf);
      name=buf;
    }

    k++;

    // if this was a &page= ... (blank), skip
    if (k==j) {
      i++;
      //cout << "skipping: next:"<< *i<<"<br/>" << endl;
      continue;
    }

    /* super hack! */
    if (name == "sortField2" ||
	name == "sortDir2" ||
	name == "FmPprobability2" ||
	name == "FMPprobability2") {
      i++;
      continue;
    }



    // get the value
    string value = *k;
    if (encoded) { 
      strcpy(buf, value.c_str());
      plustospace(buf);
      unescape_url(buf);
      value=buf;
    }
    //cout << name << " = " << value << endl;

    // list splitter
    boost::regex reComma(",");

    // big if/else comparison loop
    // name and value are decoded at this point
    if (name == "xmlFileName") {
      // remove whitespace
      boost::trim(name);
#ifdef __CYGWIN__
      string cmd = "cygpath -u '";
      cmd += value;
      cmd += "'";
      FILE* fp;
      if((fp = popen(cmd.c_str(), "r")) == NULL) {
	throw(runtime_error("cygpath error-- unable to convert\n"));
      }
      else {
	char buf[255];
	fgets(buf, 255, fp);
	pclose(fp);
	buf[strlen(buf)-1] = 0;
	xmlFileName = buf;
      }
#else 
      xmlFileName = value;
#endif
    }
    else if (name == "exportSpreadsheet") {
      toBool(value, exportSpreadsheet);
    } 
    else if (name == "page") {
      toInt(value, page);
    }
    else if (name == "perPage") {
      toInt(value, perPage);
    }
    else if (name == "columns") {
      parseColumnString(value);
    }
    else if (name == "displayState" ) {
      displayState = value;
    }
    else if(name =="sortField") {
      sortField.set(value.substr(1), value[0]);
    }
    else if(name =="sortDir") {
      toInt(value, sortDir);
    }
    else if (name == "excludeCharge1" ) {
      //cout <<"ex1<br/>" << endl; 
      excludeCharge1 = true;
      excludeCharges = true;
    }
    else if (name == "excludeCharge2" ) {
      excludeCharge2 = true;
      excludeCharges = true;
    }
    else if (name == "excludeCharge3" ) {
      excludeCharge3 = true;
      excludeCharges = true;
    }
    else if (name == "excludeChargeOthers" ) {
      excludeChargeOthers = true;
      excludeCharges = true;
    }


    else if (name == "expandProteinList" ) {
      if (value == "expand") {
	expandProteinList = true;
      }
      else if (value == "condense" ) {
	expandProteinList = false;
      }
    }

    else if(name[0] == 'F' ||
	    name[0] == 'f') {


      //cout << "new filter: " << name << ", " << value << endl;
      Filter* filter = new Filter(name,value);

      filterMap_[filter->field_.nodeName_].filterVec_.push_back(filter);
      filterMap_[filter->field_.nodeName_].isEmpty_ = false;

    }


    else if(name =="requireIonscoreGTidentityscore") {
      requireIonscoreGTidentityscore = true;
    }

    else if(name =="requireIonscoreGThomologyscore") {
      requireIonscoreGThomologyscore = true;
    }

    else if(name =="minimizeTableHeaders") {
      if (value == "yes") {
	minimizeTableHeaders = true;
      }
      else {
	minimizeTableHeaders = false;
      }
    }


    else if(name == "requireASAP") {
      requireASAP = true;
    }

    else if(name =="requireXPRESS") {
      requireXPRESS = true;
    }

    else if(name =="requiredXPRESSMinArea") {
      requireXPRESSMinArea = true;
      toDouble(value, requiredXPRESSMinArea);
    }


    else if(name =="requireASAPConsistent") {
      requireASAPConsistent = true;
    }

    else if(name =="ASAPRatioDisplay") {
      if (value == "lowToHigh") {
	lowToHighASAP = true;
      }
      else {
	lowToHighASAP = false;
      }
    }

    else if (name == "libraResultType") {
      if (value == "absolute") {
	libraAbsoluteValues = true;
      }
      else {
	libraAbsoluteValues = false;
      }
    }


    else if(name =="requireGlyc") {
      requireGlyc = true;
    }
    else if(name =="includeBinGlycMotif") {
      includeBinGlycMotif = true;
    }

    else if(name =="highlightedPeptideText" ||
	    name =="highlightedAA" // maintain compat. with old url querystrings
	    ) {
      highlightPeptideText = true;
      highlightedPeptideText = value;
    }
    else if(name =="includeHighlightedPepTextMods") {
      includeHighlightedPepTextMods = true;
    }
    else if(name =="highlightedProteinText") {
      highlightProteinText = true;
      highlightedProteinText = value;
    }
    else if(name =="highlightedSpectrumText") {
      highlightSpectrumText = true;
      highlightedSpectrumText = value;
    }
    else if(name =="requiredAA") {
      requireAA = true;
      requiredAA = value;
    }
    else if(name =="requiredPeptideText") {
      requirePeptideText = true;
      requiredPeptideText = value;
    }
    else if(name =="includeRequiredPepTextMods") {
      includeRequiredPepTextMods = true;
    }
    else if(name =="requiredProteinText") {
      requireProteinText = true;
      requiredProteinText = value;
    }
    else if(name =="requiredSpectrumText") {
      requireSpectrumText = true;
      requiredSpectrumText = value;
    }
    else {
      // unknown -- error
      //cout << "unrecogized option " << name << endl;
      //exit(1);
    }

    // go to next pair
    i++;
  }


  //print();
  
}






bool
Options::
parseCommandLineArgs(int argc, char** argv) {

  // TODO: check arc > 3
  // try/catch

  

  string filterNodeName;

  // i is incremented in loop
  for (int i=1; i<argc; ) {

    if (*(argv[i]) != '-') {
      cerr << "expected option instead of " << argv[i] << endl;
      // TODO: print usage
      return(false);
    }
      
    // check that argv[i]+2 = \0?
    char flag = *(argv[i]+1);
    string name, val;

    switch(flag) {
    case 'I' :
      val = argv[i+1];
      xmlFileName = val;
      i+=2;
      break;

    case 'P' :
      val = argv[i+1];
      toInt(val, perPage);
      i+=2;
      break;

    case 'p' :
      val = argv[i+1];
      toInt(val, page);
      i+=2;
      break;

    case 'C' : 
      // columns
      parseColumnString(argv[i+1]);
      i += 2;
      break;
    case 'S' :
      sortField.set(argv[i+1]+1, *(argv[i+1]) );
      toInt(argv[i+2], sortDir);
      i += 3;
      break;
    case 'X' :
      val = argv[i+1];
      if (val == "1" ) {
	excludeCharge1 = true;
	excludeCharges = true;
      }
      else if (val == "2" ) {
	excludeCharge2 = true;
	excludeCharges = true;
      }
      else if (val == "3" ) {
	excludeCharge3 = true;
	excludeCharges = true;
      }
      else if (val == "others" ) {
	excludeChargeOthers = true;
	excludeCharges = true;
      }
      else {
	// error, unrecogized charge exclusion
	cerr << "unrecognized charge exclusion " << val << endl;
	return(false);
      }
      i += 2;
      break;

    case 'F':
    case 'f':
      {
	// include 'f' or 'F' prefix
	name = flag;
	name += argv[i+1];

	val = argv[i+2];
      
	Filter* filter = new Filter(name,val);

	filterMap_[filter->field_.nodeName_].filterVec_.push_back(filter);
	filterMap_[filter->field_.nodeName_].isEmpty_ = false;


	i+=3;
      }
      break;
      


    case 'B' :
      // bool flag

      name = argv[i+1];
      val = argv[i+2];
      if (name == "requireGlyc" ) {
	toBool(val, requireGlyc);
      }
      else if (name == "includeBinGlycMotif") {
	toBool(val, includeBinGlycMotif);
      }
      else if (name =="requireIonscoreGTidentityscore") {
	toBool(val, requireIonscoreGTidentityscore);
      }
      else if (name =="requireIonscoreGThomologyscore") {
	toBool(val, requireIonscoreGThomologyscore);
      }
      else if (name =="requireASAP") {
	toBool(val, requireASAP);
      }
      else if (name =="requireXPRESS") {
	toBool(val, requireXPRESS);
      }
      else if (name =="requireASAPConsistent") {
	toBool(val, requireASAPConsistent);
      }
      else if (name =="lowToHighASAP") {
	toBool(val, lowToHighASAP);
      }
      else if (name =="expandProteinList") {
	toBool(val, expandProteinList);
      }
      else if (name =="exportSpreadsheet") {
	toBool(val, exportSpreadsheet);
      }
      else if (name =="minimizeTableHeaders") {
	toBool(val, minimizeTableHeaders);
      }
      else if (name =="includeHighlightedPepTextMods") {
	toBool(val, includeHighlightedPepTextMods);
      }
      else if (name =="includeRequiredPepTextMods") {
	toBool(val, includeRequiredPepTextMods);
      }
      else {
	// unrec option
	cerr << "unrecognized boolean option" << name << endl;	
	return(false);
      }
      i+=3;
      break;

    case 'R' :
      name = argv[i+1];
      val = argv[i+2];
      if (name == "highlightedPeptideText" ||
	  name =="highlightedAA" // maintain compat. with old url querystrings
	  ) {
	highlightPeptideText = true;
	highlightedPeptideText = val;
      } 
      else if (name == "highlightedProteinText") {
	highlightProteinText = true;
	highlightedProteinText = val;
      } 
      else if (name == "highlightedSpectrumText") {
	highlightSpectrumText = true;
	highlightedSpectrumText = val;
      } 
      else if (name == "requiredAA") {
	requireAA = true;
	requiredAA = val;
      }
      else if (name == "requiredPeptideText") {
	requirePeptideText = true;
	requiredPeptideText = val;
      }
      else if (name == "requiredProteinText") {
	requireProteinText = true;
	requiredProteinText = val;
      }
      else if (name == "requiredSpectrumText") {
	requireSpectrumText = true;
	requiredSpectrumText = val;
      }
      else if (name == "libraResultType") {
	if (val == "absolue") {
	  libraAbsoluteValues = true;
	}
	else {
	  libraAbsoluteValues = false;
	}
      }
      else {
	// error
	cerr << "unrecognized 'option" << name << endl;
	return(false);
      }
      i+=3;
      break;
    default : 
      cerr << "unrecognized option " << argv[i] << endl;
      //usage
      return(false);
    }
    
  }
  
  return true; // satisfy "not all control paths return a value" check

}




void Options::print() const { // useful for gdb enthusiasts
	print(std::cout);
}

void
Options::
print(std::ostream &out) const {
  out << "query string: " << queryString_ << endl;
  out << "pepxml filename:" << xmlFileName << endl;

  out << "sort field: " << sortField.getFieldParamName() << endl;

  out << "sort direction: " << sortDir << endl;

  out << "page: " << page << endl;

  out << "results per page: " << perPage << endl;

  out << "columns: " << getColumnString() << endl;

  out << "exclude +1: " << excludeCharge1 << endl;

  out << "exclude +2: " << excludeCharge2 << endl;

  out << "exclude +3: " << excludeCharge3 << endl;

  out << "exclude others: " << excludeChargeOthers << endl;

  out << "require ionscore > identityscore: " << requireIonscoreGTidentityscore << endl;

  out << "require ionscore > homologyscore: " << requireIonscoreGThomologyscore << endl;

  out << "require glyc motif: " << requireGlyc << endl;
  out << "include B in glyc motif: " << includeBinGlycMotif << endl;

  out << "require AA: ";
  if (requireAA) {out << requiredAA;} else {out << "[none]";}
  out << endl;

  out << "required peptide text: ";
  if (requirePeptideText) {out << requiredPeptideText;} else {out << "[none]";}
  if (includeRequiredPepTextMods) {out << endl << "  (including modification text";}
  out << endl;

  out << "required protein text: ";
  if (requireProteinText) {out << requiredProteinText;} else {out << "[none]";}
  out << endl;

  out << "required spectrum text: ";
  if (requireSpectrumText) {out << requiredSpectrumText;} else {out << "[none]";}
  out << endl;

  out << "highlight peptide text: ";
  if (highlightPeptideText) {out << highlightedPeptideText;} else {out << "[no]";}
  if (includeHighlightedPepTextMods) {out << endl << "  (including modification text";}
  out << endl;

  out << "highlight protein Text: ";
  if (highlightProteinText) {out << highlightedProteinText;} else {out << "[no]";}
  out << endl;

  out << "highlight spectrum Text: ";
  if (highlightSpectrumText) {out << highlightedSpectrumText;} else {out << "[no]";}
  out << endl;


  out << "expandProteinList: " << expandProteinList << endl;



  map< std::string, FilterVec >::const_iterator it;
  out << endl << "filters:" << endl;
  for (it=filterMap_.begin();it !=filterMap_.end();it++) {
    string nodeName = (*it).first;
    out << "filters for " << nodeName;
    out << "(" << (*it).second.filterVec_.size() << " filters)" << endl;
    for (unsigned int j=0; j<(*it).second.filterVec_.size(); j++) {
      ((*it).second.filterVec_[j])->print();
    }
  }


  out << "----------" << endl << endl;
    


}





const Options& 
Options::operator=(Options & rhs) {
  //cout << "!!! = " << endl;
  queryString_ = rhs.queryString_;
  xmlFileName = rhs.xmlFileName;
  exportSpreadsheet = rhs.exportSpreadsheet;
  sortField = rhs.sortField;
  sortDir = rhs.sortDir;
  page = rhs.page;
  perPage = rhs.perPage;

  displayState = rhs.displayState;
    
//  int i;

  columns.clear();
  for (unsigned i=0; i<rhs.columns.size();i++){ 
    columns.push_back(rhs.columns[i]);
  }

  excludeCharge1 = rhs.excludeCharge1;
  excludeCharge2 = rhs.excludeCharge2;
  excludeCharge3 = rhs.excludeCharge3;
  excludeChargeOthers = rhs.excludeChargeOthers;
  excludeCharges = rhs.excludeCharges;

  std::map<std::string,FilterVec>::const_iterator it;
  filterMap_.clear();
  for (it = (rhs.filterMap_).begin(); 
       it != (rhs.filterMap_).end(); 
       it++) {
    filterMap_[ ((*it).first) ] = (*it).second;
  }

  requireIonscoreGTidentityscore = rhs.requireIonscoreGTidentityscore;
  requireIonscoreGThomologyscore = rhs.requireIonscoreGThomologyscore;
  minimizeTableHeaders = rhs.minimizeTableHeaders;
  highlightPeptideText = rhs.highlightPeptideText;
  includeHighlightedPepTextMods = rhs.includeHighlightedPepTextMods;
  highlightedPeptideText = rhs.highlightedPeptideText;
  highlightProteinText = rhs.highlightProteinText;
  highlightedProteinText = rhs.highlightedProteinText;
  highlightSpectrumText = rhs.highlightSpectrumText;
  highlightedSpectrumText = rhs.highlightedSpectrumText;

  requireAA = rhs.requireAA;
  requiredAA = rhs.requiredAA;
  requireGlyc = rhs.requireGlyc;
  includeBinGlycMotif = rhs.includeBinGlycMotif;

  requirePeptideText = rhs.requirePeptideText;
  includeRequiredPepTextMods = rhs.includeRequiredPepTextMods;
  requiredPeptideText = rhs.requiredPeptideText;
  requireProteinText = rhs.requireProteinText;
  requiredProteinText = rhs.requiredProteinText;
  requireSpectrumText = rhs.requireSpectrumText;
  requiredSpectrumText = rhs.requiredSpectrumText;

  expandProteinList = rhs.expandProteinList;
  requireASAP = rhs.requireASAP;
  requireXPRESS = rhs.requireXPRESS;
  requireXPRESSMinArea = rhs.requireXPRESSMinArea;
  requiredXPRESSMinArea = rhs.requiredXPRESSMinArea;
  requireASAPConsistent = rhs.requireASAPConsistent;
  lowToHighASAP = rhs.lowToHighASAP;

  libraAbsoluteValues = rhs.libraAbsoluteValues;

  return *this;
}


Options::Options(const Options & rhs) {
  //cout << "!!! = " << endl;
  queryString_ = rhs.queryString_;
  xmlFileName = rhs.xmlFileName;
  exportSpreadsheet = rhs.exportSpreadsheet;
  sortField = rhs.sortField;
  sortDir = rhs.sortDir;
  page = rhs.page;
  perPage = rhs.perPage;

  displayState = rhs.displayState;

    
//  int i;

  columns.clear();
  for (unsigned i=0; i<rhs.columns.size();i++){ 
    columns.push_back(rhs.columns[i]);
  }

  excludeCharge1 = rhs.excludeCharge1;
  excludeCharge2 = rhs.excludeCharge2;
  excludeCharge3 = rhs.excludeCharge3;
  excludeChargeOthers = rhs.excludeChargeOthers;
  excludeCharges = rhs.excludeCharges;

  std::map<std::string,FilterVec>::const_iterator it;
  filterMap_.clear();
  for (it = (rhs.filterMap_).begin(); 
       it != (rhs.filterMap_).end(); 
       it++) {
    filterMap_[ ((*it).first) ] = (*it).second;
  }

  requireIonscoreGTidentityscore = rhs.requireIonscoreGTidentityscore;
  requireIonscoreGThomologyscore = rhs.requireIonscoreGThomologyscore;
  minimizeTableHeaders = rhs.minimizeTableHeaders;
  highlightPeptideText = rhs.highlightPeptideText;
  includeHighlightedPepTextMods = rhs.includeHighlightedPepTextMods;
  highlightedPeptideText = rhs.highlightedPeptideText;
  highlightProteinText = rhs.highlightProteinText;
  highlightedProteinText = rhs.highlightedProteinText;
  highlightSpectrumText = rhs.highlightSpectrumText;
  highlightedSpectrumText = rhs.highlightedSpectrumText;

  requireAA = rhs.requireAA;
  requiredAA = rhs.requiredAA;
  requireGlyc = rhs.requireGlyc;
  includeBinGlycMotif = rhs.includeBinGlycMotif;

  requirePeptideText = rhs.requirePeptideText;
  includeRequiredPepTextMods = rhs.includeRequiredPepTextMods;
  requiredPeptideText = rhs.requiredPeptideText;
  requireProteinText = rhs.requireProteinText;
  requiredProteinText = rhs.requiredProteinText;
  requireSpectrumText = rhs.requireSpectrumText;
  requiredSpectrumText = rhs.requiredSpectrumText;

  expandProteinList = rhs.expandProteinList;

  requireXPRESS = rhs.requireXPRESS;
  requireXPRESSMinArea = rhs.requireXPRESSMinArea;
  requiredXPRESSMinArea = rhs.requiredXPRESSMinArea;

  requireASAP = rhs.requireASAP;
  requireASAPConsistent = rhs.requireASAPConsistent;
  lowToHighASAP = rhs.lowToHighASAP;

  libraAbsoluteValues = rhs.libraAbsoluteValues;

}





void
Options::generateMinimumSortFilterInfo(vector<string>& sortFilterInfo) {
  map<string, bool> fields;
  sortFilterInfo.clear();
  
  // add the sort field
  fields[sortField.getFieldParamName()] = true;
  //sortFilterInfo.push_back(sortField.getFieldParamName());


  // we need these for the total-subset-stats  
  fields["Gpeptide"] = true;
  fields["GpeptideModText"] = true;

  // protein will include the next col as # alt prot.
  fields["Gprotein"] = true;

  
  // add applicable filters
  if (excludeChargeOthers ||
      excludeCharge1 ||
      excludeCharge2 ||
      excludeCharge3) {
    fields["Gassumed_charge"] = true;
    //sortFilterInfo.push_back("Gassumed_charge");
  }

  if (requireSpectrumText) {
    //sortFilterInfo.push_back("Gspectrum");
    fields["Gspectrum"] = true;
  }

    
  /*
  if (requireGlyc ||
      requireAA ||
      requirePeptideText) {
    //sortFilterInfo.push_back("Gpeptide");
    fields["Gpeptide"] = true;
  }

  if (includeRequiredPepTextMods) {
    fields["GpeptideModText"] = true;
  }

  if (requireProteinText) {
    //sortFilterInfo.push_back("Gprotein");
    fields["Gprotein"] = true;
  }
  */

  

  // go through all the filters.  If any require charge, add assumed charge
  map< std::string, FilterVec >::const_iterator it;
  //cout << endl << "filters:" << endl;

  for (it=filterMap_.begin();it !=filterMap_.end();it++) {
    //string nodeName = (*it).first;
    //cout << "filters for " << nodeName;
    //cout << "(" << (*it).second.filterVec_.size() << " filters)" << endl;
    for (unsigned int j=0; j<(*it).second.filterVec_.size(); j++) {
      if (((*it).second.filterVec_[j])->chargeSpecfication_) {
	fields["Gassumed_charge"] = true;
      }
      fields[(((*it).second.filterVec_[j])->field_).getFieldParamName()] =
	true;
    }

  }


  // add special dependencies

  // mascot
  if (requireIonscoreGThomologyscore) {
    fields["Sionscore"] = true;
    fields["Shomologyscore"] = true;
  }

  if (requireIonscoreGTidentityscore) {
    fields["Sionscore"] = true;
    fields["Sidentityscore"] = true;
  }


  if (requireASAP) {
    fields["Qasapratio"] = true;
  }

  if (requireXPRESS) {
    fields["Qxpress"] = true;
  }

  if (requireASAPConsistent) {
    fields["Qasapratio"] = true;
    fields["Qxpress"] = true;
  }

  if (requireXPRESSMinArea) {
    fields["Qlight_area"] = true;
    fields["Qheavy_area"] = true;
  }
  

  // go through the fields we pulled and add them to our column vector
  map<string, bool>::iterator key;
  for (key=fields.begin(); key != fields.end(); key++) {
    if ( (*key).second == true) {
      sortFilterInfo.push_back((*key).first);
    }
  }
  

}
