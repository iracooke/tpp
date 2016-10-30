/*
 * COMET RESULT by Jimmy Eng (c) Institute for Systems Biology, 2002
 *
 * Date Initiated:  01/02/2002
 * Purpose:  CGI program to show individual COMET search result
 *
 * 2002/01/09  Update for multipl database in output
 * 2004/07/20  File= in cgi link can have ./ or just file
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include "common/TPPVersion.h" // contains version number, name, revision



//#define BGCOLOR  "#EEB422"
#define IMAGEWIDTH  1.0
#define IMAGEHEIGHT 0.70

#define MAX_NUM_FRAGMENT_IONS 400
#define MAX_NUM_LABEL_IONS 50

#define TITLE "COMET Result View"

/*
 * -1=black  0=gray  1=red  2=brightgreen  3=blue  4=lightblue  5=violet  6=yellow
 *  7=brown  8=drkgreen  9=drkblue  10=orange
 */
#define LINECOLOR_UNLABEL   0 
#define LINECOLOR_LABEL     8 
 
#include "Comet.h"

 
struct EnvironmentStruct
{
   char szInputFile[SIZE_FILE];
   char szTarFile[SIZE_FILE];
} pEnvironment;


void EXTRACT_CGI_QUERY(void);
void INITIALIZE(void);
void READ_RESULTS(void);
void CREATE_IMAGE(char *szBaseName,
        FILE *ppIn);

#include "common/util.h"

int main(int argc, char **argv)
{
  hooks_tpp handler(argc,argv); // set up install paths etc

  /*
   * Print HTML header
   */
   printf("Content-type: text/html\n\n");
   printf("<HTML>\n");
   printf("   <HEAD>\n");
   printf("      <TITLE>%s by J.Eng (c) ISB 2002 (%s)</TITLE>\n", TITLE,szTPPVersionInfo);
/*
   printf("      <STYLE TYPE=\"text/css\">\n");
   printf("         TT { font-size: x-small }\n");
   printf("      </STYLE>\n");
*/
   printf("   </HEAD>\n");
   printf("\n");
   printf("<BODY BGCOLOR=\"#FFFFFF\" OnLoad=\"self.focus();\">\n");
   printf("<B>%s (%s)</B><BR>\n", TITLE,szTPPVersionInfo);

   INITIALIZE();
   EXTRACT_CGI_QUERY();

   READ_RESULTS();
                            
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

      if (!strcmp(szWord, "File"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
	 if (strchr(szWord,';')!=NULL) {
	   printf("GET query string failure, found illegal character ';' ...\n");
	   exit(EXIT_FAILURE);
	 }
	 sscanf(szWord, "%s", pEnvironment.szInputFile);
      }
      else if (!strcmp(szWord, "TarFile"))
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
	 if (strchr(szWord,';')!=NULL) {
	   printf("GET query string failure, found illegal character ';' ...\n");
	   exit(EXIT_FAILURE);
	 }
         sscanf(szWord, "%s", pEnvironment.szTarFile);
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
   pEnvironment.szTarFile[0]='\0';

} /*INITIALIZE*/


void READ_RESULTS(void)
{
   char   szBuf[SIZE_BUF],
          szBaseName[SIZE_FILE],
          szCommand[SIZE_BUF];
   FILE   *ppIn;

   if (!strncmp(pEnvironment.szTarFile+strlen(pEnvironment.szTarFile)-11, ".cmt.tar.gz", 11))
   {
      sprintf(szCommand, "tar -xzOf %s %s", pEnvironment.szTarFile, pEnvironment.szInputFile);
      if ( (ppIn=tpplib_popen(szCommand, "r")) == NULL)
      {
         sprintf(szCommand, "tar -xzOf %s ./%s", pEnvironment.szTarFile, pEnvironment.szInputFile);
         if ( (ppIn=tpplib_popen(szCommand, "r")) == NULL)
         {
            printf(" Error - can't read from command %s\n", szCommand);
            exit(EXIT_FAILURE);
         }
      }

      strcpy(szBaseName, pEnvironment.szTarFile);
      szBaseName[strlen(szBaseName)-11]='\0';
   }
   else if (!strncmp(pEnvironment.szTarFile+strlen(pEnvironment.szTarFile)-8, ".cmt.tar", 8))
   {
      sprintf(szCommand, "tar -xOf %s %s", pEnvironment.szTarFile, pEnvironment.szInputFile);
      if ( (ppIn=tpplib_popen(szCommand, "r")) == NULL)
      {
         sprintf(szCommand, "tar -xOf %s ./%s", pEnvironment.szTarFile, pEnvironment.szInputFile);
         if ( (ppIn=tpplib_popen(szCommand, "r")) == NULL)
         {
           printf(" Error - can't read from command %s\n", szCommand);
           exit(EXIT_FAILURE);
        }
      }

      strcpy(szBaseName, pEnvironment.szTarFile);
      szBaseName[strlen(szBaseName)-8]='\0';
   }
   else
   {
      printf(" TarFile %s does not end in .cmt.tar or .cmt.tar.gz\n\n", pEnvironment.szTarFile);
      printf(" Error - file in wrong format.\n\n");
      exit(EXIT_FAILURE);
   }

   printf("<HTML><BODY BGCOLOR=\"#FFFFFF\"><TABLE CELLPADDING=\"3\"><TR><TD BGCOLOR=\"%s\">\n", BGCOLOR);
   printf("<TABLE CELLPADDING=\"5\"><TR><TD BGCOLOR=\"#FFFFFF\"><PRE>\n");

   char *fgot=fgets(szBuf, SIZE_BUF, ppIn);  /* skip first blank line */

  /*
   * print search results
   */
   while (fgets(szBuf, SIZE_BUF, ppIn))
   {
      if (strlen(szBuf)<2)
         break;
      if (!strncmp(szBuf, "File = ", 7))
         printf("%s", szBuf);
   }
   printf("%s", szBuf);  /* print blank line */

   while (fgets(szBuf, SIZE_BUF, ppIn))
   {
      if (!strncmp(szBuf, "#distribution", 13))
         break;
      printf("%s", szBuf);
   }

   printf("</PRE></TD></TR>\n");

  /*
   * print score distribution images
   */
   CREATE_IMAGE(szBaseName, ppIn);
   pclose(ppIn);

  /*
   * print search header
   */
   printf("<TR><TD><PRE>");
   if ( (ppIn=tpplib_popen(szCommand, "r")) == NULL)
   {
      printf(" Error - can't read from command %s\n", szCommand);
      exit(EXIT_FAILURE);
   }
   while (fgets(szBuf, SIZE_BUF, ppIn))
   {
      if (strstr(szBuf, "Mass") && strstr(szBuf, "Scor") && strstr(szBuf, "Peptide"))
         break;
      printf("%s", szBuf);
   }
   pclose(ppIn);
   printf("</PRE></TD></TR>\n");

  /*
   * open comet.def and print out search parameters
   */

   printf("</TABLE>\n");
   printf("</TD></TR></TABLE></BODY></HTML>\n");

   fflush(stdout);


} /*READ_RESULTS*/


void CREATE_IMAGE(char *szBaseName_in,
        FILE *ppIn)
{
   char   szBuf[SIZE_BUF],
          szBaseName[SIZE_BUF],
          szCommand[SIZE_BUF],
          szRandom[SIZE_FILE],
          szPlotFile[SIZE_FILE],
          szImageFile[SIZE_FILE],
          szImageFile2[SIZE_FILE],
          szTmpDataFile[SIZE_FILE];
   time_t tStartTime;
   FILE   *fp;

   tStartTime=time((time_t *)NULL);
   srandom(strlen(pEnvironment.szInputFile)+strlen(pEnvironment.szTarFile)+tStartTime);
          
   fixPath(szBaseName_in,0); // pretty up the path seperators etc
   strncpy(szBaseName,szBaseName_in,sizeof(szBaseName));
   replace_path_with_webserver_tmp(szBaseName,sizeof(szBaseName)); // do this in designated tmp area
   sprintf(szRandom, "%ld", (long)random() );
   sprintf(szImageFile, "%s.%s.png", szBaseName, szRandom);
   sprintf(szImageFile2, "%s.%sb.png", szBaseName, szRandom);
   sprintf(szPlotFile, "%s.%s.gp", szBaseName, szRandom);
   sprintf(szTmpDataFile, "%s.%s.data", szBaseName, szRandom);

  /*
   * print out images in table
   */
   printf("<TR><TD NOWRAP><IMG SRC=\"%s\"> <IMG SRC=\"%s\"></TD></TR>\n", 
	   makeTmpPNGFileSrcRef(szImageFile).c_str(), 
	   makeTmpPNGFileSrcRef(szImageFile2).c_str());

  /*
   * print out raw distribution data + gnuplot datafile in table
   */
   printf("<TR><TD BGCOLOR=\"#FFFFFF\"><PRE>");

   if ( (fp=fopen(szTmpDataFile,"w"))==NULL)
   {
      printf(" Error - can't write plot file %s\n\n", szTmpDataFile);
      exit(EXIT_FAILURE);
   }
   while (fgets(szBuf, SIZE_BUF, ppIn))
   {
      int iNum=0,
          iCt=0,
          iTot=0;

      sscanf(szBuf, "%d %d %d", &iNum, &iCt, &iTot);
      if (iCt>0)
      {
         fprintf(fp, "%s", szBuf);
//       fprintf(fp, "%d %d %d %f\n", iNum, iCt, iTot, log10((double)iTot));
      }
   }
   fclose(fp);

   printf("</TD></TR>\n");

  /*
   * Create gnuplot file
   */
   if ((fp=fopen(szPlotFile,"w"))==NULL)
   {
      printf(" Error - can't write plot file %s\n\n", szPlotFile);
      exit(EXIT_FAILURE);
   }
   fprintf(fp, "set terminal png\n");
   fprintf(fp, "set output \"%s\"\n", szImageFile);
   fprintf(fp, "set border\n");
   fprintf(fp, "set title \"Output Score Distribution\"\n");
   fprintf(fp, "set xlabel \"Score\"\n");
   fprintf(fp, "set ylabel \"# Match\"\n");
   fprintf(fp, "set grid\n");
   fprintf(fp, "set nokey\n");
   fprintf(fp, "set size 0.5,0.5\n");
   fprintf(fp, "plot \"%s\" using 1:2 with linespoints 1 5\n", szTmpDataFile);
   fclose(fp);

   sprintf(szCommand, "%s %s", GNUPLOT_BINARY, szPlotFile);
   verified_system(szCommand);
   verified_unlink(szPlotFile);
   verified_unlink(szTmpDataFile);

  /*
   * Create gnuplot file
   */
   if ((fp=fopen(szPlotFile,"w"))==NULL)
   {
      printf(" Error - can't write plot file %s\n\n", szPlotFile);
      exit(EXIT_FAILURE);
   }
   fprintf(fp, "set terminal png\n");
   fprintf(fp, "set output \"%s\"\n", szImageFile2);
   fprintf(fp, "set border\n");
   fprintf(fp, "set title \"Cummulative Score Distribution\"\n");
   fprintf(fp, "set xlabel \"Score\"\n");
   fprintf(fp, "set ylabel \"log10(# Total Match)\"\n");
   fprintf(fp, "set grid\n");
   fprintf(fp, "set nokey\n");
   fprintf(fp, "set size 0.5,0.5\n");
   fprintf(fp, "plot \"%s\" using 1:4 with linespoints 1 5\n", szTmpDataFile);
   fclose(fp);

   sprintf(szCommand, "%s %s", GNUPLOT_BINARY, szPlotFile);
   verified_system(szCommand); // system() with verbose error check
   verified_unlink(szPlotFile);
   verified_unlink(szTmpDataFile);

} /*CREATE_IMAGE*/
