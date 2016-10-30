/*
 * COMET-FASTADB by Jimmy Eng (c) Institute for Systems Biology, 2001
 *
 * Date Initiated:  10/31/2001
 * Purpose:  CGI program to access and retrieve sequences from sequences databases.
 *
 * $Id: comet-fastadb1.cxx 6327 2013-11-05 23:57:57Z slagelwa $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define MAX_NUM_PEPTIDES   200
#define MAX_NUM_REFERENCES 100
#define MAX_SEQ            400000

#define TEXTHIGHLIGHT  "#0000FF"
#define TEXTHIGHLIGHT2 "#FF0000"
#define TEXTHIGHLIGHT3 "#FF00FF"

#define TITLE "COMET Sequence View"

#include "common/AminoAcidMasses.h"
#include "Visualization/Comet/Comet.h"
#include "Enzyme/ProteolyticEnzyme/ProteolyticEnzymeFactory/ProteolyticEnzymeFactory.h"
#include "Enzyme/ProteolyticEnzyme/ProteolyticEnzyme.h"

#include "common/TPPVersion.h" // contains version number, name, revision

int inMsgPane = 0;

double pdMassAA[256];

char szPeptideLink[SIZE_BUF];

struct EnvironmentStruct
{
   int  iNumPeptides;
   int  iNumReferences;

   int  bNucleotideDb;
   int  bMonoMass;
   int  iModifiedCys;       /* 0=no, 1=old icat, 2=new icat, 3=iodoacetamide */
   int  bMarkNXST;      /* N-glycosylation motif NxS/T */
   int  iEnzymeKey;
   int  iMissedCleavage;
   int  iSortBy;
   int  iTerm;
   int  iMinNTT;
   int  iReadingFrame;

   char szEnzymeBreak[24];      /* which residues to fragment */
   char szEnzymeNoBreak[24];
   char szMarkAA[24];
   char szInclAA[24];
   double dMinMass;
   double dMaxMass;

   char szDbFile[SIZE_FILE];    /* database filename */
   char szEnzyme[SIZE_FILE];    /* enzyme */
   char szPeptide[MAX_NUM_PEPTIDES][MAX_PEPTIDE_LEN];   /* can take up to MAX_PEPTIDE number of peptides */
   char szReference[MAX_NUM_REFERENCES][MAX_HEADER_LEN];        /* sequence reference */
   char cPepMatched[MAX_NUM_PEPTIDES];
   char cRefMatched[MAX_NUM_PEPTIDES];

   double dPepMass[MAX_NUM_PEPTIDES];


} pOptions;

struct DbStruct
{
   char szHdr[SIZE_BUF];
   char szSeq[MAX_SEQ];
   char szTmpSeq[MAX_SEQ];      /* horrible way of doing this but I'm lazy */
   char szHighLight[MAX_SEQ];
   char szHighLight2[MAX_SEQ];
   char szHighLight3[MAX_SEQ];
   double dMW;
   int  iLenHdr;
   int  iLenSeq;
} pDb;


void EXTRACT_CGI_QUERY(void);
void INITIALIZE(void);
void RETRIEVE_HEADER(FILE * fpdb,
   char *szDbFile);
void SEARCH_PEPTIDE_HEADER(FILE * fpdb,
   char *szDbFile);
void PRINT_PROTEIN(int iReadingFrame,
      char *szSeq,
      int iLenSeq);
double PEPMASS(char *szPeptide);
void DIGEST_PROTEIN(char *szSeq,
       int iLenSeq);

extern double COMPUTE_PI(char *seq,
   unsigned long uliNumberAAcid,
   int charge_increment);

int isTolerableTerminus(char *enzyme,
   char aa);

void TRANSLATE(int iReadingFrame,
      char *szReturnSeq,
      char *szNucSeq,
      int  *iLenSeq);

void prntMsg(const char *message);
void prntPageFooter(void);

#include "common/util.h"
   
ProteolyticEnzymeFactory* enzyme_spec;
ProteolyticEnzyme* enzyme_digest;


int main( int argc, char **argv)
{
  hooks_tpp handler(argc,argv); // set up install paths etc

   FILE *fpdb;
   int  i;
   char szDbFile[SIZE_FILE],
     szBuf[SIZE_BUF],
     tmpMsg[500];

   enzyme_spec = NULL;
   enzyme_digest = NULL;
   /*
    * Print HTML header
    */
   printf("Content-type: text/html\n\n");
   printf("<html>\n");
   printf("<head>\n");
   printf("<title>%s by J.Eng (c) ISB 2001 (%s)</title>\n", TITLE,szTPPVersionInfo);

   // style-sheet
   printf("<style type=\"text/css\">\n");
   printf(".hideit {display:none}\n");
   printf(".showit {display:table-row}\n");
   printf(".accepted {background: #87ff87; font-weight:bold;}\n");
   printf(".rejected {background: #ff8700;}\n");
   printf("body{font-family: Helvetica, sans-serif; }\n");
   printf("h1  {font-family: Helvetica, Arial, Verdana, sans-serif; font-size: 24pt; font-weight:bold; color:#0E207F}\n");
   printf("h2  {font-family: Helvetica, Arial, sans-serif; font-size: 20pt; font-weight: bold; color:#0E207F}\n");
   printf("h3  {font-family: Helvetica, Arial, sans-serif; font-size: 16pt; color:#FF8700}\n");
   printf("h4  {font-family: Helvetica, Arial, sans-serif; font-size: 14pt; color:#0E207F}\n");
   printf("h5  {font-family: Helvetica, Arial, sans-serif; font-size: 10pt; color:#AA2222}\n");
   printf("h6  {font-family: Helvetica, Arial, sans-serif; font-size:  8pt; color:#333333}\n");
   printf("table   {border-collapse: collapse; border-color: #000000;}\n");
   printf(".banner_cid   {\n");
   printf("                 background: #0e207f;\n");
   printf("                 border: 2px solid #0e207f;\n");
   printf("                 color: #eeeeee;\n");
   printf("                 font-weight:bold;\n");
   printf("              }\n");
   printf(".markSeq      {\n");
   printf("                 color: #0000FF;\n");
   printf("                 font-weight:bold;\n");
   printf("              }\n");
   printf(".markAA       {\n");
   printf("                 color: #EE2222;\n");
   printf("                 font-weight:bold;\n");
   printf("              }\n");
   printf(".glyco        {\n");
   printf("                 background: #d0d0ff;\n");
   printf("                 border: 1px solid black;\n");
   printf("              }\n");
   printf(".messages_h   {\n");
   printf("                 background: #FF8700;\n");
   printf("                 border: 2px solid #FF8700;\n");
   printf("                 color: white;\n");
   printf("              }\n");
   printf(".messages     {\n");
   printf("                 background: #ffffff;\n");
   printf("                 border: 2px solid #FF8700;\n");
   printf("                 color: black;\n");
   printf("                 padding: 1em;\n");
   printf("              }\n");
   printf(".formentry    {\n");
   printf("               background: #eeeeee;\n");
   printf("                 border: 2px solid #0e207f;\n");
   printf("                 color: black;\n");
   printf("                 padding: 1em;\n");
   printf("              }\n");
   printf(".nav          {\n");
   printf("                 border-bottom: 1px solid black;\n");
   printf("                 font-weight:bold;\n");
   printf("              }\n");
   printf(".graybox      {\n");
   printf("                 background: #dddddd;\n");
   printf("                 border: 1px solid black;\n");
   printf("                 font-weight:bold;\n");
   printf("              }\n");
   printf(".seq          {\n");
   printf("                 background: #ffaa33;\n");
   printf("                 border: 1px solid black;\n");
   printf("                 font-weight:bold;\n");
   printf("              }\n");
   printf(".info	      {\n");
   printf("                 border-top: 1px solid black;\n");
   printf("                 color: #333333;\n");
   printf("		 font-size: 10pt;\n");
   printf("              }\n");
   printf("</style>\n");

   // javascript
   printf("<script language=\"JavaScript\">\n");
   printf("    function showdiv(div_id,nothidden){\n");
   printf("	if (document.getElementById(div_id).className == nothidden) {\n");
   printf("	  new_state = 'hideit';\n");
   printf("	} else {\n");
   printf("	  new_state = nothidden;\n");
   printf("      }\n");
   printf("	document.getElementById(div_id).className = new_state;\n");
   printf("    }\n");
   printf("    function hilight(id,yesno) {\n");
   printf("      if (yesno == 'yes') {\n");
   printf("	        document.getElementById(id).className = 'seqselected';\n");
   printf("	} else {\n");
   printf("	        document.getElementById(id).className = 'seq';\n");
   printf("	}\n");
   printf("    }\n");
   printf("</script>\n");

   printf("</head>\n\n");

   printf("<body bgcolor=\"%s\" onload=\"self.focus();\" link=%s vlink=%s>\n",
          "#c0c0c0", TEXTHIGHLIGHT, TEXTHIGHLIGHT);

   INITIALIZE();
   EXTRACT_CGI_QUERY();

   if(strlen(pOptions.szEnzyme) > 0) {
     enzyme_spec = new ProteolyticEnzymeFactory();
     enzyme_digest = enzyme_spec->getProteolyticEnzyme(pOptions.szEnzyme);
     if(enzyme_digest == NULL) {
       sprintf(tmpMsg,"Warning: no enzyme information for <b>%s</b>. Ignoring enzyme.", pOptions.szEnzyme);
       prntMsg(tmpMsg);
       pOptions.szEnzyme[0] = 0;
     }
   }

// printf("<h1>%s</h1>\n",TITLE);

   if (strlen(pOptions.szDbFile) == 0)
   {
      prntMsg(" Error - no database file specified.");
      prntPageFooter();
      exit(EXIT_FAILURE);
   }

   /*
    * Get NCBI Blast link
    */
   szPeptideLink[0] = '\0';
   char Cometlinksfile[SIZE_BUF];
   sprintf(Cometlinksfile, "%s%s", COMETLINKSDIR, COMETLINKSFILE);
   if ((fpdb = fopen(Cometlinksfile, "r")) == NULL)
   {
      sprintf(tmpMsg," Error - cannot open <b.%s%s</b>", COMETLINKSDIR, COMETLINKSFILE);
      prntMsg(tmpMsg);
      prntPageFooter();
      exit(EXIT_FAILURE);
   }   


   while (fgets(szBuf, SIZE_BUF, fpdb))
   {
      if (!strncmp(szBuf, "PEPTIDELINK=", 12))
      {
         strcpy(szPeptideLink, szBuf + 12);
         break;
      }
   }
   fclose(fpdb);

   struct stat statbuf;
   if (stat(pOptions.szDbFile,&statbuf)) {
      // db file not found, try prepending wwwroot
      const char *wwwroot = getWebserverRoot();
      if (wwwroot) {
         char szDbFile[SIZE_FILE];    /* database filename */
         strcpy(szDbFile,wwwroot);
         strcat(szDbFile,pOptions.szDbFile);
         for (char *cp;(cp=strstr(szDbFile,"//"));) {
            memmove(cp,cp+1,strlen(cp)); // get rid of double path seps
         }
         if (!stat(szDbFile,&statbuf)) {
            strcpy(pOptions.szDbFile,szDbFile); /* there it is */
         }
      }
   }

   if ((fpdb = fopen(pOptions.szDbFile, "r")) == NULL)
   {
      sprintf(tmpMsg, " Error - cannot open <b>%s</b> (%s)", pOptions.szDbFile, strerror(errno));
      prntMsg(tmpMsg);
      prntPageFooter();
      exit(EXIT_FAILURE);
   }

   for (i=0; i<256; i++)
      pdMassAA[i]=9999.9;

   INITIALIZE_MASS(pdMassAA, pOptions.bMonoMass);

   if (pOptions.iModifiedCys == 1)
      pdMassAA['C'] += 442.2244;
   else if (pOptions.iModifiedCys == 2)
      pdMassAA['C'] += 227.13;
   else if (pOptions.iModifiedCys == 3)
   {
      if (pOptions.bMonoMass == 1)
         pdMassAA['C'] += 57.021464;
      else
         pdMassAA['C'] += 57.0513;
   }


   if (pOptions.iNumReferences > 0)
      RETRIEVE_HEADER(fpdb, szDbFile);
   else if (pOptions.iNumPeptides > 0)
      SEARCH_PEPTIDE_HEADER(fpdb, szDbFile);
   else
   {
      prntMsg(" Error - enter references and/or peptides on link with Ref= &amp; Pep=");
      prntPageFooter();
      exit(EXIT_FAILURE);
   }

   fclose(fpdb);

   if (pOptions.iNumReferences > 0)
   {
      for (i = 0; i < pOptions.iNumReferences; i++)
      {
         if (pOptions.cRefMatched[i] == 0)
	 {
	   sprintf(tmpMsg, "Unmatched ref: <b>%s</b>", pOptions.szReference[i]);
	   prntMsg(tmpMsg);
	 }
      }
   }

   if (pOptions.iNumPeptides > 0)
   {
      for (i = 0; i < pOptions.iNumPeptides; i++)
      {
         if (pOptions.cPepMatched[i] == 0)
	 {
	   sprintf(tmpMsg, "Unmatched pep: <b>%s</b>", pOptions.szPeptide[i]);
	   prntMsg(tmpMsg);
	 }
      }
   }

   prntPageFooter();
   return (EXIT_SUCCESS);

} /*main */


void EXTRACT_CGI_QUERY(void)
{
   char *pRequestType, *pQS, szWord[8000];
   int  i, ii;

   pRequestType = getenv("REQUEST_METHOD");
   if (pRequestType == NULL)
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

   for (i = 0; pQS[0] != '\0'; i++)
   {
      getword(szWord, pQS, '=');
      plustospace(szWord);
      unescape_url(szWord);

      if (!strcmp(szWord, "Db"))
      {
         getword(szWord, pQS, '&');
         plustospace(szWord);
         unescape_url(szWord);
         sscanf(szWord, "%s", pOptions.szDbFile);
         fixPath(pOptions.szDbFile,1); // tidy up path seperators etc - expect existence
      }
      else if (!strcmp(szWord, "Ref"))
      {
         getword(szWord, pQS, '&');
         plustospace(szWord);
         unescape_url(szWord);
         if (pOptions.iNumPeptides < MAX_NUM_REFERENCES)
            sscanf(szWord, "%s", pOptions.szReference[pOptions.iNumReferences++]);
      }
      else if (!strcmp(szWord, "Pep"))
      {
         getword(szWord, pQS, '&');
         plustospace(szWord);
         unescape_url(szWord);

         if (pOptions.iNumPeptides < MAX_NUM_PEPTIDES)
         {
            if (strchr(szWord, ' '))
            {
               int  iLen = (int)strlen(szWord);
               char *t1;

               /*
                * Read in each word.
                */
               for (t1 = strtok(szWord, " "); t1 != NULL;
                    t1 = strtok(NULL, " "))
               {
                  if (pOptions.iNumPeptides < MAX_NUM_PEPTIDES)
                     sscanf(t1, "%s", pOptions.szPeptide[pOptions.iNumPeptides++]);
               }
            }
            else
            {
               sscanf(szWord, "%s", pOptions.szPeptide[pOptions.iNumPeptides++]);
            }
         }
      }
      else if (!strcmp(szWord, "EnzymeKey"))
      {
         getword(szWord, pQS, '&');
         plustospace(szWord);
         unescape_url(szWord);
         sscanf(szWord, "%d", &(pOptions.iEnzymeKey));
      }
      else if (!strcmp(szWord, "InclAA"))
      {
         int  iLen;
         getword(szWord, pQS, '&');
         plustospace(szWord);
         unescape_url(szWord);

         iLen = (int)strlen(szWord);
         for (ii = 0; ii < iLen; ii++)
            pOptions.szInclAA[ii] = toupper(szWord[ii]);
         pOptions.szInclAA[ii] = '\0';
      }
      else if (!strcmp(szWord, "MinMass"))
      {
         getword(szWord, pQS, '&');
         plustospace(szWord);
         unescape_url(szWord);
         sscanf(szWord, "%lf", &(pOptions.dMinMass));
      }
      else if (!strcmp(szWord, "MaxMass"))
      {
         getword(szWord, pQS, '&');
         plustospace(szWord);
         unescape_url(szWord);
         sscanf(szWord, "%lf", &(pOptions.dMaxMass));
      }
      else if (!strcmp(szWord, "MarkAA"))
      {
         int  iLen;

         getword(szWord, pQS, '&');
         plustospace(szWord);
         unescape_url(szWord);

         iLen = (int)strlen(szWord);

         for (ii = 0; ii < iLen; ii++)
            pOptions.szMarkAA[ii] = toupper(szWord[ii]);
         pOptions.szMarkAA[ii] = '\0';
      }
      else if (!strcmp(szWord, "MissedCleave"))
      {
         getword(szWord, pQS, '&');
         plustospace(szWord);
         unescape_url(szWord);
         sscanf(szWord, "%d", &(pOptions.iMissedCleavage));
      }
      else if (!strcmp(szWord, "min_ntt"))
      {
         getword(szWord, pQS, '&');
         plustospace(szWord);
         unescape_url(szWord);
         sscanf(szWord, "%d", &(pOptions.iMinNTT));
      }
      else if (!strcmp(szWord, "sample_enzyme"))
      {
         getword(szWord, pQS, '&');
         plustospace(szWord);
         unescape_url(szWord);
         sscanf(szWord, "%s", pOptions.szEnzyme);
      }
      else if (!strcmp(szWord, "SortBy"))
      {
         getword(szWord, pQS, '&');
         plustospace(szWord);
         unescape_url(szWord);
         sscanf(szWord, "%d", &(pOptions.iSortBy));
      }
      else if (!strcmp(szWord, "IcatCys"))
      {
         getword(szWord, pQS, '&');
         plustospace(szWord);
         unescape_url(szWord);
         sscanf(szWord, "%d", &(pOptions.iModifiedCys));
      }
      else if (!strcmp(szWord, "NucDb"))
      {
         getword(szWord, pQS, '&');
         plustospace(szWord);
         unescape_url(szWord);
         sscanf(szWord, "%d", &(pOptions.bNucleotideDb));
      }
      else if (!strcmp(szWord, "MassType"))
      {
         getword(szWord, pQS, '&');
         plustospace(szWord);
         unescape_url(szWord);
         sscanf(szWord, "%d", &(pOptions.bMonoMass));
      }
      else if (!strcmp(szWord, "ReadingFrame"))
      {
         getword(szWord, pQS, '&');
         plustospace(szWord);
         unescape_url(szWord);
         sscanf(szWord, "%d", &(pOptions.iReadingFrame));
      }
      else if (!strcmp(szWord, "N-Glyco"))
      {
         getword(szWord, pQS, '&');
         plustospace(szWord);
         unescape_url(szWord);
         pOptions.bMarkNXST = 1;
      }
      else
      {
         getword(szWord, pQS, '&');
         plustospace(szWord);
         unescape_url(szWord);
      }
   }

} /*EXTRACT_CGI_QUERY */


void INITIALIZE(void)
{
   int  i;

   for (i = 0; i < MAX_NUM_PEPTIDES; i++)
   {
      pOptions.szPeptide[i][0] = '\0';
      pOptions.dPepMass[i] = 0.0;
      pOptions.cPepMatched[i] = 0;
      pOptions.cRefMatched[i] = 0;
   }
   for (i = 0; i < MAX_NUM_REFERENCES; i++)
   {
      pOptions.szReference[i][0] = '\0';
   }

   pOptions.iNumPeptides = 0;
   pOptions.iNumReferences = 0;
   pOptions.iMissedCleavage = 0;
   pOptions.iSortBy = 1;
   pOptions.iModifiedCys = 0;
   pOptions.bMonoMass = 1;  /* use monoisotopic masses by default */
   pOptions.bMarkNXST = 0;
   pOptions.iTerm = 0;
   pOptions.iEnzymeKey = 0;
   pOptions.iReadingFrame = 0;
   pOptions.szDbFile[0] = '\0';
   strcpy(pOptions.szMarkAA, "");
   strcpy(pOptions.szInclAA, "");
   pOptions.dMinMass = 800.0;
   pOptions.dMaxMass = 4000.0;
   pOptions.szEnzyme[0] = 0;
   pOptions.iMinNTT = 0;
} /*INITIALIZE*/

static void sortcols(const char *szOutFile, int col, bool numerical) {
	char szCommand[SIZE_FILE + SIZE_FILE];
	const char *quot=getCmdlineQuoteChar(); // get a system-appropriate quote char
	std::string tmpname(szOutFile);
	tmpname += ".XXXXXX";
    replace_path_with_webserver_tmp(tmpname); // do this in designated tmpdir, if any
    safe_fclose(FILE_mkstemp(tmpname)); // create then close a uniquely named file
	std::string quoted_outfile = quot;
	quoted_outfile += szOutFile;
	quoted_outfile += quot;
	std::string quoted_tmpfile = quot;
	quoted_tmpfile += tmpname;
	quoted_tmpfile += quot;
	sprintf(szCommand, "sort %s-k %d,%d %s -o %s", numerical?"-n ":"",
	    col, col, quoted_outfile.c_str(),
		quoted_tmpfile.c_str());
	verified_system(szCommand); // like system(), but avoids ambiguity between mingw and dos sort
	safe_rename(tmpname.c_str(),szOutFile);
}

void RETRIEVE_HEADER(FILE * fpdb,
   char *szDbFile)
{
   int  i,
        iNumEntries;
   char szBuf[SIZE_BUF];

   /*
    * header portion of file(s)
    */
   iNumEntries = 0;

   while (fgets(szBuf, SIZE_BUF, fpdb))
   {
      if (szBuf[0] == '>')
      {
         int  ii;

         strcpy(pDb.szHdr, szBuf + 1);

         for (ii = 0; ii < pOptions.iNumReferences; ii++)
         {
            if (!strncasecmp(pOptions.szReference[ii], pDb.szHdr, strlen(pOptions.szReference[ii])))
            {
               int  cAA;
               int  iLenSeq;
               char szSeq[MAX_SEQ];

               pDb.iLenSeq = 0;
               while ((cAA = fgetc(fpdb)))
               {
                  if (feof(fpdb))
                     break;
                  else if (isalpha(cAA))
                  {
                     pDb.szSeq[pDb.iLenSeq] = toupper(cAA);
                     pDb.iLenSeq++;
                     if (pDb.iLenSeq > MAX_SEQ-1)
                        break;
                  }
                  else if (cAA == '>')
                  {
                     int  iReturn;

                     iReturn = ungetc(cAA, fpdb);

                     if (iReturn != cAA)
                     {
                        printf("Error with ungetc.\n\n");
                        exit(EXIT_FAILURE);
                     }
                     break;
                  }
               }

               pOptions.cRefMatched[ii] = 1;

               if (pOptions.bNucleotideDb)
               {
                  int  j;

                  if (pOptions.iReadingFrame == 0)
                  {
                     for (j = 1; j <= 3; j++) /* forward 3 reading frames */
                     {
                        strcpy(szSeq, pDb.szSeq);
                        iLenSeq=pDb.iLenSeq;
                        TRANSLATE(j, szSeq, pDb.szSeq, &iLenSeq);

                        PRINT_PROTEIN(j, szSeq, iLenSeq);
                     }
   
                     /*
                      * reverse complement
                      */
                     memcpy(pDb.szTmpSeq, pDb.szSeq, pDb.iLenSeq);
                     for (i=0; i<pDb.iLenSeq; i++)
                     {
                        if (pDb.szTmpSeq[i]=='G')
                           pDb.szSeq[pDb.iLenSeq-i-1]='C';
                        else if (pDb.szTmpSeq[i]=='C')
                           pDb.szSeq[pDb.iLenSeq-i-1]='G';
                        else if (pDb.szTmpSeq[i]=='A')
                           pDb.szSeq[pDb.iLenSeq-i-1]='T';
                        else if (pDb.szTmpSeq[i]=='T' || pDb.szTmpSeq[i]=='U')
                           pDb.szSeq[pDb.iLenSeq-i-1]='A';
                        else
                           pDb.szSeq[pDb.iLenSeq-i-1]=pDb.szSeq[i];
                     }
                     pDb.szSeq[pDb.iLenSeq]='\0';
   
                     for (j = 1; j <= 3; j++) /* reverse 3 reading frames */
                     {
                        strcpy(szSeq, pDb.szSeq);
                        iLenSeq=pDb.iLenSeq;
                        TRANSLATE(-j, szSeq, pDb.szSeq, &iLenSeq);

                        PRINT_PROTEIN(-j, szSeq, iLenSeq);
                     }
                  }
                  else
                  {
                     if (pOptions.iReadingFrame<0)
                     {
                        /*
                         * reverse complement
                         */
                        memcpy(pDb.szTmpSeq, pDb.szSeq, pDb.iLenSeq);
                        for (i=0; i<pDb.iLenSeq; i++)
                        {
                           if (pDb.szTmpSeq[i]=='G')
                              pDb.szSeq[pDb.iLenSeq-i-1]='C';
                           else if (pDb.szTmpSeq[i]=='C')
                              pDb.szSeq[pDb.iLenSeq-i-1]='G';
                           else if (pDb.szTmpSeq[i]=='A')
                              pDb.szSeq[pDb.iLenSeq-i-1]='T';
                           else if (pDb.szTmpSeq[i]=='T' || pDb.szTmpSeq[i]=='U')
                              pDb.szSeq[pDb.iLenSeq-i-1]='A';
                           else
                              pDb.szSeq[pDb.iLenSeq-i-1]=pDb.szSeq[i];
                        }
                        pDb.szSeq[pDb.iLenSeq]='\0';
                     }

                     strcpy(szSeq, pDb.szSeq);
                     iLenSeq=pDb.iLenSeq;
                     TRANSLATE(pOptions.iReadingFrame, szSeq, pDb.szSeq, &iLenSeq);

                     PRINT_PROTEIN(pOptions.iReadingFrame, szSeq, iLenSeq);

                  }
               }
               else
               {
                  strcpy(szSeq, pDb.szSeq);
                  iLenSeq=pDb.iLenSeq;

                  PRINT_PROTEIN(0, szSeq, iLenSeq);
               }
            }
         }
      }
   }

}/*RETRIEVE_HEADER */


void SEARCH_PEPTIDE_HEADER(FILE * fpdb,
   char *szDbFile)
{
   int  i;
   char szBuf[SIZE_BUF];

   while (fgets(szBuf, SIZE_BUF, fpdb))
   {
      int  cAA;

      if (szBuf[0] == '>')
      {
         int  ii;
         int  iLenSeq;
         char szSeq[MAX_SEQ];

         /*
          * retrieve header
          */
         strcpy(pDb.szHdr, szBuf + 1);

         /*
          * retrieve sequence
          */
         pDb.iLenSeq = 0;
         while ((cAA = fgetc(fpdb)))
         {
            if (isalpha(cAA))
            {
               pDb.szSeq[pDb.iLenSeq] = toupper(cAA);
               pDb.iLenSeq++;
               if (pDb.iLenSeq > MAX_SEQ-1)
                  break;
            }
            else if (cAA == '>')
            {
               int  iReturn;

               iReturn = ungetc(cAA, fpdb);

               if (iReturn != cAA)
               {
                  printf("Error with ungetc.\n\n");
                  exit(EXIT_FAILURE);
               }
               break;
            }
            else if (feof(fpdb))
               break;
         }
         pDb.szSeq[pDb.iLenSeq] = '\0';

         if (pOptions.bNucleotideDb)
         {
            int  j;

            for (j = 1; j <= 3; j++)       /* forward 3 reading frames */
            {
               int x;
               int bPrint=0;

               strcpy(szSeq, pDb.szSeq);
               iLenSeq=pDb.iLenSeq;
               TRANSLATE(j, szSeq, pDb.szSeq, &iLenSeq);

               for (x = 0; x < pOptions.iNumPeptides; x++)
               {
                  if (strstr(szSeq, pOptions.szPeptide[x]))
                  {
                     bPrint=1;
                     break;
                  }
               }

               if (bPrint)
                  PRINT_PROTEIN(j, szSeq, iLenSeq);
               fflush(stdout);
            }

            /*
             * reverse complement
             */
            memcpy(pDb.szTmpSeq, pDb.szSeq, pDb.iLenSeq);
            for (i=0; i<pDb.iLenSeq; i++)
            {
               if (pDb.szTmpSeq[i]=='G')
                  pDb.szSeq[pDb.iLenSeq-i-1]='C';
               else if (pDb.szTmpSeq[i]=='C')
                  pDb.szSeq[pDb.iLenSeq-i-1]='G';
               else if (pDb.szTmpSeq[i]=='A')
                  pDb.szSeq[pDb.iLenSeq-i-1]='T';
               else if (pDb.szTmpSeq[i]=='T' || pDb.szTmpSeq[i]=='U')
                  pDb.szSeq[pDb.iLenSeq-i-1]='A';
               else
                  pDb.szSeq[pDb.iLenSeq-i-1]=pDb.szSeq[i];
            }
            pDb.szSeq[pDb.iLenSeq]='\0';

            for (j = 1; j <= 3; j++)       /* reverse 3 reading frames */
            {
               int x;
               int bPrint=0;

               strcpy(szSeq, pDb.szSeq);
               iLenSeq=pDb.iLenSeq;
               TRANSLATE(-j, szSeq, pDb.szSeq, &iLenSeq);

               for (x = 0; x < pOptions.iNumPeptides; x++)
               {
                  if (strstr(szSeq, pOptions.szPeptide[x]))
                  {
                     bPrint=1;
                     break;
                  }
               }

               if (bPrint)
                  PRINT_PROTEIN(-j, szSeq, iLenSeq);
               fflush(stdout);
            }
         }
         else
         {
            for (ii = 0; ii < pOptions.iNumPeptides; ii++)
            {
               if (strstr(pDb.szSeq, pOptions.szPeptide[ii]))
               {
                  if (pOptions.iMinNTT==0 || !strcmp(pOptions.szEnzyme, "Nonspecific") || enzyme_digest == NULL)
                  {
                     strcpy(szSeq, pDb.szSeq);
                     iLenSeq=pDb.iLenSeq;

                     PRINT_PROTEIN(0, szSeq, iLenSeq);
                     fflush(stdout);
                  }
                  else
                  {
                     int x;
		     //printf("seq: %s<=>\n", pDb.szSeq);
                     /*
                      * Confirm NTT of peptide before printing
                      */
                     for (x=0; x<pDb.iLenSeq; x++)
                     {
		       if(! strncmp(pDb.szSeq + x, pOptions.szPeptide[ii], strlen(pOptions.szPeptide[ii]))) { // check how many ntt
			 char prev = x > 0 && isalpha(pDb.szSeq[x-1]) ? pDb.szSeq[x-1] : '-';
			 char foll = x + (int)strlen(pOptions.szPeptide[ii]) < pDb.iLenSeq && 
			   isalpha(pDb.szSeq[x+strlen(pOptions.szPeptide[ii])]) ? pDb.szSeq[x+strlen(pOptions.szPeptide[ii])] : '-';
			 //printf("found: %c %s %c: %d\n", prev,  pOptions.szPeptide[ii], foll, enzyme_digest->numCompatibleTermini(prev, pOptions.szPeptide[ii], foll));
			 if(enzyme_digest->getNumTolTerm(prev, pOptions.szPeptide[ii], foll) >= pOptions.iMinNTT) { // print

			   strcpy(szSeq, pDb.szSeq);
			   iLenSeq=pDb.iLenSeq;

			   PRINT_PROTEIN(0, szSeq, iLenSeq);
			   fflush(stdout);
			   x = pDb.iLenSeq;
			 }

		       }

                     }
                  }
               }
            }
         }

         if (feof(fpdb))
            break;
      }
   }
} /*SEARCH_PEPTIDE_HEADER */


/*
 * Given retrieve sequence, print out sequence w/peptide coverage
 */
void PRINT_PROTEIN(int iReadingFrame,
      char *szSeq,
      int iLenSeq)
{
   int  i, ii;
   char szReference[MAX_HEADER_LEN],
     szBuf[SIZE_BUF];
   double dPI;

   memset(pDb.szHighLight, '0', pDb.iLenSeq);
   memset(pDb.szHighLight2, '0', pDb.iLenSeq);
   memset(pDb.szHighLight3, '0', pDb.iLenSeq);
   pDb.szHighLight[pDb.iLenSeq] = '\0';
   pDb.szHighLight2[pDb.iLenSeq] = '\0';
   pDb.szHighLight3[pDb.iLenSeq] = '\0';

   // Protein pane
   printf("<table cellspacing=\"0\">\n");
   printf("<tr>\n<td class=\"banner_cid\">&nbsp;&nbsp;");

   if (iReadingFrame == 0)
   {
      printf("Protein: ");
   }
   else if (iReadingFrame > 0)
   {
      printf("5'3' frame %d: ", iReadingFrame);
   }
   else
   {
      printf("3'5' frame %d: ", iReadingFrame);
   }

   strcpy(szBuf, pDb.szHdr);
   printf("<font color=\"ff8700\">%s</font>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>\n</tr></table>\n\n", strtok(szBuf," ") );
   printf("<div class=\"formentry\">\n<tt>");

   for (ii = 0; ii < iLenSeq; ii++)
   {
      for (i = 0; i < pOptions.iNumPeptides; i++)
      {
         int  iLenPeptide = (int)strlen(pOptions.szPeptide[i]);

         if (!strncmp(szSeq + ii, pOptions.szPeptide[i], iLenPeptide))
         {
            int  j;

            pOptions.cPepMatched[i] = 1;

            for (j = 0; pOptions.szPeptide[i][j]; j++)
            {
               pDb.szHighLight[ii + j] = '1';
            }
         }
      }

      if (strchr(pOptions.szMarkAA, szSeq[ii]))
      {
         pDb.szHighLight2[ii] = '1';
      }

      if (pOptions.bMarkNXST
          && ii + 2 < iLenSeq
          && szSeq[ii] == 'N'
          && szSeq[ii + 1] != 'P'
          && (szSeq[ii + 2] == 'S' || szSeq[ii + 2] == 'T'))
      {
         pDb.szHighLight3[ii] = '1';
         pDb.szHighLight3[ii + 1] = '1';
         pDb.szHighLight3[ii + 2] = '1';
      }
   }

   printf("<div class=\"graybox\">\n<tt>&gt;");
   if (strstr(pDb.szHdr, "IPI:"))
   {
      int  ii, iLen = (int)strlen(pDb.szHdr);
      char *pStr, szRef[200];

      for (i = 0; i < iLen; i++)
      {
         if (!strncmp(pDb.szHdr + i, "IPI0", 4))
         {
            ii = 0;
            while (pDb.szHdr[i + 4 + ii] != '|'
                   && pDb.szHdr[i + 4 + ii] != ' '
                   && pDb.szHdr[i + 4 + ii] != '.')
            {
               ii++;
            }
            strncpy(szRef, pDb.szHdr + i, ii + 4);
            szRef[ii + 4] = '\0';
            i += (int)strlen(szRef);

            printf("<A HREF=\"http://srs.ebi.ac.uk/srsbin/cgi-bin/wgetz?-e+[ipi-AccNumber:'%s']\">%s</A>%c",
                szRef, szRef, pDb.szHdr[i]);
         }
         else if (!strncmp(pDb.szHdr + i, "SWISS-PROT:", 11))
         {
            ii = 0;
            while (pDb.szHdr[i + 11 + ii] != '|'
                   && pDb.szHdr[i + 11 + ii] != ' ')
            {
               szRef[ii] = pDb.szHdr[i + 11 + ii];
               ii++;
            }
            szRef[ii] = '\0';
            i += 11 + (int)strlen(szRef);

            printf("SWISS-PROT:<A HREF=\"http://www.uniprot.org/uniprot/%s\">%s</A> %c",
                szRef, szRef, pDb.szHdr[i]);
         }
         else if (!strncmp(pDb.szHdr + i, "TREMBL:", 7))
         {
            char szRef2[500];

            ii = 0;
            while (pDb.szHdr[i + 7 + ii] != '|'
                   && pDb.szHdr[i + 7 + ii] != ' ')
            {
               szRef[ii] = pDb.szHdr[i + 7 + ii];
               ii++;
            }

            szRef[ii] = '\0';
            i += 7 + (int)strlen(szRef);

            strcpy(szRef2, szRef);
            if ((pStr = strchr(szRef2, ';')))
               *pStr = '\0';

            printf("TREMBL:<A HREF=\"http://www.uniprot.org/uniprot/%s\">%s</A> %c",
                szRef2, szRef, pDb.szHdr[i]);
         }


	 // REFSEQ_NP section
         else if (!strncmp(pDb.szHdr + i, "REFSEQ_NP:", 10)) {
	   printf("REFSEQ_NP:");
	   
	   // JMT 5/2/06 modifications:
	   // 
	   // if there are two refseq entries, separated by a semicolon,
	   // display these as two separate links
	   //
	   // also, adding lots and lots of comments so i can figure this out 
	   
	   // pDb.szHdr is the big database string that we're parsing out
	   // i is the cursor to the beginning of a new section in the db string, ex: REFSEQ_NP
	   
	   // we'll parse the db string character by character;
	   // curCursor will track the current postion.

	   int curCursor = i + 10; // i + 10 now gets us past the characters in "REFSEQ_NP:"
	   
	   // ii is the offset into the temp. destination buffer
	   // curCursor is the offset into the big db string of the
	   // refseq we're copying out

	   // we're copying the refseq into the (temp) szRef buffer

	   while ( (pDb.szHdr[curCursor] != '|')
		   && 
		   (pDb.szHdr[curCursor] != ' ') ) {
	     // this means we're still somewhere in the refseq data

	     // add an inner loop: if we detect a semicolon, this means we start a new refseq
	     ii = 0;  // reset temp buffer offset
	     //printf(" =starting new ref %c= ", pDb.szHdr[curCursor]);
	     while ((pDb.szHdr[curCursor] != '|')
		    && 
		    (pDb.szHdr[curCursor] != ' ')
		    &&
		    (pDb.szHdr[curCursor] != ';') ) {
	       //printf(" ^^%c^^ ", pDb.szHdr[curCursor]);
	       szRef[ii] = pDb.szHdr[curCursor];
	       curCursor++;
	       ii++;
	     }
	     // we either broke out due to end of one refseq, or end of all refseqs

	     // null term the buffer c-string
	     szRef[ii] = '\0';

	     // the current db string cursor will now point to the separator: '|' OR ';'	     
	     printf("<A HREF=\"http://www.ncbi.nlm.nih.gov/sites/entrez/?db=gene&cmd=Search&term=%s\">%s</A> %c",
		    szRef, szRef, pDb.szHdr[curCursor]);
	     
	     //printf(" ==%c%c%c== ",pDb.szHdr[i - 1],pDb.szHdr[i ],pDb.szHdr[i +1 ] );
	     
	     if (pDb.szHdr[curCursor] == ';') {
	       // if we stopped because there's another refseq, so advance the cursor
	       //printf(" **;** ");
	       curCursor++;
	     }

	   }
	   //printf(" done ");
	   // remember, "i" is the cursor into the db string used by everyone else,
	   // so keep it up to date
	   i = curCursor; 
	   //printf(" **%c** ", pDb.szHdr[i] );

	 }
	 // end REFSEQ_NP section


	 // REFSEQ_XP section
         else if (!strncmp(pDb.szHdr + i, "REFSEQ_XP:", 10)) {

	   // horrible, isn't it?  just copying the code from REFSEQ_NP
	   // see there for more comments
	   // JMT 5/5/06
	   
	   printf("REFSEQ_XP:");
	   int curCursor = i + 10; // i + 10 now gets us past the characters in "REFSEQ_NP:"
	   
	   while ( (pDb.szHdr[curCursor] != '|')
		   && 
		   (pDb.szHdr[curCursor] != ' ') ) {
	     ii = 0;  // reset temp buffer offset

	     while ((pDb.szHdr[curCursor] != '|')
		    && 
		    (pDb.szHdr[curCursor] != ' ')
		    &&
		    (pDb.szHdr[curCursor] != ';') ) {

	       szRef[ii] = pDb.szHdr[curCursor];
	       curCursor++;
	       ii++;
	     }
	     szRef[ii] = '\0';

	     printf("<A HREF=\"http://www.ncbi.nlm.nih.gov/sites/entrez?db=gene&cmd=Search&term=%s\">%s</A> %c",
		    szRef, szRef, pDb.szHdr[curCursor]);
	     
	     if (pDb.szHdr[curCursor] == ';') {
	       curCursor++;
	     }

	   }
	   i = curCursor; 
	 }
	 // end REFSEQ_XP section


	 // REFSEQ section
	 // JMT: added to handle plain "REFSEQ" fields in newer IPI dbases
         else if (!strncmp(pDb.szHdr + i, "REFSEQ:", 7)) {
	   printf("REFSEQ:");
	   
	   // copied from REFSEQ_NP

	   // JMT 5/17/06 modifications:
	   // 
	   // if there are two refseq entries, separated by a semicolon,
	   // display these as two separate links
	   //
	   // also, adding lots and lots of comments so i can figure this out 
	   
	   // pDb.szHdr is the big database string that we're parsing out
	   // i is the cursor to the beginning of a new section in the db string, ex: REFSEQ_NP
	   
	   // we'll parse the db string character by character;
	   // curCursor will track the current postion.

	   int curCursor = i + 7; // i + 7 now gets us past the characters in "REFSEQ_NP:"
	   
	   // ii is the offset into the temp. destination buffer
	   // curCursor is the offset into the big db string of the
	   // refseq we're copying out

	   // we're copying the refseq into the (temp) szRef buffer

	   while ( (pDb.szHdr[curCursor] != '|')
		   && 
		   (pDb.szHdr[curCursor] != ' ') ) {
	     // this means we're still somewhere in the refseq data

	     // add an inner loop: if we detect a semicolon, this means we start a new refseq
	     ii = 0;  // reset temp buffer offset
	     //printf(" =starting new ref %c= ", pDb.szHdr[curCursor]);
	     while ((pDb.szHdr[curCursor] != '|')
		    && 
		    (pDb.szHdr[curCursor] != ' ')
		    &&
		    (pDb.szHdr[curCursor] != ';') ) {
	       //printf(" ^^%c^^ ", pDb.szHdr[curCursor]);
	       szRef[ii] = pDb.szHdr[curCursor];
	       curCursor++;
	       ii++;
	     }
	     // we either broke out due to end of one refseq, or end of all refseqs

	     // null term the buffer c-string
	     szRef[ii] = '\0';

	     // the current db string cursor will now point to the separator: '|' OR ';'	     
	     printf("<A HREF=\"http://www.ncbi.nlm.nih.gov/sites/entrez?db=gene&cmd=Search&term=%s\">%s</A> %c",
		    szRef, szRef, pDb.szHdr[curCursor]);
	     
	     //printf(" ==%c%c%c== ",pDb.szHdr[i - 1],pDb.szHdr[i ],pDb.szHdr[i +1 ] );
	     
	     if (pDb.szHdr[curCursor] == ';') {
	       // if we stopped because there's another refseq, so advance the cursor
	       //printf(" **;** ");
	       curCursor++;
	     }

	   }
	   //printf(" done ");
	   // remember, "i" is the cursor into the db string used by everyone else,
	   // so keep it up to date
	   i = curCursor; 
	   //printf(" **%c** ", pDb.szHdr[i] );

	 }
	 // end REFSEQ section


         else if (!strncmp(pDb.szHdr + i, "LocusLink:", 10))
         {
            ii = 0;
            while (pDb.szHdr[i + 10 + ii] != '|'
                   && pDb.szHdr[i + 10 + ii] != ' ')
            {
               szRef[ii] = pDb.szHdr[i + 10 + ii];
               ii++;
            }
            szRef[ii] = '\0';
            i += 10 + (int)strlen(szRef);

            printf("LocusLink:<A HREF=\"http://www.ncbi.nlm.nih.gov/sites/entrez?db=gene&cmd=Search&term=%s\">%s</A> %c",
                szRef, szRef, pDb.szHdr[i]);
         }


	 // ENSEMBL section
         else if (!strncmp(pDb.szHdr + i, "ENSEMBL:", 8)) {
	   // yes, once again just copying code from REFSEQ_NP... terribly lazy here.
	   // see there for comments
	   printf("ENSEMBL:");
	   int curCursor = i + 8;
	   while ( (pDb.szHdr[curCursor] != '|')
		   && 
		   (pDb.szHdr[curCursor] != ' ') ) {
	     ii = 0;  // reset temp buffer offset
	     while ((pDb.szHdr[curCursor] != '|')
		    && 
		    (pDb.szHdr[curCursor] != ' ')
		    &&
		    (pDb.szHdr[curCursor] != ';') ) {
	       szRef[ii] = pDb.szHdr[curCursor];
	       curCursor++;
	       ii++;
	     }
	     // we either broke out due to end of one reference, or end of all 

	     // null term the buffer c-string
	     szRef[ii] = '\0';

	     // the current db string cursor will now point to the separator: '|' OR ';'	     
	     printf("<A HREF=\"http://www.ensembl.org/Homo_sapiens/Transcript/ProteinSummary?db=core;t=%s\">%s</A> %c",
		    szRef, szRef, pDb.szHdr[curCursor]);
	     
	     if (pDb.szHdr[curCursor] == ';') {
	       curCursor++;
	     }

	   }
	   //printf(" done ");
	   // remember, "i" is the cursor into the db string used by everyone else,
	   // so keep it up to date
	   i = curCursor; 
	   //printf(" **%c** ", pDb.szHdr[i] );
	 }
	 // end ENSEMBL section


         else
         {
            printf("%c", pDb.szHdr[i]);
         }
      }
   }
   else
   {
      printf("%s", pDb.szHdr);
   }
   printf("</tt></div>");


   pDb.szHdr[MAX_HEADER_LEN - 1] = '\0';
   sscanf(pDb.szHdr, "%s", szReference);

   pDb.dMW = pdMassAA['h'] + pdMassAA['h'] + pdMassAA['o'];
   for (i = 0; i < iLenSeq; i++)
   {
      pDb.dMW += pdMassAA[szSeq[i]];

      if (pDb.szHighLight[i] == '1'
          && (i == 0 || pDb.szHighLight[i - 1] == '0'))
	//printf("<font id=\"pos%i\" class=\"seq\">",i);
         printf("<font class=\"seq\">");
      if (pDb.szHighLight3[i] == '1'
          && (i == 0 || pDb.szHighLight3[i - 1] == '0'))
         printf("<font class=\"glyco\">");
      if (pDb.szHighLight2[i] == '1')
	  //          && (i == 0 || pDb.szHighLight2[i - 1] == '0'))
         printf("<font class=\"markAA\">");

      printf("%c", szSeq[i]);

      if (pDb.szHighLight2[i] == '1')
	  //          && (i + 1 == pDb.iLenSeq || pDb.szHighLight2[i + 1] == '0'))
         printf("</font>");
      if (pDb.szHighLight[i] == '1'
          && (i + 1 == pDb.iLenSeq || pDb.szHighLight[i + 1] == '0'))
         printf("</font>");
      if (pDb.szHighLight3[i] == '1'
          && (i + 1 == pDb.iLenSeq || pDb.szHighLight3[i + 1] == '0'))
         printf("</font>");

      if (!((i + 1) % 10))
         printf(" \n");
   }
   printf("</tt><br/>\n");

   dPI = COMPUTE_PI(szSeq, iLenSeq, 0);

   printf("<div align=\"right\" class=\"info\">\n");
   printf("%s MW: <b>%0.0f</b>, pI: <b>%0.2f</b><br/>\n", (pOptions.bMonoMass == 0 ? "AVG" : "MONO"), pDb.dMW, dPI);

#ifdef WINDOWS_CYGWIN
   FILE* fp;
   char szCommand[SIZE_BUF];
   sprintf(szCommand, "cygpath -w '%s'", pOptions.szDbFile);
   if((fp = popen(szCommand, "r")) == NULL) {
     prntMsg("cygpath error, exiting");
     prntPageFooter();
     exit(1);
   }
   else {
     fgets(szBuf, SIZE_BUF, fp);
     pclose(fp);
     szBuf[strlen(szBuf)-1] = 0;
   }
   printf("Database = <b>%s</b><br/>\n", szBuf);
#else
   printf("Database = <b>%s</b><br/>\n", pOptions.szDbFile);
#endif

   if(pOptions.iMinNTT && enzyme_digest != NULL)
     printf("Sample Enzyme = <b>%s</b>, Min NTT  = <b>%d</b><br/>\n", pOptions.szEnzyme, pOptions.iMinNTT);


   // links...  LM: why print them only when iNumPeptides > 0?
   if (pOptions.iNumPeptides > 0) {
     char szTmpReference[MAX_HEADER_LEN];

     if (!strncmp(szReference, "SW:", 3))
       strcpy(szTmpReference, szReference + 3);
     else if (!strncmp(szReference, "SWN:", 4))
       strcpy(szTmpReference, szReference + 4);
     else if (!strncmp(szReference, "GP:", 3))
       strcpy(szTmpReference, szReference + 3);
     else if (!strncmp(szReference, "GPN:", 4))
       strcpy(szTmpReference, szReference + 4);
     else if (!strncmp(szReference, "PIR1:", 5))
       strcpy(szTmpReference, szReference + 5);
     else if (!strncmp(szReference, "PIR2:", 5))
       strcpy(szTmpReference, szReference + 5);
     else if (!strncmp(szReference, "PIR3:", 5))
       strcpy(szTmpReference, szReference + 5);
     else if (!strncmp(szReference, "PIR4:", 5))
       strcpy(szTmpReference, szReference + 5);
     else
       strcpy(szTmpReference, szReference);

     // jmt: added google link
     printf("Links: ");
     printf("<A HREF=\"http://www.google.com/search?q=%s\">Google</A> ", szTmpReference);
     printf("<A HREF=\"http://scholar.google.com/scholar?q=%s\">Google Scholar</A> ", szTmpReference);
     printf("<A HREF=\"http://www.ncbi.nlm.nih.gov/sites/entrez?db=gene&cmd=Search&term=%s\">NCBI</A> ", szTmpReference);
     printf("<A HREF=\"http://www.uniprot.org/uniprot/?query=%s\">UniProtKB</A> ", szTmpReference);
     //printf("<A HREF=\"http://srs.ebi.ac.uk/srs6bin/cgi-bin/wgetz?-newId+-lv+30+-view+SeqSimpleView+[SWALL-AllText:%s*]\">EBI</A> ", szTmpReference);
     //printf("<A HREF=\"http://pir.georgetown.edu/cgi-bin/pirsearch?query=%s\">PIR</A> ", szTmpReference);
     //printf("<A HREF=\"http://vms.mips.biochem.mpg.de/htbin/search_code/%s\">MIPs</A> ", szTmpReference);
     //printf("<A HREF=\"http://genome-www4.stanford.edu/cgi-bin/SGD/locus.pl?locus=%s\">SGD</A> ", szTmpReference);
   }
   printf("</div>\n\n</div>\n\n");

   // Observed Peptides pane
   if (pOptions.iNumPeptides > 0)
   {
      double dMatchedMass = 0, dTotMass = 0;
      int  iMatchedAACount = 0, iTotAACount = 0;

      printf("<br/>\n\n");
      printf("<table cellspacing=\"0\">\n");
      printf("<tr>\n");
      printf("<td class=\"banner_cid\">&nbsp;&nbsp;Observed Peptides&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>\n");
      printf("</tr>\n");
      printf("</table>\n");

      printf("<div class=\"formentry\">\n");
      printf("<table cellpadding=\"2\">\n");
      printf("<tr><th class=\"nav\">Position</th><th class=\"nav\">Mass</th><th class=\"nav\">Peptide</th></tr>\n");

      for (ii = 0; ii < iLenSeq; ii++)
      {
         for (i = 0; i < pOptions.iNumPeptides; i++)
         {
            int  iLenPeptide = (int)strlen(pOptions.szPeptide[i]);

            if (!strncmp(szSeq + ii, pOptions.szPeptide[i], iLenPeptide))
            {
	       printf("<tr>\n");
	       //printf("<tr onmouseover=\"hilight('pos%i','yes')\" onmouseout=\"hilight('pos%i','no')\">\n",ii, ii);
	       printf("<td align=\"center\"><tt>%d-%d</tt></td>\n", ii + 1, ii + iLenPeptide);
               printf("<td align=\"right\"><tt>%0.2f</tt></td>\n", PEPMASS(pOptions.szPeptide[i]));
               printf("<td><tt><A HREF=\"%s%s\">", szPeptideLink, pOptions.szPeptide[i]);
               printf("%s", pOptions.szPeptide[i]);
               printf("</a></tt></td> ");
               printf("<!-- PEPSEQ: %s -->\n</tr>\n", pOptions.szPeptide[i]);
            }
         }
      }
      printf("</table>\n");


      printf("<div align=\"right\" class=\"info\">");
      dTotMass = PEPMASS(szSeq);

      iTotAACount = iLenSeq;
      for (i = 0; i < iLenSeq; i++)
      {
         if (pDb.szHighLight[i] == '1')
         {
            iMatchedAACount++;
            dMatchedMass += pdMassAA[szSeq[i]];
         }
      }
      dMatchedMass += pdMassAA['h'] + pdMassAA['h'] + pdMassAA['o'];

      printf("<b>Coverage:  AA %0.1f%%</b> (%d / %d residues)   <b>Mass %0.1f%%</b> (%d / %d Da)</div>\n",
          100.0 * ((double) iMatchedAACount / iTotAACount), iMatchedAACount,
          iTotAACount, 100.0 * dMatchedMass / dTotMass,
          (int) (dMatchedMass + 0.5), (int) (dTotMass + 0.5));

      printf("</div>\n\n");
   }

   // In-silico Digestion pane
   printf("<!-- In-silico Digestion pane -->\n");
   printf("<br/>\n\n");
   printf("<table cellspacing=\"0\">\n");
   printf("<tr>\n");
   printf("<td class=\"banner_cid\">&nbsp;&nbsp;In-silico Digestion&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>\n");
   printf("</tr>\n");
   printf("</table>\n");

   printf("<div class=\"formentry\">\n");
   //   printf("<table cellpadding=\"2\">\n");

   printf("<table cellspacing=\"0\">\n");
   printf("<tr valign=\"top\"><td>\n");

   /*
    * Digest protein
    */
   if (pOptions.iEnzymeKey > 0)
   {
      DIGEST_PROTEIN(szSeq, iLenSeq);
   }
   printf("</td><td>\n");

   // form
   printf("<form action=\"%s\" method=\"get\">\n", getenv("SCRIPT_NAME"));
   printf("<input type=\"hidden\" name=\"Ref\" value=\"%s\">\n", szReference);
   for (int j = 0; j < pOptions.iNumPeptides; j++)
     printf("<input type=\"hidden\" name=\"Pep\" value=\"%s\">\n", pOptions.szPeptide[j]);
   if ( (pOptions.iNumPeptides > 0) && pOptions.bNucleotideDb )
     {
       printf("<input type=\"hidden\" name=\"NucDb\" value=\"1\">\n");
       printf("<input type=\"hidden\" name=\"ReadingFrame\" value=\"%d\">\n", iReadingFrame);
     }
   printf("<input type=\"hidden\" name=\"Db\" value=\"%s\">\n", pOptions.szDbFile);
   printf("<input type=\"hidden\" name=\"sample_enzyme\" value=\"%s\">\n", pOptions.szEnzyme); // in case of custom
   printf("<table class=\"info\" style=\"border: 2px solid black;\">\n");
   printf("<tr><th class=\"graybox\" colspan=\"2\">\n");
   printf("Digest this protein:</th></tr>\n");

   printf("<tr><th align=\"right\">Enzyme/NMC:</td>\n");
   printf("    <td><select name=\"EnzymeKey\">\n");
   printf("    <option value=\"1\"%s>Trypsin</option>\n", ((! pOptions.iEnzymeKey && ! strcmp(pOptions.szEnzyme, "Trypsin")) ||
							 pOptions.iEnzymeKey == 1 ? " SELECTED" : ""));
   printf("    <option value=\"2\"%s>ChymoTrypsin</option>\n", ((! pOptions.iEnzymeKey && ! strcmp(pOptions.szEnzyme, "Chymotrypsin")) ||
							      pOptions.iEnzymeKey == 2 ? " SELECTED" : ""));
   printf("    <option value=\"3\"%s>Clostripain</option>\n", (pOptions.iEnzymeKey == 3 ? " SELECTED" : ""));
   printf("    <option value=\"4\"%s>Cyanogen Bromide</option>\n", ((! pOptions.iEnzymeKey && ! strcmp(pOptions.szEnzyme, "CNBr")) ||
								  pOptions.iEnzymeKey == 4 ? " SELECTED" : ""));
   printf("    <option value=\"5\"%s>IodosoBenzoate</option>\n", (pOptions.iEnzymeKey == 5 ? " SELECTED" : ""));
   printf("    <option value=\"6\"%s>Proline Endopeptidase</option>\n", (pOptions.iEnzymeKey == 6 ? " SELECTED" : ""));
   printf("    <option value=\"7\"%s>Staph Protease</option>\n", (pOptions.iEnzymeKey == 7 ? " SELECTED" : ""));
   printf("    <option value=\"8\"%s>Trypsin K</option>\n", (pOptions.iEnzymeKey == 8 ? " SELECTED" : ""));
   printf("    <option value=\"9\"%s>Trypsin R</option>\n", (pOptions.iEnzymeKey == 9 ? " SELECTED" : ""));
   printf("    <option value=\"10\"%s>Asp N</option>\n", ((! pOptions.iEnzymeKey && ! strcmp(pOptions.szEnzyme, "AspN")) ||
							pOptions.iEnzymeKey == 10 ? " SELECTED" : ""));
   printf("    <option value=\"11\"%s>modified Chymotryp</option>\n", (pOptions.iEnzymeKey == 11 ? " SELECTED" : ""));
   printf("    <option value=\"12\"%s>Elastase</option>\n", ((! pOptions.iEnzymeKey && ! strcmp(pOptions.szEnzyme, "Elastase")) ||
							   pOptions.iEnzymeKey == 12 ? " SELECTED" : ""));
   printf("    <option value=\"13\"%s>Elast/Tryp/Chymo</option>\n", (pOptions.iEnzymeKey == 13 ? " SELECTED" : ""));
   printf("    </select>\n");
   printf(" / \n");
   printf("    <select name=\"MissedCleave\">\n");
   printf("    <option value=\"0\"%s>0</option>\n", ((! pOptions.iEnzymeKey && pOptions.iMinNTT == 0) ||
						   pOptions.iMissedCleavage == 0 ? " SELECTED" : ""));
   printf("    <option value=\"1\"%s>1</option>\n", ((! pOptions.iEnzymeKey && pOptions.iMinNTT == 1) ||
						   pOptions.iMissedCleavage == 1 ? " SELECTED" : ""));
   printf("    <option value=\"2\"%s>2</option>\n", ((! pOptions.iEnzymeKey && pOptions.iMinNTT == 2) ||
						   pOptions.iMissedCleavage == 2 ? " SELECTED" : ""));
   printf("    </select></td>\n</tr>\n");

   printf("<tr><th align=\"right\">Sort by:</td>\n");
   printf("    <td><input type=\"radio\" name=\"SortBy\" value=\"0\"%s>Pos\n", (pOptions.iSortBy == 0 ? " CHECKED" : ""));
   printf("        <input type=\"radio\" name=\"SortBy\" value=\"1\"%s>Mass\n", (pOptions.iSortBy == 1 ? " CHECKED" : ""));
   printf("        <input type=\"radio\" name=\"SortBy\" value=\"3\"%s>pI\n", (pOptions.iSortBy == 3 ? " CHECKED" : ""));
   printf("        <input type=\"radio\" name=\"SortBy\" value=\"2\"%s>Seq\n", (pOptions.iSortBy == 2 ? " CHECKED" : ""));
   printf("    </td>\n</tr>\n");

   printf("<tr><th align=\"right\">Cysteines:</td>\n");
   printf("    <td><select name=\"IcatCys\">\n");
   printf("    <option value=\"0\"%s>cys unmodified</option>\n", (pOptions.iModifiedCys == 0 ? " SELECTED" : ""));
   printf("    <option value=\"3\"%s>iodoacetamide</option>\n", (pOptions.iModifiedCys == 3 ? " SELECTED" : ""));
   printf("    <option value=\"1\"%s>orig ICAT</option>\n", (pOptions.iModifiedCys == 1 ? " SELECTED" : ""));
   printf("    <option value=\"2\"%s>acid cleavable ICAT</option>\n", (pOptions.iModifiedCys == 2 ? " SELECTED" : ""));
   printf("    </select></td>\n</tr>\n");

   printf("<tr><th align=\"right\">Display:</td>\n");
   printf("    <td>\n");
   printf("    InclAA<input type=\"text\" size=\"3\" name=\"InclAA\" value=\"%s\">\n", pOptions.szInclAA);
   printf("    MarkAA<input type=\"text\" size=\"3\" name=\"MarkAA\" value=\"%s\">\n", pOptions.szMarkAA);
   printf("    NxS/T<input type=\"checkbox\" name=\"N-Glyco\" value=\"1\"%s>\n", (pOptions.bMarkNXST == 1 ? " CHECKED" : ""));
   printf("    </td>\n</tr>\n");

   printf("<tr><th align=\"right\">Mass Range:</td>\n");
   printf("    <td>\n");
   printf("    <input type=\"text\" size=\"5\" name=\"MinMass\" value=\"%0.1f\">--\n", pOptions.dMinMass);
   printf("    <input type=\"text\" size=\"5\" name=\"MaxMass\" value=\"%0.1f\">\n", pOptions.dMaxMass);
   printf("    </td>\n</tr>\n");

   printf("<tr><th align=\"right\">Mass Type:</td>\n");
   printf("    <td>\n");
   printf("        <input type=\"radio\" name=\"MassType\" value=\"1\"%s>Mono\n", (pOptions.bMonoMass == 1 ? " CHECKED" : ""));
   printf("        <input type=\"radio\" name=\"MassType\" value=\"0\"%s>Avg\n", (pOptions.bMonoMass == 0 ? " CHECKED" : ""));
   printf("    </td>\n</tr>\n");


   printf("<tr><td class=\"graybox\" colspan=\"2\" align=\"right\"><input value=\"GO\" type=\"submit\"></td></tr>\n");
   printf("</table>\n");
   printf("</form>\n");

   printf("</td></tr></table>\n");
   printf("</div>\n<br/>\n");
   printf("<!-- End in-silico Digestion pane -->\n");

   printf("<hr noshade/><br/>\n\n");
   fflush(stdout);

} /*PRINT_PROTEIN */


void DIGEST_PROTEIN(char *szSeq,
       int iLenSeq)
{
   int  i,
      iStart,
      iEnd,
      iMissed,
      iNextStart, iNumDigestedPeptides = 0, iNumMatchedDigestedPeptides = 0;
   double dTerminusMass;
   char szBuf[SIZE_BUF], tmpMsg[500],
      szOutFile[SIZE_FILE];
   FILE *fp;

   memset(pDb.szHighLight, '0', iLenSeq);   /* used to define digest point */
   pDb.szHighLight[iLenSeq] = '\0';

   switch (pOptions.iEnzymeKey)
   {
      case 1:
         sprintf(szBuf, "1 KR P");
         break;                 /*Trypsin */
      case 2:
         sprintf(szBuf, "1 FWY P");
         break;                 /*Chymotrypsin */
      case 3:
         sprintf(szBuf, "1 R -");
         break;                 /*Clostripain */
      case 4:
         sprintf(szBuf, "1 M -");
         break;                 /*Cyanogen_Bromide */
      case 5:
         sprintf(szBuf, "1 W -");
         break;                 /*IodosoBenzoate */
      case 6:
         sprintf(szBuf, "1 P -");
         break;                 /*Proline_Endopept */
      case 7:
         sprintf(szBuf, "1 E -");
         break;                 /*Staph_Protease */
      case 8:
         sprintf(szBuf, "1 K P");
         break;                 /*Trypsin_K */
      case 9:
         sprintf(szBuf, "1 R P");
         break;                 /*Trypsin_R */
      case 10:
         sprintf(szBuf, "0 D -");
         break;                 /*AspN */
      case 11:
         sprintf(szBuf, "1 FWYL P");
         break;                 /*Chymotrypsin modified */
      case 12:
         sprintf(szBuf, "1 ALIV P");
         break;                 /*Elastase */
      case 13:
         sprintf(szBuf, "1 ALIVKRWFY P");
         break;                 /*Elastate-Tryp-Chymo */
   }

   sscanf(szBuf, "%d %s %s", &(pOptions.iTerm), pOptions.szEnzymeBreak,
          pOptions.szEnzymeNoBreak);

   /*
    * assign all possible enzyme break points
    */
   iStart = 0;
   iEnd = 0;
   iMissed = 0;
   iNextStart = 0;
   dTerminusMass = pdMassAA['h'] + pdMassAA['h'] + pdMassAA['h'] + pdMassAA['o'];

   srand(iLenSeq - pOptions.iNumPeptides + pOptions.iNumReferences);
   // do this in designated tmp dir, if any
   sprintf(szOutFile, "%scometdb.tmp.XXXXXX", getWebserverTmpPath()?getWebserverTmpPath():"");

   if ((fp = FILE_mkstemp(szOutFile)) == NULL)
   {
      sprintf(tmpMsg," Error - cannot write to temp file <b>%s</b>", szOutFile);
      prntMsg(tmpMsg);
      prntMsg(" Skipping digesting protein.");
      prntPageFooter();
      exit(EXIT_FAILURE);
   }

   /*
    * generate enzyme-specific peptides
    */
   do
   {
      if ((strchr (pOptions.szEnzymeBreak, szSeq[iEnd + 1 - pOptions.iTerm])
           && !strchr(pOptions.szEnzymeNoBreak, szSeq[iEnd + 2 - pOptions.iTerm]))
          || iEnd == iLenSeq - 1)
      {
         int  i, bPass = TRUE;
         char szPeptide[MAX_PEPTIDE_LEN];
         double dCalcMass=0;

         if (iEnd - iStart + 2 < MAX_PEPTIDE_LEN)
         {
            dCalcMass = dTerminusMass;

            for (i = iStart; i <= iEnd; i++)
            {
               dCalcMass += pdMassAA[szSeq[i]];
            }
            strncpy(szPeptide, szSeq + iStart, iEnd - iStart + 1);
            szPeptide[iEnd - iStart + 1] = '\0';

            /* check mass range */
            if (pOptions.dMinMass > dCalcMass
                || dCalcMass > pOptions.dMaxMass)
               bPass = FALSE;

            /* check specified AA */
            if (bPass && strlen(pOptions.szInclAA) > 0)
            {
               int  i, iLen;

               bPass = FALSE;

               iLen = (int)strlen(pOptions.szInclAA);
               for (i = 0; i < iLen; i++)
               {
                  if (strchr(szPeptide, pOptions.szInclAA[i]))
                  {
                     bPass = TRUE;
                     break;
                  }
               }
            }

            if (bPass)
            {
               double dPI = COMPUTE_PI(szPeptide, (int)strlen(szPeptide), 0);

               fprintf(fp, "%d	%d	%f	%f	%s\n",
                       iStart + 1, iEnd + 1, dCalcMass, dPI, szPeptide);
            }

         }

         iMissed++;
         if (iMissed == 1)      /* first break point is start of next peptide */
            iNextStart = iEnd + 1;

         if (iMissed <= pOptions.iMissedCleavage
             && dCalcMass < pOptions.dMaxMass && iEnd < iLenSeq - 1)
         {
            iEnd++;
         }
         else
         {
            iMissed = 0;
            iStart = iNextStart;
            iEnd = iStart;
         }
      }
      else
      {
         iEnd++;
      }

   }
   while (iStart < iLenSeq);

   fclose(fp);
   if (pOptions.iSortBy == 0)
   {
      /* already sorted by position */
   }
   else if (pOptions.iSortBy == 1)      /*mass */
   {
	   sortcols(szOutFile,3,true);
   }
   else if (pOptions.iSortBy == 2)      /*seq */
   {
	   sortcols(szOutFile,5,false);
   }
   else                         /* pOptions.iSortBy==3  pI */
   {
	   sortcols(szOutFile,4,true);
   }

   if ((fp = fopen(szOutFile, "r")) == NULL)
   {
      sprintf(tmpMsg," Error - cannot read file <b>%s</b>", szOutFile);
      prntMsg(tmpMsg);
      prntMsg(" Skipping digesting protein.");
      prntPageFooter();
      exit(EXIT_FAILURE);
   }

   printf("<table cellpadding=\"2\">\n");
   printf("<tr>\n");
   printf("  <th class=\"%s\">Position</th>\n", (pOptions.iSortBy == 0 ? "graybox" : "nav") );
   if (pOptions.iModifiedCys == 0 || pOptions.iModifiedCys==3)
      printf("  <th class=\"%s\">Mass</th>\n", (pOptions.iSortBy == 1 ? "graybox" : "nav") );
   else
   {
      printf("  <th class=\"%s\">Light</th>\n", (pOptions.iSortBy == 1 ? "graybox" : "nav") );
      printf("  <th class=\"nav\">Heavy</th>\n");
   }
   printf("  <th class=\"%s\">pI</th>\n", (pOptions.iSortBy == 3 ? "graybox" : "nav") );
   printf("  <th class=\"%s\" align=\"left\">Peptide</th>\n</tr>\n", (pOptions.iSortBy == 2 ? "graybox" : "nav") );

   while (fgets(szBuf, SIZE_BUF, fp))
   {
      double dCalcMass, dPI;
      char szPeptide[MAX_PEPTIDE_LEN];
      int  bEndTag = FALSE;

      sscanf(szBuf, "%d	%d	%lf	%lf	%s\n",
             &iStart, &iEnd, &dCalcMass, &dPI, szPeptide);

      printf
         ("<tr align=\"right\"><td align=\"center\" nowrap><tt>%d-%d</tt></td>",
          iStart, iEnd);
      if (pOptions.iModifiedCys == 0 || pOptions.iModifiedCys==3)
         printf("<td><tt>%0.4f</tt></td>", dCalcMass);
      else
      {
         int  i, iCysCount = 0, iLen = (int)strlen(szPeptide);
         double dAddMass = 0.0;

         for (i = 0; i < iLen; i++)
         {
            if (szPeptide[i] == 'C')
               iCysCount++;
         }

         if (pOptions.iModifiedCys == 1)
            dAddMass = (double) iCysCount *8.0;
         else if (pOptions.iModifiedCys == 2)
            dAddMass = (double) iCysCount *9.0;

         printf("<td><tt>%0.2f</tt></td>", dCalcMass);
         printf("<td><tt>%0.2f</tt></td>", dCalcMass + dAddMass);
      }
      printf("<td><tt>%0.2f</tt></td>", dPI);

      printf("<td align=\"left\"><tt>");


      iNumDigestedPeptides++;

      for (i = 0; i < pOptions.iNumPeptides; i++)
      {
         unsigned int  iLen = (int)strlen(szPeptide);

         if (iLen == strlen(pOptions.szPeptide[i])
             && !strcmp(szPeptide, pOptions.szPeptide[i]))
         {
            printf(" <font class=\"seq\" >");
            bEndTag = TRUE;
            iNumMatchedDigestedPeptides++;

            break;
         }
      }

      if (strlen(pOptions.szMarkAA) == 0 && !(pOptions.bMarkNXST))
      {
         printf("%s", szPeptide);
      }
      else
      {
         int  iLen = (int)strlen(szPeptide);

         for (i = 0; i < iLen; i++)
         {
            if (pOptions.bMarkNXST
                && szPeptide[i] == 'N'
                && szPeptide[i + 1] != 'P'
                && (szPeptide[i + 2] == 'S' || szPeptide[i + 2] == 'T'))
            {
               printf("<font class=\"glyco\">%c%c%c</font>",
                      szPeptide[i], szPeptide[i + 1], szPeptide[i + 2]);
               i += 2;
            }
            else if (strchr(pOptions.szMarkAA, szPeptide[i]))
            {
               printf("<font class=\"markAA\">%c</font>", szPeptide[i]);
            }
            else
            {
               printf("%c", szPeptide[i]);
            }
         }
      }

      if (bEndTag)
      {
         printf("</font>");
      }

      printf("</tt></td></tr>\n");

      fflush(stdout);
   }

   unlink(szOutFile);

   printf("</td></tr></table>\n");
   if (iNumDigestedPeptides > 0)
      printf("<div class=\"info\">Matched %d of %d digested peptides (%0.1f%%).</div>\n\n",
             iNumMatchedDigestedPeptides,
             iNumDigestedPeptides,
             100.0 * (double) iNumMatchedDigestedPeptides /
             iNumDigestedPeptides);

} /*DIGEST_PROTEIN */


double PEPMASS(char *szPeptide)
{
   int  i, iLen;
   double dNterm = pdMassAA['h'], dCterm = pdMassAA['o'] + pdMassAA['h'];
   double dPepMass = 0;

   dPepMass = dNterm + dCterm;
   iLen = (int)strlen(szPeptide);

   for (i = 0; i < iLen; i++)
      if (isalpha(szPeptide[i]))
         dPepMass += pdMassAA[szPeptide[i]];

   return (dPepMass);

} /*PEPMASS*/


/*
int SORT_LABELIONS_MASS(const void *p0, const void *p1)
{
   if ( ((struct LabelIonsStruct *) p0)->dMass < ((struct LabelIonsStruct *) p1)->dMass )
      return (1);
   else if ( ((struct LabelIonsStruct *) p0)->dMass > ((struct LabelIonsStruct *) p1)->dMass )
      return (-1);
   else
      return (0);

}


int SORT_LABELIONS_INTEN(const void *p0, const void *p1)
{
   if ( ((struct LabelIonsStruct *) p0)->dIntensity < ((struct LabelIonsStruct *) p1)->dIntensity )
      return (1);
   else if ( ((struct LabelIonsStruct *) p0)->dIntensity > ((struct LabelIonsStruct *) p1)->dIntensity)
      return (-1);
   else
      return (0);

}
*/


// given enzyme, prev aa, peptide, next aa, and Boolean nterm for whether or not peptide begins at position 2 (i.e.
// protein's N-terminal M is the prev aa
/*
int numTolerableTermini2(char *enzyme,
   char prev,
   char *peptide,
   char next,
   int nterm)
{

   int  ntt = 0;

   if (peptide == NULL)
      return 0;
   // use max
   if (strcmp(enzyme, "tca") == 0)
   {
      int  next = numTolerableTermini2("tryptic", prev, peptide, next, nterm);
      ntt = next;
      next = numTolerableTermini2("chymotryptic", prev, peptide, next, nterm);
      if (next > ntt)
         ntt = next;
      next = numTolerableTermini2("AspN", prev, peptide, next, nterm);
      if (next > ntt)
         ntt = next;
      return ntt;
   }
   if (isTolerableTerminus(enzyme, prev) == -1)
      return 2;                 // unknown enzyme
   if (strcmp(enzyme, "AspN") == 0)
   {
      if ((nterm || prev == '-')
          || (strlen(peptide) > 2 && isTolerableTerminus(enzyme, peptide[0])
              && peptide[1] != 'P'))
         ntt++;
      if (next == '-'
          || isTolerableTerminus(enzyme, peptide[strlen(peptide) - 1]))
         ntt++;
      return ntt;
   }
   else
   {
      if ((nterm || prev == '-')
          || (strlen(peptide) > 0 && isTolerableTerminus(enzyme, prev)
              && peptide[0] != 'P'))
         ntt++;
      if (next == '-'
          || (isTolerableTerminus(enzyme, peptide[strlen(peptide) - 1])
              && next != 'P'))
         ntt++;
      return ntt;
   }
   return 2;                    // default if don't recognize enzyme
}
*/

/*
int isTolerableTerminus(char *enzyme,
   char aa)
{
   if (strcmp(enzyme, "tryptic") == 0)
      return (aa == 'K' || aa == 'R');
   if (strcmp(enzyme, "chymotryptic") == 0)
      return (aa == 'M' || aa == 'Y' || aa == 'F' || aa == 'W');
   if (strcmp(enzyme, "AspN") == 0)
      return (aa == 'D');
   if (strcmp(enzyme, "CNBr") == 0)
      return (aa == 'M');
   if (strcmp(enzyme, "gluC") == 0)
      return (aa == 'D' || aa == 'E');
   if (strcmp(enzyme, "gluC_bicarb") == 0)
      return (aa == 'E');
   if (strcmp(enzyme, "nonspecific") == 0)
      return 1;
   if (strcmp(enzyme, "tryptic/CNBr") == 0)
      return (aa == 'K' || aa == 'R' || aa == 'M');

   return -1;                   // default if don't recognize enzyme

}
*/



/*
 * Translation table:
 *
 *   ------------------------------------------ 
 *  |     ||   T    |    C   |    A   |    G   |  <- 2nd base
 *  |==========================================|
 *  |     || T  Phe |        | T  Tyr | T  Cys |
 *  |  T  ||_C______|   Ser  |_C______|_C______|
 * F|     || A  Leu |        | A Stop |_A_Stop_|
 * i|     || G      |        | G      | G  Trp |
 * r|------------------------------------------|
 * s|     ||        |        | T  His |        |
 * t|  C  ||   Leu  |   Pro  |_C______|   Arg  |
 *  |     ||        |        | A  Gln |        |
 * B|     ||        |        | G      |        |
 * a|------------------------------------------|
 * s|     || T  Ile |        | T  Asn | T  Ser |
 * e|  A  || C      |   Thr  |_C______|_C______|
 *  |     ||_A______|        | A  Lys | A  Arg |
 *  |     || G  Met |        | G      | G      |
 *  |------------------------------------------|
 *  |     ||        |        | T  Asp |        |
 *  |  G  ||   Val  |   Ala  |_C______|   Gly  |
 *  |     ||        |        | A  Glu |        |
 *  |     ||        |        | G      |        |
 *   ------------------------------------------ 
 *
 *  $ = stop
 *  ? = error/unknown
 */
void TRANSLATE(int iFrame,
      char *szNewSeq,
      char *szSeq,
      int  *iLenSeq)
{
   int i,
       iEnd= *iLenSeq -2,
       iStart=iFrame-1;
       
   *iLenSeq = 0;

   for (i=iStart; i<iEnd; i += 3)
   {
      int i1=i,
          i2=i+1,
          i3=i+2;

      if (szSeq[i1]=='T' || szSeq[i1]=='U')
      {
         if (szSeq[i2]=='T' || szSeq[i2]=='U')
         {
            if (szSeq[i3]=='T' || szSeq[i3]=='U' || szSeq[i3]=='C')
               szNewSeq[*iLenSeq]='F';
            else if (szSeq[i3]=='A' || szSeq[i3]=='G')
               szNewSeq[*iLenSeq]='L';
            else
               szNewSeq[*iLenSeq]='?';
         }
         else if (szSeq[i2]=='C')
            szNewSeq[*iLenSeq]='S';
         else if (szSeq[i2]=='A')
         {
            if (szSeq[i3]=='T' || szSeq[i3]=='U' || szSeq[i3]=='C')
               szNewSeq[*iLenSeq]='Y';
            else if (szSeq[i3]=='A' || szSeq[i3]=='G')
               szNewSeq[*iLenSeq]='$';
            else
               szNewSeq[*iLenSeq]='?';
         }
         else if (szSeq[i2]=='G')
         {
            if (szSeq[i3]=='T' || szSeq[i3]=='U' || szSeq[i3]=='C')
               szNewSeq[*iLenSeq]='C';
            else if (szSeq[i3]=='G')
               szNewSeq[*iLenSeq]='W';
            else if (szSeq[i3]=='A')
               szNewSeq[*iLenSeq]='$';
            else
               szNewSeq[*iLenSeq]='?';
         }
         else
            szNewSeq[*iLenSeq]='?';
      }
      else if (szSeq[i1]=='C')
      {
         if (szSeq[i2]=='T' || szSeq[i2]=='U')
            szNewSeq[*iLenSeq]='L';
         else if (szSeq[i2]=='C')
            szNewSeq[*iLenSeq]='P';
         else if (szSeq[i2]=='A')
         {
            if (szSeq[i3]=='T' || szSeq[i3]=='U' || szSeq[i3]=='C')
               szNewSeq[*iLenSeq]='H';
            else if (szSeq[i3]=='A' || szSeq[i3]=='G')
               szNewSeq[*iLenSeq]='Q';
            else
               szNewSeq[*iLenSeq]='?';
         }
         else if (szSeq[i2]=='G')
            szNewSeq[*iLenSeq]='R';
         else
            szNewSeq[*iLenSeq]='?';
      }
      else if (szSeq[i1]=='A')
      {
         if (szSeq[i2]=='T' || szSeq[i2]=='U')
         {
            if (szSeq[i3]=='T' || szSeq[i3]=='U' || szSeq[i3]=='C' || szSeq[i3]=='A')
               szNewSeq[*iLenSeq]='I';
            else if (szSeq[i3]=='G')
               szNewSeq[*iLenSeq]='M';
            else
               szNewSeq[*iLenSeq]='?';
         }
         else if (szSeq[i2]=='C')
            szNewSeq[*iLenSeq]='T';
         else if (szSeq[i2]=='A')
         {
            if (szSeq[i3]=='T' || szSeq[i3]=='U' || szSeq[i3]=='C')
               szNewSeq[*iLenSeq]='N';
            else if (szSeq[i3]=='A' || szSeq[i3]=='G')
               szNewSeq[*iLenSeq]='K';
            else
               szNewSeq[*iLenSeq]='?';
         }
         else if (szSeq[i2]=='G')
         {
            if (szSeq[i3]=='T' || szSeq[i3]=='U' || szSeq[i3]=='C')
               szNewSeq[*iLenSeq]='S';
            else if (szSeq[i3]=='A' || szSeq[i3]=='G')
               szNewSeq[*iLenSeq]='R';
            else
               szNewSeq[*iLenSeq]='?';
         }
         else
            szNewSeq[*iLenSeq]='?';
      }
      else if (szSeq[i1]=='G')
      {
         if (szSeq[i2]=='T' || szSeq[i2]=='U')
            szNewSeq[*iLenSeq]='V';
         else if (szSeq[i2]=='C')
            szNewSeq[*iLenSeq]='A';
         else if (szSeq[i2]=='A')
         {
            if (szSeq[i3]=='T' || szSeq[i3]=='U' || szSeq[i3]=='C')
               szNewSeq[*iLenSeq]='D';
            else if (szSeq[i3]=='A' || szSeq[i3]=='G')
               szNewSeq[*iLenSeq]='E';
            else
               szNewSeq[*iLenSeq]='?';
         }
         else if (szSeq[i2]=='G')
            szNewSeq[*iLenSeq]='G';
         else
            szNewSeq[*iLenSeq]='?';
      }
      else
         szNewSeq[*iLenSeq]='?';

      *iLenSeq += 1;
   }

} /*TRANSLATE*/


/************************************************************************
 *
 * Various html helper subs
 *
 ************************************************************************/
void prntMsg(const char *message)
{
  if (!inMsgPane) {
    printf("<table cellspacing=\"0\">\n");
    printf("<tbody>\n<tr>\n");
    printf("<td class=\"messages_h\">&nbsp;&nbsp;&nbsp;Messages&nbsp;&nbsp;&nbsp;</td>\n");
    printf("<td align=\"left\">&nbsp;&nbsp;&nbsp;&nbsp;<u><a onclick=\"showdiv('msgs','messages')\">[ Show | Hide ]</a></u></td>\n");
    printf("</tr></tbody>\n</table>\n");
    printf("<div id=\"msgs\" class=\"messages\">\n");
    printf("</div>\n<br/>\n");
    inMsgPane = 1;
  }

  printf("<SCRIPT LANGUAGE=\"JavaScript\" TYPE=\"text/javascript\">\n");
  printf("document.getElementById(\"msgs\").innerHTML += ");

  if (strlen(message) != 0)
    printf("\"<li>%s</li>\";\n",message);
  else 
    printf("\"<br/><br/>\";\n");

  printf("</SCRIPT>\n");

  return;
} /* prntMsg */


void prntPageFooter(void)
{
  printf("<!-- page footer -->\n");
  printf("<hr noshade/>\n");
  printf("<h6>%s by J.Eng (c) ISB 2001<br/>\n", TITLE);
  printf("(%s)</h6>\n", szTPPVersionInfo);
  printf("</body>\n</html>");

  return;
} /* prntPageFooter */

