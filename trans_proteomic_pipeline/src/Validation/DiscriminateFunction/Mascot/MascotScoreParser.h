#ifndef MASCOT_SCORE_PARSER_H
#define MASCOT_SCORE_UPDATE_PARSER_H
/*

Program       : MascotScoreParser                                                      
Author        : Andrew Keller <akeller@systemsbiology.org>                                                       
Date          : 11.27.02 

Primary data object holding all mixture distributions for each precursor ion charge

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


#include <stdio.h>
#include <math.h>
#include <time.h>

#include "Parsers/Parser/Parser.h"
#include "Parsers/Parser/TagFilter.h"
#include "Quantitation/Option.h"




class MascotScoreParser : public Parser {

 public:

  MascotScoreParser(char* xmlfile, int charge);
  MascotScoreParser(char* xmlfile, int charge, MascotStarOption star = MASCOTSTAR_PENALIZE);
  virtual ~MascotScoreParser() {};

  void setFilter(Tag* tag);
  Boolean update();
  double getMeanIdentityScore(); 
  double getMaxAdjustedIonscoreIdentityScoreDiff(double adj_ionscore_homology);
  void setMascotScoreParserDescription(char* description, int descr_length);

 protected:

  void parse(const char* xmlfile);

  double identity_mean_;
  int identity_tot_;

  double adj_ionscore_identity_mean_;
  double adj_ionscore_identity_meansq_;
  double adj_ionscore_identity_stdev_;
  double adj_ionscore_homology_mean_;
  double adj_ionscore_homology_meansq_;
  double adj_ionscore_homology_stdev_;
  double adj_ionscore_corr_;
  double regression_error_;

  int adj_ionscore_tot_;

  int charge_;

  int min_adj_ionscore_;

  double slope_;
  double intercept_;
  double regression_stdev_;

  int num_stdevs_;

  Boolean regression_complete_;
  MascotStarOption star_;
};











#endif
