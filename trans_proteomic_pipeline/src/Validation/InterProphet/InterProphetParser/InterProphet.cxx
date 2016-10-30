/*

Program       : InterProphet                                                       
Author        : David Shteynberg <dshteynb  AT systemsbiology.org>                                                       
Date          : 12.12.07

Primary data object holding all mixture distributions for each precursor ion charge
%
Copyright (C) 2007 David Shteynberg

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
akeller@systemsbiology.org

*/

#include "InterProphet.h"

#ifdef __MINGW__
#define MSVC
#endif

#ifdef MSVC
#include "windows.h"
#else

#include <pthread.h>

#endif


#define MAX_PROB 0.999999
#define PROB_ADJ 1
#define MAX_DELTA 0.001


typedef struct str_thdata
{
  int thread_no;
  InterProphet* ipro;
} thdata;
#ifdef MSVC
DWORD WINAPI NSPThread(LPVOID ptr) {
#else
void* NSPThread(void* ptr) {
#endif
  int step = 0;
  int tot = 0;
  int offset = -1;
  thdata *data;            
  int inc = 1;

  data = (thdata *) ptr; 
  inc = data->ipro->max_threads_;
  offset = data->thread_no;
  step = data->ipro->hit_arr_->length()/100;
  
  for (int i=offset; i < data->ipro->hit_arr_->length() ; i+=inc) {      
    //progress
    if (i % step == 0) {
      cerr << "."; cerr.flush();
    }
    //data->ipro->progress(i, step, tot);
    data->ipro->getNSPCount((*(*(*data).ipro).hit_arr_)[i], offset);
  }
#ifdef MSVC
   ExitThread(0);
#else
   pthread_exit(0); 
#endif


}

#ifdef MSVC
DWORD WINAPI LengthThread(LPVOID ptr) {
#else
void* LengthThread(void* ptr) {
#endif
  int step = 0;
  int tot = 0;
  int offset = -1;
  thdata *data;            
  int inc = 1;

  data = (thdata *) ptr; 
  inc = data->ipro->max_threads_;
  offset = data->thread_no;
  step = data->ipro->hit_arr_->length()/100;
  
  for (int i=offset; i < data->ipro->hit_arr_->length() ; i+=inc) {      
    //progress
    if (i % step == 0) {
      cerr << "."; cerr.flush();
    }
    //data->ipro->progress(i, step, tot);
    data->ipro->getPeptideLength((*(*(*data).ipro).hit_arr_)[i], offset);
  }
#ifdef MSVC
   ExitThread(0);
#else
   pthread_exit(0); 
#endif


}


#ifdef MSVC
DWORD WINAPI computeNSSModelThread(LPVOID ptr) {
#else
void* computeNSSModelThread(void* ptr) {
#endif
  int step = 0;
  int tot = 0;
  int offset = -1;
  thdata *data;            
  int inc = 1;

  data = (thdata *) ptr; 
  inc = data->ipro->max_threads_;
  offset = data->thread_no;
  step = data->ipro->hit_arr_->length()/100;

  data->ipro->nss_model_->clear();
  double max = -100000;
  for (size_t i=0; i < (size_t)data->ipro->hit_arr_->length() ; i++) {
    data->ipro->nss_model_->insert((*data->ipro->hit_arr_)[i]->adj_prob_, (*data->ipro->hit_arr_)[i]->nss_);
    if ( (*data->ipro->hit_arr_)[i]->nss_ > max || i == 0) {
      max = (*data->ipro->hit_arr_)[i]->nss_;
    }
  }
  
  if (max <= 0) max = 2; 
  if (data->ipro->nss_model_->makeReady(true, 2)) {  //  if (nss_model_->makeReady(1, 1)) {
    //getNSSAdjProbs();
  }

#ifdef MSVC
   ExitThread(0);
#else
   pthread_exit(0); 
#endif

}


#ifdef MSVC
DWORD WINAPI computeNRSModelThread(LPVOID ptr) {
#else
void* computeNRSModelThread(void* ptr) {
#endif
  int step = 0;
  int tot = 0;
  int offset = -1;
  thdata *data;            
  int inc = 1;

  data = (thdata *) ptr; 
  inc = data->ipro->max_threads_;
  offset = data->thread_no;
  step = data->ipro->hit_arr_->length()/100;

  data->ipro->nrs_model_->clear();

  for (int i=0; i < data->ipro->hit_arr_->length() ; i++) {
    //if ((*hit_arr_)[i]->spect_ == "OR20070625_HS_L-H-1-1_12.07860.07860.2") {
    //  cerr << "Here now!" << endl;
    //}
    if ((*data->ipro->hit_arr_)[i]->nrs_ > -1000000)
      data->ipro->nrs_model_->insert((*data->ipro->hit_arr_)[i]->adj_prob_, (*data->ipro->hit_arr_)[i]->nrs_);
  }

  if(data->ipro->nrs_model_->makeReady(false, 10)) {  //  if(nrs_model_->makeReady(5,5)) {
    //getNRSAdjProbs();
  }
#ifdef MSVC
   ExitThread(0);
#else
   pthread_exit(0); 
#endif

}


#ifdef MSVC
DWORD WINAPI computeNSEModelThread(LPVOID ptr) {
#else
void* computeNSEModelThread(void* ptr) {
#endif
  int step = 0;
  int tot = 0;
  int offset = -1;
  thdata *data;            
  int inc = 1;

  data = (thdata *) ptr; 
  inc = data->ipro->max_threads_;
  offset = data->thread_no;
  step = data->ipro->hit_arr_->length()/100;

  data->ipro->nse_model_->clear();
  for (int i=0; i < data->ipro->hit_arr_->length() ; i++) {

    if ((*data->ipro->hit_arr_)[i]->nse_ > -1000000)
      data->ipro->nse_model_->insert((*data->ipro->hit_arr_)[i]->adj_prob_, (*data->ipro->hit_arr_)[i]->nse_);
  }
  
  if (data->ipro->nse_model_->makeReady(false, 10)) {  //  if (nse_model_->makeReady(1,1)) {
    // getNSEAdjProbs();
  }
#ifdef MSVC
   ExitThread(0);
#else
   pthread_exit(0); 
#endif

}


#ifdef MSVC
DWORD WINAPI computeNSIModelThread(LPVOID ptr) {
#else
void* computeNSIModelThread(void* ptr) {
#endif
  int step = 0;
  int tot = 0;
  int offset = -1;
  thdata *data;            
  int inc = 1;

  data = (thdata *) ptr; 
  inc = data->ipro->max_threads_;
  offset = data->thread_no;
  step = data->ipro->hit_arr_->length()/100;

  data->ipro->nsi_model_->clear();
  for (int i=0; i < data->ipro->hit_arr_->length() ; i++) {
    if ((*data->ipro->hit_arr_)[i]->nsi_ > -1000000)
      data->ipro->nsi_model_->insert((*data->ipro->hit_arr_)[i]->adj_prob_, (*data->ipro->hit_arr_)[i]->nsi_);
  }
  if (data->ipro->nsi_model_->makeReady(false, 5)) {  //  if (nsi_model_->makeReady(1, 1)) {
    //getNSIAdjProbs();
  }
#ifdef MSVC
   ExitThread(0);
#else
   pthread_exit(0); 
#endif

}


#ifdef MSVC
DWORD WINAPI computeNSMModelThread(LPVOID ptr) {
#else
void* computeNSMModelThread(void* ptr) {
#endif
  int step = 0;
  int tot = 0;
  int offset = -1;
  thdata *data;            
  int inc = 1;

  data = (thdata *) ptr; 
  inc = data->ipro->max_threads_;
  offset = data->thread_no;
  step = data->ipro->hit_arr_->length()/100;

  data->ipro->nsm_model_->clear();
  for (int i=0; i < data->ipro->hit_arr_->length() ; i++) {      
    data->ipro->nsm_model_->insert((*data->ipro->hit_arr_)[i]->adj_prob_, (*data->ipro->hit_arr_)[i]->nsm_);
  }
  if (data->ipro->nsm_model_->makeReady(false, 10)) {  //  if (nsm_model_->makeReady(1, 1)) {
    //getNSMAdjProbs();get
  }
#ifdef MSVC
   ExitThread(0);
#else
   pthread_exit(0); 
#endif

}


#ifdef MSVC
DWORD WINAPI computeNSPModelThread(LPVOID ptr) {
#else
void* computeNSPModelThread(void* ptr) {
#endif
  int step = 0;
  int tot = 0;
  int offset = -1;
  thdata *data;            
  int inc = 1;

  data = (thdata *) ptr; 
  inc = data->ipro->max_threads_;
  offset = data->thread_no;
  step = data->ipro->hit_arr_->length()/100;

  data->ipro->nsp_model_->clear();
  for (int i=0; i < data->ipro->hit_arr_->length() ; i++) {      
    data->ipro->nsp_model_->insert((*data->ipro->hit_arr_)[i]->adj_prob_, (*data->ipro->hit_arr_)[i]->nsp_);
  }
  if (data->ipro->nsp_model_->makeReady(false, 10)) {  //  if (nsp_model_->makeReady(1, 1)) {
    //getNSPAdjProbs();get
  }
#ifdef MSVC
   ExitThread(0);
#else
   pthread_exit(0); 
#endif

}



#ifdef MSVC
DWORD WINAPI computeLengthModelThread(LPVOID ptr) {
#else
void* computeLengthModelThread(void* ptr) {
#endif
  int step = 0;
  int tot = 0;
  int offset = -1;
  thdata *data;            
  int inc = 1;

  data = (thdata *) ptr; 
  inc = data->ipro->max_threads_;
  offset = data->thread_no;
  step = data->ipro->hit_arr_->length()/100;

  data->ipro->len_model_->clear();
  for (int i=0; i < data->ipro->hit_arr_->length() ; i++) {      
    data->ipro->len_model_->insert((*data->ipro->hit_arr_)[i]->adj_prob_, (*data->ipro->hit_arr_)[i]->peptide_.length() < 15 ? (*data->ipro->hit_arr_)[i]->peptide_.length() : 15 );
  }
  if (data->ipro->len_model_->makeReady(false, 10)) {  //  if (nsp_model_->makeReady(1, 1)) {
    //getNSPAdjProbs();get
  }
#ifdef MSVC
   ExitThread(0);
#else
   pthread_exit(0); 
#endif

}

#ifdef MSVC
DWORD WINAPI computeFPKMModelThread(LPVOID ptr) {
#else
void* computeFPKMModelThread(void* ptr) {
#endif
  int step = 0;
  int tot = 0;
  int offset = -1;
  thdata *data;            
  int inc = 1;

  data = (thdata *) ptr; 
  inc = data->ipro->max_threads_;
  offset = data->thread_no;
  step = data->ipro->hit_arr_->length()/100;

  data->ipro->fpkm_model_->clear();
  for (int i=0; i < data->ipro->hit_arr_->length() ; i++) {      
   if ( (*data->ipro->hit_arr_)[i]->fpkm_ > 20) {
      (*data->ipro->hit_arr_)[i]->fpkm_ = 20;
    }
    data->ipro->fpkm_model_->insert((*data->ipro->hit_arr_)[i]->adj_prob_, (*data->ipro->hit_arr_)[i]->fpkm_);
  }
  if (data->ipro->fpkm_model_->makeReady(false, 10)) {  //  if (fpkm_model_->makeReady(1, 1)) {
    //getFPKMAdjProbs();get
  }
#ifdef MSVC
   ExitThread(0);
#else
   pthread_exit(0); 
#endif

}


#ifdef MSVC
DWORD WINAPI computeCATModelThread(LPVOID ptr) {
#else
void* computeCATModelThread(void* ptr) {
#endif
  int step = 0;
  int tot = 0;
  int offset = -1;
  thdata *data;            
  int inc = 1;

  data = (thdata *) ptr; 
  inc = data->ipro->max_threads_;
  offset = data->thread_no;
  step = data->ipro->hit_arr_->length()/100;

  data->ipro->cat_model_->clear();
  for (int i=0; i < data->ipro->hit_arr_->length() ; i++) {
    data->ipro->cat_model_->insert((*data->ipro->hit_arr_)[i]->adj_prob_, data->ipro->isInTopCat((*data->ipro->hit_arr_)[i]));
  }
  
  if (data->ipro->cat_model_->makeReady()) {  //  if (nse_model_->makeReady(1,1)) {
    // getNSEAdjProbs();
  }
#ifdef MSVC
   ExitThread(0);
#else
   pthread_exit(0); 
#endif

}


 InterProphet::InterProphet(bool nss_flag, bool nrs_flag, bool nse_flag, bool sharp_nse, bool nsi_flag, bool nsm_flag, bool nsp_flag, bool use_fpkm, bool use_length, bool cat_flag, int max_threads) {
  max_threads_ = max_threads;

  
  sharp_nse_ = sharp_nse;

  use_fpkm_ = use_fpkm;
  use_nss_ = nss_flag;
  use_nrs_ = nrs_flag;
  use_nse_ = nse_flag;
  use_nsi_ = nsi_flag;
  use_nsm_ = nsm_flag;
  use_nsp_ = nsp_flag;
  use_cat_ = cat_flag;
  use_length_ = use_length;


  nss_model_ = new KDModel("NSS");
  nrs_model_ = new KDModel("NRS");
  nse_model_ = new KDModel("NSE");
  nsi_model_ = new KDModel("NSI");
  nsm_model_ = new KDModel("NSM");
  nsp_model_ = new KDModel("NSP");
  len_model_ = new KDModel("LENGTH");
  fpkm_model_ = new KDModel("FPKM");

  
  cat_model_ = new BoolModel("TopCat");

  num_cats_ = 0;

  nss_cnt_ = 0;
  nrs_cnt_ = 0;
  nse_cnt_ = 0;
  nsi_cnt_ = 0;
  nsm_cnt_ = 0;
  nsp_cnt_ = 0;

  allprobs_ = new Array<double>();
  
  maxprob_hash_ = new dbl_hash();

  maxprob_sp_hash_ = new dbl_hash();
  
  search_names_ = new Array<string*>();
  
  top_i_hash_ = new int_hash();

  bypep_tophit_hash_ = new arrhit_hash();

  byprot_tophit_hash_ = new arrhit_hash();


  byprot_tophit_hashes_ = new Array<arrhit_hash*>();


  hits_hash_ = new Array<hit_hash*>();

  hits_hashes_ = new Array<Array<hit_hash*>*>();

  bypep_nsp_hash_ = new Array<dbl_hash*>();

  
  for (int a = 0; a < max_threads_; a++) {
    byprot_tophit_hashes_->insertAtEnd(new arrhit_hash());
    hits_hashes_->insertAtEnd(new Array<hit_hash*>());
    bypep_nsp_hash_->insertAtEnd(new dbl_hash());
  }


  decoy_hits_hash_ = new Array<bool_hash*>();
  hit_arr_ = new Array<SearchHit*>();
  decoy_hit_arr_ = new Array<bool>();
  num_runs_ = 0;
  new_prob_tot_ = 0;
  max_pep_prob_tot_ = 0;
  max_spec_prob_tot_ = 0;
  old_prob_tot_ = 0;

  swath_mode_ = true;
  iRT_sum_ = 0;
  exp_hash_ = new int_hash();
  if (use_cat_) {
    cat_index_hash_ = new int_hash();
    byrun_topcat_hash_ = new int_hash();
    bypep_cat_hash_ = new intarr_hash();
    catsarr_byrun_hash_ = new dblarr_hash();
  }
}  


void InterProphet::computeModels() {


    
  findBestMatches();
  buildAllRunHitArray();
 
  if ( use_nss_ || use_nrs_ || use_nse_ || use_nsi_ || use_nsm_ || use_nsp_ || use_fpkm_ || use_length_ || use_cat_) {
   
    cerr << "Running " ;

    if(use_nss_)
      cerr << "NSS ";
  
    if(use_nrs_)
      cerr << "NRS ";

    if(use_nse_)
      cerr << "NSE ";

    if(use_nsi_)
      cerr << "NSI ";

    if(use_nsm_)
      cerr << "NSM ";

    if(use_nsp_)
      cerr << "NSP ";

    if(use_fpkm_)
      cerr << "FPKM ";

    if(use_length_)
      cerr << "LENGTH ";


    if(use_cat_)
      cerr << "CAT ";
      
      
    cerr <<"Model EM:"<< endl;

    size_t itr_count = 0;
    int done_count = 0;
    bool not_done = true;
    last_iter_ = false;

    while(not_done && itr_count < 50) {
      if (itr_count <= 0) {

	if (use_nss_) {
	  cerr << "Computing NSS values ... " << endl; 
	  getNSSCounts();
	  cerr << " done" << endl;
	}

	if (use_nrs_) {
	  cerr << "Computing NRS values ... " << endl; 
	  getNRSCounts();
	  cerr << " done" << endl;
	}
      
	if (use_nse_) {
	  cerr << "Computing NSE values ... " << endl; 
	  getNSECounts();
	  cerr << " done" << endl;
	}
	

	if (use_nsi_) {
	  cerr << "Computing NSI values ... " << endl; 
	  getNSICounts();
	  cerr << " done" << endl;
	}

	if (use_nsm_) {
	  cerr << "Computing NSM values ... " << endl; 
	  getNSMCounts();
	  cerr << " done" << endl;
	}

	if (use_nsp_) {
	  cerr << "Computing NSP values ... " << endl; 
	  getNSPCounts();
	  cerr << " done" << endl;
	}
	
	if (use_length_) {
	  cerr << "Using Peptide Lengths ... " << endl; 
	  //getPeptideLengths();
	  cerr << " done " << endl;
	}

	if (fpkm_cnt_ <= 0.) { 
	  use_fpkm_ = false;
	  cerr << "FPKM values are unavailable ... " << endl; 
	}
	else if (use_fpkm_) {
	  cerr << "FPKM values are available, to disable rerun with option NOFPKM ... " << endl; 
	}
      

	//	if (use_fpkm_) {
	//  cerr << "Computing NSP values ... " << endl; 
	//  getFPKMCounts();
	//  cerr << " done" << endl;
	//}

	if (use_cat_) {
	  cerr << "Computing CAT values ... " << endl; 
	  findRunTopCats();
	  cerr << " done" << endl;
	}


	old_prob_tot_ = 0;
	cerr << "Iterations: " ; 
      }

      if (use_nss_) {

	computeNSSModel();
      }

      if (use_nrs_) {
	computeNRSModel();
      }
      
      if (use_nse_) {
	computeNSEModel();
      }
       

      if (use_nsi_) {
	computeNSIModel();
      }
      
      if (use_nsm_) {
	computeNSMModel();
      }

      if (use_nsp_) {
	computeNSPModel();
      }

      if (use_fpkm_) {
	computeFPKMModel();
      }

      if (use_length_) {
	computeLengthModel();
      }

      if (use_cat_) {
	computeCatModel();
      }
      
      not_done = getAllModelAdjProbs(use_nss_, use_nrs_, use_nse_, use_nsi_, use_nsm_, use_nsp_, use_fpkm_, use_length_, use_cat_);
      old_prob_tot_ = new_prob_tot_;
      
      if (++itr_count % 10 == 0) {
	cerr << itr_count;
      }
      else {
	cerr << ".";
      }
    }
    last_iter_ = true;
    getAllModelAdjProbs(use_nss_, use_nrs_, use_nse_,  use_nsi_, use_nsm_, use_nsp_, use_fpkm_, use_length_, use_cat_);

    cerr << "done" << endl;
    
    

    if (use_nss_) {
      nss_model_->clearKernels();
    }
    
    if (use_nrs_) {
      nrs_model_->clearKernels();
    }
    
    if (use_nse_) {
      nse_model_->clearKernels();
    }
  
    if (use_nsi_) {
      nsi_model_->clearKernels();
    }
    
    if (use_nsm_) {
      nsm_model_->clearKernels();
    }
    
    if (use_nsp_) {
      nsp_model_->clearKernels();
    }

    if (use_length_) {
      len_model_->clearKernels();
    }
     
    if (use_fpkm_) {
      fpkm_model_->clearKernels();
    }

  }



  findBestMatches();
 
}




void InterProphet::computeModelsThreaded() {


    
  findBestMatches();
  buildAllRunHitArray();
 
  if (use_nss_ || use_nrs_ || use_nse_ || use_nsi_ || use_nsm_ || use_nsp_ || use_cat_ || use_fpkm_ || use_length_) {
   
    cerr << "Running " ;
    
    if(use_fpkm_)
      cerr << "FPKM ";

    if(use_length_)
      cerr << "LENGTH ";

    if(use_nss_)
      cerr << "NSS ";
  
    if(use_nrs_)
      cerr << "NRS ";

    if(use_nse_)
      cerr << "NSE ";

    if(use_nsi_)
      cerr << "NSI ";

    if(use_nsm_)
      cerr << "NSM ";

    if(use_nsp_)
      cerr << "NSP ";

    if(use_cat_)
      cerr << "CAT ";
      
      
    cerr <<"Model EM:"<< endl;

    size_t itr_count = 0;
    int done_count = 0;
    bool not_done = true;
    last_iter_ = false;

    unsigned long a = 0;
#ifdef MSVC
    DWORD *pId = new DWORD[max_threads_];
    HANDLE *pHandle = new HANDLE[max_threads_];
#else
    int *pId = new int[max_threads_];
    int *pHandle = new int[max_threads_];
    pthread_t pThreads[max_threads_];
#endif

    thdata data[max_threads_];


    while(not_done && itr_count < 50) {
      if (itr_count <= 0) {
	if (use_nss_) {
	  cerr << "Computing NSS values ... " << endl; 
	  getNSSCounts();
	  cerr << " done" << endl;
	}
	
	if (use_nrs_) {
	  cerr << "Computing NRS values ... " << endl; 
	  getNRSCounts();
	  cerr << " done" << endl;
	}
	
	if (use_nse_) {
	  cerr << "Computing NSE values ... " << endl; 
	  getNSECounts();
	  cerr << " done" << endl;
	}

	
	if (use_nsi_) {
	  cerr << "Computing NSI values ... " << endl; 
	  getNSICounts();
	  cerr << " done" << endl;
	}
	
	if (use_nsm_) {
	  cerr << "Computing NSM values ... " << endl; 
	  getNSMCounts();
	  cerr << " done" << endl;
	}
	
	if (use_nsp_) {
	  cerr << "Computing NSP values ... " << endl; 
	  getNSPCounts();
	  cerr << " done" << endl;
	}
		
	if (use_length_) {
	  cerr << "Computing Peptide Lengths ... " << endl; 
	  //getPeptideLengths();
	  cerr << " done" << endl;
	}

	if (use_cat_) {
	  cerr << "Computing CAT values ... " << endl; 
	  findRunTopCats();
	  cerr << " done" << endl;
	}
	
	if (fpkm_cnt_ <= 0) {
	  use_fpkm_ = false;
	  cerr << "FPKM values are unavailable ... " << endl; 
	}
	else if (use_fpkm_) {
	  cerr << "FPKM values are available, to disable rerun with option NOFPKM ... " << endl; 
	}
	old_prob_tot_ = 0;
	cerr << "Iterations: " ; 
      }
      int liveThreads = 0;
      int maxThreads = max_threads_;
      vector<int> threads;
      a = 0;

      bool more_models = true;

      bool done_nss = false;
      bool done_nrs = false;
      bool done_nse = false;
      bool done_nsi = false;
      bool done_nsm = false;
      bool done_nsp = false;
      bool done_cat = false;
      bool done_fpkm = false;
      bool done_len = false;

      while (!(
	       done_nss && 
	       done_nrs && 
	       done_nse && 
	       done_nsi && 
	       done_nsm && 
	       done_nsp && 
	       done_cat && 
	       done_fpkm &&
	       done_len )
	     ) { 

	while(!(
	       done_nss && 
	       done_nrs && 
	       done_nse && 
	       done_nsi && 
	       done_nsm && 
	       done_nsp && 
	       done_cat && 
	       done_fpkm &&
	       done_len ) && liveThreads < maxThreads) {

	  more_models = false;

	  data[liveThreads].thread_no = liveThreads;
	  data[liveThreads].ipro = this;
	  
	  if (use_nss_ && !done_nss) {
	    done_nss = true;
#ifdef MSVC
	    pHandle[liveThreads] = CreateThread(NULL,0,computeNSSModelThread,(void*) &data[liveThreads],0, NULL);
#else
	    pthread_create(&pThreads[liveThreads],NULL,computeNSSModelThread, (void*) &data[liveThreads]);
#endif
	    liveThreads ++;
	    continue;
	  }
	  else if (!use_nss_) {
	    done_nss = true;
	    
	  }

	  if (use_nrs_  && !done_nrs) {
	    done_nrs = true;
#ifdef MSVC
	    pHandle[liveThreads] = CreateThread(NULL,0,computeNRSModelThread,(void*) &data[liveThreads],0, NULL);
#else
	    pthread_create(&pThreads[liveThreads],NULL,computeNRSModelThread, (void*) &data[liveThreads]);
#endif
	    liveThreads ++;
	    continue;
	    
	  } 
	  else if (!use_nrs_) {
	    done_nrs = true;
	    
	  }

	  if (use_nse_ && !done_nse) {
	    done_nse = true;
#ifdef MSVC
	    pHandle[liveThreads] = CreateThread(NULL,0,computeNSEModelThread,(void*) &data[liveThreads], 0, NULL);
#else
	    pthread_create(&pThreads[liveThreads],NULL,computeNSEModelThread, (void*) &data[liveThreads]);
#endif
	    liveThreads ++;
	    continue;
	    
	  }
	  else if (!use_nse_) {
	    done_nse = true;
	    
	  }

	  if (use_nsi_ && !done_nsi) {
	    done_nsi = true;
#ifdef MSVC
	    pHandle[liveThreads] = CreateThread(NULL,0,computeNSIModelThread,(void*) &data[liveThreads],0, NULL);
#else
	    pthread_create(&pThreads[liveThreads],NULL,computeNSIModelThread, (void*) &data[liveThreads]);
#endif
	    liveThreads ++;
	    continue;
	    
	  }
	  else if (!use_nsi_) {
	    done_nsi = true;
	    
	  }

	  if (use_nsm_ && !done_nsm) {
	    done_nsm = true;
#ifdef MSVC
	    pHandle[liveThreads] = CreateThread(NULL,0,computeNSMModelThread,(void*) &data[liveThreads],0, NULL);
#else
	    pthread_create(&pThreads[liveThreads],NULL,computeNSMModelThread, (void*) &data[liveThreads]);
#endif
	    liveThreads ++;
	    continue;
	    
	  }
	  else if (!use_nsm_) {
	    done_nsm = true;
	    
	  }

	  if (use_nsp_ && !done_nsp) {
	     done_nsp = true;
#ifdef MSVC
	    pHandle[liveThreads] = CreateThread(NULL,0,computeNSPModelThread,(void*) &data[liveThreads],0, NULL);
#else
	    pthread_create(&pThreads[liveThreads],NULL,computeNSPModelThread, (void*) &data[liveThreads]);
#endif
	    liveThreads ++;
	    continue;
	    
	  }
	  else if (!use_nsp_) {
	    done_nsp = true;
	    
	  }
	  
	  if (use_cat_ && !done_cat) {
	    done_cat = true;
#ifdef MSVC
	    pHandle[liveThreads] = CreateThread(NULL,0,computeCATModelThread,(void*) &data[liveThreads],0, NULL);
#else
	    pthread_create(&pThreads[liveThreads],NULL,computeCATModelThread, (void*) &data[liveThreads]);
#endif
	    liveThreads ++;	
	    
	  }
	  else if (!use_cat_) {
	    done_cat = true;
	    
	  }
	  
	  if (use_fpkm_ && !done_fpkm ) {
	    done_fpkm = true;
#ifdef MSVC
	    pHandle[liveThreads] = CreateThread(NULL,0,computeFPKMModelThread,(void*) &data[liveThreads],0, NULL);
#else
	    pthread_create(&pThreads[liveThreads],NULL,computeFPKMModelThread, (void*) &data[liveThreads]);
#endif
	    liveThreads ++;
	    continue;
	    
	  }
	  else if (!use_fpkm_) {
	    done_fpkm = true;
	    
	  }

	  if (use_length_ && !done_fpkm) {
	    done_len = true;
#ifdef MSVC
	    pHandle[liveThreads] = CreateThread(NULL,0,computeLengthModelThread,(void*) &data[liveThreads],0, NULL);
#else
	    pthread_create(&pThreads[liveThreads],NULL,computeLengthModelThread, (void*) &data[liveThreads]);
#endif
	    liveThreads ++;
	    continue;
	    
	  }
	  else if (!use_length_) {
	     done_len = true;
	    
	  }

	}
	
	
	a = 0;
	while(a < liveThreads)  {
#ifdef MSVC
	  WaitForSingleObject(pHandle[a],INFINITE);
#else
	  pthread_join(pThreads[a],NULL);
#endif
	  a++;
	}
	liveThreads = 0;
      }
      

      not_done = getAllModelAdjProbs(use_nss_, use_nrs_, use_nse_, use_nsi_, use_nsm_, use_nsp_, use_fpkm_, use_length_, use_cat_);
      old_prob_tot_ = new_prob_tot_;
      
      if (++itr_count % 10 == 0) {
	cerr << itr_count;
      }
      else {
	cerr << ".";
      }
    }
    last_iter_ = true;
    getAllModelAdjProbs(use_nss_, use_nrs_, use_nse_, use_nsi_, use_nsm_, use_nsp_, use_fpkm_, use_length_, use_cat_);
    
    cerr << "done" << endl;
    
    

    if (use_nss_) {
      nss_model_->clearKernels();
    }
    
    if (use_nrs_) {
      nrs_model_->clearKernels();
    }
    
    if (use_nse_) {
      nse_model_->clearKernels();
    }
  
    if (use_nsi_) {
      nsi_model_->clearKernels();
    }
    
    if (use_nsm_) {
      nsm_model_->clearKernels();
    }
    
    if (use_nsp_) {
      nsp_model_->clearKernels();
    }

    if (use_length_) {
      len_model_->clearKernels();
    }

    if (use_fpkm_) {
      fpkm_model_->clearKernels();
    }
      
	delete [] pId;	
	delete [] pHandle;  
  }
 
  findBestMatches();
}

void InterProphet::addPeptideCategory(string& pep, string& cat) {
  string * key = new string(pep);
  string * val = new string(cat);

   // Isoleucine -> Leu for comparison purposes
  for (int i=0; i<(int)(*key).size(); i++) {
    (*key)[i] = toupper((*key)[i]);
    if ((*key)[i] == 'I') {
      (*key)[i] = 'L';
    }
  }


  int idx = 0;

  int_hash::iterator itr = cat_index_hash_->find(*val);
 
  if (itr ==  cat_index_hash_->end()) {
    cat_index_hash_->insert(make_pair(*val, num_cats_));
    cerr << *val << "\t" << num_cats_ << endl;
    idx = num_cats_;
    num_cats_++;
  }
  else {
    idx = (*cat_index_hash_)[*val];
    delete val;
  }

  intarr_hash::iterator itr2 = bypep_cat_hash_->find(*key);

  if (itr2 ==  bypep_cat_hash_->end()) {
    bypep_cat_hash_->insert(make_pair(*key, new Array<int>()));
    
  }

  (*bypep_cat_hash_)[*key]->insertAtEnd(idx);
  

}



void InterProphet::insertResult(const int run_idx, string& spectrum, 
				double prob, Array<double>* allntt_prob, 
				string& pepseq, string& modpep, string& swathpep,
				double calcnmass, string& exp, 
				string& charge,  Array<string*>* prots, 
				Boolean is_decoy, double maxFPKM, double emp_iRT, int swath_window, int alt_swath) {
  if (prob < 0)  {
    delete allntt_prob;
    return;
  }
 
  //prob -= 0.10; // Adjust input prob
  if (prob > MAX_PROB) {
    prob = MAX_PROB;
  }
  if (prob < 1-MAX_PROB) {
    prob = 0;
  }
  

  swath_mode_ =  swath_mode_ && (swathpep != "");

  swathpep = swathpep.substr(0, swathpep.find(":"));
  
  double tprob = prob;
  for (size_t i =0; i<3; i++) {
    if ((*allntt_prob)[i] > MAX_PROB) {
      (*allntt_prob)[i] = MAX_PROB;
    }
  }
  
  string* key = new string(exp);

  if (exp_hash_->find(*key) == exp_hash_->end()) {
    exp_hash_->insert(make_pair(*key,1));
  }

  *key += spectrum + swathpep;


  string* msrunstr = new string(*key);

  size_t pos = msrunstr->find_first_of('.');
  
  if (pos != msrunstr->npos) {
      msrunstr->erase(pos);
  }
 
  // Isoleucine -> Leu for comparison purposes
  for (int i=0; i<(int)pepseq.size(); i++) {
    pepseq[i] = toupper(pepseq[i]);
    if (pepseq[i] == 'I') {
      pepseq[i] = 'L';
    }
  }

  

  SearchHit* hit = new SearchHit(run_idx, spectrum, 
				 prob, allntt_prob, 
				 pepseq, modpep, swathpep,
				 *msrunstr, calcnmass, emp_iRT, swath_window, alt_swath, exp, charge, prots);

  //DDS: ASSUME no real iRT value is so negative
  iRT_sum_ += emp_iRT;
  


  if (maxFPKM > 20) {
    maxFPKM = 20;
  }
  
  hit->setMaxFPKM(maxFPKM);

  fpkm_cnt_ += maxFPKM;

  (*hits_hash_)[run_idx]->insert(make_pair(*key, hit));

  for (int a=0; a<max_threads_; a++) {
    
    (*(*hits_hashes_)[a])[run_idx]->insert(make_pair(*key, hit));
    
  }
  

  (*decoy_hits_hash_)[run_idx]->insert(make_pair(*key, (bool)is_decoy));

}

//int InterProphet::getSpectrumCharge(string& spectrum) {
//  return atoi(&(spectrum.c_str()[(int)(spectrum.length()-1)]));
//}

void InterProphet::addSearch(string*& name) {

  
  hits_hash_->insertAtEnd(new hit_hash());
  for (int a=0; a<max_threads_; a++) {
    
    (*hits_hashes_)[a]->insertAtEnd(new hit_hash());
    
  }
  
  decoy_hits_hash_->insertAtEnd(new bool_hash());
  num_runs_++;
  
  search_names_->insertAtEnd(name);
}

void InterProphet::buildAllModelHitArray() {
  hit_arr_->clear();
  for (int_hash::iterator i_itr = (*top_i_hash_).begin(); i_itr != (*top_i_hash_).end(); i_itr++) {
    hit_hash::iterator h_itr = (*(*hits_hash_)[i_itr->second]).find(i_itr->first);

    hit_arr_->insertAtEnd(h_itr->second);
  }
}

void InterProphet::buildAllRunHitArray() {
  hit_arr_->clear();
  decoy_hit_arr_->clear();
  for (size_t i=0; i<num_runs_; i++) {

    for (hit_hash::iterator i_itr = (*(*hits_hash_)[i]).begin(); 
	 i_itr != (*(*hits_hash_)[i]).end() ; 
	 i_itr++) {
      hit_arr_->insertAtEnd(i_itr->second);
    }

    for (bool_hash::iterator b_itr = (*(*decoy_hits_hash_)[i]).begin();  b_itr!= (*(*decoy_hits_hash_)[i]).end(); b_itr++) {
    
      decoy_hit_arr_->insertAtEnd(b_itr->second);
   
    }
  }
}
      

//find the best match among all search engines
void InterProphet::findBestMatches() {
  top_i_hash_->clear();
  bypep_tophit_hash_->clear();
  byprot_tophit_hash_->clear();

  for (int a = 0; a < max_threads_; a++) {
    (*byprot_tophit_hashes_)[a]->clear();

  }
  
  for (size_t i=0; i<num_runs_; i++) {

    for (hit_hash::iterator i_itr = (*(*hits_hash_)[i]).begin(); i_itr != (*(*hits_hash_)[i]).end(); i_itr++) {

      if ((*top_i_hash_).find(i_itr->first) == (*top_i_hash_).end()) {

	int top_i = (*top_i_hash_)[i_itr->first] = computeBestMatch(i_itr->second->exp_,i_itr->second->spect_,i_itr->second->swathpep_);
	

	arrhit_hash::iterator itr = bypep_tophit_hash_->find((*(*hits_hash_)[top_i])[i_itr->first]->peptide_);

	if (itr == bypep_tophit_hash_->end()) {
	  Array<SearchHit*>* hit_arr = new Array<SearchHit*>();
	  bypep_tophit_hash_->insert(make_pair((*(*hits_hash_)[top_i])[i_itr->first]->peptide_, hit_arr));
	}
	(*bypep_tophit_hash_)[(*(*hits_hash_)[top_i])[i_itr->first]->peptide_]->insertAtEnd((*(*hits_hash_)[top_i])[i_itr->first]);

	for (size_t j=0; j<(*(*hits_hash_)[top_i])[i_itr->first]->prots_->size(); j++) {
	  itr = byprot_tophit_hash_->find(*(*(*(*hits_hash_)[top_i])[i_itr->first]->prots_)[j]);

	  if (itr == byprot_tophit_hash_->end()) {
	    Array<SearchHit*>* hit_arr = new Array<SearchHit*>();
	    byprot_tophit_hash_->insert(make_pair(*(*(*(*hits_hash_)[top_i])[i_itr->first]->prots_)[j], hit_arr));
	    
	    for (int a = 0; a < max_threads_; a++) {
	      (*byprot_tophit_hashes_)[a]->insert(make_pair(*(*(*(*hits_hash_)[top_i])[i_itr->first]->prots_)[j], hit_arr));
	      
	    }
	  }
	  (*byprot_tophit_hash_)[*(*(*(*hits_hash_)[top_i])[i_itr->first]->prots_)[j]]->insertAtEnd((*(*hits_hash_)[top_i])[i_itr->first]);

	  for (int a = 0; a < max_threads_; a++) {
	    (*(*byprot_tophit_hashes_)[a])[*(*(*(*hits_hash_)[top_i])[i_itr->first]->prots_)[j]]->insertAtEnd((*(*hits_hash_)[top_i])[i_itr->first]);
	      
	  }
	}



      }
    }
  }
}


 int InterProphet::getBestMatch(string& exp_lbl, string& spec, string& swath) {
  int out = -1;
  string key = exp_lbl + spec + swath;

  int_hash::iterator itr = (*top_i_hash_).find(key);
  if (itr != (*top_i_hash_).end()) {
    out = itr->second;
  }
  return out;
}

void InterProphet::computeTopProbs() {
  
  for (int_hash::iterator i_itr = (*top_i_hash_).begin(); i_itr != (*top_i_hash_).end(); i_itr++) {
    
    string spec_name(i_itr->first);

    //if (spec_name == "OR20070625_HS_L-H-1-1_12.07860.07860.2") {
    //  cerr << "Here now!" << endl;
    //}

    int chg = atoi(&spec_name[spec_name.length()-1]);
    spec_name.erase(spec_name.length()-1, 1);
    
    if (chg == 2) {
      chg = 3;
    }
    else if (chg == 3) {
      chg = 2;
    }

    if (chg == 2 || chg == 3)    {  
      // append partner charge
      ostringstream converter;
      converter << chg;
      spec_name += converter.str();
    }

    int_hash::iterator j_itr = (*top_i_hash_).find(spec_name);
    if (j_itr != (*top_i_hash_).end()) {
      (*(*hits_hash_)[i_itr->second])[i_itr->first]->adj_prob_  = adjDoubleTripleCharge( (*(*hits_hash_)[i_itr->second])[i_itr->first]->adj_prob_,  (*(*hits_hash_)[(*top_i_hash_)[j_itr->first]])[j_itr->first]->adj_prob_);
      for (int n=0; n<3; n++) {
	(*(*hits_hash_)[i_itr->second])[i_itr->first]->allntt_adj_prob_[n]  = adjDoubleTripleCharge( (*(*hits_hash_)[i_itr->second])[i_itr->first]->allntt_adj_prob_[n],  (*(*hits_hash_)[(*top_i_hash_)[j_itr->first]])[j_itr->first]->allntt_adj_prob_[n]);
      }
    }

    //dblarr_hash::iterator n_itr = (*top_allntt_prob_hash_).find(spec_name);
    //if (n_itr != (*top_allntt_prob_hash_).end()) {
    //  for (int n=0; n<3; n++) {
    //	(*(*top_allntt_prob_hash_)[i_itr->first])[n]  = adjDoubleTripleCharge((*(*top_allntt_prob_hash_)[i_itr->first])[n], (*n_itr->second)[n]);
    //  }
    //}
  }

}

double InterProphet::adjDoubleTripleCharge(double prob_2_adj, double prob_of_partner) {
  if(prob_2_adj == 0 || prob_2_adj + prob_of_partner == 0) {
    return 0.0;
  }
  if(prob_2_adj == 1 && prob_of_partner == 1) {
    return 0.5;
  }
  return (prob_2_adj * (1-prob_of_partner) / (1-prob_2_adj*prob_of_partner));

}

int InterProphet::computeBestMatch(string& exp_lbl, string& spectrum, string& swath_assay  ) {
   
  double top_prob = 0;
  double prob = 0;
  int top_i = -1;
  size_t i = 0 ;
  string key = exp_lbl + spectrum + swath_assay;
  hit_hash::iterator itr;
  for (i=0; i<num_runs_; i++) {
    itr = (*(*hits_hash_)[i]).find(key);
    if (itr != (*(*hits_hash_)[i]).end()) {
      prob = itr->second->adj_prob_;
      if (top_prob <= prob) {
	top_prob = prob;
	top_i = i;
      }
    }
  }

  return top_i;
}

Array<Tag*>* InterProphet::getRocDataPointTags() {
  Array<Tag*>* output = new Array<Tag*>;
  Tag* tag;
  char text[500];

  int total = (*top_i_hash_).size();
  
  double* combinedprobs = new double[total];
  size_t i=0;
  for (int_hash::iterator i_itr = (*top_i_hash_).begin(); i_itr != (*top_i_hash_).end(); i_itr++) {
    combinedprobs[i] = (*(*hits_hash_)[i_itr->second])[i_itr->first]->adj_prob_;
    i++;
  }

  qsort(combinedprobs, total, sizeof(double), (int(*)(const void*, const void*)) comp_nums);

     // now sens and error as walk down list
  double thresh[] = {0.9999, 0.999, 0.99, 0.98, 0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6, 0.55, 0.5, 0.45, 0.4, 0.35, 0.3, 0.25, 0.2, 0.15, 0.1, 0.05, 0.0};
  int threshind = 0;
  double correct = 0.0;
  double incorrect = 0.0;
  double totcorrect = 0.0;
  for(int k = 0; k < total; k++) {
    totcorrect += combinedprobs[k];
  }

  tag = new Tag("roc_error_data", true, false);
  tag->setAttributeValue("charge", "all");
  output->insertAtEnd(tag);

  double grandTotal = totcorrect;
  if(grandTotal > 0.0) {
    for(int k = 0; k < total; k++) {
      double rnd;
      sprintf(text, "%0.4f", combinedprobs[k]);
      rnd = atof(text);
      if(rnd >= thresh[threshind]) {
	correct += combinedprobs[k];
	incorrect += 1 - combinedprobs[k];
      }
      else {
	tag = new Tag("roc_data_point", True, True);
	sprintf(text, "%0.4f", thresh[threshind]);
	tag->setAttributeValue("min_prob", text);
	sprintf(text, "%0.4f", correct/totcorrect);
	tag->setAttributeValue("sensitivity", text);

	if(correct + incorrect > 0.0) {

	  sprintf(text, "%0.4f", incorrect/(correct + incorrect));
	  tag->setAttributeValue("error", text);

	}
	else {
	  sprintf(text, "%0.0f", 0.0);
	  tag->setAttributeValue("error", text);
	}
	sprintf(text, "%0.0f", correct);
	tag->setAttributeValue("num_corr", text);
	sprintf(text, "%0.0f", incorrect);
	tag->setAttributeValue("num_incorr", text);
	output->insertAtEnd(tag);
	threshind++;
	k--; // assay again using next threshold
      }
    } // next member


    tag = new Tag("roc_data_point", True, True);
    sprintf(text, "%0.4f", thresh[threshind]);
    tag->setAttributeValue("min_prob", text);
    sprintf(text, "%0.4f", correct/totcorrect);
    tag->setAttributeValue("sensitivity", text);
    sprintf(text, "%0.4f", incorrect/(correct + incorrect));
    tag->setAttributeValue("error", text);
    sprintf(text, "%0.0f", correct);
    tag->setAttributeValue("num_corr", text);
    sprintf(text, "%0.0f", incorrect);
    tag->setAttributeValue("num_incorr", text);
    output->insertAtEnd(tag);

  } // if have data
  else {
    tag = new Tag("roc_data_point", True, True);
    sprintf(text, "%0.4f", 0.0);
    tag->setAttributeValue("min_prob", text);
    sprintf(text, "%0.4f", 0.0);
    tag->setAttributeValue("sensitivity", text);
    sprintf(text, "%0.4f", 0.0);
    tag->setAttributeValue("error", text);
    sprintf(text, "0.0");
    tag->setAttributeValue("num_corr", text);
    sprintf(text, "0.0");
    tag->setAttributeValue("num_incorr", text);
    output->insertAtEnd(tag);


  }
  if(grandTotal > 0.0) { 
    double error_rates[] = {0.0, 0.0001, 0.0002, 0.0003, 0.0004, 0.0005, 0.0006, 0.0007, 0.0008, 0.0009, 0.001, 0.0015, 0.002, 0.0025, 0.003, 0.004, 0.005, 0.006, 0.007, 0.008, 0.009, 0.01, 0.015, 0.02, 0.025, 0.03, 0.04, 0.05, 0.075, 0.1, 0.15};
    correct = 0.0;
    incorrect = 0.0;
    threshind = 0;
    for(int k = 0; k < total; k++) {
      if(incorrect / (correct + incorrect) > error_rates[threshind]) {
	tag = new Tag("error_point", True, True);
	if(k == 0) {
	  sprintf(text, "%0.4f", error_rates[threshind]);
	  tag->setAttributeValue("error", text);
	  sprintf(text, "%0.4f", combinedprobs[k]);
	  tag->setAttributeValue("min_prob", text);
	  sprintf(text, "%0.0f", correct + combinedprobs[k]);
	  tag->setAttributeValue("num_corr", text);
	  sprintf(text, "%0.0f", incorrect + 1.0 - combinedprobs[k]);
	  tag->setAttributeValue("num_incorr", text);
	}
	else {

	  sprintf(text, "%0.4f", error_rates[threshind]);
	  tag->setAttributeValue("error", text);
	  sprintf(text, "%0.4f", combinedprobs[k-1]);
	  tag->setAttributeValue("min_prob", text);
	  sprintf(text, "%0.0f", correct + combinedprobs[k]);
	  tag->setAttributeValue("num_corr", text);
	  sprintf(text, "%0.0f", incorrect + 1.0 - combinedprobs[k]);
	  tag->setAttributeValue("num_incorr", text);
	}
	output->insertAtEnd(tag);
	threshind++;
	k--;
      }
      else {
	correct += combinedprobs[k];
	incorrect += 1 - combinedprobs[k];
      }
      if(threshind == (sizeof(error_rates)/sizeof(double)) - 1) {
	k = total; // done
      }
    } // next ordered prob
  } // if have data
  
  //finally insert end of roc_error_data tag
  output->insertAtEnd(new Tag("roc_error_data", false, true));
  
  delete [] combinedprobs;
  return output;
}

void InterProphet::findRunTopCats() {
  dblarr_hash::iterator d_itr ;
  //for (size_t i=0; i<num_runs_; i++) {
  //    for (hit_hash::iterator i_itr = (*(*hits_hash_)[i]).begin(); i_itr != (*(*hits_hash_)[i]).end(); i_itr++) {
  for (size_t i=0; i < (size_t)hit_arr_->length() ; i++) {

      d_itr = catsarr_byrun_hash_->find( (*hit_arr_)[i]->msrun_ );
      
      if ( d_itr == catsarr_byrun_hash_->end() ) {
	catsarr_byrun_hash_->insert(make_pair((*hit_arr_)[i]->msrun_, new Array<double>(num_cats_)));
	for (int z=0; z < num_cats_; z++) {
	  (*(*catsarr_byrun_hash_)[(*hit_arr_)[i]->msrun_])[z]=0.;
	}		     
				     
      }
      intarr_hash::iterator it = bypep_cat_hash_->find((*hit_arr_)[i]->peptide_);
      if ( it == bypep_cat_hash_->end() ) {
	(*hit_arr_)[i]->nocat_ = true;
	continue;
      }
      else {
	(*hit_arr_)[i]->nocat_ = false;
      }
      for (int y=0; y<(*bypep_cat_hash_)[(*hit_arr_)[i]->peptide_]->size(); y++) {
	(*(*catsarr_byrun_hash_)[(*hit_arr_)[i]->msrun_])[ (*(*bypep_cat_hash_)[(*hit_arr_)[i]->peptide_])[y] ] += (*hit_arr_)[i]->adj_prob_;
      }
	

    
  }


  for (size_t i=0; i < (size_t)hit_arr_->length() ; i++) {

      d_itr = catsarr_byrun_hash_->find( (*hit_arr_)[i]->msrun_ );
      
      if ( d_itr == catsarr_byrun_hash_->end() ) {
	catsarr_byrun_hash_->insert(make_pair((*hit_arr_)[i]->msrun_, new Array<double>(num_cats_)));
	for (int z=0; z < num_cats_; z++) {
	  (*(*catsarr_byrun_hash_)[(*hit_arr_)[i]->msrun_])[z]=0.;
	}		     
				     
      }
     

      /* *************************************************************
      if ((*hit_arr_)[i]->nocat_) {
	//assign random category
	intarr_hash::iterator it = bypep_cat_hash_->find((*hit_arr_)[i]->peptide_);
	if ( it == bypep_cat_hash_->end() ) {
	  
	  bypep_cat_hash_->insert(make_pair((*hit_arr_)[i]->peptide_, new Array<int>()));
	  
	  int idx = rand() % (num_cats_+1);
	  
	  (*bypep_cat_hash_)[(*hit_arr_)[i]->peptide_]->insertAtEnd(idx);
	  
	}
	for (int y=0; y<(*bypep_cat_hash_)[(*hit_arr_)[i]->peptide_]->size(); y++) {
	  (*(*catsarr_byrun_hash_)[(*hit_arr_)[i]->msrun_])[ (*(*bypep_cat_hash_)[(*hit_arr_)[i]->peptide_])[y] ] += (*hit_arr_)[i]->adj_prob_;
	}
	
      }
      **************************************************************** */
  }

  for (d_itr = (*catsarr_byrun_hash_).begin(); d_itr != (*catsarr_byrun_hash_).end(); d_itr++) {
    int top_cat = 0;
    double top_cat_val = 0;
    for (int z=0; z < num_cats_; z++) {
      if ((*d_itr->second)[z] >  top_cat_val) {
	top_cat = z;
	top_cat_val = (*d_itr->second)[z];
      }
    }
    byrun_topcat_hash_->insert( make_pair(d_itr->first, top_cat) );
  }
    

}


//NSS Model Funcs

double InterProphet::getNSSCounts() {
  double out = 0;
  double maxdenom = 0;
  double matchnumer = 0;
  double sumprob_mismatch = 0;
  double sumprob_match = 0;
  double uncert_mismatch = 0;
  double uncert_match = 0;
  num_engines_ = 1;
  int step = (num_runs_*(num_runs_-1)/2) / 100;
  int tot = 0;
  int count = 0;
  bool first = false;
  for (size_t i=0; i<num_runs_; i++) {
    first = true;
    for (hit_hash::iterator i_itr = (*(*hits_hash_)[i]).begin(); i_itr != (*(*hits_hash_)[i]).end(); i_itr++) {
      i_itr->second->nss_ = 0;
      sumprob_mismatch = 0;
      sumprob_match = 0;	
      uncert_mismatch = 0;
      uncert_match = 0;	
      maxdenom = 0;
      matchnumer = 0;
      for (size_t j=0; j<num_runs_; j++) {
	if (first) progress(count++, step, tot);

	if (i != j) {
	  if ( (*(*hits_hash_)[j]).find(i_itr->first) != (*(*hits_hash_)[j]).end() && 
	       i_itr->second->exp_ == (*(*hits_hash_)[j])[i_itr->first]->exp_ &&
	       i_itr->second->peptide_ == (*(*hits_hash_)[j])[i_itr->first]->peptide_ ) {
	       //fabs( i_itr->second->calcnmass_ - (*(*hits_hash_)[j])[i_itr->first]->calcnmass_ ) <= 0.1 * i_itr->second->peptide_.length() ) {
	    //peptide sequence is the same
	    //i_itr->second->nss_ += (*(*hits_hash_)[j])[i_itr->first]->adj_prob_;
	    //sumprob_match += (*(*hits_hash_)[j])[i_itr->first]->adj_prob_;
	    matchnumer +=  (*(*hits_hash_)[j])[i_itr->first]->adj_prob_;;
	    maxdenom += 1;
	  }
	  else if  ( (*(*hits_hash_)[j]).find(i_itr->first) != (*(*hits_hash_)[j]).end() && 
		     i_itr->second->exp_ == (*(*hits_hash_)[j])[i_itr->first]->exp_ &&
		     i_itr->second->peptide_ != (*(*hits_hash_)[j])[i_itr->first]->peptide_ ) {
	    //peptide sequence is different
	    //i_itr->second->nss_ -= (*(*hits_hash_)[j])[i_itr->first]->adj_prob_;
	    //sumprob_mismatch += (*(*hits_hash_)[j])[i_itr->first]->adj_prob_;
	    maxdenom += 1;
	  }
	  //else if ( (*(*hits_hash_)[j]).find(i_itr->first) == (*(*hits_hash_)[j]).end() ) {
	  //  i_itr->second->nss_ -= 1;
	  //}
	  //TODO deal with homologous peptides
	}
      }
      first = false;      
      //i_itr->second->nss_ += uncert_match + uncert_mismatch;
      if (maxdenom > 0) {
      	i_itr->second->nss_ = matchnumer / maxdenom ;
      }
      else {
      	i_itr->second->nss_ = 0;
      }

      if ((int)(maxdenom+0.5) > num_engines_) {
	num_engines_ = (int)(maxdenom+0.5);
      }
      //if (i_itr->second->nss_ < -1) {
      //	i_itr->second->nss_ = -1;
      //}
      out +=  i_itr->second->nss_;
    }
  }
  if (! (fabs(out) > 0.0001) ) use_nss_ = false; 
  return out;
}

bool InterProphet::updateNSSModel() {
  bool ret = false;

  if (nss_model_->isReady()) {
    ret = nss_model_->update(allprobs_,0);
    //getNSSAdjProbs();
  }

  return ret;
}


bool InterProphet::updateNRSModel() {
  bool ret = false;

  if (nrs_model_->isReady()) {
    ret = nrs_model_->update(allprobs_,0);
    //getNRSAdjProbs();
  }

  return ret;
}

bool InterProphet::updateNSEModel() {
  bool ret = false;
  
  if (nse_model_->isReady()) {
    ret = nse_model_->update(allprobs_,0);
    //getNSEAdjProbs();
  }

  return ret;
}


bool InterProphet::updateNSIModel() {
  bool ret = false;

  if (nsi_model_->isReady()) {
    ret = nsi_model_->update(allprobs_,0);
    //getNSIAdjProbs();
  }

  return ret;
}

bool InterProphet::updateNSMModel() {
  bool ret = false;

  if (nsm_model_->isReady()) {
    ret = nsm_model_->update(allprobs_,0);
    //getNSMAdjProbs();
  }

  return ret;
}

bool InterProphet::updateNSPModel() {
  bool ret = false;

  if (nsp_model_->isReady()) {
    ret = nsp_model_->update(allprobs_,0);
    //getNSMAdjProbs();
  }

  return ret;
}




void InterProphet::computeNSSModel() {
  nss_model_->clear();
  double max = -100000;
  for (size_t i=0; i < (size_t)hit_arr_->length() ; i++) {
    nss_model_->insert((*hit_arr_)[i]->adj_prob_, (*hit_arr_)[i]->nss_);
    if ( (*hit_arr_)[i]->nss_ > max || i == 0) {
      max = (*hit_arr_)[i]->nss_;
    }
  }
  
  if (max <= 0) max = 2; 
  if (nss_model_->makeReady(true, 2)) {  //  if (nss_model_->makeReady(1, 1)) {
    //getNSSAdjProbs();
  }
}




void InterProphet::reportModels(ostream& out) {

    if (use_nss_) {
      nss_model_->report(out);
    }
    
    if (use_nrs_) {
      nrs_model_->report(out);
    }

    if (use_length_) {
      len_model_->report(out);
    }
    
    if (use_nse_) {
      nse_model_->report(out);
    }
    
  
    if (use_nsi_) {
      nsi_model_->report(out);
    }
    
    if (use_nsm_) {
      nsm_model_->report(out);
    }
    
    if (use_nsp_) {
      nsp_model_->report(out);
    }

    if (use_cat_) {
      cat_model_->report(out);
    }
      

    if (use_fpkm_) {
      fpkm_model_->report(out);
    }


}

void InterProphet::getNSSModelAdjProbs() {
  for (size_t i=0; i<num_runs_; i++) {
    for (hit_hash::iterator i_itr = (*(*hits_hash_)[i]).begin(); i_itr != (*(*hits_hash_)[i]).end(); i_itr++) {
      double prob = i_itr->second->pepproph_prob_;
       
      double posprob =  log(nss_model_->getPosProb(i_itr->second->nss_));

      double negprob =  log(nss_model_->getNegProb(i_itr->second->nss_));
 
      double num = exp(log(prob) + posprob);

      double denom = exp(log(prob) + posprob) + exp(log(1-prob) + negprob);

      if (denom <= 0) {
	i_itr->second->nssadj_prob_ = i_itr->second->adj_prob_ = 0;
       
      }
      else {
	i_itr->second->nssadj_prob_ = i_itr->second->adj_prob_ = num / denom;
	 
      }

      for (int n=0; n<3; n++) {
	prob = i_itr->second->allntt_pepproph_prob_[n];

	
	num = exp(posprob + log(prob));

	denom = exp(posprob + log(prob)) + exp(negprob + log(1-prob));

	
	if (denom <= 0) {
	  i_itr->second->allntt_nssadj_prob_[n] = i_itr->second->allntt_adj_prob_[n] = 0;
	}
	else {
	  i_itr->second->allntt_nssadj_prob_[n] = i_itr->second->allntt_adj_prob_[n] = num / denom;
	}
      }
    }
  }

  for (size_t i=0; i<num_runs_; i++) {
    for (hit_hash::iterator i_itr = (*(*hits_hash_)[i]).begin(); i_itr != (*(*hits_hash_)[i]).end(); i_itr++) {
      double prob = log(i_itr->second->adj_prob_);
      for (size_t j=0; j<num_runs_; j++) {
	if (i != j) {

	  hit_hash::iterator j_itr = (*(*hits_hash_)[j]).find(i_itr->first); 

	  if ( j_itr != (*(*hits_hash_)[j]).end() &&
	       j_itr->second->peptide_ != i_itr->second->peptide_) {
	    double tmp = j_itr->second->adj_prob_;
	    if (tmp > MAX_PROB) {
	      tmp = MAX_PROB;
	    }
	    prob += log(1 - tmp);
	  }
	}
      }
      i_itr->second->nssadj_prob_ = i_itr->second->adj_prob_ = exp(prob);
     

      for (int n=0; n<3; n++) {
	prob = 	log(i_itr->second->allntt_adj_prob_[n]);
	for (int j=0; j<(int)num_runs_; j++) {
	  if (i != j) {

	    hit_hash::iterator j_itr = (*(*hits_hash_)[j]).find(i_itr->first); 

	    if (j_itr != (*(*hits_hash_)[j]).end() &&
		j_itr->second->peptide_ != i_itr->second->peptide_) {
	      double tmp = j_itr->second->adj_prob_;
	      if (tmp > MAX_PROB) {
		tmp = MAX_PROB;
	      }
	      prob += log(1 - tmp);
	    }
	  }
	}
	i_itr->second->allntt_nssadj_prob_[n] = i_itr->second->allntt_adj_prob_[n] = exp(prob);
      }
    }
  } 
  //  adjprob_hash_ = outprobs;
  //allntt_adjprob_hash_ = allntt_outprobs;

}


bool InterProphet::getAllModelAdjProbs() {
  return getAllModelAdjProbs(use_nss_, use_nrs_, use_nse_, use_nsi_, use_nsm_, use_nsp_, use_fpkm_, use_length_, use_cat_);
}

 bool InterProphet::getAllModelAdjProbs(bool nss_flag, bool nrs_flag, bool nse_flag, bool nsi_flag, bool nsm_flag, bool nsp_flag, bool fpkm_flag, bool len_flag, bool use_cat) {
  bool ret = false;
  max_pep_prob_tot_ = 0;
  max_spec_prob_tot_ = 0;
  new_prob_tot_ = 0;
  maxprob_hash_->clear();
  maxprob_sp_hash_->clear();
  for (size_t i = 0; i < (size_t)hit_arr_->size(); i++) {

    double prob = (*hit_arr_)[i]->pepproph_prob_;
    double posprob = 	0;
    double negprob = 	0;
    double tpos, tneg, num, denom;
    
    if (!last_iter_ && (*decoy_hit_arr_)[i]) {
      prob = 0;
      num = 0;
      denom = 1;

    }   
    else {
      if (nss_flag) {
	tpos = nss_model_->getPosProb((*hit_arr_)[i]->nss_);
	tneg = nss_model_->getNegProb((*hit_arr_)[i]->nss_);
	// if (tpos > tneg) {
	//	tpos = tpos > tneg * 10 ? tneg * 10 : tpos;
	//}
	//else {
	//	tneg = tneg > tpos * 10 ? tpos * 10 : tneg;
	//}
	
	posprob += log(tpos);
	negprob += log(tneg);
      }
      if (nrs_flag && (*hit_arr_)[i]->nrs_ > -1000000) {
	tpos = nrs_model_->getPosProb((*hit_arr_)[i]->nrs_);
	tneg = nrs_model_->getNegProb((*hit_arr_)[i]->nrs_);
	//if (tpos > tneg) {
	//	tpos = tpos > tneg * 10 ? tneg * 10 : tpos;
	//}
	//else {
	//	tneg = tneg > tpos * 10 ? tpos * 10 : tneg;
	//}
	posprob += log(tpos);
	negprob += log(tneg);
      }
      if (nse_flag && (*hit_arr_)[i]->nse_ > -1000000) {
	tpos = nse_model_->getPosProb((*hit_arr_)[i]->nse_);
	tneg = nse_model_->getNegProb((*hit_arr_)[i]->nse_);
	//if (tpos > tneg) {
	//	tpos = tpos > tneg * 10 ? tneg * 10 : tpos;
	//}
	//else {
	//	tneg = tneg > tpos * 10 ? tpos * 10 : tneg;
	//}
	posprob += log(tpos);
	negprob += log(tneg);
      }
      if (nsi_flag && (*hit_arr_)[i]->nsi_ > -1000000) {
	tpos = nsi_model_->getPosProb((*hit_arr_)[i]->nsi_);
	tneg = nsi_model_->getNegProb((*hit_arr_)[i]->nsi_);
	//if (tpos > tneg) {
	//	tpos = tpos > tneg * 10 ? tneg * 10 : tpos;
	//}
	//else {
	//	tneg = tneg > tpos * 10 ? tpos * 10 : tneg;
	//}
	posprob += log(tpos);
	negprob += log(tneg);
      }
      if (len_flag) {
	tpos = len_model_->getPosProb( (*hit_arr_)[i]->peptide_.length() < 15 ? (*hit_arr_)[i]->peptide_.length() : 15 );
	tneg = len_model_->getNegProb((*hit_arr_)[i]->peptide_.length() < 15 ? (*hit_arr_)[i]->peptide_.length() : 15);
	//if (tpos > tneg) {
	//	tpos = tpos > tneg * 10 ? tneg * 10 : tpos;
	//}
	//else {
	//	tneg = tneg > tpos * 10 ? tpos * 10 : tneg;
	//}
	posprob += log(tpos);
	negprob += log(tneg);
      }
   
      if (nsm_flag) {
	tpos = nsm_model_->getPosProb((*hit_arr_)[i]->nsm_);
	tneg = nsm_model_->getNegProb((*hit_arr_)[i]->nsm_);
	//if (tpos > tneg) {
	//	tpos = tpos > tneg * 10 ? tneg * 10 : tpos;
	//}
	//else {
	//	tneg = tneg > tpos * 10 ? tpos * 10 : tneg;
	//}
	posprob += log(tpos);
	negprob += log(tneg);
      }
      
      if (nsp_flag) {
	tpos = nsp_model_->getPosProb((*hit_arr_)[i]->nsp_);
	tneg = nsp_model_->getNegProb((*hit_arr_)[i]->nsp_);
	//if (tpos > tneg) {
	//	tpos = tpos > tneg * 10 ? tneg * 10 : tpos;
	//}
	//else {
	//	tneg = tneg > tpos * 10 ? tpos * 10 : tneg;
	//}
	posprob += log(tpos);
	negprob += log(tneg);
      }
      
     if (fpkm_flag) {
	tpos = fpkm_model_->getPosProb((*hit_arr_)[i]->fpkm_);
	tneg = fpkm_model_->getNegProb((*hit_arr_)[i]->fpkm_);
	//if (tpos > tneg) {
	//	tpos = tpos > tneg * 10 ? tneg * 10 : tpos;
	//}
	//else {
	//	tneg = tneg > tpos * 10 ? tpos * 10 : tneg;
	//}
	posprob += log(tpos);
	negprob += log(tneg);
      }
      
      if (use_cat) {
	tpos = cat_model_->getPosProb(isInTopCat((*hit_arr_)[i]));
	tneg = cat_model_->getNegProb(isInTopCat((*hit_arr_)[i]));
	//if (tpos > tneg) {
	//	tpos = tpos > tneg * 10 ? tneg * 10 : tpos;
	//}
	//else {
	//	tneg = tneg > tpos * 10 ? tpos * 10 : tneg;
	//}
	posprob += log(tpos);
	negprob += log(tneg);
      }
      tpos = exp(posprob);
      tneg = exp(negprob);
      // if (tpos > tneg) {
      //  tpos = tpos > tneg * 10 ? tneg * 10 : tpos;
      //}
      //else {
      //  tneg = tneg > tpos * 10 ? tpos * 10 : tneg;
      //}
      
      posprob = log(tpos);
      negprob = log(tneg);
      
      
      num = exp(posprob + log(prob));
      
      denom = exp(posprob + log(prob)) + exp(negprob + log(1-prob));
      
    }

    
    if (denom <= 0) {
      //	(*outprobs)[i_itr->first] = 0;
      (*hit_arr_)[i]->adj_prob_ = 0;
    }
    else {
      //(*outprobs)[i_itr->first] = num / denom;
      double newpr = num / denom;
      if (newpr > MAX_PROB) {
	newpr = MAX_PROB;
      }
      
      if (fabs(newpr - (*hit_arr_)[i]->adj_prob_) > MAX_DELTA) {
	ret = true;
      }
      (*hit_arr_)[i]->adj_prob_ = newpr;
      
      new_prob_tot_ +=  (*hit_arr_)[i]->adj_prob_;
      if (maxprob_hash_->find((*hit_arr_)[i]->peptide_) == maxprob_hash_->end()) {
			max_pep_prob_tot_ += (*hit_arr_)[i]->adj_prob_;
			(*maxprob_hash_)[(*hit_arr_)[i]->peptide_] = (*hit_arr_)[i]->adj_prob_;

      } 
      else if ((*hit_arr_)[i]->adj_prob_ > (*maxprob_hash_)[(*hit_arr_)[i]->peptide_]) {
			max_pep_prob_tot_ -= (*maxprob_hash_)[(*hit_arr_)[i]->peptide_];
			max_pep_prob_tot_ += (*hit_arr_)[i]->adj_prob_;
			(*maxprob_hash_)[(*hit_arr_)[i]->peptide_] = (*hit_arr_)[i]->adj_prob_;
      }
	  
	  if (maxprob_sp_hash_->find((*hit_arr_)[i]->spect_) == maxprob_sp_hash_->end()) {
			max_spec_prob_tot_ += (*hit_arr_)[i]->adj_prob_;
			(*maxprob_sp_hash_)[(*hit_arr_)[i]->spect_] = (*hit_arr_)[i]->adj_prob_;

      } 
      else if ((*hit_arr_)[i]->adj_prob_ > (*maxprob_sp_hash_)[(*hit_arr_)[i]->spect_]) {
			max_spec_prob_tot_ -= (*maxprob_sp_hash_)[(*hit_arr_)[i]->spect_];
			max_spec_prob_tot_ += (*hit_arr_)[i]->adj_prob_;
			(*maxprob_sp_hash_)[(*hit_arr_)[i]->spect_] = (*hit_arr_)[i]->adj_prob_;
      }
      
    }


    if (!last_iter_ && (*decoy_hit_arr_)[i]) {
      for (int n=0; n<3; n++) {
	
	  (*hit_arr_)[i]->allntt_adj_prob_[n] = 0;
	
      }
    }
    else {


      for (int n=0; n<3; n++) {
	prob = (*hit_arr_)[i]->allntt_pepproph_prob_[n];
	
	num = exp(posprob + log(prob));
	
	denom = exp(posprob + log(prob)) + exp(negprob + log(1-prob));
	
	
	if (denom <= 0) {
	  //	  (*(*allntt_outprobs)[i_itr->first])[n] = 0;
	  (*hit_arr_)[i]->allntt_adj_prob_[n] = 0;
	}
	else {
	  //(*(*allntt_outprobs)[i_itr->first])[n] = num / denom;
	  (*hit_arr_)[i]->allntt_adj_prob_[n] = num / denom;
	}
      }
    }
  }
  return ret;
  //  top_adjprob_hash_ = outprobs;
  //top_allntt_adjprob_hash_ = allntt_outprobs;
}


double InterProphet::getNSSValue(int runidx, string& exp_lbl, string& spectrum, string& swath_assay) {
  string key = exp_lbl + spectrum + swath_assay;  
  return (*(*hits_hash_)[runidx])[key]->nss_;
}

bool InterProphet::getCatValue(int runidx, string& exp_lbl, string& spectrum, string& swath_assay) {
  string key = exp_lbl + spectrum + swath_assay;  
  if ( (*(*hits_hash_)[runidx])[key]->nocat_ ) {
    return false;
  }
  return isInTopCat((*(*hits_hash_)[runidx])[key]);
}

//NRS Model Funcs
/* **** DEPRECATED
void InterProphet::getNRSCounts() {

  for (int_hash::iterator i_itr = (*top_i_hash_).begin(); i_itr != (*top_i_hash_).end(); i_itr++) {
    
    hit_hash::iterator h_itr = (*(*hits_hash_)[i_itr->second]).find(i_itr->first);


      maxprob_hash_->clear();
      h_itr->second->nrs_ = 0;

      Array<SearchHit*>* hitarr = (*bypep_tophit_hash_)[h_itr->second->peptide_];

      for (int j=0; hitarr != NULL && j < hitarr->size(); j++) {
	if ( h_itr->second != (*hitarr)[j] &&
	     getSpectrumCharge(h_itr->second->spect_) == getSpectrumCharge((*hitarr)[j]->spect_) &&
	     h_itr->second->msrun_ == (*hitarr)[j]->msrun_ &&
	     h_itr->second->spect_ != (*hitarr)[j]->spect_ &&
	     h_itr->second->peptide_ == (*hitarr)[j]->peptide_ &&
	     fabs(h_itr->second->calcnmass_ - (*hitarr)[j]->calcnmass_) <= 0.1 * (*hitarr)[j]->peptide_.length() ) {
	  
	  char* key = new char[(*hitarr)[j]->spect_.length()+1];

	  strcpy(key, (*hitarr)[j]->spect_);
	  

	  dbl_hash::iterator max_itr = (*maxprob_hash_).find(key);
	  
	  
	  if (max_itr ==  maxprob_hash_->end() || (*maxprob_hash_)[key] < (*(*hits_hash_)[(*hitarr)[j]->run_idx_])[((*hitarr)[j]->exp_+(*hitarr)[j]->spect_)]->pepproph_prob_ ) {
	    (*maxprob_hash_)[key] = (*(*hits_hash_)[(*hitarr)[j]->run_idx_])[((*hitarr)[j]->exp_+(*hitarr)[j]->spect_)]->pepproph_prob_;
	  }
	}
      }

      for (dbl_hash::iterator k_itr = maxprob_hash_->begin(); k_itr !=  maxprob_hash_->end(); k_itr++) {
	h_itr->second->nrs_  += (*maxprob_hash_)[k_itr->first];
      }
  }

    
}
*/

double InterProphet::getNRSCount(SearchHit* hit) {
  

      maxprob_hash_->clear();
      hit->nrs_ = 0;

      Array<SearchHit*>* hitarr = (*bypep_tophit_hash_)[hit->peptide_];
      double sum_nprob = 0;
      
      bool has_partner = false;
      double chk_prob = 1;
      
      string* key = new string();
      for (int j=0; hitarr != NULL && j < hitarr->size(); j++) {

	if ( hit != (*hitarr)[j] &&
	     hit->chg_ == (*hitarr)[j]->chg_ &&
	     hit->exp_ == (*hitarr)[j]->exp_ &&
	     hit->spect_ != (*hitarr)[j]->spect_ &&
	     hit->peptide_ == (*hitarr)[j]->peptide_ &&
	     hit->modpep_ == (*hitarr)[j]->modpep_ &&
	     hit->swathpep_ == (*hitarr)[j]->swathpep_ &&
	     //hit->swathwin_ == (*hitarr)[j]->swathwin_ &&
	     fabs(hit->calcnmass_ - (*hitarr)[j]->calcnmass_) <= 0.1 * (*hitarr)[j]->peptide_.length() ) {

	  if (swath_mode_ &&  hit->run_idx_ != (*hitarr)[j]->run_idx_ ) {
	    continue; //In SWATH mode has to be within the same run
	  }
	  
	  *key = (*hitarr)[j]->spect_ ;
	  *key += (*hitarr)[j]->swathpep_;

	  dbl_hash::iterator max_itr = (*maxprob_hash_).find(*key);
	  
	  double nprob = (*(*hits_hash_)[(*hitarr)[j]->run_idx_])[((*hitarr)[j]->exp_+(*hitarr)[j]->spect_)+(*hitarr)[j]->swathpep_]->adj_prob_;// - 0.5L; //to make the numbers greater than 0, subtract 1*sizeofmaxprob_hash
	  
	  if (swath_mode_) {
	    //hit->nrs_ +=  fabs(hit->rt_-(*hitarr)[j]->rt_);
	    //sum_nprob += 1;	  
	  }
	  else {
	    nprob -= 0.5;
	    
	  }
	  if (max_itr ==  maxprob_hash_->end() ) {
	    maxprob_hash_->insert(make_pair(*key, nprob));
	  }
	  else if ((*maxprob_hash_)[*key] < nprob) {
	    (*maxprob_hash_)[*key] = nprob;

	  }
	}
      }
 
      
      

      //prob += 1e-15;
      //prob = pow((1. - exp(prob)), 1./(maxprob_hash_->size()+1.));
      //prob = log(prob) - log(maxprob_hash_->size()+1);
      //prob = exp(prob);
      

      if (swath_mode_) {



      
          for (int j=0; hitarr != NULL && j < hitarr->size(); j++) {
    

	    if ( hit != (*hitarr)[j] &&
		 hit->chg_ == (*hitarr)[j]->chg_ &&
		 hit->exp_ == (*hitarr)[j]->exp_ &&
		 hit->spect_ != (*hitarr)[j]->spect_ &&
		 hit->peptide_ == (*hitarr)[j]->peptide_ &&
		 hit->modpep_ == (*hitarr)[j]->modpep_ &&
		 hit->swathpep_ == (*hitarr)[j]->swathpep_ &&
		 //hit->swathwin_ == (*hitarr)[j]->swathwin_ &&
		 fabs(hit->calcnmass_ - (*hitarr)[j]->calcnmass_) <= 0.1 * (*hitarr)[j]->peptide_.length() ) {
	      
	      // if (swath_mode_ &&  hit->run_idx_ != (*hitarr)[j]->run_idx_ ) {
	      // 	continue;  //In SWATH mode has to be within the same run

		
	      // }
	      
	      *key = (*hitarr)[j]->spect_ ;
	      *key += (*hitarr)[j]->swathpep_;

	      dbl_hash::iterator max_itr = (*maxprob_hash_).find(*key);

	      double nprob = (*(*hits_hash_)[(*hitarr)[j]->run_idx_])[((*hitarr)[j]->exp_+(*hitarr)[j]->spect_+(*hitarr)[j]->swathpep_)]->adj_prob_;
	      chk_prob *= (1-nprob);
	      //nprob -= 0.5;
	      has_partner = true;
	      if (nprob > 0 && fabs( (*maxprob_hash_)[*key] - nprob ) < 0.0000001 ) {
		hit->nrs_ +=  fabs(hit->rt_-(*hitarr)[j]->rt_)*(nprob);
		sum_nprob += (nprob);
	      } 

	    }

	  }
	  
	
	  double minProb =0;
	chk_prob = (1-chk_prob);

	if ( chk_prob > minProb && sum_nprob > 0) {
	  //	stddev = stddev * V1 / (V1*V1 - V2) ; //unbiased estimator
	  hit->nrs_  /= sum_nprob;
	  
	}
	else if (hit->altswath_ == -1)  {
	  hit->nrs_ = -99999999; //wash
	}
	else {
	  hit->nrs_ = -1;   //max demote
	}
	//hit->nrs_ = hit->nrs_ > 5 ? 5 : hit->nrs_;
      }
      else {
	double prob = SumDoubleHash(maxprob_hash_);
	hit->nrs_ = prob > 15 ? 15 : (prob < -15 ?  15 : prob);
      }
      
      
      delete key;

  return hit->nrs_;
}

double InterProphet::getNRSCounts() {
  double out=0;
  int step = hit_arr_->length() / 100;
  int tot = 0;
  double max = 0;
  double tmp = 0;
  for (int i=0; i < hit_arr_->length() ; i++) {
    progress(i, step, tot);
    out += (tmp = getNRSCount((*hit_arr_)[i]));
    if (tmp > max) {
      max = tmp;
    }
  }
  //max = 5;
  if (swath_mode_) {
    for (int i=0; i < hit_arr_->length() ; i++) {
   
      //if ( (*hit_arr_)[i]->nrs_ > max) {
      //	(*hit_arr_)[i]->nrs_ = max;
      //}

      if ((*hit_arr_)[i]->nrs_ < 0 && (*hit_arr_)[i]->altswath_ > -1 ) {
	(*hit_arr_)[i]->nrs_ = max+0.1;
      }
    }
  }

  if (! (fabs(out) > 0.0001) ) use_nrs_ = false; 
  return out;
}

void InterProphet::computeNRSModel() {
  nrs_model_->clear();

  for (int i=0; i < hit_arr_->length() ; i++) {
    //if ((*hit_arr_)[i]->spect_ == "OR20070625_HS_L-H-1-1_12.07860.07860.2") {
    //  cerr << "Here now!" << endl;
    //}
    if ((*hit_arr_)[i]->nrs_ > -1000000)
     nrs_model_->insert((*hit_arr_)[i]->adj_prob_, (*hit_arr_)[i]->nrs_);
  }

  if(nrs_model_->makeReady(false, 10)) {  //  if(nrs_model_->makeReady(5,5)) {
    //getNRSAdjProbs();
  }
}


double InterProphet::getNRSValue(int runidx, string& exp_lbl, string& spectrum, string& swath_assay ) {
  string key = exp_lbl + spectrum + swath_assay;
  return (*(*hits_hash_)[runidx])[key]->nrs_;
}


//NSE Model Funcs
/****DEPRECATED
void InterProphet::getNSECounts() {
  size_t i, j;

  for (int_hash::iterator i_itr = (*top_i_hash_).begin(); i_itr != (*top_i_hash_).end(); i_itr++) {
  
    hit_hash::iterator h_itr = (*(*hits_hash_)[i_itr->second]).find(i_itr->first);

    maxprob_hash_->clear();

    Array<SearchHit*>* hitarr = (*bypep_tophit_hash_)[h_itr->second->peptide_];
    h_itr->second->nse_ = 0;

    for (j=0; hitarr != NULL && j < hitarr->size(); j++) {

      if ( h_itr->second  != (*hitarr)[j] &&
	   getSpectrumCharge(h_itr->second->spect_) == getSpectrumCharge((*hitarr)[j]->spect_) &&
	   h_itr->second->msrun_ != (*hitarr)[j]->msrun_ &&
	   h_itr->second->peptide_ == (*hitarr)[j]->peptide_ &&
	   fabs(h_itr->second->calcnmass_ - (*hitarr)[j]->calcnmass_) <= 0.1 * (*hitarr)[j]->peptide_.length() ) {
	
	  char* key = new char[(*hitarr)[j]->msrun_.length()+1];
	  strcpy(key, (*hitarr)[j]->msrun_);
	  dbl_hash::iterator max_itr = (*maxprob_hash_).find(key);
	  
	  if (max_itr ==  maxprob_hash_->end() || 
	      (*maxprob_hash_)[key] < (*(*hits_hash_)[(*hitarr)[j]->run_idx_])[((*hitarr)[j]->exp_+(*hitarr)[j]->spect_)]->pepproph_prob_) {

	    (*maxprob_hash_)[key] = (*(*hits_hash_)[(*hitarr)[j]->run_idx_])[((*hitarr)[j]->exp_+(*hitarr)[j]->spect_)]->pepproph_prob_;
	  }
      }
      
    }
    
    for (dbl_hash::iterator k_itr = maxprob_hash_->begin(); k_itr !=  maxprob_hash_->end(); k_itr++) {
      h_itr->second->nse_  += (*maxprob_hash_)[k_itr->first];
    }
  }
    
    

}
*/
bool InterProphet::isInTopCat(SearchHit* hit) {
  intarr_hash::iterator it = bypep_cat_hash_->find(hit->peptide_);
  if ( it == bypep_cat_hash_->end() ) {
    return false; //if not in the list assume ok
  }
  for (int y=0; y < (*bypep_cat_hash_)[hit->peptide_]->size(); y++) {
    if ((*(*bypep_cat_hash_)[hit->peptide_])[y] == (*byrun_topcat_hash_)[hit->msrun_]) 
      return true;
  }
  return false;
}

double InterProphet::getNSECount(SearchHit* hit) {
  int j;

  

    maxprob_hash_->clear();

    Array<SearchHit*>* hitarr = (*bypep_tophit_hash_)[hit->peptide_];
    hit->nse_ = 0;
    
    double sum_nprob = 0;
    bool has_partner = false;
    double chk_prob = 1;
    
    int partner_count = 0;

    string* key = new string();


    for (j=0; hitarr != NULL && j < hitarr->size(); j++) {
 
      if ( hit  != (*hitarr)[j] &&
	   hit->chg_ == (*hitarr)[j]->chg_ &&
	   hit->exp_ != (*hitarr)[j]->exp_ &&
	   hit->peptide_ == (*hitarr)[j]->peptide_ &&
	   hit->modpep_ == (*hitarr)[j]->modpep_ &&
	   hit->swathpep_ == (*hitarr)[j]->swathpep_ &&
	   (hit->swathwin_ == (*hitarr)[j]->swathwin_ || !sharp_nse_) &&
	   fabs(hit->calcnmass_ - (*hitarr)[j]->calcnmass_) <= 0.1 * (*hitarr)[j]->peptide_.length() ) {
	
	*key = (*hitarr)[j]->exp_;


	*key += (*hitarr)[j]->swathpep_;

	*key += (*hitarr)[j]->swathwin_;


	dbl_hash::iterator max_itr = (*maxprob_hash_).find(*key);
	
	double nprob = (*(*hits_hash_)[(*hitarr)[j]->run_idx_])[((*hitarr)[j]->exp_+(*hitarr)[j]->spect_+(*hitarr)[j]->swathpep_)]->adj_prob_;// - 0.5;

	has_partner = true;

	nprob -= 0.5;


	if (max_itr ==  maxprob_hash_->end()) {
	  maxprob_hash_->insert(make_pair(*key, nprob));
	  partner_count++;
	  sum_nprob += (nprob + 0.4999999999);
	  //if (fabs(hit->rt_-(*hitarr)[j]->rt_) <= 2) {
	  chk_prob *= (1-(nprob+0.4999999999));	  
	    //}
	}
	else if ( (*maxprob_hash_)[*key] < nprob) {
	  sum_nprob -= ((*maxprob_hash_)[*key] + 0.4999999999);
	  chk_prob /= (1-((*maxprob_hash_)[*key]+0.4999999999));	
  
	  (*maxprob_hash_)[*key] = nprob;
          sum_nprob += ((*maxprob_hash_)[*key] + 0.4999999999);
	  chk_prob *= (1-((*maxprob_hash_)[*key]+0.4999999999));
	}
	
      
      }
    }
    
    double minProb = 0; //sharp_nse_ ? 0 : 0.5;//.9;
    double maxProb = 0;
    double minMax = 0.51;
    if (sharp_nse_ && swath_mode_) {
      hit->nse_ = 0;
      chk_prob = 1;
      sum_nprob = 0;
          for (j=0; hitarr != NULL && j < hitarr->size(); j++) {
    
	    if ( hit  != (*hitarr)[j] &&
		 hit->chg_ == (*hitarr)[j]->chg_ &&
		 hit->exp_ != (*hitarr)[j]->exp_ &&
		 hit->peptide_ == (*hitarr)[j]->peptide_ &&
		 hit->modpep_ == (*hitarr)[j]->modpep_ &&
		 hit->swathpep_ == (*hitarr)[j]->swathpep_ &&
		 hit->swathwin_ == (*hitarr)[j]->swathwin_ &&
		 fabs(hit->calcnmass_ - (*hitarr)[j]->calcnmass_) <= 0.1 * (*hitarr)[j]->peptide_.length() ) {
	      
	      *key = (*hitarr)[j]->exp_;
	      *key += (*hitarr)[j]->swathpep_;
	      *key += (*hitarr)[j]->swathwin_;

	      dbl_hash::iterator max_itr = (*maxprob_hash_).find(*key);

	      double nprob = (*(*hits_hash_)[(*hitarr)[j]->run_idx_])[((*hitarr)[j]->exp_+(*hitarr)[j]->spect_+(*hitarr)[j]->swathpep_)]->adj_prob_;

	      nprob -= 0.5;
	      //has_partner = true;


	      if (nprob+0.5 >= 0  && fabs( (*maxprob_hash_)[*key] - nprob ) < 0.0000001 ) {
		chk_prob *= (1-nprob-0.4999999999);
		hit->nse_ +=  fabs(hit->rt_-(*hitarr)[j]->rt_)*(nprob+0.4999999999);
		sum_nprob += (nprob+0.4999999999);
	      } 

	      if (nprob+0.5 > maxProb) {
		maxProb = nprob+0.5;
	      }

	    }

	  }
	  
       minProb = 0;//.9;
       
	// if (sharp_nse_) {
     	//minProb = 0;
	//}

      chk_prob = (1-chk_prob);

      if (maxProb > minMax && chk_prob > minProb && sum_nprob > 0) {//  if (sum_nprob > minSum ) {
	//	stddev = stddev * V1 / (V1*V1 - V2) ; //unbiased estimator
	hit->nse_  /= sum_nprob;
	
      }
      else if (has_partner && sharp_nse_) {
	hit->nse_ = -1; //max+1 value, max demote
      }
      else if (has_partner && !sharp_nse_) {
	hit->nse_ = -99999999; //wash value, no promote or demote
      }
      else if (sharp_nse_) {
      	hit->nse_ = -1;//-99999999; 
      }
      else  {
      	hit->nse_ = -99999999; 
      }
      //      else if (sum_nprob > 0) {
      //	hit->nse_ = -99999999;
      // }
      //else {
      // 	hit->nse_ = -1; //-999999999999999
      //}

    }
    else {
      hit->nse_ = SumDoubleHash(maxprob_hash_);
    }


    
    delete key;
    

    chk_prob = (1-chk_prob);
   
    if (!sharp_nse_) {
      if (swath_mode_) {
	//if (sum_nprob <= 0) {
	//	hit->nse_ = -99999999;
	//}
	
	
	//hit->nse_ = hit->nse_ > 5 ? 5 : (hit->nse_ < -5 ?  5 : hit->nse_);


	hit->nse_ = chk_prob; // PI prob
	
	
	hit->nse_ = SumDoubleHash(maxprob_hash_); // Sum on (prob-0.5)
	
	int size = exp_hash_->size();

	//if (hit->exp_.find("urine") != string::npos) {
	//  hit->nse_ *= 2;
	//}

	for (int p = partner_count; p < size-1; p++) {
	  hit->nse_ -= 0.5; // subtract extra 0.5 for missing values in other partner experiments
	} 



      }
      else {
	hit->nse_ = hit->nse_ > 15 ? 15 : (hit->nse_ < -15 ?  15 : hit->nse_);
      }
    }      

    return hit->nse_;
}    

double InterProphet::getNSECounts() {
  double out=0;
  int step = hit_arr_->length() / 100;
  int tot = 0;

  swath_mode_ = swath_mode_ && iRT_sum_ > -1000000 ;
    
  double max = 0;
  double min = 0;
  double tmp = 0;
  for (int i=0; i < hit_arr_->length() ; i++) {
    progress(i, step, tot);
    out += (tmp = getNSECount((*hit_arr_)[i]));
    
    if (!i || tmp > max) {
       max = tmp;
     }
    if (!i || tmp < min) {
       min = tmp;
     }


  }
  //  max = 3;

  if (sharp_nse_ && swath_mode_) {
    for (int i=0; i < hit_arr_->length() ; i++) {
      // if ((*hit_arr_)[i]->nse_ > -1000000) {
      // 	(*hit_arr_)[i]->nse_ = min - 0.5;
      // }
      // if ((*hit_arr_)[i]->nse_ > max ) {
      // 	(*hit_arr_)[i]->nse_ = max ;
      // }
      if (sharp_nse_ && (*hit_arr_)[i]->nse_ < 0 && (*hit_arr_)[i]->nse_ > -1000000 ) {
	(*hit_arr_)[i]->nse_ = max + 0.5 ;
      }
    }
  }

  if (! (fabs(out) > 0.0001) ) use_nse_ = false; 
  return out;
}

void InterProphet::computeNSEModel() {
  //  getNSECounts();
  nse_model_->clear();
  for (int i=0; i < hit_arr_->length() ; i++) {
    if ((*hit_arr_)[i]->nse_ > -1000000)
      nse_model_->insert((*hit_arr_)[i]->adj_prob_, (*hit_arr_)[i]->nse_);
  }
  
  if (nse_model_->makeReady(false, 10)) {  //  if (nse_model_->makeReady(1,1)) {
    // getNSEAdjProbs();
  }
}

void InterProphet::computeCatModel() {
  //  getNSECounts();
  cat_model_->clear();
  for (int i=0; i < hit_arr_->length() ; i++) {
    cat_model_->insert((*hit_arr_)[i]->adj_prob_, isInTopCat((*hit_arr_)[i]));
  }
  
  if (cat_model_->makeReady()) {  //  if (nse_model_->makeReady(1,1)) {
    // getNSEAdjProbs();
  }
}

//void  InterProphet::getNSEAdjProbs() {
//     getodeMlAdjProbs(adjprob_hash_, nseadjprob_hash_, allntt_adjprob_hash_, allntt_nseadjprob_hash_, nse_model_, nse_hash_);
// }

// double InterProphet::getNSEAdjProb(int runidx, string& exp_lbl, string& spectrum) {
//   return (*(*nseadjprob_hash_)[runidx])[spectrum];
// }

double InterProphet::getNSEValue(int runidx, string& exp_lbl, string& spectrum, string& swath_assay ) {
  string key = exp_lbl + spectrum + swath_assay;
  return (*(*hits_hash_)[runidx])[key]->nse_;
}


//NSI Model Funcs
/*** DEPRECATED
void InterProphet::getNSICounts() {
  size_t i, j;
  string key;
  
  for (int_hash::iterator i_itr = (*top_i_hash_).begin(); i_itr != (*top_i_hash_).end(); i_itr++) {
  
    hit_hash::iterator h_itr = (*(*hits_hash_)[i_itr->second]).find(i_itr->first);

    maxprob_hash_->clear();

    h_itr->second->nsi_ = 0;
    
    Array<SearchHit*>* hitarr = (*bypep_tophit_hash_)[h_itr->second->peptide_];
    for ( j=0; hitarr != NULL && j < hitarr->size(); j++) {
      if ( h_itr->second != (*hitarr)[j] &&
	   getSpectrumCharge(h_itr->second->spect_) !=  getSpectrumCharge((*hitarr)[j]->spect_) &&
	   h_itr->second->peptide_ == (*hitarr)[j]->peptide_ &&
	   fabs(h_itr->second->calcnmass_ - (*hitarr)[j]->calcnmass_) <= 0.1 * (*hitarr)[j]->peptide_.length() ) {
	
	char* key = new char[2];
	key[0] = (*hitarr)[j]->spect_[(*hitarr)[j]->spect_.length()-1];  key[1] = '\0';
	
	dbl_hash::iterator max_itr = (*maxprob_hash_).find(key);
	
	if (max_itr ==  maxprob_hash_->end() || (*maxprob_hash_)[key] < (*(*hits_hash_)[(*hitarr)[j]->run_idx_])[((*hitarr)[j]->exp_+(*hitarr)[j]->spect_)]->pepproph_prob_ ) {
	  (*maxprob_hash_)[key] = (*(*hits_hash_)[(*hitarr)[j]->run_idx_])[((*hitarr)[j]->exp_+(*hitarr)[j]->spect_)]->pepproph_prob_;
	}

      }
    }
    for (dbl_hash::iterator k_itr = maxprob_hash_->begin(); k_itr !=  maxprob_hash_->end(); k_itr++) {
      h_itr->second->nsi_  += (*maxprob_hash_)[k_itr->first];
    }
  }

  

}
***/

double InterProphet::getNSICount(SearchHit* hit) {
  int j;
  
  

    maxprob_hash_->clear();

    hit->nsi_ = 0;
    double sum_nprob = 0;

    if (swath_mode_) {
        Array<SearchHit*>* hitarr = (*bypep_tophit_hash_)[hit->peptide_];
      // for ( j=0; hitarr != NULL && j < hitarr->size(); j++) {
      // 	if ( hit != (*hitarr)[j] &&
      // 	     hit->run_idx_ == (*hitarr)[j]->run_idx_ && 
      // 	     hit->chg_ !=  (*hitarr)[j]->chg_ &&
      // 	     hit->peptide_ == (*hitarr)[j]->peptide_ &&
      // 	     fabs(hit->calcnmass_ - (*hitarr)[j]->calcnmass_) <= 0.1 * (*hitarr)[j]->peptide_.length() ) {
	  
      // 	  string* key = &(*hitarr)[j]->chg_;
	  
      // 	  dbl_hash::iterator max_itr = (*maxprob_hash_).find(*key);
	  
      // 	  if (max_itr ==  maxprob_hash_->end()) {
      // 	    (*maxprob_hash_)[*key] = (*(*hits_hash_)[(*hitarr)[j]->run_idx_])[((*hitarr)[j]->exp_+(*hitarr)[j]->spect_+(*hitarr)[j]->swathpep_)]->adj_prob_;
      // 	  }
      // 	  else if ( (*maxprob_hash_)[*key] < (*(*hits_hash_)[(*hitarr)[j]->run_idx_])[((*hitarr)[j]->exp_+(*hitarr)[j]->spect_+(*hitarr)[j]->swathpep_)]->adj_prob_ ) {
      // 	    (*maxprob_hash_)[*key] = (*(*hits_hash_)[(*hitarr)[j]->run_idx_])[((*hitarr)[j]->exp_+(*hitarr)[j]->spect_+(*hitarr)[j]->swathpep_)]->adj_prob_ ;
	    
      // 	  }
	  
      // 	}
      // }
	bool has_partner = false;
	double chk_prob = 1;
      for ( j=0; hitarr != NULL && j < hitarr->size(); j++) {
	if ( hit != (*hitarr)[j] &&
	     hit->run_idx_ == (*hitarr)[j]->run_idx_ && 
	     hit->chg_ !=  (*hitarr)[j]->chg_ &&
	     hit->modpep_ == (*hitarr)[j]->modpep_ &&
	     hit->peptide_ == (*hitarr)[j]->peptide_ &&
	     fabs(hit->calcnmass_ - (*hitarr)[j]->calcnmass_) <= 0.1 * (*hitarr)[j]->peptide_.length() ) {
	  
	  string* key = &(*hitarr)[j]->chg_;
	  
	  dbl_hash::iterator max_itr = (*maxprob_hash_).find(*key);
	  
	  double nprob = (*(*hits_hash_)[(*hitarr)[j]->run_idx_])[((*hitarr)[j]->exp_+(*hitarr)[j]->spect_+(*hitarr)[j]->swathpep_)]->adj_prob_;//-0.5;
	   chk_prob *= (1-nprob);
	  if (nprob > 0) { // && fabs( (*maxprob_hash_)[*key] - nprob ) < 0.0000001 ) {
	    hit->nsi_ +=  fabs(hit->rt_-(*hitarr)[j]->rt_)*(nprob);
	    sum_nprob += (nprob);
	  } 
	  has_partner = true;
	}
	  
      }

      double minProb = 0;
      chk_prob = (1-chk_prob);
      if (chk_prob > minProb && sum_nprob > 0) {
	//	stddev = stddev * V1 / (V1*V1 - V2) ; //unbiased estimator
	hit->nsi_  /= sum_nprob;
	
      }
      else if (has_partner) {
	hit->nsi_ = -1; //max+1 value, max demote
      }
      else {
      	hit->nsi_ = -99999999; //-999999999999999
      }
    }
    else {
      
      Array<SearchHit*>* hitarr = (*bypep_tophit_hash_)[hit->peptide_];
      for ( j=0; hitarr != NULL && j < hitarr->size(); j++) {
	if ( hit != (*hitarr)[j] &&
	     hit->chg_ !=  (*hitarr)[j]->chg_ &&
	     hit->peptide_ == (*hitarr)[j]->peptide_ &&
	     fabs(hit->calcnmass_ - (*hitarr)[j]->calcnmass_) <= 0.1 * (*hitarr)[j]->peptide_.length() ) {
	  
	  string* key = &(*hitarr)[j]->chg_;
	  
	  dbl_hash::iterator max_itr = (*maxprob_hash_).find(*key);
	  
	  if (max_itr ==  maxprob_hash_->end()) {
	    (*maxprob_hash_)[*key] = (*(*hits_hash_)[(*hitarr)[j]->run_idx_])[((*hitarr)[j]->exp_+(*hitarr)[j]->spect_+(*hitarr)[j]->swathpep_)]->adj_prob_;
	  }
	  else if ( (*maxprob_hash_)[*key] < (*(*hits_hash_)[(*hitarr)[j]->run_idx_])[((*hitarr)[j]->exp_+(*hitarr)[j]->spect_+(*hitarr)[j]->swathpep_)]->adj_prob_ ) {
	    (*maxprob_hash_)[*key] = (*(*hits_hash_)[(*hitarr)[j]->run_idx_])[((*hitarr)[j]->exp_+(*hitarr)[j]->spect_+(*hitarr)[j]->swathpep_)]->adj_prob_ ;
	  }
	  
	}
      }

  
      hit->nsi_  = SumDoubleHash(maxprob_hash_);
    }     
  

    
    return hit->nsi_;
}



double InterProphet::getNSICounts() {
  double out=0;
  int step = hit_arr_->length() / 100;
  int tot = 0;
     
  double max = 0;
  double tmp = 0;
  for (int i=0; i < hit_arr_->length() ; i++) {
    progress(i, step, tot);
    out +=  (tmp = getNSICount((*hit_arr_)[i]));
     if (tmp > max) {
      max = tmp;
    }
  }
  if (swath_mode_) {
    for (int i=0; i < hit_arr_->length() ; i++) {
      if ((*hit_arr_)[i]->nsi_ < 0 && (*hit_arr_)[i]->nsi_ > -1000000) {
	(*hit_arr_)[i]->nsi_ = max + 1;
      }
    }
  }

  if (! (fabs(out) > 0.0001) ) use_nsi_ = false; 
  return out;
  
}

void InterProphet::computeNSIModel() {
  //getNSICounts();
  nsi_model_->clear();
  for (int i=0; i < hit_arr_->length() ; i++) {
    if ((*hit_arr_)[i]->nsi_ > -1000000)
      nsi_model_->insert((*hit_arr_)[i]->adj_prob_, (*hit_arr_)[i]->nsi_);
  }
  if (nsi_model_->makeReady(false, 5)) {  //  if (nsi_model_->makeReady(1, 1)) {
    //getNSIAdjProbs();
  }
}



double InterProphet::getNSIValue(int runidx, string& exp_lbl, string& spectrum, string& swath_assay ) {
  string key = exp_lbl + spectrum+ swath_assay;
  return (*(*hits_hash_)[runidx])[key]->nsi_;
}


//NSM Model Funcs
/*****DEPREPCATED
void InterProphet::getNSMCounts() {
  size_t i, j;
  for (int_hash::iterator i_itr = (*top_i_hash_).begin(); i_itr != (*top_i_hash_).end(); i_itr++) {
    
    hit_hash::iterator h_itr = (*(*hits_hash_)[i_itr->second]).find(i_itr->first);
    
    maxprob_hash_->clear();
    h_itr->second->nsm_ = 0;
    
    Array<SearchHit*>* hitarr = (*bypep_tophit_hash_)[h_itr->second->peptide_];
    
    for ( j=0; hitarr != NULL && j < hitarr->size(); j++) {
      if ( h_itr->second != (*hitarr)[j] &&
	   h_itr->second->spect_ != (*hitarr)[j]->spect_ &&
	   h_itr->second->peptide_ == (*hitarr)[j]->peptide_ &&
	   fabs(h_itr->second->calcnmass_ - (*hitarr)[j]->calcnmass_) > 0.1 * (*hitarr)[j]->peptide_.length() ) {
	
	string key_str = (*hitarr)[j]->modpep_;
	
	char* key = new char[key_str.length()+1];
	
	strcpy(key, key_str);
	
	dbl_hash::iterator max_itr = (*maxprob_hash_).find(key);
	
	
	if (max_itr ==  maxprob_hash_->end() || (*maxprob_hash_)[key] < (*(*hits_hash_)[(*hitarr)[j]->run_idx_])[((*hitarr)[j]->exp_+(*hitarr)[j]->spect_)]->pepproph_prob_ ) {
	  (*maxprob_hash_)[key] = (*(*hits_hash_)[(*hitarr)[j]->run_idx_])[((*hitarr)[j]->exp_+(*hitarr)[j]->spect_)]->pepproph_prob_;
	}

      }
    }
    for (dbl_hash::iterator k_itr = maxprob_hash_->begin(); k_itr !=  maxprob_hash_->end(); k_itr++) {
      h_itr->second->nsm_  += (*maxprob_hash_)[k_itr->first];
    }
  }
  
  
}

*/

double InterProphet::getPeptideLength(SearchHit* hit, int thread) {
  return hit->peptide_.length() < 15 ? hit->peptide_.length() : 15 ;

}

double InterProphet::getNSPCount(SearchHit* hit, int thread) {
  int j,k,l; 
  
  bool found = false;
 

  hit->nsp_ = 0;
  string* key = &hit->peptide_ ;
  
  if ((*bypep_nsp_hash_)[thread]->find(*key) != (*bypep_nsp_hash_)[thread]->end()) {
    hit->nsp_ = (*(*bypep_nsp_hash_)[thread])[*key];
    return hit->nsp_;
  }
  
  dbl_hash* maxprob_hash = new dbl_hash();
  for (k=0; k < hit->prots_->size(); k ++) {
    
    Array<SearchHit*>* hitarr = (*(*byprot_tophit_hashes_)[thread])[*(*hit->prots_)[k]];
    
    for ( j=0; hitarr != NULL && j < hitarr->size(); j++) {
      if ( hit != (*hitarr)[j] &&
	   hit->spect_ != (*hitarr)[j]->spect_ &&
	   hit->peptide_ != (*hitarr)[j]->peptide_ ) {
	
	key = &(*hitarr)[j]->peptide_;
	
	dbl_hash::iterator max_itr = (*maxprob_hash).find(*key);
	double nprob = (*(*(*hits_hashes_)[thread])[(*hitarr)[j]->run_idx_])[((*hitarr)[j]->exp_+(*hitarr)[j]->spect_)+(*hitarr)[j]->swathpep_]->adj_prob_;
	
	if (max_itr ==  maxprob_hash->end()) {
	  maxprob_hash->insert(make_pair(*key, nprob));
	}
	else if ((*maxprob_hash)[*key] < nprob)  {
	  (*maxprob_hash)[*key] = nprob;
	}
      
	
      }
    }
    
  }


  hit->nsp_   = SumDoubleHash(maxprob_hash);


  hit->nsp_   =  hit->nsp_ > 20 ? 20 :  hit->nsp_;
  
  
  
  key = &hit->peptide_ ;

  (*bypep_nsp_hash_)[thread]->insert(make_pair(*key, hit->nsp_));
  
  delete maxprob_hash;
  
  return hit->nsp_;
}

void InterProphet::progress(int tic, int step, int &tot) {
  
  if (!step || tic % step == 0) {
    tot++;
    if (!step && tot % 10 == 0) {
      cerr << tot;
    }
    else if (tot % 10 == 0) {     
      cerr << tot << "%";
    }
    else {
      cerr << ".";
    }
  }
 
 

}



double InterProphet::getNSPCounts() {
  double out=0;

#ifdef MSVC
  DWORD *pId = new DWORD[max_threads_];
  HANDLE *pHandle = new HANDLE[max_threads_];
#else
  int *pId = new int[max_threads_];
  int *pHandle = new int[max_threads_];
  pthread_t pThreads[max_threads_];
#endif

  thdata data[max_threads_];
  
  unsigned long a = 0;
  cerr << "Creating " << max_threads_ << " threads " << endl;
  cerr << "Wait for threads to finish ..." << endl;
  cerr << "0";    
  for (int a=0; a < 98 ; a++) {
    if (a==50) {
      cerr << a;
    }
    cerr << "-";	
  }      
  cerr << "100%" << endl;
  while(a < max_threads_)	{

    data[a].thread_no = a;
    data[a].ipro = this;
#ifdef MSVC
    pHandle[a] = CreateThread(NULL,0,NSPThread,(void*) &data[a],0, NULL);
#else
    pthread_create(&pThreads[a],NULL,NSPThread, (void*) &data[a]);
#endif
    a++;

  }


  a = 0;
  while(a < max_threads_)  {
#ifdef MSVC
    WaitForSingleObject(pHandle[a],INFINITE);
#else
    pthread_join(pThreads[a],NULL);
#endif
    a++;
  }

  delete [] pId;
  delete [] pHandle;
  
  //if (! (fabs(out) > 0.0001) ) use_nsp_ = false; 

 return out;
}

void InterProphet::computeLengthModel() {
  //getNSPCounts();
  len_model_->clear();
  for (int i=0; i < hit_arr_->length() ; i++) {      
    len_model_->insert((*hit_arr_)[i]->adj_prob_, (*hit_arr_)[i]->peptide_.length() < 15 ? (*hit_arr_)[i]->peptide_.length() : 15 );
  }
  if (len_model_->makeReady(false, 10)) {  //  if (nsp_model_->makeReady(1, 1)) {
    //getNSPAdjProbs();get
  }
}


double InterProphet::getPeptideLengths() {
  double out=0;

#ifdef MSVC
  DWORD *pId = new DWORD[max_threads_];
  HANDLE *pHandle = new HANDLE[max_threads_];
#else
  int *pId = new int[max_threads_];
  int *pHandle = new int[max_threads_];
  pthread_t pThreads[max_threads_];
#endif

  thdata data[max_threads_];
  
  unsigned long a = 0;
  cerr << "Creating " << max_threads_ << " threads " << endl;
  cerr << "Wait for threads to finish ..." << endl;
  cerr << "0";    
  for (int a=0; a < 98 ; a++) {
    if (a==50) {
      cerr << a;
    }
    cerr << "-";	
  }      
  cerr << "100%" << endl;
  while(a < max_threads_)	{

    data[a].thread_no = a;
    data[a].ipro = this;
#ifdef MSVC
    pHandle[a] = CreateThread(NULL,0,LengthThread,(void*) &data[a],0, NULL);
#else
    pthread_create(&pThreads[a],NULL,LengthThread, (void*) &data[a]);
#endif
    a++;

  }


  a = 0;
  while(a < max_threads_)  {
#ifdef MSVC
    WaitForSingleObject(pHandle[a],INFINITE);
#else
    pthread_join(pThreads[a],NULL);
#endif
    a++;
  }

  delete [] pId;
  delete [] pHandle;
  
  //if (! (fabs(out) > 0.0001) ) use_nsp_ = false; 

 return out;
}

void InterProphet::computeNSPModel() {
  //getNSPCounts();
  nsp_model_->clear();
  for (int i=0; i < hit_arr_->length() ; i++) {      
    nsp_model_->insert((*hit_arr_)[i]->adj_prob_, (*hit_arr_)[i]->nsp_);
  }
  if (nsp_model_->makeReady(false, 10)) {  //  if (nsp_model_->makeReady(1, 1)) {
    //getNSPAdjProbs();get
  }
}

void InterProphet::computeFPKMModel() {
  //getFPKMCounts();
  fpkm_model_->clear();
  for (int i=0; i < hit_arr_->length() ; i++) {      
    if ( (*hit_arr_)[i]->fpkm_ > 20) {
      (*hit_arr_)[i]->fpkm_ = 20;
    }
    fpkm_model_->insert((*hit_arr_)[i]->adj_prob_, (*hit_arr_)[i]->fpkm_);
  }
  if (fpkm_model_->makeReady(false, 10)) {  //  if (fpkm_model_->makeReady(1, 1)) {
    //getFPKMAdjProbs();get
  }
}

double InterProphet::getFPKMValue(int runidx, string& exp_lbl, string& spectrum, string& swath_assay ) {
  string key = exp_lbl + spectrum + swath_assay;
  return (*(*hits_hash_)[runidx])[key]->fpkm_;
}


double InterProphet::getNSPValue(int runidx, string& exp_lbl, string& spectrum, string& swath_assay ) {
  string key = exp_lbl + spectrum + swath_assay;
  return (*(*hits_hash_)[runidx])[key]->nsp_;
}


double InterProphet::getNSMCount(SearchHit* hit) {
  int j;
  
    maxprob_hash_->clear();
    hit->nsm_ = 0;

    Array<SearchHit*>* hitarr = (*bypep_tophit_hash_)[hit->peptide_];
    
    for ( j=0; hitarr != NULL && j < hitarr->size(); j++) {
      if ( hit != (*hitarr)[j] &&
	   hit->spect_ != (*hitarr)[j]->spect_ &&
	   hit->peptide_ == (*hitarr)[j]->peptide_ &&
	   fabs(hit->calcnmass_ - (*hitarr)[j]->calcnmass_) > 0.1 * (*hitarr)[j]->peptide_.length() ) {
	
	
	string* key = &(*hitarr)[j]->modpep_;
	
	dbl_hash::iterator max_itr = (*maxprob_hash_).find(*key);
	double nprob = (*(*hits_hash_)[(*hitarr)[j]->run_idx_])[((*hitarr)[j]->exp_+(*hitarr)[j]->spect_)+(*hitarr)[j]->swathpep_]->adj_prob_;
	
	if (max_itr ==  maxprob_hash_->end()) {
	  maxprob_hash_->insert(make_pair(*key, nprob));
	}
	else if  ( (*maxprob_hash_)[*key] < nprob ) {
	  (*maxprob_hash_)[*key] = nprob;
	}

      }
    }
  
    hit->nsm_  = SumDoubleHash(maxprob_hash_);
  
  
  
  
  return hit->nsm_;
}

double InterProphet::getNSMCounts() {
  double out=0;
   int step = hit_arr_->length() / 100;
  int tot = 0;
 for (int i=0; i < hit_arr_->length() ; i++) {      
   out += getNSMCount((*hit_arr_)[i]);
   progress(i, step, tot);
 }
  if (! (fabs(out) > 0.0001) ) use_nsm_ = false; 
 return out;
}



void InterProphet::computeNSMModel() {
  //getNSMCounts();
  nsm_model_->clear();
  for (int i=0; i < hit_arr_->length() ; i++) {      
    nsm_model_->insert((*hit_arr_)[i]->adj_prob_, (*hit_arr_)[i]->nsm_);
  }
  if (nsm_model_->makeReady(false, 10)) {  //  if (nsm_model_->makeReady(1, 1)) {
    //getNSMAdjProbs();get
  }
}

double InterProphet::getNSMValue(int runidx, string& exp_lbl, string& spectrum, string& swath_assay) {
  string key = exp_lbl + spectrum + swath_assay;
  return (*(*hits_hash_)[runidx])[key]->nsm_;
}

/*** Deprecated
void InterProphet::printAdjProbs() {
  cout << "Run\tSpectrum\tPeptide\tPepProphProb\tNSS\tNSSAdjProb\tNRS\tNRSAdjProb\tNSE\tNSEAdjProb\tNSI\tNSIAdjProb\tNSM\tNSMAdjProb" << endl;
  //  for (size_t i=0; i<num_runs_; i++) {
  size_t i=0;
  for (i=0; i<num_runs_; i++) {
    for (hit_hash::iterator i_itr = (*(*hits_hash_)[i]).begin(); 
	 i_itr != (*(*hits_hash_)[i]).end(); i_itr++) {
      cout << *(*search_names_)[i] << "\t" 
	   << i_itr->first << "\t" 
	   << i_itr->second  << "\t" 
	   <<  i_itr->second << "\t" 
	   << (*(*nss_hash_)[i])[i_itr->first] << "\t" 
	   << (*(*nssadjprob_hash_)[i])[i_itr->first] <<  "\t" 
	   << (*(*nrs_hash_)[i])[i_itr->first] << "\t" 
	   << (*(*nrsadjprob_hash_)[i])[i_itr->first] << "\t" 
	   << (*(*nse_hash_)[i])[i_itr->first] << "\t" 
	   << (*(*nseadjprob_hash_)[i])[i_itr->first] << "\t" 
	   << (*(*nsi_hash_)[i])[i_itr->first] << "\t" 
	   << (*(*nsiadjprob_hash_)[i])[i_itr->first] << "\t" 
	   << (*(*nsm_hash_)[i])[i_itr->first] << "\t" 
	   << (*(*nsmadjprob_hash_)[i])[i_itr->first] << endl;
    }
  }
} 
***/


 double InterProphet::getAdjProb(string& exp_lbl, string& spectrum, string& swath_assay) {
  string key = exp_lbl + spectrum + swath_assay;
  return (*(*hits_hash_)[(*top_i_hash_)[key]])[key]->adj_prob_;

}

double InterProphet::getNTTAdjProb(string& exp_lbl, string& spectrum, string& swath_assay, int ntt) {
  string key = exp_lbl + spectrum + swath_assay;
  return (*(*hits_hash_)[(*top_i_hash_)[key]])[key]->allntt_adj_prob_[ntt];
}

string InterProphet::getMsRunStr(int runidx, string& exp_lbl, string& spectrum, string& swath_assay) {
  string key = exp_lbl + spectrum + swath_assay;
  return (*(*hits_hash_)[runidx])[key]->msrun_;
}

InterProphet::~InterProphet() {
  delete maxprob_hash_;
  delete maxprob_sp_hash_;
  delete bypep_nsp_hash_;
  delete exp_hash_;
}



double InterProphet::SumDoubleHash(dbl_hash* & hash) {
  double sum = 0L;
  
  dbl_hash::iterator itr = hash->begin();

  if (itr !=  hash->end()) {
    sum = itr->second + 1L; 
    itr++;
  }
  else {
    return sum;
  }

  for (; itr !=  hash->end(); itr++) {

    sum = exp( log(1 + exp(log(sum)-log(itr->second+1L)) ) + log(itr->second+1L));

  }

  sum = sum - hash->size();

  return sum;
  
}
