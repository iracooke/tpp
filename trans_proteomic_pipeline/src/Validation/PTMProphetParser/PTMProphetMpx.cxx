#include "PTMProphetMpx.h"
/* ******************************************************************************

Program       : PTMProphetMpx                                                       
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

// int nTermMod(vector<int>& ntermod) {  
//   for (int t=0; t< ntermod.size(); t++) {
//     if (ntermod[t]) 
//       return 1;
//   }
//   return 0; 
// }

PTMProphetMpx::PTMProphetMpx(string& pep, int charge, double calc_neut_mass, 
			     cRamp* cramp, long scan, 
			     vector<string>& modaas, vector<double>& shift, 
			     TPP_HASHMAP_T<char, double>* stat_mods, TPP_HASHMAP_T<char, double>* stat_prot_termods, bool is_nterm_pep, bool is_cterm_pep) {

  stat_mods_ = stat_mods;

  stat_prot_termods_ = stat_prot_termods;

  is_nterm_pep_ = is_nterm_pep;
  is_cterm_pep_ = is_cterm_pep;
  
  calc_neut_mass_ = calc_neut_mass;
  shift_ = shift;
  
  bool nterm = false,  cterm = false;
  for (int j = 0; j < shift_.size(); j++) {
    nmods_.push_back(0);
    modsite_.push_back(new vector<int>());
    mods_.push_back(new vector<int>());
    nomods_.push_back(new vector<int>());
    label_.push_back( new strp_hash());

     if (modaas[j].find('c') != string::npos) {
      ctermod_.push_back(1);
      cterm = true;
    }
    else {
      ctermod_.push_back(0);
    }

    if (modaas[j].find('n') != string::npos) {
      ntermod_.push_back(1);
      nterm = true;
    }
    else {
      ntermod_.push_back(0);
    }
  }

  if (pep.substr(0,1) != "n" && nterm) {
    pep = string("n") + pep;
  } 

  pep_ = new Peptide(pep, charge);

  pep_str_ = new string(pep);
  pep_unmod_str_ = "";
  string::size_type pos = 0;
  
  NAA_ = 1;
  while (pos != string::npos) {
    string token = pep_->nextAAToken(pep, pos, pos);
    
    pep_unmod_str_ += token.substr(0,1);
    NAA_++;
  }
  

  cramp_ = cramp;
  scan_ = scan;
  entry_ = NULL;
  charge_ = charge;
  etd_ = false;
  unknown_mod_ = false;
  bool unspec_mod = false;
  string unspec_tok = "";
  tolerance_ = 0.1;
 
  modaas_ = modaas;
  //COUNT modsites of all types
  int aa_pos = -1;
  int has_nterm_token = 0;
  int has_cterm_token = 0;



  while (1) {

    pos = 0;
    double shf = 0.;
    aa_pos = -1;
    double stat_nterm_mass = 0;
    double stat_cterm_mass = 0;
    //    int prev_pos = pos;
    //   if (*pep_str_ == "KRLGT[247]AWCS") {

    //	cerr << "DDS DEBUG" <<endl;
    //  }
    while (pos != string::npos) {
      int prev_pos = pos;
      string token = pep_->nextAAToken(pep, pos, pos);
      
      //if (unspec_mod) {
      //	break;
      //}

      aa_pos++;
      if (token.find('n') != string::npos) {
	has_nterm_token = -1;
      }
      
      if (token.find('c') != string::npos) {
	has_cterm_token = -1;
      }
      
      unspec_mod = true;
      double stat_tok_mass = pep_->getAATokenMonoisotopicMass(token.substr(0,1));
      
      if (aa_pos == 0 && has_nterm_token == -1 && token.substr(0,1)!= "n" &&  is_nterm_pep_ && (*stat_prot_termods_).find('n')!= (*stat_prot_termods_).end()) {
	stat_tok_mass =  pep_->getAATokenMonoisotopicMass("n")+(*stat_prot_termods_)['n'];
	token = "n";
	pos = prev_pos;
	
      }
      else if (aa_pos == 0 && token.substr(0,1)!= "n" && (*stat_mods_).find('n')!= (*stat_mods_).end()) {
	stat_tok_mass =  pep_->getAATokenMonoisotopicMass("n")+(*stat_mods_)['n'];
	token = "n";
	pos = prev_pos;
	
      }
      else if (pos == string::npos  && has_cterm_token == -1 && is_cterm_pep_ && (*stat_prot_termods_).find('c')!= (*stat_prot_termods_).end()) {
	stat_tok_mass =  pep_->getAATokenMonoisotopicMass("c")+(*stat_prot_termods_)['c'];
	token = "c";
      }
      else if (pos == string::npos   && (*stat_mods_).find('c')!= (*stat_mods_).end()) {
	stat_tok_mass =  pep_->getAATokenMonoisotopicMass("c")+(*stat_mods_)['c'];
	token = "c";
      }
      
      else if ((*stat_mods_).find(*token.substr(0,1).c_str()) != (*stat_mods_).end()) {
	stat_tok_mass += (*stat_mods_)[*token.substr(0,1).c_str()];
      }
      
  
      for (int j = 0; j < shift_.size(); j++) {
	if (modaas_[j].find(token.substr(0,1)) != string::npos) {
	  double tp = pep_->getAATokenMonoisotopicMass(token);
	  
	  
	  
	  if (
	      (aa_pos && token.find('c') != string::npos
	       && fabs(shift_[j]+stat_tok_mass-pep_->getAATokenMonoisotopicMass(token)) < 0.005) || 
	      (!aa_pos && has_nterm_token == -1 
	       && fabs(shift_[j]+stat_tok_mass-pep_->getAATokenMonoisotopicMass(token)) < 0.005) || 
	      (pep_->mods.find(aa_pos+has_nterm_token) != pep_->mods.end() 
	       && fabs(shift_[j]+stat_tok_mass-pep_->getAATokenMonoisotopicMass(token)) < 0.005 ) 
	      )
	    {
	      (*label_[j]).insert(make_pair(token.substr(0,1).c_str(), new string(token.substr(1,token.find_last_of("]")))));
	      
	      nmods_[j]++;	  
	      (*modsite_[j]).push_back(aa_pos);
	      unspec_mod = false;
	      unknown_mod_ = false;	  
	      unspec_tok = "";
	      shf = 0.;
	    }
	  else if (pep_->mods.find(aa_pos+has_nterm_token) != pep_->mods.end()) { 
	    (*modsite_[j]).push_back(aa_pos);
	    if (token.length() > 1) {
	      //    nmods_[j]++;	  //multiple variable mods detected
	    }
	    unspec_mod = unspec_mod && true;

	    
	  }
	  else if (pep_->mods.find(aa_pos+has_nterm_token) == pep_->mods.end()) { 
	    (*modsite_[j]).push_back(aa_pos);
	    if (token.length() > 1) {
	      nmods_[j]++;	 
	    }
	    unspec_mod = false;
	    
	  }
	}
	else if (token.length() > 1) {

	  unspec_mod = unspec_mod && true;


	}
	else if (token.length() == 1) {
	  unspec_mod = false;
	  unspec_tok = "";
	  shf = 0;
	}


      }
     
      if (unspec_mod) {
	unspec_tok = token;
	shf = fabs(stat_tok_mass-pep_->getAATokenMonoisotopicMass(token));
      }
 
    }
    
    if (unspec_mod) {
      cerr << "\tWARNING: Unrecognized mod on peptide: " << pep << endl;
      unknown_mod_ = true;
      //nmods_.clear();
      //modsite_.clear();
      //mods_.clear();
      //nomods_.clear();
      //label_.clear();
      //ntermod_.clear();
      //ctermod_.clear();
      //nmods_.push_back(0);
      //modsite_.push_back(new vector<int>());
      //mods_.push_back(new vector<int>());
      //nomods_.push_back(new vector<int>());
      //label_.push_back( new strp_hash());
      string aa = "";
      aa = unspec_tok.substr(0, 1);
      modaas.push_back(aa);
      shift.push_back(shf);
      break;
      // modaas_.push_back(aa);
      // shift_.push_back(shf);
    }
    else {
      break;
    }
  }
  shift = shift_;
  for (int j = 0; j < shift_.size(); j++) {
    siteprob_.push_back(new vector<double>());
    sitesum_.push_back(new vector<double>());


    site_Pval_.push_back(new vector<double>());

    site_ObsModEvidence_.push_back(new vector<double>());
    site_ExpModEvidence_.push_back(new vector<double>());
    site_ObsUnModEvidence_.push_back(new vector<double>());
    site_ExpUnModEvidence_.push_back(new vector<double>());

    site_MaxEvidence_.push_back(new vector<double>());
    site_MinEvidence_.push_back(new vector<double>());

    site_decoyMaxEvidence_.push_back(new vector<double>());
    site_decoyMinEvidence_.push_back(new vector<double>());

    site_decoyEvidence_.push_back(new vector<double>());

    site_nomodMaxEvidence_.push_back(new vector<double>());

    site_nomodAllEvidence_.push_back(new vector<vector<double>*>());
    site_AllEvidence_.push_back(new vector<vector<double>*>());

    site_nomodMinEvidence_.push_back(new vector<double>());

    site_nomodDecoyMaxEvidence_.push_back(new vector<double>());
    site_nomodDecoyMinEvidence_.push_back(new vector<double>());


    site_N_.push_back(new vector<int>());
    for (unsigned int pos = 0; pos < NAA_; pos++) {
      (*siteprob_[j]).push_back(0);
      
      (*sitesum_[j]).push_back(0);
      
      (*site_Pval_[j]).push_back(0);

      (*site_ObsModEvidence_[j]).push_back(-1);
      (*site_ObsUnModEvidence_[j]).push_back(-1);
      (*site_ExpModEvidence_[j]).push_back(-1);
      (*site_ExpUnModEvidence_[j]).push_back(-1);

      (*site_MaxEvidence_[j]).push_back(-1);
      (*site_MinEvidence_[j]).push_back(-1);
      (*site_decoyMaxEvidence_[j]).push_back(-1);
      (*site_decoyMinEvidence_[j]).push_back(-1);
      (*site_decoyEvidence_[j]).push_back(-1);
      (*site_nomodAllEvidence_[j]).push_back(new vector<double>());
      (*site_AllEvidence_[j]).push_back(new vector<double>());
      (*site_nomodMaxEvidence_[j]).push_back(-1);
      (*site_nomodMinEvidence_[j]).push_back(-1);
      (*site_nomodDecoyMaxEvidence_[j]).push_back(-1);
      (*site_nomodDecoyMinEvidence_[j]).push_back(-1);
      (*site_N_[j]).push_back(0);
    }    
  }

}

PTMProphetMpx::~PTMProphetMpx() {
    if (entry_) delete entry_;
    if (pep_str_) delete pep_str_;
    entry_ = NULL;
    pep_str_ = NULL;

    for (int j = 0; j < shift_.size(); j++) {
      modsite_.pop_back();
      mods_.pop_back();
      nomods_.pop_back();
      (*label_[j]).clear();
      label_.pop_back();

      siteprob_.pop_back();
      sitesum_.pop_back();
      site_MaxEvidence_.pop_back();
      site_MinEvidence_.pop_back();

      site_ObsModEvidence_.pop_back();
      site_ObsUnModEvidence_.pop_back();
      site_ExpModEvidence_.pop_back();
      site_ExpUnModEvidence_.pop_back();

      site_Pval_.pop_back();
      
      site_decoyMaxEvidence_.pop_back();
      site_decoyMinEvidence_.pop_back();
      
      site_decoyEvidence_.pop_back();
      
      site_nomodMaxEvidence_.pop_back();

      site_nomodMinEvidence_.pop_back();
      
      site_nomodDecoyMaxEvidence_.pop_back();
      site_nomodDecoyMinEvidence_.pop_back();
      

      for (unsigned int pos = 0; pos < NAA_; pos++) {
	(*site_nomodAllEvidence_[j]).pop_back();
	(*site_AllEvidence_[j]).pop_back();
      }
      site_nomodAllEvidence_.pop_back();
      site_AllEvidence_.pop_back();
      
    


    }
    

}

void PTMProphetMpx::insertMod(string& modaas, double shift) {
  shift_.push_back(shift);
  modaas_.push_back(modaas);
}

double PTMProphetMpx::getModPrior(int type) {

  if ((*modsite_[type]).size() == 0) {
    return 1;
  }
  return (double)nmods_[type] / (double)(*modsite_[type]).size();
}


double PTMProphetMpx::getNumMods(int type) {
  return (double)nmods_[type];
}

double PTMProphetMpx::getNumModSites(int type) {
  return (double)(*modsite_[type]).size();
}
  
bool PTMProphetMpx::init() {
  if (unknown_mod_) {
    return false;
  }
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
    generateDecoys(0);
    
    
    return pep_->isGood();
}



void PTMProphetMpx::processPeakList() {
  peakList_ = entry_->getPeakList();
  
  double retainedFraction = peakList_->clean(true, true, 0.0); // de-isotope, remove near-precursor ions, do not remove light ions

  //  for (int ix=0; ix<peakList->getNumPeaks(); ix++) 
  Peak* pk = new Peak();
  int n =  peakList_->getNumPeaks() ;

  if (n>0) {
	  peakList_->getNthLargestPeak(peakList_->getNumPeaks(), (*pk));

	  minInt_ = (*pk).intensity;

	  peakList_->getNthLargestPeak(1, (*pk));



	  maxInt_ = (*pk).intensity;
	  peakList_->clean(true,true, 0);
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

}

vector<vector<int>*>* PTMProphetMpx::combinations() {
  vector<gsl_combination*> comboVec; gsl_combination* c =  NULL;
  size_t i,j,k,l;
  bool all_done = false;

  vector<gsl_combination*> compareVec; 
  
  gsl_combination* mod_combo = NULL;

  //DDS TODO: Need to disallow multiple mods types on same amino acid when an amino acid can occur in multiple mods

  mod_combo = gsl_combination_calloc(shift_.size(), shift_.size());

  processPeakList();

  for (unsigned int type = 0; type < shift_.size(); type++) {
    if (nmods_[type] && nmods_[type] <= (*modsite_[type]).size()) {
      c = gsl_combination_calloc((*modsite_[type]).size(), nmods_[type]);
    }
    else {
      c = NULL;
    }
    comboVec.push_back(c);

  }
  combineMods(comboVec); 
  ostringstream omass;
  //istringstream imass;
  TPP_HASHMAP_T<int, double> pos_mass_hash;
  TPP_HASHMAP_T<int, string*> pos_label_hash;
  bool next = false;
  Peptide* mpep = NULL;;
  string test_mpep = "";

  bool nterm_mod = false;
  bool cterm_mod = false;

  int nterm = pep_unmod_str_.substr(0,1)=="n" ? 1 : 0;
  int adj = nterm ? 0 : 1;

  for (i = 0; i < combs_of_mods_.size(); i++) {
    // RESET peptide
    next = false;
    if (mpep) delete mpep;
    mpep = NULL;
    test_mpep = "";
    pos_mass_hash.clear();
    pos_label_hash.clear();


    for (j = 0; j < shift_.size(); j++) {
      (*mods_[j]).clear();     
      (*nomods_[j]).clear();      
      int start = 0;

    	//DEAL HERE WITH AVOIDING MULTIPLE MODS ON one position
      for (k = 0; k < (*combs_of_mods_[i])[j]->size(); k++) {
      	if (pos_mass_hash.find((*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]]) != pos_mass_hash.end()) {
	  next = true;
	  break;
	}
	(*mods_[j]).push_back((*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]]);

	
	//pos_label_hash[(*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]]] = 
	//  (*label_[j])[pep_unmod_str_.substr((*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]], 1).c_str()];
	pos_label_hash[(*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]]] = new string();

	if ((*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]] < 0) {
	  pos_mass_hash[(*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]]] = 
	    pep_->getAATokenMonoisotopicMass("n") + shift_[j] ;

	  if ((*stat_mods_).find('n')!= (*stat_mods_).end()) {
	    pos_mass_hash[(*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]]] += (*stat_mods_)['n'];
	  }
	}
	else if ((*stat_mods_).find(*pep_unmod_str_.substr((*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]],1).c_str()) != (*stat_mods_).end()) {//else if ((*stat_mods_).find(*pep_unmod_str_.substr((*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]]-adj,1).c_str()) != (*stat_mods_).end()) {
	  pos_mass_hash[(*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]]] = 
	    pep_->getAATokenMonoisotopicMass(pep_unmod_str_.substr((*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]], 1)) + shift_[j] ;//pep_->getAATokenMonoisotopicMass(pep_unmod_str_.substr((*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]]-adj, 1)) + shift_[j] ;
	  pos_mass_hash[(*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]]] += 
	    (*stat_mods_)[*pep_unmod_str_.substr((*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]],1).c_str()];	    //(*stat_mods_)[*pep_unmod_str_.substr((*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]]-adj,1).c_str()];
	}
	else {
	   pos_mass_hash[(*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]]] = 
	     pep_->getAATokenMonoisotopicMass(pep_unmod_str_.substr((*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]], 1)) + shift_[j] ;	     //	    pep_->getAATokenMonoisotopicMass(pep_unmod_str_.substr((*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]]-adj, 1)) + shift_[j] ;
	}

      omass.str("");
	omass.precision(0);
	omass << fixed << pos_mass_hash[(*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]]];
	*pos_label_hash[(*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]]] = "[" + omass.str() + "]";
      }

      k=0;
      for (l = 0;  l < (*modsite_[j]).size(); l++) {
	if (k < (*mods_[j]).size() && (*modsite_[j])[l]==(*mods_[j])[k]) {
	  k++;
	}
	else{
	  (*nomods_[j]).push_back((*modsite_[j])[l]);
	}
      }
      
      if (next) break;
    
    }
    unsigned int mpep_pos = 0;
    bool have_smod = false;
    double stat_mod = 0;



    // Deal with N-terms here
    unsigned int pos =0;
    if (nterm && (pos_mass_hash.find(0) != pos_mass_hash.end() || (*stat_mods_).find('n') != (*stat_mods_).end())) {
      stat_mod = 0;
      
      if ((*stat_mods_).find('n') != (*stat_mods_).end())
	stat_mod = (*stat_mods_)['n'];
	
      omass.str("");

					
      double mass = stat_mod;
      
      if (pos_mass_hash.find(0) != pos_mass_hash.end()) {
	mass = pos_mass_hash[0];
      }
      else {
	mass += pep_->getAATokenMonoisotopicMass("n");
      }

      setPrecision(omass, mass);
      omass <<  "[" << mass << "]" ;
      test_mpep += "n";
      test_mpep += omass.str().c_str();
    }
    
    //Deal with a.a's here
    for (pos = nterm; pos <  pep_unmod_str_.length(); pos++) {
      stat_mod = 0;
      have_smod = false;

      if ((*stat_mods_).find(*pep_unmod_str_.substr(pos,1).c_str()) != (*stat_mods_).end()) {
	stat_mod = (*stat_mods_)[*pep_unmod_str_.substr(pos,1).c_str()];
	have_smod = true;
      }
   
      else if (pos == pep_unmod_str_.length()-1 && (*stat_mods_).find('c') != (*stat_mods_).end()) {
	stat_mod = (*stat_mods_)['c'];
	omass.str("");
	omass.precision(0);
	double mass = mpep->getAATokenMonoisotopicMass("c")+stat_mod;
	setPrecision(omass, mass);
	omass << "["<< mass << "]";
	test_mpep += "c";
	test_mpep += omass.str().c_str();
      }
   
      if (pos_mass_hash.find(pos) != pos_mass_hash.end()) {      //      if (pos_mass_hash.find(pos+adj) != pos_mass_hash.end()) {
	omass.str("");
	omass.precision(3);
	omass << *(pos_label_hash[pos]) ;//omass << *(pos_label_hash[pos+adj]) ;
	test_mpep += pep_unmod_str_.substr(pos,1);
	test_mpep += omass.str().c_str();
	delete pos_label_hash[pos]; //	delete pos_label_hash[pos+adj];
      }
//       else if ( pep_unmod_str_.substr(pos,1) == pep_str_->substr(mpep_pos,1) ) { // && pep_str_->substr(mpep_pos+1,1) == "[" ) { 
// 	bool unmod = false;
// 	//TODO: DDS could be faster
// 	for (int t = 0; t < shift_.size(); t++) {	
// 	  for (int s = 0; s <  (*modsite_[t]).size(); s++) {
// 	    if ((*modsite_[t])[s] == pos) {
// 	      unmod = true;
// 	      break;
// 	    }
// 	  }
// 	  if (unmod) break;
// 	}
// 	if (!unmod) {
// 	  test_mpep += pep_str_->substr(mpep_pos, pep_str_->find("]", mpep_pos)-mpep_pos+1).c_str();
// 	}
// 	else {
// 	  test_mpep += pep_unmod_str_.substr(pos,1);
// 	}
//       }
      else if (have_smod) {
	omass.str("");

	double mass = mpep->getAATokenMonoisotopicMass(pep_unmod_str_.substr(pos,1))+stat_mod;
	setPrecision(omass, mass);
	omass << "["<< mass << "]";
	test_mpep += pep_unmod_str_.substr(pos,1);
	test_mpep += omass.str().c_str();
	//delete pos_label_hash[pos];	
      }
      else {
	test_mpep += pep_unmod_str_.substr(pos,1);
      }
      
      if (pep_str_->substr(mpep_pos+1,1) == "[") {
	mpep_pos = pep_str_->find("]",mpep_pos)+1;
      }
      else {
	mpep_pos++;
      }

    }
    mpep = new Peptide(test_mpep, charge_);
    if (!next && fabs(calc_neut_mass_-mpep->monoisotopicNeutralM()) < 0.001)
      evaluateModPep(mpep);  
    else 
      cerr << "WARNING: Illegal peptide with unknown mod: " << test_mpep << endl;
  }
  evaluateModPositions();
  computePosnProbs();

  // DCT - no return, what should it return?
  // return null to shut up clang

  return NULL;

}


// vector<vector<int>*>* PTMProphetMpx::combinations() {
//   vector<gsl_combination*> comboVec; gsl_combination* c =  NULL;
//   size_t i,j,k,l;
//   bool all_done = false;

//   vector<gsl_combination*> compareVec; 
  
//   gsl_combination* mod_combo = NULL;

//   //DDS TODO: Need to disallow multiple mods types on same amino acid when an amino acid can occur in multiple mods

//   mod_combo = gsl_combination_calloc(shift_.size(), shift_.size());

//   processPeakList();

//   for (unsigned int type = 0; type < shift_.size(); type++) {
//     if (nmods_[type] && nmods_[type] <= (*modsite_[type]).size()) {
//       c = gsl_combination_calloc((*modsite_[type]).size(), nmods_[type]);
//     }
//     else {
//       c = NULL;
//     }
//     comboVec.push_back(c);

//   }
//   combineMods(comboVec); 
//   ostringstream omass;
//   //istringstream imass;
//   TPP_HASHMAP_T<int, double> pos_mass_hash;
//   TPP_HASHMAP_T<int, string*> pos_label_hash;
//   bool next = false;
//   Peptide* mpep = NULL;;
//   string test_mpep = "";
//   for (i = 0; i < combs_of_mods_.size(); i++) {
//     // RESET peptide
//     next = false;
//     if (mpep) delete mpep;
//     mpep = NULL;
//     test_mpep = "";
//     pos_mass_hash.clear();
//     pos_label_hash.clear();


//     for (j = 0; j < shift_.size(); j++) {
//       (*mods_[j]).clear();     
//       (*nomods_[j]).clear();      
//       int start = 0;

    
//       for (k = 0; k < (*combs_of_mods_[i])[j]->size(); k++) {	//DEAL HERE WITH AVOIDING MULTIPLE MODS ON one position
//       	if (pos_mass_hash.find((*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]]) != pos_mass_hash.end()) {
// 	  next = true;
// 	  break;
// 	}
// 	(*mods_[j]).push_back((*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]]);

	
// 	if ( (*stat_mods_).find(pep_unmod_str_.substr((*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]], 1).c_str()[0]) != (*stat_mods_).end() ) {
// 	  pos_mass_hash[(*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]]] =    (*stat_mods_)[pep_unmod_str_.substr((*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]], 1).c_str()[0]] +
// 	    pep_->getAATokenMonoisotopicMass(pep_unmod_str_.substr((*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]], 1)) + shift_[j] ;
// 	}
// 	else {
// 	  pos_mass_hash[(*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]]] =  
// 	    pep_->getAATokenMonoisotopicMass(pep_unmod_str_.substr((*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]], 1)) + shift_[j] ;

// 	}


// 	pos_label_hash[(*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]]] = new string();

	
// 	omass.str("");
// 	omass.precision(0);
// 	omass << fixed << pos_mass_hash[(*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]]];
// 	*pos_label_hash[(*modsite_[j])[(*(*combs_of_mods_[i])[j])[k]]] = "[" + omass.str() + "]";
//       }

//       k=0;
//       for (l = 0;  l < (*modsite_[j]).size(); l++) {
// 	if (k < (*mods_[j]).size() && (*modsite_[j])[l]==(*mods_[j])[k]) {
// 	  k++;
// 	}
// 	else{
// 	  (*nomods_[j]).push_back((*modsite_[j])[l]);
// 	}
//       }
      
//       if (next) break;
    
//     }
//     unsigned int mpep_pos = 0;
//     bool have_smod = false;
//     double stat_mod = 0;
    
//     //int nterm = pep_unmod_str_.substr(0,1)=="n" ? 0 : 1;
   

//     for (unsigned int pos = 0; pos <  pep_unmod_str_.length(); pos++) {
//       stat_mod = 0;
//       have_smod = false;
      
//       //if (pos)
//       //	pos -= nterm;

//       double mass = 0;
//       if (!pos && (*stat_mods_).find('n') != (*stat_mods_).end()) {
//  	stat_mod = (*stat_mods_)['n'];
//  	omass.str("");
//  	omass.precision(0);
//  	mass = mpep->getAATokenMonoisotopicMass("n")+stat_mod;
//  	omass << "["<< mass << "]";
//  	test_mpep += "n";
//  	test_mpep += omass.str().c_str();
//       }
//       else if (pos == pep_unmod_str_.length()-1 && (*stat_mods_).find('c') != (*stat_mods_).end()) {
//  	stat_mod = (*stat_mods_)['c'];
//  	omass.str("");
//  	omass.precision(0);
//  	mass = mpep->getAATokenMonoisotopicMass("c")+stat_mod;
// 	omass << "["<< mass << "]";
//  	test_mpep += "c";
//  	test_mpep += omass.str().c_str();
//       }
//       else if ((*stat_mods_).find(*pep_unmod_str_.substr(pos,1).c_str()) != (*stat_mods_).end()) {
// 	stat_mod = (*stat_mods_)[*pep_unmod_str_.substr(pos,1).c_str()];
// 	have_smod = true;
//       }
      
//       if (pos_mass_hash.find(pos+n) != pos_mass_hash.end()) {
// 	omass.str("");
// 	omass.precision(3);
// 	omass << *(pos_label_hash[pos+nterm]) ;
// 	test_mpep += pep_unmod_str_.substr(pos,1);
// 	test_mpep += omass.str().c_str();
// 	delete pos_label_hash[pos+nterm];
//       }
//       else if ( pep_unmod_str_.substr(pos,1) == pep_str_->substr(mpep_pos,1)  ) { //&& pep_str_->substr(mpep_pos+1,1) == "[" ) { 
// 	bool unmod = false;
// 	//TODO: DDS could be faster
// 	for (int t = 0; t < shift_.size(); t++) {	
// 	  for (int s = 0; s <  (*modsite_[t]).size(); s++) {
// 	    if ((*modsite_[t])[s] == pos) {
// 	      unmod = true;
// 	      break;
// 	    }
// 	  }
// 	  if (unmod) break;
// 	}
// 	if (!unmod) {
// 	  test_mpep += pep_str_->substr(mpep_pos, pep_str_->find("]", mpep_pos)-mpep_pos+1).c_str();
// 	}
// 	else {
// 	  test_mpep += pep_unmod_str_.substr(pos,1);
// 	}
//       }
//       else if (!pos && (*stat_mods_).find('n') != (*stat_mods_).end()) {
// 	stat_mod = (*stat_mods_)['n'];
// 	omass.str("");
// 	omass.precision(0);
// 	mass = mpep->getAATokenMonoisotopicMass("n")+stat_mod;
// 	omass << "["<< mass << "]";
// 	test_mpep += "n";
// 	test_mpep += omass.str().c_str();
//       }
//       else if (pos == pep_unmod_str_.length()-1 && (*stat_mods_).find('c') != (*stat_mods_).end()) {
// 	stat_mod = (*stat_mods_)['c'];
// 	omass.str("");
// 	omass.precision(0);
// 	mass = mpep->getAATokenMonoisotopicMass("c")+stat_mod;
// 	omass << "["<< mass << "]";
// 	test_mpep += "c";
// 	test_mpep += omass.str().c_str();
//       }
//       else if (have_smod) {
// 	omass.str("");
// 	omass.precision(0);
// 	double mass = mpep->getAATokenMonoisotopicMass(pep_unmod_str_.substr(pos,1))+stat_mod;
// 	omass << "["<< mass << "]";
// 	test_mpep += pep_unmod_str_.substr(pos,1);
// 	test_mpep += omass.str().c_str();
// 	//delete pos_label_hash[pos];	
//       }
//       else {
// 	test_mpep += pep_unmod_str_.substr(pos,1);
//       }
      
//       if (pep_str_->substr(mpep_pos+1,1) == "[") {
// 	mpep_pos = pep_str_->find("]",mpep_pos)+1;
//       }
//       else {
// 	mpep_pos++;
//       }

//     }
//     mpep = new Peptide(test_mpep, charge_);
//     if (!next && fabs(calc_neut_mass_-mpep->monoisotopicNeutralM()) < 0.001)
//       evaluateModPep(mpep);  
//   }
//   evaluateModPositions();
//   computePosnProbs();

//   // DCT - no return, what should it return?
//   // return null to shut up clang

//   return NULL;

// }




bool PTMProphetMpx::nextCombo(vector<gsl_combination*>& compare, int type) {
  if (!compare[type]) return false;
  return gsl_combination_next(compare[type]) == GSL_SUCCESS;
}

void  PTMProphetMpx::rewindCombo(vector<gsl_combination*>& compare, int type) {
  if (!compare[type]) return;
  while(gsl_combination_prev(compare[type]) == GSL_SUCCESS);
}

void PTMProphetMpx::combineMods(vector<gsl_combination*>& compare) {
  //vector<int> i_s;
  //for (int i = 0 ; i < compare.size(); i++) {
    //i_s.push_back(0);
  //} 

  bool done = false;
  while (!done) {
    combs_of_mods_.push_back(getModCombo(compare));   
    for (int i = compare.size()-1 ; i >= 0; i--) {
      //go through them all and find the one to update
      if (nextCombo(compare,i)) {
        //found one to update
	break;
      }
      else {
	//end of road
	if (i) {
	  rewindCombo(compare,i);
	}
	else {
	  done = true;
	  break;
	}
	
      }
    }

  }
}

vector<vector<int>*>* PTMProphetMpx::getModCombo(vector<gsl_combination*>& compare) {
  vector<vector<int>*>* rtn = new vector<vector<int>*>();
  gsl_combination* c =  NULL;
  
  for (int i = 0 ; i< compare.size(); i++) {
    rtn->push_back(new vector<int>());
    if (compare[i]) {
      for (int j = 0; j < gsl_combination_k(compare[i]); j++) {
	(*rtn)[i]->push_back(gsl_combination_get(compare[i], j));       
      }
    }
  }
  return rtn;
     
}


void PTMProphetMpx::generateDecoys(int nDECOY) {


 decoyEntries_ = new vector<SpectraSTLibEntry*>;
 decoyPeakLists_ = new vector<SpectraSTPeakList*>;

  SpectraSTLibEntry* decoyEntry; 
  SpectraSTPeakList* decoyPeakList; 

  nDECOY_ = nDECOY;
  for (int i=0; i<nDECOY; i++) {
    decoyEntry = new SpectraSTLibEntry(*entry_);  //To establish NULL distribution
    decoyPeakList = decoyEntry->getPeakList();
    decoyPeakList->shiftAllPeaks(0.0, 10.0);
    
    decoyEntries_->push_back(decoyEntry);

    decoyPeakLists_->push_back(decoyPeakList);
    
  }


}

void  PTMProphetMpx::deleteDecoys() { 

  for (int i=0; i<nDECOY_; i++) {
    delete (*decoyEntries_)[i];
    //    delete (*decoyPeakLists)[i];
  }

  delete decoyEntries_;
  delete decoyPeakLists_;
  
}

void PTMProphetMpx::evaluateModPep(Peptide* mpep) {
  if (!pep_ ||
      !entry_->getPeakList() ||
      !entry_->getPeptidePtr()->isModsSet) {
    return;
  }

  //  if (!strcmp(pep_str_->c_str(),"GLCTSPAEHQYFM[147]TEYVAT[181]R") && scan_ == 188) {
  //  cerr << "DDS: DEBUG" << endl;
  // }

//   vector<vector<double>> siteprob;

//   vector<vector<double>> sitesum;

//   vector<vector<double>> site_MaxEvidence;
//   vector<vector<double>> site_N;
//   vector<vector<double>> site_decoyEvidence;


  Peptide* pep = mpep;


  int nDECOY = 0;
  double mzTOL = tolerance_;
  
  if (tolerance_ <= 0) {
    mzTOL = 0.1;
  }

  int idx;


  double tmpFor = 0;
  double tmpAgainst = 0;
  double minFor = 0;
  double minAgainst = 0;



 
  int nsites =0;
  int nmods = 0;
  int ntypes = 0;

   // check b ions from left+1 to pos, y ions from len-pos to len-left-1
  
  
  int left = -1;
  int right = -1;
  double expDecoyRatio = 0;
  int N = 0;
  double leastConfidentRatio = maxInt_/minInt_; 
  double forEvidence = minInt_;
  double againstEvidence = minInt_;
  double ratio = maxInt_/minInt_; 
  double altRatio = maxInt_/minInt_; //for the other position

  double decoyForEvidence = 0;//minInt_;
  double decoyAgainstEvidence = 0;//minInt_;
  double decoyRatio = maxInt_/minInt_; 

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
  
  for (int type = 0; type < shift_.size(); type++) {
    if ((*modsite_[type]).empty()) continue;

    if (nmods_[type] == (*modsite_[type]).size()) {
      for (vector<int>::size_type p = 0; p < (*modsite_[type]).size(); p++) {
	(*siteprob_[type])[(*modsite_[type])[p]] = 1;
      } 
    }

  }

  forEvidence = 0; 
  decoyForEvidence = 0;
  
  double wtSum = 1;

  double mz, foundInt, foundMZ;

  Peak* foundPK = NULL;

  map<double, double>* foundMZs = new map<double,double>();

  map<double, double>::iterator itr;

  gsl_vector* r;
  

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
	  //if (1) {
	    double wt = 1;// - sqrt(fabs(foundMZ-mz)/mzTOL);
	    forEvidence += wt*foundInt;
	    //wtSum += wt;
	    foundMZs->insert(make_pair(foundMZ, wt*foundInt));
	  }
	  else {
	    double wt = 1;// - sqrt(fabs(foundMZ-mz)/mzTOL);
	    if (itr->second <  wt*foundInt) {
	      forEvidence += wt*foundInt - itr->second;
	      (*foundMZs)[foundMZ] = wt*foundInt;
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
	    //if (1) {
	    double wt = 1;// - sqrt(fabs(foundMZ-mz)/mzTOL);
	    forEvidence += wt*foundInt;
	    //wtSum += wt;
	     foundMZs->insert(make_pair(foundMZ, wt*foundInt));
	  }
	  else {
	    double wt = 1;// - sqrt(fabs(foundMZ-mz)/mzTOL);
	    if (itr->second <  wt*foundInt) {
	      forEvidence += wt*foundInt - itr->second;
	      (*foundMZs)[foundMZ] = wt*foundInt;
	    }
	  }
	}

      }
     
      minFor = 0;
      //      r = gsl_vector_calloc(nDECOY);
      for (idx = 0; idx < nDECOY; idx++) {
	//NULL distro using randomized peaklist
	tmpFor = 0;

	mz = pep->monoisotopicMZFragment(ion1, i, ch);
	if (mz > 0) {
	  foundInt = (*decoyPeakLists_)[idx]->findPeak(mz, mzTOL);
	  tmpFor += foundInt;
	}
	
	mz = pep->monoisotopicMZFragment(ion2, pep->NAA() - i, ch);
	if (mz > 0) {
	  foundInt = (*decoyPeakLists_)[idx]->findPeak(mz, mzTOL);
	  //decoyForEvidence += foundInt;
	  tmpFor += foundInt;
	}

	
	gsl_vector_set(r, idx, tmpFor);
	minFor += tmpFor;
	
      }


      //      gsl_sort_vector(r);

      //decoyForEvidence += (minFor-gsl_vector_get(r, nDECOY-1)) / (nDECOY-1);

      //      decoyForEvidence  += gsl_stats_median_from_sorted_data(gsl_vector_ptr(r, 0), 1, nDECOY);
 
      //      gsl_vector_free(r);

      
      
    }
  }
  delete foundMZs;
  
  forEvidence /= wtSum;
  
  if (decoyForEvidence < minInt_) {
    decoyForEvidence = minInt_;
  }

  if (forEvidence < minInt_) {
    forEvidence = minInt_;
  }

  
  for (int type = 0; type < shift_.size(); type++) {
    for (vector<int>::size_type p = 0; p < (*mods_[type]).size(); p++) {
      //double Xsq = (fabs(forEvidence - decoyForEvidence ) *  fabs(forEvidence - decoyForEvidence )) / decoyForEvidence;
      //if (forEvidence < decoyForEvidence) {
      //	Xsq = 0;
      //}
      
      if ((*site_MaxEvidence_[type])[(*mods_[type])[p]] <= 0 
	  || forEvidence > (*site_MaxEvidence_[type])[(*mods_[type])[p]]) {
	(*site_MaxEvidence_[type])[(*mods_[type])[p]] = forEvidence; // / TIC_;//Xsq;
      }
      
      if ((*site_MinEvidence_[type])[(*mods_[type])[p]] <= 0 || 
	  forEvidence < (*site_MinEvidence_[type])[(*mods_[type])[p]]) {
	(*site_MinEvidence_[type])[(*mods_[type])[p]] = forEvidence; // / TIC_;//Xsq;
      }
     

      if ((*site_decoyMaxEvidence_[type])[(*mods_[type])[p]] <= 0 
	  || decoyForEvidence > (*site_decoyMaxEvidence_[type])[(*mods_[type])[p]]) {
	(*site_decoyMaxEvidence_[type])[(*mods_[type])[p]] = decoyForEvidence; // / TIC_;//Xsq;
      }
      
      if ((*site_decoyMinEvidence_[type])[(*mods_[type])[p]] <= 0 || 
	  decoyForEvidence < (*site_decoyMinEvidence_[type])[(*mods_[type])[p]]) {
	(*site_decoyMinEvidence_[type])[(*mods_[type])[p]] = decoyForEvidence; // / TIC_;//Xsq;
      }


      //      (*site_MaxEvidence_[type])[(*mods_[type])[p]] += forEvidence; // / TIC_;//Xsq;
      //(*site_decoyEvidence_[type])[(*mods_[type])[p]] += decoyForEvidence / TIC_;
      //(*site_N_[type])[(*mods_[type])[p]]++;
      (*(*site_AllEvidence_[type])[(*mods_[type])[p]]).push_back(forEvidence);
    }

    for (vector<int>::size_type p = 0; p < (*nomods_[type]).size(); p++) {
      if ((*site_nomodMaxEvidence_[type])[(*nomods_[type])[p]] <= 0 || 
	  forEvidence > (*site_nomodMaxEvidence_[type])[(*nomods_[type])[p]]) {
	(*site_nomodMaxEvidence_[type])[(*nomods_[type])[p]] = forEvidence; // / TIC_;//Xsq;
      }
      if ((*site_nomodMinEvidence_[type])[(*nomods_[type])[p]] <= 0 || 
	  forEvidence < (*site_nomodMinEvidence_[type])[(*nomods_[type])[p]]) {
	(*site_nomodMinEvidence_[type])[(*nomods_[type])[p]] = forEvidence; // / TIC_;//Xsq;
      }

      if ((*site_nomodDecoyMaxEvidence_[type])[(*nomods_[type])[p]] <= 0 || 
	  decoyForEvidence > (*site_nomodDecoyMaxEvidence_[type])[(*nomods_[type])[p]]) {
	(*site_nomodDecoyMaxEvidence_[type])[(*nomods_[type])[p]] = decoyForEvidence; // / TIC_;//Xsq;
      }
      if ((*site_nomodDecoyMinEvidence_[type])[(*nomods_[type])[p]] <= 0 || 
	  decoyForEvidence < (*site_nomodDecoyMinEvidence_[type])[(*nomods_[type])[p]]) {
	(*site_nomodDecoyMinEvidence_[type])[(*nomods_[type])[p]] = decoyForEvidence; // / TIC_;//Xsq;
      }

      (*site_N_[type])[(*nomods_[type])[p]]++;
      
      (*(*site_nomodAllEvidence_[type])[(*nomods_[type])[p]]).push_back(forEvidence);
    }
    
  }
}
  
 
void PTMProphetMpx::evaluateModPositions() {


 for (int type = 0; type < shift_.size(); type++) {

   evaluateModPositions(type);

 }
}

void PTMProphetMpx::evaluateModPositions(int type) {

  double varNoModEvid;

  double varModEvid;

  double sdNoModEvid;
  double meanNoModEvid;

  double sdModEvid;
  double meanModEvid;

  double maxModEvid;


  double Xsq =0;
  double Pval =0;
  double Qval =0;

  double modXsq =0;  
  double unmodXsq =0;  
  
  double modEvidSum=0;
  double unmodEvidSum=0;

  vector<double>* posEvidSum = new vector<double>();
  
  double minMod = minInt_;
  double minUnmod = minInt_;
  for (vector<int>::size_type p = 0; p < (*modsite_[type]).size(); p++) {
    if (p==0 || minMod > (*site_MaxEvidence_[type])[(*modsite_[type])[p]] ) {
      minMod = (*site_MaxEvidence_[type])[(*modsite_[type])[p]];
    }
    if (p==0 || minUnmod > (*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]] ) {
      minUnmod = (*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]];
    }

  }

  // if (nmods_[type] < 2 || (*modsite_[type]).size() < 2) { 
  double oneInt = maxInt_ / 100;
    //For Chi Squared Test
    for (vector<int>::size_type p = 0; p < (*modsite_[type]).size(); p++) {
      (*site_ObsModEvidence_[type])[(*modsite_[type])[p]] = (*site_MaxEvidence_[type])[(*modsite_[type])[p]]  / minInt_; // /TIC_;
      //(*site_ObsModEvidence_[type])[(*modsite_[type])[p]] = ((*site_MaxEvidence_[type])[(*modsite_[type])[p]]-minMod+minInt_)  / minInt_;
      
      
      modEvidSum += (*site_ObsModEvidence_[type])[(*modsite_[type])[p]];
      posEvidSum->push_back((*site_ObsModEvidence_[type])[(*modsite_[type])[p]]);
    }
    
    for (vector<int>::size_type p = 0; p < (*modsite_[type]).size(); p++) {
      (*site_ObsUnModEvidence_[type])[(*modsite_[type])[p]] = (*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]]  / minInt_; //TIC_;
      //(*site_ObsUnModEvidence_[type])[(*modsite_[type])[p]] = ((*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]]-minUnmod+minInt_)  / minInt_; 
      unmodEvidSum += (*site_ObsUnModEvidence_[type])[(*modsite_[type])[p]];
      
      (*posEvidSum)[p] += (*site_ObsUnModEvidence_[type])[(*modsite_[type])[p]];
    }
    //  }
  // else {
//    //For Chi Squared Test
//     for (vector<int>::size_type p = 0; p < (*modsite_[type]).size(); p++) {
//       //(*site_ObsModEvidence_[type])[(*modsite_[type])[p]] = (*site_MaxEvidence_[type])[(*modsite_[type])[p]]  / minInt_;
//       (*site_ObsModEvidence_[type])[(*modsite_[type])[p]] = ((*site_MaxEvidence_[type])[(*modsite_[type])[p]]-minMod+minInt_)  / minInt_;
      
      
//       modEvidSum += (*site_ObsModEvidence_[type])[(*modsite_[type])[p]];
//       posEvidSum->push_back((*site_ObsModEvidence_[type])[(*modsite_[type])[p]]);
//     }
    
//     for (vector<int>::size_type p = 0; p < (*modsite_[type]).size(); p++) {
//       //(*site_ObsUnModEvidence_[type])[(*modsite_[type])[p]] = (*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]]  / minInt_; 
//       (*site_ObsUnModEvidence_[type])[(*modsite_[type])[p]] = ((*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]]-minUnmod+minInt_)  / minInt_; 
//       unmodEvidSum += (*site_ObsUnModEvidence_[type])[(*modsite_[type])[p]];
      
//       (*posEvidSum)[p] += (*site_ObsUnModEvidence_[type])[(*modsite_[type])[p]];
//     }

//   }


  for (vector<int>::size_type p = 0; p < (*modsite_[type]).size(); p++) {
    (*site_ExpModEvidence_[type])[(*modsite_[type])[p]]  = ((*posEvidSum)[p]*modEvidSum) / (modEvidSum + unmodEvidSum);
    (*site_ExpUnModEvidence_[type])[(*modsite_[type])[p]]  = ((*posEvidSum)[p]*unmodEvidSum) / (modEvidSum + unmodEvidSum);
  }

  //Compute Chi-squared for each position // Works // Moved below as a case
//   for (vector<int>::size_type p = 0; p < (*modsite_[type]).size(); p++) {  
//     double Em = (*site_ExpModEvidence_[type])[(*modsite_[type])[p]];
//     double Om = (*site_ObsModEvidence_[type])[(*modsite_[type])[p]];
    
//     double Eu = (*site_ExpUnModEvidence_[type])[(*modsite_[type])[p]];
//     double Ou = (*site_ObsUnModEvidence_[type])[(*modsite_[type])[p]];

//     Xsq = pow(Em-Om,2)/Em + pow(Eu-Ou,2)/Eu;

//     if ((*site_ObsModEvidence_[type])[(*modsite_[type])[p]] > (*site_ObsUnModEvidence_[type])[(*modsite_[type])[p]]) {
//       Pval = gsl_cdf_chisq_P(Xsq, 1);
//     }
//     else {
//       Pval = gsl_cdf_chisq_Q(Xsq, 1);
//     }

//     (*site_Pval_[type])[(*modsite_[type])[p]] = Pval;

//     (*site_MaxEvidence_[type])[(*modsite_[type])[p]] = Pval;
//   }  

//   return;

  for (vector<int>::size_type p = 0; p < (*modsite_[type]).size(); p++) {
    //  gsl_vector* r = NULL;
       Xsq = 0;
      

       // if ((*(*site_nomodAllEvidence_[type])[(*modsite_[type])[p]]).size()) {
       // r = gsl_vector_calloc((*(*site_nomodAllEvidence_[type])[(*modsite_[type])[p]]).size());
       //}
       //       for (vector<int>::size_type n = 0; n < (*(*site_nomodAllEvidence_[type])[(*modsite_[type])[p]]).size(); n++) {
       //gsl_vector_set(r, n, (*(*site_nomodAllEvidence_[type])[(*modsite_[type])[p]])[n] );
	 //Xsq += ((*(*site_nomodAllEvidence_[type])[(*modsite_[type])[p]])[n]- (*site_MaxEvidence_[type])[(*modsite_[type])[p]]) * 
	 //  ((*(*site_nomodAllEvidence_[type])[(*modsite_[type])[p]])[n]- (*site_MaxEvidence_[type])[(*modsite_[type])[p]]) /  (*site_MaxEvidence_[type])[(*modsite_[type])[p]]; 
       //}
       //       Pval = gsl_cdf_chisq_P(Xsq, (*(*site_nomodAllEvidence_[type])[(*modsite_[type])[p]]).size()-1);
       // Pval = gsl_cdf_chisq_P(Xsq, (*(*site_nomodAllEvidence_[type])[(*modsite_[type])[p]]).size()-1);
       //Qval = 1 - Pval;
       //if (r) {
       //	 varNoModEvid = gsl_stats_variance(gsl_vector_ptr(r, 0), 1, (*nomods_[type]).size()); 
       // meanNoModEvid = gsl_stats_mean(gsl_vector_ptr(r, 0), 1, (*nomods_[type]).size());
       // sdNoModEvid = gsl_stats_sd(gsl_vector_ptr(r, 0), 1, (*nomods_[type]).size());
       // gsl_vector_free(r);
       // r = NULL;
       //}


       //if ((*(*site_AllEvidence_[type])[(*modsite_[type])[p]]).size()) {
       // r = gsl_vector_calloc((*(*site_AllEvidence_[type])[(*modsite_[type])[p]]).size());
       //}
       //for (vector<int>::size_type n = 0; n < (*(*site_AllEvidence_[type])[(*modsite_[type])[p]]).size(); n++) {
       // gsl_vector_set(r, n, (*(*site_AllEvidence_[type])[(*modsite_[type])[p]])[n] );
	 // Xsq += ((*(*site_AllEvidence_[type])[(*modsite_[type])[p]])[n]- (*site_MaxEvidence_[type])[(*modsite_[type])[p]]) * 
	 //  ((*(*site_AllEvidence_[type])[(*modsite_[type])[p]])[n]- (*site_MaxEvidence_[type])[(*modsite_[type])[p]]) /  (*site_MaxEvidence_[type])[(*modsite_[type])[p]]; 
       //}
       //       Pval = gsl_cdf_chisq_P(Xsq, (*(*site_AllEvidence_[type])[(*modsite_[type])[p]]).size()-1);
       //Pval = gsl_cdf_chisq_P(Xsq, (*(*site_AllEvidence_[type])[(*modsite_[type])[p]]).size()-1);
       //Qval = 1 - Pval;
       //if (r) {
       // varModEvid = gsl_stats_variance(gsl_vector_ptr(r, 0), 1, (*mods_[type]).size()); 
       //meanModEvid = gsl_stats_mean(gsl_vector_ptr(r, 0), 1, (*mods_[type]).size());
       //sdModEvid = gsl_stats_sd(gsl_vector_ptr(r, 0), 1, (*mods_[type]).size());
       // gsl_vector_free(r);
       // r = NULL;
       //}

       //double Nmod = (*(*site_AllEvidence_[type])[(*modsite_[type])[p]]).size();
       //double Nnomod = (*(*site_nomodAllEvidence_[type])[(*modsite_[type])[p]]).size();
       
       //if ( !Nmod || Nnomod < 2 || Nmod+Nnomod<= 1) {
       // continue;
       //}
       //else
       //(Nnomod < 2 || Nmod < 2 ) { //CHISQUARED FOR EVERYTHING SHORTCUT
       //	 double Xsq = pow( ((*site_MaxEvidence_[type])[(*modsite_[type])[p]]-meanNoModEvid) - ((*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]] - meanNoModEvid), 2) / ((*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]] - meanNoModEvid);
       //Pval = gsl_cdf_chisq_P(Xsq, (*site_N_[type])[(*modsite_[type])[p]]-1);
	 
//        if (nmods_[type] < 2 || (*modsite_[type]).size() < 2 || Nnomod < 2 || Nmod < 2) {
// 	     double Em = (*site_ExpModEvidence_[type])[(*modsite_[type])[p]];
// 	     double Om = (*site_ObsModEvidence_[type])[(*modsite_[type])[p]];
	     
// 	     double Eu = (*site_ExpUnModEvidence_[type])[(*modsite_[type])[p]];
// 	     double Ou = (*site_ObsUnModEvidence_[type])[(*modsite_[type])[p]];

// 	     if (Om > Ou) {
// 	       Pval = gsl_cdf_chisq_P((Om-Ou)*(Om-Ou)/Om, 1);
// 	     }
// 	     else {
// 	       Pval = gsl_cdf_chisq_Q((Om-Ou)*(Om-Ou)/Ou, 1);	       
// 	     }
//        }
//        else if (nmods_[type] < 2 || (*modsite_[type]).size() < 2) {

       if (nmods_[type]) {
	     double Em = (*site_ExpModEvidence_[type])[(*modsite_[type])[p]];
	     double Om = (*site_ObsModEvidence_[type])[(*modsite_[type])[p]];
	     
	     double Eu = (*site_ExpUnModEvidence_[type])[(*modsite_[type])[p]];
	     double Ou = (*site_ObsUnModEvidence_[type])[(*modsite_[type])[p]];
	     
	     //	     Xsq = pow(Em-Om,2)/Em + pow(Eu-Ou,2)/Eu;

	     modXsq = pow(Em-Om,2)/Em; 

	     unmodXsq = pow(Eu-Ou,2)/Eu;

	     Om = (*site_MaxEvidence_[type])[(*modsite_[type])[p]]/minInt_;
	     Ou = (*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]]/minInt_;

	     if (Om < 1) {
	       Om = 1.;
	     }

	     if (Ou < 1) {
	       Ou = 1.;
	     }
	     
	     int N = (int)ceil(Om+Ou);
	     int k = (int)ceil(Om);
	     double p =  0.5;//(nmods_)[type] / (modsite_)[type]->size();
	     //double hund_Om = 100*Om/(Om+Ou);
	     //double hund_Ou = 100*Ou/(Om+Ou);

	     //Xsq = (Om-Ou)*(Om-Ou)/Ou;

	     //if (modXsq+unmodXsq > Xsq) {
	     //  Xsq = modXsq+unmodXsq;
	     //}

	     //	     if (Om > Ou && Om > Em) {// if ((*site_MaxEvidence_[type])[(*modsite_[type])[p]] >= (*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]]) {

	     //Xsq = pow(Om-Ou,2)/1;

	     //double Exp = Om+Ou / 2;
	     //Xsq = pow(hund_Om-50,2)/50 + pow(hund_Ou-50,2)/50 + gsl_cdf_chisq_Pinv(0.5, 1);
	     //Xsq = gsl_cdf_chisq_Pinv(0.5, 1);;
	     //double df = 1;
	
//	      for (int m_i=0; m_i < (*site_AllEvidence_[type])[(*modsite_[type])[p]]->size() ; m_i++) {
// 	       for (int u_i=0; u_i < (*site_nomodAllEvidence_[type])[(*modsite_[type])[p]]->size() ; u_i++) {
// 		 Om = (*(*site_AllEvidence_[type])[(*modsite_[type])[p]])[m_i];
// 		 Ou = (*(*site_nomodAllEvidence_[type])[(*modsite_[type])[p]])[u_i];
// 		 hund_Om = 100*Om/(Om+Ou);
// 		 hund_Ou = 100*Ou/(Om+Ou);
// 		 Xsq +=  pow(hund_Om-50,2)/50 + pow(hund_Ou-50,2)/50 ;//pow(Om-Ou,2)/Ou + pow(Om-Ou,2)/50;	       
// 		 df++;
// 	       }

// 	     }
// 	     if (df <= 0) {
// 	       df = 1;
// 	     }


	     if (Om > Ou) { // && Om > Ou) {// if ((*site_MaxEvidence_[type])[(*modsite_[type])[p]] >= (*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]]) {
	       // Xsq = pow(Om-Ou,2)/Ou + 0.46;
	       //Pval = gsl_cdf_chisq_P(Xsq, df);//nmods_[type]);//* gsl_cdf_chisq_Q(unmodXsq, 1);
	       Pval = gsl_cdf_binomial_P(k, p, N);

	       
	       //Pval = 1-gsl_ran_chisq_pdf(Xsq, 2);//* gsl_cdf_chisq_Q(unmodXsq, 1);
	       
	       //if (Pval < 0.5) {
	       //	 Pval = gsl_cdf_binomial_Q(k, 0.5, N); //Pval = gsl_cdf_chisq_Q(Xsq,  df);//nmods_[type]);
		 //Pval = 0.5;
	       //}
	     }
	     else if (Om < Ou)  {
	       //p (Om-Ou)*(Om-Ou)/Ou
	       //Xsq = pow(Om-Ou,2)/Om + 0.46;
	       //Pval = gsl_cdf_chisq_Q(Xsq, df);// nmods_[type]);
	       Pval = gsl_cdf_binomial_Q(N-k, p, N); 
	       //Pval = gsl_ran_chisq_pdf(Xsq, 2);//gsl_cdf_chisq_Q(Xsq, 1);//* gsl_cdf_chisq_P(unmodXsq, 1);
	       

	       //if (Pval > 0.5) {
		 
	       //	 Pval = gsl_cdf_binomial_P(k, 0.5, N); //Pval = gsl_cdf_chisq_P(Xsq, df);//nmods_[type]);
	         //Pval = 0.5;
	       //}
		//}
		//else {
		//Pval = 0.5;
	     }
	     else {
	       Pval = p;
	     }
	     


       }
	     //	     if (Om > Ou && Om > Em && Ou > 1) {// if ((*site_MaxEvidence_[type])[(*modsite_[type])[p]] >= (*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]]) {
	     //  Pval = gsl_cdf_chisq_P(modXsq, 1);
	     // }
	     //else {
	     // Pval = gsl_cdf_chisq_Q(unmodXsq, 1);
	     //}

	     //  (*site_MaxEvidence_[type])[(*modsite_[type])[p]] = Pval;


	 //(*site_MaxEvidence_[type])[(*modsite_[type])[p]]  =  exp((*site_MaxEvidence_[type])[(*modsite_[type])[p]] -  (*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]]);





	 //	 continue;



// 	 //CHI-SQUARED TEST of INDEPENDENCE
// 	 double Xsq = 0;
// 	 double Pval = 0;
// 	 double sum_Obs_mod = 0;
// 	 double sum_Obs_nomod = 0;

// 	 vector<vector<double>*>* Eij = new vector<vector<double>*>() ;

// 	 vector<double>* sum_Oi = new vector<double>();
// 	 vector<double>* sum_Oj = new vector<double>();

// 	 double N = 0;
// 	 for (int j=0; j<Nnomod; j++) {
// 	   N += (*(*site_nomodAllEvidence_[type])[(*modsite_[type])[p]])[j];

// 	 }

// 	 for (int i=0; i<Nmod; i++) {
// 	   N += (*(*site_AllEvidence_[type])[(*modsite_[type])[p]])[i];

// 	 }

// 	 //Eij = sum_Obs_mod * sum_Obs_nomod / N;

// 	 for (int i=0; i<Nmod; i++) {
// 	   sum_Oi->push_back(0);
// 	   for (int j=0; j<Nnomod; j++) {
// 	     (*sum_Oi)[i] += (*(*site_nomodAllEvidence_[type])[(*modsite_[type])[p]])[j];
// 	   }
	   
// 	 }


// 	 for (int j=0; j<Nnomod; j++) {
// 	   sum_Oj->push_back(0);
// 	   for (int i=0; i<Nmod; i++) {
// 	      (*sum_Oj)[j] +=(*(*site_AllEvidence_[type])[(*modsite_[type])[p]])[i];	     
// 	   }
	   
// 	 }

// // 	 for (int i=0; i<Nmod; i++) {
// // 	   Eij->push_back(new vector<double>());
// // 	   for (int j=0; j<Nnomod; j++) {
// // 	     (*Eij)[i]->push_back( (*sum_Oi)[i] * (*sum_Oj)[j] / N );
// // 	   }
// // 	 }


// 	 // for (int i=0; i<Nmod; i++) {
// 	 //  for (int j=0; j<Nnomod; j++) {
// 	     //Xsq +=pow( (*(*site_AllEvidence_[type])[(*modsite_[type])[p]])[i] - (*(*Eij)[i])[j], 2) / (*(*Eij)[i])[j];

// 	 //	 Xsq +=pow( (*(*site_AllEvidence_[type])[(*modsite_[type])[p]])[i] - (*sum_Oj)[j]/Nnomod, 2) / ((*sum_Oj)[j]/Nnomod);
// 	     //Xsq +=pow( (*(*site_nomodAllEvidence_[type])[(*modsite_[type])[p]])[j] - (*(*Eij)[i])[j], 2) / (*(*Eij)[i])[j];
// 	     // }
// 	     // }

// 	 for (int j=0; j<Nnomod; j++) {
// 	   Xsq +=pow( (*site_MaxEvidence_[type])[(*modsite_[type])[p]] - (*sum_Oj)[j]/Nnomod, 2) / ((*sum_Oj)[j]/Nnomod);
// 	 }
	 
// 	 double df = (Nmod-1)*(Nnomod-1);
// 	 if (!df) df = 1;

// 	 Pval = gsl_cdf_chisq_P(Xsq, df);
	 
// 	 // 	 for (int i=0; i<Nmod; i++) {
// 	 // 	   delete (*Eij)[i];	   
// 	 // 	 }
// 	 delete sum_Oi;
// 	 delete sum_Oj;
	 //	 delete Eij;
       //       }

    //DISABLE t-test HERE
       //     else  {
	 //TODO:: TRY DEPENDENT T-test for Paired Samples
	 
	 //double t = (meanModEvid-meanNoModEvid) / sqrt(varModEvid/Nmod +
	 //varNoModEvid/Nnomod );

	 //WELCH
	 //       double Sdenom = sqrt(varModEvid/Nmod+varNoModEvid/Nnomod);
	 
	 //double Sdenom = sqrt(varNoModEvid/Nnomod);
	 
	 //double t = ( ((*site_MaxEvidence_[type])[(*modsite_[type])[p]]-meanNoModEvid) - ((*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]] - meanNoModEvid)) / Sdenom;
	 
	 
	 //WELCH_SATTERTHWAITE
	 //double v = pow(varModEvid/Nmod + varNoModEvid/Nnomod, 2) / 
	 //	 (pow(varModEvid,2)/(pow(Nmod,2)*(Nmod-1))+pow(varNoModEvid,2)/(pow(Nnomod,2)*(Nnomod-1)));

	 //double v = Nnomod-1;//pow(varModEvid/Nmod + varNoModEvid/Nnomod, 2) / 
	 //	 (pow(varModEvid,2)/(pow(Nmod,2)*(Nmod-1))+pow(varNoModEvid,2)/(pow(Nnomod,2)*(Nnomod-1)));
	 
	 //maxModEvid = (*site_MaxEvidence_[type])[(*modsite_[type])[p]];
	 //       double t = (maxModEvid-meanNoModEvid) / sqrt(varNoModEvid/Nnomod );
	 //double v = Nnomod - 1;
	 
	 //Pval = gsl_cdf_tdist_P(t, v);
	 //Qval = 1 - Pval;
       //}
	     
       (*site_Pval_[type])[(*modsite_[type])[p]] = Pval;
	     
       // (*site_MaxEvidence_[type])[(*modsite_[type])[p]] = Pval;
       //WORKS OK!       (*site_MaxEvidence_[type])[(*modsite_[type])[p]] = exp((*site_MaxEvidence_[type])[(*modsite_[type])[p]]/(*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]]);



  }

  delete posEvidSum;

}

// void PTMProphetMpx::evaluateModPositions(int type) {
  
//   double maxEvid = 0;
//   double maxDecEvid = 0;
  
//   bool start=true;
//   double minFor = 1e6;
//   double minAgainst = 1e6;

//   double meanEvid = 0;
//   double meanDecEvid = 0;

//   double sumFor =0;
//   double sumAgainst =0;


//   for (vector<int>::size_type p = 0; p < (*modsite_[type]).size(); p++) {
//     double forEvidence = (*site_MaxEvidence_[type])[(*modsite_[type])[p]] - (*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]];
    
//     double againstEvidence = (*site_decoyMaxEvidence_[type])[(*modsite_[type])[p]] - (*site_nomodDecoyMaxEvidence_[type])[(*modsite_[type])[p]];

// //     sumFor += (*site_MaxEvidence_[type])[(*modsite_[type])[p]];

// //     sumAgainst += (*site_MaxEvidence_[type])[(*modsite_[type])[p]];


//     if (start) {
//       minFor = forEvidence;
//       minAgainst = againstEvidence;
      
//     }
    
//     if (minFor > forEvidence) {
//       minFor = forEvidence;
//     }
    
//     if (minAgainst > againstEvidence) {
//       minAgainst = againstEvidence;
//     }
    
//     start = false;
    
//   }  

  
//   double forCorr = 0;
//   double againstCorr = 0;
  
//   if (minFor < forCorr) {
//     forCorr = -1*(minFor+minInt_);
//   }
  
//   if (minAgainst < againstCorr) {
//     againstCorr = -1*(minAgainst+minInt_);
//   }
  
  

//   gsl_vector* r = NULL;
//   if ((*modsite_[type]).size()) {
//     r = gsl_vector_calloc((*modsite_[type]).size());
//   }
//   for (vector<int>::size_type p = 0; p < (*modsite_[type]).size(); p++) {
//     double forEvidence =  (*site_MaxEvidence_[type])[(*modsite_[type])[p]] - (*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]] + forCorr ;
//     sumFor += forEvidence;
//     gsl_vector_set(r, p, forEvidence);
//   }
//   double var = 1;
//   if (r) {
//     var = gsl_stats_variance(gsl_vector_ptr(r, 0), 1, (*modsite_[type]).size()); 
//     gsl_vector_free(r);
//   }
//   for (vector<int>::size_type p = 0; p < (*modsite_[type]).size(); p++) {
//    //      double forEvidence = (*site_MaxEvidence_[type])[(*modsite_[type])[p]] - (*site_MinEvidence_[type])[(*modsite_[type])[p]];
//    //double againstEvidence = (*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]] - (*site_nomodMinEvidence_[type])[(*modsite_[type])[p]];
   
//    //   double forEvidence = (*site_MaxEvidence_[type])[(*modsite_[type])[p]] - (*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]] + forCorr;

//    //   double forEvidence = exp((*site_MaxEvidence_[type])[(*modsite_[type])[p]] - (*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]]);

//    //   double forEvidence = -log(nmods_[type]*(*site_MaxEvidence_[type])[(*modsite_[type])[p]]/sumFor);
//    //    double againstEvidence = -log((*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]]/sumAgainst);

//    //    double nullEvidence = -log(getModPrior(type));

//    //    //LOG Likelihood Test

//    //    double D = -2*nullEvidence+2*forEvidence;
   
//     //   double forEvidence = ( (*site_MaxEvidence_[type])[(*modsite_[type])[p]] - (*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]] + forCorr )/sumFor;

//     //   double nullEvidence = getModPrior(type);

//    double forEvidence = ( (*site_MaxEvidence_[type])[(*modsite_[type])[p]] - (*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]] + forCorr );

//    double nullEvidence = minInt_;

   
//    double X2 = (fabs(forEvidence - nullEvidence) *  fabs(forEvidence - nullEvidence )) / var;


//    double D = 2*log(nullEvidence)-2*log(forEvidence);

 

//    double BayesProb = 
//      (*site_MaxEvidence_[type])[(*modsite_[type])[p]] / ((*site_MaxEvidence_[type])[(*modsite_[type])[p]]+(*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]]) * 
//      (*site_MaxEvidence_[type])[(*modsite_[type])[p]]/sumFor + 
//      (*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]] / ((*site_MaxEvidence_[type])[(*modsite_[type])[p]]+(*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]]) * 
//      (*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]]/sumAgainst;

//    //double againstEvidence = exp((*site_decoyMaxEvidence_[type])[(*modsite_[type])[p]] - (*site_nomodDecoyMaxEvidence_[type])[(*modsite_[type])[p]]);
   

   
//    //double decoyForEvidence = (*site_nomodMaxEvidence_[type])[(*modsite_[type])[p]]/(*site_N_[type])[(*modsite_[type])[p]] 
//    //	- (*site_decoyEvidence_[type])[(*modsite_[type])[p]];
   
//    //      if (decoyForEvidence < minInt_) {
//    //      	decoyForEvidence = minInt_;
//    //      }
//    //if (forEvidence < minInt_) {
//    //  forEvidence = minInt_;
//    //}
//    //  if (againstEvidence < minInt_) {
//    //  againstEvidence = minInt_;
//   //}


//    //   double Xsq = forEvidence;


//    double Xsq = 0;//forEvidence;
  
//    if ((*site_N_[type])[(*modsite_[type])[p]] > 0) {
//      //  Xsq = (fabs(forEvidence - 1) *  fabs(forEvidence -  )) / againstEvidence;

     
//      Xsq = gsl_cdf_chisq_Q(D, 1);
     
//      //	if (forEvidence > decoyForEvidence) {
//      //Xsq = gsl_cdf_chisq_P(Xsq, (*site_N_[type])[(*modsite_[type])[p]]-1);
//      //}
//      //else {
//      //  Xsq = gsl_cdf_chisq_P(Xsq, (*site_N_[type])[(*modsite_[type])[p]]-1);
//      //}
     
//    }
   
//    (*site_MaxEvidence_[type])[(*modsite_[type])[p]] = X2;
   
   
//  }



// }

double PTMProphetMpx::getProbAtPosition(int type, int i) {
  //i += ntermod_[type];
  if ((*pos_prob_hash_[type]).find(i) != (*pos_prob_hash_[type]).end()) {
    //if ( (*pos_prob_hash_[type])[i] > 0.5) {
    //  return 0.5;
    //}
    return  (*pos_prob_hash_[type])[i];
  }
  else {
    return -1.;
  }
}   


void PTMProphetMpx::computePosnProbs() {
  for (int type = 0; type < shift_.size(); type++) {
    //double prior = (double)nmods_[type] / (double)(*modsite_[type]).size();
    if (type >= pep_prob_str_.size()) {
      pep_prob_str_.push_back("");
    }
    
    double siteProbSum = 0;
    if (type >= pos_prob_hash_.size()) {
      pos_prob_hash_.push_back(new TPP_HASHMAP_T<int, double>());  
    }
    

    if (!getNumModSites(type) || !getNumMods(type)) {
      continue;
    }
    else if (getNumModSites(type) == getNumMods(type)) {
      for (vector<int>::size_type p = 0; p < (*modsite_[type]).size(); p++) {
	(*siteprob_[type])[(*modsite_[type])[p]] = 1;
      }
      
    }
    else {
      for (vector<int>::size_type p = 0; p < (*modsite_[type]).size(); p++) {
	double Q = 0;
	//if ((*site_N_[type])[(*modsite_[type])[p]] > 0) 
	  Q = (*site_Pval_[type])[(*modsite_[type])[p]]; // / (*site_N_[type])[(*modsite_[type])[p]];
	
	(*siteprob_[type])[(*modsite_[type])[p]] = Q;
	
	siteProbSum += Q;
      }
      
      for (vector<int>::size_type p = 0; p < (*modsite_[type]).size(); p++) {
	
	if (siteProbSum > 0) {
	  (*siteprob_[type])[(*modsite_[type])[p]] = nmods_[type] *  
	    (*siteprob_[type])[(*modsite_[type])[p]]/siteProbSum;
	}
	else {
	  (*siteprob_[type])[(*modsite_[type])[p]] = 0;
	}
	
	if ((*siteprob_[type])[(*modsite_[type])[p]] > 1) {
	  (*siteprob_[type])[(*modsite_[type])[p]]  = 1;
	}
	if (isnan((*siteprob_[type])[(*modsite_[type])[p]])) {
	  (*siteprob_[type])[(*modsite_[type])[p]] = 0;
	}
      }
    }
    
 
    pep_prob_str_[type] = "";

    stringstream ss;
    ss.precision(3);
    ss.setf(ios::fixed); 
    //ntermod_[type] = (pep_->nTermMod.empty() && modaas_[type].find('n') == string::npos) ? 0 : 1;
    int ntermod = nTermMod();//ntermod_[type]; //pep_->nTermMod.empty()  ? 0 : 1;
    int adj =  nTermMod()  ? 1 : 0; //pep_->nTermMod.empty()  
    if (ntermod) {
      char aa = 'n';
      ss << aa;
      if (modaas_[type].find(aa) != string::npos) {
	
	ss << "(";
	ss.width(5); 
	ss  << (*siteprob_[type])[0] ;
	ss.width(0); 
	ss << ")";

	(*pos_prob_hash_[type])[0] = (*siteprob_[type])[0];
      } 
    }

    for (unsigned int pos = ntermod; pos < pep_->NAA() + ntermod; pos++) {
      char aa = pep_->stripped[pos-ntermod];
      ss << aa;
      if (modaas_[type].find(aa) != string::npos) {
	
	ss << "(";
	ss.width(5); 
	ss  << (*siteprob_[type])[pos] ;
	ss.width(0); 
	ss << ")";

	(*pos_prob_hash_[type])[pos] = (*siteprob_[type])[pos];
      } 
      else {
	// nomodsite_.push_back(pos);
	//siteprob.push_back(0);
      }

    }

    ctermod_[type] = (pep_->cTermMod.empty() && modaas_[type].find('c') == string::npos) ? 0 : 1;
    int ctermod = pep_->cTermMod.empty() ? 0 : 1;
    
    if (ctermod) {
      char aa = 'c';
      ss << aa;
      if (modaas_[type].find(aa) != string::npos) {
	
	ss << "(";
	ss.width(5); 
	ss  << (*siteprob_[type])[pep_->NAA() + ntermod + ctermod - 1] ;
	ss.width(0); 
	ss << ")";
	
	(*pos_prob_hash_[type])[pep_->NAA() + ntermod + ctermod - 1] = (*siteprob_[type])[pep_->NAA() + ntermod + ctermod - 10];
      } 
    }


    
    pep_prob_str_[type] = ss.str() ;
        


  }
}


int PTMProphetMpx::nTermMod() {  
  for (int t=0; t< shift_.size(); t++) {
    if (ntermod_[t]) 
      return 1;
  }
  return 0; 
}
int PTMProphetMpx::cTermMod() { 
  for (int t=0; t< shift_.size(); t++) { 
    if (ctermod_[t]) 
      return 1; 
  } 
  return 0; 
}
  


void PTMProphetMpx::setPrecision(ostringstream & stream , double& value) {
  // if (fabs(value) >= 1 && fabs(value) < 10){
  //   stream.precision(0);
  //   stream.width(1);
  // }
  // else if (fabs(value) >= 10 && fabs(value) < 100){
  //   stream.precision(0);
  //   stream.width(2);
  // }
  // else if (fabs(value) >= 100 && fabs(value) < 1000){
  //   stream.precision(0);
  //    stream.width(3);
  // }
  // else if (fabs(value) >= 1000 && fabs(value) < 10000){
  //   stream.precision(0);
  //   stream.width(4);
  // }
  // else if (fabs(value) >= 10000 && fabs(value) < 100000){
  //   stream.precision(0);
  //   stream.width(5);
  // }
  // else if (fabs(value) >= 100000 && fabs(value) < 1000000){
  //   stream.precision(0);
  //   stream.width(6);
  // }
  stream.precision(0);
  stream.setf(ios::fixed); 
  return;
}
