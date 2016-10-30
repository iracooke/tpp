#ifndef _RESPECTFILTER_H_
#define _RESPECTFILTER_H_
/*

Program       : Respectfilter                                                       
Author        : David Shteynberg <dshteynb  AT systemsbiology.org>                                                       
Date          : 03.18.2011

Primary data object holding all mixture distributions for each precursor ion charge

Copyright (C) 2011 David Shteynberg

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

David Shteynberg
Institute for Systems Biology
401 Terry Avenue North 
Seattle, WA  98109  USA

*/

#include "common/tpp_hashmap.h"
#include "Search/SpectraST/Peptide.hpp"
#include "Search/SpectraST/SpectraSTPepXMLLibImporter.hpp"
#include "Search/SpectraST/SpectraSTPeakList.hpp"
#include <sstream>

#ifndef __LGPL__
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_multifit.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_combination.h>
#include <gsl/gsl_sort_vector.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_cdf.h>
#endif


#include <pwiz/data/msdata/MSDataFile.hpp>
#include <pwiz/data/msdata/MSData.hpp>
#include <pwiz/data/msdata/IO.hpp>
#include <pwiz/data/msdata/SpectrumInfo.hpp>
#include <pwiz/utility/misc/IterationListener.hpp>
#include <pwiz_tools/common/FullReaderList.hpp>
#include <pwiz/data/common/CVTranslator.hpp>
#include <pwiz/utility/misc/Std.hpp>
#include <pwiz/data/msdata/Serializer_mzML.hpp>

using namespace std;
using namespace pwiz::data;
using namespace pwiz::msdata;

typedef TPP_STDSTRING_HASHMAP(string*) strp_hash;
typedef TPP_STDSTRING_HASHMAP(SpectrumPtr) spectptr_hash;

class RespectFilter {

 public:
  RespectFilter(string& out_file, string& in_file, bool mzML=false);
  
  void init( string& spec_name, 
	     string& pep, int charge, 
	     double prob, double parentMass,
	     cRamp* cramp, long scan, 
	     TPP_HASHMAP_T<char, double>* stat_mods, 
	     TPP_HASHMAP_T<char, double>* stat_prot_termods, 
	     bool is_nterm_pep, bool is_cterm_pep, 
	     double tol = 0.1, double itol = 0.1, bool keepChg=false);

  ~RespectFilter();
  
  bool set();


  void write();
 
  int NAA() { return NAA_; }
  
  Peptide* getPeptide() { return pep_; }
  void run();
  void setTolerance(double tol) {
    tolerance_ = tol;
  }

  void setIsoTolerance(double tol) {
   isoTolerance_ = tol;
  }

 

  void evaluateModPep(Peptide* mpep, double isoTOL);

  void processPeakList();
  
 private:
  
  Peptide* pep_;
  string* pep_str_;
  string* spect_str_;
  SpectraSTLibEntry* entry_;
  SpectraSTPeakList* peakList_;
  long scan_;
  unsigned long num_spectra_;
  int charge_;
  cRamp* cramp_;
  int NAA_;
  double tolerance_;
  double isoTolerance_;
  bool etd_;

  double prob_;

  bool unknown_mod_;
  

  int recur_index_; 

  int nDECOY_;
  
  double parentMZ_;
  double parentMass_;

  double TIC_;

  double minInt_;

  double maxInt_;

  bool is_nterm_pep_;
  bool is_cterm_pep_;
  string pep_unmod_str_;

  string out_dir_;
  string out_file_;
  string in_file_;

  //  int minChg_;
  //int maxChg_;

  bool keepChg_;
  //vector<MZIntensityPair*> all_peaks_;
  pwiz::msdata::MSDataFile* inmsd_;
  pwiz::msdata::MSDataFile* outmsd_;
  shared_ptr<SpectrumListSimple> spectrumList;// (new SpectrumListSimple);

  //FullReaderList readers_;
  //const ReaderList::Config readerConfig_;
  //SpectrumListPtr inSpecList_;
  SpectrumListPtr outSpecList_;
  std::vector<SpectrumPtr> reportSpectra_;
  TPP_HASHMAP_T<int, SpectrumPtr>* spectra_hash_;
  
  //SpectrumListSimplePtr specList_;
  // shared_ptr<pwiz::msdata::SpectrumListSimple> specList_;// (new SpectrumListSimple);
  
  pwiz::msdata::MSDataFile::WriteConfig writeConfig_;

  //vector<MSData*> msdList_;
  rampScanInfo* scanInfo_;

};



#endif
