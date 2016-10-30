/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

/*
 * Output View by Jimmy Eng (c) Institute for Systems Biology, 2002
 *
 * Date Initiated:  11/15/2002
 * Purpose:  Replace sequest showout cgi program by reading tgz output files
 *
 * 2002/11/15:  add in links to spectral display, database sequence, and NCBI Blast
 * 2002/12/04:  modify tgz filename creationg ... Mac IE browser changes /./ in input .dta filename to /
 * 2005/08/30:  handle sequest searches not run by runsearch, which don't have their .out files tarred up - bpratt
 * 2005/09/12:  add in __SORCERER__ support
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "common/TPPVersion.h" // contains version number, name, revision
#include "common/AminoAcidMasses.h"
#include "common/tpp_tarball.h" // tarfile access

#define TITLE "Output View"

#define COMETLINKS

#include "Visualization/Comet/Comet.h"
#include "common/constants.h"
 
struct EnvironmentStruct
{
   char szInputFile[SIZE_FILE];
   char szFullPathInputFile[SIZE_FILE];
   char szDtaFile[SIZE_FILE];
   char szTarFile[SIZE_FILE];
} pEnvironment;


void EXTRACT_CGI_QUERY(void);
void INITIALIZE(void);
#include "common/util.h"

static int is_path_sep(char c) {
   return ('/'==c)||('\\'==c);
}

static void fail(int code) {
   printf("</BODY></HTML>\n");
   exit(code);
}

int main(int argc, char **argv)
{
   hooks_tpp handler(argc,argv); // set up install paths etc
   FILE *ppIn,
        *fpPlot;

   int  i,
        iLen,
        iMassType=-1,
        bNucDb=0,
        bIncludeMod=0,
        bTurboSequest=0;

   double dMass1=0.0,
          dMass2=0.0,
          dMass3=0.0,
          dMass4=0.0,
          dMass5=0.0,
          dMass6=0.0,
          pdMassAA[128];

   int    pbStaticModAA[128]; /* track which AA are statically modified */
   int    bStaticallyModified;

   char szBaseName[SIZE_FILE],
        szBuf[SIZE_BUF],
        szTmp[SIZE_BUF],
        szMod[SIZE_BUF],
        szDatabase[SIZE_BUF],
        szPeptideLink[SIZE_BUF],
        *pStr;

   double fA=0.0, fB=0.0, fC=0.0, fD=0.0, fV=0.0, fW=0.0, fX=0.0, fY=0.0, fZ=0.0;

   char Cometlinksfile[SIZE_BUF];

   char *cp;

   //#ifndef WINDOWS_CYGWIN
   sprintf(Cometlinksfile, "%s%s", COMETLINKSDIR, COMETLINKSFILE);
   //#else
   //sprintf(Cometlinksfile, "%s%s", LOCAL_BIN, COMETLINKSFILE);
   //#endif

  /*
   * Print HTML header
   */
   printf("Content-type: text/html\n\n");
   printf("<HTML>\n");
   printf("   <HEAD>\n");
   printf("      <TITLE>%s by J.Eng (c) ISB 2001 (%s)</TITLE>\n", TITLE, szTPPVersionInfo);
   printf("   </HEAD>\n");
   printf("\n");
   printf("<BODY BGCOLOR=\"%s\" OnLoad=\"self.focus();\">\n", "#FFFFFF");
   printf("<B>%s (%s)</B><BR>\n", TITLE, szTPPVersionInfo);

   INITIALIZE();

   EXTRACT_CGI_QUERY();

   /*
    * pEnvironment.szInputFile has input .dta file ... need to get this
    * from the tar/gzipped archive now
    *
    * So, first modify szTarFile to be the gzipped tar file name
    */
   strcpy(pEnvironment.szTarFile, pEnvironment.szInputFile);
   strcpy(pEnvironment.szDtaFile, pEnvironment.szInputFile);
   pEnvironment.szDtaFile[strlen(pEnvironment.szDtaFile)-4]='\0';
   strcat(pEnvironment.szDtaFile, ".dta");

   iLen=strlen(pEnvironment.szTarFile);
   for (i=iLen-1; i>2; i--)
   {
      if (is_path_sep(pEnvironment.szTarFile[i]) && pEnvironment.szTarFile[i-1]=='.' && is_path_sep(pEnvironment.szTarFile[i-2]))
      {
         pEnvironment.szTarFile[i-2]='\0';
         strcat(pEnvironment.szTarFile, ".tgz");
         break;
      }
      else if (is_path_sep(pEnvironment.szTarFile[i]))
      {
         pEnvironment.szTarFile[i]='\0';
         strcat(pEnvironment.szTarFile, ".tgz");
         break;
      }
   }

   iLen=strlen(pEnvironment.szTarFile);
   strcpy(szBaseName, pEnvironment.szTarFile);
   if (!strstr(pEnvironment.szTarFile+iLen-4, ".tgz"))
   {
      printf(" Error converting %s to tar file (%s).\n", pEnvironment.szInputFile, pEnvironment.szTarFile);
      fail(0);
   }

   /*
    * Get NCBI Blast link
    */
   szPeptideLink[0]='\0';
   if ( (fpPlot=fopen(Cometlinksfile, "r"))==NULL)
   {
      printf(" Error - cannot open %s\n", Cometlinksfile);
      fail(EXIT_FAILURE);
   }
   while (fgets(szBuf, SIZE_BUF, fpPlot))
   {
      if (!strncmp(szBuf, "PEPTIDELINK=", 12))
      {
         strcpy(szPeptideLink, szBuf+12);
         break;
      }
   }
   fclose(fpPlot);

   /*
    * Get local directory & make it current working dir.  This needs to be
    * set prior to the call to read_dta_or_out_from_tgz_file() in case 
	* tmpdir hasn't been specified so we don't try to create tmpfile in
	* cgi-bin
    */
   char *szLocalDir = strdup(pEnvironment.szTarFile);
   cp = findRightmostPathSeperator(szLocalDir);
   if (cp) { // any path info provided?
	   *cp = 0; // cut off the filename
	   verified_chdir(szLocalDir);
   }
   free(szLocalDir);

   ppIn = read_dta_or_out_from_tgz_file(pEnvironment.szTarFile, // not const, contents may be altered
                         pEnvironment.szInputFile, // not const, contents may be altered
                         pEnvironment.szFullPathInputFile, // not const, contents may be altered
                         sizeof(pEnvironment.szTarFile)); // assumed buffer size for all three inputs

   printf("<PRE>");
   for (i=5;i--;) {
      if ((!ppIn) || !fgets(szBuf, SIZE_BUF, ppIn)) {
         if (ppIn && feof(ppIn)) {
            puts("empty file?");
         }
         printf(" Error - can't read .out file %s: %s\n\n", 
            pEnvironment.szInputFile, (ppIn && ferror(ppIn))?strerror(errno):"unknown error");
         fail(EXIT_FAILURE);
      }
      printf("%s", szBuf);
   }      
#ifndef __SORCERER__
   char *fgott=fgets(szBuf, SIZE_BUF, ppIn); printf("%s", szBuf);
#endif
  
   /* grab mass type */
   char *fgot=fgets(szBuf, SIZE_BUF, ppIn); printf("%s", szBuf);
   if (strstr(szBuf, "/MONO"))
      iMassType=1;
   else if (strstr(szBuf, "/AVG"))
      iMassType=0;

   INITIALIZE_MASS(pdMassAA, iMassType);
   memset(pbStaticModAA, 0, sizeof(pbStaticModAA));
   bStaticallyModified=0;

   fgot=fgets(szBuf, SIZE_BUF, ppIn); printf("%s", szBuf);

   /* gets sequence database */
   fgot=fgets(szBuf, SIZE_BUF, ppIn); printf("%s", szBuf);
   iLen=strlen(szBuf);
   for (i=iLen-1; i>0; i--)
      if (szBuf[i]==' ')
         break;
   strcpy(szDatabase, szBuf+i+1);
   szDatabase[strlen(szDatabase)-1]='\0';
   if (strstr(szBuf, "# bases ="))
      bNucDb=1;

   /* get ion series selected for plotting */
   fgot=fgets(szBuf, SIZE_BUF, ppIn); printf("%s", szBuf);
   sscanf(szBuf, "%s %s %s %s %s %s %s %lf %lf %lf %lf %lf %lf %lf %lf %lf",
         szTmp, szTmp, szTmp, szTmp, szTmp, szTmp, szTmp,
         &fA, &fB, &fC, &fD, &fV, &fW, &fX, &fY, &fZ);

   fgot=fgets(szBuf, SIZE_BUF, ppIn); printf("%s", szBuf);

   fgot=fgets(szBuf, SIZE_BUF, ppIn); printf("%s", szBuf);

   szMod[0]='\0';
   if (strlen(szBuf)>2)
   {
      bIncludeMod=1;

      /* check for modifications */
      if ((pStr=strchr(szBuf, '*')))
         sscanf(pStr+1, "%lf", &dMass1);
      if ((pStr=strchr(szBuf, '#')))
         sscanf(pStr+1, "%lf", &dMass2);
      if ((pStr=strchr(szBuf, '@')))
         sscanf(pStr+1, "%lf", &dMass3);
      if ((pStr=strchr(szBuf, '^')))
         sscanf(pStr+1, "%lf", &dMass4);
      if ((pStr=strchr(szBuf, '~')))
         sscanf(pStr+1, "%lf", &dMass5);
      if ((pStr=strchr(szBuf, '$')))
         sscanf(pStr+1, "%lf", &dMass6);

      /*
       * delete terminating semi-cleavage output as this adversely affects the
       * next strrchr() command logic if left in place; relevant only to isb but
       * shouldn't affect other versions
       */
      if ((pStr=strstr(szBuf, "(2:1)")))
         *pStr='\0';

      /* look for static modifications */
      if ((pStr=strchr(szBuf, '=')))
      {
         int i,
             iLen;
         char tmp[32];
         int tmp_i = 0;

         iLen=strlen(szBuf);

         for (i=1; i<iLen; i++)
         {
            if (szBuf[i]=='=')
            {
               double dMass=0.0;
     
               sscanf(szBuf+i+1, "%lf", &dMass);
               if ((pStr=strstr(tmp, "N-term"))) {
                  sprintf(szMod+strlen(szMod), "&amp;Nterm=%0.4f", dMass);
                  pdMassAA['n'] = dMass;
                  pbStaticModAA['n'] = 1;
                  bStaticallyModified=1;
               }
               else if ((pStr=strstr(tmp, "C-term"))) {
                  sprintf(szMod+strlen(szMod), "&amp;Cterm=%0.4f", dMass);
                  pdMassAA['c'] = dMass;
                  pbStaticModAA['c'] = 1;
                  bStaticallyModified=1;
               }
               else {
                  pdMassAA[szBuf[i-1]] = dMass;
                  pbStaticModAA[szBuf[i-1]] = 1;
                  bStaticallyModified=1;
               }
               tmp_i=0;
               tmp[tmp_i] = '\0';
            }
            else 
            {
               tmp[tmp_i] = szBuf[i];
               tmp_i++;
               tmp[tmp_i] = '\0';
            }
         }
      }
   }

   //fgets(szBuf, SIZE_BUF, ppIn);
   //printf("%s", szBuf);
   //fgets(szBuf, SIZE_BUF, ppIn);
   //printf("%s", szBuf);
   //if (bIncludeMod)
   //{
   //   fgets(szBuf, SIZE_BUF, ppIn);
   //   printf("%s", szBuf);
   //}
   if (bIncludeMod)
   {
      char *fgot=fgets(szBuf, SIZE_BUF, ppIn);
      printf("%s", szBuf);

      fgot=fgets(szBuf, SIZE_BUF, ppIn);
      printf("%s", szBuf);
      if (strstr(szBuf, "Id#"))
         bTurboSequest=1;

      fgot=fgets(szBuf, SIZE_BUF, ppIn);
      printf("%s", szBuf);
   }
   else
   {
      char *fgot=fgets(szBuf, SIZE_BUF, ppIn);
      printf("%s", szBuf);
      if (strstr(szBuf, "Id#"))
         bTurboSequest=1;

      fgot=fgets(szBuf, SIZE_BUF, ppIn);
      printf("%s", szBuf);
   }

   /*
    * print out search lines
    */
   while (fgets(szBuf, SIZE_BUF, ppIn))
   {
      int ii;
      int iLen=strlen(szBuf);
      int iLenPep;
      int iNum1, iRankXC, iRankSp, iIon, iTot, iId;
      double dMass, dDeltCn, dXC, dSp;
      char szProt[200], szDup[200], szPep[200], szIons[50];
      char szPlainPep[200],
           DSite[200];
      char szTmpBuf[SIZE_BUF];
      int  iSlashCount=0;

   
      szProt[0]='\0';
      szDup[0]='\0';
      szPep[0]='\0';
      szIons[0]='\0';
      szPlainPep[0]='\0';
      memset(DSite, 0, sizeof(DSite));
      szTmpBuf[0]='\0';

      if (iLen < 2)
      {
         printf("%s", szBuf);
         break;
      }

      /* only substitute first 2 slashes */
      for (i=0; i<iLen; i++)
      {
         if (szBuf[i]=='/' && iSlashCount<2)
         {
            szTmpBuf[i]=' ';
            iSlashCount++;
         }
         else
            szTmpBuf[i]=szBuf[i];
      }

      if (bTurboSequest)
      {
         sscanf(szTmpBuf, "%d. %d %d %d %lf %lf %lf %lf %d %d %s %s %s",
            &iNum1, &iRankXC, &iRankSp, &iId,
            &dMass, &dDeltCn, &dXC, &dSp,
            &iIon, &iTot,
            szProt, szDup, szPep);
      }
      else
      {
         sscanf(szTmpBuf, "%d. %d %d %lf %lf %lf %lf %d %d %s %s %s",
            &iNum1, &iRankXC, &iRankSp,
            &dMass, &dDeltCn, &dXC, &dSp,
            &iIon, &iTot,
            szProt, szDup, szPep);
      }
 
      if (szProt[0]!='\0')
      {
         sprintf(szIons, "%d/%d", iIon, iTot);

//printf("<br>DDS: Ions: %s Pro: %s Dup: %s Pep: %s <br>", szIons, szProt, szDup, szPep); fflush(stdout);
         if (szDup[0]!='+')
         {
            strcpy(szPep, szDup);
            szDup[0]='\0';
         }
   
         strcpy(szPep, szPep+2);
         szPep[strlen(szPep)-2]='\0';
//printf("<br>DDS: szPep: %s<br>", szPep); fflush(stdout);
         iLenPep=strlen(szPep);
         ii=0;
         for (i=0; i<iLenPep; i++)
         {
            if (!isalpha(szPep[i]))
            {
               if (szPep[i]=='*')
                  DSite[ii-1]='1';
               else if (szPep[i]=='#')
                  DSite[ii-1]='2';
               else if (szPep[i]=='@')
                  DSite[ii-1]='3';
               else if (szPep[i]=='^')
                  DSite[ii-1]='4';
               else if (szPep[i]=='~')
                  DSite[ii-1]='5';
               else if (szPep[i]=='$')
                  DSite[ii-1]='6';
            }
            else
            {
               szPlainPep[ii]=szPep[i];
               DSite[ii]='0';
               ii++;
            }
         }
         szPlainPep[ii]='\0';
         DSite[ii]='\0';
//printf("<br>DDS: szPlainPep: %s<br>", szPlainPep); fflush(stdout);

//*** STALLS INSIDE FOR LOOP
         for (i=0; i<iLen; i++)
         { 
   
            if (!strncmp(szBuf+i, szIons, strlen(szIons)))
            {
               printf("<A HREF=\"%splot-msms.cgi?Dta=%s&amp;MassType=%d%s",
                     CGI_BIN, pEnvironment.szDtaFile, iMassType, szMod);
   
               if (bStaticallyModified || strlen(szPep)!=strlen(szPlainPep))
               {
                  int j;
   
                  for (j=0; j<iLenPep; j++)
                  {
                     /*
                      * still need to account for N- and C-term mods here
                      */

                     if (DSite[j]=='1')
                        printf("&amp;Mod%d=%0.8f", j+1, pdMassAA[szPlainPep[j]] + dMass1);
                     else if (DSite[j]=='2')
                        printf("&amp;Mod%d=%0.8f", j+1, pdMassAA[szPlainPep[j]] + dMass2);
                     else if (DSite[j]=='3')
                        printf("&amp;Mod%d=%0.8f", j+1, pdMassAA[szPlainPep[j]] + dMass3);
                     else if (DSite[j]=='4')
                        printf("&amp;Mod%d=%0.8f", j+1, pdMassAA[szPlainPep[j]] + dMass4);
                     else if (DSite[j]=='5')
                        printf("&amp;Mod%d=%0.8f", j+1, pdMassAA[szPlainPep[j]] + dMass5);
                     else if (DSite[j]=='6')
                        printf("&amp;Mod%d=%0.8f", j+1, pdMassAA[szPlainPep[j]] + dMass6);
                     else if (pbStaticModAA[szPlainPep[j]] == 1) /* static mod */
                        printf("&amp;Mod%d=%0.8f", j+1, pdMassAA[szPlainPep[j]]);
                  }
               }
   
               if (pbStaticModAA['n'] == 1)
                  printf("&map;ModN=%0.8f", pdMassAA['n']);
               if (pbStaticModAA['c'] == 1)
                  printf("&map;ModC=%0.8f", pdMassAA['c']);
               
               printf("&amp;Pep=%s\">%s</A>", szPlainPep, szIons);
   
               i += strlen(szIons)-1;
            }
            else if (!strncmp(szBuf+i, szProt, strlen(szProt)))
            {
               printf("<A HREF=\"%scomet-fastadb.cgi?Ref=%s&amp;Db=%s&amp;Pep=%s&amp;MassType=%d\">%s</A>",
                     CGI_BIN, szProt, szDatabase, szPlainPep, iMassType, szProt);
               i += strlen(szProt)-1;
            }
            else if (!strncmp(szBuf+i, szPep, strlen(szPep)))
            {
               printf("<A HREF=\"%s%s\">%s</A>", szPeptideLink, szPlainPep, szPep);
               i += strlen(szPep)-1;
            }
            else if (strlen(szDup)>0 && !strncmp(szBuf+i, szDup, strlen(szDup)))
            {
               printf("<A HREF=\"%scomet-fastadb.cgi?Db=%s&amp;Pep=%s&amp;MassType=%d",
                     CGI_BIN, szDatabase, szPlainPep, iMassType);

               if (bNucDb)
                  printf("&amp;NucDb=1");

               printf("\">%s</A>", szDup);

               i += strlen(szDup)-1;
            }
            else
            {
               printf("%c", szBuf[i]);
            }
         }
      }
      else
         printf("%s", szBuf);
           
   }
   
   /*
    * print out the rest of the output file
    */
   while (fgets(szBuf, SIZE_BUF, ppIn))
      printf("%s", szBuf);

   printf("</BODY></HTML>\n");
                            
   close_dta_or_out_from_tgz_file(ppIn);

   return(EXIT_SUCCESS);

} /*main*/


void EXTRACT_CGI_QUERY(void)
{
   char *pRequestType,
        *pQS,
        szWord[512];
   int  i;

   pRequestType=getenv("REQUEST_METHOD");
   if(pRequestType==NULL)
   {
      printf(" This program needs to be called with CGI GET method.\n");
      exit(EXIT_FAILURE);
   }
   else if (strcmp(pRequestType, "GET"))
   {
      printf(" Binary not called with GET method!\n");
      exit(EXIT_FAILURE);
   }

   /*
    * Decode GET method
    */
   pQS = getenv("QUERY_STRING");
   if (pQS == NULL)
   {
      printf("GET query string empty.\n");
      exit(EXIT_FAILURE);
   }

   for (i=0; pQS[0]!='\0';i++)
   {

      getword(szWord, pQS, '=');
      plustospace(szWord);
      unescape_url(szWord);

      if (!strcmp(szWord, "OutFile"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%s", pEnvironment.szInputFile);
      }
      else
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
      }
   }

} /*EXTRACT_CGI_QUERY*/


void INITIALIZE(void)
{

   pEnvironment.szInputFile[0]='\0';

} /*INITIALIZE*/
