/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library or "Lesser" General Public      *
 *   License (LGPL) as published by the Free Software Foundation;          *
 *   either version 2 of the License, or (at your option) any later        *
 *   version.                                                              *
 *                                                                         *
 ***************************************************************************/


/*
 * Create subset sequence databases; reverse database; database statistics
 * Jimmy Eng, Copyright 2004, all rights reserved
 *
 * date initiated: 2004/06/17
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define INIT_SEQ_LEN         1000
#define SIZE_BUF             50000
#define SIZE_FILE            1024
#define MAX_FILTER           5000
#define SIZE_STRING          64

int  iAACount[256];

void SET_OPTION(char *arg,
      char *szOutput,
      char (*pMatch)[SIZE_STRING],
      char (*pNoMatch)[SIZE_STRING],
      int  *iNumMatch,
      int  *iNumNoMatch,
      int  *bOutputDir,
      int  *bVerbose,
      int  *bCaseSensitive,
      int  *bReverseDb,
      char *szReverseText,
      int  *bPrintStats,
      int  *bRemoveM,
      int  *bMutuallyExclusive);

void SUBSET_DBASE(char *szInput,
      char *szOutput,
      char (*pMatch)[SIZE_STRING],
      char (*pNoMatch)[SIZE_STRING],
      int  iNumMatch,
      int  iNumNoMatch,
      int  bCaseSensitive,
      int  bReverseDb,
      char *szReverseText,
      int  bPrintStats,
      int  bRemoveM,
      int  bMutuallyExclusive);


int main(int argc, char **argv)
{
   int  i;
   int  iNumArg;
   int  iStartArgc;
   int  iNumMatch;
   int  iNumNoMatch;
   int  bOutputDir;        /* default FALSE=write to current dir; TRUE=write to input db dir */
   int  bVerbose;
   int  bCaseSensitive;    /* default FALSE=case insensitive, TRUE=case sensitive */
   int  bReverseDb;        /* default FALSE, TRUE=reverse sequences*/
   int  bPrintStats;       /* default FALSE, TRUE=print db statistics*/
   int  bRemoveM;
   int  bMutuallyExclusive;  /* default FALSE, TRUE= apply pNoMatch separately */
   char pMatch[MAX_FILTER][SIZE_STRING];
   char pNoMatch[MAX_FILTER][SIZE_STRING];
   char szInput[SIZE_FILE];
   char szOutput[SIZE_FILE];
   char szReverseText[SIZE_FILE];
   char *arg;

   printf("\n");
   printf(" SubsetDB by J.Eng\n");
   printf("\n");

   if (argc<2)
   {
      printf(" USAGE:  %s [options] fasta.database\n", argv[0]);
      printf("\n");
      printf(" Options:   -M<str>    strings to match\n");
      printf("            -N<str>    strings to not match\n");
      printf("                       Note: if -M or -N strings point to file names that exist,\n");
      printf("                             the match/nomatch accessions will be read from those\n");
      printf("                             files; one accession per line.\n");
      printf("            -e         by default, -N option only excludes entries that pass the -M\n");
      printf("                             criteria.  Using this option, both -M and -N options\n");
      printf("                             are mutually exclusive.\n");
      printf("            -O<str>    new subset database name (default appends .new to input name)\n");
      printf("            -P         put subset database in same directory as input database\n");
      printf("            -C         use case-sensitive comparisons (default is case in-sensitive)\n");
      printf("            -R         reverse output database (\"rev_\" appended to description lines.)\n");
      printf("            -D<str>    when reversing output, append <string> instead of \"rev_\" to description lines.\n");
      printf("                          using this parameter invokes -R automatically.\n");
      printf("            -S         print database statistics\n");
      printf("            -V         verbose reporting\n");
      printf("            -m         cleave/remove N-term methionine from each sequence\n");
      printf("\n");
      printf(" A maximum of %d strings each can be specied for match/no match.  Spaces can\n", MAX_FILTER);
      printf(" be substituted in the the string comparison by using the carat (^) character,\n");
      printf(" for example 'bos^taurus'.\n");
      printf("\n");
      printf(" Output database is written to the current directory unless a full path output\n");
      printf(" is specified or the -P option is used.\n");
      printf("\n");
      printf(" If no strings match/no match or statistics options are specified then output\n");
      printf(" database is validated version of input database with non-alphabetic characters\n");
      printf(" stripped out of the sequence.\n");
      printf("\n");
      printf(" If no command line arguments are given, the entire input database will be read\n");
      printf(" in and written out.\n");
      printf("\n");
      exit(EXIT_FAILURE);
   }

   iNumArg=0;
   arg = argv[iNumArg = 1];
   iStartArgc=1;
   bOutputDir=0;
   bVerbose=0;
   bCaseSensitive=0;
   bReverseDb=0;
   bRemoveM=0;
   bMutuallyExclusive=0;
   bPrintStats=0;
   szInput[0]='\0';
   szOutput[0]='\0';
   iNumMatch=0;
   iNumNoMatch=0;
   strcpy(szReverseText, "rev_");

   while (iNumArg < argc)
   {
      if (arg[0] == '-')
         SET_OPTION(arg, szOutput, pMatch, pNoMatch, &iNumMatch, &iNumNoMatch,
            &bOutputDir, &bVerbose, &bCaseSensitive, &bReverseDb, szReverseText, &bPrintStats,
            &bRemoveM, &bMutuallyExclusive);
      else
         break;

      iStartArgc++;
      arg = argv[++iNumArg];
   }

   /*
    * need to check if pMatch or pNoMatch entries point to a file; if so
    * read accessions from file.
    */
   if (iNumMatch>0)
   {
      for (i=0; i<iNumMatch; i++)
      {
         FILE *fp;

         if ( (fp=fopen(pMatch[i], "r"))!=NULL)
         {
            char szBuf[SIZE_BUF];

            iNumMatch=0;
            printf(" *** match file %s", pMatch[i]);
            fflush(stdout);
            while (fgets(szBuf, SIZE_BUF, fp))
            {
               char szAccession[SIZE_BUF];

               szAccession[0]=0;

               sscanf(szBuf, "%s", szAccession);
               if (strlen(szAccession)>0 && iNumMatch<MAX_FILTER)
               {
                  strncpy(pMatch[iNumMatch], szAccession, SIZE_STRING);
                  iNumMatch++;
               }
            }
            printf(", %d strings to match\n", iNumMatch);
            if (iNumMatch == MAX_FILTER)
               printf(" *** warning, maximum number of match strings reached\n");
            break;
         }
      }
   }
   if (iNumNoMatch>0)
   {
      for (i=0; i<iNumNoMatch; i++)
      {
         FILE *fp;

         if ( (fp=fopen(pNoMatch[i], "r"))!=NULL)
         {
            char szBuf[SIZE_BUF];

            iNumNoMatch=0;
            printf(" *** no-match file %s", pMatch[i]);
            fflush(stdout);
            while (fgets(szBuf, SIZE_BUF, fp))
            {
               char szAccession[SIZE_BUF];

               szAccession[0]=0;

               sscanf(szBuf, "%s", szAccession);
               if (strlen(szAccession)>0 && iNumMatch<MAX_FILTER)
               {
                  strncpy(pMatch[iNumMatch], szAccession, SIZE_STRING);
                  iNumMatch++;
               }
            }
            printf(", %d strings to not match\n", iNumMatch);
            if (iNumMatch == MAX_FILTER)
               printf(" *** warning, maximum number of no-match strings reached\n");
            break;
         }
      }
   }

   if (iStartArgc == argc)
   {
      printf(" Please enter original database file on the command line.\n\n");
      exit(EXIT_FAILURE);
   }
   else
      strcpy(szInput, argv[iStartArgc]);


   if (szOutput[0]=='\0') /* output not specified */
   {
      if (bOutputDir)  /* use output full path */
      {
         strcpy(szOutput, szInput);
         strcat(szOutput, ".new");
      }
      else
      {
         char *pStr;

         if ((pStr=strrchr(szInput, '/')))
         {
            strcpy(szOutput, pStr+1);
            strcat(szOutput, ".new");
         }
         else
         {
            strcpy(szOutput, szInput);
            strcat(szOutput, ".new");
         }
      }
   }
   else /*output specified */
   {
      if (bOutputDir && isalnum(szOutput[0]))  /* see if input path required */
      {
         char *pStr;
         char szTmp[SIZE_FILE];

         strcpy(szTmp, szInput);
         if ((pStr=strrchr(szTmp, '/')))
         {
            *(pStr+1) ='\0';
            strcat(szTmp, szOutput);
            strcpy(szOutput, szTmp);
         }
      }
   }

   for (i=0; i<iNumMatch; i++)
   {
      int ii;
      
      for (ii=0; ii<(int)strlen(pMatch[i]); ii++)
      {
         if (!bCaseSensitive)
            pMatch[i][ii]=toupper(pMatch[i][ii]);
 
         if (pMatch[i][ii]=='^')
            pMatch[i][ii]=' ';
      }
   }
   for (i=0; i<iNumNoMatch; i++)
   {
      int ii;
      
      for (ii=0; ii<(int)strlen(pNoMatch[i]); ii++)
      {
         if (!bCaseSensitive)
            pNoMatch[i][ii]=toupper(pNoMatch[i][ii]);

         if (pNoMatch[i][ii]=='^')
            pNoMatch[i][ii]=' ';
      }
   }


   if (bVerbose)
   {
      printf(" match string:\n");
      for (i=0; i<iNumMatch; i++)
         printf("  %d   %s\n", i+1, pMatch[i]);

      printf(" no match string:\n");
      for (i=0; i<iNumNoMatch; i++)
         printf("  %d   %s\n", i+1, pNoMatch[i]);

      printf(" input %s\n", szInput);
      printf(" output %s\n", szOutput);
      if (bReverseDb)
         printf(" decoy text: \"%s\"\n", szReverseText);
      printf("\n");
   }

   SUBSET_DBASE(szInput, szOutput, pMatch, pNoMatch, iNumMatch,
      iNumNoMatch, bCaseSensitive, bReverseDb, szReverseText, bPrintStats,
      bRemoveM, bMutuallyExclusive);

   printf("\n Done.\n\n");
   return(EXIT_SUCCESS);

} /*main*/


void SET_OPTION(char *arg,
      char *szOutput,
      char (*pMatch)[SIZE_STRING],
      char (*pNoMatch)[SIZE_STRING],
      int  *iNumMatch,
      int  *iNumNoMatch,
      int  *bOutputDir,
      int  *bVerbose,
      int  *bCaseSensitive,
      int  *bReverseDb,
      char *szReverseText,
      int  *bPrintStats,
      int  *bRemoveM,
      int  *bMutuallyExclusive)
{
   switch (arg[1])
   {
      case 'M':
         if (*iNumMatch<MAX_FILTER)
         {
            strncpy(pMatch[*iNumMatch], arg+2, SIZE_STRING);
            *iNumMatch += 1;
         }
         break;
      case 'N':
         if (*iNumNoMatch<MAX_FILTER)
         {
            strncpy(pNoMatch[*iNumNoMatch], arg+2, SIZE_STRING);
            *iNumNoMatch += 1;
         }
         break;
      case 'O':
         strcpy(szOutput, arg+2);
         break;
      case 'e':
         *bMutuallyExclusive = 1;
         break;
      case 'm':
         *bRemoveM = 1;
         break;
      case 'P':
         *bOutputDir = 1;
         break;
      case 'V':
         *bVerbose = 1;
         break;
      case 'C':
         *bCaseSensitive= 1;
         break;
      case 'R':
         *bReverseDb = 1;
         break;
      case 'D':
         strcpy(szReverseText, arg+2);
         *bReverseDb = 1;
         break;
      case 'S':
         *bPrintStats = 1;
         break;
      default:
         break;
   }
   arg[0] = '\0';

} /*SET_OPTION*/



void SUBSET_DBASE(char *szInput,
      char *szOutput,
      char (*pMatch)[SIZE_STRING],
      char (*pNoMatch)[SIZE_STRING],
      int  iNumMatch,
      int  iNumNoMatch,
      int  bCaseSensitive,
      int  bReverseDb,
      char *szReverseText,
      int  bPrintStats,
      int  bRemoveM,
      int  bMutuallyExclusive)
{
   int  i;

   long lLenAllocated,
        lLenSeq,
        lMinLength=0,
        lMaxLength=0;

   long lNumMatch=0,
        lNumTot=0,
        lNumResidues=0;

   FILE *fp,
        *fpOut;

   char szBuf[SIZE_BUF],
        *pSeq,
        szDef[SIZE_BUF],      /* protein def line from database*/
        szCaseDef[SIZE_BUF];  /* upper case version of above */


   if ( (fp=fopen(szInput, "r"))==NULL)
   {
      printf(" Error - read:  %s\n\n", szInput);
      exit(EXIT_FAILURE);
   }

   if ( (fpOut=fopen(szOutput, "w"))==NULL)
   {
      printf(" Error - write:  %s\n\n", szOutput);
      exit(EXIT_FAILURE);
   }

   if ( (pSeq=(char*)malloc(INIT_SEQ_LEN))==NULL)
   {
      printf(" Error cannot malloc pSeq\n\n");
      exit(EXIT_FAILURE);
   }

   lLenAllocated=INIT_SEQ_LEN;

   if (bPrintStats)
      memset(iAACount, 0, sizeof(iAACount));

   printf("\n running ...");
   fflush(stdout);

   while (fgets(szBuf, SIZE_BUF, fp))
   {
      if (szBuf[0]=='>') /*definition line*/
      {
         int cResidue;
         int bPass;

         strcpy(szDef, szBuf);
         szDef[strlen(szDef)-1]='\0';

         if (bCaseSensitive)
            strcpy(szCaseDef, szDef);
         else
         {
            int ii;
            for (ii=0; ii<(int)strlen(szDef); ii++)
               szCaseDef[ii] = toupper(szDef[ii]);
            szCaseDef[strlen(szDef)]='\0';
         }

         lLenSeq=0;

         while ((cResidue=fgetc(fp)))
         {
            if (isalpha(cResidue))
            {
               pSeq[lLenSeq]=cResidue;
               lLenSeq++;

               if (bPrintStats)
                  iAACount[toupper(cResidue)] += 1;

               if (lLenSeq == lLenAllocated-1)
               {
                  char *pStr;

                  lLenAllocated += 500;
                  pStr=(char*)realloc(pSeq, lLenAllocated);

                  if (pStr==NULL)
                  {
                     printf(" Error - cannot realloc pSeq[%ld]\n\n", lLenAllocated);
                     exit(EXIT_FAILURE);
                  }
                  pSeq=pStr;
               }
            }
            else if (feof(fp) || cResidue=='>')
            {
               int iReturn;

               iReturn=ungetc(cResidue, fp);
               if (iReturn!=cResidue)
               {
                  printf("Error with ungetc.\n\n");
                  fclose(fp);
                  fclose(fpOut);
                  exit(EXIT_FAILURE);
               }
               break;
            }
         }

         pSeq[lLenSeq]='\0';

         if (lNumTot==0)
         {
            lMaxLength = lLenSeq;
            lMinLength = lLenSeq;
         }
         else
         {
            if (lLenSeq > lMaxLength)
               lMaxLength = lLenSeq;
            if (lLenSeq < lMinLength)
               lMinLength = lLenSeq;
         }

         lNumTot++;
         lNumResidues += lLenSeq;

         bPass=0;

         if (iNumMatch==0 && iNumNoMatch==0)
            bPass=1;
         else
         {
            /*
             * check match strings
             */
            for (i=0; i<iNumMatch; i++)
               if (strstr(szCaseDef, pMatch[i]))
                  bPass=1;

            /*
             * next check no match strings
             */
            if (bMutuallyExclusive)
            {
               /*
                * With the -e option, if protein matches *any* of the -N
                * entries, it will not be printed exported
                */
               int bTmp=1;

               for (i=0; i<iNumNoMatch; i++)
                  if (strstr(szCaseDef, pNoMatch[i]))
                  {
                     bTmp=0;
                     break;
                  }

               if (bTmp==1)
                  bPass = 1;
            }
            else
            {
               if (bPass)
                  for (i=0; i<iNumNoMatch; i++)
                     if (strstr(szCaseDef, pNoMatch[i]))
                        bPass=0;
            }
         }

         if (bPass)
         {
            lNumMatch++;

            if (bReverseDb)
               fprintf(fpOut, ">%s%s\n", szReverseText, szDef+1);
            else
               fprintf(fpOut, "%s\n", szDef);

            if (bReverseDb)
            {
               for (i=0; i<lLenSeq; i++)
               {
                  fprintf(fpOut, "%c", pSeq[lLenSeq-i-1]);
                  if (!((i+1) % 80))
                     fprintf(fpOut, "\n");
               }
            }
            else
            {
               int iStart;

               if (bRemoveM && pSeq[0]=='M')
                  iStart=1;
               else
                  iStart=0;
               for (i=iStart; i<lLenSeq; i++)
               {
                  fprintf(fpOut, "%c", pSeq[i]);
                  if (!((i+1) % 80))
                     fprintf(fpOut, "\n");
               }
            }

            if ((i) % 80)
               fprintf(fpOut, "\n");
         }
      }
   }

   free(pSeq);

   fclose(fp);

   if (iNumMatch!=0 || iNumNoMatch!=0 || bReverseDb)
   {
      fclose(fpOut);

      printf("\n\n");
      printf(" %ld entries matched out of %ld total entries.\n", lNumMatch, lNumTot);
      printf(" File created:  %s\n", szOutput);
   }
   else if (bPrintStats)
   {
      printf("\n\n");
      printf(" Statistics for file '%s'\n\n", szInput);
      printf(" Tot num of sequence entries: %ld\n", lNumTot);
      printf("         Tot num of residues: %ld\n", lNumResidues);
      printf("         Min sequence length: %ld\n", lMinLength);
      printf("         Max sequence length: %ld\n", lMaxLength);

      printf("         Residue Composition: A %d\n", iAACount['A']);
      if (iAACount['B'] > 0)
         printf("                              B %d\n", iAACount['B']);
      printf("                              C %d\n", iAACount['C']);
      printf("                              D %d\n", iAACount['D']);
      printf("                              E %d\n", iAACount['E']);
      printf("                              F %d\n", iAACount['F']);
      printf("                              G %d\n", iAACount['G']);
      printf("                              H %d\n", iAACount['H']);
      printf("                              I %d\n", iAACount['I']);
      if (iAACount['J'] > 0)
         printf("                              J %d\n", iAACount['J']);
      printf("                              K %d\n", iAACount['K']);
      printf("                              L %d\n", iAACount['L']);
      printf("                              M %d\n", iAACount['M']);
      printf("                              N %d\n", iAACount['N']);
      if (iAACount['O'] > 0)
         printf("                              O %d\n", iAACount['O']);
      printf("                              P %d\n", iAACount['P']);
      printf("                              Q %d\n", iAACount['Q']);
      printf("                              R %d\n", iAACount['R']);
      printf("                              S %d\n", iAACount['S']);
      printf("                              T %d\n", iAACount['T']);
      if (iAACount['U'] > 0)
         printf("                              U %d\n", iAACount['U']);
      printf("                              V %d\n", iAACount['V']);
      printf("                              W %d\n", iAACount['W']);
      if (iAACount['X'] > 0)
         printf("                              X %d\n", iAACount['X']);
      printf("                              Y %d\n", iAACount['Y']);
      if (iAACount['Z'] > 0)
         printf("                              Z %d\n", iAACount['Z']);
   }
    
} /*SUBSET_DBASE*/
