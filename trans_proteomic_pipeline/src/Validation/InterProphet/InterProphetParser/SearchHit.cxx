#include "SearchHit.h"

SearchHit::SearchHit(const int run_idx, string& spect, 
	    double pepproph_prob, Array<double>* allntt_pepproph_prob, 
	    string& pepseq, string& modpep, string& swathpep, 
	    string& msrun, double calcnmass, string& exp, string& charge) {
    run_idx_ = run_idx;
    pepproph_prob_ = pepproph_prob;
    adj_prob_ = pepproph_prob;
    for (int i = 0; i < 3; i++) {
      allntt_pepproph_prob_[i] = (*allntt_pepproph_prob)[i];
      allntt_adj_prob_[i]= (*allntt_pepproph_prob)[i];
    }
    spect_ = spect;
    //chg_ = spect.substr(spect.find_last_of("."));
    chg_ = charge;
    peptide_ = pepseq;
    modpep_ = modpep;
    swathpep_ = swathpep;
    calcnmass_ = calcnmass;
    msrun_ = msrun;
    exp_ = exp;
    prots_ = NULL;
}

SearchHit::SearchHit(const int run_idx, string& spect, 
	    double pepproph_prob, Array<double>* allntt_pepproph_prob, 
	    string& pepseq, string& modpep,  string& swathpep, 
	    string& msrun, double calcnmass, string& exp, string& charge, Array<string*>* prots) {
    run_idx_ = run_idx;
    pepproph_prob_ = pepproph_prob;
    adj_prob_ = pepproph_prob;
    for (int i = 0; i < 3; i++) {
      allntt_pepproph_prob_[i] = (*allntt_pepproph_prob)[i];
      allntt_adj_prob_[i]= (*allntt_pepproph_prob)[i];
    }
    spect_ = spect;
    //chg_ = spect.substr(spect.find_last_of("."));
    chg_ = charge;
    peptide_ = pepseq;
    modpep_ = modpep;
    swathpep_ = swathpep;
    calcnmass_ = calcnmass;
    msrun_ = msrun;
    exp_ = exp;
    prots_ = prots;
}

SearchHit::SearchHit(const int run_idx, string& spect, 
	    double pepproph_prob, Array<double>* allntt_pepproph_prob, 
	    string& pepseq, string& modpep,  string& swathpep, 
		     string& msrun, double calcnmass, double rt, string& exp, string& charge) {
    run_idx_ = run_idx;
    pepproph_prob_ = pepproph_prob;
    adj_prob_ = pepproph_prob;
    for (int i = 0; i < 3; i++) {
      allntt_pepproph_prob_[i] = (*allntt_pepproph_prob)[i];
      allntt_adj_prob_[i]= (*allntt_pepproph_prob)[i];
    }
    spect_ = spect;
    //chg_ = spect.substr(spect.find_last_of("."));
    chg_ = charge;
    peptide_ = pepseq;
    modpep_ = modpep;
    swathpep_ = swathpep;
    calcnmass_ = calcnmass;
    msrun_ = msrun;
 
    exp_ = exp;
    rt_ = rt;
    prots_ = NULL;
    
  }


SearchHit::SearchHit(const int run_idx, string& spect, 
	    double pepproph_prob, Array<double>* allntt_pepproph_prob, 
	    string& pepseq, string& modpep,   string& swathpep, 
		     string& msrun, double calcnmass, double rt,  int swath_window, int alt_swath, string& exp, string& charge, Array<string*>* prots) {
    run_idx_ = run_idx;
    pepproph_prob_ = pepproph_prob;
    adj_prob_ = pepproph_prob;
    for (int i = 0; i < 3; i++) {
      allntt_pepproph_prob_[i] = (*allntt_pepproph_prob)[i];
      allntt_adj_prob_[i]= (*allntt_pepproph_prob)[i];
    }
    spect_ = spect;
    //chg_ = spect.substr(spect.find_last_of("."));
    chg_ = charge;
    peptide_ = pepseq;
    modpep_ = modpep;
    swathpep_ = swathpep;
    swathwin_ = swath_window;
    altswath_ = alt_swath;
    calcnmass_ = calcnmass;
    msrun_ = msrun;
    exp_ = exp;
    rt_ = rt;
    prots_ = prots;
    
  }
