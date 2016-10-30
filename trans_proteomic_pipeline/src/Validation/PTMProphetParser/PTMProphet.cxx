#include "PTMProphet.h"
/* ******************************************************************************

Program       : PTMProphet                                                       
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

PTMProphet::PTMProphet(string& pep, int charge, cRamp* cramp, long scan, string& modaas, double shift) {
  pep_ = new Peptide(pep, charge);
  pep_str_ = new string(pep);
 
  string::size_type pos = 0;
  NAA_ = 0;
  while (pos != string::npos) {
    string token = pep_->nextAAToken(pep, pos, pos);
    NAA_++;
  }
  cramp_ = cramp;
  scan_ = scan;
  entry_ = NULL;
  charge_ = charge;
  etd_ = false;
  shift_ = shift;
  modaas_ = modaas;
};

PTMProphet::~PTMProphet() {
    if (entry_) delete entry_;
    if (pep_str_) delete pep_str_;
    //    if (pep_) {
    //      pep_->deleteTables();
      //delete pep_;
    //}
    entry_ = NULL;
    pep_str_ = NULL;
    //    pep_ = NULL;
    // delete pep_;
    //delete pep_str_;
};

double PTMProphet::getModPrior() {
  if (nomods_.size() == 0 && mods_.size() == 0 ) {
    return 1;
  }
  return (double)mods_.size() / ((double)nomods_.size()+(double)mods_.size());
}


double PTMProphet::getNumMods() {
  return (double)mods_.size();
}

double PTMProphet::getNumModSites() {
  return (double)mods_.size()+(double)nomods_.size();
}
  
void PTMProphet::init() {
    rampScanInfo* scanInfo = NULL;
    rampPeakList* peaks = NULL;
    Peptide* pep = pep_;
    scanInfo = cramp_->getScanHeaderInfo(scan_);
    //read the peaks
    peaks = cramp_->getPeakList(scan_);	    

    double precursorMz = scanInfo->m_data.precursorMZ;
    int peakCount = peaks->getPeakCount();  
    int precursorCharge = scanInfo->m_data.precursorCharge;
    double precursorIntensity = scanInfo->m_data.precursorIntensity;
    double totIonCurrent = scanInfo->m_data.totIonCurrent;
    double retentionTime = scanInfo->m_data.retentionTime;
    string fragType(scanInfo->m_data.activationMethod);

    if (fragType.find("ETD",0)==0) {
      etd_ = true;
      cerr << "Using ETD mode..." << endl;
    }
    double collisionEnergy = scanInfo->m_data.collisionEnergy;

    entry_ = new SpectraSTLibEntry(pep, "", "Normal", NULL, fragType);

    if (!(fragType.empty()) && entry_->getFragType().empty()) {
      entry_->setFragType(fragType);
    }
    
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
    
    delete scanInfo;
    delete peaks;
    
    entry_->annotatePeaks(true);
    
    
}


bool PTMProphet::evaluateModSites(double tolerance) {
  if (!pep_ ||
      !entry_->getPeakList() ||
      !entry_->getPeptidePtr()->isModsSet) {
    return false;
  }


  if (!strcmp(pep_str_->c_str(),"GLCTSPAEHQYFM[147]TEYVAT[181]R") && scan_ == 188) {
    cerr << "DDS: DEBUG" << endl;
    }
  
  // vector<int> nomods; // positions of all non-modsmodrylated S, T, Y
  //vector<int> mods; // positions of all modsmodrylated S, T, Y

  vector<double> siteprob;

  vector<double> sitesum;

  vector<double> site_Evidence;
  vector<double> site_N;
  vector<double> site_decoyEvidence;


  Peptide* pep = pep_;
  SpectraSTPeakList* peakList = entry_->getPeakList();

  int nDECOY = 10;
  double mzTOL = tolerance;
  
  if (tolerance <= 0) {
    mzTOL = 0.1;
  }

  int idx;

  vector<SpectraSTLibEntry*>* decoyEntries = new vector<SpectraSTLibEntry*>;
  vector<SpectraSTPeakList*>* decoyPeakLists = new vector<SpectraSTPeakList*>;

  SpectraSTLibEntry* decoyEntry; 
  SpectraSTPeakList* decoyPeakList; 

  double tmpFor = 0;
  double tmpAgainst = 0;
  double minFor = 0;
  double minAgainst = 0;
  
  for (int i=0; i<nDECOY; i++) {
    decoyEntry = new SpectraSTLibEntry(*entry_);  //To establish NULL distribution
    decoyPeakList = decoyEntry->getPeakList();
    decoyPeakList->shiftAllPeaks(0.0, 10.0);
    
    decoyEntries->push_back(decoyEntry);

    decoyPeakLists->push_back(decoyPeakList);
    
  }



  Peak* pk = new Peak();
  double minInt = 1e-6;
  double maxInt = 1e6;

  //  for (int ix=0; ix<peakList->getNumPeaks(); ix++) 

  peakList->getNthLargestPeak(peakList->getNumPeaks(), (*pk));

  minInt = (*pk).intensity;

  peakList->getNthLargestPeak(1, (*pk));


  maxInt = (*pk).intensity;
  
  int nsites =0;
  int nmods = 0;
  for (unsigned int pos = 0; pos < pep->NAA(); pos++) {
    char aa = pep->stripped[pos];
    if (modaas_.find(aa) != string::npos) {
      nsites++;
      if (pep->mods.find(pos) != pep->mods.end() &&
	  pep->mods.find(pos)!= pep->mods.end() && 
fabs(shift_-Peptide::getModMonoisotopicMass(pep->mods[pos])) < 0.05) {
	nmods++;
      }
      else {
	cout << "WARNING: Unknown mod on amino acid: " << aa << ", with mass shift of: " << Peptide::getModMonoisotopicMass(pep->mods[pos])
	     << ", skipping spectrum ... ";
	return false;
      }
    }
  }

  for (unsigned int pos = 0; pos < pep->NAA(); pos++) {
    char aa = pep->stripped[pos];
    if (modaas_.find(aa) != string::npos) {
      if (pep->mods.find(pos) != pep->mods.end() && pep->mods.find(pos)!= pep->mods.end() && fabs(shift_-Peptide::getModMonoisotopicMass(pep->mods[pos])) < 0.05) {
	mods_.push_back(pos);
	//	siteprob.push_back((double)nmods/(double)nsites);
	siteprob.push_back(0);
      } else {
	nomods_.push_back(pos);
	//	siteprob.push_back((double)nmods/(double)nsites);
	siteprob.push_back(0);
      }
    } 
    else {
      nomodsite_.push_back(pos);
      siteprob.push_back(0);
    }
    sitesum.push_back(0);

    site_Evidence.push_back(0);
    site_decoyEvidence.push_back(0);
    site_N.push_back(0);
  }

  if (mods_.empty() && nomods_.empty()) {
    return false;
  }

  if (mods_.empty()) {
    return false;
  }

  if (nomods_.empty()) {
    for (vector<int>::size_type p = 0; p < mods_.size(); p++) {
      siteprob[mods_[p]] = 1;
    }  
  }
  

  double modsMass = shift_;//Peptide::getModMonoisotopicMass("Phospho");

  // check b ions from left+1 to pos, y ions from len-pos to len-left-1
  
  
  int left = -1;
  int right = -1;
  double expDecoyRatio = 0;
  int N = 0;
  double leastConfidentRatio = maxInt/minInt; 
  double forEvidence = minInt;
  double againstEvidence = minInt;
  double ratio = maxInt/minInt; 
  double altRatio = maxInt/minInt; //for the other position

  double decoyForEvidence = 0;//minInt;
  double decoyAgainstEvidence = 0;//minInt;
  double decoyRatio = maxInt/minInt; 

  char ion1 = 'b';
  char ion2 = 'y';


  if (etd_) {
    ion1 = 'c';
    ion2 = 'z';
  }

  double siteProbSum = 0;
  double Xsq = 0;
  left = -1;
  right = -1;
  for (vector<int>::size_type p = 0; p < mods_.size(); p++) {
    for (vector<int>::size_type s = 0; s < nomods_.size(); s++) {
      leastConfidentRatio =  maxInt/minInt; 

      forEvidence = 0;//minInt;
      againstEvidence = 0;//minInt;
      ratio =  maxInt/minInt; 

      decoyForEvidence = 0;//minInt;
      decoyAgainstEvidence = 0;//minInt;
      decoyRatio =  maxInt/minInt; 

      Xsq = 0;
      //Evaluate all potential positions
      left = -1;
      right = -1;
      if (nomods_[s] < mods_[p]) {
	left = nomods_[s];
      } else if (nomods_[s] > mods_[p]) {
	right = nomods_[s];
      }

    
      if (left != -1) {
	// potential site on the left
	
	// check b ions from left+1 to pos, y ions from len-pos to len-left-1
	forEvidence = 0; //minInt;
	againstEvidence = 0; //minInt;
	
      for (int i = left + 1; i <= mods_[p]; i++) {
	for (unsigned int ch = 1; ch <= (unsigned int)pep->charge; ch++) {
	  forEvidence = minInt; 
	  againstEvidence = minInt;

	  double mz = pep->monoisotopicMZFragment(ion1, i, ch);
	 
	  double foundInt = peakList->findPeak(mz, mzTOL);
	  forEvidence = foundInt;
	
	  mz = pep->monoisotopicMZFragment(ion2, pep->NAA() - i, ch);
	  foundInt = peakList->findPeak(mz, mzTOL);
	  forEvidence += foundInt;
	
	  mz = pep->monoisotopicMZFragment(ion1, i, ch) + modsMass / (double)ch;
	  foundInt = peakList->findPeak(mz, mzTOL);
	  againstEvidence = foundInt;
	
	  mz = pep->monoisotopicMZFragment(ion2, pep->NAA() - i, ch) - modsMass / (double)ch;	     
	  foundInt = peakList->findPeak(mz, mzTOL);
	  againstEvidence += foundInt;


	   tmpFor = 0;
	   tmpAgainst = 0;	  
	   minFor = 0;
	   minAgainst = 0;
	  for (idx = 0; idx < nDECOY; idx++) {
	    //NULL distro using randomized peaklist
	    foundInt = (*decoyPeakLists)[idx]->findPeak(mz, mzTOL);
	    tmpFor = foundInt;
	    //	    decoyForEvidence += foundInt;
	    
	    mz = pep->monoisotopicMZFragment(ion2, pep->NAA() - i, ch);
	    foundInt = (*decoyPeakLists)[idx]->findPeak(mz, mzTOL);
	    //decoyForEvidence += foundInt;
	    tmpFor += foundInt;
	    
	    mz = pep->monoisotopicMZFragment(ion1, i, ch) + modsMass / (double)ch;
	    foundInt = (*decoyPeakLists)[idx]->findPeak(mz, mzTOL);
	    //decoyAgainstEvidence += foundInt;
	    tmpAgainst = foundInt;

	    mz = pep->monoisotopicMZFragment(ion2, pep->NAA() - i, ch) - modsMass / (double)ch;	     
	    foundInt = (*decoyPeakLists)[idx]->findPeak(mz, mzTOL);
	    //decoyAgainstEvidence += foundInt;
	    tmpAgainst += foundInt;
// 	    if (idx == 0) {
// 	      minFor = tmpFor;
// 	      minAgainst = tmpAgainst;
// 	    }
// 	    else {
	      
// 	      if (tmpFor < minFor) {
// 		minFor = tmpFor;
// 	      }

// 	      if (tmpAgainst < minAgainst){
// 		minAgainst = tmpAgainst;
// 	      }
//	  }
	    minFor += tmpFor;
	    minAgainst += tmpAgainst;
	    
	  }
	  decoyForEvidence = minFor / nDECOY;
	  decoyAgainstEvidence = minAgainst / nDECOY;
	  if (decoyForEvidence < minInt) {
	    decoyForEvidence = minInt;
	  }
	  if (forEvidence < minInt) {
	    forEvidence = minInt;
	  }
	  if (decoyAgainstEvidence < minInt) {
	    decoyAgainstEvidence = minInt;
	  }
	  if (againstEvidence < minInt) {
	    againstEvidence = minInt;
	  }
	  if (decoyAgainstEvidence < minInt) {
	    decoyAgainstEvidence = minInt;
	  }
	  if (againstEvidence < minInt) {
	    againstEvidence = minInt;
	  }
	  
	  double Xsq = (fabs(forEvidence - decoyForEvidence ) *  fabs(forEvidence - decoyForEvidence )) / decoyForEvidence;
	  double Xsq2 = (fabs(againstEvidence - decoyAgainstEvidence ) *  fabs(againstEvidence - decoyAgainstEvidence )) / decoyAgainstEvidence;	   
	  
	  if (forEvidence < decoyForEvidence) {
	    Xsq = 0;
	  }

	  if (againstEvidence < decoyAgainstEvidence) {
	    Xsq2 = 0;
	  }

	  site_Evidence[mods_[p]] += Xsq;//forEvidence;
	  site_decoyEvidence[mods_[p]] += decoyForEvidence;
	  
	  site_Evidence[nomods_[s]] += Xsq2;//againstEvidence;
	  site_decoyEvidence[nomods_[s]] += decoyAgainstEvidence;
	  site_N[mods_[p]]++;
	  site_N[nomods_[s]]++;
	 

	}
      }
      

    }
    
    if (right != -1) {
      // potential site on the right
      
      // check b ions from left+1 to pos, y ions from len-pos to len-left-1
      
      for (int i = mods_[p] + 1; i <= right; i++) {
	for (unsigned int ch = 1; ch <= (unsigned int)pep->charge; ch++) {
	  forEvidence = minInt; 
	  againstEvidence = minInt;
	  double mz = pep->monoisotopicMZFragment(ion1, i, ch);
	  double foundInt = peakList->findPeak(mz, mzTOL);
	  forEvidence = foundInt;

	  mz = pep->monoisotopicMZFragment(ion2, pep->NAA() - i, ch);
	  foundInt = peakList->findPeak(mz, mzTOL);
	  forEvidence += foundInt;

	  mz = pep->monoisotopicMZFragment(ion1, i, ch) - modsMass / (double)ch;
	  foundInt = peakList->findPeak(mz, mzTOL);
	  againstEvidence = foundInt;

	  mz = pep->monoisotopicMZFragment(ion2, pep->NAA() - i, ch) + modsMass / (double)ch;	     
	  foundInt = peakList->findPeak(mz, mzTOL);
	  againstEvidence += foundInt;
	   tmpFor = 0;
	   tmpAgainst = 0;
	   minFor = 0;
	   minAgainst = 0;
	  for (idx = 0; idx < nDECOY; idx++) {
	     //NULL distro using randomized peaklist
	    foundInt = (*decoyPeakLists)[idx]->findPeak(mz, mzTOL);
	    tmpFor = foundInt;
	    //	    decoyForEvidence += foundInt;
	    
	    mz = pep->monoisotopicMZFragment(ion2, pep->NAA() - i, ch);
	    foundInt = (*decoyPeakLists)[idx]->findPeak(mz, mzTOL);
	    //decoyForEvidence += foundInt;
	    tmpFor += foundInt;
	    
	    mz = pep->monoisotopicMZFragment(ion1, i, ch) - modsMass / (double)ch;
	    foundInt = (*decoyPeakLists)[idx]->findPeak(mz, mzTOL);
	    //decoyAgainstEvidence += foundInt;
	    tmpAgainst = foundInt;

	    mz = pep->monoisotopicMZFragment(ion2, pep->NAA() - i, ch) + modsMass / (double)ch;	     
	    foundInt = (*decoyPeakLists)[idx]->findPeak(mz, mzTOL);
	    //decoyAgainstEvidence += foundInt;
	    tmpAgainst += foundInt;
// 	    if (idx == 0) {
// 	      minFor = tmpFor;
// 	      minAgainst = tmpAgainst;
// 	    }
// 	    else {
	      
// 	      if (tmpFor < minFor) {
// 		minFor = tmpFor;
// 	      }

// 	      if (tmpAgainst < minAgainst){
// 		minAgainst = tmpAgainst;
// 	      }
	      
// 	    }

	    minFor += tmpFor;
	    minAgainst += tmpAgainst;
     

	  }

	  decoyForEvidence = minFor / nDECOY;
	  decoyAgainstEvidence = minAgainst / nDECOY;
	  
	  if (decoyForEvidence < minInt) {
	    decoyForEvidence = minInt;
	  }
	  if (forEvidence < minInt) {
	    forEvidence = minInt;
	  }
	  if (decoyAgainstEvidence < minInt) {
	    decoyAgainstEvidence = minInt;
	  }
	  if (againstEvidence < minInt) {
	    againstEvidence = minInt;
	  }

	  //Set both decoys to the lower estimate
	  if ( decoyForEvidence > decoyAgainstEvidence) {
	    decoyForEvidence = decoyAgainstEvidence;
	  }
	  if ( decoyForEvidence < decoyAgainstEvidence) {
	    decoyAgainstEvidence = decoyForEvidence;
	  }

	  double Xsq = (fabs(forEvidence - decoyForEvidence ) *  fabs(forEvidence - decoyForEvidence )) / decoyForEvidence;
	  double Xsq2 = (fabs(againstEvidence - decoyAgainstEvidence ) *  fabs(againstEvidence - decoyAgainstEvidence )) / decoyAgainstEvidence;	   

	  if (forEvidence < decoyForEvidence) {
	    Xsq = 0;
	  }

	  if (againstEvidence < decoyAgainstEvidence) {
	    Xsq2 = 0;
	  }

	  site_Evidence[mods_[p]] += Xsq;//forEvidence;
	  site_decoyEvidence[mods_[p]] += decoyForEvidence;
	  
	  site_Evidence[nomods_[s]] += Xsq2;//againstEvidence;
	  site_decoyEvidence[nomods_[s]] += decoyAgainstEvidence;
	  site_N[mods_[p]]++;
	  site_N[nomods_[s]]++;
	  
	}
      }
    
    }
    
  

  //   ratio = forEvidence / decoyAgainstEvidence;
//     altRatio = againstEvidence / decoyAgainstEvidence;
//     decoyRatio = decoyForEvidence / decoyAgainstEvidence;

//     if (ratio < leastConfidentRatio) leastConfidentRatio = ratio;
//     if (altRatio < leastConfidentRatio) leastConfidentRatio = ratio;
      

//    double Xsq = (forEvidence - decoyForEvidence) *  (forEvidence - decoyForEvidence) / decoyForEvidence;
//    double Xsq2 = (againstEvidence - decoyAgainstEvidence) *  (againstEvidence - decoyAgainstEvidence) / decoyAgainstEvidence;

//     double Q = gsl_cdf_chisq_Q(Xsq, 1);

//     double Q2 = gsl_cdf_chisq_Q(Xsq2, 1);

//     if (Q > siteprob[mods_[p]]) {
//       siteProbSum = siteProbSum - siteprob[mods_[p]] + Q;
//       siteprob[mods_[p]] = Q;
      
//     }
    
//     if (Q2 > siteprob[nomods_[s]]) {
//       siteProbSum = siteProbSum - siteprob[nomods_[s]] + Q2;
//       siteprob[nomods_[s]] = Q2;
      
//     }

    }    
  }

  siteProbSum = 0;

  for (vector<int>::size_type p = 0; p < mods_.size(); p++) {
    // if (site_Evidence[mods_[p]] < minInt) {
    //  site_Evidence[mods_[p]] = minInt;
    //}
    //if (site_decoyEvidence[mods_[p]] < minInt) {
    //  site_decoyEvidence[mods_[p]] = minInt;
    //}
    //    double Xsq = (site_Evidence[mods_[p]] - site_decoyEvidence[mods_[p]]) *  ( site_Evidence[mods_[p]] - site_decoyEvidence[mods_[p]]) / site_decoyEvidence[mods_[p]];

    //    double Q = gsl_cdf_chisq_P(site_Evidence[mods_[p]], site_N[mods_[p]]);
    double Q = site_Evidence[mods_[p]] / site_N[mods_[p]];


    //if (Xsq <= 0.000001) {
    //  Q = 0.000001;
    //}

    siteprob[mods_[p]] = Q;
    
    siteProbSum += Q;

  }
  

  for (vector<int>::size_type p = 0; p < nomods_.size(); p++) {
    //if (site_Evidence[nomods_[p]] < minInt) {
    //  site_Evidence[nomods_[p]] = minInt;
    //}
    //if (site_decoyEvidence[nomods_[p]] < minInt) {
    //  site_decoyEvidence[nomods_[p]] = minInt;
    //}
    //double Xsq = (site_Evidence[nomods_[p]] - site_decoyEvidence[nomods_[p]]) *  ( site_Evidence[nomods_[p]] - site_decoyEvidence[nomods_[p]]) / site_decoyEvidence[nomods_[p]];
   
    //double Q = gsl_cdf_chisq_P(site_Evidence[nomods_[p]], site_N[nomods_[p]]);

    double Q = site_Evidence[nomods_[p]] / site_N[nomods_[p]];

    //    if (Xsq <= 0.000001) {
    //  Q = 0.000001;
    // }
    
    //if (Xsq >= 11) {
    //  Q = 1 - 0.000001;
    //}
    
    siteprob[nomods_[p]] = Q;
    
    siteProbSum += Q;

  }
  
  



  for (vector<int>::size_type p = 0; p < mods_.size(); p++) {
    
    //siteprob[mods_[p]] = siteprob[mods_[p]] / sitesum[mods_[p]] ;
    siteprob[mods_[p]] = nmods * siteprob[mods_[p]]/siteProbSum;
    if (siteprob[mods_[p]] > 1) {
      siteprob[mods_[p]] = 1;
    }
    if (isnan(siteprob[mods_[p]])) {
      siteprob[mods_[p]] = 0;
    }
  }
  
  for (vector<int>::size_type p = 0; p < nomods_.size(); p++) {
    //siteprob[nomods_[p]] = siteprob[nomods_[p]] / sitesum[nomods_[p]] ;
    siteprob[nomods_[p]] = nmods * siteprob[nomods_[p]]/siteProbSum;
    if (siteprob[nomods_[p]] > 1) {
      siteprob[nomods_[p]] = 1;
    }
    if (isnan(siteprob[nomods_[p]])) {
      siteprob[nomods_[p]] = 0;
    }
  }
  
  
  pep_prob_str_ = "";
  stringstream ss;
  ss.precision(3);
  ss.setf(ios::fixed); 
  for (unsigned int pos = 0; pos < pep->NAA(); pos++) {
    char aa = pep->stripped[pos];
    ss << aa;
    if (modaas_.find(aa) != string::npos) {
      
      ss << "(";
      ss.width(5); 
      ss  << siteprob[pos] ;
      ss.width(0); 
      ss << ")";
      pos_prob_hash_[pos] = siteprob[pos];
    } 
    else {
      // nomodsite_.push_back(pos);
      //siteprob.push_back(0);
    }

  }

  pep_prob_str_ = ss.str() ;
    
  //cout << endl;


  delete pk;
  
  for (int i=0; i<nDECOY; i++) {
    delete (*decoyEntries)[i];
    //    delete (*decoyPeakLists)[i];
  }

  delete decoyEntries;
  delete decoyPeakLists;
  return true;

}
    




// void PTMProphet::evaluateModsmodSites(int nsites) {
//   if (!pep_ || 
//       !entry_->getPeakList() ||
//       !entry_->getPeptidePtr()->isModsSet) {
//     return;
//   }
  
//   // vector<int> nomods; // positions of all non-modsmodrylated S, T, Y
//   //vector<int> mods; // positions of all modsmodrylated S, T, Y

//   vector<double> siteprob;
//   vector<double> siteevid;

//   Peptide* pep = pep_;
//   SpectraSTPeakList* peakList = entry_->getPeakList();

//   Peak* pk = new Peak();
//   double minInt = 1e-6;
//   double maxInt = 1e6;

//   //  for (int ix=0; ix<peakList->getNumPeaks(); ix++) 

//   peakList->getNthLargestPeak(peakList->getNumPeaks(), (*pk));

//   minInt = (*pk).intensity;

//   peakList->getNthLargestPeak(1, (*pk));


//   maxInt = (*pk).intensity;
  

//   for (unsigned int pos = 0; pos < pep->NAA(); pos++) {
//     char aa = pep->stripped[pos];
//     if (aa == 'S' || aa == 'T' || aa == ion2) {
//       if (pep->mods.find(pos) != pep->mods.end()) { // && pep->mods[pos] == "Modsmod") {
// 	mods_.push_back(pos);
// 	siteprob.push_back(1/nsites);
//       }
//       //else {
//       //	nomods_.push_back(pos);
//       //	siteprob.push_back(1/nsites);
//       //}
	
//     } 
//     else {
//       nomodsite_.push_back(pos);
//       siteprob.push_back(0);
//     }

//     siteevid.push_back(0);

//   }

//   if (mods_.empty()) {// || nomods_.empty()) {
//     //  return;
//   }

//   double modsMass = Peptide::getModMonoisotopicMass("Modsmod");

//   // check b ions from left+1 to pos, y ions from len-pos to len-left-1
  
  
//   int left = -1;
//   int right = -1;
//   double expDecoyRatio = 0;
//   int N = 0;
//   for (vector<int>::size_type p = 0; p < mods_.size(); p++) {
//     for (vector<int>::size_type s = 0; s < nomodsite_.size(); s++) {
//       double leastConfidentRatio = maxInt/minInt; 
//       double forEvidence = minInt;
//       double againstEvidence = minInt;
//       double ratio = maxInt/minInt; 
//       //Evaluate all potential positions
//       left = -1;
//       right = -1;
//       if (nomodsite_[s] < mods_[p]) {
// 	left = nomodsite_[s];
//       } else if (nomodsite_[s] > mods_[p]) {
// 	right = nomodsite_[s];
//       }

    
//       if (left != -1) {
// 	// potential site on the left
	
// 	// check b ions from left+1 to pos, y ions from len-pos to len-left-1
// 	forEvidence = minInt;
// 	againstEvidence = minInt;
      
//       for (int i = left + 1; i <= mods_[p]; i++) {
// 	for (unsigned int ch = 1; ch <= (unsigned int)pep->charge; ch++) {
// 	  double mz = pep->monoisotopicMZFragment(ion1, i, ch);
// 	  double foundInt = peakList->findPeak(mz, 0.5);
// 	  forEvidence += foundInt;
// 	  // cerr << "    LEFT(+): " << ion1 << i << "^" << ch << " = " << foundInt << endl;
// 	  mz = pep->monoisotopicMZFragment(ion2, pep->NAA() - i, ch);
// 	  foundInt = peakList->findPeak(mz, 0.5);
// 	  forEvidence += foundInt;
// 	  // cerr << "    LEFT(+): " << ion2 << pep->NAA() - i << "^" << ch << " = " << foundInt << endl;
// 	  mz = pep->monoisotopicMZFragment(ion1, i, ch) + modsMass / (double)ch;
// 	  foundInt = peakList->findPeak(mz, 0.5);
// 	  againstEvidence += foundInt;
// 	  // cerr << "    LEFT(-): " << ion1 << i << "+80^" << ch << " = " << foundInt << endl;
// 	  mz = pep->monoisotopicMZFragment(ion2, pep->NAA() - i, ch) - modsMass / (double)ch;	     
// 	  foundInt = peakList->findPeak(mz, 0.5);
// 	  againstEvidence += foundInt;
// 	  // cerr << "    LEFT(-): " << ion2 << pep->NAA() - i << "-80^" << ch << " = " << foundInt << endl;

// 	}
// 	ratio = forEvidence / againstEvidence;
	

// 	if (ratio < leastConfidentRatio) leastConfidentRatio = ratio; 
//       }

      
//       if (right != -1) {
// 	// potential site on the right
	
// 	// check b ions from left+1 to pos, y ions from len-pos to len-left-1
// 	forEvidence = minInt; 
// 	againstEvidence = minInt;
	
// 	for (int i = mods_[p] + 1; i <= right; i++) {
// 	  for (unsigned int ch = 1; ch <= (unsigned int)pep->charge; ch++) {
// 	    double mz = pep->monoisotopicMZFragment(ion1, i, ch);
// 	    double foundInt = peakList->findPeak(mz, 0.5);
// 	    forEvidence += foundInt;
// 	    // cerr << "    RIGHT(+): " << ion1 << i << "^" << ch << " = " << foundInt << endl;
// 	    mz = pep->monoisotopicMZFragment(ion2, pep->NAA() - i, ch);
// 	    foundInt = peakList->findPeak(mz, 0.5);
// 	    forEvidence += foundInt;
// 	    // cerr << "    RIGHT(+): " << 'y' << pep->NAA() - i << "^" << ch << " = " << foundInt << endl;
// 	    mz = pep->monoisotopicMZFragment(ion1, i, ch) - modsMass / (double)ch;
// 	    foundInt = peakList->findPeak(mz, 0.5);
// 	    againstEvidence += foundInt;
// 	    // cerr << "    RIGHT(-): " << ion1 << i << "-80^" << ch << " = " << foundInt << endl;
// 	    mz = pep->monoisotopicMZFragment('y', pep->NAA() - i, ch) + modsMass / (double)ch;	     
// 	    foundInt = peakList->findPeak(mz, 0.5);
// 	    againstEvidence += foundInt;


// 	}
//       }
  
      
//     }

//     expDecoyRatio += ratio; N++;

//     ratio = forEvidence / againstEvidence;

//     if (ratio < leastConfidentRatio) leastConfidentRatio = ratio; 
//       }
      
//     }
//   }
 

//   for (vector<int>::size_type p = 0; p < mods_.size(); p++) {
//     siteprob[mods_[p]] = 1 / siteprob[mods_[p]];
//   }
  
  
//   pep_prob_str_ = "";
//   stringstream ss;
//   ss.precision(3);
//   ss.setf(ios::fixed); 
//   for (unsigned int pos = 0; pos < pep->NAA(); pos++) {
//     char aa = pep->stripped[pos];
//     ss << aa;
//     if (aa == 'S' || aa == 'T' || aa == 'Y') {
      
//       ss << "(";
//       ss.width(5); 
//       ss  << siteprob[pos] ;
//       ss.width(0); 
//       ss << ")";
//       pos_prob_hash_[pos] = siteprob[pos];
//     } 
//     else {
//       // nomodsite_.push_back(pos);
//       //siteprob.push_back(0);
//     }

//   }

//   pep_prob_str_ = ss.str() ;
    
//   //cout << endl;


//   delete pk;

// }
