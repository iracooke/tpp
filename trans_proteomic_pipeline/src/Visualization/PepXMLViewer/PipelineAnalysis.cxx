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


//for fstat


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <cmath>
#include <sstream>
#include <fstream>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "common/util.h" // tpplib funcs
#include "common/constants.h"
#include "common/TPPVersion.h"

#include "PepXUtility.h"

#include "PipelineAnalysis.h"

using namespace std;
// using std::cout;
// using std::endl;
// using std::map;
// using std::vector;
// using std::string;
// using std::ostream;
// using std::runtime_error;

//using namespace boost;
using boost::to_upper;
using boost::lexical_cast;



PipelineAnalysis::PipelineAnalysis() {
  reset();
}

PipelineAnalysis::~PipelineAnalysis() {
}




void
PipelineAnalysis::reset(void) {
  
  rewroteIndex_ = false;
  
  haveIndexFile_ = false;

  prepedFields_ = false;

  xmlFileName_ = "";

  totalNumSQ_ = 0;
  acceptedSQ_ = 0;

  numUniquePeptides_ = 0;
  numUniqueStrippedPeptides_ = 0;
  numUniqueProteins_ = 0;
  numSingleHitProteins_ = 0;


  mrsOffsets_.clear();
  pepXNodeVec_.clear();
  currentRunSummary_ = 0;
  numRunSummaries_ = 0;
  runSummaryVec_.clear();
  currentRunSummary_ = 0;
  numRunSummaries_ = 0;
  fields_.clear();

  generalInfo_.clear();
  conditions_.clear();

  interact_ = false;
  peptideProphet_ = false;
  ptmProphet_ = false;
  interProphet_ = false;
  asapratio_ = false;
  xpress_ = false;
  libra_ = false;
  libraChannelsMZ_.clear();

  modificationInfo_.clear();

  // get the cgiBase (something like "/tpp-ntasman/cgi-bin/" on linux,
  // "/tpp-bin/" on windows, but instead use the actual URL-path that
  // this script is running from; this way, if the script is moved to
  // another dir, like ("oldDir"), all linkouts will be in the same
  // directory.

  /* generalInfo_["cgiBase"] = CGI_BIN; */
  const char* envTest = getenv("SCRIPT_NAME");
  if (envTest == NULL) {
    // assume commandline mode
    //throw runtime_error(string("unable to access webserver environmental variable SCRIPT_NAME; check your webserver configuration."));
    envTest = "unset/PepXMLViewer.cgi";
  }
  string tmpStr = envTest;
  int slashPos = findRightmostPathSeperator(tmpStr);
  if (slashPos == string::npos) {
    // throw exception
    throw runtime_error(string("unable to parse webserver environmental variable SCRIPT_NAME; check your webserver configuration."));
  }
  generalInfo_["cgiBase"] = tmpStr.substr(0, slashPos+1); // +1 to include the slash character

  // TEMP
  generalInfo_["cgiBaseStd"] = generalInfo_["cgiBase"];

  generalInfo_["cgiName"] = "PepXMLViewer.cgi";



  // web dir:
  generalInfo_["pepXMLResources"] = DEFAULT_HTML_DIR; // from constants.h
  generalInfo_["html_dir"] = DEFAULT_HTML_DIR; // from constants.h


  envTest = getenv("WEBSERVER_ROOT");
  if (envTest == NULL) {
    if (getenv("SCRIPT_NAME") != NULL) { // run by webserver?
      cerr << "unable to access webserver environmental variable WEBSERVER_ROOT; check your webserver configuration." << endl;
    }
    envTest = "default";
  }
  generalInfo_["webserver_root"] = envTest;

  

  conditions_["exportedSpreadsheet"] = false;

}
  









void
PipelineAnalysis::setCurrentRunSummary(int runSumNum) {
  // error if illegal choice?
  currentRunSummary_ = runSumNum;
  generalInfo_["searchEngine"] =  (*(runSummaryVec_[currentRunSummary_]))["searchEngine"];
}


int
PipelineAnalysis::getNumRunSummaries(void) {
  return numRunSummaries_;
}

int
PipelineAnalysis::getCurrentRunSummaryNum(void) {
  return currentRunSummary_;
}


void
PipelineAnalysis::newRunSummary(void) {
  mapPtr newMap(new map<string, string>);
  runSummaryVec_.push_back(newMap);
  ++numRunSummaries_;
  currentRunSummary_ = numRunSummaries_ - 1;

  if (numRunSummaries_ != runSummaryVec_.size()) {
    throw runtime_error(string("inconsistent number of run summaries"));
  }

}
  


// lookup
/*
    
  return the string if it's in the general map
  otherwise, return it if it's in the current run summary map
  otherwise, return empty string
*/

// lookup only!
string 
PipelineAnalysis::operator[](const string& from) {
  static map<string, string>::iterator i;
  i = generalInfo_.find(from);
  if (i != generalInfo_.end()) {
    return (*i).second;
  }
  else {
    return (*(runSummaryVec_[currentRunSummary_]))[from];
  }
}



string& 
PipelineAnalysis::runSummaryParameter(const string& from) {
  return (*(runSummaryVec_[currentRunSummary_]))[from];
}

void PipelineAnalysis::print(void) { // useful for gdb enthusiasts
	print(cout);
}


void 
PipelineAnalysis::print(ostream &out) {
  map<string, string>::iterator i;

  out <<  "header info" << endl;
  out << "-------------" << endl;

  for (i=generalInfo_.begin(); i!=generalInfo_.end(); i++) {
    out <<  (*i).first << " = " << (*i).second << endl;
  }
  out << endl << endl;
    

  out << "fields" << endl;
  out << "------" << endl;


  for (unsigned int j=0; j<fields_.size();j++) {
    out << fields_[j].fieldName_ << endl;
  }
  out << endl << endl;




  for (unsigned int j = 0; j<runSummaryVec_.size(); j++) {
    out << "summary #" << j << endl;
    out << "-------------" << endl;
    for (i=(runSummaryVec_[j])->begin(); i!=(runSummaryVec_[j])->end(); i++) {
      out <<  (*i).first << " = " << (*i).second << endl;
    }
    out << endl << endl;
  }



}






// PrepareFields sets the list of all *available* columns
// Displayed columns are set in PrepareParameters
void
PipelineAnalysis::prepareParameters(Options& options) {
  prepareFields();
  
  if (interProphet_) {
    // if we don't explicately set this here, operator[] falls back to the 
    // first MRS searchEngine
    generalInfo_["searchEngine"] = "(iProphet analysis, combined results)";
  }

  generalInfo_["totalNumSpectra"] = lexical_cast<string>(totalNumSQ_);
  
  generalInfo_["numSubsetSpectra"] = lexical_cast<string>(acceptedSQ_);



  generalInfo_["numUniquePeptides"] = lexical_cast<string>(numUniquePeptides_);
  generalInfo_["numUniqueStrippedPeptides"] = lexical_cast<string>(numUniqueStrippedPeptides_);
  generalInfo_["numUniqueProteins"] = lexical_cast<string>(numUniqueProteins_);
  generalInfo_["numSingleHitProteins"] = lexical_cast<string>(numSingleHitProteins_);


  // which pane, if any is open currently
  generalInfo_["displayState"] = options.displayState;
  generalInfo_["TPPVersionInfo"] = szTPPVersionInfo;


  if (options.perPage == 0) {
    generalInfo_["perPage"] = "all";
  }
  else {
    generalInfo_["perPage"] = lexical_cast<string>(options.perPage);
  }


  // determine the number of rows per page

  // "number of rows per page: all" is coded as 0 from the form's
  // <select>; sort of hackish, I admit.
  int totalPages=0;
  if (options.perPage == 0) {
    totalPages=1; // we know we'll display "1 out of 1" 
  }
  else {
    totalPages = int(ceil( (acceptedSQ_*1.0)/(options.perPage * 1.0)));
  }
  generalInfo_["totalPages"] = lexical_cast<string>(totalPages);


  if (options.page < 1) {
    options.page = 1;
  }

  if (options.page > totalPages) {
    options.page = totalPages;
  }
  generalInfo_["page"] = lexical_cast<string>(options.page);



  generalInfo_["firstPageLink"] = 
    "<a target=\"_self\" title=\"page 1\" onclick=\"document.PepXMLViewerForm.page.value=1;document.PepXMLViewerForm.submit();return true;\" >FIRST</a>"; 
  
  string lp;
  lp =   "<a target=\"_self\" title=\"";
  lp += "page " + lexical_cast<string>(totalPages) + "\" ";
  lp += "onclick=\"document.PepXMLViewerForm.page.value=";
  lp += lexical_cast<string>(totalPages);
  lp += ";document.PepXMLViewerForm.submit();return true;\" >";
  lp += "LAST</a>\n";
  generalInfo_["lastPageLink"] = lp;

  string pl;
  if ((options.page - 1) > 0 ) {
    pl =
      "<a target=\"_self\" title=\"";
    pl += "page " + lexical_cast<string>((options.page - 1)) + "\" ";
    pl += "onclick=\"document.PepXMLViewerForm.page.value=";
    pl += lexical_cast<string>((options.page - 1));
    pl += ";document.PepXMLViewerForm.submit();return true;\" >";
    pl += "PREV</a>\n";
    
    generalInfo_["prevPageLink"] = pl;
  }

  if ((options.page + 1) <= totalPages ) {
    pl =
      "<a target=\"_self\" title=\"";
    pl += "page " + lexical_cast<string>(options.page + 1) + "\" ";
    pl += "onclick=\"document.PepXMLViewerForm.page.value=";
    pl += lexical_cast<string>(options.page + 1);
    pl += ";document.PepXMLViewerForm.submit();return true;\" >";
    pl += "NEXT</a>\n";
    
    generalInfo_["nextPageLink"] = pl;
  }
  
  vector<int> pd;
  pd.push_back(1);
  pd.push_back(2);
  pd.push_back(3);
  pd.push_back(4);
  pd.push_back(5);
  pd.push_back(10);
  pd.push_back(50);
  pd.push_back(100);
  pd.push_back(250);
  pd.push_back(500);
  pd.push_back(1000);


  string pds = " <b>";
  pds += lexical_cast<string>(options.page);
  pds += "</b> ";
  generalInfo_["pageSelectionList"] += pds;

  unsigned int pdc=0;
  while ( (pdc < pd.size()) && (options.page+pd[pdc]) <= totalPages) {
    pds =
      "<a target=\"_self\" title=\"";
    pds += "page " + lexical_cast<string>(options.page + pd[pdc]) + "\" ";
    pds += "onclick=\"document.PepXMLViewerForm.page.value=";
    pds += lexical_cast<string>(options.page + pd[pdc]);
    pds += ";document.PepXMLViewerForm.submit();return true;\" >";
    pds += lexical_cast<string>(options.page + pd[pdc]) + "</a>\n";

    pdc++;
    generalInfo_["pageSelectionList"] += pds;
  }

  pdc=0;
  while ( (pdc < pd.size()) && (options.page-pd[pdc])> 0) {
    pds =
      "<a target=\"_self\" title=\"";
    pds += "page " + lexical_cast<string>(options.page - pd[pdc]) + "\" ";
    pds += "onclick=\"document.PepXMLViewerForm.page.value=";
    pds += lexical_cast<string>(options.page - pd[pdc]);
    pds += ";document.PepXMLViewerForm.submit();return true;\" >";
    pds += lexical_cast<string>(options.page - pd[pdc]) + "</a>\n";

    pdc++;
    generalInfo_["pageSelectionList"] = pds + generalInfo_["pageSelectionList"];
  }


  
  // filters
  map< string, FilterVec >::iterator it;
  //cout << endl << "filters:" << endl;
  for (it=options.filterMap_.begin();it !=options.filterMap_.end();it++) {
    string nodeName = (*it).first;
    //cout << "filters for " << nodeName << "<br/>" << endl;
    //cout << "(" << (*it).second.filterVec_.size() << " filters)" << endl;
    string filterParam;
    for (unsigned int j=0; j<(*it).second.filterVec_.size(); j++) {

      filterParam = ((*it).second.filterVec_[j])->getFilterParamName();

      debug(((*it).second.filterVec_[j])->print(debugout); debugout << "<br/>" << endl);


      //cout << "creating filter parameter " << filterParam << "<br/>" << endl;
      char t[255];
      if (filterParam.rfind("sprank", filterParam.length()) != string::npos) {
	// maintain integer value for sprank-related fields
	sprintf(t, "%d",(int)((*it).second.filterVec_[j])->testValue_);
      }
      else {
	sprintf(t, "%1.4f",((*it).second.filterVec_[j])->testValue_);
      }
      generalInfo_[filterParam] = t;
    } 
  }




  generalInfo_["columns"] = options.getColumnString();


  generalInfo_["xmlFileName"] = options.xmlFileName;

  // make a url path which is relative to the webserver's root
  string webRoot = generalInfo_["webserver_root"];
  if (options.xmlFileName.substr(0, webRoot.length())
      != webRoot) {
    // try this first
    string wafn = options.xmlFileName;
    translate_absolute_filesystem_path_to_relative_webserver_root_path(wafn);

    // assume the filename is already relative to document root
    generalInfo_["webserverAccessableFileName"] = wafn;
  }
  else {
    // strip the document root from the filename
    generalInfo_["webserverAccessableFileName"] = options.xmlFileName.substr(webRoot.length());
  }
#ifdef __CYGWIN__
  // make sure starts with slash
  string wafn = generalInfo_["webserverAccessableFileName"];
  if (wafn[0] != '/') {
    generalInfo_["webserverAccessableFileName"] = string("/") + wafn;
  }
#endif // __CYGWIN__

  string modelsFileName = options.xmlFileName.substr
    (0, options.xmlFileName.rfind('.'))
    +
    "-MODELS.html";
  generalInfo_["modelsFileName"] = modelsFileName;

  string modelsFileNameWeb = generalInfo_["webserverAccessableFileName"].substr
    (0, generalInfo_["webserverAccessableFileName"].rfind('.'))
    +
    "-MODELS.html";
  generalInfo_["modelsFileNameWeb"] = modelsFileNameWeb;


  string checked="checked=\"checked\"";


  if (options.sortDir == Options::ascending) {
    generalInfo_["sortDirAscending"] = checked;
  }
  else {
    generalInfo_["sortDirDescending"] = checked;
  }


  if (options.requireIonscoreGTidentityscore) {
    generalInfo_["requireIonscoreGTidentityscore"] = checked;
  }

  if (options.requireIonscoreGThomologyscore) {
    generalInfo_["requireIonscoreGThomologyscore"] = checked;
  }

  if (options.minimizeTableHeaders) {
    generalInfo_["minimizedTableHeaders"] = checked;
  } 
  else {
    generalInfo_["regularTableHeaders"] = checked;
  } 


  if (options.requireGlyc) {
    generalInfo_["requireGlyc"] = checked;
  }
  if (options.includeBinGlycMotif) {
    generalInfo_["includeBinGlycMotif"] = checked;
  }


  generalInfo_["requiredAA"] = options.requiredAA;

  generalInfo_["requiredPeptideText"] = options.requiredPeptideText;

  if (options.includeRequiredPepTextMods) {
    generalInfo_["includeRequiredPepTextMods"] = checked;
  }

  generalInfo_["requiredProteinText"] = options.requiredProteinText;

  generalInfo_["requiredSpectrumText"] = options.requiredSpectrumText;
  
  if (options.highlightPeptideText) {
    generalInfo_["highlightedPeptideText"] = options.highlightedPeptideText;
  }
  
  if (options.includeHighlightedPepTextMods) {
    generalInfo_["includeHighlightedPepTextMods"] = checked;
  }

  if (options.highlightProteinText) {
    generalInfo_["highlightedProteinText"] = options.highlightedProteinText;
  }

  if (options.highlightSpectrumText) {
    generalInfo_["highlightedSpectrumText"] = options.highlightedSpectrumText;
  }

  if (options.expandProteinList) {
    generalInfo_["expandProteinList"] = checked;
  }
  else {
    generalInfo_["condenseProteinList"] = checked;
  }

  if (options.excludeCharge1) {
    generalInfo_["excludeCharge1"] = checked;
  }

  if (options.excludeCharge2) {
    generalInfo_["excludeCharge2"] = checked;
  }

  if (options.excludeCharge3) {
    generalInfo_["excludeCharge3"] = checked;
  }

  if (options.excludeChargeOthers) {
    generalInfo_["excludeChargeOthers"] = checked;
  }


  if (peptideProphet_) {
    conditions_["peptideProphet"] = true;
  }

  if (ptmProphet_) {
    conditions_["ptmProphet"] = true;
  }


  if (options.requireASAP) {
    generalInfo_["requireASAP"] = checked;
  }

  if (options.requireXPRESS) {
    generalInfo_["requireXPRESS"] = checked;
  }

  if (options.requireXPRESSMinArea) {
    generalInfo_["requireXPRESSMinArea"] = checked;
    string minAreaVal;
    toString(options.requiredXPRESSMinArea, minAreaVal);
    generalInfo_["requiredXPRESSMinArea"] = minAreaVal;
  }

  if (options.libraAbsoluteValues) {
    generalInfo_["absoluteLibraValues"] = checked;
  }
  else {
    generalInfo_["normalizedLibraValues"] = checked;
  }



  if (asapratio_) {
    conditions_["asapratio"] = true;

    // display ratio L:H or H:L?
    if (options.lowToHighASAP) {
      generalInfo_["lowToHighASAP"] = checked;
    }
    else {
      generalInfo_["highToLowASAP"] = checked;
    }


    if (generalInfo_["quantitationList"] != "") {
      generalInfo_["quantitationList"] += ", ";
    }
      
    generalInfo_["quantitationList"] += "ASAPratio";
  }


  if (xpress_) {
    conditions_["xpress"] = true;

    if (generalInfo_["quantitationList"] != "") {
      generalInfo_["quantitationList"] += ", ";
    }
      
    generalInfo_["quantitationList"] += "xpress";

  }


  if (libra_) {
    conditions_["libra"] = true;

    if (generalInfo_["quantitationList"] != "") {
      generalInfo_["quantitationList"] += ", ";
    }
      
    generalInfo_["quantitationList"] += lexical_cast<string>(libraChannelsMZ_.size()) + "-channel libra";

  }



  if (generalInfo_["quantitationList"] == "") {
    generalInfo_["quantitationList"] = "[none]";
  }
 

  if (asapratio_
      &&
      xpress_) {
    conditions_["xpressANDasapratio"] = true;
  }

  if (options.requireASAPConsistent) {
    generalInfo_["requireASAPConsistent"] = checked;
  }



  string searchEngine = (*(runSummaryVec_[0]))["searchEngine"];

  if (interProphet_) {
    conditions_["INTERPROPHET"] = true;
  }
  else {
    if (searchEngine == "SEQUEST") {
      conditions_["SEQUEST"] = true;
    }
    else if (searchEngine == "MASCOT") {
      conditions_["MASCOT"] = true;
    }
    else if (searchEngine == "SPECTRAST") {
      conditions_["SPECTRAST"] = true;
    }
    else if (searchEngine == "PROBID") {
      conditions_["PROBID"] = true;
    }
    // treat x!comet as comet
    else if (searchEngine == "COMET") {
      conditions_["COMET"] = true;
    }
  
    else if (searchEngine == "X!TANDEM" ||
	     searchEngine == "X! TANDEM" ||
	     searchEngine == "X! TANDEM (K-SCORE)") {
      conditions_["XTANDEM"] = true;
    }
    else if (searchEngine == "HYDRA" ||
	     searchEngine == "HYDRA(K-SCORE)") {
      conditions_["HYDRA"] = true;
    }
    // phenyx
    else if (searchEngine == "PHENYX") {
      conditions_["PHENYX"] = true;
    }

    // myrimatch
    else if (searchEngine == "MYRIMATCH") {
      conditions_["MYRIMATCH"] = true;
    }

    // OMSSA
    else if (searchEngine == "OMSSA") {
      conditions_["OMSSA"] = true;
    }

    // CRUX
    else if (searchEngine == "CRUX") {
      conditions_["CRUX"] = true;
    }

    // YABSE
    else if (searchEngine == "YABSE") {
      conditions_["YABSE"] = true;
    }

    // MSGF+
    else if (searchEngine == "MS-GF+") {
      conditions_["MS-GF+"] = true;
    }

  }
  vector<int> pp; 
  pp.push_back(10);
  pp.push_back(25);
  pp.push_back(50);
  pp.push_back(100);
  pp.push_back(250);
  pp.push_back(500);
  pp.push_back(1000);
  pp.push_back(0);

  string p;
  for (unsigned int c=0;c<pp.size();c++) {
    p = generalInfo_["pageSelectionOptions"];
    p += "<option ";
    if (pp[c] == options.perPage) {
      p += "selected=\"selected\" "; 	      
    }
    string perPageValue = lexical_cast<string>(pp[c]);
    string perPageDisplay;
    if (pp[c] == 0) {
      perPageDisplay = "all";
    }
    else {
      perPageDisplay = perPageValue;
    }
      
    p += 	      
      "value=\"" + perPageValue + "\" >" + perPageDisplay + "</option>\n";
    generalInfo_["pageSelectionOptions"] = p;
    //cout << "***c=" << c << endl;
  }





  // cout << "AC size: " << allColumns_.size() << endl;

  // 
  // set up default DISPLAYED column list
  // (all-column list is set up in prepareFields)
  //
  // if options.columns is empty, set it up
  if (options.columns.size() == 0) {

    //cout << "no default cols" << endl;

    //options.columns.push_back(Field("Gindex"));
    
    //options.columns.push_back(Field("Gend_scan"));
    //options.columns.push_back("Gassumed_charge");

    if (peptideProphet_) {
      options.columns.push_back(Field("Pprobability"));
    }

    if (ptmProphet_) {
      options.columns.push_back(Field("Mptm_peptide"));
    }

    options.columns.push_back(Field("Gspectrum"));
    options.columns.push_back(Field("Gstart_scan"));

    string searchEngine = (*(runSummaryVec_[0]))["searchEngine"];

    if (interProphet_) {
      options.columns.push_back(Field("Iiprobability"));
    }
    else {
      // search engine specific
      if (searchEngine == "SEQUEST") {
	options.columns.push_back(Field("Sspscore"));
	if (!peptideProphet_) {  
	  options.columns.push_back(Field("Sxcorr"));
	  options.columns.push_back(Field("Sdeltacn"));
	  options.columns.push_back(Field("Ssprank"));
	  //options.columns.push_back(Field("Sdeltacnstar"));
	}
      }

      else if (searchEngine == "MASCOT") {
	//options.columns.push_back("S");
	if (!peptideProphet_) {  
	  options.columns.push_back(Field("Sionscore"));
	  options.columns.push_back(Field("Sidentityscore"));
	  options.columns.push_back(Field("Shomologyscore"));
	}
	options.columns.push_back(Field("Sexpect"));
      }

      else if (searchEngine == "SPECTRAST") {
	//options.columns.push_back("S");
	options.columns.push_back(Field("Sdot"));
	options.columns.push_back(Field("Sdelta_dot"));
	options.columns.push_back(Field("Sdot_bias"));
	options.columns.push_back(Field("Smz_diff"));
      }

      else if (searchEngine == "PROBID") {
	options.columns.push_back(Field("Sbays_score"));
	options.columns.push_back(Field("Sz_score"));
      }

      else if (searchEngine == "InsPecT") {
	options.columns.push_back(Field("Smqscore"));
	options.columns.push_back(Field("Sexpect"));
	options.columns.push_back(Field("Sfscore"));
	options.columns.push_back(Field("Sdeltascore"));
      }

      else if (searchEngine == "MYRIMATCH") {
	options.columns.push_back(Field("Smvh"));
	if (!peptideProphet_) {
	  options.columns.push_back(Field("Sxcorr"));
	  options.columns.push_back(Field("SmzFidelity"));
	}
	//	options.columns.push_back(Field("SmassError"));
	//	options.columns.push_back(Field("SmzSSE"));
      }

      else if (searchEngine == "COMET") {
	if (!peptideProphet_) {  
	  options.columns.push_back(Field("Sxcorr"));
	  options.columns.push_back(Field("Sdeltacn"));
	}
	options.columns.push_back(Field("Sexpect"));
      }

      else if (searchEngine == "X!TANDEM" ||
	       searchEngine == "X! TANDEM" ||
	       searchEngine == "X! TANDEM (K-SCORE)") {
	if (!peptideProphet_) {  
	  options.columns.push_back(Field("Shyperscore"));
	  options.columns.push_back(Field("Snextscore"));
	}
	//options.columns.push_back(Field("Sbscore"));
	//options.columns.push_back(Field("Syscore"));
	options.columns.push_back(Field("Sexpect"));
      }

      else if (searchEngine == "PHENYX") {
	options.columns.push_back(Field("Szscore"));
	//options.columns.push_back(Field("SorigScore"));
      }

      else if (searchEngine == "OMSSA") {
	options.columns.push_back(Field("Spvalue"));
	options.columns.push_back(Field("Sexpect"));
      }

      else if (searchEngine == "CRUX") {
	options.columns.push_back(Field("Sdelta_cn"));
	options.columns.push_back(Field("Sxcorr_score"));
      }

      else if (searchEngine == "YABSE") {
	options.columns.push_back(Field("SDELTAHYPERGEOMETRIC"));
	options.columns.push_back(Field("SHYPERGEOMETRIC"));
	options.columns.push_back(Field("SNORMEDSNRATIO"));
	options.columns.push_back(Field("SNORMEDSPC"));
	options.columns.push_back(Field("SSNRATIO"));
	options.columns.push_back(Field("SSPC"));
      }

      else if (searchEngine == "MS-GF+") {
	options.columns.push_back(Field("SEValue"));
	if (!peptideProphet_) {
	  options.columns.push_back(Field("Sraw"));
	  options.columns.push_back(Field("Sdenovo"));
	  options.columns.push_back(Field("SSpecEValue"));
	  options.columns.push_back(Field("SIsotopeError"));
	}
      }

    }


    if (searchEngine != "SPECTRAST" ) {
      //options.columns.push_back(Field("Gions"));
      options.columns.push_back(Field("Gions2"));
    }

    //options.columns.push_back(Field("Gnum_tol_term"));
    //options.columns.push_back(Field("Gnum_missed_cleavages"));
    //options.columns.push_back(Field("Gmassdiff"));


    options.columns.push_back(Field("Gpeptide"));
    options.columns.push_back(Field("Gprotein"));
    options.columns.push_back(Field("Gcalc_neutral_pep_mass"));

    if (xpress_) {
      options.columns.push_back(Field("Qxpress"));
    }

    if (asapratio_) {
      options.columns.push_back(Field("Qasapratio"));
    }

    if (libra_) {
      for (unsigned int i=0;i<libraChannelsMZ_.size(); i++) {
	// channel numbering starts from 1
	string libraFieldName = "Qlibra" + lexical_cast<string>(i+1);
	options.columns.push_back(Field(libraFieldName));
      }
    }


    


  }
  
  string s;
  p = "";//generalInfo_["selectedColumnList"];
  s = "";//generalInfo_["sortingOptions"];

  for (unsigned int d=0; 
       d< (options.columns.size()); 
       d++) {


    p+="<option ";
    s+="<option ";

    if (options.columns[d] == options.sortField) {
      s += "selected=\"selected\" ";
    }

    p+= "value=\"" + options.columns[d].getFieldParamName() + "\" >" +
      options.columns[d].getFieldParamName().substr(1) + "</option>\n";

    s+= "value=\"" + options.columns[d].getFieldParamName() + "\" >" +
      options.columns[d].getFieldParamName().substr(1) + "</option>\n";



    //cout << "***" << options.columns[d].substr(1)<< endl;
    //cout << " * p:" << p << endl;
    //cout << " * s:" << s << endl;

  }
  generalInfo_["selectedColumnOptions"] = p;
  generalInfo_["sortingOptions"] = s;


  
  p="";
  for (unsigned int e=0; 
       e< (fields_.size()); 
       e++) {
    bool r = true;
    for (unsigned int f=0; f<options.columns.size();f++){ 
      if (options.columns[f] == fields_[e]) {
	r = false;
	break;
      }
    }
    if (r) {
      // add fields_[e] to p
      p+="<option ";
      
      p+= "value=\"" + fields_[e].getFieldParamName() + "\" >" +
	fields_[e].getFieldParamName().substr(1) + "</option>\n";
    }
  }
  generalInfo_["unselectedColumnList"] = p;



  if (rewroteIndex_) {
    conditions_["rewroteIndex"] = true;
  }

  if (options.exportSpreadsheet) {
    string spreadsheetFileName = options.xmlFileName.substr
      (0, options.xmlFileName.rfind('.')) 
      +
      ".xls";

    generalInfo_["spreadsheetFileName"] = spreadsheetFileName;
    conditions_["exportedSpreadsheet"] = true;
  }
    

  // for help link
  generalInfo_["helpDir"] = DEFAULT_HELP_DIR; // from common/constants.h


  // for status lines
  
  if (options.perPage == 0) {
    generalInfo_["statusPerPage"] = "all";
  }
  else {
    generalInfo_["statusPerPage"] = lexical_cast<string>(options.perPage);
  }
 

  generalInfo_["statusSortField"] = options.sortField.fieldName_;
  if (options.sortDir == Options::ascending) {
    generalInfo_["statusSortDir"] = "ascending";
  }
  else {
    generalInfo_["statusSortDir"] = "descending";
  }

  if (options.highlightedPeptideText == "") {
    generalInfo_["statusHighlightedPeptideText"] = "[none]";
  } 
  else {
    generalInfo_["statusHighlightedPeptideText"] = options.highlightedPeptideText;
  }

  if (options.highlightedProteinText == "") {
    generalInfo_["statusHighlightedProteinText"] = "[none]";
  } 
  else {
    generalInfo_["statusHighlightedProteinText"] = options.highlightedProteinText;
  }

  if (options.highlightedSpectrumText == "") {
    generalInfo_["statusHighlightedSpectrumText"] = "[none]";
  } 
  else {
    generalInfo_["statusHighlightedSpectrumText"] = options.highlightedSpectrumText;
  }


  if (options.expandProteinList) {
    generalInfo_["statusExpandProteins"] = "all hits";
  }
  else {
    generalInfo_["statusExpandProteins"] = "top hit";
  }

  if (options.libraAbsoluteValues) {
    generalInfo_["statusLibraValues"] = "absolute";
  }
  else {
    generalInfo_["statusLibraValues"] = "normalized";
  }
    

  string filterStatus;
  if (options.requireAA ) {
    filterStatus += "required AAs: ";
    filterStatus += options.requiredAA;
    filterStatus += ", ";
  }

  if (options.requireGlyc) {
    filterStatus += "NxS/T motif required, ";
  }
  if (options.requireGlyc && options.includeBinGlycMotif) {
    filterStatus += "using B in glycosolation motif, ";
  }

  if (options.requirePeptideText) {
    filterStatus += "required peptide text: ";
    filterStatus += options.requiredPeptideText;
    filterStatus += ", ";
  }

  if (options.requireProteinText) {
    filterStatus += "required protein text: ";
    filterStatus += options.requiredProteinText;
    filterStatus += ", ";
  }

  if (options.requireSpectrumText) {
    filterStatus += "required spectrum text: ";
    filterStatus += options.requiredSpectrumText;
    filterStatus += ", ";
  }

  if (options.excludeCharge1 ||
      options.excludeCharge2 ||
      options.excludeCharge3 ||
      options.excludeChargeOthers) {
    filterStatus += "excluding charges [ ";
    if (options.excludeCharge1) filterStatus += "+1 ";
    if (options.excludeCharge2) filterStatus += "+2 ";
    if (options.excludeCharge3) filterStatus += "+3 ";
    if (options.excludeChargeOthers) filterStatus += "others ";
    filterStatus += " ], ";
  }

  //map< string, FilterVec >::iterator it;
  //filterStatus += "filters: ";
  for (it=options.filterMap_.begin();it !=options.filterMap_.end();it++) {
    if ((*it).second.filterVec_.size() > 0 ) {
      //string nodeName = (*it).first;
      //filterStatus += nodeName;
      //filterStatus += ": ";
      //cout << "(" << (*it).second.filterVec_.size() << " filters)" << endl;
      for (unsigned int j=0; j<(*it).second.filterVec_.size(); j++) {
	filterStatus += ((*it).second.filterVec_[j])->getSummary();
	filterStatus += ", ";
      }
    }
  }

  if (options.requireIonscoreGTidentityscore) {
    filterStatus += "requiring ionscore &gt; identityscore, ";
  }

  if (options.requireIonscoreGThomologyscore) {
    filterStatus += "requiring ionscore &gt; homologyscore, ";
  }
  
  if (options.requireASAP) {
    filterStatus += "requiring valid ASAPratio, ";
  }

  if (options.requireXPRESS) {
    filterStatus += "requiring valid XPRESS ratio, ";
  }

  if (options.requireASAPConsistent) {
    filterStatus += "requiring xpress ratio within asap mean &plusmn; error ";
  }

  generalInfo_["statusFilteringSummary"] = filterStatus;
  
  


}




void 
PipelineAnalysis::extractHeaderInfo(XMLNodePtr node) {
  XMLNodePtr np = node;


  string name, attrName, attrVal;

  // msms pipeline analysis
  //np = tree_.getRootNode()->findChild(name="msms_pipeline_analysis");
  generalInfo_["msms_pipeline_analysis[@date]"] = np->getAttrValue(attrName="date");
  generalInfo_["msms_pipeline_analysis[@summary_xml]"] = np->getAttrValue(attrName="summary_xml");
  

  // interact
  np = node->
    findDescendent(name = "analysis_summary",
		   attrName="analysis",
		   attrVal="interact");  
  if (!np->isEmpty_) {
    interact_ = true;
    //cout << "found interact analysis marker." << endl;
    generalInfo_["interact[@time]"] = np->getAttrValue(attrName="time");
    np=np->findChild(name="interact_summary");
    generalInfo_["interact[@filename]"] = np->getAttrValue(attrName="filename");
    generalInfo_["interact[@directory]"] = np->getAttrValue(attrName="directory");
    np=np->findChild(name="inputfile");
    generalInfo_["interact[@inputfilename]"] = np->getAttrValue(attrName="name");
  }
  




  // peptide prophet
  np = node->
    findDescendent(name = "analysis_summary",
		   attrName="analysis",
		   attrVal="ptmprophet");  
  if (!np->isEmpty_) {
    ptmProphet_ = true;
    //cout << "found peptideProphet analysis marker." << endl;
    //generalInfo_[];

    generalInfo_["ptmprophet[@time]"] = np->getAttrValue(attrName="time");
    
  }
  np = node->
    findDescendent(name = "analysis_summary",
                   attrName="analysis",
                   attrVal="peptideprophet");
  if (!np->isEmpty_) {
    peptideProphet_ = true;
    //cout << "found peptideProphet analysis marker." << endl;
    //generalInfo_[];

    generalInfo_["peptideprophet[@time]"] = np->getAttrValue(attrName="time");

    np=np->findChild(name="peptideprophet_summary");

    peptideProphetOpts_ = np->getAttrValue("options");

    np=np->findChild(name="inputfile");
    //np->print();
    generalInfo_["peptideprophet[@inputfilename]"] = np->getAttrValue(attrName="name");
  }





  // interProphet
  np = node->
    findDescendent(name = "analysis_summary",
		   attrName="analysis",
		   attrVal="interprophet");  
  if (!np->isEmpty_) {
    interProphet_ = true;
    generalInfo_["interprophet"] = "true";
    //cout << "found interProphet analysis marker." << endl;
    //generalInfo_[];

    generalInfo_["interprophet[@time]"] = np->getAttrValue(attrName="time");

    peptideProphet_ = true;
    

    // TODO: deal with interprophet input files
    //     np=np->findChild(name="inputfile");
    //     generalInfo_["interprophet[@inputfilename]"] = np->getAttrValue(attrName="name");
  }




  // libra
  np = node->
    findDescendent(name = "analysis_summary",
		   attrName="analysis",
		   attrVal="libra");

  if (!np->isEmpty_) {
    libra_ = true;
    //cout << "found libra analysis marker." << endl;

    generalInfo_["libra[@time]"] = np->getAttrValue(attrName="time");
    
    np=np->findChild(name="libra_summary");
    XMLNodeVecPtr channels = np->findChildren("fragment_masses");
  

    int numChannels = (int)channels->size();
    for (int i=0; i<numChannels; i++) {
      libraChannelsMZ_.push_back(0);
    }

    for (int i=0; i<numChannels; i++) {
      XMLNodePtr cNode = ((*channels)[i]);
      int curChannel = lexical_cast<int>(cNode->getAttrValue("channel") );
      // channels start from 1, vector starts from 0
      curChannel--;

      libraChannelsMZ_[curChannel]=
	lexical_cast<int>(cNode->getAttrValue("mz") );
    }
    /*
      <analysis_summary analysis="libra" time="2006-07-14T13:35:25">
      <libra_summary mass_tolerance="0.200" centroiding_preference="2" normalization="0" output_type="1" channel_code="114115116117">
      <fragment_masses channel="1" mz="114"/>
      <fragment_masses channel="2" mz="115"/>
      <fragment_masses channel="3" mz="116"/>
      <fragment_masses channel="4" mz="117"/>
      <isotopic_contributions>
      <contributing_channel channel="1">
      <affected_channel channel="2" correction="0.063"/>
      <affected_channel channel="3" correction="0.000"/>
      <affected_channel channel="4" correction="0.000"/>
      </contributing_channel>
      <contributing_channel channel="2">
      <affected_channel channel="1" correction="0.020"/>
      <affected_channel channel="3" correction="0.060"/>
      <affected_channel channel="4" correction="0.000"/>
      </contributing_channel>
      <contributing_channel channel="3">
      <affected_channel channel="1" correction="0.000"/>
      <affected_channel channel="2" correction="0.030"/>
      <affected_channel channel="4" correction="0.049"/>
      </contributing_channel>
      <contributing_channel channel="4">
      <affected_channel channel="1" correction="0.000"/>
      <affected_channel channel="2" correction="0.000"/>
      <affected_channel channel="3" correction="0.040"/>
      </contributing_channel>
      </isotopic_contributions>
      </libra_summary>
      </analysis_summary>
    */

    /* 
       spectrum query:

       <analysis_result analysis="libra">
       <libra_result>
       <intensity channel="1" target_mass="114.099" absolute="159.040" normalized="159.040"/>
       <intensity channel="2" target_mass="115.115" absolute="136.870" normalized="136.870"/>
       <intensity channel="3" target_mass="116.141" absolute="27.468" normalized="27.468"/>
       <intensity channel="4" target_mass="117.201" absolute="31.654" normalized="31.654"/>
       </libra_result>
       </analysis_result>
    */


  }
  






  // ASAP ratio
  np = node->
    findDescendent(name = "analysis_summary",
		   attrName="analysis",
		   attrVal="asapratio");  
  if (!np->isEmpty_) {
    asapratio_ = true;
    
    generalInfo_["asapratio[@time]"] = np->getAttrValue(attrName="time");
     
    //DDS for ASAPRatio Display Options
    XMLNodePtr np_tmp = np->findDescendent(name = "parameter",
					   attrName="name",
					   attrVal="quantHighBG");  
    if (!np_tmp->isEmpty_) {
      generalInfo_["asapratio[@quantHighBG]"] = np_tmp->getAttrValue(attrName="value");
    }

    np_tmp = np->findDescendent(name = "parameter",
					   attrName="name",
					   attrVal="wavelet");  
    if (!np_tmp->isEmpty_) {
      generalInfo_["asapratio[@wavelet]"] = np_tmp->getAttrValue(attrName="value");
    }

    np_tmp = np->findDescendent(name = "parameter",
				attrName="name",
				attrVal="zeroBG");  
    if (!np_tmp->isEmpty_) {
      generalInfo_["asapratio[@zeroBG]"] = np_tmp->getAttrValue(attrName="value");
    }
    np_tmp = np->findDescendent(name = "parameter",
				attrName="name",
				attrVal="mzBound");  
    if (!np_tmp->isEmpty_) {
      generalInfo_["asapratio[@mzBound]"] = np_tmp->getAttrValue(attrName="value");
    }
    //end DDS
    

    np=np->findChild(name="asapratio_summary");
    
    // pull out asap data here

    //generalInfo_["peptideprophet[@inputfilename]"] = np->getAttrValue(attrName="name");
  }



  // xpress
  np = node->
    findDescendent(name = "analysis_summary",
		   attrName="analysis",
		   attrVal="xpress");  
  if (!np->isEmpty_) {
    xpress_ = true;

    generalInfo_["xpress[@time]"] = np->getAttrValue(attrName="time");
    
    np=np->findChild(name="xpressratio_summary");
    
    // pull out xpress data here
    generalInfo_["xpress[@PpmTol]"] = np->getAttrValue(attrName="ppmtol");

    //generalInfo_["peptideprophet[@inputfilename]"] = np->getAttrValue(attrName="name");
  }


}





// set up columns-- must happen before 1st spec-query parse
// PrepareFields sets the list of all *available* columns
// (The list of initial *displayed* columns is set in PrepareParameters)
void
PipelineAnalysis::prepareFields(void) {
  if (!prepedFields_) {
    string searchEngine = (*(runSummaryVec_[0]))["searchEngine"];
    string precursorMassType = (*(runSummaryVec_[0]))["precursorMassType"];

    //    cerr << "!!!!!" << searchEngine << endl;

    fields_.push_back(Field("Gindex"));
    fields_.push_back(Field("Gstart_scan"));
    fields_.push_back(Field("Gassumed_charge"));
    fields_.push_back(Field("Gprecursor_neutral_mass"));
    fields_.push_back(Field("Gcalc_neutral_pep_mass"));
    fields_.push_back(Field("GMZratio"));
    fields_.push_back(Field("Gprotein_descr"));
    fields_.push_back(Field("GpI"));
    fields_.push_back(Field("Gretention_time_sec"));
    fields_.push_back(Field("Gcompensation_voltage"));
    fields_.push_back(Field("Gprecursor_intensity"));
    fields_.push_back(Field("Gcollision_energy"));

    if (precursorMassType == "1") { // require monoisotopic
      fields_.push_back(Field("Gppm"));    
    }

    if (ptmProphet_) {
      fields_.push_back(Field("Mptm_peptide"));
    }

    if (peptideProphet_) {
      fields_.push_back(Field("Pprobability"));
    }
    fields_.push_back(Field("Gspectrum"));

    if (interProphet_) {
      fields_.push_back(Field("Iiprobability"));
      fields_.push_back(Field("Inss"));
      fields_.push_back(Field("Inss_adj_prob"));
      fields_.push_back(Field("Inrs"));
      fields_.push_back(Field("Inrs_adj_prob"));
      fields_.push_back(Field("Inse"));
      fields_.push_back(Field("Inse_adj_prob"));
      fields_.push_back(Field("Insi"));
      fields_.push_back(Field("Insi_adj_prob"));
      fields_.push_back(Field("Insm"));
      fields_.push_back(Field("Insm_adj_prob"));

      fields_.push_back(Field("Sxcorr"));
      fields_.push_back(Field("Sdeltacn"));
      fields_.push_back(Field("Sdeltacnstar"));
      fields_.push_back(Field("Sspscore"));
      fields_.push_back(Field("Ssprank"));

      fields_.push_back(Field("Sionscore"));
      fields_.push_back(Field("Sidentityscore"));
      fields_.push_back(Field("Shomologyscore"));

      fields_.push_back(Field("Shyperscore"));
      fields_.push_back(Field("Snextscore"));
      fields_.push_back(Field("Sbscore"));
      fields_.push_back(Field("Syscore"));
      fields_.push_back(Field("Sexpect"));

      fields_.push_back(Field("Smvh"));
      fields_.push_back(Field("SmzFidelity"));
      fields_.push_back(Field("Sxcorr"));
      //      fields_.push_back(Field("SmassError"));
      //      fields_.push_back(Field("SmzSSE"));

      fields_.push_back(Field("Smqscore"));
   
      fields_.push_back(Field("Sfscore"));
      fields_.push_back(Field("Sdeltascore"));

      fields_.push_back(Field("Spvalue"));
   

    }
    
    if (!interProphet_) {
      // search engine specific
      if (searchEngine == "SEQUEST") {
	fields_.push_back(Field("Sxcorr"));
	fields_.push_back(Field("Sdeltacn"));
	fields_.push_back(Field("Sdeltacnstar"));
	fields_.push_back(Field("Sspscore"));
	fields_.push_back(Field("Ssprank"));
      }

      else if (searchEngine == "MASCOT") {
	//fields_.push_back(Field("S"));
	fields_.push_back(Field("Sionscore"));
	fields_.push_back(Field("Sidentityscore"));
	fields_.push_back(Field("Shomologyscore"));
      }
      else if (searchEngine == "SPECTRAST") {
	fields_.push_back(Field("Sdot"));
	fields_.push_back(Field("Sdelta_dot"));
	fields_.push_back(Field("Sdot_bias"));
	fields_.push_back(Field("Smz_diff"));
      }
      else if (searchEngine == "PROBID") {
	fields_.push_back(Field("Sbays_score"));
	fields_.push_back(Field("Sz_score"));
      }
      else if (searchEngine == "MYRIMATCH") {
	fields_.push_back(Field("Smvh"));
	fields_.push_back(Field("SmzFidelity"));
	fields_.push_back(Field("Sxcorr"));
	//	fields_.push_back(Field("SmassError"));
	//	fields_.push_back(Field("SmzSSE"));
      }
      else if (searchEngine == "InsPecT") {
	fields_.push_back(Field("Smqscore"));
	fields_.push_back(Field("Sexpect"));
	fields_.push_back(Field("Sfscore"));
	fields_.push_back(Field("Sdeltascore"));
      }
      else if (searchEngine == "COMET") {
	fields_.push_back(Field("Sxcorr"));
	fields_.push_back(Field("Sdeltacn"));
	fields_.push_back(Field("Sdeltacnstar"));
	fields_.push_back(Field("Sspscore"));
	fields_.push_back(Field("Ssprank"));
	fields_.push_back(Field("Sexpect"));
      }

      else if (searchEngine == "X!TANDEM" ||
	       searchEngine == "X! TANDEM" ||
	       searchEngine == "X! TANDEM (K-SCORE)") {
	fields_.push_back(Field("Shyperscore"));
	fields_.push_back(Field("Snextscore"));
	fields_.push_back(Field("Sbscore"));
	fields_.push_back(Field("Syscore"));
	fields_.push_back(Field("Sexpect"));
      }

      else if (searchEngine == "PHENYX") {
	fields_.push_back(Field("Szscore"));
	fields_.push_back(Field("SorigScore"));
      }
    
      else if (searchEngine == "OMSSA") {
	fields_.push_back(Field("Spvalue"));
	fields_.push_back(Field("Sexpect"));
      }

      else if (searchEngine == "CRUX") {
	fields_.push_back(Field("Sdelta_cn"));
	fields_.push_back(Field("Sxcorr_score"));
      }

      else if (searchEngine == "YABSE") {
	fields_.push_back(Field("SDELTAHYPERGEOMETRIC"));
	fields_.push_back(Field("SHYPERGEOMETRIC"));
	fields_.push_back(Field("SNORMEDSNRATIO"));
	fields_.push_back(Field("SNORMEDSPC"));
	fields_.push_back(Field("SSNRATIO"));
	fields_.push_back(Field("SSPC"));
      }

      else if (searchEngine == "MS-GF+") {
	fields_.push_back(Field("Sraw"));
	fields_.push_back(Field("Sdenovo"));
	fields_.push_back(Field("SSpecEValue"));
	fields_.push_back(Field("SEValue"));
	fields_.push_back(Field("SIsotopeError"));
      }

    }


    if (searchEngine != "SPECTRAST" ) {
      fields_.push_back(Field("Gions"));
      fields_.push_back(Field("Gions2"));
    }


    fields_.push_back(Field("Gnum_tol_term"));
    fields_.push_back(Field("Gnum_missed_cleavages"));
    fields_.push_back(Field("Gmassdiff"));


    fields_.push_back(Field("Gpeptide"));
    fields_.push_back(Field("Gprotein"));


    if (xpress_) {
      fields_.push_back(Field("Qxpress"));
      fields_.push_back(Field("Qlight_area"));
      fields_.push_back(Field("Qheavy_area"));
    }

    if (asapratio_) {
      fields_.push_back(Field("Qasapratio"));
      fields_.push_back(Field("Qasapratio_HL"));
    }

    if (libra_) {
      for (unsigned int i=0;i<libraChannelsMZ_.size(); i++) {
	// channel numbering starts from 1
	string libraFieldName = "Qlibra" + lexical_cast<string>(i+1);
	fields_.push_back(Field(libraFieldName));
      }
    }




    if (peptideProphet_) {
      fields_.push_back(Field("Pfval"));
    }
    if (peptideProphetOpts_.find(" PI ", 0) != string::npos) {
      fields_.push_back(Field("PpI_zscore"));
    }
    if (peptideProphetOpts_.find(" RT ", 0) != string::npos) {
      fields_.push_back(Field("PRT"));
      fields_.push_back(Field("PRT_score"));
    }

    prepedFields_ = true;

  }
}


/**
   extract pepXML header information from the msms_run_summary (here,
   "mrs") element, and sub header elements
*/
void 
PipelineAnalysis::extractMRSInfo(XMLNodePtr mrsNode, int mrsNum) {
  newRunSummary(); // now, every following call to runSummaryParameter
		   // will store the information for this mrs only (as
		   // there can be more than one per pepXML file.)
		   // This is a substitute for a full-on mrs model object.

  XMLNodePtr msms_run_summary = mrsNode;

  // msms run summary
  runSummaryParameter("runSummaryRawData") = msms_run_summary->getAttrValue("raw_data");
  runSummaryParameter("runSummaryBaseName") = msms_run_summary->getAttrValue("base_name");
  runSummaryParameter("msManufacturer") = msms_run_summary->getAttrValue("msManufacturer");
  runSummaryParameter("msModel") = msms_run_summary->getAttrValue("msModel");
  runSummaryParameter("msIonization") = msms_run_summary->getAttrValue("msIonization");
  runSummaryParameter("msMassAnalyzer") = msms_run_summary->getAttrValue("msMassAnalyzer");
  //runSummaryParameter("ms") = msms_run_summary->getAttrValue("ms");
  //runSummaryParameter("ms") = msms_run_summary->getAttrValue("ms");

  /*
    cout << "data collected on a " << runSummaryParameter("msManufacturer") 
    << " " << runSummaryParameter("msModel")
    << "(" << runSummaryParameter("msIonization") << " " << runSummaryParameter("msMassAnalyzer")
    << ")." << endl;
  */


  // some ASAP stuff
  runSummaryParameter("asap_time") = msms_run_summary
    ->findChild("analysis_timestamp", "analysis", "asapratio")
    ->getAttrValue("time");
  

  runSummaryParameter("sampleEnzyme") = msms_run_summary->findChild("sample_enzyme")->getAttrValue("name");


  XMLNodePtr search_summary = msms_run_summary->findChild("search_summary");


  // get aminoacid mod info, if any
  XMLNodeVecPtr nvp = search_summary->findChildren("aminoacid_modification");
  for (unsigned int i=0; i<nvp->size(); i++) {
    string aa = (*nvp)[i]->getAttrValue("aminoacid");
    double modDelta = lexical_cast<double>
      ((*nvp)[i]->getAttrValue("massdiff"));
    int modType=-1;
    string typeStr=(*nvp)[i]->getAttrValue("variable");
    to_upper(typeStr);
    if ( typeStr == "Y") {
      modType = AAModInfo::variable_mod;
    }
    else {
      modType = AAModInfo::static_mod;
    }

    AAModInfoPtr mi(new AAModInfo(aa, modDelta, modType));
    (modificationInfo_[currentRunSummary_]).push_back(mi);
  }


  runSummaryParameter("searchSummaryBaseName") = search_summary->getAttrValue("base_name");
  runSummaryParameter("searchSummaryRawData") = search_summary->getAttrValue("raw_data");
  string searchEngine;
  searchEngine=search_summary->getAttrValue("search_engine");
  to_upper(searchEngine); // boost string lib
  runSummaryParameter("searchEngine") = searchEngine;

  runSummaryParameter("precursorMassType") = (search_summary->getAttrValue("precursor_mass_type") == "average") ? "0" : "1";
  runSummaryParameter("fragmentMassType") = (search_summary->getAttrValue("fragment_mass_type") == "average") ? "0" : "1";
  
  runSummaryParameter("searchDatabase") = search_summary->findChild("search_database")->getAttrValue("local_path");



  // go back up to search summary

  // extract some xpress info
  runSummaryParameter("xpress_light") = 
    msms_run_summary
    ->findDescendent("analysis_timestamp", "analysis", "xpress")
    ->findChild("xpressratio_timestamp")
    ->getAttrValue("xpress_light");


  // extract spectrast's spectral library

  // JMT: as of specraST 2.0, this is now "spectral_library");
  XMLNodePtr specLib = 
    search_summary
    ->findDescendent("parameter", "name", "spectra_library");
  
  if (specLib->isEmpty_) { 
    //cout << "did not find spectra_library<br/>" << endl;

    // try spectral_library (with an "l")
    specLib = 
      search_summary
      ->findDescendent("parameter", "name", "spectral_library");

    // if (specLib->isEmpty_) { 
    //   cout << "did not find spectral_library<br/>" << endl;
    // }
    // else {
    //   cout << "found spectral_library<br/>" << endl;
    // }
  
  }
  // else {
  //   cout << "found spectra_library<br/>" << endl;
  // }

     
  if (!specLib->isEmpty_) {
    runSummaryParameter("spectraST_spectraLibrary") = specLib
      ->getAttrValue("value");
    // cout << "speclib is " << runSummaryParameter("spectraST_spectraLibrary") << endl;
    
  }
  // else {
  //   cout << "didn't find speclib" << endl;
  // }






  XMLNodePtr esc = search_summary->findChild("enzymatic_search_constraint");
  string ntt = esc->getAttrValue("min_number_termini");
  if (ntt != "" && lexical_cast<double>(ntt) > 0) {
    runSummaryParameter("searchConstraintMinNtt") =  ntt;
  }
  else {
    runSummaryParameter("searchConstraintMinNtt") =  "0";
  }



  // ion series
  //   SEQUEST: <parameter name="ion_series" value=" 0 0 0 0.0 1.0 0.0 0.0 0.0 0.0 0.0 1.0 0.0"/>
  runSummaryParameter("sequestIonString") = search_summary->name_;
  if (searchEngine == "SEQUEST") {
    XMLNodePtr ionSeriesPtr=search_summary->findChild("parameter", "name", "ion_series");
    if (ionSeriesPtr->isEmpty_) {
      runSummaryParameter("sequestIonString") = "empty";
    }
    else {
      runSummaryParameter("sequestIonString") = ionSeriesPtr->name_ + string(":") + ionSeriesPtr->getAttrValue("value");
    }
    if (!ionSeriesPtr->isEmpty_) {
      runSummaryParameter("sequestIonString") = string("really!") + string(":") + ionSeriesPtr->getAttrValue("value");

      string ionSeries = ionSeriesPtr->getAttrValue("value");
      if (ionSeries != "") { // double checking
	runSummaryParameter("sequestIonString") = ionSeries;
	int tmp1, tmp2, tmp3; // disregard these
	// these floats can be read as bools
	float a=0, b=0, c=0, d=0, v=0, w=0, x=0, y=0, z=0;
	sscanf(ionSeries.c_str(), 
	       " %d %d %d %f %f %f %f %f %f %f %f %f",
	       &tmp1, &tmp2, &tmp3,
	       &a, &b, &c, &d, &v, &w, &x, &y, &z);

	runSummaryParameter("sequestIonSeries") = "1";
	int val;
	val = (int)a;
	runSummaryParameter("a-ions") = val ? "1":"0";

	val = (int)b;
	runSummaryParameter("b-ions") = val ? "1":"0";
     
	val = (int)c;
	runSummaryParameter("c-ions") = val ? "1":"0";

	val = (int)d;
	runSummaryParameter("d-ions") = val ? "1":"0";

	val = (int)v;
	runSummaryParameter("v-ions") = val ? "1":"0";

	val = (int)w;
	runSummaryParameter("w-ions") = val ? "1":"0";

	val = (int)x;
	runSummaryParameter("x-ions") = val ? "1":"0";

	val = (int)y;
	runSummaryParameter("y-ions") = val ? "1":"0";

	val = (int)z;
	runSummaryParameter("z-ions") = val ? "1":"0";
      }
    }
  }

  // X!Tandem version: <parameter name="ion_series" value=" 0 0 0 0.0 1.0 0.0 0.0 0.0 0.0 0.0 1.0 0.0"/>
  if (searchEngine == "X!TANDEM" ||
	     searchEngine == "X! TANDEM" ||
	     searchEngine == "X! TANDEM (K-SCORE)") {
    XMLNodePtr ion;

    ion = search_summary->findChild("parameter", "name", "scoring, a ions");
    if (!ion->isEmpty_) {
      string aions = ion->getAttrValue("value");
      runSummaryParameter("a-ions") = (aions=="yes") ? "1" : "0";
    }
    ion = search_summary->findChild("parameter", "name", "scoring, b ions");
    if (!ion->isEmpty_) {
      string bions = ion->getAttrValue("value");
      runSummaryParameter("b-ions") = (bions=="yes") ? "1" : "0";
    }
    ion = search_summary->findChild("parameter", "name", "scoring, c ions");
    if (!ion->isEmpty_) {
      string cions = ion->getAttrValue("value");
      runSummaryParameter("c-ions") = (cions=="yes") ? "1" : "0";
    }
    ion = search_summary->findChild("parameter", "name", "scoring, x ions");
    if (!ion->isEmpty_) {
      string xions = ion->getAttrValue("value");
      runSummaryParameter("x-ions") = (xions=="yes") ? "1" : "0";
    }
    ion = search_summary->findChild("parameter", "name", "scoring, y ions");
    if (!ion->isEmpty_) {
      string yions = ion->getAttrValue("value");
      runSummaryParameter("y-ions") = (yions=="yes") ? "1" : "0";
    }
    ion = search_summary->findChild("parameter", "name", "scoring, z ions");
    if (!ion->isEmpty_) {
      string zions = ion->getAttrValue("value");
      runSummaryParameter("z-ions") = (zions=="yes") ? "1" : "0";
    }
  }

  //print(); // print mrs info so far (for debugging)

}

