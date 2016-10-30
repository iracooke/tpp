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

/** @file PepXField.cxx
    @brief see PepXField.h
*/

#include <boost/regex.hpp>

#include "../Comet/comet-fastadb/calc_pI.cxx"
#include "mzParser.h"

#include "PepXUtility.h"

#include "PepXField.h"

using boost::regex;
using boost::regex_search;
using boost::match_results;
using boost::match_flag_type;
using boost::match_default;


// forward declare ramp functions, so we don't have to do the giant include
//std::string rampConstructInputFileName(const std::string &basename);



Field::Field() :
  fieldCode_(Field::unknown),
  isValueNode_(false)
{

}


// given "Gindex", "Sxcorr"
Field::Field(const string & name)  :
  isValueNode_(false)
{
  fieldName_ = name.substr(1);
  fieldCode_ = fieldCodeLookup(name[0]);
  nodeNameLookup();
}




// given ("index", "G")
Field::Field(const string & name, char fieldCode) :
  isValueNode_(false)
{
  fieldName_ = name;
  fieldCode_ = fieldCodeLookup(fieldCode);
  nodeNameLookup();
}


void
// given ("Gindex")
Field::set(const string & name, char fieldCode) {
  isValueNode_ = false;
  fieldName_ = name;
  fieldCode_ = fieldCodeLookup(fieldCode);
  nodeNameLookup();
}




/**

   clarify which pepXML node a field attribute belongs to.

   (up to date with pepXML v18)

 */

void
Field::nodeNameLookup(void) {
  //cout << "lookup: " << fieldName << endl;

  if (fieldCode_ == Field::general) {
    if (fieldName_ == "spectrum" ||
	fieldName_ == "start_scan" ||
	fieldName_ == "end_scan" ||
	fieldName_ == "precursor_neutral_mass" ||
	fieldName_ == "assumed_charge" ||
	fieldName_ == "index" ||
	fieldName_ == "search_specification" // optional
	) {
      nodeName_ = "spectrum_query";
      isValueNode_ = false;
    }
    else {
      /*

	assume it's a search_hit field; skip the specific tests to
	keep things flexible for future changes.

	if (fieldName_ == "hit_rank" ||
	fieldName_ == "peptide" ||
	fieldName_ == "peptide_prev_aa" || // optional
	fieldName_ == "peptide_next_aa" || // optional
	fieldName_ == "protein" ||
	fieldName_ == "num_tot_proteins" ||
	fieldName_ == "num_matched_ions" || // optional
	fieldName_ == "tot_num_ions" || // optional
	fieldName_ == "calc_neutral_pep_mass" ||
	fieldName_ == "massdiff" ||
	fieldName_ == "num_tol_term" || // optional
	fieldName_ == "num_missed_cleavages" || // optional
	fieldName_ == "is_rejected" || // optional
	fieldName_ == "protein_descr" // optional
	) {
      */
      nodeName_ = "search_hit";
      isValueNode_ = false;
    }
  }
  else if (fieldCode_ == Field::searchScore) {
    // one size fits all
    nodeName_ = "search_score";
      isValueNode_ = true;
  }
  else if (fieldCode_ == Field::ptmProphet) {
    if (fieldName_ == "ptm_peptide") {
      nodeName_ = "ptmprophet_result";
      isValueNode_ = false;
    }
    else {
      nodeName_ = "mod_aminoacid_probability";
      isValueNode_ = false;
    }
  }
  else if (fieldCode_ == Field::peptideProphet) {
    if (fieldName_ == "probability") {
      nodeName_ = "peptideprophet_result";
      isValueNode_ = false;
    }
    else {
      nodeName_ = "parameter";
      isValueNode_ = true;
    }
  }
  else if (fieldCode_ == Field::interProphet) {
    if (fieldName_ == "iprobability") {
      nodeName_ = "interprophet_result";
      isValueNode_ = false;
    }
    else {
      nodeName_ = "parameter";
      isValueNode_ = true;
    }
  }
  else if (fieldCode_ == Field::quantitation) {
    if (fieldName_ == "xpress") {
      nodeName_ = "xpressratio_result";
      isValueNode_ = false;
    }
    else if (fieldName_ == "asapratio" ||
	     fieldName_ == "asapratio_HL") {
      nodeName_ = "asapratio_result";
      isValueNode_ = false;
    }
    else if (fieldName_.substr(0,5) == "libra") {
      nodeName_ = "libra_result";
      isValueNode_ = false;
    }
  }
  // modify the valuenode names
  if (isValueNode_) {
    nodeName_ += "[@" + fieldName_ + "]";
  }


}






// returns enum code
int
Field::fieldCodeLookup(char code) {
  int result = Field::unknown;

  switch (code) {
  case 'G':
    result = Field::general;
    break;
  case 'S':
    result = Field::searchScore;
    break;
  case 'M':
    result = Field::ptmProphet;
    break;
  case 'P':
    result = Field::peptideProphet;
    break;
  case 'I':
    result = Field::interProphet;
    break;
  case 'Q':
    result = Field::quantitation;
    break;

  default:
    result = Field::unknown;
    break;
  }

  return result;
}







// returns "Gions", "Sxcorr", etc
string
Field::getFieldParamName(void) const {
  string result="";

  switch (fieldCode_) {
  case Field::general:
    result += "G";
    break;
  case Field::searchScore:
    result += "S";
    break;
  case Field::ptmProphet:
    result += "M";
    break;
  case Field::peptideProphet:
    result += "P";
    break;
  case Field::interProphet:
    result += "I";
    break;
  case Field::quantitation:
    result += "Q";
    break;
  default:
    result += "?";
    break;
  }

  result += fieldName_;
  return result;
}






bool
Field::operator==(const Field& rhs) {
  if (rhs.fieldName_ == fieldName_
      &&
      rhs.fieldCode_ == fieldCode_) {
    return true;
  }
  else {
    return false;
  }
}










string
Field::getAbbreviation(void) {
  string text = "";

  switch (fieldCode_) {



  case Field::quantitation:
    // no abbreviations here yet

    /*
    // xpress
    if (fieldName_ == "xpress") {
    }

    // libra
    else if (fieldName_.substr(0,5) == "libra") {
    }

    // asap
    else if (fieldName_ == "asapratio") {
    }
    */
    break;

  case Field::general: // general

    // straightforward spectrum_query attrs
    if (fieldName_ == "index") {
      text = "#";
    }
    else if (fieldName_ == "assumed_charge") {
      text = "z";
    }
    else if (fieldName_ == "calc_neutral_pep_mass") {
      text = "calc_mass";
    }
    else if (fieldName_ == "precursor_neutral_mass") {
      text = "exp_mass";
    }
    else if (fieldName_ == "start_scan") {
      text = "sscan";
    }
    else if (fieldName_ == "end_scan") {
      text = "escan";
    }

    else if (fieldName_ == "spscore") {
       text = "E-value";
    }

    /*
    // spectrum
    else if (fieldName_ == "spectrum") {
      text = "spec";
    }

    // ppm
    else if (fieldName_ == "ppm") {
    }


    // ions
    else if (fieldName_ == "ions") {
    }


    // peptide
    else if (fieldName_ == "peptide") {
    }

    // protein
    else if (fieldName_ == "protein") {
    }
    */

    else if (fieldName_ == "num_tol_term") {
      text = "ntt";
    }

    else if (fieldName_ == "num_missed_cleavages") {
      text = "nmc";
    }
    else if (fieldName_ == "pI_zscore") {
      text = "pI_zscore";
    }
    else if (fieldName_ == "RT") {
      text = "RT";
    }
    else if (fieldName_ == "RT_score") {
      text = "RT_score";
    }

    break;



  case Field::searchScore: // search scores
    // unchanged for now
    break;


  case Field::ptmProphet:

    // PROBABILITY
    if (fieldName_=="ptm_peptide") {
      text = "ptm_peptide";
    }

    /*
    // assume it's a parameter node: fval, etc
    else {
    }
    */

    break;
  case Field::peptideProphet:

    // PROBABILITY
    if (fieldName_=="probability") {
      text = "prob";
    }

    /*
    // assume it's a parameter node: fval, etc
    else {
    }
    */

    break;
    // end Field::probability


  case Field::interProphet:

    // PROBABILITY
    if (fieldName_=="iprobability") {
      text = "iprob";
    }

    /*
    // assume it's a parameter node: fval, etc
    else {
    }
    */

    break;
    // end Field::probability

  default:
    break;
  }



  if (text=="") {
    // if we didn't abbr, just the name.
    text = fieldName_;
  }

  return text;


}








string
Field::getDescription(const string& searchEngine) {

  switch (fieldCode_) {
  case 'G':
    if (fieldName_ == "index")
      return string("index in pepXML file");
    else if (fieldName_ == "ions")
      return string("# matched ions / # calculated ions");
    break;
  case 'S':
    if (searchEngine == "SEQUEST") {
      if (fieldName_ == "xcorr") {
	return string("SEQUEST: cross-correlation");
      }
      else if (fieldName_ == "deltacn") {
	return string("SEQUEST: separation of xcorr of top spectrum match from next-best match.");
      }
    }
    break;
  case 'P':
    if (fieldName_ == "probability")
      return string("Peptide Prophet: probability");
    break;
  case 'I':
    if (fieldName_ == "iprobability")
      return string("iProphet: probability");
    break;
  default:
    break;
  }


  // by default just return the field name
  return fieldName_;

}













string
Field::getText(int mode,
	       const XMLNodePtr& node,
	       PMap& infoMap,
	       const string &highlightedPeptideText,
	       const string &highlightedProteinText,
	       const string &highlightedSpectrumText,
	       const bool includeHighlightedPepTextMods,
	       const bool libraAbsoluteValues) {
  string text = "";
  char t[255]; // temp buffer for formatting
  string name;
  XMLNodePtr np;
  string pname="";




  // used often
  string searchEngine = infoMap["searchEngine"];


  switch (fieldCode_)
  {



  case Field::quantitation:
    // xpress
    if (fieldName_ == "xpress") {

      // use decimal_ratio (low:high)

      if (mode == Field::plainText) {
	text=
	  node
	  ->findChild("search_result")
	  ->findChild("search_hit", "hit_rank", "1")
	  ->findChild("analysis_result","analysis","xpress")
	  ->findChild("xpressratio_result")
	  ->getAttrValue("decimal_ratio");
      }
      else if (mode == Field::html) {

	XMLNodePtr xpressratio_result =
	  node
	  ->findChild("search_result")
	  ->findChild("search_hit", "hit_rank", "1")
	  ->findChild("analysis_result","analysis","xpress")
	  ->findChild("xpressratio_result");


	if ( xpressratio_result->getAttrValue("decimal_ratio")
	     ==
	     "" ) {
	  // n/a
	  text = "unavailable";
	}
	else {



	  text = "<a target=\"Win1\" href=\"";
	  text += infoMap["cgiBase"] + "XPressPeptideUpdateParser.cgi?";



	  string light_first_scan = xpressratio_result->getAttrValue("light_firstscan");
	  string light_last_scan = xpressratio_result->getAttrValue("light_lastscan");
	  string heavy_first_scan = xpressratio_result->getAttrValue("heavy_firstscan");
	  string heavy_last_scan = xpressratio_result->getAttrValue("heavy_lastscan");
	  string xpress_charge = node->getAttrValue("assumed_charge");
	  string LightMass = xpressratio_result->getAttrValue("light_mass");
	  string HeavyMass = xpressratio_result->getAttrValue("heavy_mass");
	  string MassTol = xpressratio_result->getAttrValue("mass_tol");		  
	  string xpress_index = node->getAttrValue("index");
	  string xpress_spec = node->getAttrValue("spectrum");

	  string xmlfile = infoMap["xmlFileName"];

	  string basename = infoMap["runSummaryBaseName"]; // per MRS
	  string rawdata = infoMap["runSummaryRawData"]; // per MRS
	  string xpress_display = infoMap["xpress_light"]; // per MRS
	  string PpmTol = infoMap["xpress[@PpmTol]"]; // per MRS


	  text += "LightFirstScan=" + light_first_scan + "&amp;";
	  text += "LightLastScan=" + light_last_scan + "&amp;";
	  text += "HeavyFirstScan=" + heavy_first_scan + "&amp;";
	  text += "HeavyLastScan=" + heavy_last_scan + "&amp;";
	  text += "XMLFile=" + rampConstructInputFileName(basename) + "&amp;";
	  text += "ChargeState=" + xpress_charge + "&amp;";
	  text += "LightMass=" + LightMass + "&amp;";
	  text += "HeavyMass=" + HeavyMass + "&amp;";
	  text += "MassTol=" + MassTol + "&amp;";
	  text += "PpmTol=" + PpmTol + "&amp;";
	  text += "index=" + xpress_index + "&amp;";
	  text += "xmlfile=" + xmlfile + "&amp;";
	  text += "OutFile=" + xpress_spec + "&amp;";
	  text += "bXpressLight1=" + xpress_display;

	  text += " \">";


	  text +=
	    node
	    ->findChild("search_result")
	    ->findChild("search_hit", "hit_rank", "1")
	    ->findChild("analysis_result","analysis","xpress")
	    ->findChild("xpressratio_result")
	    ->getAttrValue("decimal_ratio");


	  text += "</a>";
	}
	// end xpress html
      }
      else if (mode == Field::value) {
	text = node
	  ->findChild("search_result")
	  ->findChild("search_hit", "hit_rank", "1")
	  ->findChild("analysis_result","analysis","xpress")
	  ->findChild("xpressratio_result")
	  ->getAttrValue("decimal_ratio");
      }
      // end field "xpress"
    }


    if (fieldName_ == "light_area") {
      text = node
	->findChild("search_result")
	->findChild("search_hit", "hit_rank", "1")
	->findChild("analysis_result","analysis","xpress")
	->findChild("xpressratio_result")
	->getAttrValue("light_area");
    }


    if (fieldName_ == "heavy_area") {
      text = node
	->findChild("search_result")
	->findChild("search_hit", "hit_rank", "1")
	->findChild("analysis_result","analysis","xpress")
	->findChild("xpressratio_result")
	->getAttrValue("heavy_area");
    }



    // libra
    if (fieldName_.substr(0,5) == "libra") {
      if (mode == Field::plainText ||
	  mode == Field::html ||
	  mode == Field::value) {
	XMLNodePtr lNode =
	  node
	  ->findChild("search_result")
	  ->findChild("search_hit", "hit_rank", "1")
	  ->findChild("analysis_result","analysis","libra")
	  ->findChild("libra_result")
	  ->findDescendent("intensity", "channel", fieldName_.substr(5));

	if (libraAbsoluteValues) {
	  text = lNode->getAttrValue("absolute");
	}
	else {
	  text = lNode->getAttrValue("normalized");
	}
      }
    }



    // asap
    else if (fieldName_ == "asapratio" || // L:H
	     fieldName_ == "asapratio_HL") {

      string asapMeanAttr;
      string asapErrorAttr;
      if (fieldName_ == "asapratio_HL") {
	asapMeanAttr = "heavy2light_mean";
	asapErrorAttr = "heavy2light_error";
      }
      else { // L:H, default
	asapMeanAttr = "mean";
	asapErrorAttr = "error";
      }

      if (mode == Field::value) {
      text=
	node
	->findChild("search_result")
	->findChild("search_hit", "hit_rank", "1")
	->findChild("analysis_result","analysis","asapratio")
	->findChild("asapratio_result")
	->getAttrValue(asapMeanAttr);
      }
      else if (mode == Field::html) {

	XMLNodePtr asapratio_result =
	  node
	  ->findChild("search_result")
	  ->findChild("search_hit", "hit_rank", "1")
	  ->findChild("analysis_result","analysis","asapratio")
	  ->findChild("asapratio_result");

	if (asapratio_result->getAttrValue(asapMeanAttr)
	    ==
	    "") {
	  // not found
	  text = "N/A";
	}
	else {
	  text = "<a target=\"Win1\" href=\"";
	  text += infoMap["cgiBase"] + "ASAPRatioPeptideCGIDisplayParser.cgi?";

	  string xpress_index = node->getAttrValue("index");
	  string xpress_spec = node->getAttrValue("spectrum");


	  text += "Xmlfile="  + infoMap["xmlFileName"] + "&amp;";
	  text += "Basename=" + infoMap["runSummaryBaseName"] + "&amp;";
	  text += "Indx=" + xpress_index + "&amp;";
	  text += "Timestamp=" + infoMap["asap_time"] + "&amp;";
	  text += "Spectrum=" + xpress_spec + "&amp;";

	  // NOTE: there was originally a choice to display light:heavy or heavy:light;
	  // with 0 for light:heavy and 1 for the opposite.  This could be added back in as a choice, if desired.
	  text += "ratioType=0";

	  //DDS ASAP quantitation opts
	  if (infoMap["asapratio[@quantHighBG]"] == "True") {
	    text += "&amp;quantHighBG=1";
	  }
	  if (infoMap["asapratio[@wavelet]"] == "True") {
	    text += "&amp;wavelet=1";
	  }
	  else if (infoMap["asapratio[@wavelet]"] == "False") {
	    text += "&amp;wavelet=0";
	  }
	  if (infoMap["asapratio[@zeroBG]"] == "True") {
	    text += "&amp;zeroBG=1";
	  }
	  if (atof(infoMap["asapratio[@mzBound]"].c_str()) > 0) {
	    text += "&amp;mzBound="+infoMap["asapratio[@mzBound]"];
	  }
	  //end DDS ASAP quantitation opts

	  text += " \">";

	  text +=
	    node
	    ->findChild("search_result")
	    ->findChild("search_hit", "hit_rank", "1")
	    ->findChild("analysis_result","analysis","asapratio")
	    ->findChild("asapratio_result")
	    ->getAttrValue(asapMeanAttr);
	  text += " &#177; ";
	  text += node
	    ->findChild("search_result")
	    ->findChild("search_hit", "hit_rank", "1")
	    ->findChild("analysis_result","analysis","asapratio")
	    ->findChild("asapratio_result")
	    ->getAttrValue(asapErrorAttr);

	  text += "</a>";


	}
      }
      else if (mode == Field::plainText) {
	// actually returns as TWO tab-delim columns: mean and error
	text=
	  node
	  ->findChild("search_result")
	  ->findChild("search_hit", "hit_rank", "1")
	  ->findChild("analysis_result","analysis","asapratio")
	  ->findChild("asapratio_result")
	  ->getAttrValue(asapMeanAttr);
	if (text == "") {
	  text = "\t";
	  break;
	}
	text += "\t";
	text += node
	  ->findChild("search_result")
	  ->findChild("search_hit", "hit_rank", "1")
	  ->findChild("analysis_result","analysis","asapratio")
	  ->findChild("asapratio_result")
	  ->getAttrValue(asapErrorAttr);
      }
    }
    break;

  case Field::general: // general


    // straightforward spectrum_query attrs
    if (fieldName_ == "index" ||
	fieldName_ == "assumed_charge" ||
	fieldName_ == "precursor_neutral_mass" ||
	fieldName_ == "start_scan" ||
	fieldName_ == "end_scan" ||
	fieldName_ == "search_specification" ||
	fieldName_ == "collision_energy" ||
	fieldName_ == "compensation_voltage" ||
	fieldName_ == "precursor_intensity" ||
	fieldName_ == "retention_time_sec"
	) {
      if (mode == Field::value ||
	  mode == Field::plainText ||
	  mode == Field::html) {
	text = node->getAttrValue(fieldName_);
      }
    }




    // spectrum
    else if (fieldName_ == "spectrum") {
      if (mode == Field::value ||
	  mode == Field::plainText ) {
	text = node->getAttrValue(fieldName_);
      }
      else if (mode == Field::html) {

	string spectrum=node->getAttrValue("spectrum");
	string displayedSpectrumText = spectrum;
	searchEngine =  infoMap["searchEngine"];
	// highlight the spectrum text, if requested
	/* regex highlighting */
	if (highlightedSpectrumText.size() > 0 ) {
	  string::const_iterator textStart, textEnd;
	  textStart = spectrum.begin();
	  textEnd = spectrum.end();

	  match_results<string::const_iterator> what;
	  match_flag_type flags = match_default;
	  //int startPos = 0;

	  regex re2(highlightedSpectrumText);
	  if (regex_search(textStart, textEnd, what, re2, flags)) {
	    displayedSpectrumText =
	      string("<span class=\"highlight\">")
	      + spectrum
	      + string("</span>");

	  }
	}

	if (searchEngine == "MASCOT") {
	  text = "<a target=\"Win1\" href=\"";
	  text += infoMap["cgiBase"] + "mascotout.pl?OutFile=";
	  text += infoMap["runSummaryBaseName"] + "/" + spectrum + ".out";
	  text += "\">" + displayedSpectrumText + "</a>";
	}
	else if (searchEngine == "SPECTRAST") {
	  text = "<a target=\"Win1\" href=\"";
	  text += infoMap["cgiBaseStd"] + "plotspectrast.cgi?LibFile=";
	  text += infoMap["spectraST_spectraLibrary"];
	  text += "&LibFileOffset=" +
	    node
	    ->findChild("search_result")
	    ->findChild("search_hit", "hit_rank", "1")
	    ->findDescendent("search_score[@lib_file_offset]")
	    ->getAttrValue("value");

	  string queryfile;
	  queryfile = infoMap["runSummaryBaseName"].substr
	    (0,
	     findRightmostPathSeperator(infoMap["runSummaryBaseName"])+1);
	  queryfile += spectrum.substr(0, spectrum.find('.'));
	  queryfile += infoMap["runSummaryRawData"];

	  //text += "&QueryFile=" + rampConstructInputFileName(queryfile);
	  text += "&QueryFile=" + queryfile;
	  text += "&QueryScanNum=" + node->getAttrValue("start_scan");
	  text += "\">" + displayedSpectrumText + "</a>";
	}
	else if (searchEngine == "SEQUEST" ||
		 searchEngine == "COMET") {
	  text = "<a target=\"Win1\" href=\"";
	  text += infoMap["cgiBase"] + "comet-pepxml.cgi?File=";
	  text += infoMap["searchSummaryBaseName"] + ".pep.xml/" + spectrum;
	  text += "\">" + displayedSpectrumText + "</a>";
	}
	// else, unspecified search engine.
	else {
	  text = displayedSpectrumText;
	}



	// spectrast over peptide atlas
	text += "<a target=\"SpectrastPA\" href=\"";
	text += infoMap["html_dir"] + "post_to_spectrast.html?";
	// query string

	//name of fasta file used (just name, not full path)
	text += "fasta=";
	string dbase=infoMap["searchDatabase"];
	// remove everything except the actual filename
	string::size_type slashPos = findRightmostPathSeperator(dbase);
	if (slashPos != string::npos) {
	  // we can just get the filename (after the last slash)
	  dbase = dbase.substr(slashPos+1);
	}
	text += dbase;

	// path to where cgi's live. (i.e. where the PepXMLViewer is)
	text += "&amp;tppcgi=" + infoMap["cgiBase"];

	// full path to dta file
	text += "&amp;Dta=";
	//string spectrum=node->getAttrValue("spectrum");
	text += infoMap["runSummaryBaseName"] + "/" + spectrum + ".dta";

/*
	// (optional)COMET=1 (if Comet results; logic same as for plot-msms.cgi)
	if (searchEngine == "COMET" ||
	    searchEngine == "X!COMET" ||
	    searchEngine == "X! COMET" ||
	    searchEngine == "X! TANDEM (COMET)") {
	  text += "&amp;COMET=1";
	}
*/

	text += "\">";

	text += "<img border=\"0\" src=\"";
	text += infoMap["pepXMLResources"]; // includes slash
	text += "images/spectrast_tiny.png\" alt=\"SpectraST\" /></a>";


      } // end spectrum, html
    } // end spectrum






    // ppm
    // TODO: check precision?

    else if (fieldName_ == "ppm") {
      if (mode == Field::value ||
	  mode == Field::plainText ||
	  mode == Field::html) {

	double calc, measured, Z;

	if (!toDouble(node->getAttrValue("precursor_neutral_mass"), measured) ||
	    !toDouble(node
		      ->findChild("search_result")
		      ->findChild("search_hit", "hit_rank", "1")
		      ->getAttrValue("calc_neutral_pep_mass"), calc) ||
	    !toDouble(node
		      ->getAttrValue("assumed_charge"), Z) ) {
	  // bad value(s)
	  text = "";
	}
	else {
	  //sprintf(t, "%.4f", 1000000.0*(calc-measured)/measured);
	  static double protonMass = 1.00727646688;
	  sprintf(t, "%.4f", 1000000.0*( measured - calc )/(measured+Z*protonMass));
	  text = t;
	}
      }
    }


    // pI calculation
    else if (fieldName_ == "pI") {
      if (mode == Field::value ||
	  mode == Field::plainText ||
	  mode == Field::html) {
	// call Comet's calc_pI code
	// note-- doesn't take modifications into account
	string pep = node->
	  findChild("search_result")->
	  findChild("search_hit", "hit_rank", "1")->getAttrValue("peptide");
	int peplen = (int)pep.length();
	char* pepString = new char[peplen + 1];
	strcpy(pepString, pep.c_str());
	double pI = COMPUTE_PI(pepString, peplen, 0);
	delete[] pepString;
	sprintf(t, "%.2f", pI);
	text=t;
      }
    }



    // M/Z ratio
    //  (calc. neutral mass + Z*(H+mass)) / Z
    else if (fieldName_ == "MZratio") {
      if (mode == Field::value ||
	  mode == Field::plainText ||
	  mode == Field::html) {
	
	double calc; // calculated mass
	double Z; // assumed charge
	// from
	// http://physics.nist.gov/cgi-bin/cuu/Value?mpu|search_for=proton+mass:
	static double protonMass = 1.00727646688;

	if (!toDouble(node
		      ->findChild("search_result")
		      ->findChild("search_hit", "hit_rank", "1")
		      ->getAttrValue("calc_neutral_pep_mass"), calc) ||
	    !toDouble(node
		      ->getAttrValue("assumed_charge"), Z)) {
	  text = "";
	}
	else if (mode == Field::html) {

	  sprintf(t, "%.4f", (calc + Z*protonMass) / Z );
	  text = "<a target=\"Win1\" href=\"";
	  text += infoMap["cgiBase"] + "ShowXIC.cgi?FILE=";
	  text += infoMap["runSummaryBaseName"] + infoMap["runSummaryRawData"]+ "&amp;MZ="+t;
	  text += "\">";
	  text += t;
	  text += "</a>";
	}
		  
	else {
	  sprintf(t, "%.4f", (calc + Z*protonMass) / Z );
	  text += t;
	}

      }
    }


    // massdiff: previously, used value from pepxml file, which was only printed to 1 decimal place.
    //  node[@precursor_neutral_mass] - (node->search_result->search_hit[@calc. neutral mass]
    else if (fieldName_ == "massdiff") {
      if (mode == Field::value ||
	  mode == Field::plainText ||
	  mode == Field::html) {

	double calcmass;
	double precursorneutralmass;

	if (!toDouble(node
		      ->findChild("search_result")
		      ->findChild("search_hit", "hit_rank", "1")
		      ->getAttrValue("calc_neutral_pep_mass"), calcmass) ||
	    !toDouble(node
		      ->getAttrValue("precursor_neutral_mass"), precursorneutralmass)) {
	  text = "";
	}
	else {
	  sprintf(t, "%.4f", (precursorneutralmass - calcmass));
	  text = t;
	}
      }
    }





    // ions
    else if (fieldName_ == "ions") {
      if (mode == Field::value ||
	  mode == Field::plainText) {
	text = node->
	  findChild("search_result")->
	  findChild("search_hit", "hit_rank", "1")->getAttrValue("num_matched_ions");
	text += "/";
	text += node->
	  findChild("search_result")->
	  findChild("search_hit", "hit_rank", "1")->getAttrValue("tot_num_ions");
      }
      else if ( mode == Field::html) {

	string spectrum=node->getAttrValue("spectrum");
	text = "<a target=\"Win1\" href=\"";
	text += infoMap["cgiBase"] + "plot-msms.cgi?Expect=1&amp;MassType=";
	text += infoMap["precursorMassType"];

	text += "&amp;NumAxis=1";

	{
	  // specify ion series to the viewer.
	  // from Jimmy:
	  // plot-msms handles a,b,c and x,y,z ions for charges 1, 2, 3
	  // a,b,c >=+2 should include charges <2
	  // x,y,z >=+2 should include charges <=2

	  bool a=false, b=false, c=false, d=false, v=false, w=false, x=false, y=false, z=false;
	  // the strings are "1" or "0"-- there's probably a more
	  // elegant way, but...

	  // (ignoring possible bad casts; will be set to false)
	  toBool(infoMap["a-ions"], a);
	  toBool(infoMap["b-ions"], b);
	  toBool(infoMap["c-ions"], c);
	  toBool(infoMap["d-ions"], d);
	  toBool(infoMap["v-ions"], v);
	  toBool(infoMap["w-ions"], w);
	  toBool(infoMap["x-ions"], x);
	  toBool(infoMap["y-ions"], y);
	  toBool(infoMap["z-ions"], z);

	  //text += "&amp;origSequest=" + infoMap["sequestIonString"];
	  //text += "&amp;sequestions=" + infoMap["a-ions"] + infoMap["b-ions"] + infoMap["c-ions"] + infoMap["d-ions"] + infoMap["v-ions"] + infoMap["w-ions"] + infoMap["x-ions"] + infoMap["y-ions"] + infoMap["z-ions"];



	  int charge;
	  if (!toInt(node
		     ->getAttrValue("assumed_charge"), charge)) {
	    charge=-1; // TODO: something better?
	  }

	  if (a || b || c || d) {
	    if (charge >= 1) { // show +1 for all changes
	      if (a) {
		text += "&amp;ShowA=1";
	      }
	      if (b) {
		text += "&amp;ShowB=1";
	      }
	      if (c) {
		text += "&amp;ShowC=1";
	      }
	      // if (d) {
	      // 	text += "&amp;ShowD=1";
	      // }
	    }
	    if (charge >= 3) { // show +2 for +3 or higher
	      if (a) {
		text += "&amp;ShowA2=1";
	      }
	      if (b) {
		text += "&amp;ShowB2=1";
	      }
	      if (c) {
		text += "&amp;ShowC2=1";
	      }
	      // if (d) {
	      // 	text += "&amp;ShowD2=1";
	      // }
	    }
	    if (charge >= 4) { // show +3 for +4 or higher
	      if (a) {
		text += "&amp;ShowA3=1";
	      }
	      if (b) {
		text += "&amp;ShowB3=1";
	      }
	      if (c) {
		text += "&amp;ShowC3=1";
	      }
	      // if (d) {
	      // 	text += "&amp;ShowD3=1";
	      // }
	    }
	  }
	  if (v || w || x || y || z) {
	    if (charge >= 1) {
	      // if (v) {
	      // 	text += "&amp;ShowV=1";
	      // }
	      // if (w) {
	      // 	text += "&amp;ShowW=1";
	      // }
	      if (x) {
		text += "&amp;ShowX=1";
	      }
	      if (y) {
		text += "&amp;ShowY=1";
	      }
	      if (z) {
		text += "&amp;ShowZ=1";
	      }
	    }
	    if (charge >= 2) {
	      // if (v) {
	      // 	text += "&amp;ShowV2=1";
	      // }
	      // if (w) {
	      // 	text += "&amp;ShowW2=1";
	      // }
	      if (x) {
		text += "&amp;ShowX2=1";
	      }
	      if (y) {
		text += "&amp;ShowY2=1";
	      }
	      if (z) {
		text += "&amp;ShowZ2=1";
	      }
	    }
	    if (charge >= 3) {
	      // if (v) {
	      // 	text += "&amp;ShowV3=1";
	      // }
	      // if (w) {
	      // 	text += "&amp;ShowW3=1";
	      // }
	      if (x) {
		text += "&amp;ShowX3=1";
	      }
	      if (y) {
		text += "&amp;ShowY3=1";
	      }
	      if (z) {
		text += "&amp;ShowZ3=1";
	      }
	    }
	  }
	} // end ion series


	// add N/C term mods if they exist
	{

	  string nTermMod = node
	    ->findChild("search_result")
	    ->findChild("search_hit", "hit_rank", "1")
	    ->findChild("modification_info")
	    ->getAttrValue("mod_nterm_mass");
	  if (nTermMod != "") {
	    text += "&amp;ModN=" + nTermMod;
	  }

	  string cTermMod = node
	    ->findChild("search_result")
	    ->findChild("search_hit", "hit_rank", "1")
	    ->findChild("modification_info")
	    ->getAttrValue("mod_cterm_mass");
	  if (cTermMod != "") {
	    text += "&amp;ModC=" + cTermMod;
	  }

	}



	XMLNodeVec *v = &
	  (node
	   ->findChild("search_result")
	   ->findChild("search_hit", "hit_rank", "1")
	   ->findChild("modification_info")
	   ->children_);
	for (unsigned int vc=0;vc<v->size();vc++) {
	  text += "&amp;Mod";
	  text += (*v)[vc]->getAttrValue("position");
	  text += "=";
	  text += (*v)[vc]->getAttrValue("mass");
	}

	text += "&amp;Pep=";
	text += node->
	  findChild("search_result")->
	  findChild("search_hit", "hit_rank", "1")->getAttrValue("peptide");

	text += "&amp;Dta=";
	text += infoMap["runSummaryBaseName"] + "/" + spectrum + ".dta";

	// output global aa mods for this MRS
	AAModInfoVec* modvec = infoMap.getModVec();
	for (unsigned int mn=0; mn<(*modvec).size(); mn++) {
	  text += "&amp;Global_Mod="; // ex, CV9.000 is a delta 9 variable mod on cys
	  text += ((*modvec)[mn])->aa_;

	  if ( ((*modvec)[mn])->modType_ == AAModInfo::static_mod) {
	    text += "S";
	  }
	  else {
	    // assume variable
	    text += "V";
	  }

	  string modValue;
	  toString( ((*modvec)[mn])->modValue_ , modValue);
	  text += modValue;

	} // end mod loop

	text += "\">";

	if (!(node->findChild("search_result")->findChild("search_hit", "hit_rank", "1")->getAttrValue("tot_num_ions")).empty()) {
	  text += node->
	    findChild("search_result")->
	    findChild("search_hit", "hit_rank", "1")->getAttrValue("num_matched_ions");
	  text += "/";
	  text += node->
	    findChild("search_result")->
	    findChild("search_hit", "hit_rank", "1")->getAttrValue("tot_num_ions");
	}
	else {
	  text += "[spectrum]";
	}

	text += "</a>";
      }
    }

 // ions

   else if (fieldName_ == "ions2") {

      if (mode == Field::value ||
            mode == Field::plainText) {
         text = node->findChild("search_result")->findChild("search_hit", "hit_rank", "1")->getAttrValue("num_matched_ions");
         text += "/";
         text += node->findChild("search_result")->findChild("search_hit", "hit_rank", "1")->getAttrValue("tot_num_ions");
      }
      else if ( mode == Field::html) {
         string spectrum=node->getAttrValue("spectrum");
         text = "<a target=\"Win1\" href=\"";
         text += infoMap["cgiBase"] + "plot-msms-js.cgi?PrecursorMassType=";
         text += infoMap["precursorMassType"];
         text += "&amp;FragmentMassType=";
         text += infoMap["fragmentMassType"];
         string precursorNeutralMass=node->getAttrValue("precursor_neutral_mass");
         text += "&amp;PepMass=" + precursorNeutralMass;

	 if (!node->findChild("search_result")->findChild("search_hit", "hit_rank", "1")->findChild("analysis_result", "analysis", "libra")->isEmpty_)
	   text += "&amp;Isobaric=1";

         int charge;
         if (!toInt(node->getAttrValue("assumed_charge"), charge)) {
            charge=-1; // TODO: something better?
         }

      // add N/C term mods if they exist
      string nTermMod = node->findChild("search_result")->findChild("search_hit", "hit_rank", "1")->findChild("modification_info")->getAttrValue("mod_nterm_mass");
      if (nTermMod != "") {
         text += "&amp;ModN=" + nTermMod;
      }

      string cTermMod = node->findChild("search_result")->findChild("search_hit", "hit_rank", "1")->findChild("modification_info")->getAttrValue("mod_cterm_mass");
      if (cTermMod != "") {
         text += "&amp;ModC=" + cTermMod;
      }

      XMLNodeVec *v = &(node->findChild("search_result")->findChild("search_hit", "hit_rank", "1")->findChild("modification_info")->children_);
      for (unsigned int vc=0;vc<v->size();vc++) {
         text += "&amp;Mod";
         text += (*v)[vc]->getAttrValue("position");
         text += "=";
         text += (*v)[vc]->getAttrValue("mass");
      }

      text += "&amp;Pep=";
      text += node->findChild("search_result")->findChild("search_hit", "hit_rank", "1")->getAttrValue("peptide");

      text += "&amp;Dta=";
      text += infoMap["runSummaryBaseName"] + "/" + spectrum + ".dta";

      text += "\">";

      if (!(node->findChild("search_result")->findChild("search_hit", "hit_rank", "1")->getAttrValue("tot_num_ions")).empty()) {
	text += node->findChild("search_result")->findChild("search_hit", "hit_rank", "1")->getAttrValue("num_matched_ions");
	text += "/";
	text += node->findChild("search_result")->findChild("search_hit", "hit_rank", "1")->getAttrValue("tot_num_ions");
      }
      else {
	text += "[spectrum]";
      }
      text += "</a>";
      }
    }


    // peptideModText
    else if (fieldName_ == "peptideModText"){
      string textPepString;


      XMLNodePtr searchHitNode = (node->
				  findChild("search_result")->
				  findChild("search_hit", "hit_rank", "1"));

      Field::searchPeptideText(searchHitNode,
			       string(""), // no search text
			       true, // yes, include modification text
			       false, // no, not just searching-- returning text
			       textPepString,
			       false, // no, don't return html
			       false // !no brackets on subscripts!
			       );
      text = textPepString;
    }



    // peptide
    else if (fieldName_ == "peptide") {
      if (mode == Field::value) {
         text = node->findChild("search_result")->findChild("search_hit", "hit_rank", "1")->getAttrValue("peptide");
      }
      else if (mode == Field::plainText) {
	{
	  text = node->findChild("search_result")->findChild("search_hit", "hit_rank", "1")->getAttrValue("peptide_prev_aa");
	  text += ".";


	  string textPepString;


	  XMLNodePtr searchHitNode = (node->findChild("search_result")->findChild("search_hit", "hit_rank", "1"));

	  Field::searchPeptideText(searchHitNode,
				   string(""), // no search text
				   true, // yes, include modification text
				   false, // no, not just searching
				   textPepString,
				   false // no, don't return html
				   );

	  // htmlPepString now contains the appropriately html-ified text


	  text += textPepString;

	  text += ".";
	  text += node->findChild("search_result")->findChild("search_hit", "hit_rank", "1")->getAttrValue("peptide_next_aa");
	}
      } // end plain-text peptide

      else if (mode == Field::html) {

	string htmlPepString;


	XMLNodePtr searchHitNode = node->findChild("search_result")->findChild("search_hit", "hit_rank", "1");


	string htext = highlightedPeptideText;
	Field::searchPeptideText(searchHitNode,
				 htext,
				 includeHighlightedPepTextMods,
				 false, // no, not just searching
				 htmlPepString,
				 true); // yes, return html!

	// htmlPepString now contains the appropriately html-ified text


	// assemble the returned value, "text"


	// prev AA
	text = node->findChild("search_result")->findChild("search_hit", "hit_rank", "1")->getAttrValue("peptide_prev_aa");
	text += ".";

	// peptide link
	text += "<a target=\"Win1\" href=\"";
	text += "http://www.ncbi.nlm.nih.gov/blast/Blast.cgi?CMD=Web&amp;LAYOUT=TwoWindows&amp;AUTO_FORMAT=Semiauto&amp;ALIGNMENTS=50&amp;ALIGNMENT_VIEW=Pairwise&amp;CDD_SEARCH=on&amp;CLIENT=web&amp;COMPOSITION_BASED_STATISTICS=on&amp;DATABASE=nr&amp;DESCRIPTIONS=100&amp;ENTREZ_QUERY=(none)&amp;EXPECT=1000&amp;FILTER=L&amp;FORMAT_OBJECT=Alignment&amp;FORMAT_TYPE=HTML&amp;I_THRESH=0.005&amp;MATRIX_NAME=BLOSUM62&amp;NCBI_GI=on&amp;PAGE=Proteins&amp;PROGRAM=blastp&amp;SERVICE=plain&amp;SET_DEFAULTS.x=41&amp;SET_DEFAULTS.y=5&amp;SHOW_OVERVIEW=on&amp;END_OF_HTTPGET=Yes&amp;SHOW_LINKOUT=yes&amp;QUERY=";

	text += node->
	  findChild("search_result")->
	  findChild("search_hit", "hit_rank", "1")->getAttrValue("peptide");

	text += "\">";

	// peptide html string
	text += htmlPepString;

	text += "</a>";


	// next AA
	text += ".";
	text += node->
	  findChild("search_result")->
	  findChild("search_hit", "hit_rank", "1")->getAttrValue("peptide_next_aa");


	// peptide atlas callout
	text += "<a target=\"Win1\" href=\"https://db.systemsbiology.net/sbeams/cgi/PeptideAtlas/Search?organism_name=Any;search_key=%25";
	text += node->
	  findChild("search_result")->
	  findChild("search_hit", "hit_rank", "1")->getAttrValue("peptide");
	text += "%25;action=GO\"><img border=\"0\" src=\"";
	text += infoMap["pepXMLResources"]; // includes trailing slash
	text += "images/pa_tiny.png\" alt=\"PeptideAtlas\" /></a>";

      }
    }



    // protein
    else if (fieldName_ == "protein") {
      if (mode == Field::value) {
	text= node->
	  findChild("search_result")->
	  findChild("search_hit", "hit_rank", "1")->getAttrValue("protein");
      }
      else if (mode == Field::plainText) {
	// primary protein
	text= node->
	  findChild("search_result")->
	  findChild("search_hit", "hit_rank", "1")->getAttrValue("protein");

	// alternates, if any
	XMLNodeVecPtr nv = node->
	  findChild("search_result")->
	  findChild("search_hit", "hit_rank", "1")->
	  findChildren("alternative_protein");
	int numAlt = (int)nv->size();
	if (numAlt > 0) {
	  for (int ii = 0; ii<numAlt; ii++) {
	    text += ",";
	    text += ((*nv)[ii])->getAttrValue("protein");
	  }
	}
      }

      else if (mode == Field::html) {
	text = "<a target=\"Win1\" href=\"";
	text += infoMap["cgiBase"] + "comet-fastadb.cgi?";
	text += "Ref=" +  node->
	  findChild("search_result")->
	  findChild("search_hit", "hit_rank", "1")->getAttrValue("protein");
	text += "&amp;Db=" + infoMap["searchDatabase"];
	text += "&amp;Pep=" +  node->
	  findChild("search_result")->
	  findChild("search_hit", "hit_rank", "1")->getAttrValue("peptide");
	text += "&amp;MassType=" + infoMap["precursorMassType"];
	text += "&amp;sample_enzyme=" + infoMap["sampleEnzyme"];
	text += "&amp;min_ntt=" + infoMap["searchConstraintMinNtt"];
	text += "\"";

	name = node->
	  findChild("search_result")->
	  findChild("search_hit", "hit_rank", "1")->getAttrValue("protein_descr");

	if (name != "") {
	  text += " title=\"";
	  text += name;
	  text += "\" ";
	}

	text += " >";


	/* regex highlighting */
	bool highlighting = false;
	string proteinNameText = node->
	  findChild("search_result")->
	  findChild("search_hit", "hit_rank", "1")->getAttrValue("protein");
	if (highlightedProteinText.size() > 0 ) {
	  string::const_iterator protstart, protend;
	  protstart = proteinNameText.begin();
	  protend = proteinNameText.end();

	  match_results<string::const_iterator> what;
	  match_flag_type flags = match_default;
	  //int startPos = 0;

	  regex re2(highlightedProteinText);
	  if (regex_search(protstart, protend, what, re2, flags)) {
	    highlighting = true;
	  }
	}



	if (highlighting) {
	  text += "<span class=\"highlight\">";
	}

	text += //string(y) + string(": ") +
	  node->
	  findChild("search_result")->
	  findChild("search_hit", "hit_rank", "1")->getAttrValue("protein");

	if (highlighting) {
	  text += "</span>";
	}

	text += "</a>";

	XMLNodeVecPtr nv = node->
	  findChild("search_result")->
	  findChild("search_hit", "hit_rank", "1")->
	  findChildren("alternative_protein");
	int numAlt = (int)nv->size();
	if (numAlt > 0) {
	  if (infoMap["expandProteinList"] != "") {
	    // print alternative proteins, if any
	    for (int ii = 0; ii<numAlt; ii++) {
	      if (ii == 0) {
		text += "<br />";
	      }
	      //cout << ii << "<br />" << endl;
	      string altP=((*nv)[ii])->getAttrValue("protein");
	      text += "<a target=\"Win1\" href=\"";
	      text += infoMap["cgiBase"] + "comet-fastadb.cgi?";
	      text += "Ref=" + altP;
	      text += "&amp;Db=" + infoMap["searchDatabase"];
	      text += "&amp;Pep=" +  node->
		findChild("search_result")->
		findChild("search_hit", "hit_rank", "1")->getAttrValue("peptide");
	      text += "&amp;MassType=" + infoMap["precursorMassType"];
	      text += "\" ";

	      name = ((*nv)[ii])->getAttrValue("protein_descr");
	      if (name != "") {
		text += " title=\"";
		text += name;
		text += "\" ";
	      }


	      text += " >";
	      text += altP;
	      text += "</a>";
	      text += "<br />";
	    }
	  }
	  else {
	    //cout << "numalt: " << numAlt << "<br />" << endl;
	    text += "  <a target=\"Win1\" href=\"";
	    text += infoMap["cgiBase"] + "comet-fastadb.cgi?";
	    text += "Db=" + infoMap["searchDatabase"];
	    text += "&amp;Pep=" +  node->
	      findChild("search_result")->
	      findChild("search_hit", "hit_rank", "1")->getAttrValue("peptide");
	    text += "&amp;MassType=" + infoMap["precursorMassType"];
	    text += "&amp;sample_enzyme=" + infoMap["sampleEnzyme"];
	    text += "&amp;min_ntt=" + infoMap["searchConstraintMinNtt"];
	    text += "\"";

	    text += " title=\"";
	    for (int ii = 0; ii<numAlt; ii++) {
	      if (ii != 0) {
		text += "&lt;br /&gt;";
	      }
	      name = node->
		findChild("search_result")->
		findChild("search_hit", "hit_rank", "1")->getAttrValue("protein_descr");
	      string altPDesc=((*nv)[ii])->getAttrValue("protein");
	      text += altPDesc;
	    }
	    text += "\" ";


	    text += " >";


	    text += " &#43;";
	    sprintf(t,"%d",numAlt);
	    text += t;

	    text += "</a>";


	  }
	}
      }
    }



    // if here without finding a name match, unmatched general field;
    // assume anything else is a search_hit attr
    else {
      if (mode == Field::value ||
	  mode == Field::plainText ||
	  mode == Field::html) {
	text = node->
	  findChild("search_result")->
	  findChild("search_hit", "hit_rank", "1")->
	  getAttrValue(fieldName_);
      }
    }

	break;

  case Field::searchScore: // search scores
    if (mode == Field::value ||
	mode == Field::plainText ||
	mode == Field::html) {
      np=node->findChild("search_result")->findChild("search_hit", "hit_rank", "1");
      np=np->findDescendent(nodeName_);
      //np->print();
      //cout << "###" << endl;

      if (!np->isEmpty_) {
	text = np->getAttrValue("value");
	//cout << nodeName_ << ": " << text << endl;

	if (mode != value ) {
	  // mascot star
	  if (infoMap["searchEngine"] == "MASCOT"
	      &&
	      fieldName_ == "ionscore") {
	    name = "search_score";
	    name += "[@star]";
	    np=np->parent_.lock();
	    np=np->findDescendent(name);
	    if (np->getAttrValue("value") == "1") {
	      text += "*";
	    }
	  }
	}

      }
      else {
	// try alternates first
	bool foundAlternate = false;
	if (infoMap["searchEngine"] == "SPECTRAST"
	    &&
	    fieldName_ == "mz_diff") {
	  np=node->findChild("search_result")->findChild("search_hit", "hit_rank", "1");
	  name = "search_score";
	  name += "[@precursor_mz_diff]";
	  np=np->findDescendent(name);
	  if (!np->isEmpty_) {
	    foundAlternate = true;
	    text = np->getAttrValue("value");
	  }
	}
	else if (infoMap["searchEngine"] == "SPECTRAST"
		 &&
		 fieldName_ == "delta_dot") {
	  np=node->findChild("search_result")->findChild("search_hit", "hit_rank", "1");
	  name = "search_score";
	  name += "[@delta]";
	  np=np->findDescendent(name);
	  if (!np->isEmpty_) {
	    foundAlternate = true;
	    text = np->getAttrValue("value");
	  }
	}
      }
    }

    break;

  case Field::ptmProphet:

    //PTM_PEPTIDE
    if(fieldName_ == "ptm_peptide")
      {
	if (mode == Field::plainText) {
	  text = node->
	    findChild("search_result")->
	    findChild("search_hit", "hit_rank", "1")->
	    findChild("analysis_result", "analysis", "ptmprophet")->
	    findChild("ptmprophet_result")->
	    getAttrValue("ptm_peptide");
	}

	else if (mode == Field::html) {
	  XMLNodePtr ptm_result =
	    node->
	    findChild("search_result")->
	    findChild("search_hit", "hit_rank", "1")->
	    findChild("analysis_result", "analysis", "ptmprophet")->
	    findChild("ptmprophet_result");

	  if ( ptm_result->getAttrValue("ptm_peptide") == "" ) {
	    text = "n/a";
	  }
	  else {
	    // generate probs plot
	    string pep = node->
	      findChild("search_result")->
	      findChild("search_hit", "hit_rank", "1")->getAttrValue("peptide");
	    int peplen = (int)pep.length();

	    text = getPTMProbsPlot(ptm_result, peplen);

	    // get assigned mods, highlight them in peptide string
	    XMLNodePtr mod_result =
	      node->
	      findChild("search_result")->
	      findChild("search_hit", "hit_rank", "1")->
	      findChild("modification_info");

	    text += getModPeptide(mod_result, pep);
	  }
	}
      }

    break; //shouldn't get here
    //end ptmProphet

	case Field::peptideProphet:

		//PROBABILITY
		if(fieldName_ == "probability" )
		{
			//disable link if interProphet, for now
			if(mode == Field::value || mode == Field::plainText ||
			((mode == Field::html) && (infoMap["interprophet"] == "true")))
			{
				text = node->
				findChild("search_result")->
				findChild("search_hit", "hit_rank", "1")->
				findChild("analysis_result", "analysis", "peptideprophet")->
				findChild("peptideprophet_result")->
				getAttrValue("probability");
			}
			else if(mode == Field::html)
			{
				XMLNodePtr pnode = node->
				findChild("search_result")->
				findChild("search_hit", "hit_rank", "1")->
				findChild("analysis_result", "analysis", "peptideprophet")->
				findChild("peptideprophet_result");

				string p = pnode->getAttrValue("probability");

				text = "<a target = \"pepmodels\" href = \"";
				text += infoMap["modelsFileNameWeb"] + "?";
				text += "Spectrum=";
				text += node->getAttrValue("spectrum");
				text += "&amp;Scores=";

				XMLNodeVec *v = &(node->
				findChild("search_result")->
				findChild("search_hit", "hit_rank", "1")->
				findChild("analysis_result","analysis","peptideprophet")->
				findChild("peptideprophet_result")->
				findChild("search_score_summary")->
				children_);

				for (unsigned int vc = 0; vc != v->size(); vc++)
				{
					if (vc != 0)
						text += "%20";
					text += (*v)[vc]->getAttrValue("name") + ":";
					text += (*v)[vc]->getAttrValue("value");
				}

				text += "&amp;Prob=" + p + "\">";

				//highlight any probabilities in which +2 and +3 both
				//received high prob. and were therefore adjusted down
				if(pnode->getAttrValue("analysis") == "adjusted")
					p = "<span class=\"highlight\">" + p + "</span>";

				text += p + "</a>";
			}
		}
		//assume it's a parameter node: fval, etc
		else if(mode == Field::value || mode == Field::plainText || mode == Field::html)
		{
			text = node->
			findChild("search_result")->
			findChild("search_hit", "hit_rank", "1")->
			findChild("analysis_result","analysis","peptideprophet")->
			findChild("peptideprophet_result")->
			findChild("search_score_summary")->
			findChild(nodeName_)->
			getAttrValue("value");
		}
	break; //shouldn't get here
	//end peptideProphet

	case Field::interProphet:

		//PROBABILITY
		if(fieldName_ == "iprobability")
		{
			if(mode == Field::value || mode == Field::plainText) //|| mode == Field::html)
			{
				text = node->
				findChild("search_result")->
				findChild("search_hit", "hit_rank", "1")->
				findChild("analysis_result","analysis","interprophet")->
				findChild("interprophet_result")->
				getAttrValue("probability");
			}
			//RBS modifications !!!
			else if (mode == Field::html)
			{
				XMLNodePtr pnode = node->
				findChild("search_result")->
				findChild("search_hit", "hit_rank", "1")->
				findChild("analysis_result", "analysis", "interprophet")->
				findChild("interprophet_result");

				string p = pnode->getAttrValue("probability");

				text = "<a target = \"Win1\" href = \"";
				text += infoMap["cgiBase"] + "iModelParser.cgi?Xmlfile=";
				text += infoMap["msms_pipeline_analysis[@summary_xml]"];
				text += "&amp;Timestamp=";
				text += infoMap["interprophet[@time]"];
				text += "&amp;Spectrum=";
				text += node->getAttrValue("spectrum");
				text += "&amp;Scores=";

				XMLNodeVec *v = &(node->
				findChild("search_result")->
				findChild("search_hit", "hit_rank", "1")->
				findChild("analysis_result", "analysis", "interprophet")->
				findChild("interprophet_result")->
				findChild("search_score_summary")->
				children_);

				for (unsigned int vc = 0; vc != v->size(); vc++)
				{
					if (vc != 0)
						text += "%20"; //space
					text += (*v)[vc]->getAttrValue("name") + ":";
					text += (*v)[vc]->getAttrValue("value");
				}

				text += "&amp;iProb=" + p + "\">";

				//highlight any probabilities in which +2 and +3 both
				//received high prob. and were therefore adjusted down
				if(pnode->getAttrValue("analysis") == "adjusted")
					p = "<span class=\"highlight\">" + p + "</span>";

				text += p + "</a>";
			}
		}
		//assume it's a parameter node: fval, etc
		else if(mode == Field::value || mode == Field::plainText || mode == Field::html)
		{
			text = node->
			findChild("search_result")->
			findChild("search_hit", "hit_rank", "1")->
			findChild("analysis_result","analysis","interprophet")->
			findChild("interprophet_result")->
			findChild("search_score_summary")->
			findChild(nodeName_)->
			getAttrValue("value");
		}

	break; //shouldn't get here
	//end interProphet

	default:
	break;
  }

  if (text == "") {
    if (mode == Field::value) {
      // no change
      // text = "";
    }
    else {
      text = "[unavailable]";
    }
  }
  return text;
}

bool
Field::searchPeptideText(XMLNodePtr node,
			 string searchText,
			 bool includeModText,
			 bool searchOnly, // then no highlight text generated
			 string& returnText,
			 bool returnHtml,
			 bool returnBrackets) {



  string peptideSequence = node->attrs_["peptide"];

  // each vector element is the text for one AA
  vector<string>* peptideAAVec = new vector<string>();

  // each vector element is the text for the subscripted numeric value
  vector<string>* peptideAAVecMods = new vector<string>();


  // add n term. modification, if any
  int nTermOffset = 0;
  string nTermMod = node
    ->findChild("modification_info")
    ->getAttrValue("mod_nterm_mass");
  if (nTermMod != "") {
    nTermOffset = 1;
    peptideSequence = string("n") + peptideSequence;
  }

  // add modified c term indicator if required
  string cTermMod = node
    ->findChild("modification_info")
    ->getAttrValue("mod_cterm_mass");
  if (cTermMod != "") {
    peptideSequence = peptideSequence + string("c");
  }


  char t[255];

  /**
     make a string vector, in which each element is one amino acid;
  */

  /*
  // put nTerm text into vector
  if (nTermMod != "") {
    peptideAAVec.push_back("n");
  }
  */

  // put main peptide text into vector
  for (unsigned int i=0; i<peptideSequence.size(); i++) {
    peptideAAVec->push_back(peptideSequence.substr(i,1));
  }


  /**
      make a string vector, in which each element is one
      AA's (or [c|n]Term's)  numeric modification
  **/

  // first, allocate blank strings in mod vector
  for (unsigned int c=0; c<peptideAAVec->size();c++) {
    peptideAAVecMods->push_back("");
  }


  // put in nTerm mod value, if any
  if (nTermMod != "") {
    double d;
    toDouble(nTermMod,d);
    sprintf(t, "%.2f", d);
    (*peptideAAVecMods)[0] = (string(t));
  }

  // put in all main peptide text mods
  if (!
      node->findChild("modification_info")->isEmpty_) {

    // yes, there are modifications

    int numMods = (int)node->findChild("modification_info")->children_.size();

    for (int c=0; c<numMods; c++) {
      int modPos;
      toInt(node->findChild("modification_info")
	    ->children_[c]->getAttrValue("position"), modPos);
      string modVal =
	(node->findChild("modification_info")
	 ->children_[c]->getAttrValue("mass"));
      double d;
      toDouble(modVal, d);
      sprintf(t, "%.2f", d);

      // string offset starts from zero
      modPos--;

      if (modPos + nTermOffset < 0 || modPos + nTermOffset >= (int)peptideAAVecMods->size() ) {
	printf( "DDS: internal error\n") ;
      }
      (*peptideAAVecMods)[modPos + nTermOffset] = t;
    }
  }


  // put in cTerm mod value, if any
  if (cTermMod != "") {
    double d;
    toDouble(cTermMod, d);
    sprintf(t, "%.2f", d);
    (*peptideAAVecMods)[peptideAAVecMods->size() - 1] = string(t);
  }



  /**
     assemble strings back together
  */
  string peptideTextNoMods = "";
  string peptideTextMods = "";
  string peptideTextBracketMods = "";

  for (unsigned int j=0; j<peptideAAVec->size(); j++) {
    peptideTextNoMods += (*peptideAAVec)[j];

    peptideTextMods += (*peptideAAVec)[j]
      + (*peptideAAVecMods)[j];

    peptideTextBracketMods += (*peptideAAVec)[j];
    if ((*peptideAAVecMods)[j] != "") {
      peptideTextBracketMods +=
	string("[")
	+ (*peptideAAVecMods)[j]
	+ string("]");
    }

  }


  if (searchOnly) {
    // either search against the plain peptide text (include 'n' and 'c'),
    // for ex: nTRLKc,
    // or the peptide text with numeric numbers, but no brackets:
    // for ex: n44.01TRL234.12Kc34.56,

    if (searchText == "") {
      return true; // free pass
    }

    string textToSearch;
    if (includeModText) {
      textToSearch = peptideTextMods;
    }
    else {
      textToSearch = peptideTextNoMods;
    }

    //cout << "requred peptide text: " << searchText << endl;

    // init the regex
    regex regPeptide;
    regPeptide = searchText;

    if (!regex_search(textToSearch, regPeptide)) {
      return false;
    }
    else {
      return true;
    }
  } // end searchOnly



  // so, if we're here, we're generating text.  What kind?
  if (returnHtml == false) {
    if (includeModText) {
      if (returnBrackets) {
	returnText = peptideTextBracketMods;
      }
      else {
	returnText = peptideTextMods;
      }
      return false; // no meaning
    }
    else {
      returnText = peptideTextNoMods;
      return false; // no meaning
    }
  }



  // if HERE, returning html text

  // this always includes subscripted numerics,
  // BUT, the text we search against changes:
  //
  // either search against the plain peptide text (include 'n' and 'c'),
  // for ex: nTRLKc,
  // or the peptide text with numeric numbers, but no brackets:
  // for ex: n44.01TRL234.12Kc34.56,

  string textToSearch;
  if (includeModText) {
    textToSearch = peptideTextMods;
  }
  else {
    textToSearch = peptideTextNoMods;
  }


  /**
     make a bool vector marking each AA (and subscript if any) as highlighted or not
  */
  vector<bool>* markHighlightVec = new vector<bool>();
  // first, allocate blank strings in mod vector
  for (unsigned int c=0; c<peptideAAVec->size();c++) {
    markHighlightVec->push_back(false);
  }



  /**
     do the regex highlighting with regPeptide

     we'll search against either the plain text or text-with-numbers (no brackets)

     we'll get the position of the match.

     * if not including mods, easy case: mark matched AAs as highlighted

     * if including mods:
         figure out which AA is the lower bound,
	 figure out which AA is the upper bound
	 mark highlighted

  */

  /** debug **/
  //cout << "in html text highlight production: " << endl;
  //cout << "text to search against: " << textToSearch << endl;
  //cout << "search string: " << searchText << endl;
  /** debug **/

  if (searchText.size() > 0 ) {

    string::const_iterator pepstart, pepend;
    pepstart = textToSearch.begin();
    pepend = textToSearch.end();

    match_results<string::const_iterator> what;
    match_flag_type flags = match_default;
    string::size_type startPos = 0;
    string::size_type startPosN = 0; // for numeric search

    // init the regex
    regex regPeptide;
    regPeptide = searchText;

    int round=0;
    while(regex_search(pepstart, pepend, what, regPeptide, flags)) {
      round++;
      //cout << "in regex search loop " << round << endl;

      // for each match...
      //cout << "found match:";
      //cout << startPos + what.position(0) << "): " << what[0] << ":" << endl;


      if (!includeModText) {
	// highlight the range of the matched text

	int match_index = 0;
	// defining this as an int type;
	// passing 0 (even cast as int) to what.position below results in ambigous overload

	string::size_type lowerBound = (startPos + what.position(match_index));
	string::size_type upperBound = (startPos+what.position(match_index) + what.length(match_index));
	for (string::size_type c=lowerBound; c<upperBound; c++) {
	  // easy case: no numerics
	  (*markHighlightVec)[c] = true;
	}

	startPos += (what.position(match_index) + what.length(match_index));
      }

      else {
	// we're including numerics


	// realStart and realEnd will mark the boundaries in the
	// "textToSearch" string of what characters should be
	// highlighted; if part of the numerics are highlighted we
	// shift the boundaries to mark them all

	// we have to figure out which AA is the lower bound
	int match_index = 0; // see comments above (re: ambigious overload)
	string::size_type realStart = (startPosN + what.position(match_index)); // pos in searched-against text
	// walk realStart back to first on non-numeric text or beginning of string
	while ((realStart !=0) &&
	       (
		(textToSearch[realStart] >= '0') &&
		(textToSearch[realStart] <= '9')
		)
	       ||
	       (
		(textToSearch[realStart] == '.')
		)
	       ) {
	  realStart--;
	}


	// we have to figure out which AA is the upper bound
	string::size_type realEnd = (startPosN+what.position(match_index) + what.length(match_index));
	// advance realEnd to first non-numeric or end of string
	while ((realEnd < textToSearch.length()) &&
	       (
		(textToSearch[realEnd] >= '0') &&
		(textToSearch[realEnd] <= '9')
		)
	       ||
	       (
		(textToSearch[realEnd] == '.')
		)
	       ) {
	  realEnd++;
	}


	// now, we want to find the range of values in the AA vec to highlight

	// realStart points to an AA: map it
	int aaIndex=0;
	string::size_type stringPos=0;
	while (stringPos != realStart) {
	  char cursor = textToSearch[stringPos];
	  if (!(cursor >= '0' &&
		cursor <= '9')
	      &&
	      cursor != '.') {
	    // cursor was an AA, not mod, so
	    // cursor's AA is in (*peptideAAVec)[aaIndex] *before* this increment
	    aaIndex++;
	  }
	  stringPos++;
	}
	int aaIndexStart = aaIndex;

	// realEnd points to an AA: map it
	aaIndex = 0;
	stringPos = 0;
	while (stringPos != realEnd) {
	  char cursor = textToSearch[stringPos];
	  if (!(cursor >= '0' &&
		cursor <= '9')
	      &&
	      cursor != '.') {
	    // cursor was an AA, not mod, so
	    // cursor's AA is in (*peptideAAVec)[aaIndex] *before* this increment
	    aaIndex++;
	  }
	  stringPos++;
	}
	int aaIndexEnd = aaIndex;

	for (int m=aaIndexStart; m<aaIndexEnd; ++m) {
	  (*markHighlightVec)[m] = true;
	}

	startPosN += (int)(what.position(match_index) + what.length(match_index));

      } // end numerics highlighting



      // increment the while-loop search
      pepstart = what[0].second;
    } // end regex while-loop

  } // end highlighting text


  /**
     NOW, we get to build up the final html string!

  */
  returnText = "";
  for (unsigned int jj=0; jj<peptideAAVec->size(); jj++) {

    if ((*markHighlightVec)[jj]) {
      returnText += "<span class=\"highlight\">";
    }

    returnText += (*peptideAAVec)[jj];

    if ((*peptideAAVecMods)[jj] != "") {
      returnText += "<span class=\"sub\">";
      returnText += (*peptideAAVecMods)[jj];
      returnText += "</span>";
    }

    if ((*markHighlightVec)[jj]) {
      returnText += "</span>";
    }
  }

  delete markHighlightVec;
  delete peptideAAVec;
  delete peptideAAVecMods;

  return false; // no meaning

}


string
Field::getPTMProbsPlot(XMLNodePtr ptm_node, int pepsize) {
  char t[255]; // temp buffer for formatting
  string ptmjstext = "<span style='border-bottom:1px solid #777;'>";

  string ptm_arr [10000];
  for (int c=0; c<pepsize; c++) {
    ptm_arr[c] = "&nbsp;";
  }

  int numPTM = (int)ptm_node->children_.size();

  for (int c=0; c<numPTM; c++) {
    int ptmPos;
    toInt(ptm_node->children_[c]->getAttrValue("position"), ptmPos);
    ptmPos--; // make it zero-based
    double ptmProb;
    toDouble(ptm_node->children_[c]->getAttrValue("probability"), ptmProb);

    int pct = (int)(100*ptmProb);

    ptm_arr[ptmPos]  = "<span title='";
    sprintf(t, "%.4f", ptmProb);
    ptm_arr[ptmPos] += t;
    ptm_arr[ptmPos] += "' style='background:linear-gradient(to top, #5a5, #5a5 ";
    sprintf(t,"%d",pct);
    ptm_arr[ptmPos] += t;
    ptm_arr[ptmPos] += "%, #cbb ";
    ptm_arr[ptmPos] += t;
    ptm_arr[ptmPos] += "%);'>&nbsp;</span>";
  }

  for (int c=0; c<pepsize; c++) {
    ptmjstext += ptm_arr[c];
  }
  ptmjstext += "</span><br/>\n";
  return ptmjstext;
}


string
Field::getModPeptide(XMLNodePtr mods_node, string pep) {
  char t[255]; // temp buffer for formatting
  string modpeptext = "";

  string mod_arr [10000];
  for (int c=0; c<pep.size(); c++) {
    mod_arr[c] = pep[c];
  }

  int numMods = (int)mods_node->children_.size();

  for (int c=0; c<numMods; c++) {
    int modPos;
    toInt(mods_node->children_[c]->getAttrValue("position"), modPos);
    modPos--; // make it zero-based
    double modMass;
    toDouble(mods_node->children_[c]->getAttrValue("mass"), modMass);

    mod_arr[modPos]  = "<span title='mod mass=";
    sprintf(t, "%.4f", modMass);
    mod_arr[modPos] += t;
    mod_arr[modPos] += "' style='color:red;font-weight:bold'>";
    mod_arr[modPos] += pep[modPos];
    mod_arr[modPos] += "</span>";
  }

  for (int c=0; c<pep.size(); c++) {
    modpeptext += mod_arr[c];
  }

  return modpeptext;
}
