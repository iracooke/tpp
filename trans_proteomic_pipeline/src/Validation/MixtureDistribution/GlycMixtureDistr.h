#ifndef GLYC_MIX_H
#define GLYC_MIX_H

#include "DiscreteMixtureDistr.h"


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


class GlycMixtureDistr : public DiscreteMixtureDistr {

 public:

  GlycMixtureDistr(int charge, const char* name, const char* tag);
#ifndef USE_STD_MODS
  int inttranslate(const char* val);
#endif
  Boolean hasGlycMotif(const char* pep);
  Boolean isModification(char c);
  void enter(SearchResult* result);

};


#endif
