/*
  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
*/
# ifndef _ASAPRATIO_PROPHET_H_
# define _ASAPRATIO_PROPHET_H_

/************************************************************************/
/*
  Constants
*/
/************************************************************************/

# define _MXSTRLEN_ 20000 // maximium string length
# define _ASAPRATIO_MXQ_ 4 // maximium charge state
# define _ASAPRATIO_CONFL_ 0.6826 // confidence level at 1 sigma


/************************************************************************/
/*
  Structures
*/
/************************************************************************/
/*
  structure of LC peak
*/
/*
typedef struct {
  int indx; // 2 for valid peak, 1 for invalid peak, 0 for new peak, -1 for no data
  int peak; // peak
  int valley[2]; // peak valleys
  double bckgrnd; // background
  double area[2]; // area and its error
  double time[2]; // peak time and it error
} lcPeakStrct;

*/
/*
  structure of peptide data list
*/
/*
typedef struct {
  int indx; // -1 invalid, 0 new, 1 analyzed, 2 verified
  long scan; // CID scan number 
  int chrg; // charge
  int cidIndx; // cidIndx = 0 if Light identified; 1 if Heavy
  double msLight; // theoretical [M] value of Light: Q = 0; -1. if no pair
  double msHeavy; // theoretical [M] value of Heavy: Q = 0; -1. if no pair
  int eltn; // elution order: 1 if Light elutes earlier, -1 later, 0 same 
  int areaFlag; //area cal.: 1, raw spect.; 2, fitting spect; other, ave.
  lcPeakStrct peaks[_ASAPRATIO_MXQ_][2]; // lc peaks: light, heavy
  double pkRatio[_ASAPRATIO_MXQ_]; // light:heavy ratio for charge Q
  double pkError[_ASAPRATIO_MXQ_]; // error in light:heavy ratio
  int pkCount[_ASAPRATIO_MXQ_]; // whether to use ratio for charge Q
  double pepRatio[2]; // peptide light:heavy ratio and it error
  double pepTime[2][2]; // peptide peakTime and error: highest light and heavy
  double pepArea; // area of most intensity charge state
} pepDataStrct;

*/
/*
  structure for identify unique sequence
*/

typedef struct {
  int pepNum;
  char **seqs;
  double mass[2];
  double wght;
} seqIdStrct;
  

/*
  structure of data
*/
typedef struct {
  int indx; // 0 new, 1 analyzed, -1 invalid
  double ratio[2]; // ratio and error 
  int dataNum; // data number
  int *dataIndx; // index for data
  int *dataCnts; // whether to count the data: 0 no, 1 yes
  double weight; // data weight
  int bofIndx;
} dataStrct;


/*
  structure of unique sequence
*/
typedef struct {
  int indx; // 0 new, 1 analyzed, -1 invalid
  double ratio[2]; // ratio and error 
  int dataNum; // data number
  dataStrct *peaks; // unique peaks
  int *dataCnts; // whether to count the data: 0 no, 1 yes
  double weight; // data weight from ProteinProphet
} seqDataStrct;


/*
  structure of protein 
*/
typedef struct {
  int indx; // 0 new, 1 analyzed, 2 verified, -1 invalid STATUS
  double ratio[2]; // ratio and error
  int dataNum; // data number
  seqDataStrct *sequences; // unique sequences
  int *dataCnts; // whether to count the data: 0 no, 1 yes INCLUDE
} proDataStrct;


typedef struct {
  double mean;  // mean
  double merr; // fitting error on mean
  double stddev; // standard deviation of the data
} pValueStrct;

/*
  structure of spectrum
*/
typedef struct {
  int size;
  double *xval;
  double *yval;
} spectStrct;



/************************************************************************/
/*
  Functions
*/
/************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////////
/*
  This function gets a ASAPRatio_proDataStrct *protein.
*/
//void Prophet_getProDataStrct(proDataStrct *protein, char **htmlFiles, char **peptides,
//			     double *weights, double mnProb, double mnWght);

//////////////////////////////////////////////////////////////////////////////
/*
  This function evaluates the ratio of a protein.
*/
//void ASAPRatio_getProDataStrct(proDataStrct *data, char **pepBofFiles); 

////////////////////////////////////////////////////////////////////////////
/*
  For a given set of data, dataErrs, and dataWghs, this function identifies 
  any outliers and gets the mean and confidence interval of ratio.
*/ 
//void getDataRatio(double *ratio, double *error, double confL, 
//		  double *data, double *dataErrs, 
//		  double *dataWghs, int *dataIndx, int dataSize, 
//		  int testType);


# endif /* _ASAPRATIO_PROPHET_H_ */
/*
  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
*/
