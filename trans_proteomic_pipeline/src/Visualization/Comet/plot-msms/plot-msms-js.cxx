/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

/*
 * COMETPLOT by Jimmy Eng (c) Institute for Systems Biology, 2001
 * $Id$
 *
 * Purpose:  CGI program to plot MS/MS spectrum and associated masses or fragment ions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "common/constants.h"

#define PROTON_MASS 1.00727646688

#define TITLE "Lorikeet Spectrum Viewer"

#include "common/AminoAcidMasses.h"
#include "Visualization/Comet/Comet.h"
#include "common/TPPVersion.h" // contains version number, name, revision
#include "mzParser.h"

#include <sys/stat.h>
#include <ctype.h>
#include <math.h>


int inMsgPane = 0;
char tmpMsg[500];

double pdMassAA[256];

struct EnvironmentStruct
{
   int  iLenPeptide;
   int  iMassTypeFragment;
   int  iMassTypePrecursor;
   int  iWidth;
   int  iHeight;

   int  iScanStart;
   int  iScanEnd;
   int  iCharge;

   int  bLabelReporterIons;   // for iTraq/TMT

   char szInputFile[SIZE_FILE];
   char szFullPathInputFile[SIZE_FILE];
   char szTarFile[SIZE_FILE];
   char szMZXMLFile[SIZE_FILE];
   char szPeptide[MAX_PEPTIDE_LEN];

   double dPrecMz;
   double dPepMass;
   double pdModPeptide[MAX_PEPTIDE_LEN]; // just contains a list of modified masses at each peptide position
   double dModN;  // mass of new N
   double dModC;  // mass of new C
} pEnvironment;


static void EXTRACT_CGI_QUERY(void);
static void INITIALIZE(void);
static void PRINT_MS2_PEAKS(FILE *ppIn,
        RAMPFILE *fp_,
        int  iFileType,  // 0 = .dta file, 1 = read from .tgz file, 2 = read from mzXML file
        char *szPrintName,
        int  iCharge,
        ramp_fileoffset_t *index_,
        int iAnalysisLastScan);
void PRINT_MS1_PEAKS(FILE *ppIn,
        RAMPFILE *fp_,
        ramp_fileoffset_t *index_,
        int iAnalysisLastScan);
static int GET_SCANNUM_FROM_DTA_NAME(const char* dtaName);
void PRINT_MESSAGE(const char *message);
void PRINT_FOOTER(void);
bool HAS_ENDING (std::string const &fullStr, std::string const &ending);

#include "common/util.h"

int main(int argc, char **argv)
{
   FILE *ppIn;
   int  i,
        iFileType=0,  // 0 = .dta file, 1 = read from .tgz file, 2 = read from mzXML file
        iLen,
        iCharge=0,
        bVariableMod=0;
   struct stat statbuf;
   char szBaseName[SIZE_FILE],
        szPrintName[SIZE_FILE],
        szCommand[SIZE_BUF];
   char *pStr;
   RAMPFILE *fp_=NULL;
   hooks_tpp(argc,argv); // installdir issues etc

   // Print HTML header
   printf("Content-type: text/html\n\n");
   printf("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n");
   printf("<html>\n");
   printf(" <head>\n");
   printf("   <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n");
   printf("   <title>%s</title>\n",TITLE);
   printf("   <!--[if IE]><script language=\"javascript\" type=\"text/javascript\" src=\"%shtml/js/excanvas.min.js\"></script><![endif]-->\n", DEFAULT_HTML_DIR);
   printf("   <script type=\"text/javascript\" src=\"%sjs/jquery.min.js\"></script>\n", DEFAULT_HTML_DIR);
   printf("   <script type=\"text/javascript\" src=\"%sjs/jquery-ui.min.js\"></script>\n", DEFAULT_HTML_DIR);
   printf("   <script type=\"text/javascript\" src=\"%sjs/jquery.flot.js\"></script>\n", DEFAULT_HTML_DIR);
   printf("   <script type=\"text/javascript\" src=\"%sjs/jquery.flot.selection.js\"></script>\n", DEFAULT_HTML_DIR);
   printf("   <script type=\"text/javascript\" src=\"%sjs/specview.js\"></script>\n", DEFAULT_HTML_DIR);
   printf("   <script type=\"text/javascript\" src=\"%sjs/peptide.js\"></script>\n", DEFAULT_HTML_DIR);
   printf("   <script type=\"text/javascript\" src=\"%sjs/aminoacid.js\"></script>\n", DEFAULT_HTML_DIR);
   printf("   <script type=\"text/javascript\" src=\"%sjs/ion.js\"></script>\n", DEFAULT_HTML_DIR);
   printf("   <link REL=\"stylesheet\" TYPE=\"text/css\" HREF=\"%scss/lorikeet.css\">\n", DEFAULT_HTML_DIR);
   printf(" </head>\n");

   printf("<body BGCOLOR=\"%s\" OnLoad=\"self.focus();\">\n", "#C0C0C0");

   INITIALIZE();

   EXTRACT_CGI_QUERY();

   i=0;
   if (HAS_ENDING(pEnvironment.szInputFile, ".mzML"))
      i=4;
   else if (HAS_ENDING(pEnvironment.szInputFile, ".mzXML"))
      i=5;
   else if (HAS_ENDING(pEnvironment.szInputFile, ".mzData"))
      i=6;

   if (i>1)
   {
      strcpy(pEnvironment.szMZXMLFile, pEnvironment.szInputFile);
      pEnvironment.szInputFile[strlen(pEnvironment.szInputFile)-i]='\0';   // remove mzML/mzXML/mzData extension
      sprintf(tmpMsg, "%d.%d.%d.dta", pEnvironment.iScanStart, pEnvironment.iScanEnd, pEnvironment.iCharge );
      strcat(pEnvironment.szInputFile, tmpMsg);
   }

   // look for an X!Tandem style declaration
   for (const char *c=pEnvironment.szInputFile;c && *c && (c=strstr(c,"scan"));)
   {
      int iScanNum;
      if (2==sscanf(c++,"scan %d (charge %d", &iScanNum,&iCharge))
      {
         break;
      }
   }

   if (!iCharge)   // try the traditional encoding
      sscanf(pEnvironment.szInputFile+strlen(pEnvironment.szInputFile)-5, "%d", &iCharge);

   // small safeguard in case above parsing of charge state from .dta name fails
   if (iCharge<0 || iCharge>9)
      iCharge=1;

   // pEnvironment.szInputFile has input .dta file ... need to get this
   // from the tar/gzipped archive now
   //
   // So, first modify szTarFile to be the gzipped tar file name
   strcpy(pEnvironment.szTarFile, pEnvironment.szInputFile);
   iLen=strlen(pEnvironment.szTarFile);
   for (i=iLen-1; i>2; i--)
   {
      if (isPathSeperator(pEnvironment.szTarFile[i])
            && pEnvironment.szTarFile[i-1]=='.'
            && isPathSeperator(pEnvironment.szTarFile[i-2]))
      {
         pEnvironment.szTarFile[i-2]='\0';

         strcat(pEnvironment.szTarFile, ".tgz");
         break;
      }
         else if (isPathSeperator(pEnvironment.szTarFile[i])) // this case is needed for IE on the Mac ... /./ changes to just /
      {
         pEnvironment.szTarFile[i]='\0';

         strcat(pEnvironment.szTarFile, ".tgz");
         break;
      }
   }

   iLen=strlen(pEnvironment.szTarFile);
   strcpy(szBaseName, pEnvironment.szTarFile);

   szBaseName[strlen(szBaseName)-4]='\0';

   if (strlen(pEnvironment.szMZXMLFile) < 2) // do we already have a value here?
     rampConstructInputFileName(pEnvironment.szMZXMLFile, sizeof(pEnvironment.szMZXMLFile), szBaseName);
  
   if (!strstr(pEnvironment.szTarFile+iLen-4, ".tgz"))
   {
       sprintf(tmpMsg, "Error converting %s to tar file.", pEnvironment.szInputFile);
       PRINT_MESSAGE(tmpMsg);
       sprintf(tmpMsg,"szBaseName=%s", szBaseName);
       PRINT_MESSAGE(tmpMsg);
       sprintf(tmpMsg,"szTarFile=%s", pEnvironment.szTarFile);
       PRINT_MESSAGE(tmpMsg);
       sprintf(tmpMsg,"iLen=%d", iLen);
       PRINT_MESSAGE(tmpMsg);
       PRINT_FOOTER();
       exit(EXIT_FAILURE);
   }

   // Next, modify pEnvironment.szInputFile to remove full path
   strcpy(pEnvironment.szFullPathInputFile, pEnvironment.szInputFile);
   strcpy(pEnvironment.szInputFile, pEnvironment.szInputFile+i+1);
   sprintf(szCommand, "*%s", pEnvironment.szInputFile);
   strcpy(pEnvironment.szInputFile, szCommand);

   // handle searches which weren't done by runsearch, and so don't have tgz'd .out and .dta
   if (stat(pEnvironment.szTarFile,&statbuf)                 // does the tgz file exist?
         && stat(pEnvironment.szFullPathInputFile,&statbuf)) // does the .dta file exist as named in the command?
   { // no, try to remove that middle foo in wwwroot/foo/foo/foo.0001.0001.2.dta 
      char *slash;
      char szUntarredDTAfile[SIZE_BUF];

      strncpy(szUntarredDTAfile,pEnvironment.szFullPathInputFile,sizeof(szUntarredDTAfile));
      slash = strrchr(szUntarredDTAfile,'/');

      if (slash) 
      {
         char *slash2;
         *slash = 0;
         slash2 = strrchr(szUntarredDTAfile,'/');
         if (slash2)
         {
            strcpy(slash2+1,slash+1);
         }
      }
      if (!stat(szUntarredDTAfile,&statbuf))
      { // use this as the filename
         strncpy(pEnvironment.szFullPathInputFile,szUntarredDTAfile,sizeof(pEnvironment.szFullPathInputFile));
      }
   }

   iFileType = 0;

   // (jmt) The goal is to end up with a filepointer that will be
   // reading in dta-format data.  This filepointer may be a direct
   // fopen from an existing dta filename, a pipe-open from a
   // uncompressing a tgz file, or the result of the ramp parser
   // operating on an mzXML file.
   //
   // iFileType == 0: direct from dta file
   // iFileType == 1: from tgz archive
   // iFileType == 2: from mzXML via ramp

   // try to open the dta file directly, from the filesystem; then mzXML then .tgz
   if ((ppIn=fopen(pEnvironment.szFullPathInputFile, "r"))==NULL)
   {
      // printf("<b>not in archive!(%s == %d)</b><br />\n", szCommand, archiveResult);
      // try reading spectrum directly from a RAMP-supported mass spec format
      // whether sucessful or not

      fp_ = NULL;

      if ((fp_ = rampOpenFile(pEnvironment.szMZXMLFile)) != NULL)
      {
         iFileType = 2;   // success ... read from mzXML file
      }
      else
      {  // try adding the various RAMP-supported filename exts and see if we find a file
         const char **rampFileTypes = rampListSupportedFileTypes();
         while (*rampFileTypes)   // list of .exts is null terminated
         {
            strcpy(pEnvironment.szMZXMLFile, szBaseName);
            strcat(pEnvironment.szMZXMLFile, *rampFileTypes);
            if ((fp_ = rampOpenFile(pEnvironment.szMZXMLFile)) != NULL)
            {
               iFileType = 2;   // success ... read from mass spec file
               break;
            }
            rampFileTypes++; // try next supported .ext
         }
         if (!fp_) // no luck finding a mass spec file
         {
   
            int archiveResult;
   
            // if can't open .dta directly, try from .tgz file
            iFileType = 1;
   
            // test if the archive contains the specified dta
            sprintf(szCommand, "tar --wildcards -tzf %s \"%s\" > /dev/null",
                  pEnvironment.szTarFile, pEnvironment.szInputFile);
   
            archiveResult = tpplib_system(szCommand); // like system(), but deals with potential win32 tar issues
   
            if (archiveResult == 0)
            {
               sprintf(szCommand, "tar --wildcards  -xzOf %s \"%s\"",
                     pEnvironment.szTarFile, pEnvironment.szInputFile);
               ppIn=tpplib_popen(szCommand, "r");
            }
            else // not found in archive
            {
               PRINT_MESSAGE("Error - cannot read spectrum; tried direct .dta, from mzML/mzXML/mzData and from .tgz");
               PRINT_MESSAGE("Verify that file path is correct.");
               PRINT_MESSAGE(pEnvironment.szInputFile);
               PRINT_FOOTER();
               exit(EXIT_FAILURE);
            }
         }
      }
   }
   else // if here, the dta file actually existed in the filesystem (iFileType == 0)
   {
      strcpy(szBaseName, pEnvironment.szFullPathInputFile);
      szBaseName[strlen(szBaseName)-4]='\0';   // remove .dta extension
   }

   pEnvironment.iLenPeptide=strlen(pEnvironment.szPeptide);

   // If experimental neutral mass not passed via environment variable PepMass
   // then set dPepMass to calculated mass just so that MS1 plot will show
   // correct ms1 region.  In this case, user will be oblivious to fact that
   // PepMass variable is missing from URL.
   if (pEnvironment.dPepMass == 0.0)
   {
      if (pEnvironment.dModN==0.0)  // add N terminus 
         pEnvironment.dPepMass += pdMassAA['h'];
      else
         pEnvironment.dPepMass += pEnvironment.dModN;
   
      if (pEnvironment.dModC==0.0)  // add C terminus
         pEnvironment.dPepMass += pdMassAA['o'] + pdMassAA['h'];
      else
         pEnvironment.dPepMass += pEnvironment.dModC;
   
      for (i=0; i<pEnvironment.iLenPeptide; i++)
      {
         pEnvironment.szPeptide[i]=toupper(pEnvironment.szPeptide[i]);
   
         if (pEnvironment.pdModPeptide[i]==0.0)
            pEnvironment.dPepMass += pdMassAA[pEnvironment.szPeptide[i]];
         else
            pEnvironment.dPepMass += pEnvironment.pdModPeptide[i];
      }
   }
   if (pEnvironment.dPrecMz == 0.0)  // only calculate it if not passed on URL
      pEnvironment.dPrecMz = (pEnvironment.dPepMass + iCharge*1.00727646688 )/iCharge;
              
   iLen=strlen(szBaseName);

   printf("<div id=\"lorikeet\"></div>\n\
\n\
<script type=\"text/javascript\">\n\
\n\
$(document).ready(function () {\n\
\n\
   /* render the spectrum with the given options */\n\
   $(\"#lorikeet\").specview({\"sequence\":sequence,\n\
                        \"scanNum\":scanNum,\n\
                        \"charge\":charge,\n\
                        \"fragmentMassType\":fragmentMassType,\n\
                        \"precursorMassType\":precursorMassType,\n\
                        \"width\":width,\n\
                        \"height\":height,\n\
                        \"precursorMz\":precursorMz,\n\
                        \"showMassErrorPlot\":showMassErrorPlot,\n\
                        \"fileName\":fileName,\n");

   if (pEnvironment.bLabelReporterIons)
      printf("                        \"labelReporters\":true,\n");
   if (pEnvironment.dModN!=0.0)
      printf("                        \"ntermMod\":ntermMod,\n");
   if (pEnvironment.dModC!=0.0)
      printf("                        \"ctermMod\":ctermMod,\n");
   for (i=0; i<pEnvironment.iLenPeptide; i++)
   {
      if (pEnvironment.pdModPeptide[i]!=0.0)
      {
         bVariableMod=1;
         printf("                        \"variableMods\":variableMods,\n");
         break;
      }
   }

   if (iFileType==2)
      printf("                        \"ms1peaks\":ms1peaks,\n\
                        \"ms1scanLabel\":ms1scanLabel,\n");

   printf("                        \"zoomMs1\":zoomMs1,\n\
                        \"precursorPeakClickFn\":precursorPeakClicked,\n\
                        \"peaks\":ms2peaks});\n\
\n\
});\n\
\n\
\n\
function precursorPeakClicked(precursorMz) {\n\
   alert(\"precursor peak clicked: \"+precursorMz);\n\
}\n");

   if ( (pStr=findRightmostPathSeperator(pEnvironment.szInputFile)) )
      strcpy(szPrintName, pStr+1);
   else
      strcpy(szPrintName, pEnvironment.szInputFile+1);
   szPrintName[strlen(szPrintName)-4]=0;

   printf("var fragmentMassType = \"%s\";\n", pEnvironment.iMassTypeFragment?"mono":"avg");
   printf("var precursorMassType = \"%s\";\n", pEnvironment.iMassTypePrecursor?"mono":"avg");
   printf("var width = %d;\n", pEnvironment.iWidth);
   printf("var height = %d;\n", pEnvironment.iHeight);
   printf("var charge = %d;\n", iCharge);
   printf("var scanNum = %d;\n", GET_SCANNUM_FROM_DTA_NAME(pEnvironment.szInputFile));
   printf("var sequence = \"%s\";\n", pEnvironment.szPeptide);
   printf("var fileName = \"%s\";\n", szPrintName);
   printf("var showMassErrorPlot = \"true\";\n");
   printf("var precursorMz = %0.4f;\n", pEnvironment.dPrecMz);
   printf("var zoomMs1 = \"true\";\n");
   if (pEnvironment.dModN!=0.0)
      printf("var ntermMod = %0.6f;\n", pEnvironment.dModN - pdMassAA['h']);   // dModN contains mass of modified N-term
   if (pEnvironment.dModC!=0.0)
      printf("var ctermMod = %0.6f;\n", pEnvironment.dModC - pdMassAA['o'] - pdMassAA['h'] - pdMassAA['h']);
   
   if (bVariableMod)
   {
      int bPrintComma=0;
      printf("var variableMods = [");
      for (i=0; i<pEnvironment.iLenPeptide; i++)
      {
         if (pEnvironment.pdModPeptide[i]!=0.0)
         {
            if (bPrintComma)
               printf(",");

            printf(" {index: %d, modMass: %0.6f, aminoAcid: \"%c\"}",
                  i+1,
                  pEnvironment.pdModPeptide[i] - pdMassAA[pEnvironment.szPeptide[i]],
                  pEnvironment.szPeptide[i]);

            bPrintComma=1;
         }
      }
      printf("];\n");
   }

   ramp_fileoffset_t *index_=NULL;
   int iAnalysisLastScan;

   if (iFileType == 2)
      index_ = readIndex(fp_, getIndexOffset(fp_), &(iAnalysisLastScan));

   PRINT_MS2_PEAKS(ppIn, fp_, iFileType, szPrintName, iCharge, index_, iAnalysisLastScan);

   if (iFileType==2)
      PRINT_MS1_PEAKS(ppIn, fp_, index_, iAnalysisLastScan);

   printf("</script>\n");
   PRINT_FOOTER();

   if (index_ != NULL)  // done with index
      delete index_;

   fflush(stdout);

   if (iFileType == 0)
      fclose(ppIn);
   else if (iFileType == 1)
      pclose(ppIn);
   else if (iFileType == 2)
   {
      if (fp_ != NULL)
         rampCloseFile(fp_);
   }

   fflush(stdout);

   return(EXIT_SUCCESS);

}


void EXTRACT_CGI_QUERY(void)
{
   char *pRequestType,
        *pQS,
        szWord[1024];
   int  i;

   pRequestType=getenv("REQUEST_METHOD");
   if(pRequestType==NULL)
   {
      printf(" This program needs to be called with CGI GET method.\n");
      exit(EXIT_FAILURE);
   }
   else if (strcmp(pRequestType, "GET"))
   {
      printf(" This program needs to be called with CGI GET method.\n");
      exit(EXIT_FAILURE);
   }

   // Decode GET method
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

      if (!strcmp(szWord, "File"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%s", pEnvironment.szInputFile);
      }
      else if (!strcmp(szWord, "FragmentMassType") || !strcmp(szWord, "MassType"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%d", &(pEnvironment.iMassTypeFragment));
      }
      else if (!strcmp(szWord, "PrecursorMassType"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%d", &(pEnvironment.iMassTypePrecursor));
      }
      else if (!strcmp(szWord, "Isobaric"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%d", &(pEnvironment.bLabelReporterIons));
      }
      else if (!strcmp(szWord, "Dta"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         strcpy(pEnvironment.szInputFile, szWord);
      }
      else if (!strcmp(szWord, "Pep"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%s", pEnvironment.szPeptide);
      }
      else if (!strcmp(szWord, "PepMass"))  // precursor_neutral_mass from pep.xml
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%lf", &(pEnvironment.dPepMass));
      }
      else if (!strcmp(szWord, "PrecMz"))  // precursor m/z
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%lf", &(pEnvironment.dPrecMz));
      }
      else if (!strcmp(szWord, "ScanStart"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%d", &(pEnvironment.iScanStart));
      }
      else if (!strcmp(szWord, "ScanEnd"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%d", &(pEnvironment.iScanEnd));
      }
      else if (!strcmp(szWord, "Charge"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%d", &(pEnvironment.iCharge));
      }
      else if (!strncmp(szWord, "Mod", 3))
      {
         int  iIndex;
         char szMod[24];

         strcpy(szMod, szWord+3);

         if (strlen(szMod)==1 && (szMod[0]=='N' || szMod[0]=='C'))
         {
            getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);

            if (szMod[0]=='N')
               sscanf(szWord, "%lf", &(pEnvironment.dModN)); // nterm mod
            else
               sscanf(szWord, "%lf", &(pEnvironment.dModC)); // cterm mod
         }
         else
         {
            sscanf(szMod, "%d", &iIndex);
            iIndex -= 1;

            getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
            sscanf(szWord, "%lf", &(pEnvironment.pdModPeptide[iIndex]));
         }
      }
      else
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
      }
   }

   if (strlen(pEnvironment.szInputFile) < 2)
   {
      PRINT_MESSAGE("Error - No file name or dta specified");
      PRINT_FOOTER();
      exit(1);
   }

   if (HAS_ENDING(pEnvironment.szInputFile, ".dta"))
   {
      char szTmp[SIZE_FILE];
      strcpy(szTmp, pEnvironment.szInputFile);
      fixPath(pEnvironment.szInputFile,1);
      // if fixPath replaces the input with just a directory, just revert to un-fixed string
      if (!HAS_ENDING(pEnvironment.szInputFile, ".dta"))
         strcpy(pEnvironment.szInputFile, szTmp);
   }
   else
   {
      if (fixPath(pEnvironment.szInputFile,1)<1)  // pretty up the path separators etc  - expect existence
      {
         if (!HAS_ENDING(pEnvironment.szInputFile, ".dta"))
         {
            // fixPath mangles the input string in a way that we cannot check if
            // file is not there, but only validate its path -- oh, the irony!
            PRINT_MESSAGE("Error - Bad path to file!");
            PRINT_MESSAGE(pEnvironment.szInputFile);
            PRINT_FOOTER();
            exit(1);
         }
      }
   }

   INITIALIZE_MASS(pdMassAA, pEnvironment.iMassTypeFragment);

}


void INITIALIZE(void)
{
   int i;

   pEnvironment.iLenPeptide=0;

   pEnvironment.iScanStart=0;
   pEnvironment.iScanEnd=0;
   pEnvironment.iCharge=1;

   pEnvironment.szInputFile[0]='\0';
   pEnvironment.szPeptide[0]='\0';

   pEnvironment.dPrecMz=0.0;
   pEnvironment.dPepMass=0.0;
   pEnvironment.dModN=0.0;
   pEnvironment.dModC=0.0;

   pEnvironment.iMassTypeFragment = 1;  // default to mono
   pEnvironment.iMassTypePrecursor = 1;

   pEnvironment.iWidth = 650;
   pEnvironment.iHeight = 400;

   memset(pEnvironment.pdModPeptide, 0, sizeof(pEnvironment.pdModPeptide));
   memset(pdMassAA, 0, sizeof(pdMassAA));

   for (i=0; i<MAX_PEPTIDE_LEN; i++)
      pEnvironment.pdModPeptide[i]=0.0;
}


void PRINT_MS2_PEAKS(FILE *ppIn,
        RAMPFILE *fp_,
        int  iFileType,
        char *szPrintName,  // last argument passed just for temp tof-tof data - mascot generic format starts with pps_
        int  iCharge,
        ramp_fileoffset_t *index_,
        int iAnalysisLastScan)
{
   int    i,
          iScanNum=0;
   char   szBuf[SIZE_BUF],
          szCommand[SIZE_BUF];
   double dPepMass;

   struct ScanHeaderStruct scanHeader;
  

   if (iFileType != 2)  // is not mzXML
   {
      char *fgot=fgets(szBuf, SIZE_BUF, ppIn);       // skip first line of .dta file
      sscanf(szBuf, "%lf %d", &dPepMass, &iCharge);  // override previous charge from .dta name
   }

   if (iFileType == 2)
   {
      // mzXML file: read file index and parse scan # from encoded .dta file
      iScanNum = GET_SCANNUM_FROM_DTA_NAME(pEnvironment.szInputFile);
   }

   // parse through mass/intensity pairs
   printf("\nvar ms2peaks = [");
   if (iFileType != 2)
   {
      while (fgets(szBuf, SIZE_BUF, ppIn))
      {
         double dMass=0.0,
                dIntensity=0.0;
   
         sscanf(szBuf, "%lf %lf", &dMass, &dIntensity);
         if (dIntensity > 0.0)
            printf("[%0.6f,%0.6f],\n", dMass, dIntensity);
      }
   }
   else
   {
      if (iScanNum > iAnalysisLastScan)
      {
         printf("];\n");
         printf("</script>\n");
         sprintf(tmpMsg, " Error - can't parse scan number (got %d, only %d in file)", iScanNum, iAnalysisLastScan);
         PRINT_MESSAGE(tmpMsg);
         PRINT_FOOTER();
         exit(EXIT_FAILURE);
      }
      scanHeader.msLevel = readMsLevel(fp_, index_[iScanNum]);

      if (scanHeader.msLevel > 1)
      {
         RAMPREAL *pPeaks;
         int n = 0;
         int bFirstDataPoint = 1;

         // Open a scan
         pPeaks = readPeaks(fp_, index_[iScanNum]);

         while (pPeaks != NULL && pPeaks[n] != -1)
         {
            RAMPREAL fMass;
            RAMPREAL fInten;
            

            fMass = pPeaks[n];
            n++;
            fInten = pPeaks[n];
            n++;

            if (fInten > 0.0)
            {
               if (!bFirstDataPoint)
                 printf(",\n"); 
               printf("[%0.6f,%0.6f]", fMass, fInten);
               bFirstDataPoint = 0;
            }
         }
         if (pPeaks != NULL)
            free(pPeaks);
      }
      else
      {
         printf("];\n");
         printf("</script>\n");
         sprintf(tmpMsg, "Error - scan %d is an <b>MS1</b> scan in the mzXML file %s", iScanNum, pEnvironment.szMZXMLFile);
         PRINT_MESSAGE(tmpMsg);
         PRINT_FOOTER();
         exit(1);
      }
   }

   printf("];\n");
}


void PRINT_MS1_PEAKS(FILE *ppIn,
        RAMPFILE *fp_,
        ramp_fileoffset_t *index_,
        int iAnalysisLastScan)
{
   int i=0;
   int    iScanNum;
   struct ScanHeaderStruct scanHeader, scanHeaderMS;

   // mzXML file: read file index and parse scan # from encoded .dta file
   iScanNum = GET_SCANNUM_FROM_DTA_NAME(pEnvironment.szInputFile);

   readHeader(fp_, index_[iScanNum], &scanHeader);

   // loop back through scans to find MS1 scan
   iScanNum--;
   while (1)
   {
      readHeader(fp_, index_[iScanNum], &scanHeaderMS);
      if (iScanNum < 1 || scanHeaderMS.msLevel == (scanHeader.msLevel - 1))
      {
         break;
      }
      iScanNum--;
   }

   if (scanHeaderMS.msLevel == (scanHeader.msLevel - 1))
   {
      RAMPREAL *pPeaks;
      int n = 0;
      int bFirstDataPoint=1;
      double dPrecMass=0.0;
      double dPrecInten=0.0;
      double dLowMass = scanHeader.precursorMZ - 0.25;
      double dHighMass = scanHeader.precursorMZ + 0.25;

      // Open a scan
      pPeaks = readPeaks(fp_, index_[iScanNum]);

      printf("\nvar ms1peaks = [");
      while (pPeaks != NULL && pPeaks[n] != -1)
      {
         RAMPREAL fMass;
         RAMPREAL fInten;

         fMass = pPeaks[n];
         n++;
         fInten = pPeaks[n];
         n++;

         if (fInten > 0.0)
         {
            if (dLowMass<fMass && fMass<dHighMass)
            {
               if (fInten > dPrecInten)
               {
                 dPrecInten = fInten;
                 dPrecMass = fMass;
               }
            }

            if (!bFirstDataPoint)
               printf(",\n"); 
            printf("[%0.6f,%0.6f]", fMass, fInten);
            bFirstDataPoint = 0;
         }
      }
      if (pPeaks != NULL)
         free(pPeaks);

      printf("];\n");

      printf("\nvar ms1scanLabel = \"%d, RT %0.2f\";\n", iScanNum, scanHeaderMS.retentionTime);
   }
   else
   {
      printf("\nvar ms1scanLabel = \" \";\n");
      printf("var ms1peaks= [[0.0,0.0]];\n");
   }
}


// extract the scan number from the dta file name, which is expected be of the format
// xxxxxxxxxxxxxx.scanstart.scanend.charge
int GET_SCANNUM_FROM_DTA_NAME(const char* dtaName)
{
   int scanNum = -1;
   char szTmp[500];
   char *pStr;


   // look for an X!Tandem style declaration
   for (const char *c=dtaName;c && *c && (c=strstr(c,"scan"));)
   {
      if (1==sscanf(c++,"scan %d (charge ", &scanNum))
      {
         return scanNum;
      }
   }

   // assume dta filename format is xxxxxxxx.scanstart.scanend.charge.dta
      
   // start from right to left, in order to avoid issues with messy naming schemes,
   // like maldi, which may have a format like xxxxxxx.SPOT_DESIGNATOR.xxxx.xxxx.xxx.dta

   strcpy(szTmp, dtaName);

   int fieldCount=4;
   while (fieldCount != 0)
   {
      pStr = strrchr(szTmp, '.');
      if (pStr == NULL)
      {
         printf("</script>\n");
         sprintf(tmpMsg, "Error - cannot get scan number from input file <b>%s</b>; unexpected dta name format", dtaName);
         PRINT_MESSAGE(tmpMsg);
         PRINT_FOOTER();
         exit(1);
      }
      // first time here: pStr is one before 'dta'
      // second time: pStr is one before charge
      // third time: pStr is one before scanend
      // forth: one before scanstart (what we want)
      *pStr = '\0';
      --fieldCount;
   }
   // szTemp is now xxxxxxx.scanstartNULLxxxxxx
   pStr++;
   sscanf(pStr, "%d", &scanNum);
 
   return scanNum;
}


void PRINT_MESSAGE(const char *message)
{
   if (!inMsgPane)
   {
      printf("<style type=\"text/css\">\n");
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
      printf("</style>\n");

      printf("<table cellspacing=\"0\">\n");
      printf("<tbody>\n<tr>\n");
      printf("<td class=\"messages_h\">&nbsp;&nbsp;&nbsp;Messages&nbsp;&nbsp;&nbsp;</td>\n");
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
}

void PRINT_FOOTER(void)
{
   printf("<!-- page footer -->\n");
   printf("<hr noshade/>\n");
   printf("<div class='lorikeet'><a href=\"http://code.google.com/p/lorikeet\">%s</a><br/>\n", TITLE);
   printf("(%s)</div>\n", szTPPVersionInfo);
   printf("</body>\n</html>");

   return;
}

bool HAS_ENDING (std::string const &fullStr, std::string const &ending)
{
   if (fullStr.length() >= ending.length())
   {
      return (0 == fullStr.compare(fullStr.length() - ending.length(), ending.length(), ending));
   }
   else
   {
      return false;
   }
}
