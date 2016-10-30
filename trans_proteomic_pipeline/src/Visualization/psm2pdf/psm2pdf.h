/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#define PROBCUTOFF 0.90

#define SIZE_FILE 1024
#define SIZE_BUF 8192
#define MAX_PEPTIDE_LEN 256

#define DEFAULT_OUTPUT "spectra.pdf"
#define BATCH_SIZE 100       /* number of plot-msms.cgi URLs for each htmldoc call */
#define BODYCOLOR "FFFCEE"   /* body color of resulting PDF */

#define DEFAULT_TOL 0.95
#define DEFAULT_YZOOM 1.0

char   szAttributeVal[SIZE_FILE];

#include "../../common/tpp_hashmap.h" // deals with different compilers
#include <string>

//using namespace std;
typedef TPP_STDSTRING_HASHMAP(bool) bool_hash;

struct ParamsStruct
{
   int    iLenPeptide;
   int    iLabelType;   /* 0=ion labels, 1=fragment masses, 2=no labels, 3=force fragment masses */
   int    iMassTypeFragment;
   int    iNumAxis;
   int    iXmin;
   int    iXmax;
   int    iSortOption;  /* 1=file/scan, 2=protein/peptide, 3=peptide */

   int    iStaticTermModType;

   int    bZoom113;     /* zoom in on 113 to 118 mass range */

   char   cShowA;
   char   cShowA2;
   char   cShowA3;
   char   cShowB;
   char   cShowB2;
   char   cShowB3;
   char   cShowC;
   char   cShowC2;
   char   cShowC3;
   char   cShowX;
   char   cShowX2;
   char   cShowX3;
   char   cShowY;
   char   cShowY2;
   char   cShowY3;
   char   cShowZ;
   char   cShowZ2;
   char   cShowZ3;
   char   cShowNH3Loss; /* unused */
   char   cShowH2OLoss; /* mark -17 NH3 and -18 H20 loss by tracking just -17.5 */

   char   pcMod[MAX_PEPTIDE_LEN];       /* old way of noting variable modification string */

   char   szOutputFile [SIZE_FILE];     /* final PDF file name */
   char   szHost[SIZE_FILE];            /* webserver hostname */
   char   szCgiBin[SIZE_FILE];            /* webserver hostname */
   char   szUser[SIZE_FILE];            /* webserver hostname */
   char   szPass[SIZE_FILE];            /* webserver hostname */
   char   szListURL[SIZE_FILE];         /* contains plot-msms cgi URL list*/

   char   szListFile [SIZE_FILE];     /* final PDF file name */ 

   double dMaxPeakMass;

   double pdModPeptide[MAX_PEPTIDE_LEN];        /* just contains a list of modified masses at each peptide position */
   double pdModNC[2];   /* mass of new N or C terminus */

   double dMatchTol;
   double dIntensityZoom;
   double dProbCutoff;
   double dExpectCutoff;

   bool_hash* specList;
  
};


char *GET_VAL(char *pSpectrumQuery,
      char *szAttribute);
void MYREALLOC(int *iSizeSpectrumQuery,
      char **pSpectrumQuery);
void SET_OPTION(char *arg,
      struct ParamsStruct *pParams);
void GLOBAL_INIT(struct ParamsStruct *pParams);
void USAGE(char *szArg);
void PARSE_PEPXML(char *szInputFile,
      FILE *fplist,
      struct ParamsStruct pParams);
void CREATE_PDF(struct ParamsStruct pParams);
