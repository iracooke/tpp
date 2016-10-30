/*
 * 20081030, Jimmy Eng
 *                      
 * This program generates FDR plots from input pep.xml files
 *
 * FDR = percentage of accepted hits above cutoff that are incorrect,
 *       estimated as #decoy/#target
 * qvalue = calculated as minimum FDR threshold at which a given
 *          potential peptide hit is accepted
 *
 * To calculate qvalue, first calculate simple FDR for all hits then
 * go backwards/reverse in accepted target hits and assign FDR as
 *
 * if (FDR > prevFDR)
 *    Qvalue = prevFDR;
 * else if (FDR < prevFDR)
 *    prevFDR = FDR;
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#define SIZE_FILE 512
#define SIZE_BUF 81920

char szAttributeVal[SIZE_FILE];

struct InputStruct
{
   int bForward;
   double dFDR;
   double dQvalue;
   double dScore;
} *pInput;

struct OptionsStruct
{
   double dXrange;
   double dSize;
   double dTargetDecoyRatio;
   char szTitle[500];
   char szDecoy[100];
   char szScores[100];
   char szImageName[SIZE_FILE];
   char szGnuPlotFile[SIZE_FILE];
   int bCleanUp;
} pOptions;

void SET_OPTION(char *arg);
void READ_PEPXML(char *szInputFile);
void PRINT_FDR(FILE *fpout,
      char *pSpectrumElement);
char *GET_VAL(char *pSpectrumElement,
      const char *szAttribute);
void PRINT_GP(void);

int main(int argc, char *argv[])
{
   int i;
   int iNumArg;
   int iStartArgc;
   char szCommand[512];
   char *arg;

   if (argc < 2)
   {
      printf("\n");
      printf(" This program generates FDR plots from pep.xml files.                 - J.Eng\n");
      printf("\n");
      printf(" Input files are pep.xml files that have been searched against a target:decoy sequence\n");
      printf(" database.  Default score that the FDR is calculated on is PeptideProphet's probability\n");
      printf(" score but this can be changed to 'hyper', 'expect', or 'xcorr'.\n");
      printf("\n");
      printf(" USAGE:  %s [options] *.pep.xml(s) \n", argv[0]);
      printf("\n");
      printf("         options:  -Xnum     where num is a float for maximum FDR value to plot (x-axis)\n");
      printf("                   -Snum     where num is a float (0.0 to 1.0) for size of created image (default=1.0)\n");
      printf("                   -Tstring  where string will be printed as title value (empty=no title)\n");
      printf("                   -Pstring  where string is a comma separated list of scores that will be\n");
      printf("                             plotted.  Default is 'prob'; also 'hyper', 'expect', 'xcorr' allowed\n");
      printf("                   -Dstring  where string is the decoy identifier (default is 'rev_')\n");
      printf("                   -Nstring  where string is basename for output PNG image (default is 1st input filename)\n");
      printf("                   -Rnum     where num is a float representing target:decoy ratio (default=1.0)\n");
      printf("                                e.g. if decoy size is twice target database size, use -R0.5\n");
      printf("                   -C        do not cleanup intermediate files\n");
      printf("\n");
      exit(1);
   }
   else
      printf("\n Running ...\n");

   iNumArg=0;
   iStartArgc=1;
   arg = argv[iNumArg = 1];
   pOptions.dXrange = 0.05;
   pOptions.dSize = 1.0;
   pOptions.dTargetDecoyRatio = 1.0;
   pOptions.bCleanUp = 1;
   strcpy(pOptions.szDecoy, "rev_");
   strcpy(pOptions.szScores, "prob");
   pOptions.szImageName[0]=0;
   pOptions.szTitle[0]=0;

   while (iNumArg < argc)
   {
      if (arg[0] == '-')
      {
         SET_OPTION(arg);
      }
      else
         break;

      iStartArgc++;
      arg = argv[++iNumArg];
   }

   if (strlen(pOptions.szImageName)==0)
      strcpy(pOptions.szImageName, argv[iStartArgc]);

   sprintf(pOptions.szGnuPlotFile, "%s.fdr.gp", argv[iStartArgc]);
   PRINT_GP();

   for (i=iStartArgc; i<argc; i++)
   {
      printf(" parsing file: %s\n", argv[i]);
      READ_PEPXML(argv[i]);
   }

   sprintf(szCommand, "echo \"LAST\" >> %s ; perl -i -pe 'undef $/ ; s/,\\\\\\nLAST//' %s",
         pOptions.szGnuPlotFile, pOptions.szGnuPlotFile);
   int errCheckMe = -1;
   errCheckMe = system(szCommand);

   sprintf(szCommand, "gnuplot %s", pOptions.szGnuPlotFile);
   errCheckMe = -1;
   errCheckMe = system(szCommand);

   if (pOptions.bCleanUp)
   {
      for (i=iStartArgc; i<argc; i++)
      {
         char *tok;
         char szScores[100];

         strcpy(szScores, pOptions.szScores);
         tok=strtok(szScores, ",");
         while (tok!=NULL)
         {
            sprintf(szCommand, "%s.fdr.%s", argv[i], tok);
            unlink(szCommand);
            tok=strtok(NULL, ",");
         }
      }
      unlink(pOptions.szGnuPlotFile);
   }

   printf("\n Created:  %s.png\n\n", argv[iStartArgc]);

   return(0);
} /*main*/




void SET_OPTION(char *arg)
{
   switch (arg[1])
   {
      case 'X':
         sscanf(arg+2, "%lf", &(pOptions.dXrange));
         break;
      case 'S':
         sscanf(arg+2, "%lf", &(pOptions.dSize));
         break;
      case 'T':
         strcpy(pOptions.szTitle, arg+2);
         break;
      case 'P':
         strcpy(pOptions.szScores, arg+2);
         break;
      case 'D':
         strcpy(pOptions.szDecoy, arg+2);
         break;
      case 'N':
         strcpy(pOptions.szImageName, arg+2);
         break;
      case 'R':
         sscanf(arg+2, "%lf", &(pOptions.dTargetDecoyRatio));
         break;
      case 'C':
         pOptions.bCleanUp = 0;
         break;
   }
}


void PRINT_GP(void)
{
   FILE *fp;

   if ((fp=fopen(pOptions.szGnuPlotFile, "w"))==NULL)
   {
      printf(" Error - cannot write file %s\n", pOptions.szGnuPlotFile);
      exit(1);
   }

   fprintf(fp, "set terminal png\n");
   fprintf(fp, "set xlabel \"FDR (q-value)\"\n");
   fprintf(fp, "set ylabel \"# peptide IDs\"\n");
   fprintf(fp, "set key center bottom\n");
   fprintf(fp, "\n");
   fprintf(fp, "set size 1.0\n");
   fprintf(fp, "set output \"%s.png\"\n", pOptions.szImageName);
   fprintf(fp, "\n");
   fprintf(fp, "set size square %0.2f\n", pOptions.dSize);
   fprintf(fp, "set xrange [-0.001:%0.3f]\n", pOptions.dXrange);
   fprintf(fp, "set xtics nomirror\n");
   fprintf(fp, "set ytics nomirror\n");
   fprintf(fp, "set yrange [*:*]\n");
   fprintf(fp, "set key center bottom\n");
   if (strlen(pOptions.szTitle)>0)
      fprintf(fp, "set title \"%s\"\n", pOptions.szTitle);

   fprintf(fp, "plot ");

   fclose(fp);
}


void READ_PEPXML(char *szInputFile)
{
   FILE *fpin;
   FILE *fpout;
   char szOutputFile[SIZE_FILE];
   char szPlotFile[SIZE_FILE];
   char szScores[100];
   char *tok;
   int iWhichColumn;

   strcpy(szOutputFile, szInputFile);
   strcat(szOutputFile, ".fdr");

   if ((fpin=fopen(szInputFile, "r"))!=NULL)
   {
      if ((fpout=fopen(szOutputFile, "w"))!=NULL)
      {
         char szBuf[SIZE_BUF];
         char *pSpectrumElement;
         int iSizeSpectrumElement;

         iSizeSpectrumElement = SIZE_BUF;
         pSpectrumElement = (char *)malloc(iSizeSpectrumElement * sizeof(char));
         if (pSpectrumElement==NULL)
         {
            printf(" Error allocating pSpectrumElement\n");
            exit(1);
         }

         while (fgets(szBuf, SIZE_BUF, fpin))
         {
            /* grab entire spectrum query tag */
            if (strstr(szBuf, "<spectrum_query spectrum"))
            {
               strcpy(pSpectrumElement, szBuf);

               /* append subsequent lines together */
               pSpectrumElement[strlen(pSpectrumElement)-1]='\0';
               strcat(pSpectrumElement, " ");

               while (fgets(szBuf, SIZE_BUF, fpin))
               {
                  if (!strstr(szBuf, "alternative_protein"))
                  {
                     if (strlen(szBuf) + strlen(pSpectrumElement) >= iSizeSpectrumElement)
                     {
                        char *pTmp;

                        pTmp = (char*) realloc(pSpectrumElement, iSizeSpectrumElement + SIZE_BUF);
                        if (pTmp = NULL)
                        {
                           printf(" Error realloc (size=%d)\n", iSizeSpectrumElement + SIZE_BUF);
                           exit(1);
                        }

                        pSpectrumElement = pTmp;
                        iSizeSpectrumElement += SIZE_BUF;
                     }
                     strcat(pSpectrumElement, szBuf);
                  }

                  /* append subsequent lines together */
                  pSpectrumElement[strlen(pSpectrumElement)-1]='\0';
                  strcat(pSpectrumElement, " ");

                  if (strstr(szBuf, "</spectrum_query>"))
                  {
                     PRINT_FDR(fpout, pSpectrumElement);
                     break;
                  }
               }

            }
         }
         fclose(fpout);
      }
      else
      {
         printf(" error - cannot write file %s\n", szOutputFile);
         exit(1);
      }
   }
   else
   {
      printf(" error - cannot read input file %s\n", szInputFile);
      exit(1);
   }

   /*
    * split each *.fdr file into individual score components
    * and sort in correct order
    */
   strcpy(szScores, pOptions.szScores);

   tok=strtok(szScores, ",");
   iWhichColumn=2;
   while (tok!=NULL)
   {
      char szCommand[SIZE_FILE];
      char szSortedScoreFile[SIZE_FILE];
      char szLine[512];
      int  iCount;
      int  iNumEntries;
      int  iDecoy;
      int  iTarget;
      int  i;
      double dPrevFDR;

      sprintf(szSortedScoreFile, "%s.%s", szOutputFile, tok);

      if (!strcmp(tok, "prob") || !strcmp(tok, "xcorr") || !strcmp(tok, "hyper"))
      {
         sprintf(szCommand, "cat %s | awk '{print $1 \"\\t\" $%d}' | sort -k 2,2 -r > %s",
               szOutputFile, iWhichColumn, szSortedScoreFile);
         int errCheckMe = -1;
	 errCheckMe = system(szCommand);
      }
      else if (!strcmp(tok, "expect") || !strcmp(tok, "spscore"))
      {
         sprintf(szCommand, "cat %s | awk '{print $1 \"\\t\" $%d}' | sort -k 2,2  > %s",
               szOutputFile, iWhichColumn, szSortedScoreFile);
         int errCheckMe = -1;
	 errCheckMe = system(szCommand);
      }

      if ( (fpin=fopen(pOptions.szGnuPlotFile, "a"))!=NULL)
      {
         fprintf(fpin, "      \"%s\" using 2:1 with lines title \"%s.%s\",\\\n",
               szSortedScoreFile, szInputFile, tok);
         fclose(fpin);
      }
      else
      {
         printf(" Error - cannot append file qvalue.gp\n\n");
      }

      /*
       * Find number entries in input file
       */
      sprintf(szCommand, "wc -l %s", szSortedScoreFile);

      if ( (fpin=popen(szCommand, "r"))==NULL)
      {
         printf(" Error with popen: %s\n", szCommand);
         exit(1);
      }
      char* errCheckMe2 = NULL;
      errCheckMe2 = fgets(szLine, 512, fpin);
      pclose(fpin);

      sscanf(szLine, "%d", &iNumEntries);

      /*
       * Allocate memory to store all input
       */
      pInput = (InputStruct*) malloc(iNumEntries * sizeof(struct InputStruct));
      if (pInput == NULL)
      {
         printf(" Error with malloc (%d)\n", iNumEntries);
         exit(1);
      }
      memset(pInput, 0, iNumEntries * sizeof(struct InputStruct));

      /*
       * Read in input
       */
      if ( (fpin=fopen(szSortedScoreFile, "r"))==NULL)
      {
         printf(" error with fopen: %s\n", szSortedScoreFile);
         exit(1);
      }

      iCount=0;
      while (fgets(szLine, 512, fpin))
      {
         double dScore=0.0;
         char szProtein[512];
         char szTmp[512];

         sscanf(szLine, "%s %s\n", szProtein, szTmp);
         if (!strcmp(szTmp, "-"))
         {
         }
         else
         {
            sscanf(szTmp, "%lf", &dScore);

            pInput[iCount].dScore = dScore;
            if (strncmp(szProtein, "rev_", 4))
               pInput[iCount].bForward = 1;
            else
               pInput[iCount].bForward = 0;

            iCount++;
         }
      }
      fclose(fpin);

      sprintf(szPlotFile, "%s.plot", szSortedScoreFile);
      if ( (fpout=fopen(szPlotFile, "w"))==NULL)
      {
         printf("\n Error -  cannot open %s to write\n", szPlotFile);
         exit(1);
      }

      /*
       * Now calculate simple FDR = TargetDecoyRatio * (#decoy / #forward);
       */
      iDecoy=0;
      iTarget=0;
      for (i=0; i<iCount; i++)
      {
         if (pInput[i].bForward == 1)
            iTarget++;
         else
            iDecoy++;

         pInput[i].dFDR = pOptions.dTargetDecoyRatio * ((double)iDecoy / (double)iTarget);
      }

      /*
       * Now calculate qvalue
       */
      pInput[iCount-1].dQvalue = pInput[iCount-1].dFDR;
      dPrevFDR = pInput[iCount-1].dFDR;

      for (i=iCount-2; i>=0; i--)
      {
         pInput[i].dQvalue = pInput[i].dFDR;

         if (pInput[i].dQvalue > dPrevFDR)
            pInput[i].dQvalue = dPrevFDR;
         else if (pInput[i].dQvalue < dPrevFDR)
            dPrevFDR = pInput[i].dQvalue;
      }

      iDecoy=0;
      iTarget=0;
      fprintf(fpout, "#forward\tqvalue\tsimple_fdr\tscore\t#decoy\n");
      for (i=0; i<iCount; i++)
      {
         if (pInput[i].bForward == 1)
            iTarget++;
         else
            iDecoy++;

         fprintf(fpout, "%d\t%f\t%f\t%f\t%d\n", iTarget, pInput[i].dQvalue, pInput[i].dFDR, pInput[i].dScore, iDecoy);
      }
      fclose(fpout);
      free(pInput);

      sprintf(szCommand, "mv %s %s", szPlotFile, szSortedScoreFile);
      int errCheckMe = -1;
      errCheckMe = system(szCommand);

      iWhichColumn++;
      tok=strtok(NULL, ",");
   }

   unlink(szOutputFile);

} /*READ_PEPXML*/


void PRINT_FDR(FILE *fpout, char *pSpectrumElement)
{
   char *tok;
   char szScores[100];

   strcpy(szScores, pOptions.szScores);

   fprintf(fpout, "%s\t", GET_VAL(pSpectrumElement, "protein"));

   tok=strtok(szScores, ",");
   while (tok!=NULL)
   {
      if (!strcmp(tok, "prob"))
         fprintf(fpout, "%s\t", GET_VAL(pSpectrumElement, "probability"));
      else if (!strcmp(tok, "xcorr"))
         fprintf(fpout, "%s\t", GET_VAL(pSpectrumElement, "\"xcorr\" value"));
      else if (!strcmp(tok, "hyper"))
         fprintf(fpout, "%s\t", GET_VAL(pSpectrumElement, "\"hyperscore\" value"));
      else if (!strcmp(tok, "expect"))
         fprintf(fpout, "%s\t", GET_VAL(pSpectrumElement, "\"expect\" value"));
      else if (!strcmp(tok, "spscore"))
         fprintf(fpout, "%s\t", GET_VAL(pSpectrumElement, "\"spscore\" value"));

      tok=strtok(NULL, ",");
   }
   fprintf(fpout, "\n");
}


/* for a given attribute, return the attribute value */
char *GET_VAL(char *pSpectrumElement, const char *szAttribute)
{
   char *pStr;

   if ( (pStr=strstr(pSpectrumElement, szAttribute))!=NULL)
   {
      strncpy(szAttributeVal, pStr+strlen(szAttribute)+2, SIZE_FILE);  /* +2 to skip =" */

      if ( (pStr=strchr(szAttributeVal, '"'))!=NULL)
      {
         *pStr='\0';
         return(szAttributeVal);
      }
      else
      {
         //printf(" Error - expecting an end quote in %s\n", szAttributeVal);
	 strcpy(szAttributeVal, "-");
	 return(szAttributeVal);
      }
   }
   else
   {
      //printf("Error - cannot find %s in %s\n", szAttribute, pSpectrumElement);
      strcpy(szAttributeVal, "-");
      getchar();
      return(szAttributeVal);
   }
} /*GET_VAL*/
