#ifndef MIX_DISTR_FAC_H
#define MIX_DISTR_FAC_H

#include "Parsers/Algorithm2XML/SearchResult/SearchResult.h"
#include "Parsers/Algorithm2XML/SearchResult/SequestResult.h"
#include "Parsers/Algorithm2XML/SearchResult/CruxResult.h"
#include "Parsers/Algorithm2XML/SearchResult/MascotResult.h"
#include "Parsers/Algorithm2XML/SearchResult/ProbIDResult.h"
#include "Parsers/Algorithm2XML/SearchResult/TandemResult.h"
//#include "Parsers/Algorithm2XML/SearchResult/HydraResult.h"
#include "Parsers/Algorithm2XML/SearchResult/PhenyxResult.h"
#include "Parsers/Algorithm2XML/SearchResult/OMSSAResult.h"
#include "Parsers/Algorithm2XML/SearchResult/YABSEResult.h"
#include "Parsers/Algorithm2XML/SearchResult/MyrimatchResult.h"
#include "Parsers/Algorithm2XML/SearchResult/MSGFDBResult.h"
#include "Parsers/Algorithm2XML/SearchResult/MSGFPLResult.h"
#include "Parsers/Algorithm2XML/SearchResult/InspectResult.h"
#include "Parsers/Parser/Tag.h"
#include "common/Array.h"
#include "Validation/DiscriminateFunction/DiscrimValMixtureDistr.h"
#include "Validation/DiscriminateFunction/Mascot/MascotDiscrimValMixtureDistr.h"
#include "Validation/DiscriminateFunction/Comet/CometDiscrimValMixtureDistr.h"
#include "Validation/DiscriminateFunction/ProbID/ProbIDDiscrimValMixtureDistr.h"
#include "Validation/DiscriminateFunction/Tandem/TandemDiscrimValMixtureDistr.h"
//#include "Validation/DiscriminateFunction/Hydra/HydraDiscrimValMixtureDistr.h"
#include "Validation/DiscriminateFunction/Phenyx/PhenyxDiscrimValMixtureDistr.h"
#include "Validation/DiscriminateFunction/OMSSA/OMSSADiscrimValMixtureDistr.h"
#include "Validation/DiscriminateFunction/YABSE/YABSEDiscrimValMixtureDistr.h"
#include "Validation/DiscriminateFunction/Myrimatch/MyrimatchDiscrimValMixtureDistr.h"
#include "Validation/DiscriminateFunction/MSGFDB/MSGFDBDiscrimValMixtureDistr.h"
#include "Validation/DiscriminateFunction/MSGFPL/MSGFPLDiscrimValMixtureDistr.h"
#include "Validation/DiscriminateFunction/Inspect/InspectDiscrimValMixtureDistr.h"
// HENRY: add SpectraST
#include "Validation/DiscriminateFunction/SpectraST/SpectraSTDiscrimValMixtureDistr.h"
#include "Validation/DiscriminateFunction/Crux/CruxDiscrimValMixtureDistr.h"
#include "NTTMixtureDistr.h"
#include "NMCMixtureDistr.h"
/*

Program       : MixtureDistFactory                                                     
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

#include "MassDifferenceDiscrMixtureDistr.h"
// HENRY: add MzDifference
#include "MzDifferenceDiscrMixtureDistr.h"
#include "IsoMassDiffDiscrMixtureDistr.h"
#include "ICATMixtureDistr.h"
#include "GlycMixtureDistr.h"
#include "PhosphoMixtureDistr.h"
#include "VariableOffsetMassDiffDiscrMixtureDistr.h"
#include "AccurateMassDiffDiscrMixtureDistr.h"
#include "Quantitation/Option.h"
#include "pIMixtureDistr.h"
#include "VariableOffsetpIMixtureDistr.h"
#include "KernelDensityPIMixtureDistr.h"
#include "RTMixtureDistr.h"
//#include "VariableOffsetRTMixtureDistr.h"
#include "KernelDensityRTMixtureDistr.h"

class MixtureDistrFactory {


 public:
  MixtureDistrFactory();
  //MixtureDistrFactory(char* engine, char* enzyme, char* massspec, ScoreOptions options);
  MixtureDistrFactory(ModelOptions& modelopts, ScoreOptions& options); 


  SearchResult* getSearchResult(Array<Tag*>* tags);
  SearchResult* getSearchResult(Array<Tag*>* tags, char* engine);
  Boolean applyOptions(SearchResult* result);
  SearchResult* getSearchResultWithAppliedOpts(Array<Tag*>* tags);
  DiscrimValMixtureDistr* getDiscrimValMixtureDistr(int charge);
  DiscrimValMixtureDistr* getDiscrimValMixtureDistr(int charge, Boolean gamma);
  DiscrimValMixtureDistr* getDiscrimValMixtureDistr(int charge, Boolean gamma, Boolean nonparam);
  DiscrimValMixtureDistr* getDiscrimValMixtureDistr(int charge, Boolean gamma, Boolean nonparam, Boolean use);
  IsoMassDiffDiscrMixtureDistr* getIsoMassDiffDiscrMixtureDistr(int charge);
  NTTMixtureDistr* getNTTMixtureDistr(int charge);
  NMCMixtureDistr* getNMCMixtureDistr(int charge);
  MassDifferenceDiscrMixtureDistr* getMassDifferenceDiscrMixtureDistr(int charge);
  // HENRY: Precursor m/z distribution
  MzDifferenceDiscrMixtureDistr* getMzDifferenceDiscrMixtureDistr(int charge);
  // HENRY: modify prototype
  ICATMixtureDistr* getICATMixtureDistr(int charge, int icatType = 0);

  GlycMixtureDistr* getGlycMixtureDistr(int charge);
  PhosphoMixtureDistr* getPhosphoMixtureDistr(int charge);
  pIMixtureDistr* getPIMixtureDistr(int charge);
  //  VariableOffsetRTMixtureDistr* getRTMixtureDistr(int charge);
  RTMixtureDistr* getRTMixtureDistr(int charge);
  const char* getDiscrMixtureDistrName();
  const char* getNTTMixtureDistrName();
  const char* getNTTMixtureDistrName(const char* enzyme);

 protected:
  
  std::string search_engine_;
  std::string enzyme_;
  std::string mass_spec_type_;
  ScoreOptions scoreOpts_;
  ModelOptions modelOpts_;
  char discr_mixture_distr_name_[500];
  char ntt_mixture_distr_name_[500];


};


#endif
