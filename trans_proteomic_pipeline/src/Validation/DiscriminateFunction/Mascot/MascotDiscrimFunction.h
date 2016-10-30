#ifndef MAS_DISCR_FUN_H
#define MAS_DISCR_FUN_H

#include "Validation/DiscriminateFunction/DiscriminantFunction.h"
#include "Parsers/Algorithm2XML/SearchResult/SearchResult.h"
#include "common/sysdepend.h"
#include "Parsers/Algorithm2XML/SearchResult/MascotResult.h"
#include "MascotScoreParser.h"

/*

Program       : MascotDiscriminantFunction for discr_calc of PeptideProphet                                                       
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

class MascotDiscrimFunction : public DiscriminantFunction {

 public:

  MascotDiscrimFunction(int charge);
  MascotDiscrimFunction(int charge, char* xmlfile);
  MascotDiscrimFunction(int charge, char* xmlfile, MascotStarOption star = MASCOTSTAR_PENALIZE);
  virtual ~MascotDiscrimFunction();
  Boolean isComputable(SearchResult* result);
  double getDiscriminantScore(SearchResult* result);
  //double getMeanIdentity(char* xmlfile);
  char* getMascotScoreParserDescription();

 protected:

  double ionscore_wt_;
  double identity_wt_;
  double ave_identity_;
  MascotScoreParser* parser_;
  MascotStarOption star_;
};

#endif
