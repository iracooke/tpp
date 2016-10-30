
#include "ICATMixtureDistr.h"


/*

Program       : ICATMixtureDistr for PeptideProphet                                                       
Author        : Andrew Keller <akeller@systemsbiology.org>                                                       
Date          : 11.27.02 

Copyright (C) 2003 Andrew Keller

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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Andrew Keller
Institute for Systems Biology
401 Terry Avenue North 
Seattle, WA  98109  USA
akeller@systemsbiology.org

*/
// static class variables
double ICATMixtureDistr::light_icat_masses_[] = {545.2, 330.26}; // old fashioned, cleavable
double ICATMixtureDistr::heavy_icat_masses_[] = {553.2, 339.26}; // 
int ICATMixtureDistr::num_icat_ = 2; // number of icat labels (array length)

ICATMixtureDistr::ICATMixtureDistr(int charge, const char* name, const char* tag, int icatType) : DiscreteMixtureDistr(charge, 2, name, tag) {
  const char* bindefs[] = {"icat=0 (incompatible)", "icat=1 (compatible)"};
  maxdiff_ = 0.005;
  negOnly_ = True;

 // HENRY: set the icat_masses accordingly
  if (icatType == 1) {
  	// cleavable only
  	ICATMixtureDistr::light_icat_masses_[0] = 330.26;
  	ICATMixtureDistr::heavy_icat_masses_[0] = 339.26;
  	ICATMixtureDistr::num_icat_ = 1;
  	
  } else if (icatType == 2) {
  	// uncleavable only
	ICATMixtureDistr::num_icat_ = 1;  	
  }

  DiscreteMixtureDistr::init(bindefs);
}

void ICATMixtureDistr::enter(SearchResult* result) {
#ifdef USE_STD_MODS
  int light = -1;
  int heavy = -1;
  double error = 1.0;

  int icat_index = -1;

  //  cout << "ready to enter...";
 
  if(result->mod_info_ == NULL) { // just check for c
    //    cout << " w/ no modifications!" << endl;
    //    if(strchr(result->peptide_, 'C') != NULL)
    //      MixtureDistr::enter(0, 1);
    //    else
    MixtureDistr::enter(0, 0);
    return;
  }
  else { // check mods

    //    cout << result->peptide_ << ": ";
    //    for(int k = 0; k < result->mod_info_->getNumModAAs(); k++) {
    //      cout << result->mod_info_->getModAAPos(k) << "=" << result->mod_info_->getModAAMass(k) << " ";
    //    }
    //    cout << endl;

    //    cout << " with " << result->mod_info_->getNumModAAs() << " mod aa's" << endl;
    for(int k = 0; k < result->mod_info_->getNumModAAs(); k++) {
      int nextpos = result->mod_info_->getModAAPos(k);
      //      cout << "nextpos: " << nextpos << " vs " << strlen(result->peptide_) << endl;
      if(result->peptide_[nextpos-1] == 'C') { // check whether light or heavy
	double nextmass = result->mod_info_->getModAAMass(k);
	for(int j = 0; j < num_icat_; j++) {
	  if(nextmass - light_icat_masses_[j] <= error && light_icat_masses_[j] - nextmass <= error) {
	    if(heavy > -1 || (light > -1 && light != j)) { // return 0, two diff't labels
	      MixtureDistr::enter(0, 0);
	      return;
	    }
	    light = j;
	  }
	  if(nextmass - heavy_icat_masses_[j] <= error && heavy_icat_masses_[j] - nextmass <= error) {
	    if(light > -1 || (heavy > -1 && heavy != j)) { // return 0, two diff't labels
	      MixtureDistr::enter(0, 0);
	      return;
	    }
	    heavy = j;
	  }
	} // next icat label

      } // c mod
    } // next pos
    int peplen = (int)strlen(result->peptide_);
    Boolean cont = False;
    for (int i = 0 ; i < peplen; i++) {
      cont = False;
      if (result->peptide_[i] == 'C') {
	for(int j = 0; j < result->mod_info_->getNumModAAs(); j++) {
	  int nextpos = result->mod_info_->getModAAPos(j);
	  if (nextpos-1 == i) {
	    cont = True;
	  }
	}
	if (cont) continue;
	//Found an unmodified 'C'
	MixtureDistr::enter(0, 0);
	return;
      }
    }


    if(light > -1 || heavy > -1) {
      MixtureDistr::enter(0, 1);
      //cout << "returning 1" << endl;
    }
    else
      MixtureDistr::enter(0, 0);
    return;
  } // heav mod info

#endif

  MixtureDistr::enter(0, result->peptide_);
}

#ifndef USE_STD_MODS
int ICATMixtureDistr::inttranslate(const char* val) {
  if(icatCompatible(val)) {
    return 1;
  }
  return 0;
}
#endif

Boolean ICATMixtureDistr::icatCompatible(const char* pep) {
  Boolean light = False;
  Boolean heavy = False;
  int peplen = (int)strlen(pep);
  for(int k = 0; k < peplen-1; k++) {
    if(pep[k] == 'C') {
      if(pep[k+1] < 'A' || pep[k+1] > 'Z') {


	//if(pep[k+1] == '*' || pep[k+1] == '#' || pep[k+1] == '@') {
	heavy = True;
      }
      else {
	light = True;
      }
    }
  }
  if(pep[peplen-1] == 'C') {
    light = True;
  }
  if(light && heavy) {
    return False;
  }
  if(! light && ! heavy) {
    return False;
  }
  return True;
}

Boolean ICATMixtureDistr::isICAT(double mass, double error) {
  for(int k = 0; k < num_icat_; k++)
    if(light_icat_masses_[k] - mass <= error && mass - light_icat_masses_[k] <= error)
      return True;
    else if(heavy_icat_masses_[k] - mass <= error && mass - heavy_icat_masses_[k] <= error)
      return True;

  return False;
}
