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




#include <exception>

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

#include "PepXDebug.h"
#include "PepXUtility.h"
#include "XMLNode.h"
#include "XMLTree.h"

#include "HeaderMRSSQ.h"

using std::runtime_error;
using boost::regex;
using boost::regex_search;



HeaderMRSSQ::HeaderMRSSQ() : 
  inMRS(false),
  inSQ(false),
  inHeader(false),
  MRSCount(0),
  SQCount(0),
  acceptedSQCount(0),
  skipping(false),
  mode(HeaderMRSSQ::lookingForHeader)
{
}








HeaderMRSSQ::~HeaderMRSSQ() {
}





void 
HeaderMRSSQ::init(const string& xmlFileName,
		  Options& options,
		  PipelineAnalysis& pipeline,
		  SQHandler& sqAcceptor) {
  setFileName(xmlFileName.c_str());
  pipeline_ = &pipeline;
  sqAcceptor_ = &sqAcceptor;
  options_ = &options;
}



// rename to 'sq or sq subnode'
void 
HeaderMRSSQ::startSQ(const XML_Char *elementName, 
		     const XML_Char **attr) {


  /*
    #define OLD_NDEBUG NDEBUG
    #undef _INCLUDED_DEBUG_H_
    #undef NDEBUG
    #include "PepXDebug.h"
  */
  debug(debugout << "in startSQ: " << elementName << "(xml line " << getCurrentLineNumber() << ", column " << getCurrentColumnNumber() << ")");

  XMLNodePtr np;

  if (skipping) {
    return;
  }
  else {
    // -- not skipping --

    string name = elementName;

    // if top of SQ, turn on inSQ
    if (name == "spectrum_query") {
      inSQ = true;
    }


    // make a node; 
    np = XMLNode::create(elementName, attr);


    np->startOffset_ = getByteOffset();

    // add the node to the tree we're building
    tree.addNode(np);

    if (name == "spectrum_query") {
      curSQ = np; // used in checkSQ
    }



    if (inSQ) {

      // special value nodes; ***rename the actual node-name*** to
      // include attr name
      if ( (name == "search_score") ||
	   (name == "parameter") ) {
	np->name_ += "[@" + np->attrs_[string("name")] + "]";
	name = np->name_;
	//cout << "renamed ss: " << name << endl;
      }



      //       /** 

      // 	  test spectrum_query attrs and subnodes; if we find a reason to
      // 	  discard this spectrum_query, set skip to true


      // 	  we could just test for nodes that are know to take attrs, but
      // 	  this is less extensible, i.e. if in the future attrs are
      // 	  added to search_result, the only function that needs to be
      // 	  changed is the lookup function

      // 	  make this smarter?  only test for nodes with filters?

      // 	  add: if filters exist for this nodename, apply the tests,
      // 	  otherwise just add.

      //       */


      
      
      //       //cout << "sq: about to test " << name << endl;
      //       if ( //a
      // 	  ( //b
      // 	   // one of the following must be true before we call
      // 	   // actually do the checking 
      // 	   !(options_->filterMap_[name].isEmpty_) 
      // 	   ||
      // 	   (options_->excludeCharges && (name == "spectrum_query"))
      // 	   ||

      // 	   ( // c
      // 	    (name == "search_hit") 
      // 	    ||
      // 	    options_->requireGlyc
      // 	    ||
      // 	    options_->requireAA
      // 	    //||
      // 	    //options_->requirePeptideText // moved to endInSQ, requires mod info
      // 	    ||
      // 	    options_->requireProteinText
      // 	    ||
      // 	    options_->requireSpectrumText
      // 	     ) //c

      // 	    ) //b
      // 	  &&
      // 	  !(checkSQNode(np))
      // 	   )// a
      // 	{
      // 	  // discard
      // 	  debug(debugout << "this node failed! " << np->name_);
      // 	  skipping = true; // can only be reset at end of spectrum_query
      // 	  // keep the last added node involved in the sq subtree
      // 	  np.reset();
      // 	  return;
      // 	}
    
      
    }      

  } // !skip

  /*
    #undef _INCLUDED_DEBUG_H_
    #define NDEBUG OLD_NDEBUG
    #include "PepXDebug.h"
  */
}











void 
HeaderMRSSQ::startMRS(const XML_Char *elementName, 
		      const XML_Char **attr) {
  inMRS = true;
  XMLNodePtr np = XMLNode::create(elementName, attr);
  np->startOffset_ = getByteOffset();
  tree.addNode(np);
}

  








  
void 
HeaderMRSSQ::startElement(const XML_Char *elementName, 
			  const XML_Char **attr) {
  string name = elementName;

  if (mode == HeaderMRSSQ::lookingForHeader) {

    if (!inHeader) {
      if (name != "msms_pipeline_analysis") {
	return;
      }
      else {
	inHeader = true;
	debug(debugout << "<found header start>");
      }
    }
    
    // if here, we're in the header: this is either the start tag or a subnode;/

    // stop if we see the first MRS
    if (name == "msms_run_summary") {

      //extract header info from tree;
      inHeader = false;
      debug(debugout << "///////////////");
      debug(debugout << "finished header ");
      debug(debugout << "tree top name is " << tree.top()->name_);
      XMLNodePtr headerNode = tree.top();
      debug(debugout << "header start offset: " << headerNode->startOffset_);
      debug(tree.getRootNode()->print(debugout));
      debug(debugout << "///////////////" << endl);

      pipeline_->extractHeaderInfo(headerNode);
      tree.popStack();
      tree.getRootNode()->removeChild(headerNode);

      debug(pipeline_->print());


      mode = HeaderMRSSQ::lookingForMRS;
      startMRS(elementName, attr);
      return;
    }
    else {
      XMLNodePtr np = XMLNode::create(elementName, attr);
      np->startOffset_ = getByteOffset();
      tree.addNode(np);
    }


  }

  
  else if (mode == HeaderMRSSQ::lookingForMRS) {

    if (!inMRS) {
      if (name != "msms_run_summary" ) {
	return;
      }
      else {
	inMRS = true;

	debug(debugout << "<found mrs start>" << endl);

      }
    }
    
    // in MRS: either start tag or inside
    if (name == "spectrum_query") {
      //extract MRS info from tree;
      inMRS = false;


      debug(debugout << "///////////////" << endl);
      debug(debugout << "finished MRS " << MRSCount << endl);
      debug(debugout << "tree top name is " << tree.top()->name_ << endl);


      XMLNodePtr curMRS = tree.top();

      debug(debugout << "current MRS start offset: " << curMRS->startOffset_ << endl);
      debug(tree.getRootNode()->print());
      debug(debugout << "::::::::::::" << endl);
      debug(curMRS->print());
      debug(debugout << "///////////////" << endl <<endl);

      pipeline_->mrsOffsets_.push_back(curMRS->startOffset_);
      pipeline_->extractMRSInfo(curMRS, MRSCount);
      MRSCount++;
      tree.popStack();
      tree.getRootNode()->removeChild(curMRS);

      debug(pipeline_->print());

      // set up the fields before starting the SQs!
      pipeline_->prepareFields();


      mode = HeaderMRSSQ::lookingForSQ;
      startSQ(elementName, attr);

      return;
    }
    else {
      XMLNodePtr np = XMLNode::create(elementName, attr);
      np->startOffset_ = getByteOffset();
      tree.addNode(np);
    }
    

  }// end mode: looking for MRS
  


  else if (mode == HeaderMRSSQ::lookingForSQ) {
    if (!inSQ) {
      if (name != "spectrum_query" ) {
	return;
      }
      else {
	// just hit the start of an sq
	inSQ = true;
#ifdef PRINT
	cout << "<found SQ start>" << endl;
#endif
	startSQ(elementName, attr);
	return;
      }
    }
    
    // otherwise, already in SQ: either the start tag or inside

    startSQ(elementName, attr);
    return;
    
    /*
      XMLNodePtr np = XMLNode::create(elementName, attr);
      // modify the names for our conventions
      if ( (name == "search_score") ||
      (name == "parameter") ) {
      // these special value nodes; ***rename the actual node-name*** to
      // include attr name
      name += "[@" +  np->attrs_[string("name")] + "]";
      np->name_ = name;
      //cout << "renamed node to: " << name << endl;
      }


      np->startOffset_ = getByteOffset();
      tree.addNode(np);
    */
    
  }


}
      










void
HeaderMRSSQ::endElementInSQ(const string& name) {

  XMLNodePtr np;

  if (name == "spectrum_query") {
    ++SQCount;
  }



  if (skipping) {
    // we have a chance to turn off skipping here
    if (name == "spectrum_query") {

      // at close of skipped sq,


      // the sq node was added to the tree;
      // delete it from tree *and remove parent's link to it*
      np = tree.top();
      while (np->name_ != "spectrum_query") {
	//cout << "skip end, clearing stack of " << np->name_ << endl;
	tree.popStack();
	np = tree.top();
      }

      /*
	cout << endl << "skipped" << endl;
	np->print();
	cout << endl << endl;
      */


      inSQ = false;
      skipping = false;
      tree.popStack();
      tree.top()->removeChild(np);
      np.reset();
      curSQ.reset();
      return;
    }
    
    /* 
       name != "spectrum_query", but still skipping in SQ
       otherwise, continue skipping until we reach the end of a sq node;
       then we'll clean up the tree;

       inelegant: leaving the tree in a state unreflective of current
       parse situation, but..

    */
  }

  else {
    // --not skipping--

    // -- accepting a non-skipped SQ node --
    if (name == "spectrum_query") {

      // this was the end of an accepted sq node
      ++acceptedSQCount;
#ifdef PRINT
      cout << acceptedSQCount << " sqs found" << endl;
#endif
      inSQ = false;
      np = tree.top();

      np->endOffset_ = getByteEndOffset();

      sqAcceptor_->acceptSQ(np);
      tree.popStack();
#ifdef PRINT
      cout << "before remove child: np, top: " << np->name_;
      cout << ", " << tree.top()->name_ << endl;
#endif
      tree.top()->removeChild(np);
#ifdef PRINT
      cout << "||| finished SQ |||" << endl;
      np->print();
      cout << "||| |||" << endl << endl;
#endif

      np.reset();
      curSQ.reset();

      /*
	cout << "accepted sq # " << acceptedSQ_
	<< ", cur mrs children: " << tree.top()->children_.size()
	<< endl;
      */
      return;

    }


    // else, in SQ, not skipping
    else if (inSQ) {
      tree.popStack();      
    }

  } // not skipping
  
}





  
void 
HeaderMRSSQ::endElement(const XML_Char *elementName) {
  string name = elementName;
  if (mode == HeaderMRSSQ::lookingForHeader ) {
    if (inHeader) {
      //normal end element handling;
      tree.popStack();
    }
    // otherwise do nothing

    return;
  }

  else if (mode == HeaderMRSSQ::lookingForMRS ) {

    if (inMRS) {
      //normal end element handling;
      tree.popStack();
    }
    // otherwise do nothing

    return;
  }
  else if (mode == HeaderMRSSQ::lookingForSQ) {
    XMLNodePtr np;
    np = tree.top();
    np->endOffset_ = getByteEndOffset();

    if (inSQ) {
      endElementInSQ(name);
    }
    else if (name == "msms_run_summary") {
      mode = HeaderMRSSQ::lookingForMRS;
      //tree.popStack();
      return;
    }
    else {
      // normal
      tree.popStack();
      return;
    }
	   
  }

}












/**

   check spectrum query node
   
   as we start with the opening spectrum_query tag, and descend, we
   can evaluate node attributes as we encounter them.  if any are
   enough to cause discarding of the entire spectrum_query, we can
   skip until the end of the query, and not add this spectrum_query to
   the tree.

   filters are stored on a per-node basis; look up the node (don't
   worry about creating entry on lookup); if there are filters, apply
   them, taking appropriate action based on filter type.


*/




bool 
HeaderMRSSQ::checkSQNode(XMLNodePtr node){
  
  // assume that the check for 
  // (options_->filterMap_[name].isEmpty_) was already done

  // this node is either the opening spectrum_query node or a subnode;
  // check the args, and if anything fails, return false.
  
  /*
    #define OLD_NDEBUG NDEBUG
    #undef _INCLUDED_DEBUG_H_
    #undef NDEBUG
    #include "PepXDebug.h"
  */  
  debug(debugout << "checking sq node: " << node->name_);
  
  XMLNodePtr np;

  double nval,tval;
  //string indent = "  ";
  vector<Filter*>* fvp;  
  
  /*
  static XMLNodePtr ss;
  ss.reset();
  ss = (node->findDescendent(string("search_score"),
			     string("name"),
			     field));
  */






  // specific tests

  if (node->name_ == "spectrum_query") {
    debug(debugout << "this is a spectrum_query node");

    // check charges
    if (!toDouble(node->attrs_[string("assumed_charge")],nval)) {
      nval = -1;
    }

    if (options_->excludeCharge1 && (nval == 1)) return false;
    if (options_->excludeCharge2 && (nval == 2)) return false;
    if (options_->excludeCharge3 && (nval == 3)) return false;
    if (options_->excludeChargeOthers &&
	(nval != 1) &&
	(nval != 2) &&
	(nval != 3)) return false;


    // check spectrum name
    if (options_->requireSpectrumText) {
      regex regSpectrum;
      regSpectrum = options_->requiredSpectrumText;
      string spectrumText = node->getAttrValue("spectrum");
      if (!regex_search(spectrumText, regSpectrum)) {
	return false;
      }
    }


  } 
  else if (node->name_ == "search_hit") {
    debug(debugout << "this is a search_hit node");

    string peptide = node->attrs_[string("peptide")];
    // N-linked glycosylation motif: X is any AA except pro (P)
    static const regex NxS_T("(?:N[A-OQ-Z](?:S|T))+");
    // sometimes B is also acceptable
    static const regex NBxS_T("(?:[NB][A-OQ-Z](?:S|T))+");

    static string::iterator si;

    static regex regPeptide;

    // check for NxS|T glycosolation motif
    if (options_->requireGlyc) {
      // (?:pattern) lexically groups pattern, without generating an additional sub-expression.
      if (options_->includeBinGlycMotif) {
	// use the [NB] glyc motif
	if (!regex_search(peptide, NBxS_T)) return false;
      }
      else {
	if (!regex_search(peptide, NxS_T)) return false;
      }
    }

    // check required peptide AAs (string of single-letter AA codes)
    if (options_->requireAA) {
      for (si = options_->requiredAA.begin();si != options_->requiredAA.end(); si++) {
	char buf[2];
	buf[0] = (*si);
	buf[1] = 0;
	string sc;
	//sc += "[A-Z]*";
	sc += buf;
	//sc += "[A-Z]*";
	regPeptide = sc;
	//cout << "***testing req AA: " << sc << endl;
	if (!regex_search(peptide, regPeptide)) return false;
      }
    }


    // check required peptide text
//     if (options_->requirePeptideText) {
      
//       // add modified n term indicator if required
//       string nTermMod = node
// 	->findChild("modification_info")
// 	->getAttrValue("mod_nterm_mass");
//       if (nTermMod != "") {
// 	peptide = string("n") + peptide;
//       }

//       // add modified c term indicator if required
//       string cTermMod = node
// 	->findChild("modification_info")
// 	->getAttrValue("mod_cterm_mass");
//       if (cTermMod != "") {
// 	peptide = peptide + string("c");
//       }


//       //cout << "requred p txt: " << options_->requiredPeptideText << endl;
//       // *? matches 0 or more, consuming as little as possible
//       regPeptide = (/*"[A-Z].*?" +*/  options_->requiredPeptideText /* + "[A-Z]*?"*/ );
//       if (!regex_search(peptide, regPeptide)) return false;
//     }


    // check required protein text
    if (options_->requireProteinText) {
      regex regProtein;
      string proteinText = node->getAttrValue("protein");
      regProtein = options_->requiredProteinText;
      if (!regex_search(proteinText, regProtein)) {
	return false;
      }
    }

  } // end search_hit


  
  debug(debugout << "this is a non-general node (" << node->name_ << ")");
  

  // don't limit filters to specific nodes; there are too many possibilities

  bool checkCharge = false;
  //cout << "curcharge: " <<  (curSQ->getAttrValue("assumed_charge")) << "<br/>" << endl;
  int sqCharge = -1;
  if (!toInt(curSQ->getAttrValue("assumed_charge"),sqCharge)) {
    sqCharge=-1;
  }



  // go through all the filters for this node
  fvp = &(options_->filterMap_[node->name_].filterVec_);
  for (vector<Filter*>::size_type i=0; i!= fvp->size(); i++) {
    //cout << indent << "testing : " << endl;
    //(*fvp)[i]->print();
    //cout << "vs" << endl;
    checkCharge = (*fvp)[i]->chargeSpecfication_;

    // skip this filter if it's charge-specific and the wrong charge
    if (checkCharge &&
	((*fvp)[i]->charge_ != sqCharge)) {
      continue;
    }


    nval = -999; // TODO: something better
    tval = (*fvp)[i]->testValue_;

    if (!(*fvp)[i]->field_.isValueNode_) {
      if (!toDouble(node->attrs_[(*fvp)[i]->attrName_], nval)) {
	nval = -999;
      }
    }
    else {
      if (!toDouble(node->attrs_[string("value")], nval)) {
	nval = -999;
      }
    }

    switch ((*fvp)[i]->filterType_) {
    case Filter::min:
      if (nval < tval) return false;
      break;
    case Filter::max:
      if (nval > tval) return false;
      break;
    case Filter::equals:
      if (nval != tval) return false;
      break;      
    default:
      // error
      cout << "testing error" << endl;
      throw runtime_error("filtering error");
    }
  }

  // if we made it here, we passed all the tests

  //cout << nval << endl;

  //cout << endl << endl;
  debug(debugout << node->name_ << " passed filters!" << endl << endl);
  return(true);
}
