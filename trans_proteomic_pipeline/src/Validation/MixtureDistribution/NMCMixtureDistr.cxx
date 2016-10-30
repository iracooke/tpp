#include "NMCMixtureDistr.h"

/*

Program       : NMCMixtureDistr for PeptideProphet                                                       
Author        : Andrew Keller <akeller@systemsbiology.org>                                                       
Date          : 11.27.02 

Mixture distributions for peptide number of missed tryptic cleavages

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


NMCMixtureDistr::NMCMixtureDistr(int charge, const char* name, const char* tag) : DiscreteMixtureDistr(charge, 3, name, tag) {
  const char* bindefs[] = {"nmc=0", "1<=nmc<=2", "nmc>=3"};
  //NMCMixtureDistr::NMCMixtureDistr(int charge, const char* name, const char* tag) : DiscreteMixtureDistr(charge, 4, name, tag) {
  //char* bindefs[] = {"short", "nmc=0", "1<=nmc<=2", "nmc>=3"};
  maxdiff_ = 0.005;
  negOnly_ = True;
  DiscreteMixtureDistr::init(bindefs);
}


int NMCMixtureDistr::inttranslate(char* val) {
  int value = atoi(val);
  /*
  if(value < 0)
    return 0;
  if(value < 5)
    return 1;
  if(value < 16)
    return 2;
  return 3;
  */

  if(value < 1) {
    return 0;
  }
  else if(value < 3) {
    return 1;
  }
  return 2;
}

void NMCMixtureDistr::enter(SearchResult* result) {
  if(result->num_missed_cl_ < 1)
    MixtureDistr::enter(0, 0);
  else if(result->num_missed_cl_ < 3)
    MixtureDistr::enter(0, 1);
  else
    MixtureDistr::enter(0, 2);
}
