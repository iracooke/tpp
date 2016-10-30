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
 *
 * Purpose:  CGI program to plot MS/MS spectrum and associated masses or fragment ions
 *
 * $Id: plot-msms1.cxx 6739 2014-11-12 22:38:49Z slagelwa $
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

#define IMAGEWIDTH  0.8
#define IMAGEHEIGHT 0.6

#define MASSBKGRND  "#FFFFFF"
#define NTERMBKGRND "#aaaaff" /* blue */
#define CTERMBKGRND "#ffaaaa" /* red/orange */

#define MAX_NUM_FRAGMENT_IONS 2000
#define MAX_NUM_LABELIONS 50

#define TITLE "COMET Spectrum View"

#define MASS_TOL 0.95  /* default match mass tol */

#define SMALL_MASS_TOL  0.25 /* mass tol for immonium and itraq peak labels */

#define MAX_GLOBAL_MODS 20

#define PROTON_MASS 1.00727646688

/*
 * note that these colors codes change with particular gnuplot revisions
 * -1=black 0=gray 1=red 2=brightgreen 3=blue 4=violet 5=lightgreen 6=brown
 *  7=aqua 8=drkblue 9=purple 10=gray
 */
#define LINECOLOR_UNLABEL  "#8A8A8A" /* gray */
#define LINECOLOR_PRECURSOR "#EED700" /* gold */

#define LINECOLOR_LABEL2   "#1E90FF"  /* blue; n-term peaks */
#define LINECOLOR_LABEL3   "#FF0000"  /* red; c-term peaks */
#define LINECOLOR_LABEL4   "#000080"  /* dark blue; phospho neutral loss peak */
#define LINECOLOR_LABEL5   "#CD950C"  /* goldenrod; immonium ions */
#define LINECOLOR_LABEL6   "#FFFF00"  /* kakhi; precursor plot ions */

#include "common/AminoAcidMasses.h"
#include "Visualization/Comet/Comet.h"
#include "common/TPPVersion.h" // contains version number, name, revision
#include "mzParser.h"


#include <sys/stat.h>
#include <ctype.h>
#include <math.h>

 
double pdMassAA[256];

int   iFragmentIonCt=0;

struct FragmentIonsStruct
{
   char   szIon[12];     /* string to print out on plot ... e.g. b3/y12++ */
   int    bUsed;
   int    iType;         /* 0=Aion, 1=Bion ... 7=Yion, 8=Zion */
   double dMass;         /* mass of the fragment ion */
} pFragmentIons[MAX_NUM_FRAGMENT_IONS];

struct LabelIonsStruct
{
   char   szLabel[128];
   double dMass;
   double dIntensity;
} pLabelIons[MAX_NUM_LABELIONS],
  *pLabelAllIons;

struct EnvironmentStruct
{
   int  iLenPeptide;
   int  iLabelType;  /* 0=ion labels, 1=fragment masses, 2=no labels, 3=force fragment masses */
   int  iMassTypeFragment;
   int  iNumAxis;
   int  iXmin;
   int  iXmax;

   int  bPhospho;  /* highlight phospho neutral losses */
   int  bRemoveMods;  /* plot peptide w/out differential modification */

   int  iStaticTermModType;

   int  bZoom113;  /* zoom in on 113 to 118 mass range for iTRAQ */
   int  bZoom126;  /* zoom in on 126 to 131 mass range for TMT */
   int  bDebugging;    /* do not delete intermediate files if true*/
   int  bExpect;   /* display expectation plot */
   int  bPDF;      /* if true leave out imagemap, left control pane, etc. */

   char cShowA;
   char cShowA2;
   char cShowA3;
   char cShowB;
   char cShowB2;
   char cShowB3;
   char cShowC;
   char cShowC2;
   char cShowC3;
   char cShowX;
   char cShowX2;
   char cShowX3;
   char cShowY;
   char cShowY2;
   char cShowY3;
   char cShowZ;
   char cShowZ2;
   char cShowZ3;
   char cShowNH3Loss; /* unused */
   char cShowH2OLoss; /* mark -17 NH3 and -18 H20 loss by tracking just -17.5 */

   char szInputFile[SIZE_FILE];
   char szFullPathInputFile[SIZE_FILE];
   char szTarFile[SIZE_FILE];
   char szMZXMLFile[SIZE_FILE];
   char szPeptide[MAX_PEPTIDE_LEN];
   char szProtein[SIZE_FILE];
   char pcMod[MAX_PEPTIDE_LEN];  /* old way of noting variable modification string */

   double dPepMass;
   double dMaxPeakMass;
   double dImageScale;

   double pdModPeptide[MAX_PEPTIDE_LEN]; /* just contains a list of modified masses at each peptide position */
   double pdModNC[2];  /* mass of new N or C terminus */

   double dMatchTol;
   double dIntensityZoom;

   double dScore;  /* score to print out with PDF option */

} pEnvironment;


static void EXTRACT_CGI_QUERY(void);
static void INITIALIZE(void);
static void CALC_IONS(void);
static void DISPLAY_IONS(void);
static void CREATE_IMAGE(FILE *ppIn,
        RAMPFILE *fp_,
        int  iFileType,  /* 0 = .dta file, 1 = read from .tgz file, 2 = read from mzXML file */
        FILE *fpPlot,
        char *szImageFile,
        char *szPlotFile,  /* .gp file of gnuplot commands */
        char *szPrintName,
        int  iCharge,
        ramp_fileoffset_t *index_,
        int iAnalysisLastScan);
void CREATE_PRECURSOR_IMAGE(FILE *ppIn,
        RAMPFILE *fp_,
        FILE *fpPlot,
        char *szImageFileMS,
        char *szPlotFile,
        double dPrecursorMZ,    /* theoretical precursor */
        ramp_fileoffset_t *index_,
        int iAnalysisLastScan);
void STORE_TOP_PEAKS(double dMass,
        double dIntensity);
void READ_PEAK(double *d113,
      double *d114,
      double *d115,
      double *d116,
      double *d117,
      double *d118,
      double *d119,
      double *d121,
      double *d126,
      double *d127,
      double *d128,
      double *d129,
      double *d130,
      double *d131,
      double *dMaxMass,
      double *dMaxInten,      /* overall max intensity */
      double *dMaxIntensity1,  /* max intensity in 1st half of 2-axis plot */
      double *dMaxIntensity2, /* max intensity in 2nd half of 2-axis plot */
      double dMass,
      double dIntensity,
      FILE *fpTmpData,
      char *szBuf,
      int  iMid);
static int SORT_LABELIONS_MASS(const void *p0,
      const void *p1);
static int SORT_LABELIONS_INTEN(const void *p0,
      const void *p1);
static void HIGHLIGHT(double dMass,
      int iWhichIonSeries);
static int GET_SCANNUM_FROM_DTA_NAME(const char* dtaName);
void DISPLAY_EXPECT(void);

#include "common/util.h"

int main(int argc, char **argv)
{
   hooks_tpp(argc,argv); // installdir issues etc

   FILE *ppIn,
        *fpPlot;

   RAMPFILE *fp_=NULL;

   int  i,
        ii,
        iFileType=0,  /* 0 = .dta file, 1 = read from .tgz file, 2 = read from mzXML file */
        iLen,
        iInterval,
        iCharge=0;

   struct stat statbuf;

   char szBaseName[SIZE_FILE],
     szPlotFile[SIZE_FILE],
     szPrintName[SIZE_FILE],
     szRandom[SIZE_FILE],
     szImageFile[SIZE_FILE],
     szImageFileMS[SIZE_FILE],
     szCommand[SIZE_BUF],
     szImageMapLink[SIZE_BUF],
     szImageMapXrange[SIZE_BUF],
     szWebserverRoot[SIZE_BUF],
     szScriptName[SIZE_BUF];  /* cgi script name */
   const char *wsr;
   char *pStr;
   char* result = NULL;

   time_t tStartTime;

   /*
    * Print HTML header
    */
   printf("Content-type: text/html\n\n");
   printf("<HTML>\n");
   printf("   <HEAD>\n");
   printf("      <TITLE>%s by J.Eng (c) ISB 2001 (%s)</TITLE>\n", TITLE,szTPPVersionInfo);

   /* simple javascript to show/hide */
   printf("\n");
   printf("      <script language=\"JavaScript\">\n");
   printf("      var ids=new Array('_show');\n");
   printf("      function toggle(id)\n");
   printf("      {\n");
   printf("         if (document.getElementById)\n");
   printf("         {\n");
   printf("            if (document.getElementById(id).style.display == 'none')\n");
   printf("               document.getElementById(id).style.display = 'block';\n");
   printf("            else\n");
   printf("               document.getElementById(id).style.display = 'none';\n");
   printf("         }\n");
   printf("      } \n");
   printf("      </script>\n");
   /* end javascript */

   printf("<STYLE TYPE=\"text/css\"> <!-- \n");

   printf("TD{font-size: 8pt;} TH{font-size: 9pt;}\n");

   printf(".accepted {background: #87ff87; font-weight:bold;}\n");
   printf(".rejected {background: #ff8700;}\n");
   printf("body{font-family: Helvetica, sans-serif; }\n");
   printf("h1  {font-family: Helvetica, Arial, Verdana, sans-serif; font-size: 24pt; font-weight:bold; color:#0E207F}\n");
   printf("h2  {font-family: Helvetica, Arial, sans-serif; font-size: 20pt; font-weight: bold; color:#0E207F}\n");
   printf("h3  {font-family: Helvetica, Arial, sans-serif; font-size: 16pt; color:#FF8700}\n");
   printf("h4  {font-family: Helvetica, Arial, sans-serif; font-size: 14pt; color:#0E207F}\n");
   printf("h5  {font-family: Helvetica, Arial, sans-serif; font-size: 10pt; color:#AA2222}\n");
   printf("h6  {font-family: Helvetica, Arial, sans-serif; font-size:  8pt; color:#333333}\n");
   printf("table   {border-collapse: collapse; border-color: #dddddd;}\n");
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
   printf("                 color: #AA2222;\n");
   printf("                 font-weight:bold;\n");
   printf("              }\n");
   printf(".glyco        {\n");
   printf("                 background: %s;\n",NTERMBKGRND);
   printf("                 border: 1px solid black;\n");
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
   printf("                 background: %s;\n",CTERMBKGRND);
   printf("                 border: 1px solid black;\n");
   printf("                 font-weight:bold;\n");
   printf("              }\n");
   printf(".info         {\n");
   printf("                 border-top: 1px solid black;\n");
   printf("                 color: #333333;\n");
   printf("                 font-size: 10pt;\n");
   printf("              }\n");
   printf("--->\n");
   printf("</style>\n");

   printf("   </HEAD>\n");
   printf("\n");
   printf("<BODY BGCOLOR=\"%s\" OnLoad=\"self.focus();\">\n", "#C0C0C0");

   INITIALIZE();

   wsr=getWebserverRoot();
   if (wsr==NULL)
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
      FILE *fp;
      sprintf(szCommand, "cygpath '%s'", wsr);
      if((fp = popen(szCommand, "r")) == NULL)
      {
         printf("cygpath error, exiting\n");
         exit(1);
      }
      else
      {
         char szBuf[SIZE_BUF];
         fgets(szBuf, SIZE_BUF, fp);
         pclose(fp);
         szBuf[strlen(szBuf)-1] = 0;
         strcpy(szWebserverRoot, szBuf);
      }
#else
      strcpy(szWebserverRoot, wsr);
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

   EXTRACT_CGI_QUERY();

   // look for an X!Tandem style declaration
   for (const char *c=pEnvironment.szInputFile;c && *c && (c=strstr(c,"scan"));) {
      int iScanNum;
      if (2==sscanf(c++,"scan %d (charge %d", &iScanNum,&iCharge)) {
         break;
      }
   }

   if (!iCharge) { // try the traditional encoding
      sscanf(pEnvironment.szInputFile+strlen(pEnvironment.szInputFile)-5, "%d", &iCharge);
   }
   /* small safeguard in case above parsing of charge state from .dta name fails */
   if (iCharge<0 || iCharge>9)
      iCharge=1;

   /*
    * set default ions on when first called and nothing selected
    */
   if (pEnvironment.cShowA==0
       && pEnvironment.cShowA2==0
       && pEnvironment.cShowA3==0
       && pEnvironment.cShowB==0
       && pEnvironment.cShowB2==0
       && pEnvironment.cShowB3==0
       && pEnvironment.cShowC==0
       && pEnvironment.cShowC2==0
       && pEnvironment.cShowC3==0
       && pEnvironment.cShowX==0
       && pEnvironment.cShowX2==0
       && pEnvironment.cShowX3==0
       && pEnvironment.cShowY==0
       && pEnvironment.cShowY2==0
       && pEnvironment.cShowY3==0
       && pEnvironment.cShowZ==0
       && pEnvironment.cShowZ2==0 
       && pEnvironment.cShowZ3==0)
   {
      if (iCharge == 1)
      {
         pEnvironment.cShowB=1;
         pEnvironment.cShowY=1;
      }
      else if (iCharge == 2)
      {
         pEnvironment.cShowB=1;
         pEnvironment.cShowY=1;
         pEnvironment.cShowY2=1;
      }
      else
      {
         pEnvironment.cShowB=1;
         pEnvironment.cShowB2=1;
         pEnvironment.cShowY=1;
         pEnvironment.cShowY2=1;
         pEnvironment.cShowY3=1;
      }
   }

   tStartTime=time((time_t *)NULL);

   /*
    * simple peptide hash; needed to keep consecutive fast
    * calls to cgi from generating same image name
    */
   ii=0;
   for (i=0; pEnvironment.szPeptide[i]!=0; i++)
      ii += pEnvironment.szPeptide[i];
   ii += GET_SCANNUM_FROM_DTA_NAME(pEnvironment.szInputFile);
   srandom(ii + tStartTime);

   /*
    * pEnvironment.szInputFile has input .dta file ... need to get this
    * from the tar/gzipped archive now
    *
    * So, first modify szTarFile to be the gzipped tar file name
    */
   strcpy(pEnvironment.szTarFile, pEnvironment.szInputFile);
   iLen=strlen(pEnvironment.szTarFile);
   for (i=iLen-1; i>2; i--)
   {
     if (isPathSeperator(pEnvironment.szTarFile[i]) && pEnvironment.szTarFile[i-1]=='.' && isPathSeperator(pEnvironment.szTarFile[i-2]))
      {
         pEnvironment.szTarFile[i-2]='\0';

         strcat(pEnvironment.szTarFile, ".tgz");
         break;
      }
         else if (isPathSeperator(pEnvironment.szTarFile[i])) /* this case is needed for IE on the Mac ... /./ changes to just / */
      {
         pEnvironment.szTarFile[i]='\0';

         strcat(pEnvironment.szTarFile, ".tgz");
         break;
      }
   }

   iLen=strlen(pEnvironment.szTarFile);
   strcpy(szBaseName, pEnvironment.szTarFile);

   szBaseName[strlen(szBaseName)-4]='\0';

   rampConstructInputFileName(pEnvironment.szMZXMLFile, sizeof(pEnvironment.szMZXMLFile), szBaseName);

   if (!strstr(pEnvironment.szTarFile+iLen-4, ".tgz")
       && !strstr(pEnvironment.szTarFile+iLen-11, ".cmt.tar.gz"))
   {
       printf(" Error converting %s to tar file.\n", pEnvironment.szInputFile);
       printf("<P>szBaseName=%s\n", szBaseName);
       printf("<P>szTarFile=%s\n", pEnvironment.szTarFile);
       printf("<P>iLen=%d\n", iLen);
       exit(EXIT_FAILURE);
   }

   /*
    * Next, modify pEnvironment.szInputFile to remove full path
    */
   strcpy(pEnvironment.szFullPathInputFile, pEnvironment.szInputFile);
   strcpy(pEnvironment.szInputFile, pEnvironment.szInputFile+i+1);
   sprintf(szCommand, "*%s", pEnvironment.szInputFile);
   strcpy(pEnvironment.szInputFile, szCommand);

   /*
    * handle searches which weren't done by runsearch, and so don't have tgz'd .out and .dta
    */
   if (stat(pEnvironment.szTarFile,&statbuf) && /* does the tgz file exist? */
       stat(pEnvironment.szFullPathInputFile,&statbuf)) /* does the .dta file exist as named in the command? */
   { /* no, try to remove that middle foo in wwwroot/foo/foo/foo.0001.0001.2.dta */
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
      { /* use this as the filename */
         strncpy(pEnvironment.szFullPathInputFile,szUntarredDTAfile,sizeof(pEnvironment.szFullPathInputFile));
      }
   }

   iFileType = 0;

   /*
    * (jmt) The goal is to end up with a filepointer that will be
    * reading in dta-format data.  This filepointer may be a direct
    * fopen from an existing dta filename, a pipe-open from a
    * uncompressing a tgz file, or the result of the ramp parser
    * operating on an mzXML file.
    *
    * iFileType == 0: direct from dta file
    * iFileType == 1: from tgz archive
    * iFileType == 2: from mzXML via ramp
    */


   /* try to open the dta file directly, from the filesystem; then mzXML then .tgz */
   if ((ppIn=fopen(pEnvironment.szFullPathInputFile, "r"))==NULL)
   {
      /* printf("<b>not in archive!(%s == %d)</b><br />\n", szCommand, archiveResult); */
      /* try reading spectrum directly from a RAMP-supported mass spec format */
      /* whether sucessful or not */

      fp_ = NULL;

      if ((fp_ = rampOpenFile(pEnvironment.szMZXMLFile)) != NULL)
      {
         iFileType = 2;   /* success ... read from mzXML file */
      }
      else
      {  // try adding the various RAMP-supported filename exts and see if we find a file
         const char **rampFileTypes = rampListSupportedFileTypes();
         while (*rampFileTypes) { // list of .exts is null terminated
            strcpy(pEnvironment.szMZXMLFile, szBaseName);
            strcat(pEnvironment.szMZXMLFile, *rampFileTypes);
            if ((fp_ = rampOpenFile(pEnvironment.szMZXMLFile)) != NULL)
            {
               iFileType = 2;   /* success ... read from mass spec file */
               break;
            }
            rampFileTypes++; // try next supported .ext
         }
         if (!fp_) // no luck finding a mass spec file
         {
   
            int archiveResult;
   
            /* if can't open .dta directly, try from .tgz file */
            iFileType = 1;
   
            /* test if the archive contains the specified dta */
            sprintf(szCommand, "tar --wildcards -tzf %s \"%s\" > /dev/null",
                  pEnvironment.szTarFile, pEnvironment.szInputFile);
   
            archiveResult = tpplib_system(szCommand); // like system(), but deals with potential win32 tar issues
   
            if (archiveResult == 0)
            {
               sprintf(szCommand, "tar --wildcards  -xzOf %s \"%s\"",
                     pEnvironment.szTarFile, pEnvironment.szInputFile);
               ppIn=tpplib_popen(szCommand, "r");
            }
            else /* not found in archive */
            {
               printf(" Error - cannot read spectrum; tried direct .dta, from mzXML/mzData and from .tgz\n");
               exit(EXIT_FAILURE);
            }
         }
      }
   }
   else /* if here, the dta file actually existed in the filesystem (iFileType == 0) */
   {
      strcpy(szBaseName, pEnvironment.szFullPathInputFile);
      szBaseName[strlen(szBaseName)-4]='\0';   /* remove .dta extension */
   }

   sprintf(szRandom, "%ld", (long)random() );
   strcpy(szImageFile, szBaseName);
   // guard against case like c:/Inetpub/wwwroot/ISB/tandem/raft4041/raft4041.30176.gp
   // where raft4041 subdir doesn't actually exist
   char *slash1 = findRightmostPathSeperator(szImageFile);
   if (slash1) {
      struct stat statbuf;
      char cTmp = *slash1;
      *slash1 = 0;
      if (stat(szImageFile,&statbuf)) {
         // c:/Inetpub/wwwroot/ISB/tandem/raft4041 does not exist
         char *slash2 = findRightmostPathSeperator(szImageFile);
         if (slash2) {
            *slash2 = 0;
            if (stat(szImageFile,&statbuf)) {
               // c:/Inetpub/wwwroot/ISB/tandem/raft4041 does not exist
               strcpy(szImageFile, szBaseName); // retreat!
            } else {
               *slash2++ = '/';
               strcpy(slash2,slash1);
            }
         }
      }
      else
         *slash1 = cTmp;
   }
   strcpy(szPlotFile, szImageFile);
   sprintf(szImageFile+strlen(szImageFile), ".%s.png", szRandom);
   sprintf(szPlotFile+strlen(szPlotFile), ".%s.gp", szRandom);
   replace_path_with_webserver_tmp(szImageFile,sizeof(szImageFile)); // do this in designated tmp dir
   strcpy(szImageFileMS, szImageFile);

   replace_path_with_webserver_tmp(szPlotFile,sizeof(szPlotFile)); // do this in designated tmp dir
   if ( (fpPlot=fopen(szPlotFile,"w"))==NULL)
   {
      printf("%s<P>\n", szBaseName);
      printf(" Error - can't write plot file %s\n\n", szPlotFile);
      exit(EXIT_FAILURE);
   }

   pEnvironment.iLenPeptide=strlen(pEnvironment.szPeptide);

   /* convert peptide to upper case (expected for pdMassAA[]) */
   pEnvironment.dPepMass = 0.0;
   if (pEnvironment.pdModNC[0]==0.0)  /* add N terminus */
      pEnvironment.dPepMass += pdMassAA['h'];
   else
      pEnvironment.dPepMass += pEnvironment.pdModNC[0];

   if (pEnvironment.pdModNC[1]==0.0)  /* add C terminus */
      pEnvironment.dPepMass += pdMassAA['o'] + pdMassAA['h'] + pdMassAA['h'];
   else
      pEnvironment.dPepMass += pEnvironment.pdModNC[1];

   for (i=0; i<pEnvironment.iLenPeptide; i++)
   {
      pEnvironment.szPeptide[i]=toupper(pEnvironment.szPeptide[i]);

      if (pEnvironment.pdModPeptide[i]==0.0)
         pEnvironment.dPepMass += pdMassAA[pEnvironment.szPeptide[i]];
      else
         pEnvironment.dPepMass += pEnvironment.pdModPeptide[i];
   }
                   
   CALC_IONS();

   iLen=strlen(szBaseName);
   if (szBaseName[iLen-2]=='.' && szBaseName[iLen-7]=='.' && szBaseName[iLen-12]=='.')
      szBaseName[iLen-12]='\0';

   if (pEnvironment.bZoom113)
   {
      pEnvironment.iXmin=112;
      pEnvironment.iXmax=122;
      pEnvironment.iLabelType=1;
      pEnvironment.iNumAxis=1;
   }
   else if (pEnvironment.iXmin==113 && pEnvironment.iXmax==118)
   {
      pEnvironment.iXmin=0;
      pEnvironment.iXmax=0;
      pEnvironment.iLabelType=0;
   }

   if (pEnvironment.bZoom126)
   {
      if (!pEnvironment.bZoom113)
         pEnvironment.iXmin=124;
      pEnvironment.iXmax=133;
      pEnvironment.iLabelType=1;
      pEnvironment.iNumAxis=1;
   }
   else if (pEnvironment.iXmin==124 && pEnvironment.iXmax==133)
   {
      pEnvironment.iXmin=0;
      pEnvironment.iXmax=0;
      pEnvironment.iLabelType=0;
   }

   strcpy(szScriptName, getenv("SCRIPT_NAME"));
   if (pEnvironment.bPDF)
   {
      /*
       * note need modified peptide string here
       */
      printf("sequence: %s\n", pEnvironment.szPeptide);
      printf("<br>protein: %s\n", pEnvironment.szProtein);
      printf("<br>score: <font color=\"red\">%0.2f</font>\n", pEnvironment.dScore);
      printf("<br>precursor charge: %d+\n", iCharge);
      printf("<br>scan: %d\n", GET_SCANNUM_FROM_DTA_NAME(pEnvironment.szInputFile));
      printf("<br>mass: %0.4f\n", pEnvironment.dPepMass);
      if (pEnvironment.dIntensityZoom != 1.0)
         printf("<br>Y-axis scale: %0.2f\n", pEnvironment.dIntensityZoom);
      printf("<p>\n");
   }

   printf("<TABLE CELLPADDING=\"3\"><TR ALIGN=MIDDLE CLASS=\"formentry\">\n");
   if (!pEnvironment.bPDF)
   {
      printf("<TD VALIGN=TOP CLASS=\"graybox\" ALIGN=\"LEFT\" NOWRAP><FORM ACTION=\"%s\" METHOD=\"GET\">\n", szScriptName);

      printf("<TT><B>X-range:</B><BR>\n");
      printf(" &nbsp;<INPUT TYPE=\"TEXT\" NAME=\"Xmin\" VALUE=\"%d\" SIZE=\"2\">-", pEnvironment.iXmin);
      printf(    "<INPUT TYPE=\"TEXT\" NAME=\"Xmax\" VALUE=\"%d\" SIZE=\"2\"><BR>\n", pEnvironment.iXmax);

      printf("<B>MassTol: Y-zoom:</B><BR>\n");
      printf(" &nbsp;<INPUT TYPE=\"TEXT\" NAME=\"MatchTol\" VALUE=\"%0.3f\" SIZE=\"2\">\n", pEnvironment.dMatchTol);
      printf(" &nbsp;<INPUT TYPE=\"TEXT\" NAME=\"IntensityZoom\" VALUE=\"%0.2f\" SIZE=\"2\"><BR>\n", pEnvironment.dIntensityZoom);

      printf("<B>ImageSize:</B><BR>\n");
      printf("   <INPUT TYPE=\"RADIO\" NAME=\"ImgScale\" VALUE=\"1\"%s>Sm", (pEnvironment.dImageScale==1?" CHECKED":"")); 
      printf("   <INPUT TYPE=\"RADIO\" NAME=\"ImgScale\" VALUE=\"1.5\"%s>Lg<BR>\n", (pEnvironment.dImageScale==1.5?" CHECKED":"")); 

      if (pEnvironment.iLenPeptide>0)
      {
         printf("<B>MassType:</B><BR>\n");
         printf("   <INPUT TYPE=\"RADIO\" NAME=\"MassType\" VALUE=\"0\"%s>AVG", (pEnvironment.iMassTypeFragment==0?" CHECKED":"")); 
         printf("   <INPUT TYPE=\"RADIO\" NAME=\"MassType\" VALUE=\"1\"%s>MONO<BR>\n", (pEnvironment.iMassTypeFragment==1?" CHECKED":"")); 
      }

      printf("<B>Axis:</B><BR>\n");
      printf("   <INPUT TYPE=\"RADIO\" NAME=\"NumAxis\" VALUE=\"1\"%s>1",
            (pEnvironment.iNumAxis==1?" CHECKED":""));
      printf("   <INPUT TYPE=\"RADIO\" NAME=\"NumAxis\" VALUE=\"2\"%s>2<BR>\n",
            (pEnvironment.iNumAxis==2?" CHECKED":""));

      printf("<B>Label:</B><BR>\n");
      if (pEnvironment.iLenPeptide>0)
         printf("   <INPUT TYPE=\"RADIO\" NAME=\"LabelType\" VALUE=\"0\"%s>I\n", (pEnvironment.iLabelType==0?" CHECKED":""));
      printf("   <INPUT TYPE=\"RADIO\" NAME=\"LabelType\" VALUE=\"1\"%s>M\n", (pEnvironment.iLabelType==1?" CHECKED":""));
      printf("   <INPUT TYPE=\"RADIO\" NAME=\"LabelType\" VALUE=\"2\"%s>-<BR>\n", (pEnvironment.iLabelType==2?" CHECKED":""));

      if (pEnvironment.iLenPeptide>0)
      {
         printf("<B>Ions:</B><BR>\n");
         printf("<B>a</B><INPUT TYPE=\"CHECKBOX\" NAME=\"ShowA\" VALUE=\"1\"%s><SUP>+</SUP>", (pEnvironment.cShowA==1?" CHECKED":""));
         printf("<INPUT TYPE=\"CHECKBOX\" NAME=\"ShowA2\" VALUE=\"1\"%s><SUP>2+</SUP>", (pEnvironment.cShowA2==1?" CHECKED":""));
         printf("<INPUT TYPE=\"CHECKBOX\" NAME=\"ShowA3\" VALUE=\"1\"%s><SUP>3+</SUP>", (pEnvironment.cShowA3==1?" CHECKED":""));
         printf("<BR>\n");

         printf("<B>b</B><INPUT TYPE=\"CHECKBOX\" NAME=\"ShowB\" VALUE=\"1\"%s><SUP>+</SUP>", (pEnvironment.cShowB==1?" CHECKED":""));
         printf("<INPUT TYPE=\"CHECKBOX\" NAME=\"ShowB2\" VALUE=\"1\"%s><SUP>2+</SUP>", (pEnvironment.cShowB2==1?" CHECKED":""));
         printf("<INPUT TYPE=\"CHECKBOX\" NAME=\"ShowB3\" VALUE=\"1\"%s><SUP>3+</SUP>", (pEnvironment.cShowB3==1?" CHECKED":""));
         printf("<BR>\n");

         printf("<B>c</B><INPUT TYPE=\"CHECKBOX\" NAME=\"ShowC\" VALUE=\"1\"%s><SUP>+</SUP>", (pEnvironment.cShowC==1?" CHECKED":""));
         printf("<INPUT TYPE=\"CHECKBOX\" NAME=\"ShowC2\" VALUE=\"1\"%s><SUP>2+</SUP>", (pEnvironment.cShowC2==1?" CHECKED":""));
         printf("<INPUT TYPE=\"CHECKBOX\" NAME=\"ShowC3\" VALUE=\"1\"%s><SUP>3+</SUP>", (pEnvironment.cShowC3==1?" CHECKED":""));
         printf("<BR>\n");

         printf("<B>x</B><INPUT TYPE=\"CHECKBOX\" NAME=\"ShowX\" VALUE=\"1\"%s><SUP>+</SUP>", (pEnvironment.cShowX==1?" CHECKED":""));
         printf("<INPUT TYPE=\"CHECKBOX\" NAME=\"ShowX2\" VALUE=\"1\"%s><SUP>2+</SUP>", (pEnvironment.cShowX2==1?" CHECKED":""));
         printf("<INPUT TYPE=\"CHECKBOX\" NAME=\"ShowX3\" VALUE=\"1\"%s><SUP>3+</SUP>", (pEnvironment.cShowX3==1?" CHECKED":""));
         printf("<BR>\n");

         printf("<B>y</B><INPUT TYPE=\"CHECKBOX\" NAME=\"ShowY\" VALUE=\"1\"%s><SUP>+</SUP>", (pEnvironment.cShowY==1?" CHECKED":""));
         printf("<INPUT TYPE=\"CHECKBOX\" NAME=\"ShowY2\" VALUE=\"1\"%s><SUP>2+</SUP>", (pEnvironment.cShowY2==1?" CHECKED":""));
         printf("<INPUT TYPE=\"CHECKBOX\" NAME=\"ShowY3\" VALUE=\"1\"%s><SUP>3+</SUP>", (pEnvironment.cShowY3==1?" CHECKED":""));
         printf("<BR>\n");

         printf("<B>z</B><INPUT TYPE=\"CHECKBOX\" NAME=\"ShowZ\" VALUE=\"1\"%s><SUP>+</SUP>", (pEnvironment.cShowZ==1?" CHECKED":""));
         printf("<INPUT TYPE=\"CHECKBOX\" NAME=\"ShowZ2\" VALUE=\"1\"%s><SUP>2+</SUP>", (pEnvironment.cShowZ2==1?" CHECKED":""));
         printf("<INPUT TYPE=\"CHECKBOX\" NAME=\"ShowZ3\" VALUE=\"1\"%s><SUP>3+</SUP>", (pEnvironment.cShowZ3==1?" CHECKED":""));
         printf("<BR>\n");

         printf("<B>hide H<sub>2</sub>O/NH<sub>3</sub></B> <INPUT TYPE=\"CHECKBOX\" NAME=\"ShowH2OLoss\" VALUE=\"1\"%s>\n", (pEnvironment.cShowH2OLoss==0?" CHECKED":""));

         printf("    <INPUT TYPE=\"HIDDEN\" NAME=\"Pep\" VALUE=\"%s\">\n", pEnvironment.szPeptide);
      }
      printf("<BR><B>zoom 112-122</B> <INPUT TYPE=\"CHECKBOX\" NAME=\"Zoom113\" VALUE=\"1\"%s>\n", (pEnvironment.bZoom113==1?" CHECKED":""));
      printf("<BR><B>zoom 124-133</B> <INPUT TYPE=\"CHECKBOX\" NAME=\"Zoom126\" VALUE=\"1\"%s>\n", (pEnvironment.bZoom126==1?" CHECKED":""));

      // printf("    <INPUT TYPE=\"HIDDEN\" NAME=\"File\" VALUE=\"%s\">\n", pEnvironment.szInputFile);
      printf("    <INPUT TYPE=\"HIDDEN\" NAME=\"Dta\" VALUE=\"%s\">\n", pEnvironment.szFullPathInputFile);
      printf("    <INPUT TYPE=\"HIDDEN\" NAME=\"PepMass\" VALUE=\"%0.6f\">\n", pEnvironment.dPepMass);

      for (i=0; i<pEnvironment.iLenPeptide; i++)
         if (pEnvironment.pdModPeptide[i] != 0.0)
            printf("    <INPUT TYPE=\"HIDDEN\" NAME=\"Mod%d\" VALUE=\"%0.6f\">\n", i+1, pEnvironment.pdModPeptide[i]); 
      if (pEnvironment.pdModNC[0] != 0.0)
         printf("    <INPUT TYPE=\"HIDDEN\" NAME=\"ModN\" VALUE=\"%0.6f\">\n", pEnvironment.pdModNC[0]); 
      if (pEnvironment.pdModNC[1] != 0.0)
         printf("    <INPUT TYPE=\"HIDDEN\" NAME=\"ModC\" VALUE=\"%0.6f\">\n", pEnvironment.pdModNC[1]); 


      /*
       * Imagemap link
       */
      sprintf(szImageMapLink, "MatchTol=%0.3f&amp;IntensityZoom=%0.2f&amp;MassType=%d&amp;NumAxis=%d&amp;LabelType=%d",
         pEnvironment.dMatchTol,
         pEnvironment.dIntensityZoom,
         pEnvironment.iMassTypeFragment,
         pEnvironment.iNumAxis,
         pEnvironment.iLabelType);

      if (pEnvironment.iLenPeptide>0)
      {
         if (pEnvironment.cShowA)
            sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;ShowA=%d", pEnvironment.cShowA);
         if (pEnvironment.cShowA2)
            sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;ShowA2=%d", pEnvironment.cShowA2);
         if (pEnvironment.cShowA3)
            sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;ShowA3=%d", pEnvironment.cShowA3);

         if (pEnvironment.cShowB)
            sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;ShowB=%d", pEnvironment.cShowB);
         if (pEnvironment.cShowB2)
            sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;ShowB2=%d", pEnvironment.cShowB2);
         if (pEnvironment.cShowB3)
            sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;ShowB3=%d", pEnvironment.cShowB3);

         if (pEnvironment.cShowC)
            sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;ShowC=%d", pEnvironment.cShowA);
         if (pEnvironment.cShowC2)
            sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;ShowC2=%d", pEnvironment.cShowA2);
         if (pEnvironment.cShowC3)
            sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;ShowC3=%d", pEnvironment.cShowA3);

         if (pEnvironment.cShowX)
            sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;ShowX=%d", pEnvironment.cShowX);
         if (pEnvironment.cShowX2)
            sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;ShowX2=%d", pEnvironment.cShowX2);
         if (pEnvironment.cShowX3)
            sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;ShowX3=%d", pEnvironment.cShowX3);

         if (pEnvironment.cShowY)
            sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;ShowY=%d", pEnvironment.cShowY);
         if (pEnvironment.cShowY2)
            sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;ShowY2=%d", pEnvironment.cShowY2);
         if (pEnvironment.cShowY3)
            sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;ShowY3=%d", pEnvironment.cShowY3);

         if (pEnvironment.cShowZ)
            sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;ShowZ=%d", pEnvironment.cShowZ);
         if (pEnvironment.cShowZ2)
            sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;ShowZ2=%d", pEnvironment.cShowZ2);
         if (pEnvironment.cShowZ3)
            sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;ShowZ3=%d", pEnvironment.cShowZ3);

         sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;ShowH20Loss=%d", pEnvironment.cShowH2OLoss);
         sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;Pep=%s", pEnvironment.szPeptide);
      }
      sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;Zoom113=%d", pEnvironment.bZoom113);
      sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;Zoom126=%d", pEnvironment.bZoom126);

      sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;Dta=%s", pEnvironment.szFullPathInputFile);
      sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;PepMass=%0.6f", pEnvironment.dPepMass);

      for (i=0; i<pEnvironment.iLenPeptide; i++)
         if (pEnvironment.pdModPeptide[i] != 0.0)
            sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;Mod%d=%0.6f", i+1, pEnvironment.pdModPeptide[i]);
      if (pEnvironment.pdModNC[0] != 0.0)
         sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;ModN=%0.6f", pEnvironment.pdModNC[0]);
      if (pEnvironment.pdModNC[1] != 0.0)
         sprintf(szImageMapLink+strlen(szImageMapLink), "&amp;ModC=%0.6f", pEnvironment.pdModNC[1]);

      /*
       * end Imagemap link
       */

      if (pEnvironment.bPhospho)
      {
         printf("    <INPUT TYPE=\"HIDDEN\" NAME=\"Phos\" VALUE=\"1\">\n");
         printf("<BR><B>Neutral:</B> <INPUT TYPE=\"CHECKBOX\" NAME=\"RemoveMods\" VALUE=\"1\"%s>\n",
               (pEnvironment.bRemoveMods==1?" CHECKED":""));
      }

      printf("<CENTER>\n");
      printf("    <INPUT TYPE=\"SUBMIT\" VALUE=\"GO\"><BR>\n");
      printf("</CENTER>\n");

      printf("</FORM></TD>\n");
   }

   printf("<TD VALIGN=TOP CLASS=\"formentry\">\n");
   printf("<DIV CLASS=\"banner_cid\">\n");

   if ( (pStr=findRightmostPathSeperator(pEnvironment.szInputFile)) )
      strcpy(szPrintName, pStr+1);
   else
      strcpy(szPrintName, pEnvironment.szInputFile+1);

   ramp_fileoffset_t *index_=NULL;
   int iAnalysisLastScan;

   if (iFileType == 2)
      index_ = readIndex(fp_, getIndexOffset(fp_), &(iAnalysisLastScan));

   CREATE_IMAGE(ppIn, fp_, iFileType, fpPlot, szImageFile, szPlotFile, szPrintName, iCharge, index_, iAnalysisLastScan);

   if (strlen(pEnvironment.szPeptide)>0)
      printf("<TT><FONT COLOR=\"ff8700\">%s</FONT>, MH+ %0.4f, m/z %0.4f<BR>%s</TT></DIV>\n",
         pEnvironment.szPeptide, pEnvironment.dPepMass, (pEnvironment.dPepMass+(iCharge-1)*PROTON_MASS) / iCharge, szPrintName);
   else
      printf("<TT>%s</TT></DIV>\n", szPrintName);

   if (pEnvironment.iXmax==0)
      iInterval = (int) (((int)(pEnvironment.dMaxPeakMass/100.0) + 1)*100);
   else
      iInterval = pEnvironment.iXmax - pEnvironment.iXmin;

   iInterval /= 10;

   translate_absolute_filesystem_path_to_relative_webserver_root_path(szImageFile);
   
   if (pEnvironment.bPDF)
      printf("<IMG SRC=\"%s\" BORDER=0>\n", makeTmpPNGFileSrcRef(szImageFile).c_str());
   else
   printf("<IMG SRC=\"%s\" USEMAP=\"#spectrum_map\" BORDER=0>\n", makeTmpPNGFileSrcRef(szImageFile).c_str());

   if (!pEnvironment.bPDF)
   {
      printf("   <MAP NAME=\"spectrum_map\">\n");
      sprintf(szImageMapXrange, "&amp;Xmin=%d&amp;Xmax=%d", 0, 0);
      printf("      <AREA SHAPE=\"rect\" COORDS=\"0,0,20,20\" HREF=\"%s?%s%s&amp;Zoom113=0&amp;Zoom126=0\">\n", szScriptName, szImageMapLink, szImageMapXrange);
      printf("      <AREA SHAPE=\"rect\" COORDS=\"492,0,512,20\" HREF=\"%s?%s%s&amp;Zoom113=0&amp;Zoom126=0\">\n", szScriptName, szImageMapLink, szImageMapXrange);

      if ((pEnvironment.iXmax==0 || (pEnvironment.iXmax - pEnvironment.iXmin >= 30)) && !pEnvironment.bPDF)
      {
         sprintf(szImageMapXrange, "&amp;Xmin=%d&amp;Xmax=%d", pEnvironment.iXmin, 2*iInterval+pEnvironment.iXmin);
         printf("      <AREA SHAPE=\"rect\" COORDS=\"20,0,48,288\" HREF=\"%s?%s%s\">\n", szScriptName, szImageMapLink, szImageMapXrange);
         sprintf(szImageMapXrange, "&amp;Xmin=%d&amp;Xmax=%d", pEnvironment.iXmin, 3*iInterval+pEnvironment.iXmin);
         printf("      <AREA SHAPE=\"rect\" COORDS=\"49,0,95,288\" HREF=\"%s?%s%s\">\n", szScriptName, szImageMapLink, szImageMapXrange);
         sprintf(szImageMapXrange, "&amp;Xmin=%d&amp;Xmax=%d", 1*iInterval+pEnvironment.iXmin, 4*iInterval+pEnvironment.iXmin);
         printf("      <AREA SHAPE=\"rect\" COORDS=\"96,0,142,288\" HREF=\"%s?%s%s\">\n", szScriptName, szImageMapLink, szImageMapXrange);
         sprintf(szImageMapXrange, "&amp;Xmin=%d&amp;Xmax=%d", 2*iInterval+pEnvironment.iXmin, 5*iInterval+pEnvironment.iXmin);
         printf("      <AREA SHAPE=\"rect\" COORDS=\"142,0,189,435\" HREF=\"%s?%s%s\">\n", szScriptName, szImageMapLink, szImageMapXrange);
         sprintf(szImageMapXrange, "&amp;Xmin=%d&amp;Xmax=%d", 3*iInterval+pEnvironment.iXmin, 6*iInterval+pEnvironment.iXmin);
         printf("      <AREA SHAPE=\"rect\" COORDS=\"190,0,236,288\" HREF=\"%s?%s%s\">\n", szScriptName, szImageMapLink, szImageMapXrange);
         sprintf(szImageMapXrange, "&amp;Xmin=%d&amp;Xmax=%d", 4*iInterval+pEnvironment.iXmin, 7*iInterval+pEnvironment.iXmin);
         printf("      <AREA SHAPE=\"rect\" COORDS=\"237,0,283,288\" HREF=\"%s?%s%s\">\n", szScriptName, szImageMapLink, szImageMapXrange);
         sprintf(szImageMapXrange, "&amp;Xmin=%d&amp;Xmax=%d", 5*iInterval+pEnvironment.iXmin, 8*iInterval+pEnvironment.iXmin);
         printf("      <AREA SHAPE=\"rect\" COORDS=\"284,0,330,288\" HREF=\"%s?%s%s\">\n", szScriptName, szImageMapLink, szImageMapXrange);
         sprintf(szImageMapXrange, "&amp;Xmin=%d&amp;Xmax=%d", 6*iInterval+pEnvironment.iXmin, 9*iInterval+pEnvironment.iXmin);
         printf("      <AREA SHAPE=\"rect\" COORDS=\"331,0,377,288\" HREF=\"%s?%s%s\">\n", szScriptName, szImageMapLink, szImageMapXrange);
         sprintf(szImageMapXrange, "&amp;Xmin=%d&amp;Xmax=%d", 7*iInterval+pEnvironment.iXmin, 10*iInterval+pEnvironment.iXmin);
         printf("      <AREA SHAPE=\"rect\" COORDS=\"378,0,424,288\" HREF=\"%s?%s%s\">\n", szScriptName, szImageMapLink, szImageMapXrange);
         sprintf(szImageMapXrange, "&amp;Xmin=%d&amp;Xmax=%d", 8*iInterval+pEnvironment.iXmin, 10*iInterval+pEnvironment.iXmin);
         printf("      <AREA SHAPE=\"rect\" COORDS=\"425,0,491,288\" HREF=\"%s?%s%s\">\n", szScriptName, szImageMapLink, szImageMapXrange);
      }
      printf("   </MAP>\n");
      printf("<BR><TT><FONT>click image to zoom in; click top corners to zoom out");
      if (iFileType==2)
         printf(" &nbsp;&nbsp;&nbsp; [<a href=\"javascript:toggle('_show');\">precursor&#177</a>]");
   }

   if (iFileType==2)
   {
      /*
       * Try printing out precursor ion trace from mzXML file
       */
      szImageFileMS[strlen(szImageFileMS)-4]=0;
      strcat(szImageFileMS, "-ms.png");

      szPlotFile[strlen(szPlotFile)-3]=0;
      strcat(szPlotFile, "-ms.gp");

      if ( (fpPlot=fopen(szPlotFile, "w"))!=NULL)
      {
         CREATE_PRECURSOR_IMAGE(ppIn, fp_, fpPlot, szImageFileMS, szPlotFile,
               (pEnvironment.dPepMass + (iCharge-1)*pdMassAA['h'])/iCharge, index_, iAnalysisLastScan);

         translate_absolute_filesystem_path_to_relative_webserver_root_path(szImageFileMS);
   
         printf("<P><div id='_show' style=\"display:block;\">\n");
         printf("<IMG SRC=\"%s\" BORDER=0>", makeTmpPNGFileSrcRef(szImageFileMS).c_str());
         printf("<BR>zoomed in precursor plot; T=theoretical m/z, A=acquired m/z");
         printf("</div>\n");
      }
      printf("</FONT></TT>\n");
   }

   if (index_ != NULL)  /* done with index */
      delete index_;

   printf("</TD>\n");

   if (pEnvironment.iLenPeptide>0)
   {
      printf("<TD VALIGN=TOP ALIGN=MIDDLE CLASS=\"formentry\">\n");
      DISPLAY_IONS();

      /* list out modified residues and masses */
      printf("<BR><TT>");
      if (pEnvironment.pdModNC[0]!=0.0)
         printf("N-term:%+0.2f ", pEnvironment.pdModNC[0]);
   
      for (i=0; i<pEnvironment.iLenPeptide; i++)
         if (pEnvironment.pdModPeptide[i]!=0.0)
            printf("%c(%d):%+0.2f ", pEnvironment.szPeptide[i], i+1, pEnvironment.pdModPeptide[i]);
   
      if (pEnvironment.pdModNC[1]!=0.0)
         printf("C-term:%+0.2f ", pEnvironment.pdModNC[1]);
   
      printf("</TT></TD>\n");
   }

   printf("</TR>\n");

   fflush(stdout);

   if (pEnvironment.bExpect)
   {
      printf("<TR><TD></TD><TD>");
      DISPLAY_EXPECT();
      printf("</TD><TD></TD></TR>\n");
   }

   printf("</TABLE>\n");

   printf("<hr noshade/>\n");
   printf("<h6>%s by J.Eng (c) ISB 2001<br/>\n", TITLE);
   printf("(%s)</h6>\n", szTPPVersionInfo);

   printf("</BODY></HTML>\n");
                            
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

} /*main*/


void EXTRACT_CGI_QUERY(void)
{
   char *pRequestType,
        *pQS,
        szWord[1024];
   int  i;

   int  bAddNterm=0;
   int  bAddCterm=0;
   double dOldModMass[256],
          dModMass1=0.0,
          dModMass2=0.0,
          dModMass3=0.0;

   for (i=0; i<256; i++)
      dOldModMass[i]=0.0;
   for (i=0; i<MAX_PEPTIDE_LEN; i++)
      pEnvironment.pcMod[i]='0';

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

//    printf("Word=%s<BR>\n", szWord);

      if (!strcmp(szWord, "File"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%s", pEnvironment.szInputFile);
      }
      else if (!strcmp(szWord, "MassType"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%d", &(pEnvironment.iMassTypeFragment));
      }
      else if (!strcmp(szWord, "Dta"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         strcpy(pEnvironment.szInputFile, szWord);
      }
      else if (!strcmp(szWord, "Phos"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         pEnvironment.bPhospho=1;
      }
      else if (!strcmp(szWord, "RemoveMods"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%d", &(pEnvironment.bRemoveMods));
      }
      else if (!strcmp(szWord, "Pep"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%s", pEnvironment.szPeptide);
      }
      else if (!strcmp(szWord, "Prot"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%s", pEnvironment.szProtein);
      }
      else if (!strcmp(szWord, "NumAxis"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%d", &(pEnvironment.iNumAxis));
         if (pEnvironment.iNumAxis<1)
            pEnvironment.iNumAxis=1;
         else if (pEnvironment.iNumAxis>2)
            pEnvironment.iNumAxis=2;
      }
      else if (!strcmp(szWord, "LabelType"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%d", &(pEnvironment.iLabelType));
         if (pEnvironment.iLabelType<0)
            pEnvironment.iLabelType=0;
         else if (pEnvironment.iLabelType>3)
            pEnvironment.iLabelType=3;
      }
      else if (!strcmp(szWord, "Xmin"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%d", &(pEnvironment.iXmin));
      }
      else if (!strcmp(szWord, "Xmax"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%d", &(pEnvironment.iXmax));
      }
      else if (!strcmp(szWord, "ImgScale"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%lf", &(pEnvironment.dImageScale));
      }
      else if (!strcmp(szWord, "MatchTol"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%lf", &(pEnvironment.dMatchTol));
      }
      else if (!strcmp(szWord, "IntensityZoom"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%lf", &(pEnvironment.dIntensityZoom));
      }
      else if (!strcmp(szWord, "PepMass"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%lf", &(pEnvironment.dPepMass));
      }
      else if (!strcmp(szWord, "Zoom113"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%d", &(pEnvironment.bZoom113));
      }
      else if (!strcmp(szWord, "Zoom126"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%d", &(pEnvironment.bZoom126));
      }
      else if (!strcmp(szWord, "Debug"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         pEnvironment.bDebugging=1;
      }
      else if (!strcmp(szWord, "Expect"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         pEnvironment.bExpect=1;
      }
      else if (!strcmp(szWord, "PDF"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         pEnvironment.bPDF=1;
      }
      else if (!strcmp(szWord, "Score"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%lf", &(pEnvironment.dScore));
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
               sscanf(szWord, "%lf", &(pEnvironment.pdModNC[0])); /* nterm mod */
            else
               sscanf(szWord, "%lf", &(pEnvironment.pdModNC[1])); /* cterm mod */
         }
         else
         {
            sscanf(szMod, "%d", &iIndex);
            iIndex -= 1;

            getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
            sscanf(szWord, "%lf", &(pEnvironment.pdModPeptide[iIndex]));
         }

      }
      else if (!strncmp(szWord, "Show", 4))
      {
         int iLen=strlen(szWord);

         if (iLen==5 && szWord[4]=='A')
            pEnvironment.cShowA=1;
         else if (iLen==5 && szWord[4]=='B')
            pEnvironment.cShowB=1;
         else if (iLen==5 && szWord[4]=='C')
            pEnvironment.cShowC=1;
         else if (iLen==5 && szWord[4]=='X')
            pEnvironment.cShowX=1;
         else if (iLen==5 && szWord[4]=='Y')
            pEnvironment.cShowY=1;
         else if (iLen==5 && szWord[4]=='Z')
            pEnvironment.cShowZ=1;

         else if (iLen==6 && szWord[4]=='A' && szWord[5]=='2')
            pEnvironment.cShowA2=1;
         else if (iLen==6 && szWord[4]=='B' && szWord[5]=='2')
            pEnvironment.cShowB2=1;
         else if (iLen==6 && szWord[4]=='C' && szWord[5]=='2')
            pEnvironment.cShowC2=1;
         else if (iLen==6 && szWord[4]=='X' && szWord[5]=='2')
            pEnvironment.cShowX2=1;
         else if (iLen==6 && szWord[4]=='Y' && szWord[5]=='2')
            pEnvironment.cShowY2=1;
         else if (iLen==6 && szWord[4]=='Z' && szWord[5]=='2')
            pEnvironment.cShowZ2=1;

         else if (iLen==6 && szWord[4]=='A' && szWord[5]=='3')
            pEnvironment.cShowA3=1;
         else if (iLen==6 && szWord[4]=='B' && szWord[5]=='3')
            pEnvironment.cShowB3=1;
         else if (iLen==6 && szWord[4]=='C' && szWord[5]=='3')
            pEnvironment.cShowC3=1;
         else if (iLen==6 && szWord[4]=='X' && szWord[5]=='3')
            pEnvironment.cShowX3=1;
         else if (iLen==6 && szWord[4]=='Y' && szWord[5]=='3')
            pEnvironment.cShowY3=1;
         else if (iLen==6 && szWord[4]=='Z' && szWord[5]=='3')
            pEnvironment.cShowZ3=1;

         else if (iLen==11 && !strcmp(szWord+4, "NH3Loss"))
            pEnvironment.cShowNH3Loss=1;
         else if (iLen==11 && !strcmp(szWord+4, "H2OLoss"))
            pEnvironment.cShowH2OLoss=0;

         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
      }

      /*
       * Following are deprecated options - for historical compatibility only
       */
      else if (!strcmp(szWord, "Nterm"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
            sscanf(szWord, "%lf", &(pEnvironment.pdModNC[0]));

         /*
          * Nterm just refers to additional modification mass whereas
          * pcModNC wants the entire new modified mass specified.  So need to
          * note this and add in original, unmodified terminus back after
          * INITIALIZE_MASS below.
          */
         bAddNterm=1;

      }
      else if (!strcmp(szWord, "Cterm"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
            sscanf(szWord, "%lf", &(pEnvironment.pdModNC[1]));
         bAddCterm=1;
      }
      else if (!strncmp(szWord, "Mass", 4))
      {
         double dTmpMass=0.0;
         char cWhichAA=szWord[4];

         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%lf", &dTmpMass);

         dOldModMass[cWhichAA]=dTmpMass;
      }
      else if (!strcmp(szWord, "DSite"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         strcpy(pEnvironment.pcMod, szWord);
      }
      else if (!strcmp(szWord, "DMass1"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%lf", &dModMass1);
      }
      else if (!strcmp(szWord, "DMass2"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%lf", &dModMass2);
      }
      else if (!strcmp(szWord, "DMass3"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
         sscanf(szWord, "%lf", &dModMass3);
      }

      /*
       * End deprecated options
       */

      else
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
      }
   }

   // fixPath does bad things here when the directory exists but the .dta file does not
   // hence this workaround although I'm tempted to just uncomment use of fixPath here.
   char szTmp[SIZE_FILE];
   strcpy(szTmp, pEnvironment.szInputFile);
   fixPath(pEnvironment.szInputFile,1); // pretty up the path seperators etc  - expect existence
   if (!strcmp(szTmp+strlen(szTmp)-4, ".dta") && strcmp(pEnvironment.szInputFile+strlen(pEnvironment.szInputFile)-4, ".dta"))
      strcpy(pEnvironment.szInputFile, szTmp);

   INITIALIZE_MASS(pdMassAA, pEnvironment.iMassTypeFragment);

   /*
    * Force old attributes to new system
    */
   for (i=0; pEnvironment.szPeptide[i]; i++)
   {
      if (pEnvironment.pcMod[i]=='1')
         pEnvironment.pdModPeptide[i] += dModMass1;
      else if (pEnvironment.pcMod[i]=='2')
         pEnvironment.pdModPeptide[i] += dModMass2;
      else if (pEnvironment.pcMod[i]=='3')
         pEnvironment.pdModPeptide[i] += dModMass3;

      if (dOldModMass[pEnvironment.szPeptide[i]]!=0.0)
         pEnvironment.pdModPeptide[i] += dOldModMass[pEnvironment.szPeptide[i]];
      else if (pEnvironment.pdModPeptide[i]!=0.0 && pEnvironment.pcMod[i]!='0')
         pEnvironment.pdModPeptide[i] += pdMassAA[pEnvironment.szPeptide[i]];
   }

   if (bAddNterm)
   {
      pEnvironment.pdModNC[0] += pdMassAA['h'];
   }
   if (bAddCterm)
   {
      pEnvironment.pdModNC[1] += pdMassAA['o'] + pdMassAA['h'] + pdMassAA['h'];
   }

} /*EXTRACT_CGI_QUERY*/


void INITIALIZE(void)
{
   int i;

   for (i=0; i<MAX_NUM_FRAGMENT_IONS; i++)
   { 
      pFragmentIons[i].szIon[0]='\0';
      pFragmentIons[i].dMass=0.0;
      pFragmentIons[i].iType=0;
      pFragmentIons[i].bUsed=FALSE;
   }

   for (i=0; i<MAX_NUM_LABELIONS; i++)
   { 
      pLabelIons[i].szLabel[0]='\0';
      pLabelIons[i].dMass=0.0;
      pLabelIons[i].dIntensity=0.0;
   }

   pEnvironment.iLenPeptide=0;
   pEnvironment.iLabelType=0;
   pEnvironment.iNumAxis=1;
   pEnvironment.iXmin=0;
   pEnvironment.iXmax=0;
   pEnvironment.bPhospho=0;
   pEnvironment.bRemoveMods=0;
   pEnvironment.bZoom113=0;
   pEnvironment.bZoom126=0;
   pEnvironment.bDebugging=0;
   pEnvironment.bExpect=0;

   pEnvironment.cShowA=0;
   pEnvironment.cShowA2=0;
   pEnvironment.cShowA3=0;
   pEnvironment.cShowB=0;
   pEnvironment.cShowB2=0;
   pEnvironment.cShowB3=0;
   pEnvironment.cShowC=0;
   pEnvironment.cShowC2=0;
   pEnvironment.cShowC3=0;
   pEnvironment.cShowX=0;
   pEnvironment.cShowX2=0;
   pEnvironment.cShowX3=0;
   pEnvironment.cShowY=0;
   pEnvironment.cShowY2=0;
   pEnvironment.cShowY3=0;
   pEnvironment.cShowZ=0;
   pEnvironment.cShowZ2=0;
   pEnvironment.cShowZ3=0;
   pEnvironment.cShowNH3Loss=0;
   pEnvironment.cShowH2OLoss=1;

   pEnvironment.szInputFile[0]='\0';
   pEnvironment.szPeptide[0]='\0';
   pEnvironment.szProtein[0]='\0';

   pEnvironment.dPepMass=0.0;
   pEnvironment.dMatchTol=MASS_TOL;
   pEnvironment.dIntensityZoom=1.0;
   pEnvironment.dImageScale=1.0;

   memset(pEnvironment.pdModPeptide, 0, sizeof(pEnvironment.pdModPeptide));
   memset(pEnvironment.pdModNC, 0, sizeof(pEnvironment.pdModNC));
   memset(pdMassAA, 0, sizeof(pdMassAA));

   for (i=0; i<MAX_PEPTIDE_LEN; i++)
   {
      pEnvironment.pdModPeptide[i]=0.0;
   }
} /*INITIALIZE*/


void CALC_IONS(void)
{
   int i;
   double dAion = 0.0,
          dBion = 0.0,
          dCion = 0.0,
          dDion = 0.0,
          dVion = 0.0,
          dWion = 0.0,
          dXion = 0.0,
          dYion = 0.0,
          dZion = 0.0,
          dNterm = pdMassAA['h'],
          dCterm = pdMassAA['o'] + pdMassAA['h'];
 
   double dMass_nh,
          dMass_nh3,
          dMass_co;
 
   dMass_nh = pdMassAA['n'] + pdMassAA['h'];
   dMass_nh3 = pdMassAA['n'] + pdMassAA['h'] + pdMassAA['h'] + pdMassAA['h'];
   dMass_co = pdMassAA['c'] + pdMassAA['o'];
 
   if (pEnvironment.pdModNC[0] != 0.0)
      dNterm = pEnvironment.pdModNC[0];

   if (pEnvironment.pdModNC[1] != 0.0)
      dCterm = pEnvironment.pdModNC[1];

   dBion = dNterm - pdMassAA['h'] + PROTON_MASS;
   dYion = pEnvironment.dPepMass - dNterm + PROTON_MASS;

   /* correct y-ion calculation assuming neutral loss of phosphate */
   if (pEnvironment.bRemoveMods)
   {
      for (i=0; i<pEnvironment.iLenPeptide; i++)
      {
         if (pEnvironment.pdModPeptide[i]!=0.0)
            dYion -= (pEnvironment.pdModPeptide[i] - pdMassAA[pEnvironment.szPeptide[i]] +  pdMassAA['h']+pdMassAA['h']+pdMassAA['o']);
      }
   }

   for (i=0; i<pEnvironment.iLenPeptide; i++)
   {
      if (i<pEnvironment.iLenPeptide-1)
      {
         if (pEnvironment.pdModPeptide[i]==0.0 || pEnvironment.bRemoveMods)
         {
            dBion += pdMassAA[pEnvironment.szPeptide[i]];

            if (pEnvironment.bRemoveMods && pEnvironment.pdModPeptide[i]!=0.0)
               dBion -= 18.0;
         }
         else
            dBion += pEnvironment.pdModPeptide[i];

         dAion = dBion - dMass_co;
         dCion = dBion + dMass_nh3;

         if (pEnvironment.cShowA)
         {
            pFragmentIons[iFragmentIonCt].dMass=dAion;
            pFragmentIons[iFragmentIonCt].iType=0;
            sprintf(pFragmentIons[iFragmentIonCt].szIon, "a%d+", i+1);
            iFragmentIonCt++;

            if (pEnvironment.cShowNH3Loss)
            {
               pFragmentIons[iFragmentIonCt].dMass=dAion-17.0;
               pFragmentIons[iFragmentIonCt].iType=0;
               sprintf(pFragmentIons[iFragmentIonCt].szIon, "[a%d+]", i+1);
               iFragmentIonCt++;
            }
            if (pEnvironment.cShowH2OLoss)
            {
               pFragmentIons[iFragmentIonCt].dMass=dAion-17.5;
               pFragmentIons[iFragmentIonCt].iType=0;
               sprintf(pFragmentIons[iFragmentIonCt].szIon, "<a%d+>", i+1);
               iFragmentIonCt++;
            }
         }
         if (pEnvironment.cShowA2)
         {
            pFragmentIons[iFragmentIonCt].dMass=(dAion+pdMassAA['h'])/2.0;
            pFragmentIons[iFragmentIonCt].iType=0;
            sprintf(pFragmentIons[iFragmentIonCt].szIon, "a%d++", i+1);
            iFragmentIonCt++;
         }
         if (pEnvironment.cShowA3)
         {
            pFragmentIons[iFragmentIonCt].dMass=(dAion+pdMassAA['h']+pdMassAA['h'])/3.0;
            pFragmentIons[iFragmentIonCt].iType=0;
            sprintf(pFragmentIons[iFragmentIonCt].szIon, "a%d+++", i+1);
            iFragmentIonCt++;
         }
   
         if (pEnvironment.cShowB)
         {
            pFragmentIons[iFragmentIonCt].dMass=dBion;
            pFragmentIons[iFragmentIonCt].iType=1;
            sprintf(pFragmentIons[iFragmentIonCt].szIon, "b%d+", i+1);
            iFragmentIonCt++;

            if (pEnvironment.cShowNH3Loss)
            {
               pFragmentIons[iFragmentIonCt].dMass=dBion-17.0;
               pFragmentIons[iFragmentIonCt].iType=1;
               sprintf(pFragmentIons[iFragmentIonCt].szIon, "[b%d+]", i+1);
               iFragmentIonCt++;
            }
            if (pEnvironment.cShowH2OLoss)
            {
               pFragmentIons[iFragmentIonCt].dMass=dBion-17.5;
               pFragmentIons[iFragmentIonCt].iType=1;
               sprintf(pFragmentIons[iFragmentIonCt].szIon, "<b%d+>", i+1);
               iFragmentIonCt++;
            }
         }
         if (pEnvironment.cShowB2)
         {
            pFragmentIons[iFragmentIonCt].dMass=(dBion+pdMassAA['h'])/2.0;
            pFragmentIons[iFragmentIonCt].iType=1;
            sprintf(pFragmentIons[iFragmentIonCt].szIon, "b%d++", i+1);
            iFragmentIonCt++;
         }
         if (pEnvironment.cShowB3)
         {
            pFragmentIons[iFragmentIonCt].dMass=(dBion+pdMassAA['h']+pdMassAA['h'])/3.0;
            pFragmentIons[iFragmentIonCt].iType=1;
            sprintf(pFragmentIons[iFragmentIonCt].szIon, "b%d+++", i+1);
            iFragmentIonCt++;
         }

         if (pEnvironment.cShowC)
         {
            pFragmentIons[iFragmentIonCt].dMass=dCion;
            pFragmentIons[iFragmentIonCt].iType=2;
            sprintf(pFragmentIons[iFragmentIonCt].szIon, "c%d+", i+1);
            iFragmentIonCt++;
         }
         if (pEnvironment.cShowC2)
         {
            pFragmentIons[iFragmentIonCt].dMass=(dCion+pdMassAA['h'])/2.0;
            pFragmentIons[iFragmentIonCt].iType=2;
            sprintf(pFragmentIons[iFragmentIonCt].szIon, "c%d++", i+1);
            iFragmentIonCt++;
         }
         if (pEnvironment.cShowC3)
         {
            pFragmentIons[iFragmentIonCt].dMass=(dCion+pdMassAA['h']+pdMassAA['h'])/3.0;
            pFragmentIons[iFragmentIonCt].iType=2;
            sprintf(pFragmentIons[iFragmentIonCt].szIon, "c%d+++", i+1);
            iFragmentIonCt++;
         }
      }

      if (i>0)
      {
         if (pEnvironment.pdModPeptide[i-1]==0.0 || pEnvironment.bRemoveMods)
            dYion -= pdMassAA[pEnvironment.szPeptide[i-1]];
         else
            dYion -= pEnvironment.pdModPeptide[i-1];

         dXion = dYion + dMass_co - (pdMassAA['h'] + pdMassAA['h']);
         dZion = dYion - dMass_nh3 + pdMassAA['h'];

         if (pEnvironment.cShowX)
         {
            pFragmentIons[iFragmentIonCt].dMass=dXion;
            pFragmentIons[iFragmentIonCt].iType=6;
            sprintf(pFragmentIons[iFragmentIonCt].szIon, "x%d+", pEnvironment.iLenPeptide-i);
            iFragmentIonCt++;
         }
         if (pEnvironment.cShowX2)
         {
            pFragmentIons[iFragmentIonCt].dMass=(dXion+pdMassAA['h'])/2.0;
            pFragmentIons[iFragmentIonCt].iType=6;
            sprintf(pFragmentIons[iFragmentIonCt].szIon, "x%d++", pEnvironment.iLenPeptide-i);
            iFragmentIonCt++;
         }
         if (pEnvironment.cShowX3)
         {
            pFragmentIons[iFragmentIonCt].dMass=(dXion+pdMassAA['h']+pdMassAA['h'])/3.0;
            pFragmentIons[iFragmentIonCt].iType=6;
            sprintf(pFragmentIons[iFragmentIonCt].szIon, "x%d+++", pEnvironment.iLenPeptide-i);
            iFragmentIonCt++;
         }

         if (pEnvironment.cShowY)
         {
            pFragmentIons[iFragmentIonCt].dMass=dYion;
            pFragmentIons[iFragmentIonCt].iType=7;
            sprintf(pFragmentIons[iFragmentIonCt].szIon, "y%d+", pEnvironment.iLenPeptide-i);
            iFragmentIonCt++;

            if (pEnvironment.cShowNH3Loss)
            {
               pFragmentIons[iFragmentIonCt].dMass=dYion-17.0;
               pFragmentIons[iFragmentIonCt].iType=7;
               sprintf(pFragmentIons[iFragmentIonCt].szIon, "[y%d+]", pEnvironment.iLenPeptide-i);
               iFragmentIonCt++;
            }
            if (pEnvironment.cShowH2OLoss)
            {
               pFragmentIons[iFragmentIonCt].dMass=dYion-17.5;
               pFragmentIons[iFragmentIonCt].iType=7;
               sprintf(pFragmentIons[iFragmentIonCt].szIon, "<y%d+>", pEnvironment.iLenPeptide-i);
               iFragmentIonCt++;
            }
         }
         if (pEnvironment.cShowY2)
         {
            pFragmentIons[iFragmentIonCt].dMass=(dYion+pdMassAA['h'])/2.0;
            pFragmentIons[iFragmentIonCt].iType=7;
            sprintf(pFragmentIons[iFragmentIonCt].szIon, "y%d++", pEnvironment.iLenPeptide-i);
            iFragmentIonCt++;
         }
         if (pEnvironment.cShowY3)
         {
            pFragmentIons[iFragmentIonCt].dMass=(dYion+pdMassAA['h']+pdMassAA['h'])/3.0;
            pFragmentIons[iFragmentIonCt].iType=7;
            sprintf(pFragmentIons[iFragmentIonCt].szIon, "y%d+++", pEnvironment.iLenPeptide-i);
            iFragmentIonCt++;
         }

         if (pEnvironment.cShowZ)
         {
            pFragmentIons[iFragmentIonCt].dMass=dZion;
            pFragmentIons[iFragmentIonCt].iType=8;
            sprintf(pFragmentIons[iFragmentIonCt].szIon, "z%d+", pEnvironment.iLenPeptide-i);
            iFragmentIonCt++;
         }
         if (pEnvironment.cShowZ2)
         {
            pFragmentIons[iFragmentIonCt].dMass=(dZion+pdMassAA['h'])/2.0;
            pFragmentIons[iFragmentIonCt].iType=8;
            sprintf(pFragmentIons[iFragmentIonCt].szIon, "z%d++", pEnvironment.iLenPeptide-i);
            iFragmentIonCt++;
         }
         if (pEnvironment.cShowZ3)
         {
            pFragmentIons[iFragmentIonCt].dMass=(dZion+pdMassAA['h']+pdMassAA['h'])/3.0;
            pFragmentIons[iFragmentIonCt].iType=8;
            sprintf(pFragmentIons[iFragmentIonCt].szIon, "z%d+++", pEnvironment.iLenPeptide-i);
            iFragmentIonCt++;
         }
      }
   }

} /*CALC_IONS */


void DISPLAY_IONS(void)
{
   int    i,
          iLenMinus1;
   double dAion = 0.0,
          dBion = 0.0,
          dCion = 0.0,
          dDion = 0.0,
          dVion = 0.0,
          dWion = 0.0,
          dXion = 0.0,
          dYion = 0.0,
          dZion = 0.0,
          dNterm = pdMassAA['h'],
          dCterm = pdMassAA['o'] + pdMassAA['h'];
 
   double dMass_nh,
          dMass_nh3,
          dMass_co;


   dMass_nh = pdMassAA['n'] + pdMassAA['h'];
   dMass_nh3 = pdMassAA['n'] + pdMassAA['h'] + pdMassAA['h'] + pdMassAA['h'];
   dMass_co = pdMassAA['c'] + pdMassAA['o'];
 
   if (pEnvironment.pdModNC[0] != 0.0)
      dNterm = pEnvironment.pdModNC[0];

   if (pEnvironment.pdModNC[1] != 0.0)
      dCterm = pEnvironment.pdModNC[1];

   dBion = dNterm - pdMassAA['h'] + PROTON_MASS;
   dYion = pEnvironment.dPepMass - dNterm + PROTON_MASS; 

   /* correct y-ion calculation assuming neutral loss of phosphate */
   if (pEnvironment.bRemoveMods)
   {
      for (i=0; i<pEnvironment.iLenPeptide; i++)
      {
         if (pEnvironment.pdModPeptide[i]!=0.0)
         {
            dYion -= (pEnvironment.pdModPeptide[i] - pdMassAA[pEnvironment.szPeptide[i]] +  pdMassAA['h']+pdMassAA['h']+pdMassAA['o']);
         }
      }
   }
 
   dXion = 0.0;
   dZion = 0.0;
 
   printf("\n");
   printf("   <TABLE BORDER=2 CELLPADDING=\"2\">\n");
   printf("      <TR CLASS=\"graybox\" ALIGN=\"CENTER\">");

   if (pEnvironment.cShowA)
      printf("<TH><B>a<SUP>+</SUP></B></TH>");
   if (pEnvironment.cShowA2)
      printf("<TH><B>a<SUP>2+</SUP></B></TH>");
   if (pEnvironment.cShowA3)
      printf("<TH><B>a<SUP>3+</SUP></B></TH>");
   if (pEnvironment.cShowB)
      printf("<TH><B>b<SUP>+</SUP></B></TH>");
   if (pEnvironment.cShowB2)
      printf("<TH><B>b<SUP>2+</SUP></B></TH>");
   if (pEnvironment.cShowB3)
      printf("<TH><B>b<SUP>3+</SUP></B></TH>");
   if (pEnvironment.cShowC)
      printf("<TH><B>c<SUP>+</SUP></B></TH>");
   if (pEnvironment.cShowC2)
      printf("<TH><B>c<SUP>2+</SUP></B></TH>");
   if (pEnvironment.cShowC3)
      printf("<TH><B>c<SUP>3+</SUP></B></TH>");

   printf("<TH><B>#</B></TH>");
   printf("<TH><B>AA</B></TH>");
   printf("<TH><B>#</B></TH>");

   if (pEnvironment.cShowX)
      printf("<TH><B>x<SUP>+</SUP></B></TH>");
   if (pEnvironment.cShowX2)
      printf("<TH><B>x<SUP>2+</SUP></B></TH>");
   if (pEnvironment.cShowX3)
      printf("<TH><B>x<SUP>3+</SUP></B></TH>");
   if (pEnvironment.cShowY)
      printf("<TH><B>y<SUP>+</SUP></B></TH>");
   if (pEnvironment.cShowY2)
      printf("<TH><B>y<SUP>2+</SUP></B></TH>");
   if (pEnvironment.cShowY3)
      printf("<TH><B>y<SUP>3+</SUP></B></TH>");
   if (pEnvironment.cShowZ)
      printf("<TH><B>z<SUP>+</SUP></B></TH>");
   if (pEnvironment.cShowZ2)
      printf("<TH><B>z<SUP>2+</SUP></B></TH>");
   if (pEnvironment.cShowZ3)
      printf("<TH><B>z<SUP>3+</SUP></B></TH>");
   printf("</TR>\n");

   iLenMinus1=pEnvironment.iLenPeptide-1;
   for (i=0; i<pEnvironment.iLenPeptide; i++)
   {
      printf("      <TR ALIGN=\"RIGHT\">");
      if (i<iLenMinus1)
      {
         if (pEnvironment.pdModPeptide[i]==0.0 || pEnvironment.bRemoveMods)
         {
            dBion += pdMassAA[pEnvironment.szPeptide[i]];

            if (pEnvironment.bRemoveMods && pEnvironment.pdModPeptide[i]!=0.0)
               dBion -=  pdMassAA['h']+pdMassAA['h']+pdMassAA['o'];
         }
         else
            dBion += pEnvironment.pdModPeptide[i];

         dAion = dBion - dMass_co;
         dCion = dBion + dMass_nh3;


         if (pEnvironment.cShowA)
            HIGHLIGHT(dAion, 0);
         if (pEnvironment.cShowA2)
            HIGHLIGHT((dAion+pdMassAA['h'])/2.0, 0);
         if (pEnvironment.cShowA3)
            HIGHLIGHT((dAion+pdMassAA['h']+pdMassAA['h'])/3.0, 0);
         if (pEnvironment.cShowB)
            HIGHLIGHT(dBion, 1);
         if (pEnvironment.cShowB2)
            HIGHLIGHT((dBion+pdMassAA['h'])/2.0, 1);
         if (pEnvironment.cShowB3)
            HIGHLIGHT((dBion+pdMassAA['h']+pdMassAA['h'])/3.0, 1);
         if (pEnvironment.cShowC)
            HIGHLIGHT(dCion, 2);
         if (pEnvironment.cShowC2)
            HIGHLIGHT((dCion+pdMassAA['h'])/2.0, 2);
         if (pEnvironment.cShowC3)
            HIGHLIGHT((dCion+pdMassAA['h']+pdMassAA['h'])/3.0, 2);
      }
      else
      {
         if (pEnvironment.cShowA)
            printf("<TD BGCOLOR=\"%s\">&nbsp;</TD>", MASSBKGRND);
         if (pEnvironment.cShowA2)
            printf("<TD BGCOLOR=\"%s\">&nbsp;</TD>", MASSBKGRND);
         if (pEnvironment.cShowA3)
            printf("<TD BGCOLOR=\"%s\">&nbsp;</TD>", MASSBKGRND);
         if (pEnvironment.cShowB)
            printf("<TD BGCOLOR=\"%s\">&nbsp;</TD>", MASSBKGRND);
         if (pEnvironment.cShowB2)
            printf("<TD BGCOLOR=\"%s\">&nbsp;</TD>", MASSBKGRND);
         if (pEnvironment.cShowB3)
            printf("<TD BGCOLOR=\"%s\">&nbsp;</TD>", MASSBKGRND);
         if (pEnvironment.cShowC)
            printf("<TD BGCOLOR=\"%s\">&nbsp;</TD>", MASSBKGRND);
         if (pEnvironment.cShowC2)
            printf("<TD BGCOLOR=\"%s\">&nbsp;</TD>", MASSBKGRND);
         if (pEnvironment.cShowC3)
            printf("<TD BGCOLOR=\"%s\">&nbsp;</TD>", MASSBKGRND);
      }

      printf("<TD CLASS=\"graybox\"><TT><B>%d</B></TT></TD>", i+1);

      /*
       * print out peptide sequence down middle of table
       */
      if (pEnvironment.pdModPeptide[i]!=0.0 && !(pEnvironment.bRemoveMods))
         printf("<TD ALIGN=\"CENTER\" BGCOLOR=\"%s\"><TT><B>%c", "#FFFFAA", pEnvironment.szPeptide[i]);
      else
         printf("<TD ALIGN=\"CENTER\" CLASS=\"graybox\"><TT><B>%c", pEnvironment.szPeptide[i]);
/*
      if (pEnvironment.pcMod[i]=='1')
         printf("\'");
      if (pEnvironment.pcMod[i]=='2')
         printf("\"");
      if (pEnvironment.pcMod[i]=='3')
         printf("~");
*/
      printf("</B></TT></TD>");

      printf("<TD CLASS=\"graybox\"><TT><B>%d<B></TT></TD>", pEnvironment.iLenPeptide-i);

     if (i>0)
      {
         if (pEnvironment.pdModPeptide[i-1]==0.0 || pEnvironment.bRemoveMods)
         {
            dYion -= pdMassAA[pEnvironment.szPeptide[i-1]];

            if (pEnvironment.bRemoveMods && pEnvironment.pdModPeptide[i]!=0.0)
               dYion += pdMassAA['h']+pdMassAA['h']+pdMassAA['o'];
         }
         else
            dYion -= pEnvironment.pdModPeptide[i-1];

         dXion = dYion + dMass_co - (pdMassAA['h'] + pdMassAA['h']);
         dZion = dYion - dMass_nh3 + pdMassAA['h'];

         if (pEnvironment.cShowX)
            HIGHLIGHT(dXion, 7);
         if (pEnvironment.cShowX2)
            HIGHLIGHT((dXion+pdMassAA['h'])/2.0, 7);
         if (pEnvironment.cShowX3)
            HIGHLIGHT((dXion+pdMassAA['h']+pdMassAA['h'])/3.0, 7);
         if (pEnvironment.cShowY)
            HIGHLIGHT(dYion, 8);
         if (pEnvironment.cShowY2)
            HIGHLIGHT((dYion+pdMassAA['h'])/2.0, 8);
         if (pEnvironment.cShowY3)
            HIGHLIGHT((dYion+pdMassAA['h']+pdMassAA['h'])/3.0, 8);
         if (pEnvironment.cShowZ)
            HIGHLIGHT(dZion, 9);
         if (pEnvironment.cShowZ2)
            HIGHLIGHT((dZion+pdMassAA['h'])/2.0, 9);
         if (pEnvironment.cShowZ3)
            HIGHLIGHT((dZion+pdMassAA['h']+pdMassAA['h'])/3.0, 9);
      }
      else
      {
         if (pEnvironment.cShowX)
            printf("<TD BGCOLOR=\"%s\">&nbsp;</TD>", MASSBKGRND);
         if (pEnvironment.cShowX2)
            printf("<TD BGCOLOR=\"%s\">&nbsp;</TD>", MASSBKGRND);
         if (pEnvironment.cShowX3)
            printf("<TD BGCOLOR=\"%s\">&nbsp;</TD>", MASSBKGRND);
         if (pEnvironment.cShowY)
            printf("<TD BGCOLOR=\"%s\">&nbsp;</TD>", MASSBKGRND);
         if (pEnvironment.cShowY2)
            printf("<TD BGCOLOR=\"%s\">&nbsp;</TD>", MASSBKGRND);
         if (pEnvironment.cShowY3)
            printf("<TD BGCOLOR=\"%s\">&nbsp;</TD>", MASSBKGRND);
         if (pEnvironment.cShowZ)
            printf("<TD BGCOLOR=\"%s\">&nbsp;</TD>", MASSBKGRND);
         if (pEnvironment.cShowZ2)
            printf("<TD BGCOLOR=\"%s\">&nbsp;</TD>", MASSBKGRND);
         if (pEnvironment.cShowZ3)
            printf("<TD BGCOLOR=\"%s\">&nbsp;</TD>", MASSBKGRND);
      }

      printf("</TR>\n");
   }

   printf("    </TABLE>\n\n");
 
} /*DISPLAY_IONS */


void HIGHLIGHT(double dMass,
         int iWhichIonSeries)
{
   int    i,
          bPresent=FALSE;

   for (i=0; i<MAX_NUM_LABELIONS; i++)
   {
      if ( fabs(pLabelIons[i].dMass - dMass)<=pEnvironment.dMatchTol)
      {
         bPresent=TRUE;
      }
   }

   if (bPresent)
   {
      if (iWhichIonSeries<=3)
         printf("<TD CLASS=\"glyco\"><TT><B>");
      else
         printf("<TD CLASS=\"seq\"><TT><B>");
      printf("%0.4f", dMass);
      printf("</FONT></B></TT></TD>");
   }
   else
      printf("<TD BGCOLOR=\"%s\"><TT>%0.4f</TT></TD>", MASSBKGRND, dMass);

} /*HIGHLIGHT*/


void CREATE_IMAGE(FILE *ppIn,
        RAMPFILE *fp_,
        int  iFileType,
        FILE *fpPlot,
        char *szImageFile,
        char *szPlotFile,
        char *szPrintName,  /* last argument passed just for temp tof-tof data - mascot generic format starts with pps_ */
        int  iCharge,
        ramp_fileoffset_t *index_,
        int iAnalysisLastScan)
{
   int    i,
          ii,
          iMid=0,
          iScanNum=0,
          bPrecursorSet=0;           /* track to only label 1 precursor peak */
   char   szBuf[SIZE_BUF],
          szTmpDataFile[SIZE_FILE],   /* raw spectrum plot */
          szTmpDataFile2[SIZE_FILE],  /* n-terminal ions i.e. a,b,c-ions */
          szTmpDataFile3[SIZE_FILE],  /* c-terminal ions i.e. x,y,z-ions */
          szTmpDataFile4[SIZE_FILE],  /* phospho loss: -80, -98, -49, -32.7 */
          szTmpDataFile5[SIZE_FILE],  /* for immonium ions and precursor ion */
          szCommand[SIZE_BUF];
   double dMaxMass,    /* used in phosho neutral loss plot */
          dMaxInten,   /* used in phospho plot only ; this duplicates dMaxIntensity1 if only 1 axis is being printed */
          dMaxIntensity1,
          dMaxIntensity2,
          dPepMass,
          dLabelPad=0.0;
   double d113=0.0, d114=0.0, d115=0.0, d116=0.0,  /* track 113-121 peak intensities of bZoom113 set */
          d117=0.0, d118=0.0, d119=0.0, d121=0.0;
   double d126=0.0, d127=0.0, d128=0.0, d129=0.0,  /* track 126-131 peak intensities of bZoom126 set */
          d130=0.0, d131=0.0;
   FILE   *fpTmpData,    /* for raw spectrum plot */
          *fpTmpData2,   /* for matched fragment ion peak */
          *fpTmpData3,
          *fpTmpData4=NULL,
          *fpTmpData5;

   struct ScanHeaderStruct scanHeader;
  

   if (pEnvironment.iXmin!=0 || pEnvironment.iXmax!=0)
   {
      if (pEnvironment.iXmin >= pEnvironment.iXmax)
      {
         printf(" Your plot range (%d-%d) is not valid.<BR>\n", pEnvironment.iXmin, pEnvironment.iXmax);
         return;
      }
   }

   fprintf(fpPlot, "set terminal png\n");
   fprintf(fpPlot, "set output \"%s\"\n", szImageFile);
   fprintf(fpPlot, "set size %0.3f,%0.3f\n", IMAGEWIDTH*pEnvironment.dImageScale, IMAGEHEIGHT*pEnvironment.dImageScale);
   fprintf(fpPlot, "set origin 0.0,0.0\n");
   fprintf(fpPlot, "set nokey\n");
   fprintf(fpPlot, "set border 1\n");
   fprintf(fpPlot, "set xtics axis nomirror font \"tiny\"\n");
   fprintf(fpPlot, "set mxtics\n");
   fprintf(fpPlot, "set noytics\n");

   if (iFileType != 2)  /* is not mzXML */
   {
      char *fgot=fgets(szBuf, SIZE_BUF, ppIn);  /* skip first line of .dta file */
      sscanf(szBuf, "%lf %d", &dPepMass, &iCharge);   /* override previous charge from .dta name */
   }

   sprintf(szTmpDataFile, "%s.spec", szImageFile);     /* for raw spectrum plot */
   if ((fpTmpData=fopen(szTmpDataFile, "w"))==NULL)
   {
      printf(" Error - can't open %s to write.\n\n", szTmpDataFile);
      exit(EXIT_FAILURE);
   }
   fprintf(fpTmpData, "0.0 0.0\n");  /* include dummy data point for gnuplot in case file is empty */

   sprintf(szTmpDataFile2, "%s.spec2", szImageFile);    /* for matched n-terminal fragment ion peaks */
   if ((fpTmpData2=fopen(szTmpDataFile2, "w"))==NULL)
   {
      printf(" Error - can't open %s to write.\n\n", szTmpDataFile2);
      exit(EXIT_FAILURE);
   }
   fprintf(fpTmpData2, "0.0 0.0\n");  /* include dummy data point for gnuplot in case file is empty */

   sprintf(szTmpDataFile3, "%s.spec3", szImageFile);    /* for matched c-terminal fragment ion peaks */
   if ((fpTmpData3=fopen(szTmpDataFile3, "w"))==NULL)
   {
      printf(" Error - can't open %s to write.\n\n", szTmpDataFile3);
      exit(EXIT_FAILURE);
   }
   fprintf(fpTmpData3, "0.0 0.0\n");  /* include dummy data point for gnuplot in case file is empty */

   if (pEnvironment.bPhospho)
   {
      sprintf(szTmpDataFile4, "%s.spec4", szImageFile);
      if ((fpTmpData4=fopen(szTmpDataFile4, "w"))==NULL)
      {
         printf(" Error - can't open %s to write.\n\n", szTmpDataFile4);
         exit(EXIT_FAILURE);
      }
      fprintf(fpTmpData4, "0.0 0.0\n");  /* include dummy data point for gnuplot in case file is empty */
   }

   sprintf(szTmpDataFile5, "%s.spec5", szImageFile);    /* for immonium and precursor ions */
   if ((fpTmpData5=fopen(szTmpDataFile5, "w"))==NULL)
   {
      printf(" Error - can't open %s to write.\n\n", szTmpDataFile5);
      exit(EXIT_FAILURE);
   }
   fprintf(fpTmpData5, "0.0 0.0\n");  /* include dummy data point for gnuplot in case file is empty */

   if (iFileType == 2)
   {
      /* mzXML file: read file index and parse scan # from encoded .dta file*/
      iScanNum = GET_SCANNUM_FROM_DTA_NAME(pEnvironment.szInputFile);
   }

   if (pEnvironment.iNumAxis==2)
   {
      if (pEnvironment.iXmin!=0 || pEnvironment.iXmax!=0)
      {
         iMid = pEnvironment.iXmin + (pEnvironment.iXmax-pEnvironment.iXmin)/2 ;
      }
      else if (iFileType != 2)
      {
         dMaxMass=0.0;
         while (fgets(szBuf, SIZE_BUF, ppIn))
         {
            double dMass=0.0,
                   dIntensity=0.0;

            sscanf(szBuf, "%lf %lf", &dMass, &dIntensity);
            if (dMass>dMaxMass)
               dMaxMass=dMass;
         }

         iMid = (int)(dMaxMass/2.0 + 0.5) ;

         if (iFileType == 1)
         {
            /*
             * rewind() of pipe does not work so need to close & reopen pipe
             */
            pclose(ppIn);
            sprintf(szCommand, "tar --wildcards  -xzOf %s \"%s\"", pEnvironment.szTarFile, pEnvironment.szInputFile);

            if ( (ppIn=tpplib_popen(szCommand, "r")) == NULL)
            {
               printf(" Error - can't read from command %s\n", szCommand);
               exit(EXIT_FAILURE);
            }
         }
         else
            rewind(ppIn);

         char *fgot=fgets(szBuf, SIZE_BUF, ppIn);  /* skip first line of .dta file */
      }
      else
      {
         scanHeader.highMZ = 0.0;

         readHeader(fp_, index_[iScanNum], &scanHeader);

         /* get MaxMass from mzXML header */
         iMid = (int)(scanHeader.highMZ/2.0 + 0.5) ;

         if (iMid == 0)  /* in case highMZ is not set */
            iMid = (int)(scanHeader.precursorMZ);
         /*
         printf("iMid= %d<P>\n", iMid);
         */
      }
   }

  /*
   * parse through mass/intensity pairs
   */
   dMaxMass=0.0;        /* now dMaxMass is the mass of the base peak */
   dMaxInten=0.0;

   dMaxIntensity1=0.0;
   dMaxIntensity2=0.0;

   if (iFileType != 2)
   {
      while (fgets(szBuf, SIZE_BUF, ppIn))
      {
         double dMass=0.0,
                dIntensity=0.0;
   
         sscanf(szBuf, "%lf %lf", &dMass, &dIntensity);
         
         READ_PEAK(&d113, &d114, &d115, &d116, &d117, &d118, &d119, &d121,
               &d126, &d127, &d128, &d129, &d130, &d131,
               &dMaxMass, &dMaxInten, &dMaxIntensity1, &dMaxIntensity2,
               dMass, dIntensity, fpTmpData, szBuf, iMid);
      }
   }
   else
   {
      if (iScanNum > iAnalysisLastScan)
      {
         printf(" Error - can't parse scan number (got %d, only %d in file)\n",
               iScanNum, iAnalysisLastScan);
         exit(EXIT_FAILURE);
      }
      scanHeader.msLevel = readMsLevel(fp_, index_[iScanNum]);

      if (scanHeader.msLevel > 1)
      {
         RAMPREAL *pPeaks;
         int n = 0;

         /*
          * Open a scan
          */
         pPeaks = readPeaks(fp_, index_[iScanNum]);

         while (pPeaks != NULL && pPeaks[n] != -1)
         {
            RAMPREAL fMass;
            RAMPREAL fInten;
            double dMass=0.0,
                   dIntensity=0.0;

            fMass = pPeaks[n];
            n++;
            fInten = pPeaks[n];
            n++;

            dMass = fMass;
            dIntensity = fInten;

            /* write the peak to szBuf for printing out in READ_PEAKS */
            sprintf(szBuf, "%f %f\n", fMass, fInten);

            READ_PEAK(&d113, &d114, &d115, &d116, &d117, &d118, &d119, &d121,
                  &d126, &d127, &d128, &d129, &d130, &d131,
                  &dMaxMass, &dMaxInten, &dMaxIntensity1, &dMaxIntensity2,
                  dMass, dIntensity, fpTmpData, szBuf, iMid);
         }
         if (pPeaks != NULL)
            free(pPeaks);
      }
      else
      {
         printf(" Error - scan %d is an MS1 scan in the mzXML file %s<P>\n", iScanNum, pEnvironment.szMZXMLFile);
         exit(1);
      }
   }

   fclose(fpTmpData);

   if (dMaxIntensity1==0.0)
      dMaxIntensity1=0.01;
   if (dMaxIntensity2==0.0)
      dMaxIntensity2=0.01;

   if (pEnvironment.dIntensityZoom!=0.0)
   {
      dMaxIntensity1 /= pEnvironment.dIntensityZoom;
      dMaxIntensity2 /= pEnvironment.dIntensityZoom;
   }

   if (pEnvironment.iNumAxis==2)
      dLabelPad=0.02*(dMaxIntensity1<dMaxIntensity2?dMaxIntensity1:dMaxIntensity2);
   else
      dLabelPad=0.02*dMaxIntensity1;


   if (pEnvironment.bPhospho)
   {
      if (fabs(dPepMass - 80.0 - (dMaxMass*iCharge - (iCharge-1))) < 3.0) /*pEnvironment.dMatchTol*/
      {
         fprintf(fpTmpData4, "%f %f\n", dMaxMass, dMaxInten);
         fprintf(fpPlot, "set label \"%s\" at %0.4f,%0.4e left rotate font \"small\"\n", 
            "-80", dMaxMass, dMaxInten+dLabelPad);
      }
      if (fabs(dPepMass - 98.0 - (dMaxMass*iCharge - (iCharge-1))) < 3.0) /*pEnvironment.dMatchTol*/
      {
         fprintf(fpTmpData4, "%f %f\n", dMaxMass, dMaxInten);
         fprintf(fpPlot, "set label \"%s\" at %0.4f,%0.4e left rotate font \"small\"\n", 
            "-98", dMaxMass, dMaxInten+dLabelPad);
      }
      if (fabs(dPepMass - 49.0 - (dMaxMass*iCharge - (iCharge-1))) < 3.0) /*pEnvironment.dMatchTol*/
      {
         fprintf(fpTmpData4, "%f %f\n", dMaxMass, dMaxInten);
         fprintf(fpPlot, "set label \"%s\" at %0.4f,%0.4e left rotate font \"small\"\n", 
            "-49", dMaxMass, dMaxInten+dLabelPad);
      }
      if (fabs(dPepMass - 32.7 - (dMaxMass*iCharge - (iCharge-1))) < 3.0) /*pEnvironment.dMatchTol*/
      {
         fprintf(fpTmpData4, "%f %f\n", dMaxMass, dMaxInten);
         fprintf(fpPlot, "set label \"%s\" at %0.4f,%0.4e left rotate font \"small\"\n", 
            "-32.7", dMaxMass, dMaxInten+dLabelPad);
      }
/*
      if (fabs(dPepMass - 116.0 - (dMaxMass*iCharge - (iCharge-1))) < 3.0)
      {
         fprintf(fpTmpData4, "%f %f\n", dMaxMass, dMaxInten);
         fprintf(fpPlot, "set label \"%s\" at %0.4f,%0.4e left rotate font \"small\"\n", 
            "-116", dMaxMass, dMaxInten+dLabelPad);
      }
      else if (fabs(dPepMass - 128.0 - (dMaxMass*iCharge - (iCharge-1))) < 3.0)
      {
         fprintf(fpTmpData4, "%f %f\n", dMaxMass, dMaxInten);
         fprintf(fpPlot, "set label \"%s\" at %0.4f,%0.4e left rotate font \"small\"\n", 
            "-128", dMaxMass, dMaxInten+dLabelPad);

      }
*/
      fclose(fpTmpData4);
   }

   qsort(pLabelIons, MAX_NUM_LABELIONS, sizeof(struct LabelIonsStruct), SORT_LABELIONS_INTEN);

   for (i=0; i<MAX_NUM_LABELIONS; i++)
   {
      for (ii=0; ii<iFragmentIonCt; ii++)
      {
         if (!pFragmentIons[ii].bUsed &&  fabs(pLabelIons[i].dMass - pFragmentIons[ii].dMass)<=pEnvironment.dMatchTol)
         {
            strcat(pLabelIons[i].szLabel, pFragmentIons[ii].szIon); 
            pFragmentIons[ii].bUsed=TRUE;
            if (pFragmentIons[ii].iType<=3)
               fprintf(fpTmpData2, "%f %f\n", pLabelIons[i].dMass, pLabelIons[i].dIntensity);
            else
               fprintf(fpTmpData3, "%f %f\n", pLabelIons[i].dMass, pLabelIons[i].dIntensity);
         }
      }

      if (fabs(pLabelIons[i].dMass-70.0)<=SMALL_MASS_TOL
            || fabs(pLabelIons[i].dMass-72.0)<=SMALL_MASS_TOL
            || fabs(pLabelIons[i].dMass-86.0)<=SMALL_MASS_TOL
            || fabs(pLabelIons[i].dMass-110.0)<=SMALL_MASS_TOL
            || fabs(pLabelIons[i].dMass-120.0)<=SMALL_MASS_TOL
            || fabs(pLabelIons[i].dMass-136.0)<=SMALL_MASS_TOL
            || fabs(pLabelIons[i].dMass-159.0)<=SMALL_MASS_TOL)
         fprintf(fpTmpData5, "%f %f\n", pLabelIons[i].dMass, pLabelIons[i].dIntensity);
      
      /* mark precursor ion */
      if (fabs(pLabelIons[i].dMass - (pEnvironment.dPepMass + (iCharge-1)*PROTON_MASS)/iCharge)<=pEnvironment.dMatchTol)
         fprintf(fpTmpData5, "%f %f\n", pLabelIons[i].dMass, pLabelIons[i].dIntensity);

      /*
       * plot out ETD precursors
       */
      if (pEnvironment.cShowC && pEnvironment.cShowZ)
      {
         int iZ;

         for (iZ=iCharge-1; iZ>=1; iZ--)
         {
            if (fabs(pLabelIons[i].dMass - (pEnvironment.dPepMass + (iCharge-1)*PROTON_MASS)/iZ)<=pEnvironment.dMatchTol)
               fprintf(fpTmpData5, "%f %f\n", pLabelIons[i].dMass, pLabelIons[i].dIntensity);
         }
      }

   }
   fclose(fpTmpData2);
   fclose(fpTmpData3);
   fclose(fpTmpData5);


   if (pEnvironment.iLabelType==0)  /* show ion labels */
   {
      for (i=0; i<MAX_NUM_LABELIONS; i++)
      {
         char szLabel[128];

         szLabel[0]='\0';

         /*
          * label fragment ion
          */
         if (strlen(pLabelIons[i].szLabel) > 0)
            sprintf(szLabel, "%s ", pLabelIons[i].szLabel);
         
         /*
          * label immonium ions; not sure why I'm enforcing a tighter fixed mass tol here
          */
         if (fabs(pLabelIons[i].dMass-70)<= pEnvironment.dMatchTol)
            sprintf(szLabel+strlen(szLabel), "P-%0.1f", pLabelIons[i].dMass);
         else if (fabs(pLabelIons[i].dMass-72) <= SMALL_MASS_TOL)
            sprintf(szLabel+strlen(szLabel), "V-%0.1f", pLabelIons[i].dMass);
         else if (fabs(pLabelIons[i].dMass-86) <= SMALL_MASS_TOL)
            sprintf(szLabel+strlen(szLabel), "IL-%0.1f", pLabelIons[i].dMass);
         else if (fabs(pLabelIons[i].dMass-110) <= SMALL_MASS_TOL)
            sprintf(szLabel+strlen(szLabel), "H-%0.1f", pLabelIons[i].dMass);
         else if (fabs(pLabelIons[i].dMass-120) <= SMALL_MASS_TOL)
            sprintf(szLabel+strlen(szLabel), "F-%0.1f", pLabelIons[i].dMass);
         else if (fabs(pLabelIons[i].dMass-136) <= SMALL_MASS_TOL)
            sprintf(szLabel+strlen(szLabel), "Y-%0.1f", pLabelIons[i].dMass);
         else if (fabs(pLabelIons[i].dMass-159) <= SMALL_MASS_TOL)
            sprintf(szLabel+strlen(szLabel), "W-%0.1f", pLabelIons[i].dMass);

         if (strlen(szLabel)>0)
            fprintf(fpPlot, "set label \"%s\" at %0.4f,%0.4e left rotate font \"small\"\n", 
               szLabel, pLabelIons[i].dMass, pLabelIons[i].dIntensity+dLabelPad);

         if (!bPrecursorSet
               && fabs(pLabelIons[i].dMass - (pEnvironment.dPepMass + (iCharge-1)*PROTON_MASS)/iCharge)<=pEnvironment.dMatchTol
               && pLabelIons[i].dIntensity >= 0.5*dMaxIntensity1)
         {
            int j;
            fprintf(fpPlot, "set label \"M");
            for (j=0; j<iCharge; j++)
               fprintf(fpPlot, "+");
            fprintf(fpPlot, "\" at %0.4f,%0.4e left rotate font \"small\"\n",
                  pLabelIons[i].dMass, pLabelIons[i].dIntensity+dLabelPad);
            bPrecursorSet=1;
         }

         /*
          * try and plot out ETD precursors
          */
         if (pEnvironment.cShowC && pEnvironment.cShowZ)
         {
            int iZ;

            for (iZ=iCharge-1; iZ>=1; iZ--)
            {
               int j;

               if (fabs(pLabelIons[i].dMass - (pEnvironment.dPepMass + (iCharge-1)*PROTON_MASS)/iZ)<=pEnvironment.dMatchTol
                  && pLabelIons[i].dIntensity >= 0.25*dMaxIntensity1)
               {
                  fprintf(fpPlot, "set label \"M");
                  for (j=0; j<iCharge; j++)
                     fprintf(fpPlot, "+");
                  for (j=0; j<iCharge-iZ; j++)
                     fprintf(fpPlot, "'");
                  fprintf(fpPlot, "\" at %0.4f,%0.4e left rotate font \"small\"\n",
                        pLabelIons[i].dMass, pLabelIons[i].dIntensity+dLabelPad);
               }
            }
         }
      }
   }
   else if (pEnvironment.iLabelType==1) /* show matched masses */
   {
      if (pEnvironment.bZoom113)
      {
         double dMax=0.0;
         /*
          * calculate & display relative abundance of 113-121 peaks
          */
         if (d113 > dMax)
            dMax=d113;
         if (d114 > dMax)
            dMax=d114;
         if (d115 > dMax)
            dMax=d115;
         if (d116 > dMax)
            dMax=d116;
         if (d117 > dMax)
            dMax=d117;
         if (d118 > dMax)
            dMax=d118;
         if (d119 > dMax)
            dMax=d119;
         if (d121 > dMax)
            dMax=d121;

         /*
          * normalize abundance
          */
         if (dMax != 0.0)
         {
            d113 = 100.0*d113/dMax;
            d114 = 100.0*d114/dMax;
            d115 = 100.0*d115/dMax;
            d116 = 100.0*d116/dMax;
            d117 = 100.0*d117/dMax;
            d118 = 100.0*d118/dMax;
            d119 = 100.0*d119/dMax;
            d121 = 100.0*d121/dMax;
         }
      }
      if (pEnvironment.bZoom126)
      {
         double dMax=0.0;
         /*
          * calculate & display relative abundance of 126-131 peaks
          */
         if (d126 > dMax)
            dMax=d126;
         if (d127 > dMax)
            dMax=d127;
         if (d128 > dMax)
            dMax=d128;
         if (d129 > dMax)
            dMax=d129;
         if (d130 > dMax)
            dMax=d130;
         if (d131 > dMax)
            dMax=d131;

         /*
          * normalize abundance
          */
         if (dMax != 0.0)
         {
            d126 = 100.0*d126/dMax;
            d127 = 100.0*d127/dMax;
            d128 = 100.0*d128/dMax;
            d129 = 100.0*d129/dMax;
            d130 = 100.0*d130/dMax;
            d131 = 100.0*d131/dMax;
         }
      }

      for (i=0; i<20; i++)
      {
         if (pLabelIons[i].dMass > 0.0)
         {
            if (pEnvironment.bZoom113 && (fabs(pLabelIons[i].dMass-113.0)<=SMALL_MASS_TOL))
            {
               fprintf(fpPlot, "set label \"%0.1f (%%%0.0f)\" at %0.4f,%0.4e left rotate font \"small\"\n", 
                  pLabelIons[i].dMass, d113, pLabelIons[i].dMass, pLabelIons[i].dIntensity+dLabelPad);
            }
            else if (pEnvironment.bZoom113 && (fabs(pLabelIons[i].dMass-114.0)<=SMALL_MASS_TOL))
            {
               fprintf(fpPlot, "set label \"%0.1f (%%%0.0f)\" at %0.4f,%0.4e left rotate font \"small\"\n", 
                  pLabelIons[i].dMass, d114, pLabelIons[i].dMass, pLabelIons[i].dIntensity+dLabelPad);
            }
            else if (pEnvironment.bZoom113 && (fabs(pLabelIons[i].dMass-115.0)<=SMALL_MASS_TOL))
            {
               fprintf(fpPlot, "set label \"%0.1f (%%%0.0f)\" at %0.4f,%0.4e left rotate font \"small\"\n", 
                  pLabelIons[i].dMass, d115, pLabelIons[i].dMass, pLabelIons[i].dIntensity+dLabelPad);
            }
            else if (pEnvironment.bZoom113 && (fabs(pLabelIons[i].dMass-116.0)<=SMALL_MASS_TOL))
            {
               fprintf(fpPlot, "set label \"%0.1f (%%%0.0f)\" at %0.4f,%0.4e left rotate font \"small\"\n", 
                  pLabelIons[i].dMass, d116, pLabelIons[i].dMass, pLabelIons[i].dIntensity+dLabelPad);
            }
            else if (pEnvironment.bZoom113 && (fabs(pLabelIons[i].dMass-117.0)<=SMALL_MASS_TOL))
            {
               fprintf(fpPlot, "set label \"%0.1f (%%%0.0f)\" at %0.4f,%0.4e left rotate font \"small\"\n", 
                  pLabelIons[i].dMass, d117, pLabelIons[i].dMass, pLabelIons[i].dIntensity+dLabelPad);
            }
            else if (pEnvironment.bZoom113 && (fabs(pLabelIons[i].dMass-118.0)<=SMALL_MASS_TOL))
            {
               fprintf(fpPlot, "set label \"%0.1f (%%%0.0f)\" at %0.4f,%0.4e left rotate font \"small\"\n", 
                  pLabelIons[i].dMass, d118, pLabelIons[i].dMass, pLabelIons[i].dIntensity+dLabelPad);
            }
            else if (pEnvironment.bZoom113 && (fabs(pLabelIons[i].dMass-119.0)<=SMALL_MASS_TOL))
            {
               fprintf(fpPlot, "set label \"%0.1f (%%%0.0f)\" at %0.4f,%0.4e left rotate font \"small\"\n", 
                  pLabelIons[i].dMass, d119, pLabelIons[i].dMass, pLabelIons[i].dIntensity+dLabelPad);
            }
            else if (pEnvironment.bZoom113 && (fabs(pLabelIons[i].dMass-121.0)<=SMALL_MASS_TOL))
            {
               fprintf(fpPlot, "set label \"%0.1f (%%%0.0f)\" at %0.4f,%0.4e left rotate font \"small\"\n", 
                  pLabelIons[i].dMass, d121, pLabelIons[i].dMass, pLabelIons[i].dIntensity+dLabelPad);
            }

            else if (pEnvironment.bZoom126 && (fabs(pLabelIons[i].dMass-126.13)<=SMALL_MASS_TOL))
            {
               fprintf(fpPlot, "set label \"%0.1f (%%%0.0f)\" at %0.4f,%0.4e left rotate font \"small\"\n", 
                  pLabelIons[i].dMass, d126, pLabelIons[i].dMass, pLabelIons[i].dIntensity+dLabelPad);
            }
            else if (pEnvironment.bZoom126 && (fabs(pLabelIons[i].dMass-127.13)<=SMALL_MASS_TOL))
            {
               fprintf(fpPlot, "set label \"%0.1f (%%%0.0f)\" at %0.4f,%0.4e left rotate font \"small\"\n", 
                  pLabelIons[i].dMass, d127, pLabelIons[i].dMass, pLabelIons[i].dIntensity+dLabelPad);
            }
            else if (pEnvironment.bZoom126 && (fabs(pLabelIons[i].dMass-128.13)<=SMALL_MASS_TOL))
            {
               fprintf(fpPlot, "set label \"%0.1f (%%%0.0f)\" at %0.4f,%0.4e left rotate font \"small\"\n", 
                  pLabelIons[i].dMass, d128, pLabelIons[i].dMass, pLabelIons[i].dIntensity+dLabelPad);
            }
            else if (pEnvironment.bZoom126 && (fabs(pLabelIons[i].dMass-129.14)<=SMALL_MASS_TOL))
            {
               fprintf(fpPlot, "set label \"%0.1f (%%%0.0f)\" at %0.4f,%0.4e left rotate font \"small\"\n", 
                  pLabelIons[i].dMass, d129, pLabelIons[i].dMass, pLabelIons[i].dIntensity+dLabelPad);
            }
            else if (pEnvironment.bZoom126 && (fabs(pLabelIons[i].dMass-130.14)<=SMALL_MASS_TOL))
            {
               fprintf(fpPlot, "set label \"%0.1f (%%%0.0f)\" at %0.4f,%0.4e left rotate font \"small\"\n", 
                  pLabelIons[i].dMass, d130, pLabelIons[i].dMass, pLabelIons[i].dIntensity+dLabelPad);
            }
            else if (pEnvironment.bZoom126 && (fabs(pLabelIons[i].dMass-131.14)<=SMALL_MASS_TOL))
            {
               fprintf(fpPlot, "set label \"%0.1f (%%%0.0f)\" at %0.4f,%0.4e left rotate font \"small\"\n", 
                  pLabelIons[i].dMass, d131, pLabelIons[i].dMass, pLabelIons[i].dIntensity+dLabelPad);
            }

            else
            {
               fprintf(fpPlot, "set label \"%0.1f\" at %0.4f,%0.4e left rotate font \"small\"\n", 
                  pLabelIons[i].dMass,
                  pLabelIons[i].dMass,
                  pLabelIons[i].dIntensity+dLabelPad);
            }
         }
      }
   }
   else if (pEnvironment.iLabelType==3) /* show all masses */
   {
   }

   if (pEnvironment.iNumAxis==1)
   {
      if (pEnvironment.bZoom113 || pEnvironment.bZoom126)
         fprintf(fpPlot, "set yrange [0:%0.4e]\n", dMaxIntensity1*1.40);
      else
         fprintf(fpPlot, "set yrange [0:%0.4e]\n", dMaxIntensity1*1.20);

      if (pEnvironment.iXmin!=0 || pEnvironment.iXmax!=0)
      {
         fprintf(fpPlot, "set label \"scan %d; %0.1e\" at %d.0,%0.4e font \"tiny\"\n",
               iScanNum, dMaxIntensity1, pEnvironment.iXmin, dMaxIntensity1*1.2);
         fprintf(fpPlot, "plot [%d:%d] ", pEnvironment.iXmin, pEnvironment.iXmax);
      }
      else
      {
         fprintf(fpPlot, "set label \"scan %d; %0.1e\" at 0.0,%0.4e font \"tiny\"\n",
               iScanNum, dMaxIntensity1, dMaxIntensity1*1.2);
         fprintf(fpPlot, "plot [0:] "); /* plot full x-range */
      }

      fprintf(fpPlot, "\"%s\" with impulse lc rgb \"%s\", \"%s\" with impulse lc rgb \"%s\", \"%s\" with impulse lc rgb \"%s\", \"%s\" with impulse lc rgb \"%s\"",
         szTmpDataFile, LINECOLOR_UNLABEL, szTmpDataFile5, LINECOLOR_LABEL5,
         szTmpDataFile2, LINECOLOR_LABEL2, szTmpDataFile3, LINECOLOR_LABEL3);

      if (pEnvironment.bPhospho)
         fprintf(fpPlot, ", \"%s\" with impulse lc rgb \"%s\"", szTmpDataFile4, LINECOLOR_LABEL4);
      fprintf(fpPlot, "\n");
   }
   else if (pEnvironment.iNumAxis==2)
   {
      fprintf(fpPlot, "set multiplot\n");
      fprintf(fpPlot, "set size %0.3f,%0.3f\n", IMAGEWIDTH, IMAGEHEIGHT/2.0);
      fprintf(fpPlot, "set origin %0.3f,%0.3f\n", 0.0, IMAGEHEIGHT/2.0);
      fprintf(fpPlot, "set yrange [0:%0.4e]\n", dMaxIntensity1*1.40);

      if (pEnvironment.iXmin!=0 || pEnvironment.iXmax!=0)
      {
         fprintf(fpPlot, "set label \"scan %d; %0.1e\" at %d.0,%0.4e font \"tiny\"\n",
               iScanNum, dMaxIntensity1, pEnvironment.iXmin, dMaxIntensity1*1.4);
         fprintf(fpPlot, "plot [%d:%d] ", pEnvironment.iXmin, iMid);
      }
      else
      {
         fprintf(fpPlot, "set label \"%0.1e\" at 0.0,%0.4e font \"tiny\"\n", dMaxIntensity1, dMaxIntensity1*1.4);
         fprintf(fpPlot, "plot [0:%d] ", iMid);
      }

      fprintf(fpPlot, "\"%s\" with impulse lc rgb \"%s\", \"%s\" with impulse lc rgb \"%s\", \"%s\" with impulse lc rgb \"%s\", \"%s\" with impulse lc rgb \"%s\"",
         szTmpDataFile, LINECOLOR_UNLABEL, szTmpDataFile5, LINECOLOR_LABEL5,
         szTmpDataFile2, LINECOLOR_LABEL2, szTmpDataFile3, LINECOLOR_LABEL3);

      if (pEnvironment.bPhospho)
         fprintf(fpPlot, ", \"%s\" with impulse lc rgb \"%s\"", szTmpDataFile4, LINECOLOR_LABEL4);
      fprintf(fpPlot, "\n");

      fprintf(fpPlot, "set size %0.3f,%0.3f\n", IMAGEWIDTH, IMAGEHEIGHT/2.0);
      fprintf(fpPlot, "set origin 0.0,0.0\n");
      fprintf(fpPlot, "set yrange [0:%0.4e]\n", dMaxIntensity2*1.40);

      if (pEnvironment.iXmin!=0 || pEnvironment.iXmax!=0)
      {
         fprintf(fpPlot, "set label \"scan %d; %0.1e\" at %d.01,%0.4e font \"tiny\"\n",
               iScanNum, dMaxIntensity2, iMid, dMaxIntensity2*1.4);
         fprintf(fpPlot, "plot [%d.01:%d] ", iMid, pEnvironment.iXmax);
      }
      else
      {
         fprintf(fpPlot, "set label \"%0.1e\" at %d.01,%0.4e font \"tiny\"\n", dMaxIntensity2, iMid, dMaxIntensity2*1.4);
         fprintf(fpPlot, "plot [%d.01:] ", iMid);
      }

      fprintf(fpPlot, "\"%s\" with impulse lc rgb \"%s\", \"%s\" with impulse lc rgb \"%s\", \"%s\" with impulse lc rgb \"%s\", \"%s\" with impulse lc rgb \"%s\"",
         szTmpDataFile, LINECOLOR_UNLABEL, szTmpDataFile5, LINECOLOR_LABEL5,
         szTmpDataFile2, LINECOLOR_LABEL2, szTmpDataFile3, LINECOLOR_LABEL3);

      if (pEnvironment.bPhospho)
         fprintf(fpPlot, ", \"%s\" with impulse lc rgb \"%s\"", szTmpDataFile4, LINECOLOR_LABEL4);
      fprintf(fpPlot, "\n");
   }

   fclose(fpPlot);

  /*
   * Call gnuplot to plot out the image described in the file
   */
   sprintf(szCommand, "%s %s", GNUPLOT_BINARY, szPlotFile);
   verified_system(szCommand);
   if (!pEnvironment.bDebugging) {
      verified_unlink(szPlotFile);
      verified_unlink(szTmpDataFile); 
      verified_unlink(szTmpDataFile2); 
      verified_unlink(szTmpDataFile3); 
      verified_unlink(szTmpDataFile4); 
      verified_unlink(szTmpDataFile5);
   }
} /*CREATE_IMAGE*/


void CREATE_PRECURSOR_IMAGE(FILE *ppIn,
        RAMPFILE *fp_,
        FILE *fpPlot,
        char *szImageFileMS,
        char *szPlotFile,
        double dPrecursorMZ,   /* calculated precursor mz */
        ramp_fileoffset_t *index_,
        int iAnalysisLastScan)
{
   int    iScanNum;
   char   szTmpDataFile[SIZE_FILE],   /* raw spectrum plot */
          szTmpDataFile2[SIZE_FILE],
          szTmpDataFile3[SIZE_FILE],
          szCommand[SIZE_FILE];
   double dMaxIntensity,
          dLabelPad=0.0;
   FILE   *fpTmpData,    /* for raw spectrum plot */
          *fpTmpData2,   /* mark calculated precursor mz */
          *fpTmpData3;   /* mark acquired precursor mz */

   struct ScanHeaderStruct scanHeader, scanHeaderMS;

   double dLowMass;
   double dHighMass;

#define PRE_TOL_MINUS 7.0
#define PRE_TOL_PLUS 7.0
   dLowMass = dPrecursorMZ - PRE_TOL_MINUS;
   dHighMass = dPrecursorMZ + PRE_TOL_PLUS;
  
   fprintf(fpPlot, "set terminal png\n");
   fprintf(fpPlot, "set output \"%s\"\n", szImageFileMS);
   fprintf(fpPlot, "set size %0.3f,%0.3f\n", IMAGEWIDTH*pEnvironment.dImageScale, 0.25);
   fprintf(fpPlot, "set origin 0.0,0.0\n");
   fprintf(fpPlot, "set nokey\n");
   fprintf(fpPlot, "set border 1\n");
   fprintf(fpPlot, "set xtics axis nomirror font \"tiny\"\n");
   fprintf(fpPlot, "set mxtics\n");
   fprintf(fpPlot, "set noytics\n");

   sprintf(szTmpDataFile, "%s.MS.spec", szImageFileMS);     /* for raw spectrum plot */
   if ((fpTmpData=fopen(szTmpDataFile, "w"))==NULL)
   {
      printf(" Error - can't open %s to write.\n\n", szTmpDataFile);
      exit(EXIT_FAILURE);
   }
   fprintf(fpTmpData, "0.0 0.0\n");  /* include dummy data point for gnuplot in case file is empty */

   sprintf(szTmpDataFile2, "%s.MS.spec2", szImageFileMS);
   if ((fpTmpData2=fopen(szTmpDataFile2, "w"))==NULL)
   {
      printf(" Error - can't open %s to write.\n\n", szTmpDataFile2);
      exit(EXIT_FAILURE);
   }

   sprintf(szTmpDataFile3, "%s.MS.spec3", szImageFileMS);
   if ((fpTmpData3=fopen(szTmpDataFile3, "w"))==NULL)
   {
      printf(" Error - can't open %s to write.\n\n", szTmpDataFile3);
      exit(EXIT_FAILURE);
   }

   if (1)
   {
      /* mzXML file: read file index and parse scan # from encoded .dta file*/

      iScanNum = GET_SCANNUM_FROM_DTA_NAME(pEnvironment.szInputFile);
   }


   /*
    * parse through mass/intensity pairs
    */
   dMaxIntensity=0.0;
   if (1)
   {
      int i=0;

      readHeader(fp_, index_[iScanNum], &scanHeader);
      iScanNum--;

      /*
       * loop back through scans to find MS1 scan; break if greater than 60 seconds away
       */
      while (1)
      {
         readHeader(fp_, index_[iScanNum], &scanHeaderMS);
         if (iScanNum < 0 || scanHeaderMS.msLevel == (scanHeader.msLevel - 1) || fabs(scanHeaderMS.retentionTime - scanHeader.retentionTime)>60.0)
            break;
         iScanNum--;
      }

      if (scanHeaderMS.msLevel == (scanHeader.msLevel - 1))
      {
         RAMPREAL *pPeaks;
         int n = 0;

         if (scanHeader.precursorMZ < dPrecursorMZ)
            dLowMass = scanHeader.precursorMZ - PRE_TOL_MINUS;
         if (scanHeader.precursorMZ > dPrecursorMZ)
            dHighMass = scanHeader.precursorMZ + PRE_TOL_MINUS;

         /*
          * Open a scan
          */
         pPeaks = readPeaks(fp_, index_[iScanNum]);

         while (pPeaks != NULL && pPeaks[n] != -1)
         {
            RAMPREAL fMass;
            RAMPREAL fInten;

            fMass = pPeaks[n];
            n++;
            fInten = pPeaks[n];
            n++;

            if (scanHeaderMS.msLevel == 1)
            {
               if (dLowMass<fMass && fMass<dHighMass)
               {
                  if (fInten > dMaxIntensity)
                     dMaxIntensity = fInten;

                  /*
                   * write the peak to szBuf for printing out in READ_PEAKS
                   */
                  fprintf(fpTmpData, "%f %f\n", fMass, fInten);
               }
            }
            else
            {
               if (fInten > dMaxIntensity)
                  dMaxIntensity = fInten;
               fprintf(fpTmpData, "%f %f\n", fMass, fInten);
            }
         }
         if (pPeaks != NULL)
            free(pPeaks);
      }
   }

   fclose(fpTmpData);

   if (dMaxIntensity==0.0)
      dMaxIntensity=0.01;

   fprintf(fpTmpData2, "%f %f\n", dPrecursorMZ, dMaxIntensity/5.0); /*theoretical mass*/
   fclose(fpTmpData2);
   fprintf(fpTmpData3, "%f %f\n", scanHeader.precursorMZ, dMaxIntensity/3.0); /*measured mass */
   fclose(fpTmpData3);

   fprintf(fpPlot, "set yrange [0:%0.4e]\n", dMaxIntensity);
   if (scanHeaderMS.msLevel == 1)
   {
      fprintf(fpPlot, "set xtics 1, 1 font \"tiny\"\n");
      fprintf(fpPlot, "set mxtics 10\n");
   }
   else
   {
      fprintf(fpPlot, "set xtics 0, 100 font \"tiny\"\n");
      //fprintf(fpPlot, "set mxtics 500\n");
   }

   if (scanHeaderMS.msLevel == 1)
   {
      fprintf(fpPlot, "set label \"scan %d; %0.1f mz; %0.1e\" at %d.0,%0.4e front font \"tiny\"\n",
         iScanNum, dPrecursorMZ, dMaxIntensity, (int)(dLowMass+0.5),dMaxIntensity);
   }
   else
   {
      fprintf(fpPlot, "set label \"scan %d; %0.1f mz; %0.1e\" at %d.0,%0.4e front font \"tiny\"\n",
         iScanNum, dPrecursorMZ, dMaxIntensity, 0,dMaxIntensity);
   }
   fprintf(fpPlot, "set label \"T\" at %f,%0.4e center front font \"small\"\n", dPrecursorMZ, 1.2*(dMaxIntensity/5.0));
   fprintf(fpPlot, "set label \"A\" at %f,%0.4e center front font \"small\"\n", scanHeader.precursorMZ, 1.2*(dMaxIntensity/3.0));

   if (scanHeaderMS.msLevel == 1)
      fprintf(fpPlot, "plot [%d:%d] ", (int)(dLowMass+0.5), (int)(dHighMass+0.5));
   else
      fprintf(fpPlot, "plot "); /* plot full x-range */

   fprintf(fpPlot, "\"%s\" with impulse lc rgb \"%s\"", szTmpDataFile, LINECOLOR_PRECURSOR);
   fprintf(fpPlot, ",\"%s\" with impulse lc rgb \"%s\"",  szTmpDataFile3, LINECOLOR_LABEL3);
   fprintf(fpPlot, ",\"%s\" with impulse lc rgb \"%s\"",  szTmpDataFile2, LINECOLOR_LABEL2);
   fprintf(fpPlot, "\n");

   fclose(fpPlot);

  /*
   * Call gnuplot to plot out the image described in the file
   */
   sprintf(szCommand, "%s %s", GNUPLOT_BINARY, szPlotFile);
   verified_system(szCommand); // system() with verbose error check
   if (!pEnvironment.bDebugging) {
      verified_unlink(szPlotFile);
      verified_unlink(szTmpDataFile);
      verified_unlink(szTmpDataFile2);
      verified_unlink(szTmpDataFile3);
   }
} /*CREATE_PRECURSOR_IMAGE*/


void STORE_TOP_PEAKS(double dMass,
        double dIntensity)
{
   int i,
       iLowestIndex;
   double dLowestIntensity;


  /*
   * get lowest intensity and index to replace
   */
   iLowestIndex=0;
   dLowestIntensity=pLabelIons[0].dIntensity;
   for (i=1; i<MAX_NUM_LABELIONS; i++)
   {
      if (pLabelIons[i].dIntensity < dLowestIntensity)
      {
         dLowestIntensity=pLabelIons[i].dIntensity;
         iLowestIndex=i;
      }
      if (dIntensity==0.0)
         break;
   }

   if (dIntensity>dLowestIntensity)
   {
      int bMassPresent=FALSE;
     /*
      * Check to make sure dMass is not already stored;
      * If it is, replace if dIntensity > stored inten for stored mass
      * If it isn't, store mass/intensity
      */
      for (i=0; i<MAX_NUM_LABELIONS; i++)
      {
         if (fabs(dMass - pLabelIons[i].dMass) <= pEnvironment.dMatchTol)
         {
            bMassPresent=TRUE;

           /*
            * update stored peak to largest mass
            */
            if (dIntensity > pLabelIons[i].dIntensity)
            {
               pLabelIons[i].dMass=dMass;
               pLabelIons[i].dIntensity=dIntensity;
            }
         }
      }
      if (bMassPresent==FALSE)
      {
         pLabelIons[iLowestIndex].dMass=dMass;
         pLabelIons[iLowestIndex].dIntensity=dIntensity;
      }
   }

} /*STORE_TOP_PEAKS*/


void READ_PEAK(double *d113,
      double *d114,
      double *d115,
      double *d116,
      double *d117,
      double *d118,
      double *d119,
      double *d121,
      double *d126,
      double *d127,
      double *d128,
      double *d129,
      double *d130,
      double *d131,
      double *dMaxMass,
      double *dMaxInten,
      double *dMaxIntensity1,
      double *dMaxIntensity2,
      double dMass,
      double dIntensity,
      FILE *fpTmpData,
      char *szBuf,
      int  iMid)
{
   if (pEnvironment.bZoom113)
   {
      if (fabs(dMass-113.0)<=SMALL_MASS_TOL && dIntensity > *d113)
         *d113 = dIntensity;
      if (fabs(dMass-114.0)<=SMALL_MASS_TOL && dIntensity > *d114)
         *d114 = dIntensity;
      if (fabs(dMass-115.0)<=SMALL_MASS_TOL && dIntensity > *d115)
         *d115 = dIntensity;
      if (fabs(dMass-116.0)<=SMALL_MASS_TOL && dIntensity > *d116)
         *d116 = dIntensity;
      if (fabs(dMass-117.0)<=SMALL_MASS_TOL && dIntensity > *d117)
         *d117 = dIntensity;
      if (fabs(dMass-118.0)<=SMALL_MASS_TOL && dIntensity > *d118)
         *d118 = dIntensity;
      if (fabs(dMass-119.0)<=SMALL_MASS_TOL && dIntensity > *d119)
         *d119 = dIntensity;
      if (fabs(dMass-121.0)<=SMALL_MASS_TOL && dIntensity > *d121)
         *d121 = dIntensity;
   }
   if (pEnvironment.bZoom126)
   {
      if (fabs(dMass-126.13)<=SMALL_MASS_TOL && dIntensity > *d126)
         *d126 = dIntensity;
      if (fabs(dMass-127.13)<=SMALL_MASS_TOL && dIntensity > *d127)
         *d127 = dIntensity;
      if (fabs(dMass-128.13)<=SMALL_MASS_TOL && dIntensity > *d128)
         *d128 = dIntensity;
      if (fabs(dMass-129.14)<=SMALL_MASS_TOL && dIntensity > *d129)
         *d129 = dIntensity;
      if (fabs(dMass-130.14)<=SMALL_MASS_TOL && dIntensity > *d130)
         *d130 = dIntensity;
      if (fabs(dMass-131.14)<=SMALL_MASS_TOL && dIntensity > *d131)
         *d131 = dIntensity;
   }
   
   if ((pEnvironment.iXmin==0 && pEnvironment.iXmax==0)
      || (dMass>=(double)pEnvironment.iXmin && dMass<=(double)pEnvironment.iXmax))
   {
      if (pEnvironment.bPhospho && dIntensity > *dMaxInten)
      {
         *dMaxInten = dIntensity;
         *dMaxMass = dMass;
      }
   
      fprintf(fpTmpData, "%s", szBuf);
      if (pEnvironment.iNumAxis==2)
      {
         if (dMass >= iMid)
         {
            if (dIntensity > *dMaxIntensity2)
               *dMaxIntensity2=dIntensity;
         }
         else
         {
            if (dIntensity> *dMaxIntensity1)
               *dMaxIntensity1=dIntensity;
         }
      }
      else
      {
         if (dIntensity> *dMaxIntensity1)
            *dMaxIntensity1=dIntensity;
      }
   
      STORE_TOP_PEAKS(dMass, dIntensity);
   }
   
   pEnvironment.dMaxPeakMass = dMass;
} /*READ_PEAK*/

/*
 * sort in descending order
 */
int SORT_LABELIONS_MASS(const void *p0, const void *p1)
{
   if ( ((struct LabelIonsStruct *) p0)->dMass < ((struct LabelIonsStruct *) p1)->dMass )
      return (1);
   else if ( ((struct LabelIonsStruct *) p0)->dMass > ((struct LabelIonsStruct *) p1)->dMass )
      return (-1);
   else
      return (0);

} /*SORT_LABELIONS_MASS*/


/*
 * sort in descending order
 */
int SORT_LABELIONS_INTEN(const void *p0, const void *p1)
{
   if ( ((struct LabelIonsStruct *) p0)->dIntensity < ((struct LabelIonsStruct *) p1)->dIntensity )
      return (1);
   else if ( ((struct LabelIonsStruct *) p0)->dIntensity > ((struct LabelIonsStruct *) p1)->dIntensity)
      return (-1);
   else
      return (0);
} /*SORT_LABELIONS_INTEN*/


/*
 * extract the scan number from the dta file name, which is expected be of the format
 * xxxxxxxxxxxxxx.scanstart.scanend.charge
 */
int GET_SCANNUM_FROM_DTA_NAME(const char* dtaName) {
  int scanNum = -1;
  char szTmp[500];
  char *pStr;


  // look for an X!Tandem style declaration
  for (const char *c=dtaName;c && *c && (c=strstr(c,"scan"));) {
     if (1==sscanf(c++,"scan %d (charge ", &scanNum)) {
        return scanNum;
     }
  }

  /* 
     assume dta filename format is xxxxxxxx.scanstart.scanend.charge.dta
      
     start from right to left, in order to avoid issues with messy naming schemes,
     like maldi, which may have a format like xxxxxxx.SPOT_DESIGNATOR.xxxx.xxxx.xxx.dta
  */

  strcpy(szTmp, dtaName);

  int fieldCount=4;
  while (fieldCount != 0) {
    pStr = strrchr(szTmp, '.');
    if (pStr == NULL) {
      printf(" Error - cannot get scan number from input file %s; unexpected dta name format\n", dtaName);
      exit(1);
    }
    /* 
       first time here: pStr is one before 'dta'
       second time: pStr is one before charge
       third time: pStr is one before scanend
       forth: one before scanstart (what we want)
    */
    *pStr = '\0';
    --fieldCount;
  }
  /* szTemp is now xxxxxxx.scanstartNULLxxxxxx */
  pStr++;
  sscanf(pStr, "%d", &scanNum);

  return scanNum;
}

void DISPLAY_EXPECT()
{
   char szOut[SIZE_FILE];
   char szImageFile[SIZE_FILE];
   char szHistFile[SIZE_FILE];
   char szFullPathOut[SIZE_FILE];
   char szDir[SIZE_FILE];
   char szTarFile[SIZE_FILE];
   char szGP[SIZE_FILE];
   char szCommand[512];
   char *pStr;
   struct stat statbuf;
   FILE *fp;

   szTarFile[0] = 0;

   strcpy(szDir, pEnvironment.szTarFile);
   if ( (pStr=strrchr(szDir, '/')) != NULL)
   {
      *pStr = '\0';
   }

   sprintf(szImageFile, "%s/cometplot.png", szDir);
   sprintf(szHistFile,  "%s/cometplot.hist", szDir);
   sprintf(szGP,  "%s/cometplot.gp", szDir);

   strcpy(szOut, pEnvironment.szInputFile);

   szOut[strlen(szOut)-3]='\0';
   strcat(szOut, "out");

   if (!stat(szHistFile, &statbuf))
      unlink(szHistFile);

   if (stat(pEnvironment.szTarFile, &statbuf))
   {  /* no tgz file - maybe the .out file already exists in the directory? */

      strcpy(szFullPathOut, pEnvironment.szFullPathInputFile);
      szOut[strlen(szFullPathOut)-3]='\0';
      strcat(szFullPathOut, "out");

      if (!stat(szFullPathOut,&statbuf))
          sprintf(szCommand, "cat %s", szFullPathOut);
   }
   else
      sprintf(szCommand, "tar -xzOf %s '%s'", pEnvironment.szTarFile, szOut);

   if ( (fp=tpplib_popen(szCommand, "r"))!=NULL)
   {
      FILE *fp2;
      if ( (fp2=fopen(szHistFile, "w"))!=NULL)
      {
         char szBuf[SIZE_BUF];
         while (fgets(szBuf, SIZE_BUF, fp))
         {
            if (strstr(szBuf, "HIST:"))
               fprintf(fp2, "%s", szBuf);
         }
         fclose(fp2);
      }
      fclose(fp);
   }

   if ( (fp=fopen(szHistFile, "r"))!=NULL)
   {
      fgetc(fp);
      if (!feof(fp))  /* if expectation output not empty */
      {
         fclose(fp);

         if ( (fp=fopen(szGP, "w"))!=NULL)
         {
            fprintf(fp, "set terminal png transparent\n");
            fprintf(fp, "set size 0.5\n");
            fprintf(fp, "set size square\n");
            fprintf(fp, "set output \"%s\"\n", szImageFile);
            fprintf(fp, "set title \"expectation distribution\" font \"small\"\n");
            fprintf(fp, "set xtics nomirror font \"small\"\n");
            fprintf(fp, "set ytics nomirror font \"small\"\n");
            fprintf(fp, "set xlabel \"xcorr\" font \"small\"\n");
            fprintf(fp, "set ylabel \"log(count)\" font \"small\"\n");
            fprintf(fp, "set nokey\n");
            fprintf(fp, "plot \"%s\" using 2:4 with points ps 0.5 pt 13 lc 3, \"%s\" using 2:5 with lines lc 1\n", szHistFile, szHistFile);

            fclose(fp);

            sprintf(szCommand, "%s %s ; chmod uog+rw %s", GNUPLOT_BINARY, szGP, szImageFile);
            verified_system(szCommand); // like system(), but handles multiple commands for win32

            unlink(szGP);
            unlink(szHistFile);

            printf("<center><img src=\"%s\"></center>", szImageFile);
         }
      }
      else
      {
         sprintf(szCommand, "rm -f %s", szHistFile);
         verified_system(szCommand);
      }
   }

} /*DISPLAY_EXPECT*/
