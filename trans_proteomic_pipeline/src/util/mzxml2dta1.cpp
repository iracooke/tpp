/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library or "Lesser" General Public      *
 *   License (LGPL) as published by the Free Software Foundation;          *
 *   either version 2 of the License, or (at your option) any later        *
 *   version.                                                              *
 *                                                                         *
 ***************************************************************************/

/****
 
       Date:  01/22/2004
 
    Purpose:  Use RAMP to create .dta files from input mzXML file.
 
  Copyright:  Jimmy Eng, Copyright (c) Institute for Systems Biology, 2004.  All rights reserved.
 
   Requires:  base64.c base64.h ramp.c ramp.h from http://sourceforge.net/projects/sashimi
 
****/ 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "ramp_base64.h"
#include "mzParser.h"

#define SIZE_FILE   512
#define SIZE_BUF    8192
#define MINMASS     600.0
#define MAXMASS     4200.0
#define MINPEAKS    5

static double dHydrogenMass = 1.007825;
static double dProtonMass = 1.007276;

struct OptionsStruct
{
   int    iFirstScan;
   int    iLastScan;
   int    iMinPeakCount;
   int    iCharge;
   int    iMSLevel;
   double dMinMass;
   double dMaxMass;
   double dMinPeakThreshHold;
   double dMinPeakDtaFilter;
   char*  outputDir;
   int    useProtonMass;
};

void USAGE(char *arg);
void SET_OPTION(char *arg,
        struct OptionsStruct *pOptions);
void ANALYZE(char *szXMLFile, struct OptionsStruct pOptions);


int main(int argc, char **argv)
{
   int i;
   int  iNumArg;
   int iStartArgc; 
   char *arg;
   struct OptionsStruct pOptions;

   printf("\n");
   printf(" mzXML2dta\n\n");

   iNumArg = 1;
   iStartArgc = 1;
   pOptions.iFirstScan = 0;
   pOptions.iLastScan = 99999;
   pOptions.iCharge = 0;
   pOptions.iMSLevel = 2;
   pOptions.iMinPeakCount = MINPEAKS;
   pOptions.dMinMass = MINMASS;
   pOptions.dMaxMass = MAXMASS;
   pOptions.dMinPeakThreshHold = 1;
   pOptions.dMinPeakDtaFilter = 0;
   pOptions.outputDir = NULL;
   pOptions.useProtonMass = 0;

   arg = argv[iNumArg];
   while (iNumArg < argc)
   {
      if (arg[0] == '-')
         SET_OPTION(arg, &pOptions);
      else
         break;

      iStartArgc++;
      arg = argv[++iNumArg];
   }

   if (iStartArgc>=argc)
      USAGE(argv[0]);

   for (i=iStartArgc; i<argc; i++)
   {
      ANALYZE(argv[i], pOptions);
   }

   return 0;
} /*main*/


void ANALYZE(char *szXMLFile, struct OptionsStruct pOptions)
{
   // unused:
   // int  i;
   int  iAnalysisFirstScan,
        iAnalysisLastScan;
   long ctScan;
   char szBaseName[SIZE_FILE];
   char szCommand[SIZE_FILE];
   RAMPFILE *pFI;
   ramp_fileoffset_t indexOffset;
   ramp_fileoffset_t *pScanIndex;
   double dChargeMass;


   /*
    * check that ends in .mzXML extension
    */
   if (strcmp(szXMLFile+strlen(szXMLFile)-6, ".mzXML") )
   {
      printf(" Please enter an input file that ends in .mzXML\n\n");
      exit(EXIT_FAILURE);
   }

   if ( (pFI = rampOpenFile(szXMLFile)) == NULL)
   {
      printf("Could not open input file %s\n", szXMLFile);
      exit(EXIT_FAILURE);
   }

   printf(" Reading %s\n", szXMLFile);

   /*
    * Read the offset of the index
    */
   printf(" Getting the index offset\n");
   indexOffset = getIndexOffset( pFI );
   if (!indexOffset)
   {
      printf(" Could not find index offset.\n");
      printf(" File may be corrupted.\n");
      exit(EXIT_FAILURE);
   }
         
   /*
    * Read the scan index into a vector, get LastScan
    */
   printf(" Reading the index\n");
   pScanIndex = readIndex( pFI , indexOffset, &iAnalysisLastScan );
   iAnalysisFirstScan = 1;

   /*
    * If no output directory, use a directory located beside
    * the .mzXML file, with files base-name.
    */
   strcpy(szBaseName, szXMLFile);
   szBaseName[strlen(szBaseName)-6]='\0';

   if (pOptions.outputDir == NULL)
   {
      sprintf(szCommand, "mkdir %s", szBaseName);
      if (system(szCommand) == -1)
      {
         printf(" Error - can't create directory %s\n\n", szBaseName);
         exit(EXIT_FAILURE);
      }
      pOptions.outputDir = szBaseName;
   }

   /*
    * Use the right value for the mass of +1 charge.
    */
   if (pOptions.useProtonMass)
      dChargeMass = dProtonMass;
   else
      dChargeMass = dHydrogenMass;

   printf(" scan: ");

   if (iAnalysisFirstScan < pOptions.iFirstScan)
      iAnalysisFirstScan = pOptions.iFirstScan;
   if (iAnalysisLastScan > pOptions.iLastScan)
      iAnalysisLastScan = pOptions.iLastScan;

   for (ctScan=iAnalysisFirstScan; ctScan<=iAnalysisLastScan; ctScan++)
   {
      int   n;
      RAMPREAL *pPeaks;
      struct ScanHeaderStruct scanHeader;

      // unused:
      //char szDta[SIZE_FILE];


      /*
       * read scan header
       */
      readHeader(pFI, pScanIndex[ctScan], &scanHeader);

      printf("%5ld  %3d%%", ctScan, (int)(100.0*ctScan/iAnalysisLastScan)); fflush(stdout);
      printf("\b\b\b\b\b\b\b\b\b\b\b");

      /*
      printf("\n");
      printf("------- Scan %d header information -------\n", ctScan);
      printf("        msLevel: %d\n",   scanHeader.msLevel);
      if (scanHeader.msLevel>1)
      {
         printf("    precursorMZ: %0.2lf\n",   scanHeader.precursorMZ);
         if (scanHeader.percursorCharge > 0)
            printf("precursorCharge: %d\n", scanHeader.precursorCharge);
      }
      printf("       numPeaks: %d\n",       scanHeader.peaksCount);
      printf("       scanTime: %0.2lf\n",   scanHeader.retentionTime);
      printf("          lowMZ: %0.2lf\n",   scanHeader.lowMZ);
      printf("         highMZ: %0.2lf\n",   scanHeader.highMZ);
      printf("\n");
      */

      if (scanHeader.msLevel == pOptions.iMSLevel
            && scanHeader.peaksCount>pOptions.iMinPeakCount)
      {
         char szOut[SIZE_FILE];
         char szBase2[SIZE_FILE];
         char *pStr;
         FILE *fpOut;
         int  iCharge=0;
         float fSumBelow;
         float fSumTotal;

         /*
          * read scan peaks
          */
         pPeaks = readPeaks( pFI, pScanIndex[ctScan]);

         if (pOptions.iCharge != 0)
            iCharge = pOptions.iCharge;
         else if (scanHeader.precursorCharge != 0)
            iCharge = scanHeader.precursorCharge;
         else
         {
            /*
             * simple charge state determination
             */
            n=0;
            fSumBelow=0.0;
            fSumTotal=0.0;
            while( pPeaks[n] != -1 )
            {
               float fMass;
               float fInten;
      
               fMass=pPeaks[n];
               n++;
               fInten=pPeaks[n];
               n++;

               fSumTotal += fInten;
               if (fMass<scanHeader.precursorMZ)
                  fSumBelow += fInten;
            }
 
            /*
             * If greater than 95% signal is below precursor, then
             * it looks like singly charged peptide.
             */
            if (fSumTotal == 0.0 || fSumBelow/fSumTotal>0.95)
               iCharge=1;
         }

         if ((pStr=strrchr(szBaseName, '/')))
            strcpy(szBase2, pStr+1);
         else
            strcpy(szBase2, szBaseName);

         if (iCharge!=0)
         {
            double dPepMass;

            dPepMass = (scanHeader.precursorMZ * iCharge) - (iCharge - 1)*dChargeMass;

            if (pOptions.dMinMass<=dPepMass && pOptions.dMaxMass>=dPepMass)
            {
               sprintf(szOut, "%s/%s.%.5ld.%.5ld.%d.dta", pOptions.outputDir,
                  szBase2, ctScan, ctScan, iCharge);
      
               if ( (fpOut=fopen(szOut, "w"))!=NULL)
               {
                  fprintf(fpOut, "%0.4lf %d\n", dPepMass, iCharge);
   
                  n=0;
                  while( pPeaks[n] != -1 )
                  {
                     float fMass;
                     float fInten;
   
                     fMass=pPeaks[n];
                     n++;
                     fInten=pPeaks[n];
                     n++;

   
                     if (fInten > 0.0)
                        fprintf(fpOut, "%0.4f %0.1f\n", fMass, fInten);
                  }
                  fclose(fpOut);
               }
               else
                  printf(" Cannot open file %s\n", szOut);
            }
         }
         else
         {
            for (iCharge=2; iCharge<=3; iCharge++)
            {
               double dPepMass;

               dPepMass = (scanHeader.precursorMZ * iCharge) - (iCharge - 1)*dChargeMass;
   
               if (pOptions.dMinMass<=dPepMass && pOptions.dMaxMass>=dPepMass)
               {
                  sprintf(szOut, "%s/%s.%.5ld.%.5ld.%d.dta", pOptions.outputDir,
                     szBase2, ctScan, ctScan, iCharge);
         
                  if ( (fpOut=fopen(szOut, "w"))!=NULL)
                  {
                     fprintf(fpOut, "%0.4lf %d\n", dPepMass, iCharge);
      
                     n=0;
                     while( pPeaks[n] != -1 )
                     {
                        float fMass;
                        float fInten;
      
                        fMass=pPeaks[n];
                        n++;
                        fInten=pPeaks[n];
                        n++;
      
                        if (fInten > 0.0)
                           fprintf(fpOut, "%0.4f %0.1f\n", fMass, fInten);
                     }
                     fclose(fpOut);
                  }
                  else
                     printf(" Cannot open file %s\n", szOut);
               }
            }
         }
         free(pPeaks);
      }
   }

   rampCloseFile(pFI);

   printf("\n");
   printf(" Done.\n\n");

} /*ANALYZE*/


void USAGE(char *arg)
{
   printf(" Usage:  %s [options] *.mzXML\n", arg);
   printf("\n");
   printf("     options = -F<num>   where num is an int specifying the first scan\n");
   printf("               -L<num>   where num is an int specifying the last scan\n");
   printf("               -C<num>   where num is an int specifying the precursor charge state to analyze\n");
   printf("               -B<num>   where num is a float specifying minimum MH+ mass, default=%0.1f Da\n", MINMASS);
   printf("               -T<num>   where num is a float specifying maximum MH+ scan, default=%0.1f Da\n", MAXMASS);
   printf("               -P<num>   where num is an int specifying minimum peak count, default=%d\n", MINPEAKS);
   printf("               -M<num>   where num is an int specifying MS level, default=2\n");
/*
   printf("               -I<num>   where num is an int specifying minimum peak threshhold for peak count, default=1\n");
   printf("               -i<num>   where num is an int specifying minimum peak intensity filter, default=0\n");
*/
   printf("               -p        use proton mass for charge ion (default is hydrogen mass)\n");
   printf("\n");
/*
   printf(" Minimum peak count is defined as # of non-zero integer normalized signals\n");
   printf("\n");
*/
   exit(EXIT_FAILURE);
} /*USAGE*/


void SET_OPTION(char *arg,
        struct OptionsStruct *pOptions)
{
   int  iTmp = 0;
   double dTmp = 0.0;

   switch (arg[1])
   {
      case 'F':
         if (sscanf(&arg[2], "%d", &iTmp) != 1 || iTmp < 1)
            printf("Bad first scan #: '%s' ... ignored\n", &arg[2]);
         else
            pOptions->iFirstScan = iTmp;
         break;
      case 'L':
         if (sscanf(&arg[2], "%d", &iTmp) != 1 || iTmp < 1)
            printf("Bad last scan #: '%s' ... ignored\n", &arg[2]);
         else
            pOptions->iLastScan = iTmp;
         break;
      case 'B':
         if (sscanf(&arg[2], "%lf", &dTmp) != 1 || dTmp < 0.00)
            printf("Bad minimum mass: '%s' ... ignored\n", &arg[2]);
         else
            pOptions->dMinMass = dTmp;
         break;
      case 'T':
         if (sscanf(&arg[2], "%lf", &dTmp) != 1 || dTmp < 0.000)
            printf("Bad maximum mass: '%s' ... ignored\n", &arg[2]);
         else
            pOptions->dMaxMass = dTmp;
         break;
      case 'P':
         if (sscanf(&arg[2], "%d", &iTmp) != 1 || iTmp < 0)
            printf("Bad minimum peak count: '%s' ... ignored\n", &arg[2]);
         else
            pOptions->iMinPeakCount = iTmp;
         break;
      case 'M':
         if (sscanf(&arg[2], "%d", &iTmp) != 1 || iTmp < 2)
            printf("Bad MS level: '%s' ... ignored\n", &arg[2]);
         else
            pOptions->iMSLevel = iTmp;
         break;
      case 'I':
         if (sscanf(&arg[2], "%d", &iTmp) != 1 || iTmp < 0)
            printf("Bad minimum peak threshhold: '%s' ... ignored\n", &arg[2]);
         else
            pOptions->dMinPeakThreshHold = iTmp;
         break;
      case 'C':
         if (sscanf(&arg[2], "%d", &iTmp) != 1 || iTmp < 0)
            printf("Bad charge: '%s' ... ignored\n", &arg[2]);
         else
            pOptions->iCharge = iTmp;
         break;
      case 'i':
         if (sscanf(&arg[2], "%d", &iTmp) != 1 || iTmp < 0)
            printf("Bad minimum peak intensity for filtering: '%s' ... ignored\n", &arg[2]);
         else
            pOptions->dMinPeakDtaFilter = iTmp;
         break;
      case 'O':
         pOptions->outputDir = &arg[2];
         break;
      case 'p':
         pOptions->useProtonMass = 1;
         break;
      default:
         USAGE(arg);
         break;
   }
   arg[0] = '\0';
} /*SET_OPTIONS*/
