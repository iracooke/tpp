#include "RespectFilter.h"
/* ******************************************************************************

Program       : RespectFilter                                                       
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

******************************************************************************** */
#define H0MASS 1.0078250321
#define MINPROB 0.2
using namespace std;  
using namespace pwiz::msdata;

RespectFilter::RespectFilter(string& out_file, string & in_file, bool mzML) {
 
  //  out_dir_ = out_dir;
  in_file_ = in_file;
  out_file_ = out_file;
  scanInfo_ = NULL;
  pep_str_ = NULL;
  spect_str_ = NULL;
  pep_ = NULL;
  entry_ = NULL;

  std::string::size_type pos = 0;

  keepChg_ = false;

  spectra_hash_ = new  TPP_HASHMAP_T<int, SpectrumPtr>();

  writeConfig_.format = pwiz::msdata::MSDataFile::Format_mzXML;

  if (mzML || (pos = in_file.find("mzML")) != std::string::npos) {
    writeConfig_.format = pwiz::msdata::MSDataFile::Format_mzML;
  }
  


  
  num_spectra_ = 0;
  try {
    inmsd_ = new  pwiz::msdata::MSDataFile(in_file_);
    //outmsd_ = new  pwiz::msdata::MSDataFile(in_file_);
  }
  catch (...) {
    cerr << "WARNING: Unable to open file: " << in_file_ << endl;

  }

  //specList_ = new SpectrumListSimple();
  // outmsd_->run.spectrumListPtr = outSpecList_;
 
  if (!inmsd_->run.spectrumListPtr.get())
    throw runtime_error("[RespectFilter] Null spectrumListPtr.");

  //spectrumList_->(new shared_ptr<SpectrumListSimple>); 

}

void RespectFilter::write() {
  pwiz::util::IterationListenerRegistry* pILR =  0;
 
  spectrumList = SpectrumListSimplePtr (new SpectrumListSimple);
  
  //SpectrumListSimple* spectrumList = new SpectrumListSimple();
  //spectrumList_->empty();
  
  

  spectrumList->spectra = reportSpectra_;
  
  inmsd_->run.spectrumListPtr = spectrumList;

  //inmsd_->run.spectrumListPtr->spectra = reportSpectra_;

  pwiz::msdata::MSDataFile::write(*inmsd_, out_file_, writeConfig_, pILR);
  
  //  delete spectrumList;
  //for (int i=0; i < reportSpectra_.size(); i++) {
  //  delete reportSpectra_[i];
  //}

  reportSpectra_.clear();
  spectrumList->spectra.clear();

  //  for (int i=0; i < all_peaks_.size(); i++) {
  //  try {
  //   delete all_peaks_[i];
  //  }
  //  catch (...) {
  //    break;
  //  }
  //}

      // }
}


void RespectFilter::init(string& spec, string& pep, int charge, double prob, 
			 double parentMass, cRamp* cramp, long scan, 
			 TPP_HASHMAP_T<char, double>* stat_mods, 
			 TPP_HASHMAP_T<char, double>* stat_prot_termods, 
			 bool is_nterm_pep, bool is_cterm_pep, 
			 double tol, double itol, bool keepChg) {

 
  prob_ = prob;


  parentMass_ =   parentMass;
  
  parentMZ_ = (parentMass+charge*H0MASS)/charge;
  is_nterm_pep_ = is_nterm_pep;
  is_cterm_pep_ = is_cterm_pep;
 
   
 
  string::size_type pos = 0;

  keepChg_ = keepChg;

  if (!pep_str_) {
    pep_str_ = new string(pep);
  }
  else {
    *pep_str_ = pep;
    // delete pep_;
    pep_ = NULL;
  }

  pep_ = new Peptide(*pep_str_, charge);

  if (!spect_str_) {
    spect_str_ = new string(spec);
  }
  else {
    *spect_str_ = spec;
  }

  
  if ((pos = spec.find_first_of('.')) != string::npos) {
    
    *spect_str_ = spec.substr(0,pos)+"_rs"+spec.substr(pos);
  }
  
  pep_unmod_str_ = "";

  pos = 0;

  NAA_ = 0;
  while (pos != string::npos) {
    string token = pep_->nextAAToken(pep, pos, pos);
    pep_unmod_str_ += token.substr(0,1);
    NAA_++;
  }
  pos = 0;
  cramp_ = cramp;
  scan_ = scan;
  
  if (entry_)
    delete entry_;

  entry_ = NULL;

  charge_ = charge;
  etd_ = false;
  unknown_mod_ = false;
  bool unspec_mod = false;
  string unspec_tok = "";
  tolerance_ = tol;
  isoTolerance_ = itol;
 
   //COUNT modsites of all types
  int aa_pos = -1;
  int has_nterm_token = 0;
  int has_cterm_token = 0;

 
  double shf = 0.;
 
  double stat_nterm_mass = 0;
  double stat_cterm_mass = 0;

  int prev_pos = pos;



  
 
 
  while (pos != string::npos) {
    prev_pos = pos;
    string token = pep_->nextAAToken(pep, pos, pos);
    string aa = token.substr(0,1);
    aa_pos++;

    if (token.find('n') != string::npos) {
      has_nterm_token = -1;
    }
    
    if (token.find('c') != string::npos) {
      has_cterm_token = -1;
    }
      
    double stat_tok_mass = pep_->getAATokenMonoisotopicMass(aa);
    if (aa_pos == 0 && has_nterm_token == -1 && is_nterm_pep_ && 
	(*stat_prot_termods).find('n')!= (*stat_prot_termods).end()) {
	stat_tok_mass =  pep_->getAATokenMonoisotopicMass("n")+(*stat_prot_termods)['n'];
	token = "n";
	aa = token;
	pos = prev_pos;

    }
    if (aa_pos == 0 && has_nterm_token == -1 && is_nterm_pep_ && 
	  (*stat_prot_termods).find('n')!= (*stat_prot_termods).end()) {
	stat_tok_mass =  pep_->getAATokenMonoisotopicMass("n")+(*stat_prot_termods)['n'];
	token = "n";
	aa = token;
	pos = prev_pos;

    }
    else if (aa_pos == 0 && 
	(*stat_mods).find('n')!= (*stat_mods).end()) {
	stat_tok_mass =  pep_->getAATokenMonoisotopicMass("n")+(*stat_mods)['n'];
	token = "n";
	aa = token;
	pos = prev_pos;

    }
    else if (pos == string::npos  && has_cterm_token == -1 && is_cterm_pep_ && (*stat_prot_termods).find('c')!= (*stat_prot_termods).end()) {
      stat_tok_mass =  pep_->getAATokenMonoisotopicMass("c")+(*stat_prot_termods)['c'];
      token = "c";
      aa = token;
    }
    else if (pos == string::npos  &&  
	     (*stat_mods).find('c')!= (*stat_mods).end()) {
      stat_tok_mass =  pep_->getAATokenMonoisotopicMass("c")+(*stat_mods)['c'];
      token = "c";
      aa = token;
    }
    else if (token.length() == 1 && (*stat_mods).find(*aa.c_str()) != (*stat_mods).end()) {
      stat_tok_mass += (*stat_mods)[*aa.c_str()];
    } 
    
    if (stat_tok_mass != pep_->getAATokenMonoisotopicMass(aa)) {
      ostringstream omass;
      omass.str("");
      omass.precision(0);
      omass << fixed << stat_tok_mass;
      token = aa + "[" + omass.str() + "]";
    }
    *pep_str_ += token;
    
  }

  delete pep_;
  pep_ = new Peptide(*pep_str_, charge);
  
}

RespectFilter::~RespectFilter() {
    if (entry_) delete entry_;

    if (spect_str_) delete spect_str_;

    entry_ = NULL;
    spect_str_ = NULL;
   

    //if (pep_) delete pep_;

    //pep_ = NULL;

    if (inmsd_) delete inmsd_;
  
    if (pep_str_) delete pep_str_;
    

    if (spectra_hash_)  delete spectra_hash_;
    pep_str_ = NULL;
}

  
bool RespectFilter::set() {
  if (scanInfo_) {
    delete scanInfo_;
  }
    scanInfo_ = NULL;
    rampPeakList* peaks = NULL;
    Peptide* pep = pep_;
  
    scanInfo_ = cramp_->getScanHeaderInfo(scan_);
    //read the peaks
    peaks = cramp_->getPeakList(scan_);	    

    double precursorMz = scanInfo_->m_data.precursorMZ;
    int peakCount = peaks->getPeakCount();  
    int precursorCharge = scanInfo_->m_data.precursorCharge;
    double precursorIntensity = scanInfo_->m_data.precursorIntensity;
    double totIonCurrent = scanInfo_->m_data.totIonCurrent;
    double retentionTime = scanInfo_->m_data.retentionTime;
    string fragType(scanInfo_->m_data.activationMethod);

    if (fragType.find("ETD",0)==0) {
      etd_ = true;
    }
    double collisionEnergy = scanInfo_->m_data.collisionEnergy;

    if (entry_) {
      entry_->makeSemiempiricalSpectrum(pep);
    }
    else {
      entry_ = new SpectraSTLibEntry(pep, "", "Normal", NULL, fragType);
    }
    

    //    if (!(fragType.empty()) && entry_->getFragType().empty()) {
    entry_->setFragType(fragType);
      //}
    
    // will overwrite the retention time from pepXML file with that from the mzXML file  
    stringstream rtss;
    rtss.precision(1);
    rtss << fixed << retentionTime << ',' << retentionTime << ',' << retentionTime; 
    entry_->setOneComment("RetentionTime", rtss.str());
    
    stringstream ticss;
    ticss.precision(2);
    ticss << totIonCurrent;
    entry_->setOneComment("TotalIonCurrent", ticss.str());
    
    stringstream pintss;
    pintss.precision(2);
    pintss << precursorIntensity;
    entry_->setOneComment("PrecursorIntensity", pintss.str());
    
    //MH: Change from >0 to >=0 because sometimes CE is 0.
    if (collisionEnergy >= 0) {
      stringstream cess;
      cess.precision(1);
      cess << fixed << collisionEnergy;
      entry_->setOneComment("CollisionEnergy", cess.str());
    } else {
      stringstream errss;
      errss << "Scan #" << scan_ << " has collision energy below zero: " << collisionEnergy;
    }
    
    
    if (precursorCharge < 1) precursorCharge = 0;
    
    //  cout << "inserting peaks " << peakCount << endl;
    // create the peak list and read the peaks one-by-one
    for (int j = 0; j < peakCount; j++) {
      double mz = peaks->getPeak(j)->mz;
      float intensity = (float)(peaks->getPeak(j)->intensity);
      
      if (intensity > 0.0) {
	entry_->getPeakList()->insert(mz, intensity, "", "");
      }
    }
    
    if (precursorIntensity > 0.0) {
      entry_->getPeakList()->setPrecursorIntensity(precursorIntensity);
    }
    
    stringstream maxss;
    maxss.precision(2);
    maxss << entry_->getPeakList()->getOrigMaxIntensity();
    entry_->setOneComment("OrigMaxIntensity", maxss.str());
    
    //delete scanInfo_;
    delete peaks;
    
    entry_->annotatePeaks(true);
  
    
    return pep_->isGood();
}

void RespectFilter::run() {
  //processPeakList();

  peakList_ = entry_->getPeakList();
  if (isoTolerance_ < 0) {
    isoTolerance_ = 0.1;
  }
  evaluateModPep(pep_, isoTolerance_);
  
}

void RespectFilter::processPeakList() {
  peakList_ = entry_->getPeakList();
  
  //  double retainedFraction = peakList_->clean(false, false, 0.0); // de-isotope, remove near-precursor ions, do not remove light ions

  //  for (int ix=0; ix<peakList->getNumPeaks(); ix++) 
  Peak* pk = new Peak();
  int n =  peakList_->getNumPeaks() ;

  if (n>0) {
	  peakList_->getNthLargestPeak(peakList_->getNumPeaks(), (*pk));

	  minInt_ = (*pk).intensity;

	  peakList_->getNthLargestPeak(1, (*pk));



	  maxInt_ = (*pk).intensity;
	  //peakList_->clean(true,true, 0);
  }

  n =  peakList_->getNumPeaks() ;

  if (n>0) {
	  peakList_->getNthLargestPeak(peakList_->getNumPeaks(), (*pk));

	  minInt_ = (*pk).intensity;

	  peakList_->getNthLargestPeak(1, (*pk));


	  maxInt_ = (*pk).intensity;
  }
  else {
	  minInt_ = -1;
	  maxInt_ = -1;
  }

  TIC_ =  peakList_->getTotalIonCurrent();
  
  delete pk;
}






void RespectFilter::evaluateModPep(Peptide* mpep, double isoTOL) {
  if (!pep_ ||
      !entry_->getPeakList() ||
      !entry_->getPeptidePtr()->isModsSet) {
    return;
  }
  //  SpectraSTPeakList* outputPL = new SpectraSTPeakList(parentMZ_, charge_);
  Peptide* pep = mpep;


  int nDECOY = 0;
  double mzTOL = tolerance_;
  
  if (tolerance_ <= 0) {
    mzTOL = 0.1;
  }
  
  int idx;


  
   // check b ions from left+1 to pos, y ions from len-pos to len-left-1
  
  
  int left = -1;
  int right = -1;
  
  int N = 0;
 

  char ion1 = 'b';
  char ion2 = 'y';
  

  if (etd_) {
    ion1 = 'c';
    ion2 = 'z';
  }

   double siteProbSum = 0;

  left = -1;
  right = -1;

  double wtSum = 1;

  double mz, foundInt, foundMZ;

  Peak* foundPK = NULL;

  map<double, double>* foundMZs = new map<double,double>();

  map<double, double>::iterator itr;

  gsl_vector* r;
  
  double sumRemovedInt = 0;

  for (int i=1; i<pep->NAA() ; i++) {
    for (unsigned int ch = 1; ch <= (unsigned int)pep->charge; ch++) {
      foundPK = NULL;
   
      mz = pep->monoisotopicMZFragment(ion1, i, ch);
      if (mz>0) {
	foundPK = peakList_->findPeakPtr(mz, mzTOL, foundMZs);

	foundInt = 0;
	if (foundPK) {
	  foundInt = foundPK->intensity;
	  foundMZ = foundPK->mz;
	  if ((itr = foundMZs->find(foundMZ)) == foundMZs->end()) {

	    double wt = 1-prob_;
	    foundMZs->insert(make_pair(foundMZ, wt*foundInt));

	  }
	  else {
	    double wt = 1-prob_;
	    if (itr->second <  wt*foundInt) {
	      (*foundMZs)[foundMZ] = wt*foundInt;
	    }
	  }
	  mz = foundMZ;
	  	    //deiso
	  for (int x=1; isoTOL > 0 && x < 4; x++) {
	    foundPK = peakList_->findPeakPtr(mz+x*_ISOMASSDIS_/ch, isoTOL, foundMZs);
	    if (foundPK) {
	      foundInt = foundPK->intensity;
	      foundMZ = foundPK->mz;
	      if ((itr = foundMZs->find(foundMZ)) == foundMZs->end()) {
		
		double wt = 1-prob_;
		foundMZs->insert(make_pair(foundMZ, wt*foundInt));
		
	      }
	      else {
		double wt = 1-prob_;
		if (itr->second <  wt*foundInt) {
		  (*foundMZs)[foundMZ] = wt*foundInt;
		}
	      }
	    }
	  }
	}
      
      }
      foundPK = NULL;
      mz = pep->monoisotopicMZFragment(ion2, pep->NAA() - i, ch);
      if (mz>0) {
	foundPK = peakList_->findPeakPtr(mz, mzTOL, foundMZs);

	foundInt = 0;
	if (foundPK) {
	  foundInt = foundPK->intensity;
	  foundMZ = foundPK->mz;
	  if ((itr = foundMZs->find(foundMZ)) == foundMZs->end()) {
	  
	    double wt = 1-prob_;
	    foundMZs->insert(make_pair(foundMZ, wt*foundInt));
	  }
	  else {
	    double wt = 1-prob_;
	    (*foundMZs)[foundMZ] = wt*foundInt;
	   
	  }
	   mz = foundMZ;
	  	  	    //deiso
	  for (int x=1; isoTOL > 0 && x < 4; x++) {
	    foundPK = peakList_->findPeakPtr(mz+x*_ISOMASSDIS_/ch, isoTOL, foundMZs);
	    if (foundPK) {
	      foundInt = foundPK->intensity;
	      foundMZ = foundPK->mz;
	      if ((itr = foundMZs->find(foundMZ)) == foundMZs->end()) {
		
		double wt = 1-prob_;
		foundMZs->insert(make_pair(foundMZ, wt*foundInt));
		
	      }
	      else {
		double wt = 1-prob_;
		if (itr->second <  wt*foundInt) {
		  (*foundMZs)[foundMZ] = wt*foundInt;
		}
	      }
	    }
	  }
	}
	
      }
      
     
    
    }


  }
  SpectrumListPtr sl;
  SpectrumPtr s;
  //SpectrumInfo info;
  //DDS TODO: Find and UPDATE the correct spectrum in the MSD
  sl = inmsd_->run.spectrumListPtr;
  unsigned long SZ = sl->size();

  TPP_HASHMAP_T<int, SpectrumPtr>::iterator 
    spectitr = spectra_hash_->find(scanInfo_->m_data.scanIndex-1);
  

  //  for (unsigned long j=1; j<SZ; j++) {
  if (spectitr == spectra_hash_->end()) {
    s = sl->spectrum(scanInfo_->m_data.scanIndex-1, true);
  }
  else {
    s = spectitr->second;

  }
  //info.update(*s, true);
      //
      //}
  vector<MZIntensityPair> peaks;

  s->getMZIntensityPairs(peaks);
  peaks.clear();
  //Populate output peakList
  //info.clearBinaryData();
  for (int i=0; i<peakList_->getNumPeaks(); i++) {
    foundPK = new Peak();
    peakList_->getPeak(i, *foundPK);
    foundInt = foundPK->intensity;
    foundMZ = foundPK->mz;
    if ((itr = foundMZs->find(foundMZ)) != foundMZs->end()) {
      foundInt = itr->second ;
      
    }
    delete foundPK;


    
    //outputPL->insert( foundMZ,  foundInt, "", "");
    

    MZIntensityPair* next = new MZIntensityPair(foundMZ, foundInt);
    peaks.push_back(*next);
    delete next;
    //all_peaks_.push_back(next);

  }
  s->setMZIntensityPairs(peaks, s->getIntensityArray()->cvParams.empty() ? CVID_Unknown : s->getIntensityArray()->cvParams.front().units);

  
  if (!keepChg_) {
    //Rewrite presursors to remove charge
    int numPrecursors = s->precursors.size();
    
    for (int i=0; i < numPrecursors; i++) {
      int selctIons = s->precursors[i].selectedIons.size();
      for (int j=0; j < selctIons; j++) { 
	s->precursors[i].selectedIons.
	  insert( s->precursors[i].selectedIons.end(),
		  SelectedIon(atof(s->precursors[i].selectedIons[j].cvParam(MS_selected_ion_m_z).value.c_str()),
			      atof(s->precursors[i].selectedIons[j].cvParam(MS_peak_intensity).value.c_str()),
			      s->precursors[i].selectedIons[j].cvParam(MS_peak_intensity).units));
      }
      s->precursors[i].selectedIons.erase(s->precursors[i].selectedIons.begin(),s->precursors[i].selectedIons.begin()+selctIons);	  
    }
  }

  if (spectitr == spectra_hash_->end()) {
    reportSpectra_.push_back(s);
    s->index = num_spectra_++;
    spectra_hash_->insert(make_pair(scanInfo_->m_data.scanIndex-1, s));
  }

 

    //    delete outputPL;
    delete foundMZs;
 
}
  
