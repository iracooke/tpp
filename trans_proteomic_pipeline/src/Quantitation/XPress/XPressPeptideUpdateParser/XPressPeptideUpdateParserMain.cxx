/*

Program       : XPressPeptideUpdateParser                                                   
Author        : J.Eng and Andrew Keller <akeller@systemsbiology.org>, Robert Hubley, and 
                open source code                                                       
Date          : 11.27.02 

Primary data object holding all mixture distributions for each precursor ion charge

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "mzParser.h"

/*
 * Include gd library to create png files
 * see http://www.boutell.com/gd/
 */
#include "gd.h"
#include "gdfonts.h"

#include "XPressPeptideUpdateParser.h"
#include "Parsers/Parser/Tag.h"

#include "common/TPPVersion.h" // contains version number, name, revision
#include "common/util.h"

#define XPRESS_UPDATE        "cgiupdatexpress3"

#define szVERSION            "XPRESS"
#define szAUTHOR             "by J.Eng &copy; Institute for Systems Biology, 2000. All rights reserved."

//#define SIZE_FILE             256

#ifndef TRUE
#define TRUE                  1
#endif
#ifndef FALSE
#define FALSE                 0
#endif

#define SCALE_V(x,y,z,w)      (int)((x-y)*z/w);


int  bXpressLight1;
char szXMLFile[SIZE_FILE],
     szOutputFile[SIZE_FILE],
     szInteractBaseName[SIZE_FILE],
     szInteractDir[SIZE_FILE];
double dProton = 1.00727646688;

struct QuanStruct
{
   int  iChargeState;
   int  iSwapMass;
   int  iLightFirstScan;
   int  iLightLastScan;
   int  iHeavyFirstScan;
   int  iHeavyLastScan;
   double dLightPeptideMass;
   double dHeavyPeptideMass;
   double dLightQuanValue;
   double dHeavyQuanValue;
   double dMassTol;
  int bPpmMassTol;
   char bLightPeptide;
   char szNewQuan[20];
} pQuan;

void PRINT_FORM(char *szArgv0,
		struct QuanStruct *pQuan,
		char *szXMLFile,
		char *szOutputFile,
      int bNormalize);
void EXTRACT_QUERY_STRING(struct QuanStruct *pQuan,
			  char *szXMLFile,
			  char *szOutputFile,
			  char *szInteractDir,
			  char *szInteractBaseName,
			  int  *bXpressLight1,
			  int  *bNormalize,
           int* index, char* xmlfile, int* overwrite, char* zeroarea, char* quan);
void GET_QUANTITATION(char *szXMLFile,
        struct QuanStruct *pQuan,
        int bNormalize);
void MAKE_PLOT(int iPlotStartScan,
        int iPlotEndScan,
        int iLightStartScan,
        int iLightEndScan,
        int iHeavyStartScan,
        int iHeavyEndScan, 
        int bNormalize, 
        double dMaxLightInten,
        double dMaxHeavyInten,
        double *pdLight,
        double *pdHeavy,
        double *dLightFiltered,
        double *dHeavyFiltered,
	       char *szXMLFile,
        struct QuanStruct *pQuan);
void FILTER_MS(double *dOrigMS,
        double *dFilteredMS,
        double *dTmpFilter,
        double dMaxInten,
        int    iNumMSScans,
        int    iPlotStartScan,
        int    iPlotEndScan); 
void UPDATE_QUAN(void);
void BAD_QUAN(void);
void SPECIAL_QUAN();
void flipRatio(char* ratio, char* flipped);

#include "common/util.h"

int update_index;
char update_xmlfile[500];
int overwrite;
char zero_area[10];
char update_quan[100];

int main(int argc, char **argv)
{
   int bNormalize;  /* normalize plots to same vertical scale*/
  hooks_tpp handler(argc,argv); // set up install paths etc

  /*
   * Print html header
   */
   printf("Content-type: text/html\n\n");
   printf("<HTML>\n<HEAD><TITLE>%s (%s), %s</TITLE></HEAD>\n", szVERSION, szTPPVersionInfo, szAUTHOR);
   printf("<BODY BGCOLOR=\"#FFFFFF\" OnLoad=\"self.focus();\">\n");

   szXMLFile[0]='\0'; 
   szOutputFile[0]='\0'; 
   szInteractDir[0]='\0'; 
   szInteractBaseName[0]='\0'; 
   bXpressLight1=0;
   bNormalize=0;

   update_xmlfile[0] = 0;
   zero_area[0] = 0;
   update_quan[0] = 0;
   overwrite = 0;

   EXTRACT_QUERY_STRING(&pQuan, szXMLFile, szOutputFile, szInteractDir,
			szInteractBaseName, &bXpressLight1, &bNormalize,
         &update_index, update_xmlfile, &overwrite, zero_area, update_quan);

   if (pQuan.iSwapMass==1)
   {
      pQuan.dHeavyPeptideMass = pQuan.dLightPeptideMass;
      pQuan.dLightPeptideMass -= 8.0;
   }
   else if (pQuan.iSwapMass==2)
   {
      pQuan.dLightPeptideMass = pQuan.dHeavyPeptideMass;
      pQuan.dHeavyPeptideMass += 8.0;
   }

   PRINT_FORM(argv[0], &pQuan, szXMLFile, szOutputFile, bNormalize);

   GET_QUANTITATION(szXMLFile, &pQuan, bNormalize);

   printf("<TABLE WIDTH=100%%><TR WIDTH=\"100%%\">\n");

   printf("<TD WIDTH=33%% ALIGN=RIGHT>");
   //if (strlen(szInteractDir)>1)
   // {
   //    BAD_QUAN();
   // }
   SPECIAL_QUAN();
   printf("</TD><TD WIDTH=33%%>");

   printf("<CENTER>");
   printf("<TABLE><TR><TD ALIGN=RIGHT>Light</TD><TD>:</TD><TD ALIGN=LEFT>Heavy</TD></TR>\n");

   if (pQuan.dLightQuanValue!=0.0)
      printf("<TR><TD ALIGN=RIGHT>1</TD><TD>:</TD><TD ALIGN=LEFT>%0.2f</TD></TR>\n",
         pQuan.dHeavyQuanValue / pQuan.dLightQuanValue);
   else
      printf("<TR><TD ALIGN=RIGHT>1</TD><TD>:</TD><TD ALIGN=LEFT>NaN</TD></TR>\n");

   if (pQuan.dHeavyQuanValue!=0.0)
      printf("<TR><TD ALIGN=RIGHT>%0.2f</TD><TD>:</TD><TD ALIGN=LEFT>1</TD></TR>\n",
         pQuan.dLightQuanValue /  pQuan.dHeavyQuanValue);
   else
      printf("<TR><TD ALIGN=RIGHT>NaN</TD><TD>:</TD><TD ALIGN=LEFT>1</TD></TR>\n");

   //printf("</TABLE></CENTER>\n");

   if (bXpressLight1==1)
   {
      if (pQuan.dLightQuanValue == 0.0)
         sprintf(pQuan.szNewQuan, "1:INF");
      else
         sprintf(pQuan.szNewQuan, "1:%0.2f", pQuan.dHeavyQuanValue / pQuan.dLightQuanValue);
   }
   else if (bXpressLight1==2)
   {
      if (pQuan.dHeavyQuanValue == 0.0)
         sprintf(pQuan.szNewQuan, "INF:1");
      else
         sprintf(pQuan.szNewQuan, "%0.2f:1", pQuan.dLightQuanValue / pQuan.dHeavyQuanValue);
   }
   else
   {
      if (pQuan.dLightQuanValue==0.0 && pQuan.dHeavyQuanValue==0.0)
         sprintf(pQuan.szNewQuan, "?");
      else if (pQuan.dLightQuanValue > pQuan.dHeavyQuanValue)
         sprintf(pQuan.szNewQuan, "1:%0.2f", pQuan.dHeavyQuanValue / pQuan.dLightQuanValue);
      else
         sprintf(pQuan.szNewQuan, "%0.2f:1", pQuan.dLightQuanValue / pQuan.dHeavyQuanValue);
   }

   printf("</TABLE></TD><TD WIDTH=33%% ALIGN=LEFT>");

   //if (strlen(szInteractDir)>1)
   // {
   UPDATE_QUAN();
      //  }

   printf("</TD></TR></TABLE>\n");

   printf("<P><FONT SIZE= -2>See <A HREF=\"http://www.boutell.com/gd/\">");
   printf("http://www.boutell.com/gd/</A> for info. on the gd graphics library used by this program</FONT>\n");
   printf("</BODY></HTML>\n");

   return(0);

} /*main*/


void PRINT_FORM(char *szArgv0,
        struct QuanStruct *pQuan,
        char *szXMLFile,
        char *szOutputFile,
        int bNormalize)
{

   printf("<P>\n");
   printf("<FORM METHOD=GET ACTION=\"%s\">", getenv("SCRIPT_NAME"));

   printf("<INPUT TYPE=\"hidden\" NAME=\"InteractDir\" VALUE=\"%s\">\n", szInteractDir);
   printf("<INPUT TYPE=\"hidden\" NAME=\"InteractBaseName\" VALUE=\"%s\">\n", szInteractBaseName);
   printf("<INPUT TYPE=\"hidden\" NAME=\"bXpressLight1\" VALUE=\"%d\">\n", bXpressLight1);
   printf("<input TYPE=\"hidden\" NAME=\"xmlfile\" VALUE=\"%s\">", update_xmlfile);
   printf("<input TYPE=\"hidden\" NAME=\"index\" VALUE=\"%d\">", update_index);

   printf("<CENTER><TABLE BORDER=0 CELLPADDING=5>\n");

   printf("<TR VALIGN=TOP><TD BGCOLOR=\"#EEEEAA\">\n");
   printf("<TT>Light scans:");
   printf(" <INPUT TYPE=\"textarea\" NAME=\"LightFirstScan\" VALUE=\"%d\" SIZE=8>\n", pQuan->iLightFirstScan);
   printf(" <INPUT TYPE=\"textarea\" NAME=\"LightLastScan\"  VALUE=\"%d\" SIZE=8>\n", pQuan->iLightLastScan);
   printf(" mass:<INPUT TYPE=\"textarea\" NAME=\"LightMass\" VALUE=\"%0.4f\" SIZE=8>\n", pQuan->dLightPeptideMass);
   printf(" tol: <INPUT TYPE=\"textarea\" NAME=\"MassTol\" VALUE=\"%0.4f\" SIZE=3>", pQuan->dMassTol);
   //   printf(" ppmtol: <INPUT TYPE=\"textarea\" NAME=\"PpmTol\" VALUE=\"%d\" SIZE=3>", pQuan->bPpmMassTol);

   printf("<BR>Heavy scans:");
   printf(" <INPUT TYPE=\"textarea\" NAME=\"HeavyFirstScan\" VALUE=\"%d\" SIZE=8>\n", pQuan->iHeavyFirstScan);
   printf(" <INPUT TYPE=\"textarea\" NAME=\"HeavyLastScan\"  VALUE=\"%d\" SIZE=8>\n", pQuan->iHeavyLastScan);
   printf(" mass:<INPUT TYPE=\"text\" NAME=\"HeavyMass\" VALUE=\"%0.4f\" SIZE=8>\n", pQuan->dHeavyPeptideMass);
   printf(" &nbsp;&nbsp;Z: <INPUT TYPE=\"textarea\" NAME=\"ChargeState\" VALUE=\"%d\" SIZE=2>", pQuan->iChargeState);

   printf("<P>&nbsp;Raw file:  <input NAME=\"XMLFile\" type=textarea VALUE=\"%s\" SIZE=\"50\">\n", szXMLFile);
   printf("<BR>.out file:  <input NAME=\"OutFile\" type=textarea VALUE=\"%s\" SIZE=\"50\">", szOutputFile);
   printf("&nbsp;&nbsp;Norm:<INPUT TYPE=\"checkbox\" NAME=\"Norm\" VALUE=\"1\" %s>\n", (bNormalize?"checked":""));


   printf("</TD>\n");
   printf("<TD BGCOLOR=\"#EEEEAA\">\n<TT><INPUT TYPE=\"submit\" VALUE=\"Quantitate\">");
   printf("</TD>\n");

   printf("</TR>\n");
/*
   printf("<TR><TD BGCOLOR=\"#EEEEAA\" COLSPAN=2><CENTER><TT>");
   printf("no mass swap <INPUT TYPE=\"radio\" NAME=\"SwapMass\" VALUE=\"0\" checked>");
   printf(" &nbsp; swap light to heavy <INPUT TYPE=\"radio\" NAME=\"SwapMass\" VALUE=\"1\">");
   printf(" &nbsp; swap heavy to light <INPUT TYPE=\"radio\" NAME=\"SwapMass\" VALUE=\"2\"></TT></CENTER>");
   printf("</TR>\n");
*/
   printf("</TABLE></CENTER>\n");
   printf("</FORM>\n");

} /*PRINT_FORM*/


void EXTRACT_QUERY_STRING(struct QuanStruct *pQuan,
     char *szXMLFile,
     char *szOutputFile,
     char *szInteractDir,
     char *szInteractBaseName,
     int  *bXpressLight1,
     int  *bNormalize,
     int* index, char* xmlfile, int* overwrite, char* zeroarea, char* quan)
{
   const char *pStr = getenv("REQUEST_METHOD");


   if (pStr==NULL) 
   {
      printf(" Error - this is a CGI program that cannot be\n");
      printf(" run from the command line.\n\n");
      exit(EXIT_FAILURE);
   }
   else if (!strcmp(pStr, "GET"))
   {
      int  i;
      char *szQuery,
           szWord[512];

      /*
       * Old files will not have PpmTol attribute,
       * therefore we initialize it to 0 (Dalton by default)
       */
      pQuan->bPpmMassTol = 0;

     /*
      * Get:
      *       ChargeState - charge state of precursor
      */
      szQuery = getenv("QUERY_STRING");

      if (szQuery == NULL)
      {
         printf("<P>No query information to decode.\n");
         printf("</BODY>\n</HTML>\n");
         exit(EXIT_FAILURE);
      }

      for (i=0; szQuery[0] != '\0'; i++)
      {
         getword(szWord, szQuery, '=');
         plustospace(szWord);
         unescape_url(szWord);

         if (!strcmp(szWord, "LightFirstScan"))
         {
            getword(szWord, szQuery, '&'); plustospace(szWord); unescape_url(szWord);
            sscanf(szWord, "%d", &(pQuan->iLightFirstScan));
         }
         else if (!strcmp(szWord, "LightLastScan") )
         {
            getword(szWord, szQuery, '&'); plustospace(szWord); unescape_url(szWord);
            sscanf(szWord, "%d", &(pQuan->iLightLastScan));
         }
         else if (!strcmp(szWord, "HeavyFirstScan") )
         {
            getword(szWord, szQuery, '&'); plustospace(szWord); unescape_url(szWord);
            sscanf(szWord, "%d", &(pQuan->iHeavyFirstScan));
         }
         else if (!strcmp(szWord, "HeavyLastScan") )
         {
            getword(szWord, szQuery, '&'); plustospace(szWord); unescape_url(szWord);
            sscanf(szWord, "%d", &(pQuan->iHeavyLastScan));
         }
         else if (!strcmp(szWord, "XMLFile") )
         {
            getword(szWord, szQuery, '&'); plustospace(szWord); unescape_url(szWord);
            sscanf(szWord, "%s", szXMLFile);
            fixPath(szXMLFile,0); // tidy up path seperators etc - expect existence
         }
         else if (!strcmp(szWord, "OutFile") )
         {
            getword(szWord, szQuery, '&'); plustospace(szWord); unescape_url(szWord);
            sscanf(szWord, "%s", szOutputFile);
            fixPath(szOutputFile,0); // tidy up path seperators etc
         }
         else if (!strcmp(szWord, "InteractDir") )
         {
            getword(szWord, szQuery, '&'); plustospace(szWord); unescape_url(szWord);
            sscanf(szWord, "%s", szInteractDir);
            fixPath(szInteractDir,1); // tidy up path seperators etc - expect existence
         }
         else if (!strcmp(szWord, "InteractBaseName") )
         {
            getword(szWord, szQuery, '&'); plustospace(szWord); unescape_url(szWord);
            sscanf(szWord, "%s", szInteractBaseName);
            fixPath(szInteractBaseName,0); // tidy up path seperators etc
         }
         else if (!strcmp(szWord, "ChargeState") )
         {
            getword(szWord, szQuery, '&'); plustospace(szWord); unescape_url(szWord);
            sscanf(szWord, "%d", &(pQuan->iChargeState));
         }
         else if (!strcmp(szWord, "SwapMass") )
         {
            getword(szWord, szQuery, '&'); plustospace(szWord); unescape_url(szWord);
            sscanf(szWord, "%d", &(pQuan->iSwapMass));
         }
         else if (!strcmp(szWord, "LightMass") )
         {
            getword(szWord, szQuery, '&'); plustospace(szWord); unescape_url(szWord);
            sscanf(szWord, "%lf", &(pQuan->dLightPeptideMass));
         }
         else if (!strcmp(szWord, "HeavyMass") )
         {
            getword(szWord, szQuery, '&'); plustospace(szWord); unescape_url(szWord);
            sscanf(szWord, "%lf", &(pQuan->dHeavyPeptideMass));
         }
         else if (!strcmp(szWord, "MassTol") )
         {
            getword(szWord, szQuery, '&'); plustospace(szWord); unescape_url(szWord);
            sscanf(szWord, "%lf", &(pQuan->dMassTol));
         }
         else if (!strcmp(szWord, "PpmTol") )
         {
            getword(szWord, szQuery, '&'); plustospace(szWord); unescape_url(szWord);
            sscanf(szWord, "%d", &(pQuan->bPpmMassTol));
         }
         else if (!strcmp(szWord, "bXpressLight1") )
         {
            getword(szWord, szQuery, '&'); plustospace(szWord); unescape_url(szWord);
            sscanf(szWord, "%d", bXpressLight1);
         }
         else if (!strcmp(szWord, "Norm") )
         {
            getword(szWord, szQuery, '&'); plustospace(szWord); unescape_url(szWord);
            sscanf(szWord, "%d", bNormalize);
         }
         else if (!strcmp(szWord, "index") )
         {
            getword(szWord, szQuery, '&'); plustospace(szWord); unescape_url(szWord);
            sscanf(szWord, "%d", index);
         }
         else if (!strcmp(szWord, "xmlfile") )
         {
            getword(szWord, szQuery, '&'); plustospace(szWord); unescape_url(szWord);
            strcpy(xmlfile, szWord);
            fixPath(xmlfile,1); // tidy up path seperators etc - expect existence
            //sscanf(szWord, "%d", index);
         }
         else if (!strcmp(szWord, "overwrite") )
         {
            getword(szWord, szQuery, '&'); plustospace(szWord); unescape_url(szWord);
            sscanf(szWord, "%d", overwrite);
         }
         else if (!strcmp(szWord, "ZeroArea") )
         {
            getword(szWord, szQuery, '&'); plustospace(szWord); unescape_url(szWord);
            strcpy(zero_area, szWord);
         }
         else if (!strcmp(szWord, "NewQuan") )
         {
            getword(szWord, szQuery, '&'); plustospace(szWord); unescape_url(szWord);
            strcpy(quan, szWord);
         }
         else
         {
            getword(szWord, szQuery, '&'); plustospace(szWord); unescape_url(szWord);
         }
      } /*for*/
   }
   else
   {
      printf(" Error not called with GET method\n");
      exit(EXIT_FAILURE);
   }
   // make sure we can do this
   rampValidateOrDeriveInputFilename(szXMLFile,SIZE_FILE,szOutputFile);
} /*EXTRACT_QUERY_STRING*/



/*
 * Reads .dat files and get quantitation numbers
 */
void GET_QUANTITATION(char *szXMLFile,
        struct QuanStruct *pQuan,
        int bNormalize)
{
   int    i,
          ctScan,
          iAnalysisFirstScan,
          iAnalysisLastScan,
          iLightStartScan,
          iLightEndScan,
          iHeavyStartScan,
          iHeavyEndScan,
          iPlotStartScan,
          iPlotEndScan,
          iPlotBuffer=25,
          iStart,
          iEnd;

   double dLightMass,
          dHeavyMass,
          *dLightMS,
          *dHeavyMS,
          *dLightFilteredMS,
          *dHeavyFilteredMS,
          *dTmpMS,
          dMaxLightInten,
          dMaxHeavyInten,
          dCurrTime;

   struct ScanHeaderStruct scanHeader;

   ramp_fileoffset_t   indexOffset;
   ramp_fileoffset_t  *pScanIndex;
   RAMPFILE   *pFI;


   /*
    * confirm presence & open corresponding mzXML file
    */

   if ( (pFI=rampOpenFile(szXMLFile))==NULL)
   {
      char szXMLFile2[SIZE_FILE];

      /*
       * Not idea but works. XPressCGIProteinDisplayParser.cgi current does not
       * have mzXML/mzML file extension passed to it at all.  So xpress link
       * that it generates always has a .mzXML extension.  Simple fix here is
       * to try given XML file input and, if not found, the corresponding one.
       */
      if (strstr(szXMLFile, ".mzXML"))
      {
         strcpy(szXMLFile2, szXMLFile);
         szXMLFile2[strlen(szXMLFile2)-5]='\0';
         strcat(szXMLFile2, "mzML");
         if ( (pFI=rampOpenFile(szXMLFile2))==NULL)
         {
            printf(" Error - cannot open file \"%s\".\n\n", szXMLFile);
            exit(0);
         }
      }
      else if (strstr(szXMLFile, ".mzML"))
      {
         strcpy(szXMLFile2, szXMLFile);
         szXMLFile2[strlen(szXMLFile2)-4]='\0';
         strcat(szXMLFile2, "mzXML");
         if ( (pFI=rampOpenFile(szXMLFile2))==NULL)
         {
            printf(" Error - cannot open file \"%s\".\n\n", szXMLFile);
            exit(0);
         }
      }
      else
      {
         printf(" Error - cannot open file \"%s\".\n\n", szXMLFile);
         exit(0);
      }
   }

  /*
   * Get the analysis start, end scans
   */

   // Read the offset of the index
   indexOffset = getIndexOffset( pFI );
   
   // Read the scan index into a vector, get LastScan
   pScanIndex = readIndex( pFI , indexOffset, &iAnalysisLastScan );
   iAnalysisFirstScan = 1;

   if ( (dLightMS=(double *)malloc(sizeof(double)*(iAnalysisLastScan+5)))==NULL)
   {
      printf(" Error, cannot malloc dLightMS[%d]\n\n", iAnalysisLastScan+5);
      exit(EXIT_FAILURE);
   }
   if ( (dHeavyMS=(double *)malloc(sizeof(double)*(iAnalysisLastScan+5)))==NULL)
   {
      printf(" Error, cannot malloc dHeavyMS[%d]\n\n", iAnalysisLastScan+5);
      exit(EXIT_FAILURE);
   }
   if ( (dLightFilteredMS=(double *)malloc(sizeof(double)*(iAnalysisLastScan+5)))==NULL)
   {
      printf(" Error, cannot malloc dLightFilteredMS[%d]\n\n", iAnalysisLastScan+5);
      exit(EXIT_FAILURE);
   }
   if ( (dHeavyFilteredMS=(double *)malloc(sizeof(double)*(iAnalysisLastScan+5)))==NULL)
   {
      printf(" Error, cannot malloc dHeavyFilteredMS[%d]\n\n", iAnalysisLastScan+5);
      exit(EXIT_FAILURE);
   }
   if ( (dTmpMS=(double *)malloc(sizeof(double)*(iAnalysisLastScan+5)))==NULL)
   {
      printf(" Error, cannot malloc dTmpMS[%d]\n\n", iAnalysisLastScan+5);
      exit(EXIT_FAILURE);
   }  

  /*
   * Calculate the precursor mass
   */
   if (pQuan->iChargeState < 1)
   {
      printf(" Error, charge state = %d\n\n", pQuan->iChargeState);
      exit(EXIT_FAILURE);
   }
   else
   {
      dLightMass = (dProton * (pQuan->iChargeState - 1) + pQuan->dLightPeptideMass) / pQuan->iChargeState;
      dHeavyMass = (dProton * (pQuan->iChargeState - 1) + pQuan->dHeavyPeptideMass) / pQuan->iChargeState;
   }


  /*
   * Clear all values
   */
   memset(dLightMS, 0, sizeof(double)*(iAnalysisLastScan+5));
   memset(dHeavyMS, 0, sizeof(double)*(iAnalysisLastScan+5));
   memset(dLightFilteredMS, 0, sizeof(double)*(iAnalysisLastScan+5));
   memset(dHeavyFilteredMS, 0, sizeof(double)*(iAnalysisLastScan+5));


   iStart = pQuan->iLightFirstScan; 
   readHeader( pFI, pScanIndex[iStart], &scanHeader );
   dCurrTime = scanHeader.retentionTime;
   while (iStart>iAnalysisFirstScan && dCurrTime - scanHeader.retentionTime < 560.0 )
   {
      iStart--;
      readHeader( pFI, pScanIndex[iStart], &scanHeader );
   }

   iEnd = pQuan->iLightLastScan;
   readHeader( pFI, pScanIndex[iEnd], &scanHeader );
   dCurrTime = scanHeader.retentionTime;
   while (iEnd<iAnalysisLastScan  && scanHeader.retentionTime-dCurrTime < 560.0 )
   {
      iEnd++;
      readHeader( pFI, pScanIndex[iEnd], &scanHeader );
   }

  /*
   * Read all MS scan values
   */
   double dLightMassTol;
   double dHeavyMassTol;

   if( pQuan->bPpmMassTol )
     {
       dLightMassTol = dLightMass * pQuan->dMassTol / 1000000;
       dHeavyMassTol = dHeavyMass * pQuan->dMassTol / 1000000;       
     }
   else
     {
       dLightMassTol = pQuan->dMassTol;
       dHeavyMassTol = pQuan->dMassTol;
     }

   for (ctScan=iStart; ctScan<=iEnd; ctScan++)
   {
      int pkts;

      readHeader( pFI, pScanIndex[ctScan], &scanHeader );

      if (scanHeader.msLevel == 1)
      {
         RAMPREAL *pPeaks;
         int   n=0;

         /*
          * Open a scan
          */
         pPeaks = readPeaks( pFI, pScanIndex[ctScan] );
   
         for ( pkts=0; pkts < scanHeader.peaksCount; pkts++ )
         {
            RAMPREAL fMass;
            RAMPREAL fInten;

            fMass=pPeaks[n];
            n++;
            fInten=pPeaks[n];
            n++;

            if ( fabs(dLightMass - fMass) <= dLightMassTol)
               if (fInten > dLightMS[ctScan])
                  dLightMS[ctScan] = (double)fInten;
   
            if ( fabs(dHeavyMass - fMass) <= dHeavyMassTol)
               if (fInten > dHeavyMS[ctScan])
                  dHeavyMS[ctScan] = (double)fInten;
         }

         free(pPeaks);

      }
   } /*for*/

  /*
   * Starting from the start and end scans read from .out
   * files, need to see the real start/end scan of eluting
   * peptide by looking at smoothed/filtered MS profile.
   */

  /*
   * Get peptide start & end scans
   */
   iLightStartScan = pQuan->iLightFirstScan;
   iLightEndScan = pQuan->iLightLastScan;
   iHeavyStartScan = pQuan->iHeavyFirstScan;
   iHeavyEndScan = pQuan->iHeavyLastScan;

  /*
   * Print out data for plotting
   */
   iPlotStartScan = iLightStartScan;
   iPlotEndScan = iLightEndScan;

   if (iHeavyStartScan < iPlotStartScan)
      iPlotStartScan = iHeavyStartScan;
   if (iHeavyEndScan > iPlotEndScan)
      iPlotEndScan = iHeavyEndScan;

   readHeader( pFI, pScanIndex[iPlotStartScan], &scanHeader );
   dCurrTime = scanHeader.retentionTime;
   while (iPlotStartScan>iAnalysisFirstScan && dCurrTime - scanHeader.retentionTime < 45.0 )
   {
      iPlotStartScan--;
      readHeader( pFI, pScanIndex[iPlotStartScan], &scanHeader );
   }

   readHeader( pFI, pScanIndex[iPlotEndScan], &scanHeader );
   dCurrTime = scanHeader.retentionTime;
   while (iPlotEndScan<iAnalysisLastScan  && scanHeader.retentionTime-dCurrTime < 45.0 )
   {
      iPlotEndScan++;
      readHeader( pFI, pScanIndex[iPlotEndScan], &scanHeader );
   }

   free(pScanIndex);
   rampCloseFile(pFI);
/*
   if (iPlotStartScan-iPlotBuffer < iAnalysisFirstScan)
      iPlotStartScan = iAnalysisFirstScan;
   else
      iPlotStartScan -= iPlotBuffer;

   if (iPlotEndScan+iPlotBuffer> iAnalysisLastScan-1)
      iPlotEndScan = iAnalysisLastScan-1;
   else
      iPlotEndScan += iPlotBuffer;
*/

   dMaxLightInten=0.0;
   dMaxHeavyInten=0.0;

   for (i=iPlotStartScan; i<=iPlotEndScan; i++)
   {
      if (dLightMS[i]>dMaxLightInten)
         dMaxLightInten=dLightMS[i];
      if (dHeavyMS[i]>dMaxHeavyInten)
         dMaxHeavyInten=dHeavyMS[i];
   }

  /*
   * Sum up intensity values across each charge state
   */
   for (i=iLightStartScan; i<=iLightEndScan; i++)
      pQuan->dLightQuanValue += dLightMS[i];
   for (i=iHeavyStartScan; i<=iHeavyEndScan; i++)
      pQuan->dHeavyQuanValue += dHeavyMS[i];

   memset(dTmpMS, 0, sizeof(double)*(iAnalysisLastScan+5));
   FILTER_MS(dLightMS, dLightFilteredMS, dTmpMS, dMaxLightInten,
      iAnalysisLastScan+5, iPlotStartScan, iPlotEndScan);


   memset(dTmpMS, 0, sizeof(double)*(iAnalysisLastScan+5));
   FILTER_MS(dHeavyMS, dHeavyFilteredMS, dTmpMS, dMaxHeavyInten,
      iAnalysisLastScan+5, iPlotStartScan, iPlotEndScan); 

   if (dLightMS[iLightStartScan]!=0.0)
      iLightStartScan--;
   if (dLightMS[iLightEndScan]!=0.0)
      iLightEndScan++;

   if (dHeavyMS[iHeavyStartScan]!=0.0)
      iHeavyStartScan--;
   if (dHeavyMS[iHeavyEndScan]!=0.0)
      iHeavyEndScan++;

   MAKE_PLOT(iPlotStartScan, iPlotEndScan, iLightStartScan, iLightEndScan,
      iHeavyStartScan, iHeavyEndScan, bNormalize, dMaxLightInten, dMaxHeavyInten,
      dLightMS, dHeavyMS, dLightFilteredMS, dHeavyFilteredMS, szXMLFile, pQuan);

   free(dLightMS);
   free(dHeavyMS);

} /*GET_QUANTITATION*/


void MAKE_PLOT(int iPlotStartScan,
        int iPlotEndScan,
        int iLightStartScan,
        int iLightEndScan,
        int iHeavyStartScan,
        int iHeavyEndScan, 
        int bNormalize,
        double dMaxLightInten,
        double dMaxHeavyInten,
        double *pdLight,
        double *pdHeavy,
        double *pdLightFiltered,
        double *pdHeavyFiltered,
	       char *szXMLFile,
        struct QuanStruct *pQuan)
{
   int  i,
        iImageWidth=650,
        iImageHeight=200,
        iBottomBorder=20,
        iTopBorder=10,
        iGrey,
        iGrey2,
        iWhite,
        iWhite2,
        iBlack,
        iBlack2,
        iRed,
        iRed2,
        iBlue,
        iBlue2,
        rand1,
        rand2;
   char szImageFile[SIZE_FILE],
        szImageFile2[SIZE_FILE],
        szImageDir[SIZE_FILE],
        szImageLink[SIZE_FILE],
        szLabel[SIZE_FILE],
        szWebserverRoot[SIZE_FILE];
   const char *pStr;
   FILE *fp;
   time_t tStartTime;

   gdImagePtr gdImageLight,
              gdImageHeavy;


  /*
   * first color allocated defines background color
   */
   gdImageLight = gdImageCreate(iImageWidth, iImageHeight);   
   iWhite =  gdImageColorAllocate(gdImageLight, 248, 255, 255);
   iGrey  =  gdImageColorAllocate(gdImageLight, 150, 150, 150);
   iBlack =  gdImageColorAllocate(gdImageLight, 0, 0, 0);
   iRed   =  gdImageColorAllocate(gdImageLight, 255, 0, 0);
   iBlue  =  gdImageColorAllocate(gdImageLight, 0, 0, 255);


   gdImageHeavy = gdImageCreate(iImageWidth, iImageHeight);   
   iWhite2 = gdImageColorAllocate(gdImageHeavy, 248, 255, 255);
   iGrey2  = gdImageColorAllocate(gdImageHeavy, 150, 150, 150);
   iBlack2 = gdImageColorAllocate(gdImageHeavy, 0, 0, 0);
   iRed2   = gdImageColorAllocate(gdImageHeavy, 255, 0, 0);
   iBlue2  = gdImageColorAllocate(gdImageHeavy, 0, 0, 255);

   sprintf(szLabel, "Light: %0.3f m/z, %+d",
      (pQuan->dLightPeptideMass + dProton*(pQuan->iChargeState -1)) /(pQuan->iChargeState),
      pQuan->iChargeState);
   gdImageString(gdImageLight, gdFontSmall, 3, 3, (unsigned char *)szLabel, iBlue);
   sprintf(szLabel, "area: %0.2E", pQuan->dLightQuanValue);
   gdImageString(gdImageLight, gdFontSmall, 3, 15, (unsigned char *)szLabel, iBlue);
   sprintf(szLabel, "max_inten: %0.2E", dMaxLightInten);
   gdImageString(gdImageLight, gdFontSmall, 3, 27, (unsigned char *)szLabel, iBlue);
 
   sprintf(szLabel, "Heavy: %0.3f m/z, %+d",
      (pQuan->dHeavyPeptideMass + dProton*(pQuan->iChargeState -1)) /(pQuan->iChargeState),
      pQuan->iChargeState);
   gdImageString(gdImageHeavy, gdFontSmall, 3, 3, (unsigned char *)szLabel, iBlue2);
   sprintf(szLabel,"area: %0.2E", pQuan->dHeavyQuanValue);
   gdImageString(gdImageHeavy, gdFontSmall, 3, 15, (unsigned char *)szLabel, iBlue2);
   sprintf(szLabel,"max inten: %0.2E", dMaxHeavyInten);
   gdImageString(gdImageHeavy, gdFontSmall, 3, 27, (unsigned char *)szLabel, iBlue2); 

   if (bNormalize)
   {
      if (dMaxLightInten > dMaxHeavyInten)
         dMaxHeavyInten = dMaxLightInten;
      else if (dMaxLightInten < dMaxHeavyInten)
         dMaxLightInten = dMaxHeavyInten;
   }

  /*
   * Plot out spectra
   */
   for (i=iPlotStartScan; i<=iPlotEndScan; i++)
   {
      int iX1Pos,
          iY1PosLight,
          iY1PosHeavy;

      iX1Pos= (int)( (i-iPlotStartScan)*iImageWidth/(iPlotEndScan-iPlotStartScan));

      if (dMaxLightInten>0.0)
         iY1PosLight = iImageHeight - iBottomBorder -
            (int)((pdLight[i]/dMaxLightInten)*(iImageHeight-iBottomBorder-iTopBorder));
      else
         iY1PosLight = iImageHeight - iBottomBorder;

      if (dMaxHeavyInten>0.0)
         iY1PosHeavy = iImageHeight - iBottomBorder -
            (int)((pdHeavy[i]/dMaxHeavyInten)*(iImageHeight-iBottomBorder-iTopBorder));
      else
         iY1PosHeavy = iImageHeight - iBottomBorder;

      /*
       * plot light chromatogram points
       */
      if (i>=iLightStartScan && i<iLightEndScan)
         gdImageLine(gdImageLight, iX1Pos, iImageHeight-iBottomBorder, iX1Pos, iY1PosLight, iRed);
      else
         gdImageLine(gdImageLight, iX1Pos, iImageHeight-iBottomBorder, iX1Pos, iY1PosLight, iGrey);

      /*
       * plot heavy chromatogram points
       */
      if (i>=iHeavyStartScan && i<iHeavyEndScan)
         gdImageLine(gdImageHeavy, iX1Pos, iImageHeight-iBottomBorder, iX1Pos, iY1PosHeavy, iRed2);
      else
         gdImageLine(gdImageHeavy, iX1Pos, iImageHeight-iBottomBorder, iX1Pos, iY1PosHeavy, iGrey2);

      /*
       * Plot out smoothed trace
       */
      if (dMaxLightInten>0.0)
      {
         iY1PosLight = iImageHeight - iBottomBorder -
            (int)((pdLightFiltered[i]/dMaxLightInten)*(iImageHeight-iBottomBorder-iTopBorder));
      }
      else
      {
         iY1PosLight= iImageHeight - iBottomBorder;
      }

      if (dMaxHeavyInten>0.0)
      {
         iY1PosHeavy = iImageHeight - iBottomBorder -
            (int)((pdHeavyFiltered[i]/dMaxHeavyInten)*(iImageHeight-iBottomBorder-iTopBorder));
      }
      else
      {
         iY1PosHeavy = iImageHeight - iBottomBorder;
      }

      if (i>=iLightStartScan && i<iLightEndScan)
         gdImageSetPixel(gdImageLight, iX1Pos, iY1PosLight, iBlue);
      else
         gdImageSetPixel(gdImageLight, iX1Pos, iY1PosLight, iRed);

      if (i>=iHeavyStartScan && i<iHeavyEndScan)
         gdImageSetPixel(gdImageHeavy, iX1Pos, iY1PosHeavy,  iBlue2);
      else
         gdImageSetPixel(gdImageHeavy, iX1Pos, iY1PosHeavy,  iRed2); 


      sprintf(szLabel, "%d", i);

     /*
      * x-label, tick marks
      */
      if (iPlotEndScan-iPlotStartScan<150)
      {
         if ( !(i%10))
         {
            gdImageString(gdImageLight, gdFontSmall, iX1Pos-10, iImageHeight-13, (unsigned char *)szLabel, iBlue);
            gdImageString(gdImageHeavy, gdFontSmall, iX1Pos-10, iImageHeight-13, (unsigned char *)szLabel, iBlue2);
         }
         if ( !(i%5))
         {
           /*
            * big tick marks
            */
            gdImageLine(gdImageLight, iX1Pos, iImageHeight-iBottomBorder,
               iX1Pos, iImageHeight-iBottomBorder+5, iBlack);
            gdImageLine(gdImageHeavy, iX1Pos, iImageHeight-iBottomBorder,
               iX1Pos, iImageHeight-iBottomBorder+5, iBlack2);
         }

           /*
            * tick marks
            */
            gdImageLine(gdImageLight, iX1Pos, iImageHeight-iBottomBorder,
               iX1Pos, iImageHeight-iBottomBorder+2, iBlack);
            gdImageLine(gdImageHeavy, iX1Pos, iImageHeight-iBottomBorder,
               iX1Pos, iImageHeight-iBottomBorder+2, iBlack2);
      }
      else if  (iPlotEndScan-iPlotStartScan<500)
      {
         if ( !(i%50))
         {
            gdImageString(gdImageLight, gdFontSmall, iX1Pos-10, iImageHeight-13, (unsigned char *)szLabel, iBlue);
            gdImageString(gdImageHeavy, gdFontSmall, iX1Pos-10, iImageHeight-13, (unsigned char *)szLabel, iBlue2);
         }
         if ( !(i%10))
         {
           /*
            * big tick marks
            */
            gdImageLine(gdImageLight, iX1Pos, iImageHeight-iBottomBorder,
               iX1Pos, iImageHeight-iBottomBorder+5, iBlack);
            gdImageLine(gdImageHeavy, iX1Pos, iImageHeight-iBottomBorder,
               iX1Pos, iImageHeight-iBottomBorder+5, iBlack2);
         }
         if ( !(i%5))
         {
           /*
            * tick marks
            */
            gdImageLine(gdImageLight, iX1Pos, iImageHeight-iBottomBorder,
               iX1Pos, iImageHeight-iBottomBorder+2, iBlack);
            gdImageLine(gdImageHeavy, iX1Pos, iImageHeight-iBottomBorder,
               iX1Pos, iImageHeight-iBottomBorder+2, iBlack2);
         }
      } 
      else
      {
         if ( !(i%100))
         {
            gdImageString(gdImageLight, gdFontSmall, iX1Pos-10, iImageHeight-13, (unsigned char *)szLabel, iBlue);
            gdImageString(gdImageHeavy, gdFontSmall, iX1Pos-10, iImageHeight-13, (unsigned char *)szLabel, iBlue2);
         }
         if ( !(i%50))
         {
           /*
            * big tick marks
            */
            gdImageLine(gdImageLight, iX1Pos, iImageHeight-iBottomBorder,
               iX1Pos, iImageHeight-iBottomBorder+5, iBlack);
            gdImageLine(gdImageHeavy, iX1Pos, iImageHeight-iBottomBorder,
               iX1Pos, iImageHeight-iBottomBorder+5, iBlack2);
         }
         if ( !(i%10))
         {
           /*
            * tick marks
            */
            gdImageLine(gdImageLight, iX1Pos, iImageHeight-iBottomBorder,
               iX1Pos, iImageHeight-iBottomBorder+2, iBlack);
            gdImageLine(gdImageHeavy, iX1Pos, iImageHeight-iBottomBorder,
               iX1Pos, iImageHeight-iBottomBorder+2, iBlack2);
         }
      } 
   }


  /*
   * Draw axis
   */
   gdImageLine(gdImageLight, 0, iImageHeight-iBottomBorder,
      iImageWidth-1, iImageHeight-iBottomBorder, iBlack);
   gdImageLine(gdImageHeavy, 0, iImageHeight-iBottomBorder,
      iImageWidth-1, iImageHeight-iBottomBorder, iBlack2);

  /*
   * Draw box around image
   */
   gdImageRectangle(gdImageLight, 0, 0, iImageWidth-1, iImageHeight-1, iBlack);
   gdImageRectangle(gdImageHeavy, 0, 0, iImageWidth-1, iImageHeight-1, iBlack2);


  /*
   * Create the image file ... image_buffer needs to point to data
   */

/*
 * These definitely need to be read from somewhere ... szImageDir
 */

   //#ifdef ISB_VERSION
   //sprintf(szImageDir, "/tmp/");
   //sprintf(szImageLink, "/tmp/");
   //#else

   pStr=getWebserverRoot();
   if (pStr==NULL)
   {
      printf("<PRE> Environment variable WEBSERVER_ROOT does not exist.\n\n");
#ifdef WINDOWS_CYGWIN
      printf(" For Windows users, you can set this environment variable\n");
      printf(" through the Advanced tab under System Properties when you\n");
      printf(" right-mouse-click on your My Computer icon.\n\n");

      printf(" Set this environment variable to your webserver's document\n");
      printf(" root directory such as c:\\inetpub\\wwwroot for IIS or\n");
      printf(" c:\\website\\htdocs or WebSite Pro.\n\n");
#endif
      printf(" Exiting.\n");
      exit(0);
   }
   else
   {
     // must first pass to cygpath program
#ifdef WINDOWS_CYGWIN
     char szCommand[SIZE_FILE];
     sprintf(szCommand, "cygpath '%s'", pStr);
     if((fp = popen(szCommand, "r")) == NULL)
     {
       printf("cygpath error, exiting\n");
       exit(1);
     }
     else
     {
       char szBuf[SIZE_FILE];
       fgets(szBuf, SIZE_BUF, fp);
       pclose(fp);
       szBuf[strlen(szBuf)-1] = 0;
       strcpy(szWebserverRoot, szBuf);
     }
#else
     strcpy(szWebserverRoot, pStr);
#endif
   }
    /*
    * Check if szWebserverRoot is present
    */
   if (access(szWebserverRoot, F_OK))
   {
      printf(" Cannot access the webserver's root directory:\n");
      printf("    %s\n", szWebserverRoot);
      printf(" This was set as the environment variable WEBSERVER_ROOT\n\n");

      printf(" For Windows users, you can check this environment variable\n");
      printf(" through the Advanced tab under System Properties when you\n");
      printf(" right-mouse-click on your My Computer icon.\n\n");
      printf(" Exiting.\n");
      exit(1);
   }
   
   //DDS:
   strcpy(szImageDir, szXMLFile);
   replace_path_with_webserver_tmp(szImageDir,sizeof(szImageDir)); // write these in designated tmp area
   char* tmpStr = findRightmostPathSeperator(szImageDir); 
   if (tmpStr++)
     *tmpStr = '\0';
   strcpy(szImageLink, szImageDir);
   translate_absolute_filesystem_path_to_relative_webserver_root_path(szImageLink);

   //if (szWebserverRoot[strlen(szWebserverRoot)-1] == '/') {
   //  sprintf(szImageDir, "%stmp/", szWebserverRoot);
   //}
   //else {
   //  sprintf(szImageDir, "%s/tmp/", szWebserverRoot);
   //}
   //sprintf(szImageLink, "/tmp/");
   

  /*
   * Create the image file ... make unique name for each image file
   */
   srandom(pQuan->iLightFirstScan+pQuan->iHeavyFirstScan+ (int)(pQuan->dLightPeptideMass*5.0));
   tStartTime=time((time_t *)NULL);
  
   rand1 = random(); rand2 = random();
   
   strcpy(szImageFile, szImageDir);
   strcpy(szImageFile2, szImageDir);
   sprintf(szImageFile+strlen(szImageFile), "x%ld-%d.png", (long)tStartTime, rand1);
   sprintf(szImageFile2+strlen(szImageFile2), "x%ld-%db.png", (long)tStartTime, rand2);

   gdImageInterlace(gdImageLight, 1);
   if ( (fp=fopen(szImageFile, "wb"))!=NULL)
   {
	  // seem to need the memory based version for MSVC to avoid a crash
	  int sz;
	  char *img = (char *)gdImagePngPtr(gdImageLight, &sz); 
	  if (img) {
	    size_t fwrote = fwrite(img, 1, sz, fp);
	    free(img);
	  } else {
		printf(" Error - cannot create file %s<BR>\n", szImageFile);
	  }
      fclose(fp);
      gdImageDestroy(gdImageLight);  
   }
   else
      printf(" Error - cannot create file %s<BR>\n", szImageFile);

   gdImageInterlace(gdImageHeavy, 1);
   if ( (fp=fopen(szImageFile2, "wb"))!=NULL)
   {
	  // seem to need the memory based version for MSVC to avoid a crash
	  int sz;
	  char *img = (char *)gdImagePngPtr(gdImageHeavy, &sz); 
	  if (img) {
	    size_t fwrote = fwrite(img, 1, sz, fp);
	    free(img);
	  } else {
		printf(" Error - cannot create file %s<BR>\n", szImageFile2);
	  }
      fclose(fp);
      gdImageDestroy(gdImageHeavy);
   }
   else
      printf(" Error - cannot create file %s<BR>\n", szImageFile2);
   strcpy(szImageFile, szImageLink);
   strcpy(szImageFile2, szImageLink);
   sprintf(szImageFile+strlen(szImageFile), "x%ld-%d.png", (long)tStartTime, rand1);
   sprintf(szImageFile2+strlen(szImageFile2), "x%ld-%db.png", (long)tStartTime, rand2);
   printf("<CENTER><img src=\"%s\"><BR><img src=\"%s\"></CENTER>", 
	   makeTmpPNGFileSrcRef(szImageFile).c_str(), 
	   makeTmpPNGFileSrcRef(szImageFile2).c_str());

} /*MAKE_PLOT*/



#define FILTER_SIZE 4
/*
 * Use my standard filtering routine
 */
void FILTER_MS(double *dOrigMS,
        double *dFilteredMS,
        double *dTmpFilter,
        double dMaxInten,
        int    iNumMSScans,
        int    iPlotStartScan,
        int    iPlotEndScan)
{
   int  i,
        iArraySize=iNumMSScans*sizeof(double);
   double dTmpMax;
 
/*
   Defines 5th order butterworth filter w/ cut-off frequency
   of 0.25 where 1.0 corresponse to half the sample rate.

   5th order, 0.010
   double a[FILTER_SIZE]={1.0000, -4.8983, 9.5985, -9.4053, 4.6085, -0.9033},
          b[FILTER_SIZE]={0.000000000909, 0.000000004546, 0.000000009093, 0.000000009093, 0.000000004546, 0.000000000909};
          

   5th order, 0.075
   double a[FILTER_SIZE]={1.0000, -4.2380, 7.2344, -6.2125, 2.5821, -0.4655},
          b[FILTER_SIZE]={0.0158, 0.0792, 0.1585, 0.1585, 0.0792, 0.0158};

   5th order, 0.10
   double a[FILTER_SIZE]={1.0000, -3.9845, 6.4349, -5.2536, 2.1651, -0.3599},
          b[FILTER_SIZE]={0.0000598, 0.0002990, 0.0005980, 0.0005980, 0.0002990, 0.0000598};

   5th order, 0.15
   double a[FILTER_SIZE]={1.0000, -3.4789, 5.0098, -3.6995, 1.3942, -0.2138},
          b[FILTER_SIZE]={0.0004, 0.0018, 0.0037, 0.0037, 0.0018, 0.0004};

   5th order, 0.20
   double a[FILTER_SIZE]={1.0000, -2.9754, 3.8060, -2.5453, 0.8811, -0.1254},
          b[FILTER_SIZE]={0.0013, 0.0064, 0.0128, 0.0128, 0.0064, 0.0013};

   5th order, 0.25
   double a[FILTER_SIZE]={1.0, -2.4744, 2.8110, -1.7038, 0.5444, -0.0723},
          b[FILTER_SIZE]={0.0033, 0.0164, 0.0328, 0.0328, 0.0164, 0.0033};
*/

/*
   3rd order butterworth, 0.025 cutoff frequency
*/
   double a[FILTER_SIZE]={1.0,-2.8430, 2.6980, -0.8546},
          b[FILTER_SIZE]={0.0000561, 0.0001682, 0.0001682, 0.0000561};
 
   memset(dFilteredMS, 0, iArraySize);
   memcpy(dTmpFilter, dOrigMS, iNumMSScans*sizeof(double));
 
  /*
   * Pass MS profile through IIR low pass filter:
   * y(n) = b(1)*x(n) + b(2)*x(n-1) + ... + b(nb+1)*x(n-nb)
   *      - a(2)*y(n-1) - ... - a(na+1)*y(n-na)
   */
   for (i=0; i<iNumMSScans; i++)
   {
      int ii;

      dFilteredMS[i]=b[0]*dTmpFilter[i];
      for (ii=1;ii<FILTER_SIZE;ii++)
      {
         if ((i-ii)>=0)
         {
            dFilteredMS[i] += b[ii]*dTmpFilter[i-ii];
            dFilteredMS[i] -= a[ii]*dFilteredMS[i-ii];
         }
      }
   }
 
  /*
   * Filtered sequence is reversed and re-filtered resulting
   * in zero-phase distortion and double the filter order.
   */
   for (i=0; i<iNumMSScans; i++)
      dTmpFilter[i]=dFilteredMS[iNumMSScans-1-i];
 
   memset(dFilteredMS, 0, iArraySize);
   for (i=0; i<iNumMSScans; i++) 
   {
      int ii;
 
      dFilteredMS[i]=b[0]*dTmpFilter[i];
      for (ii=1;ii<FILTER_SIZE;ii++)
      {
         if ((i-ii)>=0)
         {
            dFilteredMS[i] += b[ii]*dTmpFilter[i-ii];
            dFilteredMS[i] -= a[ii]*dFilteredMS[i-ii];
         }
      }
   }
 
  /*
   * Filtered sequence is reversed again
   */
   dTmpMax=0.001;
   for (i=0; i<iNumMSScans; i++)
   {
      dTmpFilter[i]=dFilteredMS[iNumMSScans-1-i];

      if (i>=iPlotStartScan && i<=iPlotEndScan)
         if (dTmpFilter[i]>dTmpMax)
            dTmpMax=dTmpFilter[i];
   }

   if (dMaxInten>0.0)
   {
      for (i=iPlotStartScan; i<=iPlotEndScan; i++)
      {
         dTmpFilter[i] = dTmpFilter[i] * dMaxInten / dTmpMax;
      }
   }

   memcpy(dFilteredMS, dTmpFilter, iArraySize);
 
} /*FILTER_MS*/


void SPECIAL_QUAN() {
   printf("<TABLE><TR><TD>");
   printf("<FORM METHOD=GET ACTION=\"%s\">", getenv("SCRIPT_NAME"));
   printf("<INPUT TYPE=\"hidden\" NAME=\"InteractDir\" VALUE=\"%s\">\n", szInteractDir);
   printf("<INPUT TYPE=\"hidden\" NAME=\"InteractBaseName\" VALUE=\"%s\">\n", szInteractBaseName);
   printf("<INPUT TYPE=\"hidden\" NAME=\"bXpressLight1\" VALUE=\"%d\">\n", bXpressLight1);

   printf("<INPUT TYPE=\"hidden\" NAME=\"LightFirstScan\" VALUE=\"%d\">\n", pQuan.iLightFirstScan);
   printf("<INPUT TYPE=\"hidden\" NAME=\"LightLastScan\"  VALUE=\"%d\">\n", pQuan.iLightLastScan);
   printf("<INPUT TYPE=\"hidden\" NAME=\"LightMass\" VALUE=\"%0.3f\">\n", pQuan.dLightPeptideMass);
   printf("<INPUT TYPE=\"hidden\" NAME=\"MassTol\" VALUE=\"%0.2f\">", pQuan.dMassTol);
   printf("<INPUT TYPE=\"hidden\" NAME=\"PpmTol\" VALUE=\"%d\">", pQuan.bPpmMassTol);
   printf("<INPUT TYPE=\"hidden\" NAME=\"HeavyFirstScan\" VALUE=\"%d\">\n", pQuan.iHeavyFirstScan);
   printf("<INPUT TYPE=\"hidden\" NAME=\"HeavyLastScan\"  VALUE=\"%d\">\n", 0);
   printf("<INPUT TYPE=\"hidden\" NAME=\"HeavyMass\" VALUE=\"%0.3f\">\n", pQuan.dHeavyPeptideMass);
   printf("<input TYPE=\"hidden\" NAME=\"XMLFile\" VALUE=\"%s\">", szXMLFile);
   printf("<input TYPE=\"hidden\" NAME=\"OutFile\" VALUE=\"%s\">", szOutputFile);
   printf("<input TYPE=\"hidden\" NAME=\"xmlfile\" VALUE=\"%s\">", update_xmlfile);
   printf("<input TYPE=\"hidden\" NAME=\"index\" VALUE=\"%d\">", update_index);
   printf("<input TYPE=\"hidden\" NAME=\"overwrite\" VALUE=\"1\">");
   printf("<INPUT TYPE=\"hidden\" NAME=\"ChargeState\" VALUE=\"%d\">", pQuan.iChargeState);
   printf("<INPUT TYPE=\"hidden\" NAME=\"SwapMass\" VALUE=\"%d\">", pQuan.iSwapMass);
   printf("<INPUT TYPE=\"hidden\" NAME=\"NewQuan\" VALUE=\"%s\">\n", "1:0.00*");
   printf("<INPUT TYPE=\"hidden\" NAME=\"ZeroArea\" VALUE=\"H\">\n");
   printf("<INPUT TYPE=\"submit\" VALUE=\"1:0.00\">");

   printf("</FORM>");
   printf("</TD><TD>");
   printf("<FORM METHOD=GET ACTION=\"%s\">", getenv("SCRIPT_NAME"));

   printf("<INPUT TYPE=\"hidden\" NAME=\"InteractDir\" VALUE=\"%s\">\n", szInteractDir);
   printf("<INPUT TYPE=\"hidden\" NAME=\"InteractBaseName\" VALUE=\"%s\">\n", szInteractBaseName);
   printf("<INPUT TYPE=\"hidden\" NAME=\"bXpressLight1\" VALUE=\"%d\">\n", bXpressLight1);

   printf("<INPUT TYPE=\"hidden\" NAME=\"LightFirstScan\" VALUE=\"%d\">\n", pQuan.iLightFirstScan);
   printf("<INPUT TYPE=\"hidden\" NAME=\"LightLastScan\"  VALUE=\"%d\">\n", 0);
   printf("<INPUT TYPE=\"hidden\" NAME=\"LightMass\" VALUE=\"%0.3f\">\n", pQuan.dLightPeptideMass);
   printf("<INPUT TYPE=\"hidden\" NAME=\"MassTol\" VALUE=\"%0.2f\">", pQuan.dMassTol);
   printf("<INPUT TYPE=\"hidden\" NAME=\"PpmTol\" VALUE=\"%d\">", pQuan.bPpmMassTol);
   printf("<INPUT TYPE=\"hidden\" NAME=\"HeavyFirstScan\" VALUE=\"%d\">\n", pQuan.iHeavyFirstScan);
   printf("<INPUT TYPE=\"hidden\" NAME=\"HeavyLastScan\"  VALUE=\"%d\">\n", pQuan.iHeavyLastScan);
   printf("<INPUT TYPE=\"hidden\" NAME=\"HeavyMass\" VALUE=\"%0.3f\">\n", pQuan.dHeavyPeptideMass);

   printf("<input TYPE=\"hidden\" NAME=\"XMLFile\" VALUE=\"%s\">", szXMLFile);
   printf("<input TYPE=\"hidden\" NAME=\"OutFile\" VALUE=\"%s\">", szOutputFile);
   printf("<input TYPE=\"hidden\" NAME=\"xmlfile\" VALUE=\"%s\">", update_xmlfile);
   printf("<input TYPE=\"hidden\" NAME=\"index\" VALUE=\"%d\">", update_index);
   printf("<input TYPE=\"hidden\" NAME=\"overwrite\" VALUE=\"1\">");
   printf("<INPUT TYPE=\"hidden\" NAME=\"ChargeState\" VALUE=\"%d\">", pQuan.iChargeState);
   printf("<INPUT TYPE=\"hidden\" NAME=\"SwapMass\" VALUE=\"%d\">", pQuan.iSwapMass);
   printf("<INPUT TYPE=\"hidden\" NAME=\"NewQuan\" VALUE=\"%s\">\n", "0.00:1*");
   printf("<INPUT TYPE=\"hidden\" NAME=\"ZeroArea\" VALUE=\"L\">\n");
   printf("<INPUT TYPE=\"submit\" VALUE=\"0.00:1\">");


   printf("</FORM>");

   printf("</TD><TD>");

   printf("<FORM METHOD=GET ACTION=\"%s\">", getenv("SCRIPT_NAME"));

   printf("<INPUT TYPE=\"hidden\" NAME=\"InteractDir\" VALUE=\"%s\">\n", szInteractDir);
   printf("<INPUT TYPE=\"hidden\" NAME=\"InteractBaseName\" VALUE=\"%s\">\n", szInteractBaseName);
   printf("<INPUT TYPE=\"hidden\" NAME=\"bXpressLight1\" VALUE=\"%d\">\n", bXpressLight1);

   printf("<INPUT TYPE=\"hidden\" NAME=\"LightFirstScan\" VALUE=\"%d\">\n", pQuan.iLightFirstScan);
   printf("<INPUT TYPE=\"hidden\" NAME=\"LightLastScan\"  VALUE=\"%d\">\n", pQuan.iLightFirstScan - 1);
   printf("<INPUT TYPE=\"hidden\" NAME=\"LightMass\" VALUE=\"%0.3f\">\n", pQuan.dLightPeptideMass);
   printf("<INPUT TYPE=\"hidden\" NAME=\"MassTol\" VALUE=\"%0.2f\">", pQuan.dMassTol);
   printf("<INPUT TYPE=\"hidden\" NAME=\"PpmTol\" VALUE=\"%d\">", pQuan.bPpmMassTol);
   printf("<INPUT TYPE=\"hidden\" NAME=\"HeavyFirstScan\" VALUE=\"%d\">\n", pQuan.iHeavyFirstScan);
   printf("<INPUT TYPE=\"hidden\" NAME=\"HeavyLastScan\"  VALUE=\"%d\">\n", pQuan.iHeavyFirstScan - 1); //pQuan.iHeavyLastScan);
   printf("<INPUT TYPE=\"hidden\" NAME=\"HeavyMass\" VALUE=\"%0.3f\">\n", pQuan.dHeavyPeptideMass);

   printf("<input TYPE=\"hidden\" NAME=\"XMLFile\" VALUE=\"%s\">", szXMLFile);
   printf("<input TYPE=\"hidden\" NAME=\"OutFile\" VALUE=\"%s\">", szOutputFile);
   printf("<input TYPE=\"hidden\" NAME=\"xmlfile\" VALUE=\"%s\">", update_xmlfile);
   printf("<input TYPE=\"hidden\" NAME=\"index\" VALUE=\"%d\">", update_index);
   printf("<input TYPE=\"hidden\" NAME=\"overwrite\" VALUE=\"1\">");
   printf("<INPUT TYPE=\"hidden\" NAME=\"ChargeState\" VALUE=\"%d\">", pQuan.iChargeState);
   printf("<INPUT TYPE=\"hidden\" NAME=\"SwapMass\" VALUE=\"%d\">", pQuan.iSwapMass);
   printf("<INPUT TYPE=\"hidden\" NAME=\"NewQuan\" VALUE=\"%s*\">\n", pQuan.szNewQuan);
   printf("<INPUT TYPE=\"hidden\" NAME=\"NewQuan\" VALUE=\"%s\">\n", "?*");
   printf("<INPUT TYPE=\"hidden\" NAME=\"ZeroArea\" VALUE=\"LH\">\n");

   printf("<INPUT TYPE=\"submit\" VALUE=\"?\">");

   printf("</FORM>");
   printf("</TD></TR></TABLE>");
}

void UPDATE_QUAN() {

   //printf("</td></tr><tr><td colspan=\"3\">");  
   printf("<FORM METHOD=GET ACTION=\"%s\">", getenv("SCRIPT_NAME"));

   printf("<INPUT TYPE=\"hidden\" NAME=\"InteractDir\" VALUE=\"%s\">\n", szInteractDir);
   printf("<INPUT TYPE=\"hidden\" NAME=\"InteractBaseName\" VALUE=\"%s\">\n", szInteractBaseName);
   printf("<INPUT TYPE=\"hidden\" NAME=\"bXpressLight1\" VALUE=\"%d\">\n", bXpressLight1);

   printf("<INPUT TYPE=\"hidden\" NAME=\"LightFirstScan\" VALUE=\"%d\">\n", pQuan.iLightFirstScan);
   printf("<INPUT TYPE=\"hidden\" NAME=\"LightLastScan\"  VALUE=\"%d\">\n", pQuan.iLightLastScan);
   printf("<INPUT TYPE=\"hidden\" NAME=\"LightMass\" VALUE=\"%0.3f\">\n", pQuan.dLightPeptideMass);
   printf("<INPUT TYPE=\"hidden\" NAME=\"MassTol\" VALUE=\"%0.2f\">", pQuan.dMassTol);
   printf("<INPUT TYPE=\"hidden\" NAME=\"PpmTol\" VALUE=\"%d\">", pQuan.bPpmMassTol);
   printf("<INPUT TYPE=\"hidden\" NAME=\"HeavyFirstScan\" VALUE=\"%d\">\n", pQuan.iHeavyFirstScan);
   printf("<INPUT TYPE=\"hidden\" NAME=\"HeavyLastScan\"  VALUE=\"%d\">\n", pQuan.iHeavyLastScan);
   printf("<INPUT TYPE=\"hidden\" NAME=\"HeavyMass\" VALUE=\"%0.3f\">\n", pQuan.dHeavyPeptideMass);

   printf("<input TYPE=\"hidden\" NAME=\"XMLFile\" VALUE=\"%s\">", szXMLFile);
   printf("<input TYPE=\"hidden\" NAME=\"OutFile\" VALUE=\"%s\">", szOutputFile);
   printf("<input TYPE=\"hidden\" NAME=\"xmlfile\" VALUE=\"%s\">", update_xmlfile);
   printf("<input TYPE=\"hidden\" NAME=\"index\" VALUE=\"%d\">", update_index);
   printf("<input TYPE=\"hidden\" NAME=\"overwrite\" VALUE=\"1\">");
   printf("<INPUT TYPE=\"hidden\" NAME=\"ChargeState\" VALUE=\"%d\">", pQuan.iChargeState);
   printf("<INPUT TYPE=\"hidden\" NAME=\"SwapMass\" VALUE=\"%d\">", pQuan.iSwapMass);
   printf("<INPUT TYPE=\"submit\" VALUE=\"Save Updated Ratio\">");

   printf("</FORM>");
   //printf("</tr>");
   //printf("</CENTER>\n");

  if(overwrite) {

    // printf("overwriting xml data\n");

    if(strlen(zero_area) > 0) {
      if(strchr(zero_area, 'L') != NULL)
         pQuan.dLightQuanValue = 0.0;
      if(strchr(zero_area, 'H') != NULL)
         pQuan.dHeavyQuanValue = 0.0;
    }
    if(strlen(update_quan) > 0) {
      strcpy(pQuan.szNewQuan, update_quan);
    }

    //cout << "light: " << pQuan.dLightQuanValue << ", heavy: " << pQuan.dHeavyQuanValue << ", ratio: " << pQuan.szNewQuan << endl;


    char szBuf[4096];
    Tag* replacement = new Tag("xpressratio_result", True, True);
  
    sprintf(szBuf, "%d", pQuan.iLightFirstScan);
    replacement->setAttributeValue("light_firstscan", szBuf);
    sprintf(szBuf, "%d", pQuan.iLightLastScan);
    replacement->setAttributeValue("light_lastscan", szBuf);

    sprintf(szBuf, "%0.3f", pQuan.dLightPeptideMass);
    replacement->setAttributeValue("light_mass", szBuf);
    sprintf(szBuf, "%d", pQuan.iHeavyFirstScan);
    replacement->setAttributeValue("heavy_firstscan", szBuf);
    sprintf(szBuf, "%d", pQuan.iHeavyLastScan);
    replacement->setAttributeValue("heavy_lastscan", szBuf);
    sprintf(szBuf, "%0.3f", pQuan.dHeavyPeptideMass);
    replacement->setAttributeValue("heavy_mass", szBuf);
    //if(bXpressLight1)
    //  sprintf(szBuf, "%d", 1);
    //else
    //  sprintf(szBuf, "%d", 0);
    //replacement->setAttributeValue("xpresslight", szBuf);
    sprintf(szBuf, "%0.3f", pQuan.dMassTol);
    replacement->setAttributeValue("mass_tol", szBuf);
    sprintf(szBuf, "%s", pQuan.szNewQuan);

    //printf("NEWQUAN: %s\n",  pQuan.szNewQuan);

    // here check for light to heavy



    replacement->setAttributeValue("ratio", szBuf);

    // now the flipped guy....
    flipRatio(szBuf, szBuf);
    replacement->setAttributeValue("heavy2light_ratio", szBuf);

    sprintf(szBuf, "%0.2e", pQuan.dLightQuanValue);
    replacement->setAttributeValue("light_area", szBuf);
    sprintf(szBuf, "%0.2e", pQuan.dHeavyQuanValue);
    replacement->setAttributeValue("heavy_area", szBuf);

    if(pQuan.dHeavyQuanValue == 0.0)
      if(pQuan.dLightQuanValue > 0.0)
	sprintf(szBuf, "%0.1f", 999.0);
      else 
	sprintf(szBuf, "%0.1f", -1.0);
    else
      sprintf(szBuf, "%0.2f", pQuan.dLightQuanValue/pQuan.dHeavyQuanValue);

    replacement->setAttributeValue("decimal_ratio", szBuf);



    //printf("READY TO UPDATE....\n");
    XPressPeptideUpdateParser* parser = new XPressPeptideUpdateParser(update_xmlfile, update_index, replacement);
    //printf("and now for the results....\n");



    if(parser->update()) {
      printf("</TD></TR><TR><TD COLSPAN=\"3\" ALIGN=CENTER> ratio updated, refresh browser to view change"); // ok
    }
    else {
      printf("</TD></TR><TR><TD COLSPAN=\"3\" ALIGN=CENTER><font color=\"red\"><b>Error: ratio not updated</b></font>"); // ok
    }
  }
  
  /*
   * replace text starting with first ? in the cgixpress tag with szNewLink
   */



} /*UPDATE_QUAN*/

void flipRatio(char* ratio, char* flipped) {
  // find the : and 1
  double left, right;
  if (!strcmp(ratio, "?*")) {
    strcpy(flipped, ratio);
  }
  else {
    sscanf(ratio, "%lf:%lf", &left, &right);
    if(left == 1.0)
      ;
    else if(left == 0.0)
      left = 999;
    else if(left >= 999.)
      left = 0.0;
    else
      left = 1.0 / left;
    if(right == 1.0)
      ;
    else if(right >= 999.)
      right = 0.0;
    else if(right == 0.0)
      right = 999;
    else
      right = 1.0 / right;
    
    if(left == 1.0)
      sprintf(flipped, "1:%0.2f", right);
    else
      sprintf(flipped, "%0.2f:1", left);
  }
}      

void BAD_QUAN()
{
   char szBadQuan[4096];  /* Link to bad quantitation */

   printf("<TABLE BORDER=0><TR><TD>");
  /*
   * first quick button for bad quantitation
   */
   sprintf(szBadQuan                  , "LightFirstScan=%d&amp;", pQuan.iLightFirstScan);
   sprintf(szBadQuan+strlen(szBadQuan), "LightLastScan=%d&amp;",  pQuan.iLightFirstScan-1);
   sprintf(szBadQuan+strlen(szBadQuan), "LightMass=%0.6f&amp;",  pQuan.dLightPeptideMass);
   sprintf(szBadQuan+strlen(szBadQuan), "HeavyFirstScan=%d&amp;", pQuan.iHeavyFirstScan);
   sprintf(szBadQuan+strlen(szBadQuan), "HeavyLastScan=%d&amp;",  pQuan.iHeavyFirstScan-1);
   sprintf(szBadQuan+strlen(szBadQuan), "HeavyMass=%0.6f&amp;",   pQuan.dHeavyPeptideMass);
   sprintf(szBadQuan+strlen(szBadQuan), "XMLFile=%s&amp;",        szXMLFile);
   sprintf(szBadQuan+strlen(szBadQuan), "ChargeState=%d&amp;",    pQuan.iChargeState);
   sprintf(szBadQuan+strlen(szBadQuan), "OutFile=%s&amp;",        szOutputFile);
   sprintf(szBadQuan+strlen(szBadQuan), "MassTol=%0.6f&amp;",    pQuan.dMassTol);
   sprintf(szBadQuan+strlen(szBadQuan), "bXpressLight1=%d&amp;",    bXpressLight1);
   sprintf(szBadQuan+strlen(szBadQuan), "InteractDir=%s&amp;",    szInteractDir);
   sprintf(szBadQuan+strlen(szBadQuan), "InteractBaseName=%s",    szInteractBaseName);

   printf("<FORM METHOD=POST ACTION=\"/%s%s\">", CGI_BIN,XPRESS_UPDATE);
   printf("<INPUT TYPE=\"hidden\" NAME=\"NewLink\" VALUE=\"%s\">\n", szBadQuan);
   printf("<INPUT TYPE=\"hidden\" NAME=\"NewQuan\" VALUE=\"%s\">\n", "?");
   printf("<INPUT TYPE=\"hidden\" NAME=\"InteractDir\" VALUE=\"%s\">\n", szInteractDir);
   printf("<INPUT TYPE=\"hidden\" NAME=\"InteractBaseName\" VALUE=\"%s\">\n", szInteractBaseName);
   printf("<INPUT TYPE=\"hidden\" NAME=\"OutputFile\" VALUE=\"%s\">\n", szOutputFile);
   printf("<p><INPUT TYPE=\"submit\" VALUE=\"?*\"></FORM>");

   printf("</TD><TD>");

  /*
   * second quick button
   */
   sprintf(szBadQuan                  , "LightFirstScan=%d&amp;", pQuan.iLightFirstScan);
   sprintf(szBadQuan+strlen(szBadQuan), "LightLastScan=%d&amp;",  pQuan.iLightLastScan);
   sprintf(szBadQuan+strlen(szBadQuan), "LightMass=%0.6f&amp;",  pQuan.dLightPeptideMass);
   sprintf(szBadQuan+strlen(szBadQuan), "HeavyFirstScan=%d&amp;", pQuan.iHeavyFirstScan);
   sprintf(szBadQuan+strlen(szBadQuan), "HeavyLastScan=%d&amp;",  pQuan.iHeavyFirstScan - 1);
   sprintf(szBadQuan+strlen(szBadQuan), "HeavyMass=%0.6f&amp;",   pQuan.dHeavyPeptideMass);
   sprintf(szBadQuan+strlen(szBadQuan), "XMLFile=%s&amp;",        szXMLFile);
   sprintf(szBadQuan+strlen(szBadQuan), "ChargeState=%d&amp;",    pQuan.iChargeState);
   sprintf(szBadQuan+strlen(szBadQuan), "OutFile=%s&amp;",        szOutputFile);
   sprintf(szBadQuan+strlen(szBadQuan), "MassTol=%0.6f&amp;",    pQuan.dMassTol);
   sprintf(szBadQuan+strlen(szBadQuan), "bXpressLight1=%d&amp;",    bXpressLight1);
   sprintf(szBadQuan+strlen(szBadQuan), "InteractDir=%s&amp;",    szInteractDir);
   sprintf(szBadQuan+strlen(szBadQuan), "InteractBaseName=%s",    szInteractBaseName);

   printf("<FORM METHOD=POST ACTION=\"/%s%s\">", CGI_BIN,XPRESS_UPDATE);
   printf("<INPUT TYPE=\"hidden\" NAME=\"NewLink\" VALUE=\"%s\">\n", szBadQuan);
   printf("<INPUT TYPE=\"hidden\" NAME=\"NewQuan\" VALUE=\"%s\">\n", "1:0.00");
   printf("<INPUT TYPE=\"hidden\" NAME=\"InteractDir\" VALUE=\"%s\">\n", szInteractDir);
   printf("<INPUT TYPE=\"hidden\" NAME=\"InteractBaseName\" VALUE=\"%s\">\n", szInteractBaseName);
   printf("<INPUT TYPE=\"hidden\" NAME=\"OutputFile\" VALUE=\"%s\">\n", szOutputFile);
   printf("<p><INPUT TYPE=\"submit\" VALUE=\"1:0.00*\"></FORM>");

   printf("</TD><TD>");

  /*
   * third quick button
   */
   sprintf(szBadQuan                  , "LightFirstScan=%d&amp;", pQuan.iLightFirstScan);
   sprintf(szBadQuan+strlen(szBadQuan), "LightLastScan=%d&amp;",  pQuan.iLightFirstScan -1);
   sprintf(szBadQuan+strlen(szBadQuan), "LightMass=%0.6f&amp;",  pQuan.dLightPeptideMass);
   sprintf(szBadQuan+strlen(szBadQuan), "HeavyFirstScan=%d&amp;", pQuan.iHeavyFirstScan);
   sprintf(szBadQuan+strlen(szBadQuan), "HeavyLastScan=%d&amp;",  pQuan.iHeavyLastScan);
   sprintf(szBadQuan+strlen(szBadQuan), "HeavyMass=%0.6f&amp;",   pQuan.dHeavyPeptideMass);
   sprintf(szBadQuan+strlen(szBadQuan), "XMLFile=%s&amp;",        szXMLFile);
   sprintf(szBadQuan+strlen(szBadQuan), "ChargeState=%d&amp;",    pQuan.iChargeState);
   sprintf(szBadQuan+strlen(szBadQuan), "OutFile=%s&amp;",        szOutputFile);
   sprintf(szBadQuan+strlen(szBadQuan), "MassTol=%0.6f&amp;",    pQuan.dMassTol);
   sprintf(szBadQuan+strlen(szBadQuan), "bXpressLight1=%d&amp;",    bXpressLight1);
   sprintf(szBadQuan+strlen(szBadQuan), "InteractDir=%s&amp;",    szInteractDir);
   sprintf(szBadQuan+strlen(szBadQuan), "InteractBaseName=%s",    szInteractBaseName);

   printf("<FORM METHOD=POST ACTION=\"/%s%s\">", CGI_BIN,XPRESS_UPDATE);
   printf("<INPUT TYPE=\"hidden\" NAME=\"NewLink\" VALUE=\"%s\">\n", szBadQuan);
   printf("<INPUT TYPE=\"hidden\" NAME=\"NewQuan\" VALUE=\"%s\">\n", "0.00:1");
   printf("<INPUT TYPE=\"hidden\" NAME=\"InteractDir\" VALUE=\"%s\">\n", szInteractDir);
   printf("<INPUT TYPE=\"hidden\" NAME=\"InteractBaseName\" VALUE=\"%s\">\n", szInteractBaseName);
   printf("<INPUT TYPE=\"hidden\" NAME=\"OutputFile\" VALUE=\"%s\">\n", szOutputFile);
   printf("<p><INPUT TYPE=\"submit\" VALUE=\"0.00:1*\"></FORM>");

   printf("</TD></TR></TABLE>\n");

} /*BAD_QUAN*/
