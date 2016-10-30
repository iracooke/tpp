/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "psm2pdf.h"

int main(int argc, char **argv)
{
   int    i;
   int    fd;
   int    iNumArg;
   int    iStartArgc;
   char   *arg;
   char   szBuf[SIZE_BUF];  /* contains a list of URLs + peptide, protein */
   FILE   *fplist;

   struct ParamsStruct pParams;

   iNumArg = 0;
   iStartArgc = 1;
   arg = argv[iNumArg = 1];

   GLOBAL_INIT(&pParams);

   /*
    * First get all command line options.
    */
   while (iNumArg < argc)
   {
      if (arg[0] == '-')
         SET_OPTION(arg, &pParams);
      else
         break;

      iStartArgc++;
      arg = argv[++iNumArg];
   }

   if (argc <= iStartArgc)
   {
      printf("\n");
      USAGE(argv[0]);
      printf("\n");
      exit(1);
   }

   /*
    * Should check existence of webserver (pParams.szHost)
    * as well as htmldoc and gs binaries.
    */

   /*
    * Generate szListURL temp file name
    */
   strcpy(pParams.szListURL, "psm2pdf.XXXXXX");

   if ((fd = mkstemp(pParams.szListURL)) == -1)
   {
      printf(" Error - couldn't make \"%s\", error: %d\n", pParams.szListURL, errno);
      exit(1);
   }
   close(fd);

   if ((fplist = fopen(pParams.szListURL, "w"))==NULL)
   {
      printf(" Error - could not open \"%s\"\n", pParams.szListURL);
      exit(1);
   }

   /*
    * Now read through each pep.xml file specified on command line,
    * generate plot-msms URLs, and store those entries that pass
    * filters into pParams.szListURL
    */
   printf("\n\n psm2pdf running ...\n\n");
   for (i = iStartArgc; i < argc; i++)
   {
      printf(" reading %s\n", argv[i]);
      PARSE_PEPXML(argv[i], fplist, pParams);
   }
   fclose(fplist);

   /*
    * Sort entries in szList by peptide or protein/peptide.
    * if needed.
    */
   if (pParams.iSortOption == 1)  /* sort by run & scan number */
   {
      sprintf(szBuf, "sort -k 3,3 -k4,4n %s > %s.tmp ; mv %s.tmp %s",
            pParams.szListURL, pParams.szListURL, pParams.szListURL, pParams.szListURL);
      system(szBuf);
   }
   else if (pParams.iSortOption == 2)  /* sort by protein, peptide, run, scan number */
   {
      sprintf(szBuf, "sort -k 1,1 -k2,2 -k3,3 -k4,4n %s > %s.tmp ; mv %s.tmp %s",
            pParams.szListURL, pParams.szListURL, pParams.szListURL, pParams.szListURL);
      system(szBuf);
   }
   else  /* iSortOption==3, sort by peptide, run, scan number */
   {
      sprintf(szBuf, "sort -k 2,2 -k3,3 -k4,4n %s > %s.tmp ; mv %s.tmp %s",
            pParams.szListURL, pParams.szListURL, pParams.szListURL, pParams.szListURL);
      system(szBuf);
   }

   /*
    * Next read through szList to generate PDF files by
    * calling htmldoc.
    */
   CREATE_PDF(pParams);

   unlink(pParams.szListURL);

   printf("\n created:  %s\n\n", pParams.szOutputFile);

   return(0);
} /*main*/


void PARSE_PEPXML(char *szInputFile,
      FILE *fplist,
      struct ParamsStruct pParams)
{
   char   szBuf[SIZE_BUF];
   int    bParseIonSeries;

   /*
    * If any ion series to plot is/are entered on command line
    * then ignore corresponding parameters read from pep.xml file.
    * Otherwise, get ion series from pep.xml or leave blank for defaults.
    */
   bParseIonSeries = pParams.cShowA + pParams.cShowA2 + pParams.cShowA3
                   + pParams.cShowB + pParams.cShowB2 + pParams.cShowB3
                   + pParams.cShowC + pParams.cShowC2 + pParams.cShowC3
                   + pParams.cShowX + pParams.cShowX2 + pParams.cShowX3
                   + pParams.cShowY + pParams.cShowY2 + pParams.cShowY3
                   + pParams.cShowZ + pParams.cShowZ2 + pParams.cShowZ3;

   if (!strncmp(szInputFile + strlen(szInputFile) - 4, ".xml", 4))
   {
      FILE  *fpin;

      if ((fpin = fopen(szInputFile, "r")) != NULL)
      {
         char   szBaseName[SIZE_BUF];         
	 char   szSpecName[SIZE_BUF];

         char  *pSpectrumQuery;
         int    iSizeSpectrumQuery;

         iSizeSpectrumQuery = SIZE_BUF;
         if ((pSpectrumQuery = (char *) malloc(iSizeSpectrumQuery)) == NULL)
         {
            printf(" Error - cannot allocate pSpectrumQuery\n");
            exit(1);
         }

         while (fgets(szBuf, SIZE_BUF, fpin))
         {
            /*
             * get base_name from msms_run_summary
             */
            if (strstr(szBuf, "<msms_run_summary"))
            {
               /* need to keep reading until end '>' is reached */
               strcpy(pSpectrumQuery, szBuf);
               while (!strchr(pSpectrumQuery, '>'))
               {
                  fgets(szBuf, SIZE_BUF, fpin);

                  if ((int)(strlen(szBuf) + strlen(pSpectrumQuery)) >= iSizeSpectrumQuery)
                     MYREALLOC(&iSizeSpectrumQuery, &pSpectrumQuery);

                  strcat(pSpectrumQuery, szBuf);
               }

               strcpy(szBaseName, GET_VAL(pSpectrumQuery, " base_name"));
            }

            /*
             * get mass types and ion series from search_summary
             */
            if (strstr(szBuf, "<search_summary"))
            {
               char   szTmp[SIZE_FILE];


               /* need to keep reading until end </search_summary> is reached */
               strcpy(pSpectrumQuery, szBuf);
               while (1)
               {
                  if (strstr(szBuf, "</search_summary>"))
                     break;

                  fgets(szBuf, SIZE_BUF, fpin);

                  if ((int)(strlen(szBuf) + strlen(pSpectrumQuery)) >= iSizeSpectrumQuery)
                     MYREALLOC(&iSizeSpectrumQuery, &pSpectrumQuery);

                  strcat(pSpectrumQuery, szBuf);
               }

               strcpy(szTmp, GET_VAL(pSpectrumQuery, " fragment_mass_type"));

               if (!strcmp("average", szTmp))
                  pParams.iMassTypeFragment = 0;     /* average */
               else
                  pParams.iMassTypeFragment = 1;     /* mono */

               if (!bParseIonSeries)
               {
                  /* need flexible parsing of ion series */
                  if (strstr(pSpectrumQuery, "\"ion_series\" value"))
                  {
                     /* sequest params ion series */
                     char   szIonSeries[SIZE_FILE];
                     float  fA = 0.0,
                            fB = 0.0,
                            fC = 0.0,
                            fX = 0.0,
                            fY = 0.0,
                            fZ = 0.0;

                     strcpy(szIonSeries, GET_VAL(pSpectrumQuery, "\"ion_series\" value"));
                     sscanf(szIonSeries, "%*d %*d %*d %f %f %f %*f %*f %*f %f %f %f",
                           &fA, &fB, &fC, &fX, &fY, &fZ);

                     if (fA > 0.0)
                        pParams.cShowA = 1;
                     if (fB > 0.0)
                        pParams.cShowB = 1;
                     if (fC > 0.0)
                        pParams.cShowC = 1;
                     if (fX > 0.0)
                        pParams.cShowX = 1;
                     if (fY > 0.0)
                        pParams.cShowY = 1;
                     if (fZ > 0.0)
                        pParams.cShowZ = 1;
                  }

                  /* xtandem ion series */
                  else if (strstr(pSpectrumQuery, "<parameter name=\"scoring, a ions\" value=\"yes\""))
                        pParams.cShowA = 1;
                  else if (strstr(pSpectrumQuery, "<parameter name=\"scoring, b ions\" value=\"yes\""))
                        pParams.cShowB = 1;
                  else if (strstr(pSpectrumQuery, "<parameter name=\"scoring, c ions\" value=\"yes\""))
                        pParams.cShowC = 1;
                  else if (strstr(pSpectrumQuery, "<parameter name=\"scoring, x ions\" value=\"yes\""))
                        pParams.cShowX = 1;
                  else if (strstr(pSpectrumQuery, "<parameter name=\"scoring, y ions\" value=\"yes\""))
                        pParams.cShowY = 1;
                  else if (strstr(pSpectrumQuery, "<parameter name=\"scoring, z ions\" value=\"yes\""))
                        pParams.cShowZ = 1;
                 
                  /* mascot ion series parsing ??? */
                  /* omssa ion series parsing ??? */
                  /* myrimatch ion series parsing ??? */

                  else
                  {
                     /* leave blank and plot-msms.cgi will default to b- and y-ions */
                  }
               }
            }

            /*
             * grab entire spectrum query tag
             */
            if (strstr(szBuf, "<spectrum_query spectrum"))
            {
               strcpy(pSpectrumQuery, szBuf);
	       strcpy(szSpecName, "");
	       strcpy(szSpecName, GET_VAL(pSpectrumQuery, " spectrum"));
               while (fgets(szBuf, SIZE_BUF, fpin))
               {
                  if (!strstr(szBuf, "alternative_protein"))
                  {
                     if ((int)(strlen(szBuf) + strlen(pSpectrumQuery)) >= iSizeSpectrumQuery)
                        MYREALLOC(&iSizeSpectrumQuery, &pSpectrumQuery);

                     strcat(pSpectrumQuery, szBuf);
                  }

                  if (strstr(szBuf, "</spectrum_query>"))
                  {
                     char   szTmp[SIZE_BUF];
                     double dScore = -1.0;
                     int    bPass = 0;

                     if (pParams.dExpectCutoff > 0.0)
                     {
                        strcpy(szTmp, GET_VAL(pSpectrumQuery, "\"expect\" value"));
                        sscanf(szTmp, "%lf", &dScore);

                        if (dScore <= pParams.dExpectCutoff)
                           bPass = 1;
                     }
                     else
                     {
                        strcpy(szTmp, GET_VAL(pSpectrumQuery, " probability"));
                        sscanf(szTmp, "%lf", &dScore);

                        if (dScore >= pParams.dProbCutoff)
                           bPass = 1;
                     }
		     std::string specStr = std::string(szSpecName);
		     if (pParams.specList != NULL && pParams.specList->find(specStr) != pParams.specList->end()) {
		       bPass = bPass && 1;
		     }
		     else if (pParams.specList != NULL) {
		       bPass = 0;
		     }

                     if (bPass)
                     {
                        char   szURL[SIZE_BUF];
                        char   szDta[SIZE_FILE];
                        char   szSpectrum[SIZE_FILE];
                        char   szPeptide[MAX_PEPTIDE_LEN];
                        char   szProtein[MAX_PEPTIDE_LEN];
                        char  *pStr;
                        double dPepMass;
                        int    iScanNum;
                        int    iCharge;


                        strcpy(szProtein, GET_VAL(pSpectrumQuery, " protein"));
                        strcpy(szPeptide, GET_VAL(pSpectrumQuery, " peptide"));

                        strcpy(szSpectrum, GET_VAL(pSpectrumQuery, " spectrum"));
                        sprintf(szDta, "%s/%s.dta", szBaseName, szSpectrum);

                        strcpy(szTmp, GET_VAL(pSpectrumQuery, " calc_neutral_pep_mass"));
                        sscanf(szTmp, "%lf", &dPepMass);

                        strcpy(szTmp, GET_VAL(pSpectrumQuery, " start_scan"));
                        sscanf(szTmp, "%d", &iScanNum);

                        strcpy(szTmp, GET_VAL(pSpectrumQuery, " assumed_charge"));
                        sscanf(szTmp, "%d", &iCharge);

                        /*
                         * now make szSpectrum just the base name
                         */
                        if ((pStr=strrchr(szSpectrum, '.'))!=NULL)
                        {
                           *pStr = '\0';
                           if ((pStr=strrchr(szSpectrum, '.'))!=NULL)
                           {
                              *pStr = '\0';
                              if ((pStr=strrchr(szSpectrum, '.'))!=NULL)
                              {
                                 *pStr = '\0';
                              }
                           }
                        }

                        /*
                         * write out header info for each spectrum
                         */
                        //strcpy(szPeptide, GET_VAL(pSpectrumQuery, " modified_peptide"));
                        //if (!strcmp(szPeptide, "-"))
                          
			strcpy(szPeptide, GET_VAL(pSpectrumQuery, " peptide"));

                        sprintf(szURL, "https://%s/%s/plot-msms.cgi?", pParams.szHost, pParams.szCgiBin);
                        sprintf(szURL + strlen(szURL), "MassType=%d", pParams.iMassTypeFragment);
                        sprintf(szURL + strlen(szURL), "&NumAxis=%d", pParams.iNumAxis);
                        sprintf(szURL + strlen(szURL), "&Pep=%s", szPeptide);
                        sprintf(szURL + strlen(szURL), "&PepMass=%0.4f", dPepMass + 1.00727646688);
                        sprintf(szURL + strlen(szURL), "&Prot=%s", szProtein);
                        sprintf(szURL + strlen(szURL), "&Dta=%s", szDta);

                        if (!bParseIonSeries)
                        {
                           if (pParams.cShowA==1 && iCharge>=3)
                              sprintf(szURL + strlen(szURL), "&ShowA2=1");
                           if (pParams.cShowB==1 && iCharge>=3)
                              sprintf(szURL + strlen(szURL), "&ShowB2=1");
                           if (pParams.cShowC==1 && iCharge>=3)
                              sprintf(szURL + strlen(szURL), "&ShowC2=1");
                           if (pParams.cShowX==1 && iCharge>=3)
                              sprintf(szURL + strlen(szURL), "&ShowX2=1");
                           if (pParams.cShowY==1 && iCharge>=2)
                              sprintf(szURL + strlen(szURL), "&ShowY2=1");
                           if (pParams.cShowY==1 && iCharge>=3)
                              sprintf(szURL + strlen(szURL), "&ShowY3=1");
                           if (pParams.cShowZ==1 && iCharge>=3)
                              sprintf(szURL + strlen(szURL), "&ShowZ2=1");
                        }

                        if (pParams.cShowA==1)
                           sprintf(szURL + strlen(szURL), "&ShowA=1");
                        if (pParams.cShowA2==1)
                           sprintf(szURL + strlen(szURL), "&ShowA2=1");
                        if (pParams.cShowA3==1)
                           sprintf(szURL + strlen(szURL), "&ShowA3=1");
                        if (pParams.cShowB==1)
                           sprintf(szURL + strlen(szURL), "&ShowB=1");
                        if (pParams.cShowB2==1)
                           sprintf(szURL + strlen(szURL), "&ShowB2=1");
                        if (pParams.cShowB3==1)
                           sprintf(szURL + strlen(szURL), "&ShowB3=1");
                        if (pParams.cShowC==1)
                           sprintf(szURL + strlen(szURL), "&ShowC=1");
                        if (pParams.cShowC2==1)
                           sprintf(szURL + strlen(szURL), "&ShowC2=1");
                        if (pParams.cShowC3==1)
                           sprintf(szURL + strlen(szURL), "&ShowC3=1");
                        if (pParams.cShowX==1)
                           sprintf(szURL + strlen(szURL), "&ShowX=1");
                        if (pParams.cShowX2==1)
                           sprintf(szURL + strlen(szURL), "&ShowX2=1");
                        if (pParams.cShowX3==1)
                           sprintf(szURL + strlen(szURL), "&ShowX3=1");
                        if (pParams.cShowY==1)
                           sprintf(szURL + strlen(szURL), "&ShowY=1");
                        if (pParams.cShowY2==1)
                           sprintf(szURL + strlen(szURL), "&ShowY2=1");
                        if (pParams.cShowY3==1)
                           sprintf(szURL + strlen(szURL), "&ShowY3=1");
                        if (pParams.cShowZ==1)
                           sprintf(szURL + strlen(szURL), "&ShowZ=1");
                        if (pParams.cShowZ2==1)
                           sprintf(szURL + strlen(szURL), "&ShowZ2=1");
                        if (pParams.cShowZ3==1)
                           sprintf(szURL + strlen(szURL), "&ShowZ3=1");

                        if (pParams.iLabelType != 0)
                           sprintf(szURL + strlen(szURL), "&LabelType=%d", pParams.iLabelType);
                        if (pParams.cShowH2OLoss == 0)
                           sprintf(szURL + strlen(szURL), "&ShowH2OLoss=0");
                        if (pParams.dIntensityZoom != DEFAULT_YZOOM)
                           sprintf(szURL + strlen(szURL), "&IntensityZoom=%0.2f", pParams.dIntensityZoom);
                        if (pParams.dMatchTol != DEFAULT_TOL)
                           sprintf(szURL + strlen(szURL), "&MatchTol=%0.2f", pParams.dMatchTol);
                        if (pParams.iXmin > 0)
                           sprintf(szURL + strlen(szURL), "&Xmin=%d", pParams.iXmin);
                        if (pParams.iXmax > 0)
                           sprintf(szURL + strlen(szURL), "&Xmax=%d", pParams.iXmax);

                        if (strstr(pSpectrumQuery, "mod_aminoacid_mass"))
                        {
                           int    ii;

                           memset(pParams.pdModPeptide, 0, sizeof(pParams.pdModPeptide));
                           memset(pParams.pdModNC, 0, sizeof(pParams.pdModNC));

                           for (ii = 0; ii < (int)(strlen(szPeptide)); ii++)
                              pParams.pcMod[ii] = '0';

                           while ((pStr = strstr(pSpectrumQuery, "mod_aminoacid_mass"))!=NULL)
                           {
                              int    iModPos = 0;
                              double dModMass = 0.0;

                              strcpy(szTmp, GET_VAL(pStr, " position"));
                              sscanf(szTmp, "%d", &iModPos);
                              strcpy(szTmp, GET_VAL(pStr, " mass"));
                              sscanf(szTmp, "%lf", &dModMass);

                              sprintf(szURL + strlen(szURL), "&Mod%d=%0.4f", iModPos, dModMass);

                              strcpy(pSpectrumQuery, pStr + 1);
                           }
                        }

                        sprintf(szURL + strlen(szURL), "&Score=%0.2f&PDF=1", dScore);

                        fprintf(fplist, "%s\t%s\t%s\t%d\t%s\n",
                              szProtein, szPeptide, szSpectrum, iScanNum, szURL);
                     }
                     break;
                  }
               }
            }
         }

         free(pSpectrumQuery);
         fclose(fpin);
      }
      else
      {
         printf(" Error - cannot open %s to read\n",  szInputFile);
         exit(1);
      }
   }
   else
   {
      printf(" Warning - file %s does not have an .xml extension, skipping.\n", szInputFile);
      exit(1);
   }
}


void CREATE_PDF(struct ParamsStruct pParams)
{
   FILE *fp;
   FILE *fptmp = NULL;
   char szTmpFile[SIZE_FILE];
   char szBuf[SIZE_BUF];
   char szURL[SIZE_BUF];
   char szCmd[SIZE_BUF];
   int iCt;
   int iFirst=1;

   sprintf(szTmpFile, "%s.tmp", pParams.szListURL);

   /*
    * Now open up szListURL, read each URL and batch calls to htmldoc
    * to create PDFs that are stitched together.
    */

   if ( (fp=fopen(pParams.szListURL, "r"))==NULL)
   {
      printf(" Error - cannot open %s to read.\n\n", pParams.szListURL);
      exit(1);
   }

   iCt=0;
   while (fgets(szBuf, SIZE_BUF, fp))
   {
      sscanf(szBuf, "%*s %*s %*s %*s %s\n", szURL);

      if (iCt<BATCH_SIZE)
      {
         if ( !fptmp && (fptmp=fopen(szTmpFile, "w"))==NULL)
         {
            printf(" Error - cannot open %s to write.\n\n", szTmpFile);
            exit(1);
         }
	 if (!strcmp(pParams.szUser, "-") || !strcmp(pParams.szPass, "-")) {
	   fprintf(fptmp, "wget -qO- --no-check-certificate \"%s\" | ", szURL);
	 }
	 else {
	   fprintf(fptmp, "wget -qO- --no-check-certificate --user=%s --password=%s \"%s\" | ", pParams.szUser, pParams.szPass, szURL);
	 }
         fprintf(fptmp, "htmldoc --webpage --fontsize 9 --bodycolor %s --size a4",
               BODYCOLOR);

         /*
          * suppress header and footer
          */
         fprintf(fptmp, " --header ... --footer ... ");
         fprintf(fptmp, " --landscape --browserwidth 1000 -f %s.tmp2.pdf -\n",
               pParams.szOutputFile);

	 
         /*
          * now append pdf of current set to final pdf using ghostscript
          */
         if (iFirst)
         {
            iFirst=0;
            fprintf(fptmp, "mv %s.tmp2.pdf %s\n", 
                  pParams.szOutputFile,
                  pParams.szOutputFile);
            //system(szCmd);
         }
         else
         {
	   fprintf(fptmp, "mv %s %s.tmp1.pdf\n",
                  pParams.szOutputFile, pParams.szOutputFile);
	   //system(szCmd);
            fprintf(fptmp, "gs -q -sPAPERSIZE=a4 -dNOPAUSE -dBATCH -sDEVICE=pdfwrite -sOutputFile=%s %s.tmp1.pdf %s.tmp2.pdf\n",
                  pParams.szOutputFile, pParams.szOutputFile, pParams.szOutputFile);
            //system(szCmd);

            fprintf(fptmp, "rm -f %s.tmp1.pdf\n", pParams.szOutputFile);
            //unlink(szCmd);
            fprintf(fptmp, "rm -f %s.tmp2.pdf\n", pParams.szOutputFile);
            //unlink(szCmd);
         }
      }

      iCt++;

      if (iCt==BATCH_SIZE)
      {
         iCt=0;
	 if (fptmp) {
	   fclose(fptmp);
	 }
         /*
          * make the pdf of current set using htmldoc
          */
         sprintf(szCmd, "chmod +x %s; ./%s", szTmpFile, szTmpFile);
         system(szCmd);

      

      }
      
   }

   /*
    * now run the last batch
    */
   if (iCt!=BATCH_SIZE)
   {
     if (fptmp)
       fclose(fptmp);

      /*
       * make the pdf of current set using htmldoc
       */
      sprintf(szCmd, "chmod +x %s; ./%s", szTmpFile, szTmpFile);
      system(szCmd);

//       /*
//        * now append pdf of current set to final pdf using ghostscript
//        */
//       if (iFirst)
//       {
//          iFirst=0;
//          sprintf(szCmd, "mv %s.tmp2.pdf %s", 
//                pParams.szOutputFile,
//                pParams.szOutputFile);
//          system(szCmd);
//       }
//       else
//       {
//          sprintf(szCmd, "mv %s %s.tmp1.pdf",
//                pParams.szOutputFile, pParams.szOutputFile);
//          system(szCmd);
//          sprintf(szCmd, "gs -q -sPAPERSIZE=a4 -dNOPAUSE -dBATCH -sDEVICE=pdfwrite -sOutputFile=%s %s.tmp1.pdf %s.tmp2.pdf",
//                pParams.szOutputFile, pParams.szOutputFile, pParams.szOutputFile);
//          system(szCmd);

//          sprintf(szCmd, "%s.tmp1.pdf", pParams.szOutputFile);
//          unlink(szCmd);
//          sprintf(szCmd, "%s.tmp2.pdf", pParams.szOutputFile);
//          unlink(szCmd);
//       }
   }

   fclose(fp);

   unlink(szTmpFile);

} /*CREATE_PDF*/


char  *GET_VAL(char *pSpectrumQuery,
   char *szAttribute)
{
   char  *pStr;

   if ((pStr = strstr(pSpectrumQuery, szAttribute)) != NULL)
   {
      strncpy(szAttributeVal, pStr + strlen(szAttribute) + 2, SIZE_FILE);       /* +2 to skip =" */

      if ((pStr = strchr(szAttributeVal, '"')) != NULL)
      {
         *pStr = '\0';
         return (szAttributeVal);
      }
      else
      {
         strcpy(szAttributeVal, "-");
         return (szAttributeVal);
      }
   }
   else
   {
      strcpy(szAttributeVal, "-");
      return (szAttributeVal);
   }
}


void MYREALLOC(int *iSizeSpectrumQuery,
   char **pSpectrumQuery)
{
   char  *pTmp;

   *iSizeSpectrumQuery += SIZE_BUF;
   pTmp = (char *) realloc(*pSpectrumQuery, *iSizeSpectrumQuery);
   if (pTmp == NULL)
   {
      printf(" Error realloc pSpectrumQuery (%d)\n\n", *iSizeSpectrumQuery);
      exit(1);
   }

   *pSpectrumQuery = pTmp;
}


void SET_OPTION(char *arg,
   struct ParamsStruct *pParams)
{
   char szTmp[50];
   char *pStr;
   double dTmp;
   int iNum1;
   int iNum2;
   FILE* fpin;
   char spec_name[SIZE_BUF];
   switch (arg[1])
   {
      case '2':
         pParams->iNumAxis = 2;
         break;
      case 'P':
         sscanf(arg + 2, "%lf", &(pParams->dProbCutoff));
         break;
      case 'E':
         sscanf(arg + 2, "%lf", &(pParams->dExpectCutoff));
         break;
      case 'F':
         strcpy(pParams->szOutputFile, arg + 2);
         break;

      case 'l':

         strcpy(pParams->szListFile, arg + 2);

	 if (!pParams->specList)
	   pParams->specList = new bool_hash();
	 if ((fpin = fopen(pParams->szListFile, "r")) != NULL) {
	   while (fgets(spec_name, SIZE_BUF, fpin))
	     { 
	       spec_name[strlen(spec_name)-1] = '\0';
	       pParams->specList->insert(make_pair(* (new std::string(spec_name)), true));
	       
	     }

	 }



         break;
      case 'H':
         strcpy(pParams->szHost, arg + 2);
         break;
      case 'C':
         strcpy(pParams->szCgiBin, arg + 2);
         break;
      case 'U':
	 strcpy(pParams->szUser, arg + 2);
         break;
      case 'W':
         strcpy(pParams->szPass, arg + 2);
         break;
      case 'O':
         strcpy(pParams->szOutputFile, arg + 2);
         if (strcmp(pParams->szOutputFile + strlen(pParams->szOutputFile) - 4, ".PDF")
               && strcmp(pParams->szOutputFile + strlen(pParams->szOutputFile) - 4, ".pdf"))
         {
            strcat(pParams->szOutputFile, ".pdf");
         }
         break;
      case 'S':
         strcpy(szTmp, arg+2);
         if (!strcmp(szTmp, "file"))
            pParams->iSortOption = 1;
         else if (!strcmp(szTmp, "protein"))
            pParams->iSortOption = 2;
         else if (!strcmp(szTmp, "peptide"))
            pParams->iSortOption = 3;
         else
            printf(" unknown sort option: %s\n", szTmp);
         break;
      case 'L':
         if (!strcmp(arg+2, "ions"))
            pParams->iLabelType = 0;
         else if (!strcmp(arg+2, "masses"))
            pParams->iLabelType = 1;
         else if (!strcmp(arg+2, "none"))
            pParams->iLabelType = 2;
         else
            printf(" unknown label option: %s\n", arg+2);
         break;
      case 'h':
         pParams->cShowH2OLoss = 0;
         break;
      case 'Y':
         sscanf(arg + 2, "%lf", &dTmp);
         if (dTmp>=0.0 && dTmp<=500.0)
            pParams->dIntensityZoom = dTmp;
         break;
      case 'T':
         sscanf(arg + 2, "%lf", &dTmp);
         if (dTmp>=0.0 && dTmp<=100.0)
            pParams->dMatchTol = dTmp;
         break;
      case 'X':
         strcpy(szTmp, arg+2);
         if ((pStr = strchr(szTmp, '-'))==NULL)
            printf(" Error with -X option (%s), ignoring\n", szTmp);
         *pStr = ' ';
         iNum1=0;
         iNum2=0;
         sscanf(szTmp, "%d %d", &iNum1, &iNum2);

         if (iNum1>=0 && iNum2>=iNum1)
         {
            pParams->iXmin = iNum1;
            pParams->iXmax = iNum2;
         }
         break;
      case 'a':
         iNum1=0;
         sscanf(arg + 2, "%d", &iNum1);
         if (iNum1>=1 && iNum1<=3)
         {
            pParams->cShowA = 1;
            if (iNum1>=2)
               pParams->cShowA2 = 1;
            if (iNum1>=3)
               pParams->cShowA3 = 1;
         }
         break;
      case 'b':
         iNum1=0;
         sscanf(arg + 2, "%d", &iNum1);
         if (iNum1>=1 && iNum1<=3)
         {
            pParams->cShowB = 1;
            if (iNum1>=2)
               pParams->cShowB2 = 1;
            if (iNum1>=3)
               pParams->cShowB3 = 1;
         }
         break;
      case 'c':
         iNum1=0;
         sscanf(arg + 2, "%d", &iNum1);
         if (iNum1>=1 && iNum1<=3)
         {
            pParams->cShowC = 1;
            if (iNum1>=2)
               pParams->cShowC2 = 1;
            if (iNum1>=3)
               pParams->cShowC3 = 1;
         }
         break;
      case 'x':
         iNum1=0;
         sscanf(arg + 2, "%d", &iNum1);
         if (iNum1>=1 && iNum1<=3)
         {
            pParams->cShowX = 1;
            if (iNum1>=2)
               pParams->cShowX2 = 1;
            if (iNum1>=3)
               pParams->cShowX3 = 1;
         }
         break;
      case 'y':
         iNum1=0;
         sscanf(arg + 2, "%d", &iNum1);
         if (iNum1>=1 && iNum1<=3)
         {
            pParams->cShowY = 1;
            if (iNum1>=2)
               pParams->cShowY2 = 1;
            if (iNum1>=3)
               pParams->cShowY3 = 1;
         }
         break;
      case 'z':
         iNum1=0;
         sscanf(arg + 2, "%d", &iNum1);
         if (iNum1>=1 && iNum1<=3)
         {
            pParams->cShowZ = 1;
            if (iNum1>=2)
               pParams->cShowZ2 = 1;
            if (iNum1>=3)
               pParams->cShowZ3 = 1;
         }
         break;
   }
}


void GLOBAL_INIT(struct ParamsStruct *pParams)
{
   pParams->iLabelType = 0;     /* 0=ion labels, 1=fragment masses, 2=no labels, 3=force fragment masses */
   pParams->iMassTypeFragment = 1;      /*default mono */
   pParams->iNumAxis = 1;
   pParams->iXmin = 0;
   pParams->iXmax = 0;
   pParams->iSortOption = 1;    /* 1 = file/scan, 2=protein/peptide, 3=peptide */

   pParams->cShowA = 0;
   pParams->cShowA2 = 0;
   pParams->cShowA3 = 0;
   pParams->cShowB = 0;
   pParams->cShowB2 = 0;
   pParams->cShowB3 = 0;
   pParams->cShowC = 0;
   pParams->cShowC2 = 0;
   pParams->cShowC3 = 0;
   pParams->cShowX = 0;
   pParams->cShowX2 = 0;
   pParams->cShowX3 = 0;
   pParams->cShowY = 0;
   pParams->cShowY2 = 0;
   pParams->cShowY3 = 0;
   pParams->cShowZ = 0;
   pParams->cShowZ2 = 0;
   pParams->cShowZ3 = 0;
   pParams->cShowH2OLoss = 1;   /* mark -17 NH3 and -18 H20 loss by tracking just -17.5 */

   pParams->dMatchTol = DEFAULT_TOL;
   pParams->dIntensityZoom = DEFAULT_YZOOM;
   pParams->dProbCutoff = PROBCUTOFF;
   pParams->dExpectCutoff = -1.0;
   strcpy(pParams->szOutputFile, DEFAULT_OUTPUT);
   strcpy(pParams->szHost, "localhost");
   strcpy(pParams->szCgiBin, "tpp/cgi-bin");
   strcpy(pParams->szUser, "-");
   strcpy(pParams->szPass, "-");

   strcpy(pParams->szListFile, "");

   pParams->specList = NULL;
}


void USAGE(char *szArg)
{
   printf(" This program takes one more more pepXML files as input\n");
   printf(" and dumps a URL for each spectrum link to a file with.\n");
   printf(" same name as inputs but with .html extension.\n");
   printf("\n");
   printf(" USAGE:  %s [options] *.pep.xml\n", szArg);
   printf("\n");
   printf(" options\n");
   printf("         -P<num>    set probability cutoff (default %0.2f)\n", PROBCUTOFF);
   printf("         -E<num>    set and use E-value cutoff (in place of probability)\n");
   printf("         -2         plot spectra across 2 axis\n");
   printf("         -l<string> specify list of spectra file to report\n");
   printf("         -H<string> webserver's host name (default localhost)\n");
   printf("         -C<string> webserver's cgi-bin (default tpp/cgi-bin)\n");
   printf("         -U<username>    set username for webserver\n");
   printf("         -W<password>    set password for webserver\n");
   printf("         -O<string> output PDF file name (default %s)\n", DEFAULT_OUTPUT);
   printf("         -S<string> sort resulting pages by\n");
   printf("                       'file' = base file then scan number (default e.g. -Sfile)\n");
   printf("                       'protein' = protein then peptide\n");
   printf("                       'peptide' = peptide\n");
   printf("         -L<string> set spectrum peak labeling\n");
   printf("                       'ions' = ions (default e.g. -Lions)\n");
   printf("                       'masses' = masses\n");
   printf("                       'none' = no labels\n");
   printf("         -h        hides neutral loss labels\n");
   printf("         -Y<num>   set Y-axis zoom/scaling factor (default %0.1f)\n", DEFAULT_YZOOM);
   printf("         -T<num>   sets ion matching tolerance (default %0.2f)\n", DEFAULT_TOL);
   printf("         -X<num>-<num>   set x-axis mass range (e.g. -X500-1600)\n");
   printf("         -a<num>   display a-ions where <num> is max charge state (up to z=3)\n");
   printf("         -b<num>   display b-ions where <num> is max charge state (up to z=3)\n");
   printf("         -c<num>   display c-ions where <num> is max charge state (up to z=3)\n");
   printf("         -x<num>   display x-ions where <num> is max charge state (up to z=3)\n");
   printf("         -y<num>   display y-ions where <num> is max charge state (up to z=3)\n");
   printf("         -z<num>   display z-ions where <num> is max charge state (up to z=3)\n");
   printf("\n");
}
