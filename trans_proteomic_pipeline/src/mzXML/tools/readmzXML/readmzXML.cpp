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
 
    Purpose:  Simply reads scans from an mzXML or mzData file.
 
  Copyright:  Jimmy Eng, Copyright (c) Institute for Systems Biology, 2004.  All rights reserved.
 
   Requires:  ramp_base64.cpp base64.h ramp.cpp ramp.h from http://sourceforge.net/projects/sashimi/trans_proteomic_pipeline/src/Parsers/ramp
 
****/ 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "common/sysdepend.h"
#include "mzParser.h"

#define SIZE_FILE        512

void READ_FILE(char *szXMLFile,
      int iPrintHeaderFile,
      int iPrintPeaksOption,
      int bInteractive,
      int scanArgNum,
      int argc,
      char **argv);
void SET_OPTION(char *arg,
      int *iPrintHeaderFile,
      int *iPrintPeaksOption);
void USAGE(char *argv0);


int main(int argc, char **argv)
{
  hooks_tpp handler(argc,argv); // set up install paths etc
   int iPrintHeaderFile;
   int iPrintPeaksOption;   /* 0 = default print all peaks, 1 = print brief peaks, 2 = print no peaks */
   int bInteractive;
   int iNumArg;
   int iStartArgc;
   char *arg;
   time_t then,now;

   iPrintHeaderFile = 0;
   iPrintPeaksOption= 0;
   iNumArg = 1;
   iStartArgc = 1;
   arg = argv[iNumArg];

   while (iNumArg < argc)
   {
      if (arg[0] == '-')
         SET_OPTION(arg, &iPrintHeaderFile, &iPrintPeaksOption);
      else
         break;

      iStartArgc++;
      arg = argv[++iNumArg];
   }

   if (iPrintPeaksOption != 5) {
      printf("\n");
      printf(" mzXML/mzData/mzML Scan Decoder\n");
      printf(" Copyright (c) Institute for Systems Biology, 2004,2005,2008.  All rights reserved.\n\n");
   }

   /*
    * no input files specified
    */
   if (argc-iStartArgc == 0)
   {
      USAGE(argv[0]);
   }

   bInteractive = (iPrintPeaksOption < 3) && ((argc-iStartArgc)<2); // looking at a list, or waiting for user input?
   time(&then);

   READ_FILE(argv[iStartArgc], iPrintHeaderFile, iPrintPeaksOption, bInteractive, iStartArgc+1, argc, argv);

   time(&now);
   if (iPrintPeaksOption != 5) {
     printf("\n");
     printf("(active %d seconds) Goodbye.\n\n",(int)(now-then));
   }

   return(0);

} /*main*/


void READ_FILE(char *szXMLFile,
      int iPrintHeaderFile,
      int iPrintPeaksOption,
      int bInteractive,
      int scanArgNum, // for use with command line list of scan numbers
      int argc,
      char **argv)
{
   int  iAnalysisFirstScan,
        iAnalysisLastScan;

   RAMPFILE  *pFI;

   int performanceTest = (iPrintPeaksOption==3);

   ramp_fileoffset_t  indexOffset;
   ramp_fileoffset_t  *pScanIndex;

   if (iPrintPeaksOption < 4)
      printf(" Reading %s\n", szXMLFile);

   if ( (pFI = rampOpenFile( szXMLFile )) == NULL)
   {
      printf( "could not open input file %s\n", szXMLFile);
      exit(1);
   }
   /*
    * Read the offset of the index 
    */
   if (iPrintPeaksOption < 4)
      printf( " Getting the Index Offset\n");
   indexOffset = getIndexOffset( pFI );
   
   /*
    * Read the scan index into a vector, get LastScan
    */
   if (iPrintPeaksOption < 4)
      printf( " Reading the index\n");
   pScanIndex = readIndex( pFI , indexOffset, &iAnalysisLastScan );
   iAnalysisFirstScan = 1;

   if (iPrintHeaderFile != 0)
   {
      int ctScan;
      struct ScanHeaderStruct scanHeader;
      char szOutFile[512];
      FILE *fpout;

      strcpy(szOutFile, szXMLFile);
      char *dotext = rampValidFileType(szOutFile);
      if (dotext) 
         *(dotext+1) = 0; // generalize to all RAMP formats
      else if (pFI->bIsMzData)
         szOutFile[strlen(szOutFile)-6]='\0';
      else
         szOutFile[strlen(szOutFile)-5]='\0';
      strcat(szOutFile, "txt");
   
      if ( (fpout=fopen(szOutFile, "w"))==NULL)
      {
         printf(" Error - cannot open %s to write.\n\n", szOutFile);
         exit(0);
      }
   
      if (iPrintHeaderFile==3) /* print simple diagnostics to stdout */
      {
         int iCountMS1 = 0;
         int iCountMS2 = 0;
         int iCountTot = iAnalysisLastScan - iAnalysisFirstScan + 1;

         for (ctScan=iAnalysisFirstScan; ctScan<=iAnalysisLastScan; ctScan++)
         {
            readHeader(pFI, pScanIndex[ctScan], &scanHeader);

            if (scanHeader.msLevel==1)
               iCountMS1++;
            else if (scanHeader.msLevel==2)
               iCountMS2++;
         }
         fprintf(fpout, "#MS1\t#MS2\t#tot\tTime(min)\n");
         fprintf(fpout, "%d\t%d\t%d\t%d\n", iCountMS1, iCountMS2, iCountTot, (int)(scanHeader.retentionTime/60.0 + 0.5));
      }
      else
      {
         /*Print out header */
         if (iPrintHeaderFile == 1)
            fprintf(fpout, "#Scan\tmsLevel\tretentionTime (sec)\tBasePeakIntensity\ttotIonCurrent\tprecursorMz\t# peaks\tlowMz\thighMz\n");
         else if (iPrintHeaderFile == 2)
            fprintf(fpout, "#Scan\tmsLevel\tretentionTime (sec)\tprecursorMz\tBasePeakMass\tFragmentMass1\tFragmentMass2\tprecursorCharge\n");
         else if (iPrintHeaderFile == 4)
            fprintf(fpout, "#Scan\tmsLevel\tretentionTime (sec)\tprecursorMz\tactivationMethod\t-98\t-49\t-32.7\n");
   
         /*Print text dump */
         for (ctScan=iAnalysisFirstScan; ctScan<=iAnalysisLastScan; ctScan++)
         {
            readHeader(pFI, pScanIndex[ctScan], &scanHeader);
      
            if (iPrintHeaderFile == 1)
            {
               fprintf(fpout, "%d\t%d\t%0.4f\t%0.2lf\t%0.2lf\t%0.6lf\t%d",
                     ctScan,
                     scanHeader.msLevel,
                     scanHeader.retentionTime,
                     scanHeader.basePeakIntensity,
                     scanHeader.totIonCurrent,
                     (scanHeader.msLevel?scanHeader.precursorMZ:0.0),
                     scanHeader.peaksCount);
               if (scanHeader.lowMZ>scanHeader.highMZ) {
                  fprintf(fpout,"\tnotGiven\tnotGiven\n");
               } else  {
                  fprintf(fpout,"\t%0.6lf\t%0.6lf\n",
                     scanHeader.lowMZ,
                     scanHeader.highMZ);
               } 
            }
            else if (iPrintHeaderFile==2 || iPrintHeaderFile==4 || iPrintHeaderFile==5)
            {
               if (iPrintHeaderFile==5)   /* scan/mass/inten for MS1 scans for 3d plotting */
               {
                  if (scanHeader.msLevel==1 && scanHeader.peaksCount>0)
                  {
                     RAMPREAL *pPeaks;
                     int n=0;
               
                     /*
                      * read scan peaks
                      */
                     pPeaks = readPeaks( pFI, pScanIndex[ctScan]);
                     for (int iloop=scanHeader.peaksCount; iloop-- >0; )
                     {
                        double dMass=pPeaks[n++];
                        double dInten=pPeaks[n++];
                              
                        fprintf(fpout, "%d\t%0.2f\t%0.0f\n", ctScan, dMass, dInten);
                     }
                  }
               }
               else if (scanHeader.msLevel==2)
               {
                  scanHeader.retentionTime /= 60.0;
      
                  fprintf(fpout, "%d\t%d\t%0.6lf\t%0.6lf\t",
                        ctScan,
                        scanHeader.msLevel,
                        scanHeader.retentionTime,
                        scanHeader.precursorMZ);
      
                  if (iPrintHeaderFile==2)
                  {
                     if (scanHeader.peaksCount>0)
                     {
                        int iloop;
                        int n = 0;
                        double dBasePeakMass=0.0;  /* report base peak mass */
                        double dBasePeakInten=0.0;
                        double dFragmentMass1=0.0; /* report most intense ion != precursor; likely same as base peak */
                        double dInten1=0.0;
                        double dFragmentMass2=0.0; /* report 2nd most intense ion != precursor && != dFragmentMass1 */
                        double dInten2=0.0;
                        RAMPREAL *pPeaks;
               
                        /*
                         * read scan peaks
                         */
                        pPeaks = readPeaks( pFI, pScanIndex[ctScan]);
                        for (iloop=scanHeader.peaksCount; iloop-->0; )
                        {
                           double dMass=pPeaks[n++];
                           double dInten=pPeaks[n++];
                              
                           if (dInten > dBasePeakInten)
                           {
                              dBasePeakInten = dInten;
                              dBasePeakMass = dMass;
                           }
         
                           /*
                            * get 1st most intense fragment peak that's not around precursor
                            */
                           if (fabs(dMass - scanHeader.precursorMZ)>30.0 && dInten > dInten1)
                           {
                              if (fabs(dMass - dFragmentMass2)>30.0)
                              {
                                 dInten2 = dInten1;
                                 dFragmentMass2 = dFragmentMass1; /* save prev most intense mass */
                              }
         
                              dInten1 = dInten;
                              dFragmentMass1 = dMass;
         
                              if (fabs(dFragmentMass1 - dFragmentMass2) <= 30.0)
                              {
                                 dFragmentMass2=0.0;
                                 dInten2=0.0;
                              }
                           }
         
                           /*
                            * get 2nd most intense fragment peak that's not around precursor and 1st most intense peak
                            */
                           else if (fabs(dMass - scanHeader.precursorMZ)>30.0
                                 && fabs(dMass - dFragmentMass1)>30.0
                                 && dInten > dInten2)
                           {
                              dInten2 = dInten;
                              dFragmentMass2 = dMass;
                           }
                        }
                        free(pPeaks);
         
                        fprintf(fpout, "%0.6f\t%0.6f\t%0.6f\t%d\n", dBasePeakMass, dFragmentMass1, dFragmentMass2, scanHeader.precursorCharge);
                     }
                     else
                     {
                        fprintf(fpout, "%0.6lf\t%0.6f\t%0.6f\t%d\n", 0.0, 0.0, 0.0, scanHeader.precursorCharge);
                     }
                     fprintf(fpout, "\n");
                  }
                  else
                  {
                     fprintf(fpout, "%s\t", scanHeader.activationMethod);
   
                     if (scanHeader.peaksCount>0)
                     {
                        int iloop;
                        int n = 0;
                        double d98inten= 0.0;       /* intensity of -98 mass */
                        double d49inten= 0.0;       /* intensity of -49 mass */
                        double d327inten= 0.0;       /* intensity of -32.7 mass */
                        double dBasePeakInten= 0.0;
                        double dTolerance = 1.0;
                        RAMPREAL *pPeaks;

   
                        /*
                         * read scan peaks
                         */
                        pPeaks = readPeaks( pFI, pScanIndex[ctScan]);
                        for (iloop=scanHeader.peaksCount; iloop-->0; )
                        {
                           double dMass=pPeaks[n++];
                           double dInten=pPeaks[n++];
   
                           if (dInten > dBasePeakInten)
                              dBasePeakInten = dInten;

                           if (fabs(scanHeader.precursorMZ - dMass - 98.0)<dTolerance && dInten > d98inten)
                              d98inten = dInten;
                           if (fabs(scanHeader.precursorMZ - dMass - 49.0)<dTolerance && dInten > d49inten)
                              d49inten = dInten;
                           if (fabs(scanHeader.precursorMZ - dMass - 32.7)<dTolerance && dInten > d327inten)
                              d327inten = dInten;
                        }

                        if (dBasePeakInten > 0.0)
                        {
                           fprintf(fpout, "%0.0f\t", 100.0 *d98inten / dBasePeakInten);
                           fprintf(fpout, "%0.0f\t", 100.0 *d49inten / dBasePeakInten);
                           fprintf(fpout, "%0.0f\t", 100.0 *d327inten / dBasePeakInten);
                        }
                        else
                        {
                           fprintf(fpout, "0.0\t0.0\t");
                        }
                     }
                     fprintf(fpout, "\n");
                  }
               }
            }
         }
      }
         
      fclose(fpout);
      printf("\n File created:  %s\n", szOutFile);
   }
   else
   {
      /*
       * interactive or performance mode
       */
      long  ctScan = 0;
      while ((performanceTest && ++ctScan<=iAnalysisLastScan)  ||
         bInteractive || (scanArgNum<argc))
      {
         char  szInput[500];
         RAMPREAL *pPeaks;
         struct ScanHeaderStruct scanHeader;
   
   
         if (performanceTest) {
            szInput[0]=1;
         } else {
            if (iPrintPeaksOption!=5)
               printf("\n");
            szInput[0]='\0';
         }

         if (bInteractive) 
         {
            printf(" Enter a scan number (1-%d) or 'q' to quit: ", iAnalysisLastScan); fflush(stdout);
            char *fgot=fgets(szInput, 400, stdin);
   
            if (szInput[0]=='q')
            {
               break;
            }
            ctScan = atol(szInput);
         }
         else if (!performanceTest)
         {
            ctScan = atol(argv[scanArgNum++]);
            sprintf(szInput, "%ld", ctScan);
         }
   
         if (strlen(szInput)==0 || ctScan<iAnalysisFirstScan || ctScan>iAnalysisLastScan)
         {
            continue;
         }

         /*
          * read scan header
          */
         readHeader(pFI, pScanIndex[ctScan], &scanHeader);
         if (scanHeader.msLevel==0) continue;
         if (!performanceTest && iPrintPeaksOption<4) {
            printf("\n");
            printf("------- Scan %ld header information -------\n", ctScan);
			printf("        filterLine: %s\n", scanHeader.filterLine);
            printf("        msLevel: %d\n",   scanHeader.msLevel);
            if (scanHeader.msLevel>1) {
               printf("    precursorMZ: %0.4lf\n",   scanHeader.precursorMZ);
               if (strlen(scanHeader.activationMethod)>0)
                  printf("     activation: %s\n",    scanHeader.activationMethod);
            }
            printf("       numPeaks: %d\n",       scanHeader.peaksCount);
            printf("       scanTime: %0.2lf\n",   scanHeader.retentionTime);

            if (scanHeader.lowMZ>scanHeader.highMZ) {
               printf("          lowMZ: not given\n");
               printf("         highMZ: not given\n");
            } else  {
               printf("          lowMZ: %0.2lf\n",   scanHeader.lowMZ);
               printf("         highMZ: %0.2lf\n",   scanHeader.highMZ);
            } 
            printf("\n");
         }
   
         if (scanHeader.peaksCount>0 && iPrintPeaksOption != 2)
         {
            int iloop;
            if (bInteractive && iPrintPeaksOption!=1) 
            {
               printf(" ... hit enter to see peaks list"); fflush(stdout);
               getchar();
            }
   
            /*
             * read scan peaks
             */
            pPeaks = readPeaks( pFI, pScanIndex[ctScan]);
            if (!performanceTest) {
               int n = 0;
               int iCount = 0;
               for (iloop=scanHeader.peaksCount; iloop-->0; )
               {
                  double fMass=pPeaks[n++];
                  double fInten=pPeaks[n++];


                  if (iPrintPeaksOption==5)
                     printf("%.6f  %.6f\n", fMass, fInten);
                  else
                     printf("%4d.  mass %10.4f  inten %12.4f\n", ++iCount, fMass, fInten);

                  if (iPrintPeaksOption==1 && iCount==5)
                     break;
               }
            }
            free(pPeaks);
         }
      } /*for*/
   }

   rampCloseFile(pFI);

} /*READ_FILE*/


void SET_OPTION(char *arg,
        int *iPrintHeaderFile,
        int *iPrintPeaksOption)
{
   switch (arg[1])
   {
      case 'h':
         *iPrintHeaderFile = 1;
         break;
      case 'H':
         *iPrintHeaderFile = 2;  /* different header columns */
         break;
      case 'd':
         *iPrintHeaderFile = 3;  /* simple diagnostic output */
         break;
      case 'b':
         *iPrintPeaksOption = 1; /*brief peak list*/
         break;
      case 'n':
         *iPrintPeaksOption = 2; /*no peak list*/
         break;
      case 'p':
         *iPrintPeaksOption = 3; /*no output*/
         break;
      case 's':
         *iPrintPeaksOption = 4; /* m/z inten peaks only, no header*/
         break;
      case 'r':
         *iPrintPeaksOption = 5; /* m/z inten peaks only, no header, numbers only */
         break;
      case '3':
         *iPrintHeaderFile = 5;  /* scan, m/z, inten for 3d plotting */
         break;
      case 'P':
         *iPrintHeaderFile = 4;  /* phospho output */
         break;
      default:
         printf("unknown option \"%c\", quit\n",arg[1]);
         exit(1);
         break;
   }
   arg[0] = '\0';

} /*SET_OPTIONS*/


void USAGE(char *argv0)
{
   printf(" USAGE:\n");
   printf("         %s [options] input.mzXML [scan# [scan# [ ...]]]\n", argv0);
   printf("    or:  %s [options] input.mzData [scan# [scan# [...]]]\n", argv0);
   printf("   scan number arguments are optional - operation is interactive without them\n");
   printf("\n");
   printf(" options:  -h   print out header information to tab-delimited text file (input.txt)\n");
   printf("           -H   tab-delimited text output, MS2 scans\n");
   printf("           -b   print brief peak list (first 5 peaks only) in interactive mode\n");
   printf("           -n   do not print out peak list in interactive mode\n");
   printf("           -p   performance test - parse entire file with no output\n");
   printf("           -d   print simple diagnostics: #MS1/MS2/tot scans, LC time\n");
   printf("           -s   export scan peak list only for scans specified on command line\n");
   printf("           -r   print m/z and intensities only; no headers or other labels\n");
   printf("           -3   print out scan, m/z, intensity of MS1 scans for 3D plotting\n");
   printf("           -P   tab-delimited text output, includes -98 -49 -32.7 columns\n");
   printf("\n");

   exit(EXIT_FAILURE);
} /*USAGE*/
