
#include "GlycMixtureDistr.h"
  

/*

Program       : GlycMixtureDistr for PeptideProphet                                                       
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

                
GlycMixtureDistr::GlycMixtureDistr(int charge, const char* name, const char* tag) : DiscreteMixtureDistr(charge, 2, name, tag) {
  const char* bindefs[] = {"glyc=0 (without glyc motif)", "glyc=1 (with glyc motif)"};
  maxdiff_ = 0.005;
  negOnly_ = True;
  DiscreteMixtureDistr::init(bindefs);
}

void GlycMixtureDistr::enter(SearchResult* result) {
  // include modifications here if nec
  // i.e. S/T cannot be modified
#ifdef USE_STD_MODS
  for(int k = 0; k < (int) strlen(result->peptide_) - 2; k++) {

    if((result->peptide_[k] == 'N' || result->peptide_[k] == 'B') && result->peptide_[k+1] != 'P' &&
       (result->peptide_[k+2] == 'S' || result->peptide_[k+2] == 'T') &&
       (result->mod_info_ == NULL || ! result->mod_info_->isModifiedResidue(k+2))) {
      MixtureDistr::enter(0, 1);
      return;
    }

  }
  // still here
  MixtureDistr::enter(0, 0);
#endif
#ifndef USE_STD_MODS
  MixtureDistr::enter(0, result->peptide_);
#endif
}


#ifndef USE_STD_MODS
int GlycMixtureDistr::inttranslate(const char* val) {
  if(hasGlycMotif(val)) {
    return 1;
  }
  return 0;
}
#endif

Boolean GlycMixtureDistr::hasGlycMotif(const char* pep) {
  int peplen = (int)strlen(pep);
  for(int k = 0; k < peplen-2; k++) {

    if(pep[k] == 'N') {

      if(pep[k+1] != 'P' && (pep[k+2] == 'T' || pep[k+2] == 'S')) 
	return True;
      else if(k < peplen - 3 && isModification(pep[k+1]) && pep[k+2] != 'P' && (pep[k+3] == 'T' || pep[k+3] == 'S'))
	return True;
      else if(k < peplen - 3 && pep[k+1] != 'P' && isModification(pep[k+2]) && (pep[k+3] == 'T' || pep[k+3] == 'S')) 
	return True;
      else if(k < peplen - 4 && isModification(pep[k+1]) && pep[k+2] != 'P' && isModification(pep[k+3]) &&
	    (pep[k+4] == 'T' || pep[k+4] == 'S')) 
	return True;
    }
  } // next position in pep
  return False; // still here
}

Boolean GlycMixtureDistr::isModification(char c) {
  return (c == '*' || c == '#' || c == '@');
}
