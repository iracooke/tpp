/*

Program       : Pep3D
Author        : Xiao-jun Li <xli@systemsbiology.org>                                                      
Date          : 10.08.02 

CGI program for exporting CID spectra from a Pep3D image.

Copyright (C) 2002 Xiao-jun Li

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Xiao-jun Li
Institute for Systems Biology
401 Terry Avenue North 
Seattle, WA  98109  USA
xli@systemsbiology.org

*/
/*
  Web-based PEP3D
  Developed by: Xiao-jun Li (xli@systemsbiology.org)
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include "common/TPPVersion.h"

#include "Pep3D_functions.h"
//#include "Pep3D_functions.h"
#include "mzParser.h"
#include "common/util.h"
#include "common/spectStrct.h"

#define _MXSTRLEN_ 10000 // maximium string length
#define _MXQ_ 4 // maximium charge state
#define _HM_ 1.0078250321

int main(int argc, char **argv)
{
  hooks_tpp handler(argc,argv); // installdir issues etc
  char *xmlFile, *dir;
  char *queryString = getQueryString();
  int scanNum[2];
  double mz, et;
  spectStrct cidSpect;
  char dtaFileBase[1000];
  char *tmpValue;
  char tmpString[1000];
  FILE *file;

  int i, j;


  // print html header
  printf("Content-type: text/html\n\n");
  printf("<HTML>\n<HEAD><TITLE>CID Spectrum Export (%s)</TITLE></HEAD>\n",szTPPVersionInfo);
  printf("<BODY BGCOLOR=\"#FFFFFF\" ONLOAD=\"self.focus();\">\n");
  printf("<h1>CID Spectrum Export</h1><br/>\n(%s)<br/>\n",szTPPVersionInfo);
  printf("Developed by Dr. Xiao-jun Li at <a href=\"http://www.systemsbiology.org\"> ");
  printf("Institute for Systems Biology. </a> <br/>\n\n");
  printf("Data: October 8, 2002<br/><br/>\n");
  fflush(stdout);


  /*
    collect information
  */
  // queryString
  if(queryString == NULL) {
    printf("No information passed from cgi.<br/>\n");
    fflush(stdout);
    return 1;
  }

  // xmlFile
  if((xmlFile = getHtmlFieldValue("xmlFile", queryString)) == NULL){
    printf("No .mzXML file specified!<br/>\n");
    fflush(stdout);
    free(queryString);
    return 1;
  }

  // directory
  if((dir = getHtmlFieldValue("dir", queryString)) == NULL){
    printf("No directory can be specified!<br/>\n");
    fflush(stdout);
    free(queryString);
    free(xmlFile);
    return 1;
  }

  // scanNum
  if((tmpValue = getHtmlFieldValue("scanNum", queryString)) == NULL){
    printf("No scan number specified!<br/>\n");
    fflush(stdout);
    free(queryString);
    free(xmlFile);
    free(dir);
    return 1;
  }
  else {
    sscanf(tmpValue, "%d", scanNum);
    free(tmpValue);
  }

  // scanNum
  if((tmpValue = getHtmlFieldValue("scanNum2", queryString)) == NULL){
    printf("No scan number specified!<br/>\n");
    fflush(stdout);
    free(queryString);
    free(xmlFile);
    free(dir);
    return 1;
  }
  else {
    sscanf(tmpValue, "%d", scanNum+1);
    free(tmpValue);
  }


  /*
    collect cidSpect
  */
  getCidSpect(&mz, &et, &cidSpect, xmlFile, scanNum);
  if(cidSpect.size < 0){
    printf("Empty spectrum!<br/>\n");
    fflush(stdout);
    free(queryString);
    free(xmlFile);
    free(dir);
    return 1;
  }

  /*
    export dta files
  */
  // mkdir Pep3D
  if(chdir(dir) != 0){
    printf("Cannot access directory \"%s\"!<br/>\n", dir);
    fflush(stdout);
    free(queryString);
    free(xmlFile);
    free(dir);
    return 1;
  }

  sprintf(tmpString, "%s/Pep3D_cid", dir);
  if(access(tmpString, W_OK) != 0) {
    verified_system("mkdir Pep3D_cid");
  }
  if(chdir(tmpString) != 0){
    printf("Cannot access directory \"%s\"!<br/>\n", tmpString);
    fflush(stdout);
    free(queryString);
    free(xmlFile);
    free(dir);
    return 1;
  }
  else {
    dir = (char *) realloc(dir, (strlen(tmpString)+1)*sizeof(char));
    strcpy(dir, tmpString);
#if !defined(WINDOWS_CYGWIN) && !defined(WINDOWS_NATIVE)
    //TODO: WHAT'S UP HERE? WHY?
    verified_system("chmod g+w *.dta");
#endif  
  }

  // get file base
  if((tmpValue = findRightmostPathSeperator(xmlFile)) != NULL)
    sprintf(dtaFileBase, "%s", tmpValue+1);
  else
    sprintf(dtaFileBase, "%s", xmlFile);

  rampTrimBaseName(dtaFileBase); // remove .mzXML or .mData extension
  
#ifdef WINDOWS_CYGWIN
  char *szCommand = new char[strlen(dir) + 32];
  char szBuf[SIZE_BUF+1];
  FILE* fp;
  // must first pass to cygpath program
  sprintf(szCommand, "cygpath -w '%s'", dir);
  if((fp = popen(szCommand, "r")) == NULL)
    {
      printf("cygpath error, exiting\n");
      exit(1);
    }
  else
    {
      fgets(szBuf, SIZE_BUF, fp);
      pclose(fp);
      szBuf[strlen(szBuf)-1] = 0;
      strcpy(dir, szBuf);
    }
  delete [] szCommand;
#endif

  // generate dta file
  for (i = 0; i < _MXQ_; ++i){
    sprintf(tmpString, "charge%d", i+1);
    if((tmpValue = getHtmlFieldValue(tmpString, queryString)) != NULL){
      free(tmpValue);

      sprintf(tmpString, "%s.%04d.%04d.%d.dta", dtaFileBase, scanNum[0], scanNum[1], i+1);

      if((file = fopen(tmpString, "w")) == NULL) {
	printf("Cannot write to file \"%s\" in directory \"%s\".  Here is the data.<br/><br/>\n", 
	       tmpString, dir);
	fflush(stdout);
	printf("%f\t%d<br/>\n", (i+1)*(mz-_HM_)+_HM_, i+1);
	for (j = 0; j < cidSpect.size; ++j)
	  printf("%f\t%f<br/>\n", cidSpect.xval[j], cidSpect.yval[j]);
	printf("<br/>\n");
	fflush(stdout);
      }
      else {
	fprintf(file, "%f\t%d\n", (i+1)*(mz-_HM_)+_HM_, i+1);
	for (j = 0; j < cidSpect.size; ++j)
	  fprintf(file, "%f\t%f\n", cidSpect.xval[j], cidSpect.yval[j]);
	fclose(file);
	verified_system("chmod g+w *.dta");
	printf("File \"%s\" generated in \"%s\". <br/>\n", tmpString, dir);
	fflush(stdout);
      }
    }
  } // for (i = 0; i < _MXQ_; ++i){


  printf("</BODY></HTML>\n");
  fflush(stdout);

  // clean up
  free(queryString);
  free(xmlFile);
  free(dir);

  return 0;
}


