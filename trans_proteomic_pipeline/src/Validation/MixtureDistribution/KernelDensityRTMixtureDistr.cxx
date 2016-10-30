#include "KernelDensityRTMixtureDistr.h"

/*

Program       : KernelDensityRTMixtureDistribution for PeptideProphet                                                       
Author        : Andrew Keller <akeller@systemsbiology.org>                                
                David Shteynberg   
Date          : 11.27.02 
Revision      : $Id$

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


KernelDensityRTMixtureDistr::KernelDensityRTMixtureDistr(int charge, const char* name, const char* tag) : RTMixtureDistr(charge, name, tag) {
  vals_ = new Array<double>;
  doublevals_ = vals_;
  RTvals_ = new Array<double>;
  model_ = new KDModel(name, 100);
  update_ctr_ = 0;
  ready_ = false;
  min_ctr_ = 2; 
}

KernelDensityRTMixtureDistr::~KernelDensityRTMixtureDistr() {
  delete RTvals_;
  delete model_;
}

void KernelDensityRTMixtureDistr::write_RTstats(ostream& out) {
  for (int i = 0; i < run_RT_calc_->size(); i++) {
    (*run_RT_calc_)[i]->write_RTstats(out);
  }
  all_RT_calc_->write_RTstats(out);
  
}

void KernelDensityRTMixtureDistr::write_RTcoeff(ostream& out) {
 
  all_RT_calc_->write_RTcoeff(out);
  
}


void KernelDensityRTMixtureDistr::recalc_RTstats(Array<Array<double>*>* probs) {
  recalc_RTstats(probs, false);
}

void KernelDensityRTMixtureDistr::recalc_RTstats(Array<Array<double>*>* probs, bool catalog) {
  vals_->reserve(vals_->size());
  RTvals_->reserve(RTvals_->size());
  for (int i = 0; i < run_RT_calc_->size(); i++) {
    (*run_RT_calc_)[i]->recalc_RTstats((*probs)[i]);
    for (int j=0; j < (*run_RT_calc_)[i]->RTs_->size(); j++) {
      double dval;
      if (catalog) {
	(*run_RT_calc_)[i]->getRTScore(j);
      }
      else {
	dval = (*run_RT_calc_)[i]->getRTScore((*(*run_RT_calc_)[i]->RTs_)[j], ((*run_RT_calc_)[i]->scans_)[j], ((*run_RT_calc_)[i]->rts_)[j]);
      }

      RTvals_->insertAtEnd((*(*run_RT_calc_)[i]->RTs_)[j]);
      //if (dval > 5) dval = 5;
      //if (dval < -5) dval = -5;
      vals_->insertAtEnd(dval);
      model_->insert((*(*probs)[i])[j], dval);
    }
  }

}
bool KernelDensityRTMixtureDistr::recalc_RTstats(Array<Array<double>*>* probs, double min_prob, Array<Array<int>*>* ntts, int  min_ntt) {
 return  recalc_RTstats( probs, min_prob,  ntts, min_ntt, false);
}
bool KernelDensityRTMixtureDistr::recalc_RTstats(Array<Array<double>*>* probs, double min_prob, Array<Array<int>*>* ntts, int  min_ntt, bool catalog) {
  bool rtn = true;
  Boolean tmp = true;
  vals_->reserve(vals_->size());
  RTvals_->reserve(RTvals_->size());
  for (int i = 0; i < run_RT_calc_->size(); i++) {   
    tmp = (*run_RT_calc_)[i]->recalc_RTstats((*probs)[i], min_prob, (*ntts)[i], min_ntt, 2700);
    // tmp = (*run_RT_calc_)[i]->recalc_RTgsl((*probs)[i], min_prob, (*ntts)[i], min_ntt);
    
    //    (*run_RT_calc_)[i]->plotRegressionFits((*probs)[i], min_prob, (*ntts)[i], min_ntt);
    rtn = rtn && tmp;
    for (int j=0; j < (*run_RT_calc_)[i]->RTs_->size(); j++) {
      
      double dval;
      //if (catalog) {
      //	dval = (*run_RT_calc_)[i]->getRTScore(j);
      //}
      //else {
      dval = (*run_RT_calc_)[i]->getRTScore((*(*run_RT_calc_)[i]->RTs_)[j], ((*run_RT_calc_)[i]->scans_)[j], ((*run_RT_calc_)[i]->rts_)[j]);
      //}
      RTvals_->insertAtEnd((*(*run_RT_calc_)[i]->RTs_)[j]);
      //      if (tmp) {
      // if (dval > 5) dval = 5;
      //if (dval < -5) dval = -5;
      vals_->insertAtEnd(dval);
      model_->insert((*(*probs)[i])[j], dval);
	//    }
    }
  }
  return rtn;
}

bool KernelDensityRTMixtureDistr::recalc_RTstats(Array<double>* probs, double min_prob,  int  min_ntt) {
  bool rtn = true;
  Boolean tmp = true;
  vals_->reserve(vals_->size());
  RTvals_->reserve(RTvals_->size());
  //  for (int i = 0; i < run_RT_calc_->size(); i++) {    
  //  tmp = all_RT_calc_->recalc_RTgsl(min_prob, min_ntt);
    
    //    (*run_RT_calc_)[i]->plotRegressionFits((*probs)[i], min_prob, (*ntts)[i], min_ntt);
    rtn = rtn && tmp;
    for (int j=0; j < all_RT_calc_->RTs_->size(); j++) {
      double dval = all_RT_calc_->getRTScore((*all_RT_calc_->RTs_)[j], (all_RT_calc_->scans_)[j], (all_RT_calc_->rts_)[j]);
      RTvals_->insertAtEnd((*all_RT_calc_->RTs_)[j]);
      //      if (tmp) {
      //if (dval > 5) dval = 5;
      //if (dval < -5) dval = -5;
	vals_->insertAtEnd(dval);
	model_->insert((*probs)[j], dval);
	// }
    }
    //}

  return rtn;
}

bool KernelDensityRTMixtureDistr::recalc_RTstats(Array<double>* probs, double min_prob, Array<int>* ntts, int  min_ntt) {
  bool rtn = true;
  Boolean tmp = true;
  vals_->reserve(vals_->size());
  RTvals_->reserve(RTvals_->size());
  for (int i = 0; i < run_RT_calc_->size(); i++) {    
  //  tmp = all_RT_calc_->recalc_RTgsl(probs, min_prob, ntts, min_ntt);

  //  tmp = all_RT_calc_->recalc_RTstats(probs, min_prob, ntts, min_ntt);

    //    (*run_RT_calc_)[i]->plotRegressionFits((*probs)[i], min_prob, (*ntts)[i], min_ntt);
    rtn = rtn && tmp;
    for (int j=0; j < (*run_RT_calc_)[i]->RTs_->size(); j++) {
    //double dval = all_RT_calc_->getRTScore((*all_RT_calc_->RTs_)[j], (all_RT_calc_->scans_)[j], (all_RT_calc_->rts_)[j]);

      double dval =  (*run_RT_calc_)[i]->getRTScore((*(*run_RT_calc_)[i]->RTs_)[j], 
						    ((*run_RT_calc_)[i]->scans_)[j], 
						    ((*run_RT_calc_)[i]->rts_)[j]);
      RTvals_->insertAtEnd((*(*run_RT_calc_)[i]->RTs_)[j]);
      //      if (tmp) {
      //if (dval > 5) dval = 5;
      //if (dval < -5) dval = -5;
      vals_->insertAtEnd(dval);
      model_->insert((*probs)[j], dval);
      // }
    }
  }
    //}
  return rtn;
}

void KernelDensityRTMixtureDistr::calc_RTstats() {
  vals_->reserve(vals_->size());
  RTvals_->reserve(RTvals_->size());
  for (int i = 0; i < run_RT_calc_->size(); i++) {
    (*run_RT_calc_)[i]->calc_RTstats();
    for (int j=0; j < (*run_RT_calc_)[i]->RTs_->size(); j++) {
      double dval = (*run_RT_calc_)[i]->getRTScore((*(*run_RT_calc_)[i]->RTs_)[j], ((*run_RT_calc_)[i]->scans_)[j], ((*run_RT_calc_)[i]->rts_)[j]);
      
      RTvals_->insertAtEnd((*(*run_RT_calc_)[i]->RTs_)[j]);

      //if (dval > 5) dval = 5;
      //if (dval < -5) dval = -5;

      vals_->insertAtEnd(dval);

      model_->insert(0.5, dval);
    }
  }

}

void KernelDensityRTMixtureDistr::enter(int index, char* val) {
  double dval = atof(val);
  //  if (dval > 5) dval = 5;
  //if (dval < -5) dval = -5;
  //vals_->insertAtEnd(dval); // put the double value here
  //model_->insert(0.5, dval);
}

Boolean KernelDensityRTMixtureDistr::update(Array<Array<double>*>* all_probs) {
  return update(all_probs, (const char*) NULL);
}
Boolean KernelDensityRTMixtureDistr::update(Array<Array<double>*>* all_probs, const char* catfile) {
  Boolean rtn = False;

  Array<double>* probs = new Array<double>;
  
  
  for (int i=0; i<all_probs->size(); i++) {
    int len = (*all_probs)[i]->size();
    for (int j=0; j<len; j++) {
      probs->insertAtEnd((*(*all_probs)[i])[j]);
    }
  }
  
  if (!ready_) {
    recalc_RTstats(all_probs, catfile!=NULL);
    //    model_->makeReady(1,1);
    model_->makeReady(true,20);
    ready_ = true;
  }
  else if(model_->update(probs,0)) {
     rtn = True;
  }

  delete probs;
  return rtn;

}

Boolean KernelDensityRTMixtureDistr::update(Array<Array<double>*>* all_probs, double min_prob, 
					    Array<Array<int>*>* all_ntts, int min_ntt, int& code) {
  return update(all_probs, min_prob, all_ntts, min_ntt, code, NULL);
}

Boolean KernelDensityRTMixtureDistr::update(Array<Array<double>*>* all_probs, double min_prob, 
					    Array<Array<int>*>* all_ntts, int min_ntt, int& code, const char* catfile) {
  Boolean rtn = False;
  code = 0;
  Array<double>* probs = new Array<double>;
  Array<int>* ntts = new Array<int>;
  
  for (int i=0; i<all_probs->size(); i++) {
    int len = (*all_probs)[i]->size();
    for (int j=0; j<len; j++) {
      probs->insertAtEnd((*(*all_probs)[i])[j]);
      ntts->insertAtEnd((*(*all_ntts)[i])[j]);
    }
  }

  if (!ready_) {
    
    //DDS:First learn RT(SSR1) = a * SSR1 + b;
    for (int i = 0; i < run_RT_calc_->size(); i++) {
      //      (*run_RT_calc_)[i]->distillModifiedPeptides((*all_probs)[i], (*all_ntts)[i] );
            //      (*run_RT_calc_)[i]->recalc_RTstats( min_prob, min_ntt);	
      
      if (catfile != NULL) {
	(*run_RT_calc_)[i]->read_RTcoeff();
	(*run_RT_calc_)[i]->batchRTCalc();
	

	(*run_RT_calc_)[i]->calc_GradientCorrection( (*all_probs)[i], min_prob, (*all_ntts)[i], min_ntt, 2700);
	(*run_RT_calc_)[i]->gradientCorrect();
	(*run_RT_calc_)[i]->batchRTCalc(catfile);
	//(*run_RT_calc_)[i]->recalc_RTstats( (*all_probs)[i], min_prob, (*all_ntts)[i], min_ntt, 2700);
 
      }
      else {
	(*run_RT_calc_)[i]->batchRTCalc();

	(*run_RT_calc_)[i]->calc_GradientCorrection( (*all_probs)[i], min_prob, (*all_ntts)[i], min_ntt, 2700);
	(*run_RT_calc_)[i]->gradientCorrect();

	
	// (*run_RT_calc_)[i]->batchSSRCalcHP();
	//(*run_RT_calc_)[i]->recalc_RTstats( (*all_probs)[i], min_prob, (*all_ntts)[i], min_ntt, 2700);
      }

      //      (*run_RT_calc_)[i]->recalc_RTstats( (*all_probs)[i], min_prob, (*all_ntts)[i], min_ntt, 2700);
      //(*run_RT_calc_)[i]->gradientCorrect();
      (*run_RT_calc_)[i]->recalc_RTstats( (*all_probs)[i], min_prob, (*all_ntts)[i], min_ntt, 2700);
      
     //       (*run_RT_calc_)[i]->InvertLine();
    }  

    //    all_RT_calc_->batchSSRCalcHP();
    //    all_RT_calc_->linearTxRT(run_RT_calc_); 
    //    all_RT_calc_->distillModifiedPeptides(probs, ntts);
    //    all_RT_calc_->linearRegressRT( min_prob, min_ntt);
    // all_RT_calc_->linearRegressRT( probs, min_prob, ntts, min_ntt);


    if (!recalc_RTstats(all_probs, min_prob, all_ntts, min_ntt, catfile != NULL)) {
       code = 1;
       return False;
     }


    //    model_->makeReady(1,1);
   model_->makeReady(true,20);
    ready_ = true;
  }
  else if(model_->update(probs,0)) {
    //rtn ; // DCT this was not doing anything? why?
  }

  delete probs;
  delete ntts;
  return rtn;

}
Boolean KernelDensityRTMixtureDistr::update(Array<double>* all_probs, double min_prob, Array<int>* all_ntts, int min_ntt, int& code) {
  Boolean rtn;
  rtn = False;
  code = 0;


  if (!ready_) {
    for (int i = 0; i < run_RT_calc_->size(); i++) {
      //     bool canFit = (*run_RT_calc_)[i]->linearRegressRT(all_probs, min_prob, all_ntts, min_ntt);
      //    (*run_RT_calc_)[i]->plotRegressionFits((*all_probs)[i], (*all_ntts)[i], i);
       

      (*run_RT_calc_)[i]->batchRTCalc();

      //      (*run_RT_calc_)[i]->batchSSRCalcHP();
    }

   if (!recalc_RTstats(all_probs, min_prob, all_ntts, min_ntt)) {
      code = 1;
      return False;
    }
    //    model_->makeReady(1,1);
   model_->makeReady(true,20);
    ready_ = true;
  }
  else if(model_->update(all_probs,0)) {
   // rtn ; // DCT - This was not doing anything - why?
  }

  return rtn;
}




Array<Tag*>* KernelDensityRTMixtureDistr::getMixtureDistrTags(const char* name) {
  Array<Tag*>* output = model_->reportTags();
  return output;

}


double KernelDensityRTMixtureDistr::getPosProb(int index) {
  if (ready_) 
    return model_->getPosProb((*vals_)[index]);  
  
  return 0.5;
}

double KernelDensityRTMixtureDistr::getNegProb(int index) {
   if (ready_)
    return model_->getNegProb((*vals_)[index]);
  
  return 0.5;
}



char* KernelDensityRTMixtureDistr::getStringValue(int index) {
  char* output = new char[32];
  if (vals_ != NULL)
    sprintf(output, "%0.2f", (*vals_)[index]);

  return output;
}

char* KernelDensityRTMixtureDistr::getStringRTValue(int index) {
  char* output = new char[32];
   if(RTvals_ != NULL)
      sprintf(output, "%0.2f", (*RTvals_)[index]);
  return output;
}

int KernelDensityRTMixtureDistr::getNumVals() { 
  if (vals_ != NULL) {
    return vals_->size();
  }
  return 0;
}

