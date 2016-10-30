#include "MascotDiscrimFunction.h"

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

MascotDiscrimFunction::MascotDiscrimFunction(int charge) : DiscriminantFunction(charge) { 
  double consts[] = {-3.0, -3.0, -3.0, -3.0, -3.0, -3.0, -3.0};
  double ionscores[] = {0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1};
  double identities[] = {-0.1, -0.1, -0.1, -0.1, -0.1, -0.1, -0.1};
  const_ = consts[charge_];
  ionscore_wt_ = ionscores[charge_];

  identity_wt_ = identities[charge_];
  ave_identity_ = -1.0; // unset
  parser_ = NULL;
}

MascotDiscrimFunction::MascotDiscrimFunction(int charge, char* xmlfile, MascotStarOption star) : DiscriminantFunction(charge) { 

  double consts[] = {-3.0, -3.0, -3.0, -3.0, -3.0, -3.0, -3.0};
  double ionscores[] = {0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1};
  double identities[] = {-0.1, -0.1, -0.1, -0.1, -0.1, -0.1, -0.1};
  const_ = consts[charge_];
  ionscore_wt_ = ionscores[charge_];
  star_ = star;

  identity_wt_ = identities[charge_];
  ave_identity_ = -1.0; // unset

  parser_ = new MascotScoreParser(xmlfile, charge_+1, star_);
  if(parser_ != NULL) 
    ave_identity_ = parser_->getMeanIdentityScore();

  //ave_identity_ = getMeanIdentity(xmlfile);
  if(ave_identity_ == -1.0) {
    cout << "error: ave identity: -1.0" << endl;
    exit(1);
  }

}


MascotDiscrimFunction::~MascotDiscrimFunction() {
  if(parser_ != NULL)
    delete parser_;
}

/*
double MascotDiscrimFunction::getMeanIdentity(char* xmlfile) {
  double output = -1.0;
  MascotIdentityParser* parser = new MascotIdentityParser(xmlfile, charge_);
  if(parser != NULL) {
    output = parser->getMean();
    delete parser;
  }
    
  return output;
}
*/

char* MascotDiscrimFunction::getMascotScoreParserDescription() {
  const int len = 1000;

  if(parser_ == NULL)
    return NULL;
  char* output = new char[len];
  parser_->setMascotScoreParserDescription(output, len);
  return output;
}

Boolean MascotDiscrimFunction::isComputable(SearchResult* result) {
  return True;
}
  
double MascotDiscrimFunction::getDiscriminantScore(SearchResult* result) {
  if(strcasecmp(result->getName(), "Mascot") != 0) {
    cerr << "illegal type of Mascot result: " << result->getName() << endl;
    exit(1);
  }
  MascotResult* mascotresult = (MascotResult*)(result);

  double adj_ionscore_identity_diff = mascotresult->ionscore_ - mascotresult->identity_;
  double tot = const_;

//   if (star_ == MASCOT_EXPECT) {
//     //    const_ = 3.0;
//     if ((double)mascotresult->expect_ <= 0) {
//       tot = (0-log(1e-10));
//     }
//     else {
//       tot = (0-log((double)mascotresult->expect_));
//     }
//   }
//   else {
    // make adjustment here
    if(parser_ != NULL) {
      double max = parser_->getMaxAdjustedIonscoreIdentityScoreDiff(mascotresult->ionscore_ - mascotresult->homology_);
      if(adj_ionscore_identity_diff > max)
	adj_ionscore_identity_diff = max;
    }
    
    
    if(ave_identity_ == -1.0)
      ave_identity_ = mascotresult->ave_identity_;
    
    tot += ionscore_wt_ * (adj_ionscore_identity_diff + ave_identity_);
    //  }


    if (star_ == MASCOT_EXPECT) {
      //    const_ = 3.0;
      if ((double)mascotresult->expect_ <= 0) {
	tot = (tot-log(1e-10))/2;
      }
      else {
	tot = (tot-log((double)mascotresult->expect_))/2;
      }
 }

  //tot += ionscore_wt_ * (mascotresult->ionscore_ - mascotresult->identity_ + ave_identity_);
  //  tot += ionscore_wt_ * (mascotresult->ionscore_ - mascotresult->identity_ + mascotresult->ave_identity_);
  //tot += ionscore_wt_ * mascotresult->ionscore_;
  //tot += identity_wt_ * mascotresult->identity_;

  return tot;
}
