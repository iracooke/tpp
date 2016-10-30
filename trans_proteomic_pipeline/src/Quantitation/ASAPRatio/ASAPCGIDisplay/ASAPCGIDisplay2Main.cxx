/*

Program       : ASAPCGIDisplayMain                                                  
Author        : Andrew Keller <akeller@systemsbiology.org> 
                *Xiao-jun Li (xli@systemsbiology.org>                                                      
Date          : 11.27.02 

CGI program for displaying ASAP protein and peptide information
from ProteinProphet XML, and allowing users to make and store
modifications

Copyright (C) 2003 Andrew Keller, Xiao-jun Li

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


/*
 * Web-based ASAPRatio (Date: 09-17-2002)
 * Developed by: Xiao-jun Li (xli@systemsbiology.org)
 */
# include <stdlib.h>
# include <stdio.h>
# include <string.h>
# include <math.h>
# include <ctype.h>
# include <sys/types.h>

//# include "ASAPRatio_prophet.h"
//#include "ASAPCGIDisplayParser.h"
#include "ASAPCGIParser.h"

#include "Quantitation/ASAPRatio/ASAP_structs.h"
#include "ASAPRatioProteinCGIDisplayParser.h"
#include "common/Array.h"
#include "ASAPCGIDisplayParser.h"
#include "Quantitation/ASAPRatio/ASAPRatio_Fns/ASAPRatio_txtFns.h"

#include "common/TPPVersion.h" // contains version number, name, revision
#include "common/util.h"


/************************************************************************
 *
 * Structures
 *
 ************************************************************************/
// field structure in .html
/*
typedef struct {
  char * name;
  char * value;
} htmlFieldStrct;

*/


/************************************************************************
 *
 * Functions
 *
 ************************************************************************/

/*
 * This function evaluates the ratio of a protein.
 */
void ASAPRatio_getProDataStrct(proDataStrct *data, char **pepBofFiles); 

/*
 * For a given set of data, dataErrs, and dataWghs, this function identifies 
 * any outliers and gets the mean and confidence interval of ratio.
 */ 
void getDataRatio(double *ratio, double *error, double *inv_ratio, double *inv_error, double confL, 
		  double *data, double *dataErrs, double *inv_data, double *inv_dataErrs, 
		  double *dataWghs, int *dataIndx, int dataSize, 
		  int testType);

/*
 * This function gets the queryString passed by POST method in a form.
 */
char *getPostQueryString(void);

/*
 * This function frees a proDataStrct.
 */
void freeProDataStrct(proDataStrct data);

/*
 * This function converts a ratio and its error into strings for output.
 */
char **ratioOutput(double ratio[2], int ratioIndx); 



/*
 * This function evaluates the ratio of a protein.
 */
void ASAPRatio_getProDataStrct(proDataStrct *data, char **pepBofFiles) 
{
  void ASAPRatio_getSeqDataStrctRatio(seqDataStrct *data, char **pepBofFiles);

  double *ratios, *errors, *inv_ratios, *inv_errors, *weights;
  int *outliers, *seqIndx;
  int seqNum, tmpNum;
  double cmnErr, ratio, error, inv_ratio, inv_error;
  int num[2] = {0, 0};

  int i, j;

  if (data->indx == -1) {
    data->ratio[0] = -2.;
    data->ratio[1] = 0.;
    data->inv_ratio[0] = -2.;
    data->inv_ratio[1] = 0.;
    return;
  }

  // collect all sequences
  seqIndx = (int *) calloc(data->dataNum, sizeof(int));
  seqNum = 0;
  for (i = 0; i < data->dataNum; ++i) {
    ASAPRatio_getSeqDataStrctRatio(&(data->sequences[i]), pepBofFiles);
    // double check dataCnts
    if(data->dataCnts[i] == 1) {
      data->dataCnts[i] = 0;
      for (j = 0; j < data->sequences[i].dataNum; ++j){
	if(data->sequences[i].dataCnts[j] == 1){
	  data->dataCnts[i] = 1;
	  break;
	}
      }
    }
    if(data->dataCnts[i] == 1 && data->sequences[i].indx != -1) {
      seqIndx[i] = 1;
      ++seqNum;
    }
    else
      seqIndx[i] = 0;
  }

  // only 0 or 1 valid sequences
  if(seqNum < 2) {
    if(seqNum < 1) {
      data->ratio[0] = -2.;
      data->ratio[1] = 0.;
      data->inv_ratio[0] = -2.;
      data->inv_ratio[1] = 0.;
    } // if(seqNum < 1) {
    else {
      for (i = 0; i < data->dataNum; ++i) {
	if(seqIndx[i] == 1) {
	  data->ratio[0] = data->sequences[i].ratio[0];
	  data->ratio[1] = data->sequences[i].ratio[1];
	  data->inv_ratio[0] = data->sequences[i].inv_ratio[0];
	  data->inv_ratio[1] = data->sequences[i].inv_ratio[1];
	  break;
	}
      }
    } // else {

    if(data->indx == 0)
      data->indx = 1;
    
    free(seqIndx);

    return;
  } //if(seqNum < 2) {


  // allocate memory
  ratios = (double *) calloc(seqNum, sizeof(double));
  errors = (double *) calloc(seqNum, sizeof(double));
  inv_ratios = (double *) calloc(seqNum, sizeof(double));
  inv_errors = (double *) calloc(seqNum, sizeof(double));
  weights = (double *) calloc(seqNum, sizeof(double));
  outliers = (int *) calloc(seqNum, sizeof(int));

  // get data
  tmpNum = 0;
  for(i = 0; i < data->dataNum; ++i) {
    if(seqIndx[i] == 1) {
      ratios[tmpNum] = data->sequences[i].ratio[0];
      errors[tmpNum] = data->sequences[i].ratio[1];
      inv_ratios[tmpNum] = data->sequences[i].inv_ratio[0];
      inv_errors[tmpNum] = data->sequences[i].inv_ratio[1];
      ++tmpNum;
    }
  }
    
  // check valid error
  cmnErr = 0.;
  tmpNum = 0;
  for(i = 0; i < seqNum; ++i) {
    if(ratios[i] == 0.) // 0
      ++num[0];
    else if(ratios[i] == -1.) // -1
      ++num[1];
    if(errors[i] > 0.) { // valid error
      ++tmpNum;
      cmnErr += errors[i];
    }
  }// for(i = 0; i < seqNum; ++i) {
  
  if(tmpNum < 1) { // all ratios are 0 or -1 or -2
    if(num[0] > num[1]) {// more 0 than -1
      ratio = 0.;
      inv_ratio = 999.;
    }
    else if(num[0] < num[1]) { // more -1 than 0
      ratio = -1.;
      inv_ratio = -1.;
    }
    else {// same 0 and -1
      ratio = -2.;
      inv_ratio = -2.;
    }

    error = 0.;
    inv_error = 0.;
    
    // get outliers
    for (i = 0; i < seqNum; ++i) {
      if(ratios[i] == ratio)
	outliers[i] = 0;
      else
	outliers[i] = 1;
    }
    
    // store ratios, etc
    data->ratio[0] = ratio;
    data->ratio[1] = error;

    data->inv_ratio[0] = inv_ratio;
    data->inv_ratio[1] = inv_error;

    if(data->indx == 0) {
      tmpNum = 0;
      for(i = 0; i < data->dataNum; ++i) {
	if(seqIndx[i] == 1) {
	  data->dataCnts[i] = 1 - outliers[tmpNum];
	  ++tmpNum;
	}
      }
      data->indx = 1;
    }
    
    free(seqIndx);
    free(ratios);
    free(errors);
    free(inv_ratios);
    free(inv_errors);
    free(weights);
    free(outliers);

    return;
  }
  else // get ave err
    cmnErr /= tmpNum;


  // get weight
  for(i = 0; i < seqNum; ++i) {
    if(errors[i] <= 0.)
      weights[i] = 1./cmnErr/cmnErr;
    else if(errors[i] < cmnErr/4.)
      weights[i] = 16./cmnErr/cmnErr;
    else
      weights[i] = 1./errors[i]/errors[i];
  }// for(i = 0; i < currPepNum; ++i) {
  
  // calculate ratio and error
  if(data->indx == 0) {
    getDataRatio(&(data->ratio[0]), &(data->ratio[1]), &(data->inv_ratio[0]), &(data->inv_ratio[1]), _ASAPRATIO_CONFL_,
		 ratios, errors, inv_ratios, inv_errors, weights, outliers, seqNum, 1);
    tmpNum = 0;
    for(i = 0; i < data->dataNum; ++i) {
      if(seqIndx[i] == 1) {
	data->dataCnts[i] = 1 - outliers[tmpNum];
	++tmpNum;
      }
    }      
  }
  else
    getDataRatio(&(data->ratio[0]), &(data->ratio[1]), &(data->inv_ratio[0]), &(data->inv_ratio[1]), _ASAPRATIO_CONFL_,
		 ratios, errors, inv_ratios, inv_errors, weights, outliers, seqNum, 0);

  // reset indx
  if(data->indx == 0)
    data->indx = 1;

   // free memory
  free(seqIndx);
  free(ratios);
  free(errors);
  free(inv_ratios);
  free(inv_errors);
  free(weights);
  free(outliers);

  return;
}


/*
 * This function calculates the ratio of a unique sequence.
 */
void ASAPRatio_getSeqDataStrctRatio(seqDataStrct *data, char **pepBofFiles)
{
    void ASAPRatio_getDataStrctRatio(dataStrct *data, char *pepBofFile);

  double *ratios, *errors, *inv_ratios, *inv_errors, *weights;
  int *outliers, *peakIndx;
  int peakNum, tmpNum;

  int i, j;

  if (data->indx == -1) {
    data->ratio[0] = -2.;
    data->ratio[1] = 0.;
    data->inv_ratio[0] = -2.;
    data->inv_ratio[1] = 0.;
    return;
  }

  // collect all peaks
  peakIndx = (int *) calloc(data->dataNum, sizeof(int));
  peakNum = 0;
  for (i = 0; i < data->dataNum; ++i) {
    ASAPRatio_getDataStrctRatio(&(data->peaks[i]), pepBofFiles[data->peaks[i].bofIndx]);
    // double check dataCount
    if(data->dataCnts[i] == 1) {
      data->dataCnts[i] = 0;
      for (j = 0; j < data->peaks[i].dataNum; ++j){
	if(data->peaks[i].dataCnts[j] == 1){
	  data->dataCnts[i] = 1;
	  break;
	}
      }
    }
    if(data->dataCnts[i] == 1 && data->peaks[i].indx != -1) {
      peakIndx[i] = 1;
      ++peakNum;
    }
    else
      peakIndx[i] = 0;
  }

  // only 0 or 1 valid peaks
  if(peakNum < 2) {
    if(peakNum < 1) {
      data->ratio[0] = -2.;
      data->ratio[1] = 0.;
      data->inv_ratio[0] = -2.;
      data->inv_ratio[1] = 0.;
    } // if(peakNum < 1) {
    else {
      for (i = 0; i < data->dataNum; ++i) {
	if(peakIndx[i] == 1) {
	  data->ratio[0] = data->peaks[i].ratio[0];
	  data->ratio[1] = data->peaks[i].ratio[1];
	  data->inv_ratio[0] = data->peaks[i].inv_ratio[0];
	  data->inv_ratio[1] = data->peaks[i].inv_ratio[1];
	  break;
	}
      }
    } // else {

    if(data->indx == 0)
      data->indx = 1;
    
    free(peakIndx);

    return;
  } //if(peakNum < 2) {


  // allocate memory
  ratios = (double *) calloc(peakNum, sizeof(double));
  errors = (double *) calloc(peakNum, sizeof(double));
  inv_ratios = (double *) calloc(peakNum, sizeof(double));
  inv_errors = (double *) calloc(peakNum, sizeof(double));
  weights = (double *) calloc(peakNum, sizeof(double));
  outliers = (int *) calloc(peakNum, sizeof(int));

  // get data
  tmpNum = 0;
  for(i = 0; i < data->dataNum; ++i) {
    if(peakIndx[i] == 1) {
      ratios[tmpNum] = data->peaks[i].ratio[0];
      errors[tmpNum] = data->peaks[i].ratio[1];
      inv_ratios[tmpNum] = data->peaks[i].inv_ratio[0];
      inv_errors[tmpNum] = data->peaks[i].inv_ratio[1];
      weights[tmpNum] = data->peaks[i].weight;
      ++tmpNum;
    }
  }
    
  // calculate ratio and error
  if(data->indx == 0) {
    getDataRatio(&(data->ratio[0]), &(data->ratio[1]), &(data->inv_ratio[0]), &(data->inv_ratio[1]), _ASAPRATIO_CONFL_,
		 ratios, errors, inv_ratios, inv_errors, weights, outliers, peakNum, 1);
    tmpNum = 0;
    for(i = 0; i < data->dataNum; ++i) {
      if(peakIndx[i] == 1) {
	data->dataCnts[i] = 1 - outliers[tmpNum];
	++tmpNum;
      }
    }      
  }
  else
    getDataRatio(&(data->ratio[0]), &(data->ratio[1]), &(data->inv_ratio[0]), &(data->inv_ratio[1]), _ASAPRATIO_CONFL_,
		 ratios, errors, inv_ratios, inv_errors, weights, outliers, peakNum, 0);
  
  // reset indx
  if(data->indx == 0)
    data->indx = 1;

    // free memory
  free(peakIndx);
  free(ratios);
  free(errors);
  free(inv_ratios);
  free(inv_errors);
  free(weights);
  free(outliers);

  return;
}


/*
 * This function calculates the ratio of a unique peak.
 */
void ASAPRatio_getDataStrctRatio(dataStrct *data, char *pepBofFile)
{
  pepDataStrct *peptides=NULL;
   double *ratios, *errors, *inv_ratios, *inv_errors, *weights;
  int *outliers, *pepIndx=NULL;
  int pepNum=-1, tmpNum;
  double tol = 1.e-2;

  int i;

  if (data->indx == -1) {
    data->ratio[0] = -2.;
    data->ratio[1] = 0.;
    data->inv_ratio[0] = -2.;
    data->inv_ratio[1] = 0.;
    return;
  }


  /*
    collect all valid peptides
  */
  // open pepBofFile
  /*
  if((fin = fopen(pepBofFile, "r")) == NULL) {  
    printf("Cannot open file \"%s\" for reading.\n", pepBofFile);
    data->ratio[0] = -2.;
    data->ratio[1] = 0.;
    data->weight = 0.;
    return;
  }
  */

  
  // read peptides
  peptides = (pepDataStrct *) calloc(data->dataNum, sizeof(pepDataStrct));
  /*
  peptides = (dataStrct *) calloc(data->dataNum, sizeof(dataStrct));


  pepIndx = (int *) calloc(data->dataNum, sizeof(int));
  pepNum = 0;
  for (i = 0; i < data->dataNum; ++i) {
    if(data->dataCnts[i] == 1) { 
      //fseek(fin, data->dataIndx[i]*sizeof(pepDataStrct), SEEK_SET);
      //fread(peptides+pepNum, sizeof(pepDataStrct), 1, fin);
      if(peptides[pepNum].indx != -1) {
	pepIndx[i] = 1;
	++pepNum;
      }
      else 
	pepIndx[i] = 0;
    }
    else
      pepIndx[i] = 0;
  }
  */
  //  fclose(fin);
  

  /*
   * calculate ratio
   */
  // only 0 or 1 valid peptides
  if(pepNum < 2) {
    if(pepNum < 1) {
      data->ratio[0] = -2.;
      data->ratio[1] = 0.;
      data->inv_ratio[0] = -2.;
      data->inv_ratio[1] = 0.;
      data->weight = 0.;
    } // if(pepNum < 1) {
    else {
      data->ratio[0] = peptides[0].pepRatio[0];
      data->ratio[1] = peptides[0].pepRatio[1];
      data->inv_ratio[0] = peptides[0].pepH2LRatio[0];
      data->inv_ratio[1] = peptides[0].pepH2LRatio[1];
      data->weight = peptides[0].pepArea;
    } // else {
    if(data->indx == 0) {
      for (i = 0; i < data->dataNum; ++i) {
//	if(data->dataCnts[i] == 1 && pepIndx[i] == 0)  
	if(data->dataCnts[i] == 1) 
	  data->dataCnts[i] = 0;
      }
      data->indx = 1;
    }

    if ( pepIndx != NULL )
      free(pepIndx);
    if ( peptides != NULL )
      free(peptides);

    return;
  } //if(pepNum < 2) {

  // allocate memory
  ratios = (double *) calloc(pepNum, sizeof(double));
  errors = (double *) calloc(pepNum, sizeof(double));
  inv_ratios = (double *) calloc(pepNum, sizeof(double));
  inv_errors = (double *) calloc(pepNum, sizeof(double));
  weights = (double *) calloc(pepNum, sizeof(double));
  outliers = (int *) calloc(pepNum, sizeof(int));

  // get data
  for(i = 0; i < pepNum; ++i) {
    ratios[i] = peptides[i].pepRatio[0];
    errors[i] = peptides[i].pepRatio[1];
    inv_ratios[i] = peptides[i].pepH2LRatio[0];
    inv_errors[i] = peptides[i].pepH2LRatio[1];
    weights[i] = peptides[i].pepArea;
  }
    
  // calculate ratio and error
  if(data->indx == 0) {
    getDataRatio(&(data->ratio[0]), &(data->ratio[1]), &(data->inv_ratio[0]), &(data->inv_ratio[1]), _ASAPRATIO_CONFL_,
		 ratios, errors, inv_ratios, inv_errors, weights, outliers, pepNum, 1);
    tmpNum = 0;
    for (i = 0; i < data->dataNum; ++i) {
      if(pepIndx[i] == 1) {
	data->dataCnts[i] = 1 - outliers[tmpNum];
	++tmpNum;
      }
    }      
  }
  else
    getDataRatio(&(data->ratio[0]), &(data->ratio[1]), &(data->inv_ratio[0]), &(data->inv_ratio[1]),_ASAPRATIO_CONFL_,
		 ratios, errors, inv_ratios, inv_errors, weights, outliers, pepNum, 0);
  
  // weight
  data->weight = 0.;
  for (i = 0; i < pepNum; ++i) {
    if(outliers[i] == 0 && weights[i] > data->weight)
      data->weight = weights[i];
  }

  // if identical ratios
  tmpNum = 0;
  for (i = 0; i < pepNum; ++i) {
    if(outliers[i] == 0 
       && fabs(ratios[i]-data->ratio[0]) > tol*data->ratio[0]) {
      ++tmpNum;
    }
  }
  if(tmpNum == 0) {
    data->ratio[1] = 0.;
    data->inv_ratio[1] = 0.;
    tmpNum = 0;
    for (i = 0; i < pepNum; ++i) {
      if(outliers[i] == 0) {
	data->ratio[1] += errors[i];
	data->inv_ratio[1] += inv_errors[i];
	++tmpNum;
      }
    }
    if(tmpNum > 0){
      data->ratio[1] /= tmpNum;
      data->inv_ratio[1] /= tmpNum;
    
    }   
    else {
      for (i = 0; i < pepNum; ++i) {
	if(outliers[i] == 0) {
	  data->ratio[1] = errors[i];
	  data->inv_ratio[1] = inv_errors[i];
	  break;
	}
      }
    }
  } // if(tmpNum == 0) {
  
  // reset indx
  if(data->indx == 0)
    data->indx = 1;

  // free memory
  free(peptides);
  free(pepIndx);
  free(ratios);
  free(errors);
  free(inv_ratios);
  free(inv_errors);
  free(weights);
  free(outliers);

  return;
}


/*
 * For a given set of data, dataErrs, and dataWghs, this function identifies 
 * any outliers and gets the mean and confidence interval of ratio.
 */
void getDataRatio(double *ratio, double *error, double *inv_ratio, double *inv_error, double confL, 
		  double *data, double *dataErrs, double *inv_data, double *inv_dataErrs, 
		  double *dataWghs, int *dataIndx, int dataSize, 
		  int testType)
{
  
  int counts[4] = {0, 0, 0, 0};
  double *dtRatios, *dtErrors, *dt_inv_Ratios, *dt_inv_Errors,*dtWeights;
  int *dtIndx;
  int pass;
  int count, vldNum;
  double sum, inv_sum;
  double tmpError, tmp_inv_Error;
  double acc = 0.01;
  int i, j;


  // check whether there are valid data
  for(i = 0; i < dataSize; ++i) {
    if(data[i] == 0.) 
      ++counts[0];
    else if(data[i] == -1.) 
      ++counts[1];
    else if(data[i] == -2.) 
      ++counts[2];
    else
      ++counts[3];
  }

  // easy output for invalid data set
  if(counts[3] < 1) {
    if(counts[0] > counts[1]) {
      *ratio = 0.;
      *error = 0.;    
      *inv_ratio = 999.;
      *inv_error = 0.;
      if(testType == 1) {
	for (i = 0; i < dataSize; ++i) 
	  if(data[i] != 0.)
	    dataIndx[i] = 1;
	  else
	    dataIndx[i] = 0;
      }
    }
    else if(counts[0] < counts[1]) {
      *ratio = -1.;
      *error = 0.;
      *inv_ratio = -1.;
      *inv_error = 0.;
      if(testType == 1) {
	for (i = 0; i < dataSize; ++i) 
	  if(data[i] != -1.)
	    dataIndx[i] = 1;
	  else
	    dataIndx[i] = 0;
      }
    }
    else {
      *ratio = -2.;
      *error = 0.;     
      *inv_ratio = -2.;
      *inv_error = 0.;
      if(testType == 1) {
	for (i = 0; i < dataSize; ++i) 
	  if(data[i] != -2.)
	    dataIndx[i] = 1;
	  else
	    dataIndx[i] = 0;
      }
    }
    return;
  } // if(count[3] < 1) {

  // allocate memory
  dtRatios = (double *) calloc(dataSize, sizeof(double));
  dtErrors = (double *) calloc(dataSize, sizeof(double));
  dt_inv_Ratios = (double *) calloc(dataSize, sizeof(double));
  dt_inv_Errors = (double *) calloc(dataSize, sizeof(double));
  dtWeights = (double *) calloc(dataSize, sizeof(double));
  dtIndx = (int *) calloc(dataSize, sizeof(int));


  /*
   * identify outliers
   */
  // collect valid data, transform into log(ratio)
  for (i = 0; i < dataSize; ++i) 
    if(data[i] > 0.)
      dataIndx[i] = 0;
    else
      dataIndx[i] = 1;

  count = 0;
  for (i = 0; i < dataSize; ++i) {
    if(dataIndx[i] == 0) {
      pass = 1;
      for (j = 0; pass == 1 && j < count; ++j) {
	if(fabs(log(data[i])-dtRatios[j]) < acc*dtRatios[j]){
	  pass = 0;
	}
      }
      if(pass == 1) {
	dtRatios[count] = log(data[i]);	
	dt_inv_Ratios[count] = log(inv_data[i]);
	++count;
      }
    }//if(dataIndx[i] == 0) {
  } //for (i = 0; i < dataSize; ++i) {

  // identify any outliers
  if(testType != 0)
    DixonTest(dtRatios, dtIndx, count);
  else
    for (i = 0; i < count; ++i)
      dtIndx[i] = 0;

  for (i = 0; i < dataSize; ++i) {
    if(dataIndx[i] == 0) {
      pass = 1;
      for (j = 0; pass == 1 && j < count; ++j) {
	if(dtIndx[j] == 1 
	   && fabs(log(data[i])-dtRatios[j]) < acc*dtRatios[j]){
	  pass = 0;
	}
      }
      if(pass == 0) 
	dataIndx[i] = 1;
    } // if(dataIndx[i] == 0) {
  } //for (i = 0; i < dataSize; ++i) {


  /*
   * get ratio and error
   */
  // collect valid date
  count = 0; 
  for (i = 0; i < dataSize; ++i) {
    if(dataIndx[i] == 0) { // not an outlier
      dtRatios[count] = data[i];
      dtErrors[count] = dataErrs[i];
      dt_inv_Ratios[count] = inv_data[i];
      dt_inv_Errors[count] = inv_dataErrs[i];
      dtWeights[count] = dataWghs[i];
      ++count;
    }
  } //for (i = 0; i < dataSize; ++i) {

  // calculate ratio and error
  if(count < 1) { // no valid data
    *ratio = -2.;
    *error = 0.;
    *inv_ratio = -2.;
    *inv_error = 0.;
  } //if(count < 1) { // no valid data
  else if(count == 1) { // only one valid data
    *ratio = dtRatios[0];
    *error = dtErrors[0];
    *inv_ratio = dt_inv_Ratios[0];
    *inv_error = dt_inv_Errors[0];
  }//  else if(count == 1) { // only one valid data
  else {
    // transform into log(ratio)
    for (i = 0; i < count; ++i) {
      dtErrors[i] /= dtRatios[i];
      dtRatios[i] = log(dtRatios[i]);
      dt_inv_Errors[i] /= dt_inv_Ratios[i];
      dt_inv_Ratios[i] = log(dt_inv_Ratios[i]);
    }
    // calculate the light:heavy ratio by weight 
    findMeanAndStdDevWeight(ratio, error, dtRatios, inv_ratio, inv_error, dt_inv_Ratios, dtWeights, count);

    sum = 0.;
    inv_sum = 0.;
    vldNum = 0;
    for (i = 0; i < count; ++i) {
      if(dtErrors[i] > 0.) {
	sum += 1./dtErrors[i]/dtErrors[i];
	inv_sum += 1./dt_inv_Errors[i]/dt_inv_Errors[i];
	++vldNum;
      }
    }
    if(vldNum > 0) {
      tmpError = 1./sqrt(sum);
      tmp_inv_Error = 1./sqrt(inv_sum);
    } // if(vldNum > 0) {
    else {
      tmpError = 0.;
      tmp_inv_Error = 0.; 
    }
    
    *error = sqrt((*error)*(*error)+tmpError*tmpError);
    *inv_error = sqrt((*inv_error)*(*inv_error)+tmp_inv_Error*tmp_inv_Error);
    
    *inv_ratio = exp(*inv_ratio);
    *inv_error *= (*inv_ratio);

    // transform back to ratio
    *ratio = exp(*ratio);
    *error *= (*ratio);
  }//else {
  
  // free memory
  free(dtRatios);
  free(dtErrors);
  free(dt_inv_Ratios);
  free(dt_inv_Errors);
  free(dtWeights);
  free(dtIndx);

  return;
}


/*
 * This function gets the queryString passed by POST method in a form.
 */
char *getPostQueryString(void)
{
  char *queryStr; 
  char *queryLenStr = getenv("CONTENT_LENGTH");
  long queryLngth;
  char *tmpStr;

  if(queryLenStr != NULL 
     && sscanf(queryLenStr, "%ld", &queryLngth) == 1
     && queryLngth > 0) {

    if ( (queryStr = (char *) calloc(queryLngth+1, sizeof(char)))==NULL)
    {
      printf(" error calloc queryStr(%ld)<<BR>\n", queryLngth); fflush(stdout);
      exit(1);
    }

    // get tmpStr
    if ( (tmpStr = (char *) calloc(queryLngth+1, sizeof(char)))==NULL)
    {
      printf(" error calloc tmpStr(%ld)<<BR>\n", queryLngth); fflush(stdout);
      exit(1);
    }

    size_t nread = fread(tmpStr, sizeof(char), queryLngth, stdin);
    tmpStr[queryLngth] = '\0';

    strcpy(queryStr, tmpStr);
    free(tmpStr);

    plustospace(queryStr);
    unescape_url(queryStr);

    return queryStr;
  }
  else
    return NULL;

}


/*
 * This function converts a ratio and its error into strings for output.
 */
char **ratioOutput(double ratio[2], int ratioIndx) 
{
  char **ratioStrings;
  int i;

  ratioStrings = (char **) calloc(3, sizeof(char *));
  for (i = 0; i < 3; ++i) {
    ratioStrings[i] = (char *) calloc(10, sizeof(char));
    ratioStrings[i][0] = '\0';
  }

  if(ratio[0] == -2.) { // 0.00 : 0.00
    for (i = 0; i < 3; ++i)
      strcpy(ratioStrings[i], "-1.00");
  }
  else if(ratio[0] == -1.) { // 1.00 : 0.00
    if(ratioIndx == 1) 
      strcpy(ratioStrings[0], "0.00");
    else if(ratioIndx == 2) 
      strcpy(ratioStrings[0], "INF:1");
    else
      strcpy(ratioStrings[0], "9999.");
    for (i = 1; i < 3; ++i)
      strcpy(ratioStrings[i], "0.00");
  }
  else if(ratio[0] == 0.) { // 0.00 : 1.00
    if(ratioIndx == 1) 
      strcpy(ratioStrings[0], "9999.");
    else if(ratioIndx == 2) 
      strcpy(ratioStrings[0], "1:INF");
    else
      strcpy(ratioStrings[0], "0.00");
    for (i = 1; i < 3; ++i)
      strcpy(ratioStrings[i], "0.00");
  }
  else { // normal
    if(ratioIndx == 1) {
      sprintf(ratioStrings[0], "%.2f", 1./ratio[0]);
      sprintf(ratioStrings[1], "%.2f", ratio[1]/ratio[0]/ratio[0]);
    }
    else if(ratioIndx == 2) {
      if(ratio[0] > 1.) {
	sprintf(ratioStrings[0], "%.2f:1", ratio[0]);
	sprintf(ratioStrings[1], "%.2f", ratio[1]);
      }
      else {
	sprintf(ratioStrings[0], "1:%.2f", 1./ratio[0]);
	sprintf(ratioStrings[1], "%.2f", ratio[1]/ratio[0]/ratio[0]);
      }
    }
    else {
      sprintf(ratioStrings[0], "%.2f", ratio[0]);
      sprintf(ratioStrings[1], "%.2f", ratio[1]);
    }
    if(100.*ratio[1]/ratio[0] < 10)
      sprintf(ratioStrings[2], "%.1f", 100.*ratio[1]/ratio[0]);
    else 
      sprintf(ratioStrings[2], "%.0f", 100.*ratio[1]/ratio[0]);
  }

  
  return ratioStrings;
}


int main(int argc, char **argv)
{
  hooks_tpp handler(argc,argv); // set up install paths etc
  proDataStrct *getProDataStrctFromQueryString(char *queryString);
  void displayProDataStrctInCgi(proDataStrct protein, Normalization* norm, char* pval_link, 
				char *xmlFile, char *cgiAction, 
				char **htmlFiles, char **bofFiles, int fileNum, 
				char *proName, int ratioType, double *accRatio,  double *accH2LRatio, 
				ASAPRatioProteinCGIDisplayParser* parser, 
				char* colored_aas,unsigned int group_no);
  
  // cgi variables
  char *queryString;
  char cgiAction[1000];
  int queryIndx;

  // parameters
  char *xmlFile=NULL; // ProtienProphet xml file
  char *proName=NULL; // protein name
  proDataStrct *protein=NULL;
  Normalization* norm=NULL;
  char* pval_link=NULL;
  char **htmlFiles=NULL;
  int fileNum;
  int ratioType;
  unsigned int group_index=1;	//for "nexting" through the proteins without closing the ASAP viewer

  // variables
  char **bofFiles; // .bof file
  double accRatio[2];
  double accH2LRatio[2];
  int wrtIndx;
  char directory[1000];
  char tmpString[_MXSTRLEN_];
  char *tmpValue, *tmpField;
  char* colored_aas = NULL;

  int i, j;

  Array<char*>* inputfiles = NULL;

  ASAPRatioProteinCGIDisplayParser* parser = NULL;
  ASAPCGIDisplayParser* displayparser = NULL;

  Parser* overwriteparser = NULL;

  /*
   * html header, style-sheet, and javascript
   */
  printf("Content-type: text/html\n\n");
  printf("<html>\n<head>\n");
  printf("<title>Automated Statistical Analysis on Protein Abundance Ratio (%s)</title>\n",szTPPVersionInfo);

  printf("<SCRIPT LANGUAGE=\"JavaScript\">\n");

  printf("    function showhide(spectrum_tr){\n");
  printf("	if (document.getElementById(spectrum_tr).className == 'showit') {\n");
  printf("	  new_state = 'hideit';\n");
  printf("	} else {\n");
  printf("	  new_state = 'showit';\n");
  printf("      }\n");
  printf("	document.getElementById(spectrum_tr).className = new_state;\n");
  printf("    }\n");
  // maybe combine these two at some point...
  printf("    function showdiv(div_id){\n");
  printf("	if (document.getElementById(div_id).className == 'formentry') {\n");
  printf("	  new_state = 'hideit';\n");
  printf("	} else {\n");
  printf("	  new_state = 'formentry';\n");
  printf("      }\n");
  printf("	document.getElementById(div_id).className = new_state;\n");
  printf("    }\n");

  printf("    function expandAll(peparray,viz){\n");
  printf("      if(viz == 'expand') { new_state = 'showit';}\n");
  printf("      else { new_state = 'hideit';}\n");
  printf("      for(x in peparray) {\n");
  printf("          document.getElementById(peparray[x]).className = new_state;\n");
  printf("      }\n");
  printf("    }\n");

  printf("</SCRIPT>\n");
  printf("</head>\n\n");

  printf("<body bgcolor=\"#c0c0c0\" onload=\"self.focus();\">\n");

  printf("<style type=\"text/css\">\n");
  printf(".hideit {display:none}\n");
  printf(".showit {display:table-row}\n");
  //  printf(".showdiv{display:block}\n");
  printf(".accepted {background: #87ff87; font-weight:bold;}\n");
  printf(".rejected {background: #ff8700;}\n");

  printf("body{font: small sans-serif; }");
  printf("h1  {font-family: Helvetica, Arial, Verdana, sans-serif; font-size: 24pt; font-weight:bold; color:#0E207F}\n");
  printf("h2  {font-family: Helvetica, Arial, sans-serif; font-size: 20pt; font-weight: bold; color:#0E207F}\n");
  printf("h3  {font-family: Helvetica, Arial, sans-serif; font-size: 16pt; color:#FF8700}\n");
  printf("h4  {font-family: Helvetica, Arial, sans-serif; font-size: 14pt; color:#0E207F}\n");
  printf("h5  {font-family: Helvetica, Arial, sans-serif; font-size: 10pt; color:#AA2222}\n");
  printf("h6  {font-family: Helvetica, Arial, sans-serif; font-size:  8pt; color:#333333}\n");

  printf("table   {border-collapse: collapse; border-color: #000000;}\n");
  printf("td      {border-collapse: collapse; border-color: #000000;}\n");

  printf(".banner_cid   {\n");
  printf("                 background: #0e207f;\n");
  printf("                 border: 2px solid #0e207f;\n");
  printf("                 color: #eeeeee;\n");
  printf("                 font-weight:bold;\n");
  printf("              }\n");
  printf(".banner2      {\n");
  printf("                 background: #aaaaaa;\n");
  printf("                 color: black;\n");
  printf("              }\n");
  printf(".banner1      {\n");
  printf("                 background: #FF8700;\n");
  printf("                 color: black;\n");
  printf("              }\n");
  printf(".formentry    {\n");
  printf("                 background: #eeeeee;\n");
  printf("                 border: 2px solid #0e207f;\n");
  printf("                 color: black;\n");
  printf("                 padding: 1em;\n");
  printf("              }\n");
  printf(".nav          {\n");
  printf("                 background: #c0c0c0;\n");
  printf("                 font-family: Helvetica, Arial, Verdana, sans-serif;\n");
  printf("                 font-weight:bold;\n");
  printf("              }\n");
  printf(".navselected  {\n");
  printf("                 background: #dddddd;\n");
  printf("                 border: 1px solid black;\n");
  printf("                 border-top: 2px solid black;\n");
  printf("                 font-family: Helvetica, Arial, Verdana, sans-serif;\n");
  printf("                 font-weight:bold;\n");
  printf("              }\n");
  printf("</style>\n\n");

  printf("<h1>ASAPRatio: Protein Ratio</h1>\n");
  fflush(stdout);


  /*
   * collect information
   */

  // get queryString
  if(strcmp(getenv("REQUEST_METHOD"), "GET") == 0) {
    char *queryOrig;
    queryIndx = 0;
    if ((queryOrig=getenv("QUERY_STRING"))==NULL)
       queryString=NULL;
    else
    {
       queryString=new char[strlen(queryOrig)+1];
       strcpy(queryString, queryOrig);
       plustospace(queryString);
       unescape_url(queryString);
    }

  } //if(strcmp(getenv("REQUEST_METHOD"), "GET") == 0) {
  else {
    queryIndx = 1;
    queryString = getPostQueryString();
  }
  if(queryString == NULL) {
    printf("<font color=\"red\">Error in passing parameters from web.</font><br/>\n");
    printf("</body></html>\n");
    fflush(stdout);
    return 1;
  }

  // get cgiAction
  if((tmpValue = getenv("SCRIPT_NAME")) != NULL) {
    sprintf(cgiAction, "%s", tmpValue);
  }
  else {
    printf("<font color=\"red\">Cannot find SCRIPT_NAME. </font><br/>\n");
    printf("</body></html>\n");
    fflush(stdout);
//  if(queryIndx == 1)
      free(queryString);
    return 1;
  }

  
  // ratioType
  if ((tmpValue = getHtmlFieldValue("ratioType", queryString)) != NULL) {
    if(sscanf(tmpValue, "%d", &ratioType) != 1
       || ratioType < 0 
       || ratioType > 2) 
      ratioType = 0;
    free(tmpValue);
  }
  else {
    ratioType = 0;
  }
  
  /*
   * collect parameters from web file
   */

  // collect information from ProteinProphet
  if(queryIndx == 0) {
    // xmlFile
    if((xmlFile = getHtmlFieldValue("xmlFile", queryString)) == NULL){
      printf("<font color=\"red\">No input for xml file!</font><br/>\n");
      printf("</body></html>\n");
      fflush(stdout);
      return 1;
    }

	if((tmpValue = getHtmlFieldValue("group_no", queryString)) != NULL)
	{
	  group_index = atoi(tmpValue);
	  //group_no = new char[strlen(tmpValue)+1];
	  
	  
	}
	else
	{
      printf("<font color=\"red\">No input for group number!</font><br/>\n");
      printf("</body></html>\n");
      fflush(stdout);
      return 1;
	}

    // proName
    if((proName = getHtmlFieldValue("protein", queryString)) == NULL){
      printf("<font color=\"red\">No input for protein name!</font><br/>\n");
      printf("</body></html>\n");
      fflush(stdout);
      free(xmlFile);
      return 1;
    }

    if((tmpValue = getHtmlFieldValue("submit", queryString)) != NULL && (strcmp(tmpValue, "Next_Protein") == 0)){
      displayparser = new ASAPCGIDisplayParser(xmlFile, group_index+1);
      proName = strdup(displayparser->getName());
    }
    else if((tmpValue = getHtmlFieldValue("submit", queryString)) != NULL && (strcmp(tmpValue, "Prev_Protein") == 0)){
      displayparser = new ASAPCGIDisplayParser(xmlFile, group_index-1);
      proName = strdup(displayparser->getName());
    }
    else    
      displayparser = new ASAPCGIDisplayParser(xmlFile, proName, group_index);

    if((colored_aas = getHtmlFieldValue("markAA", queryString)) == NULL){
      //printf("<font color=\"red\">No input for protein name!</font><br/>\n");
      //printf("</BODY></HTML>\n");
      //fflush(stdout);
      //free(xmlFile);
      //return 1;
    }
    else {
      //cout << "colored: " << colored_aas << endl; 
      //exit(1);
    }

    /*
     * Andy, check this section 
     */

//  cout << "here ready to start...protein: " << proName << " and xml: " << xmlFile << endl;

//strcpy(proName, "sp|Q29443|TRFE_BOVIN");

//    cout << "here..." << endl;
    
    

    if(displayparser == NULL) {
      printf("<BR>Error - protein2==NULL<BR>\n"); fflush(stdout);
      exit(1);
    }

    norm = displayparser->getNormalized();
    if (norm == NULL) {
      printf("<BR>Warning - normalization info not found<BR>\n"); fflush(stdout);
    }
    pval_link = displayparser->getPvalLink();
    //    cout << "here2..." << endl; 
    // proDataStrct
    protein = displayparser->getProDataStrct();
    //    cout << "DDS1: ProteinH2L=" << protein->inv_ratio[0] << " ProteinH2LERR=" << protein->inv_ratio[1] << endl;

    if (protein==NULL) {
      printf("<BR>Error - protein==NULL<BR>\n"); fflush(stdout);
      exit(1);
    }

    //    cout << "non-null prot" << endl;

    accRatio[0] = protein->ratio[0];
    accRatio[1] = protein->ratio[1];

    accH2LRatio[0] = protein->inv_ratio[0];
    accH2LRatio[1] = protein->inv_ratio[1];

    inputfiles = displayparser->getInputFilesArray(); //new Array<char*>;
    htmlFiles = new char*[inputfiles->length()+1];
    for(int k = 0; k < inputfiles->length(); k++)
      htmlFiles[k] = (*inputfiles)[k];
    htmlFiles[inputfiles->length()] = strdup(""); // null term

    //    cout << "here and ok" << endl;
    //if(protein == NULL)
    //cout << "null protein" << endl;

    parser = new ASAPRatioProteinCGIDisplayParser(protein, inputfiles, ratioType == 1, colored_aas);

    //    cout << "DDS3: Protein=" << protein->inv_ratio[0] << " Protein=" << protein->inv_ratio[1] << endl;
    
    //    cout << "and here" << endl;
    if(parser == NULL) {
      printf("<BR>Error in protein display<BR>\n"); fflush(stdout);
      exit(1);
    }

    //cout << "there" << endl;
    // htmlFiles
    //htmlFiles = displayparser->getInputFiles();

    /*
     * end check section
     */


    // fileNum
    fileNum = inputfiles->length(); //0;

    //while(strlen(htmlFiles[fileNum]) > 0)
    // ++fileNum;

    // accepted ratio
    //accRatio[0] = protein->ratio[0];
    //accRatio[1] = protein->ratio[1];

  } //   if(queryIndx == 0) {


  // collect information from ASAPRatio CGI
  if(queryIndx == 1) {
    // xmlFile
    if((xmlFile = getHtmlFieldValue("xmlFile", queryString)) == NULL){
      printf("<font color=\"red\">No input for xml file!</font><br/>\n");
      printf("</body></html>\n");
      fflush(stdout);
      return 1;
    }

    // pvalLile
    //if((pval_link = getHtmlFieldValue("pvalLink", queryString)) == NULL){
    //  printf("<font color=\"red\">Warning no p-value file!</font><br/>\n");
    //  fflush(stdout);
    //}

	if((tmpValue = getHtmlFieldValue("group_no", queryString)) != NULL)
	 {
	 group_index = atoi(tmpValue);
	 }
	else
	 {
	printf("<font color=\"red\">No input for group number: ");
	  printf("%s",queryString);
	  printf("</font><br/>\n");
      printf("</body></html>\n");
      fflush(stdout);
      return 1;
	 }

    // proName
    if((proName = getHtmlFieldValue("proName", queryString)) == NULL){
      printf("<font color=\"red\">No input for protein name!</font><br/>\n");
      printf("</body></html>\n");
      fflush(stdout);
      free(queryString);
      return 1;
    }

    // fileNum
    if((tmpValue = getHtmlFieldValue("fileNum", queryString)) == NULL){
      printf("<font color=\"red\">No input for html file number!</font><br/>\n");
      printf("</body></html>\n");
      fflush(stdout);
      free(queryString);
      free(proName);
      return 1;
    }
    else if(sscanf(tmpValue, "%d", &fileNum) != 1
	    || fileNum < 1){
      printf("<font color=\"red\">Invalid input for html file number: %s!</font><br/>\n", tmpValue);
      printf("</body></html>\n");
      fflush(stdout);
      free(queryString);
      free(proName);
      free(tmpValue);
      return 1;
    }
    if((colored_aas = getHtmlFieldValue("markAA", queryString)) == NULL){
      //printf("<font color=\"red\">No input for protein name!</font><br/>\n");
      //printf("</BODY></HTML>\n");
      //fflush(stdout);
      //free(xmlFile);
      //return 1;
    }
    else
      free(tmpValue);

    // htmlFiles
    htmlFiles = (char **)calloc(fileNum, sizeof(char *));
    inputfiles = new Array<char*>;
    for (i = 0; i < fileNum; ++i) {
      sprintf(tmpString, "htmlFile_%d", i+1);
      if((htmlFiles[i] = getHtmlFieldValue(tmpString, queryString)) == NULL){
	printf("<font color=\"red\">No input for interact-data.htm file No. %d!</font><br/>\n", i+1);
	printf("</body></html>\n");
	fflush(stdout);
	free(queryString);
	free(proName);
	for (j = 0; j < i; ++j)
	  free(htmlFiles[j]);
	free(htmlFiles);
	return 1;
      }
      inputfiles->insertAtEnd(htmlFiles[i]);
    } //     for (i = 0; i < fileNum; ++i) {

    //   displayparser = new ASAPCGIDisplayParser(xmlFile, proName);
    if((tmpValue = getHtmlFieldValue("submit", queryString)) != NULL && (strcmp(tmpValue, "Next_Protein") == 0)){
      displayparser = new ASAPCGIDisplayParser(xmlFile, group_index+1);
      proName = strdup(displayparser->getName());
    }
    else if((tmpValue = getHtmlFieldValue("submit", queryString)) != NULL && (strcmp(tmpValue, "Prev_Protein") == 0)){
      displayparser = new ASAPCGIDisplayParser(xmlFile, group_index-1);
      proName = strdup(displayparser->getName());
    }
    else 
      displayparser = new ASAPCGIDisplayParser(xmlFile, proName, group_index);



    if(displayparser == NULL) {
      printf("<BR>Error - protein2==NULL<BR>\n"); fflush(stdout);
      exit(1);
    }

    norm = displayparser->getNormalized();
    if (norm == NULL) {
      printf("<BR>Warning - normalization info not found<BR>\n"); fflush(stdout);
    }
    
    pval_link = displayparser->getPvalLink();

    //    cout << "here2..." << endl; 
    // proDataStrct

    // get proDataStrct
	
    if((protein = getProDataStrctFromQueryString(queryString)) == NULL){
      printf("<font color=\"red\">Cannot construct proDataStrct from CGI.</font><br/>\n");
      printf("</body></html>\n");
      fflush(stdout);
      free(queryString);
      free(proName);
      for (j = 0; j < fileNum; ++j)
	free(htmlFiles[j]);
      free(htmlFiles);
    }

    //	protein = displayparser->getProDataStrct();

    // compute inputfiles here....
    parser = new ASAPRatioProteinCGIDisplayParser(protein, inputfiles, ratioType == 1, colored_aas);

    for (i = 0; i < 2; ++i) {
      sprintf(tmpString, "accRatio_%d", i);
      if ((tmpValue = getHtmlFieldValue(tmpString, queryString)) != NULL) {
	sscanf(tmpValue, "%lf", &(accRatio[i]));
	free(tmpValue);
      }
      else {
	accRatio[i] = protein->ratio[i];
      }
      
      sprintf(tmpString, "accH2LRatio_%d", i);
      if ((tmpValue = getHtmlFieldValue(tmpString, queryString)) != NULL) {
	sscanf(tmpValue, "%lf", &(accH2LRatio[i]));
	free(tmpValue);
      }
      else {
	accH2LRatio[i] = protein->inv_ratio[i];
      }
      
    }
  } //   if(queryIndx == 1) {

  /*
  // ratioType
  if ((tmpValue = getHtmlFieldValue("ratioType", queryString)) != NULL) {
    if(sscanf(tmpValue, "%d", &ratioType) != 1
       || ratioType < 0 
       || ratioType > 2) 
      ratioType = 0;
    free(tmpValue);
  }
  else {
    ratioType = 0;
  }
  */

  /*
   * collect bofFiles
   */
  bofFiles = (char **) calloc(fileNum, sizeof(char *));
  for (i = 0; i < fileNum; ++i){

    //    cout << i << ": " << htmlFiles[i] << endl;
    if((tmpValue = findRightmostPathSeperator(htmlFiles[i])) != NULL){
      ++tmpValue;      
      strcpy(directory, htmlFiles[i]);
      directory[strlen(directory)-strlen(tmpValue)] = '\0';
    }
    else {
      tmpValue = htmlFiles[i];
      directory[0] = '\0';
    }
    
    if(strcmp(tmpValue, "interact-data.htm") != 0) {
      if((tmpField = getSegment(tmpValue, "interact", "-", "-data.htm")) != NULL) { 
	sprintf(tmpString, "%sASAPRatio_%s_peptide.bof", directory, tmpField);
	free(tmpField);
      }
      else
	sprintf(tmpString, "%sASAPRatio_peptide.bof", directory);
    }
    else {
      sprintf(tmpString, "%sASAPRatio_peptide.bof", directory);
    }

    bofFiles[i] = (char *)calloc(strlen(tmpString)+1, sizeof(char));
    strcpy(bofFiles[i], tmpString);
  } // for (i = 0; i < fileNum; ++i){



  /*
   * submit
   */
  if((tmpValue = getHtmlFieldValue("submit", queryString)) != NULL) {
    wrtIndx = 1;
    if(strcmp(tmpValue, "Interim_Ratio") == 0) { //accept ratio
      protein->indx = 2;
      //ASAPRatio_getProDataStrct(protein, bofFiles); 
    }
    else if(strcmp(tmpValue, "0:1") == 0) { // set to 0:1
      protein->indx = 2; 
      if(ratioType == 1) {
	protein->ratio[0] = -1.;
	protein->inv_ratio[0] = -1.;
      }
      else { 
	protein->ratio[0] = 0.;
	protein->inv_ratio[0] = 999.;
      }
      protein->ratio[1] = 0.;
      protein->inv_ratio[1] = 0.;
    }
    else if(strcmp(tmpValue, "1:0") == 0) { // set to 1:0
      protein->indx = 2; 
      if(ratioType == 1) {
	protein->ratio[0] = 0.;
	protein->inv_ratio[0] = 999.;
      }
      else {
	protein->ratio[0] = -1.;
	protein->inv_ratio[0] = -1.;
      } 
      protein->inv_ratio[1] = 0.;
      protein->ratio[1] = 0.;
    }
    else if(strcmp(tmpValue, "Unknown") == 0) { // set to 0:0
      protein->indx = 2; 
      protein->ratio[0] = -2.;
      protein->ratio[1] = 0.;
      protein->inv_ratio[0] = -2.;
      protein->inv_ratio[1] = 0.;
    }
    else {
      wrtIndx = 0;
      //  if(protein->indx >= 0);
      //ASAPRatio_getProDataStrct(protein, bofFiles); 
    }
    free(tmpValue);

    if(wrtIndx == 1) {
      // accepted ratio
      accRatio[0] = protein->ratio[0];
      accRatio[1] = protein->ratio[1];

      accH2LRatio[0] = protein->inv_ratio[0];
      accH2LRatio[1] = protein->inv_ratio[1];

      /*
  displayProDataStrctInCgi(*protein, xmlFile, cgiAction, 
			   htmlFiles, bofFiles, fileNum, 
			   proName, ratioType, accRatio); 
      */

    /*
     * Andy, check this section 
     */
    // write proDataStrct
    overwriteparser = new ASAPCGIParser(xmlFile, proName, protein);
    /*
     * end check section
     */


    }// if(wrtIndx == 1) {
  } //  if((tmpValue = getHtmlFieldValue("submit", queryString)) != NULL) {
  
  // free queryString
//if(queryIndx == 1)
    



  /*
   * cgi display
   */
    //    printf("ready to display...\n"); fflush(stdout);
if((tmpValue = getHtmlFieldValue("submit", queryString)) != NULL && (strcmp(tmpValue, "Next_Protein") == 0))
  displayProDataStrctInCgi(*protein, norm, pval_link, xmlFile, cgiAction, 
			   htmlFiles, bofFiles, fileNum, 
			   proName, ratioType, accRatio, accH2LRatio,
			   parser, colored_aas,group_index+1);
else if((tmpValue = getHtmlFieldValue("submit", queryString)) != NULL && (strcmp(tmpValue, "Prev_Protein") == 0))
  displayProDataStrctInCgi(*protein, norm, pval_link, xmlFile, cgiAction, 
			   htmlFiles, bofFiles, fileNum, 
			   proName, ratioType, accRatio, accH2LRatio,
			   parser, colored_aas,group_index-1);
else
  displayProDataStrctInCgi(*protein, norm, pval_link, xmlFile, cgiAction, 
			   htmlFiles, bofFiles, fileNum, 
			   proName, ratioType, accRatio, accH2LRatio,
			   parser, colored_aas,group_index);

  free(queryString);

  printf("<hr noshade/>\n");
  printf("<h6>(%s)<br/>\n",szTPPVersionInfo);
  printf("This interface is used for evaluating and modifying results on protein abundance ratios.<br/>\n");
  printf("Developed by Dr. Xiao-jun Li and Luis Mendoza at <a target=\"_new\" href=\"http://www.systemsbiology.org\"> ");
  printf("Institute for Systems Biology. </a> </h6>\n\n");
  printf("</body></html>\n");
  fflush(stdout);
  
  // free memory
  free(proName);
  for (i = 0; i < fileNum; ++i){
    free(htmlFiles[i]);
    free(bofFiles[i]);
  }
  free(htmlFiles);
  free(bofFiles);
  freeProDataStrct(*protein);            
  free(protein);
  free(xmlFile);

  if(displayparser != NULL)
    delete displayparser;
  if(overwriteparser != NULL)
    delete overwriteparser;
  
  return 1;
}


/*
 * This function gets a proDataStrct from a queryString.
 */
proDataStrct *getProDataStrctFromQueryString(char *queryString)
{
  int getSeqDataStrctFromQueryString(seqDataStrct *sequence, int seqIndx, char *queryString);

  proDataStrct *protein;
  char **htmlFiles, **bofFiles;
  int fileNum;
  char directory[1000];
  char tmpString[_MXSTRLEN_];
  char *tmpValue, *tmpField;
  int i, j, k;
  

  /*
   * htmlFiles and bofFiles
   */

  //  cout << "DDS: " << queryString << endl;
  // fileNum
  if ((tmpValue = getHtmlFieldValue("fileNum", queryString)) != NULL) {
    sscanf(tmpValue, "%d", &fileNum);
    free(tmpValue);
  }
  else
    return NULL;

  // htmlFiles
  htmlFiles = (char **)calloc(fileNum, sizeof(char *));
  for (i = 0; i < fileNum; ++i) {
    sprintf(tmpString, "htmlFile_%d", i+1);
    if((htmlFiles[i] = getHtmlFieldValue(tmpString, queryString)) == NULL){
      for (j = 0; j < i; ++j)
	free(htmlFiles[j]);
      free(htmlFiles);
      return NULL;
    }
  } //     for (i = 0; i < fileNum; ++i) {

  // bofFiles
  bofFiles = (char **) calloc(fileNum, sizeof(char *));
  for (i = 0; i < fileNum; ++i){
    if((tmpValue = findRightmostPathSeperator(htmlFiles[i])) != NULL){
      ++tmpValue;      
      strcpy(directory, htmlFiles[i]);
      directory[strlen(directory)-strlen(tmpValue)] = '\0';
    }
    else {
      tmpValue = htmlFiles[i];
      directory[0] = '\0';
    }
    
    if(strcmp(tmpValue, "interact-data.htm") != 0) {
      if((tmpField = getSegment(tmpValue, "interact", "-", "-data.htm")) != NULL) { 
	sprintf(tmpString, "%sASAPRatio_%s_peptide.bof", directory, tmpField);
	free(tmpField);
      }
      else
	sprintf(tmpString, "%sASAPRatio_peptide.bof", directory);
    }
    else {
      sprintf(tmpString, "%sASAPRatio_peptide.bof", directory);
    }

    bofFiles[i] = (char *)calloc(strlen(tmpString)+1, sizeof(char));
    strcpy(bofFiles[i], tmpString);
  } // for (i = 0; i < fileNum; ++i){

  // free htmlFile
  for (j = 0; j < fileNum; ++j) {
    free(htmlFiles[j]);
  }
  free(htmlFiles);
  

  /*
   * protein
   */
  protein = (proDataStrct *) calloc(1, sizeof(proDataStrct));

  // indx
  if ((tmpValue = getHtmlFieldValue("protein_indx", queryString)) != NULL) {
    sscanf(tmpValue, "%d", &(protein->indx));
    free(tmpValue);
  }
  else {
    for (i = 0; i < fileNum; ++i) {
      free(bofFiles[i]);
    }
    free(bofFiles);
    free(protein);

    return NULL;
  }

  // ratio
  for (i = 0; i < 2; ++i) {
    sprintf(tmpString, "protein_ratio_%d", i);
    if ((tmpValue = getHtmlFieldValue(tmpString, queryString)) != NULL) {
      sscanf(tmpValue, "%lf", &(protein->ratio[i]));
      free(tmpValue);
    }
    else {
      for (j = 0; j < fileNum; ++j) {
	free(bofFiles[j]);
      }
      free(bofFiles);
      free(protein);

      return NULL;
    }

    sprintf(tmpString, "protein_inv_ratio_%d", i);
    if ((tmpValue = getHtmlFieldValue(tmpString, queryString)) != NULL) {
      sscanf(tmpValue, "%lf", &(protein->inv_ratio[i]));
      free(tmpValue);
    }
    else {
      for (j = 0; j < fileNum; ++j) {
	free(bofFiles[j]);
      }
      free(bofFiles);
      free(protein);

      return NULL;
    }
    
  }
  
  // dataNum
  if ((tmpValue = getHtmlFieldValue("protein_dataNum", queryString)) 
      != NULL) {
    sscanf(tmpValue, "%d", &(protein->dataNum));
    free(tmpValue);
  }
  else {
    for (j = 0; j < fileNum; ++j) {
      free(bofFiles[j]);
    }
    free(bofFiles);
    free(protein);

    return NULL;
  }

  // dataCnts
  protein->dataCnts = (int *) calloc(protein->dataNum, sizeof(int));
  for (i = 0; i < protein->dataNum; ++i) {
    sprintf(tmpString, "protein_dataCnts_%d", i);
    if ((tmpValue = getHtmlFieldValue(tmpString, queryString)) != NULL) {
      sscanf(tmpValue, "%d", &(protein->dataCnts[i]));
      free(tmpValue);
    }
    else {
      for (j = 0; j < fileNum; ++j) {
	free(bofFiles[j]);
      }
      free(bofFiles);
      free(protein->dataCnts);
      free(protein);

      return NULL;
    }
  }

  // sequences
  protein->sequences = (seqDataStrct *)
    calloc(protein->dataNum, sizeof(seqDataStrct));
  for (i = 0; i < protein->dataNum; ++i) {
    if((getSeqDataStrctFromQueryString(protein->sequences+i, i, queryString)) == 0) {
      cout << "failure at sequence " << i << endl;
      // sequences
      for (k = 0; k < i; ++k) {
	// peaks
	for (j = 0; j < protein->sequences[k].dataNum; ++j) {
	  free(protein->sequences[k].peaks[j].dataIndx);
	  free(protein->sequences[k].peaks[j].dataCnts);
	}
	free(protein->sequences[k].peaks);
	// dataCnts
	free(protein->sequences[k].dataCnts);
      } // for (k = 0; k < i; ++k) {
      free(protein->sequences);
      
      // dataCnts
      free(protein->dataCnts);

      for (j = 0; j < fileNum; ++j) {
	free(bofFiles[j]);
      }
      free(bofFiles);
      free(protein);

      return NULL;
    } // if(getSeqDataStrctFromQueryString(&(protein->sequences[i]),
  } // for (i = 0; i < protein->dataNum; ++i) {

  // calculate ratio


  //  ASAPRatio_getProDataStrct(protein, bofFiles);

  // free memory
  //  for (j = 0; j < fileNum; ++j) {
  //  free(bofFiles[j]);
  //  }
  //  free(bofFiles);

  return protein;
}


/*
 * This function gets a seqDataStrct from a queryString.  It returns 1 on success, 0 on failure.
 */
int getSeqDataStrctFromQueryString(seqDataStrct *sequence, int seqIndx, char *queryString)
{
  int getDataStrctFromQueryString(dataStrct *peak, int seqIndx, int pkIndx, char *queryString);

  char *tmpValue;
  char tmpField[100];
  int i, j;
  
  // indx
  sprintf(tmpField, "sequence_%d_indx", seqIndx);
  if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) != NULL) {
    sscanf(tmpValue, "%d", &(sequence->indx));
    free(tmpValue);
  }
  else {
    cout << "no sequence_%d_indx" << endl;
    return 0;
  }

  // ratio
  for (i = 0; i < 2; ++i) {
    sprintf(tmpField, "sequence_%d_ratio_%d", seqIndx, i);
    if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) != NULL) {
      sscanf(tmpValue, "%lf", &(sequence->ratio[i]));
      free(tmpValue);
    }
    else {
      cout << "NO sequence_%d_ratio_%d" << endl;
      return 0;
    }
    sprintf(tmpField, "sequence_%d_inv_ratio_%d", seqIndx, i);
    if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) != NULL) {
      sscanf(tmpValue, "%lf", &(sequence->inv_ratio[i]));
      free(tmpValue);
    }
    else {
      cout << "NO sequence_%d_inv_ratio_%d" << endl;
      return 0;
    }
  }
  
  // dataNum
  sprintf(tmpField, "sequence_%d_dataNum", seqIndx);
  if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) 
      != NULL) {
    sscanf(tmpValue, "%d", &(sequence->dataNum));
    free(tmpValue);
  }
  else {
    cout << "NO sequence_%d_dataNum" << endl;
    return 0;
  }

  // lightseq
  sprintf(tmpField, "sequence_%d_lightSeq", seqIndx);
  if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) 
      != NULL) {
    strcpy(sequence->lightSeq, tmpValue);
    //sscanf(tmpValue, "%s", &(sequence->dataNum));
    free(tmpValue);
  }
  else {
    cout << "NO sequence_%d_lightSeq" << endl;
    return 0;
  }

  
  // weight
  sprintf(tmpField, "sequence_%d_weight", seqIndx);
  if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) 
      != NULL) {
    sscanf(tmpValue, "%lf", &(sequence->weight));
    free(tmpValue);
  }
  else {
    cout << "no sequence_%d_weight" << endl;
    return 0;
  }
  
  // dataCnts
  sequence->dataCnts = (int *) calloc(sequence->dataNum, sizeof(int));
  for (i = 0; i < sequence->dataNum; ++i) {
    sprintf(tmpField, "sequence_%d_dataCnts_%d", seqIndx, i);
    if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) != NULL) {
      sscanf(tmpValue, "%d", &(sequence->dataCnts[i]));
      free(tmpValue);
    }
    else {
      cout << "no sequence_%d_dataCnts_%d for " << seqIndx << " and " << i << endl;
      free(sequence->dataCnts);
      return 0;
    }
  }


  // peaks
  sequence->peaks = (dataStrct *) calloc(sequence->dataNum, sizeof(dataStrct));
  for (i = 0; i < sequence->dataNum; ++i) {
    if(getDataStrctFromQueryString(sequence->peaks+i, seqIndx, i, queryString) == 0) {
      // peaks
      for (j = 0; j < i; ++j) {
	free(sequence->peaks[j].dataIndx);
	free(sequence->peaks[j].dataCnts);
      }
      free(sequence->peaks);
      // dataCnts
      free(sequence->dataCnts);

      return 0;

    } // if(getSeqDataStrctFromQueryString(&(sequence->peaks[i]),
  } // for (i = 0; i < sequence->dataNum; ++i) {

  return 1;
}


/*
 * This function gets a dataStrct from a queryString. It return 1 on success, 0 on failure.
 */
int getDataStrctFromQueryString(dataStrct *peak, int seqIndx, int pkIndx, char *queryString)
{
  
  char *tmpValue;
  char tmpField[100];
  int i;

  // indx
  sprintf(tmpField, "peak_%d_%d_indx", seqIndx, pkIndx);
  if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) != NULL) {
    sscanf(tmpValue, "%d", &(peak->indx));
    free(tmpValue);
  }
  else { 
    cout << "no peak_%d_%d_indx" << endl;
    return 0;
  }

  // ratio
  for (i = 0; i < 2; ++i) {
    sprintf(tmpField, "peak_%d_%d_ratio_%d", seqIndx, pkIndx, i);
    if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) != NULL) {
      sscanf(tmpValue, "%lf", &(peak->ratio[i]));
      free(tmpValue);
    }
    else {
      return 0;
    }
  }
  
  // dataNum
  sprintf(tmpField, "peak_%d_%d_dataNum", seqIndx, pkIndx);
  if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) 
      != NULL) {
    sscanf(tmpValue, "%d", &(peak->dataNum));
    free(tmpValue);
  }
  else {
    cout << "no peak_%d_%d_dataNum" << endl;
    return 0;
  }
  
  // dataIndx
  peak->dataIndx = (int *) calloc(peak->dataNum, sizeof(int));
  for (i = 0; i < peak->dataNum; ++i) {
    sprintf(tmpField, "peak_%d_%d_dataIndx_%d", seqIndx, pkIndx, i);
    if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) != NULL) {
      sscanf(tmpValue, "%d", &(peak->dataIndx[i]));
      free(tmpValue);
    }
    else {
      free(peak->dataIndx);
      cout << "no peak_%d_%d_dataIndx_%d" << endl;
      return 0;
    }
  }

  // dataCnts
  peak->dataCnts = (int *) calloc(peak->dataNum, sizeof(int));
  for (i = 0; i < peak->dataNum; ++i) {
    sprintf(tmpField, "peak_%d_%d_dataCnts_%d", seqIndx, pkIndx, i);
    if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) != NULL) {
      sscanf(tmpValue, "%d", &(peak->dataCnts[i]));
      //cout << seqIndx << " " << pkIndx << " " << i << " datacnts: " << peak->dataCnts[i] << "<p/>" << endl;
      free(tmpValue);
    }
    else {
      free(peak->dataIndx);
      free(peak->dataCnts);
      cout << "no peak_%d_%d_dataCnts_%d for seq " << seqIndx << " pk " << pkIndx << " data " << i << endl;
      return 0;
    }
  }

  // weight
  sprintf(tmpField, "peak_%d_%d_weight", seqIndx, pkIndx);
  if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) 
      != NULL) {
    sscanf(tmpValue, "%lf", &(peak->weight));
    free(tmpValue);
  }
  else {
    free(peak->dataIndx);
    free(peak->dataCnts);
    cout << "no peak_%d_%d_weight" << endl;
    return 0;
  }
  
  // bofIndx
  sprintf(tmpField, "peak_%d_%d_bofIndx", seqIndx, pkIndx);
  if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) 
      != NULL) {
    sscanf(tmpValue, "%d", &(peak->bofIndx));
    free(tmpValue);
  }
  else {
    free(peak->dataIndx);
    free(peak->dataCnts);
    cout << "no peak_%d_%d_bofIndx" << endl;
    return 0;
  }
  
  return 1;
}


/*
 * This function displays a proDataStrct in cgi program.
 */
void displayProDataStrctInCgi(proDataStrct protein, Normalization* norm, char* pval_link, 
			      char *xmlFile, char *cgiAction, 
			      char **htmlFiles, char **bofFiles, int fileNum, 
			      char *proName, int ratioType, double *accRatio, double *accH2LRatio,
			      ASAPRatioProteinCGIDisplayParser* parser, 
			      char* colored_aas,unsigned int group_no)
{
  void displaySeqDataStrctInCgi(int seqNo, int seqCount, seqDataStrct sequence, 
				char **htmlFiles, char **bofFiles, int ratioType);

  char **ratioStrings=NULL;
  char ratioTypeString[9];

  int i;


  /*
   * form
   */
  printf("<form method=\"POST\" action=\"%s\">\n\n", cgiAction);
  fflush(stdout);

  // proName
  printf("<table cellspacing=\"0\">\n");
  printf("<tbody><tr>\n");
  printf("<td class=\"banner_cid\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Protein: <font color=\"#ff8700\">%s</font>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>\n",proName);
  printf("</tr></tbody>\n</table>\n");

  /*
   * display manual
   */
  printf("<div class=\"formentry\">\n");
  printf("<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\">\n");
  printf("<tbody>\n");
  fflush(stdout);

  // 1st line
  if(ratioType == 0) {
    ratioStrings = ratioOutput(accRatio, 0);
    sprintf(ratioTypeString, " (L/H)");
  }
  else if(ratioType == 1) {
    ratioStrings = ratioOutput(accH2LRatio, 0);
    sprintf(ratioTypeString, " (H/L)");
  }
  else if(ratioType == 2) {
    ratioStrings = ratioOutput(accRatio, 2);
    sprintf(ratioTypeString, " ");
  }
  printf("<tr>\n<th align=\"left\">Accepted  Ratio%s: </th>\n",ratioTypeString);
  printf("<td><h2>%s +- %s (%s%%)</h2></td>\n", ratioStrings[0], ratioStrings[1], ratioStrings[2]);

  printf("<td rowspan=\"4\" align=\"center\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>\n");
  printf("<td rowspan=\"2\" class=\"navselected\" align=\"center\">\n");
  printf("Set Accepted Ratio to:<br>\n&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n");
  printf("<input name=\"submit\" value=\"Interim_Ratio\" type=\"submit\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n");
  printf("<input name=\"submit\" value=\"0:1\" type=\"submit\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n");
  printf("<input name=\"submit\" value=\"1:0\" type=\"submit\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n");
  printf("<input name=\"submit\" value=\"Unknown\" type=\"submit\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n");
  printf("</td>\n</tr>\n");

  fflush(stdout);
  for(i = 0; i < 3; ++i)
    free(ratioStrings[i]);
  free(ratioStrings);

  // 2nd line
  if(ratioType == 0) {
    ratioStrings = ratioOutput(protein.ratio, 0);
  }  
  else if(ratioType == 1) {
    ratioStrings = ratioOutput(protein.inv_ratio, 0);
  }
  else if(ratioType == 2) {
    ratioStrings = ratioOutput(protein.ratio, 2);
  }
  printf("<tr>\n<th align=\"left\">Interim Ratio%s: </th>\n",ratioTypeString);
  printf("<td><h3>%s +- %s (%s%%)</h3></td>\n</tr>\n", ratioStrings[0], ratioStrings[1], ratioStrings[2]);

  fflush(stdout);
  for(i = 0; i < 3; ++i)
    free(ratioStrings[i]);
  free(ratioStrings);

  // 3rd and 4th lines
  if (norm != NULL) {
    double norm_ratio[2];
    double pvalue;
    norm_ratio[0] = protein.ratio[0];
    norm_ratio[1] = protein.ratio[1];
    pvalue = (double)norm->normalize(norm_ratio);
    ratioStrings = ratioOutput(norm_ratio, ratioType);
    printf("<tr>\n<th align=\"left\">Normalized Ratio: </th>\n");
    printf("<td><h3><i>%s +- %s (%s%%)</i></h3></td>\n</tr>\n", ratioStrings[0], ratioStrings[1], ratioStrings[2]);

    printf("<tr>\n<th align=\"left\">p-value: </th>\n");
    printf("<td><h3><i><a href=\"%s\">%f</a></i></h3</td>\n", pval_link, pvalue);

    fflush(stdout);
    for(i = 0; i < 3; ++i)
      free(ratioStrings[i]);
    free(ratioStrings);
  } 
  else { // spacer
    printf("<tr><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr>\n");
    printf("<tr><td>&nbsp;</td><td>&nbsp;</td>\n");
  }
  printf("<td><input name=\"submit\" value=\"Evaluate_Ratio\" type=\"submit\"></td>\n</tr>\n");
  printf("<td><input name=\"submit\" value=\"Prev_Protein\" type=\"submit\"></td>\n");
  printf("<td><input name=\"submit\" value=\"Next_Protein\" type=\"submit\"></td>\n</tr>\n");

  printf("</tbody>\n</table>\n</div>\n\n<br>\n");

  /*
   * hidden fields
   */
  // xmlFile
  printf("<input type=\"hidden\" name=\"xmlFile\" value=\"%s\" />\n", xmlFile);
  fflush(stdout);

  // pvalLink
  printf("<input type=\"hidden\" name=\"pvalLink\" value=\"%s\" />\n", pval_link);
  fflush(stdout);

  // proName
  printf("<input type=\"hidden\" name=\"proName\" value=\"%s\" />\n", proName);
  fflush(stdout);
  
  // group_num
  printf("<input type=\"hidden\" name=\"group_no\" value=\"%d\" />\n", group_no);
  fflush(stdout);
  
  // fileName
  printf("<input type=\"hidden\" name=\"fileNum\" value=\"%d\" />\n", fileNum);
  fflush(stdout);
  
  // fileName
  if(colored_aas != NULL) {
  printf("<input type=\"hidden\" name=\"markAA\" value=\"%s\" />\n", colored_aas);
  fflush(stdout);
  }

  // htmlFiles
  for (i = 0; i < fileNum; ++i)
    printf("<input type=\"hidden\" name=\"htmlFile_%d\" value=\"%s\" />\n",
	   i+1, htmlFiles[i]);
  fflush(stdout);

  // ratioType
  printf("<input type=\"hidden\" name=\"ratioType\" value=\"%d\" />\n",
	 ratioType);
  fflush(stdout);

  // accRatio
  for (i = 0; i < 2; ++i) 
    printf("<input type=\"hidden\" name=\"accRatio_%d\" value=\"%f\" />\n",
	   i, accRatio[i]);
  fflush(stdout);

  // accH2LRatio
  for (i = 0; i < 2; ++i) 
    printf("<input type=\"hidden\" name=\"accH2LRatio_%d\" value=\"%f\" />\n",
	   i, accH2LRatio[i]);
  fflush(stdout);

  // indx
  printf("<input type=\"hidden\" name=\"protein_indx\" value=\"%d\" />\n",
	 protein.indx);
  fflush(stdout);
 
  // ratio
  for (i = 0; i < 2; ++i) {
    printf("<input type=\"hidden\" name=\"protein_ratio_%d\" value=\"%f\" />\n", i, protein.ratio[i]);
    printf("<input type=\"hidden\" name=\"protein_inv_ratio_%d\" value=\"%f\" />\n", i, protein.inv_ratio[i]);
  }
  fflush(stdout);

  // dataNum
  printf("<input type=\"hidden\" name=\"protein_dataNum\" value=\"%d\" />\n",
	 protein.dataNum);
  fflush(stdout);   


  /*
   * display individual sequence
   */
  fflush(stdout);

  // accepted sequence
  for (i = 0; i < protein.dataNum; ++i) {
    if(protein.dataCnts[i] == 1 && protein.sequences[i].indx >= 0) {
      displaySeqDataStrctInCgi(i, protein.dataCnts[i], protein.sequences[i],
			       htmlFiles, bofFiles, ratioType);
    }
  }
  
  // outlier sequence
  for (i = 0; i < protein.dataNum; ++i) {
    if(protein.dataCnts[i] == 0 && protein.sequences[i].indx >= 0) {
      displaySeqDataStrctInCgi(i, protein.dataCnts[i], protein.sequences[i],
			       htmlFiles, bofFiles, ratioType);
      fflush(stdout);
    }
  }

  // deleted sequence
  for (i = 0; i < protein.dataNum; ++i) {
    if(protein.dataCnts[i] == -1 && protein.sequences[i].indx >= 0) {
      displaySeqDataStrctInCgi(i, protein.dataCnts[i], protein.sequences[i],
			       htmlFiles, bofFiles, ratioType);
      fflush(stdout);
    }
  }

  // invalid sequence
  for (i = 0; i < protein.dataNum; ++i) {
    if(protein.sequences[i].indx < 0) {
      displaySeqDataStrctInCgi(i, protein.dataCnts[i], protein.sequences[i],
			       htmlFiles, bofFiles, ratioType);
    }
  }

  parser->write(cout);

  printf("</form>\n\n");

  return;
}


/*
 * This function displays a seqDataStrct in cgi program.
 */
void displaySeqDataStrctInCgi(int seqNo, int seqCount, seqDataStrct sequence, 
			      char **htmlFiles, char **bofFiles, int ratioType)
{
  void displayDataStrctInCgi(int seqNo, int peakNo, int peakCnt, dataStrct peak,
			     char *orgFile, char *pepBofFile, int ratioType);

  char orgFile[1000];
  int pkIndx=-1, pepIndx;

  int i;
  

  /*
   * collect sequence
   */
  for (i = 0; i < sequence.dataNum; ++i){
    if(sequence.peaks[i].indx >= 0){
      pkIndx = i;
      break;
    }
  }
  pepIndx = sequence.peaks[pkIndx].dataIndx[0];
  for (i = 1; i < sequence.peaks[pkIndx].dataNum; ++i){
    if(sequence.peaks[pkIndx].dataIndx[i] < pepIndx)
      pepIndx = sequence.peaks[pkIndx].dataIndx[i];
  }

  // open orgFile
  /*
  sprintf(orgFile, "%s.orig", htmlFiles[sequence.peaks[pkIndx].bofIndx]);
  if((fin = fopen(orgFile, "r")) == NULL) {  
    printf("<font color=\"red\">Cannot read file \"%s\".</font>\n", orgFile); 
    fflush(stdout);
    return;
  }
  

  // sequence
  sprintf(tmpField, "pepIndx=%d&amp", pepIndx);
  while (!feof(fin) && fgets(tmpString, _MXSTRLEN_, fin) != NULL) {
    if(strstr(tmpString, tmpField) != NULL) {
      getRidOfSpace(tmpString);
      break;
    }
  }
  fclose(fin);
  */
  /*
  if((tmpValue = getSegment(tmpString, "SEQUENCE", "\">", "<")) == NULL){
    if((tmpValue = getSegment(tmpString, "QUERY", "\">", "<")) == NULL){
      printf("<font color=\"red\">Cannot get sequence.</font>\n");
      fflush(stdout);
    }
    else {
      rmHtmlTag(tmpValue);
      if(strlen(tmpValue) == 0) {
	free(tmpValue);
	printf("<font color=\"red\">Cannot get sequence.</font>\n");
	fflush(stdout);
      }
    }
  }

  else {
    rmHtmlTag(tmpValue);
    if(strlen(tmpValue) == 0) {
      free(tmpValue);
      printf("<font color=\"red\">Cannot get sequence.</font>\n");
      fflush(stdout);
    }
  }
  strcpy(pepSeq, tmpValue);
  free(tmpValue);
  cnvtUpper(pepSeq);
*/

  /*
    print sequence
  */
  /*
  ratioStrings = ratioOutput(sequence.ratio, ratioType);
  printf("<li>\n");
  if(seqCount == 1) {
    printf("<font color=\"blue\"> %s </font>\t", pepSeq);
    printf("Prophet weight: %0.2f\t", sequence.weight);
    if(sequence.indx >= 0) 
      printf("Ratio: <font color=\"blue\"> %s +- %s (%s%%) </font>\t",
	     ratioStrings[0], ratioStrings[1], ratioStrings[2]);
  }
  else if(seqCount == 0) {
    printf("<font color=\"#ff9900\"> %s </font>\t", pepSeq);
    printf("Prophet weight: %0.2f\t", sequence.weight);
    if(sequence.indx >= 0) 
      printf("Ratio: <font color=\"#ff9900\"> %s +- %s (%s%%) </font>\t",
	     ratioStrings[0], ratioStrings[1], ratioStrings[2]);
  }
  else{
    printf("<font color=\"red\"> %s </font>\t", pepSeq);
    printf("Prophet weight: %0.2f\t", sequence.weight);
    if(sequence.indx >= 0) 
      printf("Ratio: <font color=\"red\"> %s +- %s (%s%%) </font>\t",
	     ratioStrings[0], ratioStrings[1], ratioStrings[2]);
  }
  fflush(stdout);
  for(i = 0; i < 3; ++i)
    free(ratioStrings[i]);
  free(ratioStrings);
  */
  /*
  // seqCount
  if(sequence.indx >= 0) {
    printf("Acceptance: ");
    if(seqCount == 1) {
      printf("<input type=\"radio\" name=\"protein_dataCnts_%d\" value=\"1\" checked=\"checked\"/>Yes <input type=\"radio\" name=\"protein_dataCnts_%d\" value=\"0\"/>No <input type=\"radio\" name=\"protein_dataCnts_%d\" value=\"-1\"/>NA</br></br>\n", seqNo, seqNo, seqNo);
  }
    else if(seqCount == 0) {
      printf("<input type=\"radio\" name=\"protein_dataCnts_%d\" value=\"1\"/>Yes <input type=\"radio\" name=\"protein_dataCnts_%d\" value=\"0\" checked=\"checked\"/>No <input type=\"radio\" name=\"protein_dataCnts_%d\" value=\"-1\"/>NA</br></br>\n", seqNo, seqNo, seqNo);
    }
    else {
      printf("<input type=\"radio\" name=\"protein_dataCnts_%d\" value=\"1\"/>Yes <input type=\"radio\" name=\"protein_dataCnts_%d\" value=\"0\"/>No <input type=\"radio\" name=\"protein_dataCnts_%d\" value=\"-1\" checked=\"checked\"/>NA</br></br>\n", seqNo, seqNo, seqNo);
    }      
  }
  else {
    printf("<input type=\"hidden\" name=\"protein_dataCnts_%d\" value=\"%d\" /></br></br>\n", seqNo, seqCount);
  }
  fflush(stdout);

  */

  /*
   * hidden fields
   */
  // indx
  //printf("input type=\"hidden\" name=\"sequence_%d_indx\" value=\"%d\"\n",
  //	 seqNo, sequence.indx);
  printf("<input type=\"hidden\" name=\"sequence_%d_indx\" value=\"%d\" />\n",
	 seqNo, sequence.indx);
  fflush(stdout);
  
  // ratio
  for (i = 0; i < 2; ++i) {
    printf("<input type=\"hidden\" name=\"sequence_%d_ratio_%d\" value=\"%f\" />\n", 
	   seqNo, i, sequence.ratio[i]);
    printf("<input type=\"hidden\" name=\"sequence_%d_inv_ratio_%d\" value=\"%f\" />\n", 
	   seqNo, i, sequence.inv_ratio[i]);
  }
  fflush(stdout);
  
  // dataNum
  printf("<input type=\"hidden\" name=\"sequence_%d_dataNum\" value=\"%d\" />\n",
	 seqNo, sequence.dataNum);
  fflush(stdout);
  
  // weight
  printf("<input type=\"hidden\" name=\"sequence_%d_weight\" value=\"%f\" />\n",
	 seqNo, sequence.weight);
  fflush(stdout);
  
  // sequence
  printf("<input type=\"hidden\" name=\"sequence_%d_lightSeq\" value=\"%s\" />\n",
	 seqNo, sequence.lightSeq);
  fflush(stdout);

  /*
   * display peaks one by one
   */
  //printf("<ul>\n");
  for (i = 0; i < sequence.dataNum; ++i) {
    //sprintf(orgFile, "%s.orig", htmlFiles[sequence.peaks[i].bofIndx]);
    displayDataStrctInCgi(seqNo, i, sequence.dataCnts[i], sequence.peaks[i], 
			  orgFile, bofFiles[sequence.peaks[i].bofIndx], ratioType);
  }
  //printf("</ul>\n");
  //printf("</li>\n");
  fflush(stdout);

  return;
}


/*
 * This function displays a dataStrct in cgi program.
 */
void displayDataStrctInCgi(int seqNo, int peakNo, int peakCnt, dataStrct peak,
			   char *orgFile, char *pepBofFile, int ratioType)
{
  char **getStrSects(int *sectNum, char *string, char sep);

  double cutoff_re = 0.5;
  int j, k;
  

  /*
    collect all data belong to that peak
  */
  // get sorted pkIndx
  /*
  pkIndx = (int *) calloc(peak.dataNum, sizeof(int));
  for (i = 0; i < peak.dataNum; ++i) 
    pkIndx[i] = peak.dataIndx[i];
  for (i = 0; i < peak.dataNum-1; ++i) {
    for (j = i+1; j < peak.dataNum; ++j) {
      if(pkIndx[i] > pkIndx[j]){
	pepIndx = pkIndx[i];
	pkIndx[i] = pkIndx[j];
	pkIndx[j] = pepIndx;
      }
    }
  }
  */
  /*
  // open orgFile
  if((fin = fopen(orgFile, "r")) == NULL) {  
    printf("<font color=\"red\">Cannot read file \"%s\".</font>\n", orgFile); 
    fflush(stdout);
    return;
  }

  // get dataStrings
  dataStrings = (char **) calloc(peak.dataNum, sizeof(char *));
  dataNum = 0;
  sprintf(tmpField, "pepIndx=%d&amp", pkIndx[dataNum]);
  while (dataNum < peak.dataNum
	 && !feof(fin) && fgets(tmpString, _MXSTRLEN_, fin) != NULL) {
    if(strstr(tmpString, tmpField) != NULL) {
      getRidOfSpace(tmpString);
      dataStrings[dataNum] = (char *) calloc(strlen(tmpString)+1, sizeof(char));
      strcpy(dataStrings[dataNum], tmpString);
      ++dataNum;
      if(dataNum < peak.dataNum)
	sprintf(tmpField, "pepIndx=%d&amp", pkIndx[dataNum]);
    }
  }
  fclose(fin);
  free(pkIndx);
  */

  /*
   *  display peak
   */
  //printf("<li>\n");

  // indx
  printf("<input type=\"hidden\" name=\"peak_%d_%d_indx\" value=\"%d\" />\n",
	 seqNo, peakNo, peak.indx);
    
  // ratio
  for (k = 0; k < 2; ++k) {
    printf("<input type=\"hidden\" name=\"peak_%d_%d_ratio_%d\" value=\"%f\" />\n", 
	   seqNo, peakNo, k, peak.ratio[k]);
    printf("<input type=\"hidden\" name=\"peak_%d_%d_inv_ratio_%d\" value=\"%f\" />\n", 
	   seqNo, peakNo, k, peak.inv_ratio[k]);
  }

  // dataNum
  printf("<input type=\"hidden\" name=\"peak_%d_%d_dataNum\" value=\"%d\" />\n",
	 seqNo, peakNo, peak.dataNum);

  // weight
  printf("<input type=\"hidden\" name=\"peak_%d_%d_weight\" value=\"%f\" />\n",
	 seqNo, peakNo, peak.weight);

  // bofIndx
  printf("<input type=\"hidden\" name=\"peak_%d_%d_bofIndx\" value=\"%d\" />\n",
	 seqNo, peakNo, peak.bofIndx);
  fflush(stdout);


  /*
  // expLbl
  tmpValue = getSegment(dataStrings[0], "OutFile", ">", "<");
  rmHtmlTag(tmpValue);
  strSects = getStrSects(&tmpNum, tmpValue, '.');
  free(tmpValue);
  ratioStrings = ratioOutput(peak.ratio, ratioType);
  if(peakCnt == 1) {
    printf("Experiment: <font color=\"blue\"> %s </font>", strSects[0]);
    printf("Scan: <font color=\"blue\"> %s </font>\t", strSects[1]);
    if(peak.indx >= 0)
      printf("Ratio: <font color=\"blue\"> %s +- %s (%s%%) </font>\t",
	     ratioStrings[0], ratioStrings[1], ratioStrings[2]);
  }
  else if(peakCnt == 0) {
    printf("Experiment: <font color=\"#ff9900\"> %s </font>", strSects[0]);
    printf("Scan: <font color=\"#ff9900\"> %s </font>\t", strSects[1]);
    if(peak.indx >= 0)
      printf("Ratio: <font color=\"#ff9900\"> %s +- %s (%s%%) </font>\t",
	     ratioStrings[0], ratioStrings[1], ratioStrings[2]);
  }
  else {
    printf("Experiment: <font color=\"red\"> %s </font>", strSects[0]);
    printf("Scan: <font color=\"red\"> %s </font>\t", strSects[1]);
    if(peak.indx >= 0)
      printf("Ratio: <font color=\"red\"> %s +- %s (%s%%) </font>\t",
	     ratioStrings[0], ratioStrings[1], ratioStrings[2]);
  }
  fflush(stdout);
  for(i = 0; i < tmpNum; ++i)
    free(strSects[i]);
  free(strSects);
  for(i = 0; i < 3; ++i)
    free(ratioStrings[i]);
  free(ratioStrings);

  // peak.dataCnts
  if(peak.indx >= 0) {
    printf("Acceptance: ");
    if(peakCnt == 1) {
      printf("<input type=\"radio\" name=\"sequence_%d_dataCnts_%d\" value=\"1\" checked=\"checked\"/>Yes <input type=\"radio\" name=\"sequence_%d_dataCnts_%d\" value=\"0\"/>No <input type=\"radio\" name=\"sequence_%d_dataCnts_%d\" value=\"-1\"/>NA</br>\n", seqNo, peakNo, seqNo, peakNo, seqNo, peakNo);
    }
    else if(peakCnt == 0) {
      printf("<input type=\"radio\" name=\"sequence_%d_dataCnts_%d\" value=\"1\"/>Yes <input type=\"radio\" name=\"sequence_%d_dataCnts_%d\" value=\"0\" checked=\"checked\"/>No <input type=\"radio\" name=\"sequence_%d_dataCnts_%d\" value=\"-1\"/>NA</br>\n", seqNo, peakNo, seqNo, peakNo, seqNo, peakNo);
    }
    else {
      printf("<input type=\"radio\" name=\"sequence_%d_dataCnts_%d\" value=\"1\"/>Yes <input type=\"radio\" name=\"sequence_%d_dataCnts_%d\" value=\"0\"/>No <input type=\"radio\" name=\"sequence_%d_dataCnts_%d\" value=\"-1\" checked=\"checked\"/>NA</br>\n", seqNo, peakNo, seqNo, peakNo, seqNo, peakNo);
    }
  }
  else {
    printf("<input type=\"hidden\" name=\"sequence_%d_dataCnts_%d\" value=\"%d\" /></br>\n", 
	   seqNo, peakNo, peakCnt);
  }
  fflush(stdout);
  */

  /*
    peptide identifications
  */

  
  /*
  // open pepBofFile
  if((fin = fopen(pepBofFile, "r")) == NULL) {  
    printf("<font color=\"red\">Cannot read file \"%s\".</font>\n", pepBofFile); 
    fflush(stdout);
    return;
  }
  */
  //printf("<ul>\n");
  
  for (j = 0; j < peak.dataNum; ++j) {
    /*
    //printf("<li>\n");
    
    // get htmlString
    pepIndx = peak.dataIndx[j];
    sprintf(tmpField, "pepIndx=%d&amp", pepIndx);
    for (k = 0; k < dataNum; ++k) {
      if(strstr(dataStrings[k], tmpField) != NULL) {
	strcpy(tmpString, dataStrings[k]);
	break;
      }
    }
  */

    // indx
    printf("<input type=\"hidden\" name=\"peak_%d_%d_dataIndx_%d\" value=\"%d\" />\n",
	   seqNo, peakNo, j, peak.dataIndx[j]);

    /*
    if((tmpValue=strstr(tmpString, "<A NAME=\"ASAPRatio_proRatio\"")) 
       != NULL) {
      tmpString[strlen(tmpString)-strlen(tmpValue)] = '\0';
      getRidOfSpace(tmpString);
    }
    */
    /*
    // ratio
    if((tmpValue=strstr(tmpString, "<A NAME=\"ASAPRatio_pepRatio\"")) 
       != NULL) {
      if((tmpPtr=strstr(tmpValue, ">")) != NULL) {
	tmpValue[strlen(tmpValue)-strlen(tmpPtr)+1] = '\0';
	printf("%s", tmpValue);
	fseek(fin, pepIndx*sizeof(pepDataStrct), SEEK_SET);
	fread(&peptide, sizeof(pepDataStrct), 1, fin);
	if(peptide.indx >= 0) { // if valid data
	  ratioStrings = ratioOutput(peptide.pepRatio, ratioType);
	  printf("%s</A>", ratioStrings[0]);
	  if(peptide.indx == 2) // verified
	    printf(" +- <font color=\"green\">%s (%s%%)</font>\t",
		   ratioStrings[1], ratioStrings[2]); 
	  else if(peptide.pepRatio[0] <= 0.
		  || peptide.pepRatio[1] 
		  > cutoff_re*peptide.pepRatio[0])
	    printf(" +- <font color=\"red\">%s (%s%%)</font>\t",
		   ratioStrings[1], ratioStrings[2]); 
	  else
	    printf(" +- <font color=\"blue\">%s (%s%%)</font>\t",
		   ratioStrings[1], ratioStrings[2]); 
	  for(i = 0; i < 3; ++i)
	    free(ratioStrings[i]);
	  free(ratioStrings);
	} //if(peptide.indx >= 0) { // if valid data
      } // if((tmpPtr=strstr(tmpValue, ">")) != NULL) {
      tmpString[strlen(tmpString)-strlen(tmpValue)] = '\0';
      getRidOfSpace(tmpString);
    } // if((tmpValue=strstr(tmpString, "<A NAME=\"ASAPRatio_pepRatio\"")) 
    fflush(stdout);
    */
    // Cnts
    /*
    if(peak.indx >= 0){
      
      if(peak.dataCnts[j] == 1) {
	printf("<input type=\"radio\" name=\"peak_%d_%d_dataCnts_%d\" value=\"1\" checked=\"checked\"/>Yes <input type=\"radio\" name=\"peak_%d_%d_dataCnts_%d\" value=\"0\"/>No <input type=\"radio\" name=\"peak_%d_%d_dataCnts_%d\" value=\"-1\"/>NA</br>\n", seqNo, peakNo, j, seqNo, peakNo, j, seqNo, peakNo, j);
      }
      else if(peak.dataCnts[j] == 0) {
	printf("<input type=\"radio\" name=\"peak_%d_%d_dataCnts_%d\" value=\"1\"/>Yes <input type=\"radio\" name=\"peak_%d_%d_dataCnts_%d\" value=\"0\" checked=\"checked\"/>No <input type=\"radio\" name=\"peak_%d_%d_dataCnts_%d\" value=\"-1\"/>NA</br>\n", seqNo, peakNo, j, seqNo, peakNo, j, seqNo, peakNo, j);
      }
      else {
	printf("<input type=\"radio\" name=\"peak_%d_%d_dataCnts_%d\" value=\"1\"/>Yes <input type=\"radio\" name=\"peak_%d_%d_dataCnts_%d\" value=\"0\"/>No <input type=\"radio\" name=\"peak_%d_%d_dataCnts_%d\" value=\"-1\" checked=\"checked\"/>NA</br>\n", seqNo, peakNo, j, seqNo, peakNo, j, seqNo, peakNo, j);
      }
      
    }
    else {
           printf("<input type=\"hidden\" name=\"peak_%d_%d_dataCnts_%d\" value=\"%d\" /></br>\n", 
       	     seqNo, peakNo, j, peak.dataCnts[j]);
    }
*/
    // entry
    //printf("%s\n", tmpString);
    
    // printf("</li>\n");
        fflush(stdout);
   } // for (j = 0; j < peak.dataNum; ++j) {
  //  fclose(fin);
  //printf("</ul>\n");
  //printf("</li></br>\n");
  fflush(stdout);

/*
  for(i = 0; i < dataNum; ++i)
    free(dataStrings[i]);
  free(dataStrings);
*/
  return;
}


/*
 * This function frees a proDataStrct.
 */
void freeProDataStrct(proDataStrct data)
{
  int i, j;

  // sequences
  for (i = 0; i < data.dataNum; ++i) {
    // peaks
    for (j = 0; j < data.sequences[i].dataNum; ++j) {
      free(data.sequences[i].peaks[j].dataIndx);
      free(data.sequences[i].peaks[j].dataCnts);
    }
    free(data.sequences[i].peaks);
    
    // dataCnts
    free(data.sequences[i].dataCnts);
  } // for (i = 0; i < data.dataNum; ++i) {
  free(data.sequences);

  // dataCnts
  free(data.dataCnts);

  return;
}
