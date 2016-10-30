
#include "PhosphoMixtureDistr.h"
  

/*

Program       : PhosphoMixtureDistr for PeptideProphet                                                       
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

                
PhosphoMixtureDistr::PhosphoMixtureDistr(int charge, const char* name, const char* tag) : DiscreteMixtureDistr(charge, 4, name, tag) {
  //  const char* bindefs[] = {"phospho=0 (without S/T/Y)", "phospho=1 (with unmodified S/T/Y)", "phospho=2 (with Phospho S/T/Y and NO non-Phospho S/T/Y-P)",  "phospho=3 (with Phospho S/T/Y and non-Phospho S/T/Y-P)", "phospho=4 (with Phospho-P S/T/Y-P)"};
  const char* bindefs[] = {"phospho=0 (without S/T/Y)", "phospho=1 (with unmodified S/T/Y)", "phospho=2 (with Phospho S/T/Y)",  "phospho=3 (with Phospho S/T/Y-P)"};
  maxdiff_ = 0.005;
  negOnly_ = True;
  DiscreteMixtureDistr::init(bindefs);
}

void PhosphoMixtureDistr::enter(SearchResult* result) {
  // include modifications here if nec
  // i.e. S/T cannot be modified
  int bin = 0;
#ifdef USE_STD_MODS
  for(size_t k = 0; k < strlen(result->peptide_); k++) {

    if(result->peptide_[k] == 'S' || result->peptide_[k] == 'T'  || result->peptide_[k] == 'Y'  ) {
      if (bin < 1)
	bin = 1;
      if (result->mod_info_ != NULL && result->mod_info_->isModifiedResidue((int)k)) {
	if (bin < 2)
	  bin = 2;
	if (k < strlen(result->peptide_)-1 && 
	    (result->peptide_[k+1] == 'P' 

	     //||
	     // result->peptide_[k+1] == 'D' ||
	     //result->peptide_[k+1] == 'E' || 
	     //result->peptide_[k+1] == 'F' ||
	     //result->peptide_[k+1] == 'M' 

	     ) ) {
	 
	  //	  bin = 4;
	  bin = 3;
	  break;
	}
      }
      //      else {
      //	if (bin == 2 && k < strlen(result->peptide_)-1 && 
      //	    (result->peptide_[k+1] == 'P' 


	     //||
	     //result->peptide_[k+1] == 'D' ||
	     //result->peptide_[k+1] == 'E' || 
	     //result->peptide_[k+1] == 'F' ||
	     //result->peptide_[k+1] == 'M'

      //	     ) ) {
      //	  bin = 3;
      //	}

      //    }
    
    }

  }
  // still here
  MixtureDistr::enter(0, bin);
#endif
#ifndef USE_STD_MODS
  MixtureDistr::enter(0, result->peptide_);
#endif
}


#ifndef USE_STD_MODS
int PhosphoMixtureDistr::inttranslate(const char* val) {
  if(hasPhosphoMotif(val)) {
    return 1;
  }
  return 0;
}
#endif

Boolean PhosphoMixtureDistr::hasPhosphoMotif(const char* pep) {
  int peplen = (int)strlen(pep);
  for(int k = 0; k < peplen; k++) {

    if(pep[k] == 'S' || pep[k] == 'T' || pep[k] == 'Y' ) {
	return True;
    }
  } // next position in pep
  return False; // still here
}

Boolean PhosphoMixtureDistr::isModification(char c) {
  return (c == '*' || c == '#' || c == '@');
}
