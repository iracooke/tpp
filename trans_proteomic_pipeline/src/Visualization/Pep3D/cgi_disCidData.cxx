/*
Program       : Pep3D - Display CID Spectrum
Author        : Xiao-jun Li <xli@systemsbiology.org>
Date          : 10.08.02

CGI program for displaying a CID spectrum from Pep3D images.

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
 * Web-based PEP3D
 * Developed by: Xiao-jun Li (xli@systemsbiology.org)
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>

#include "common/constants.h"
#include "Pep3D_functions.h"
#include "common/util.h"
#include "common/spectStrct.h"
#include "SpectrumParser.h"

#include "common/TPPVersion.h" // contains version number, name, revision

#define _MXSTRLEN_ 10000 // maximium string length
#define _MXQ_ 4 // maximium charge state


/************************************************************************
 *
 * main
 *
 ************************************************************************/
int main(int argc, char **argv)
{
  hooks_tpp handler(argc,argv); // set up install paths etc

  void getCidPlot(char *pngFile, const spectStrct &cidSpect, double mz);

  char *xmlFile, *origFile;
  char *queryString = getenv("QUERY_STRING");
  int scanNum[3];

  double mz, et;
  spectStrct cidSpect;
  int idIndx;
  char pngFile[512]; // png file
  char pngFileBase[512];// = "/tmp/Pep3D_CID";
  char cgiAction[1000];
  char dir[1000];
  char *tmpValue;
  char *tmpValue1;
  time_t currTime;

  int i;


  // print html header
  printf("Content-type: text/html\n\n");
  printf("<HTML>\n<HEAD><TITLE>CID Spectrum Display (%s)</TITLE></HEAD>\n",szTPPVersionInfo);
  printf("<BODY BGCOLOR=\"#FFFFFF\" ONLOAD=\"self.focus();\">\n");
  printf("<h1>CID Spectrum Display</h1><br/>\n(%s)<br/>\n",szTPPVersionInfo);
  printf("Developed by Dr. Xiao-jun Li at <a href=\"http://www.systemsbiology.org\"> ");
  printf("Institute for Systems Biology. </a> <br/>\n\n");
  printf("Date: October 8, 2002<br/><br/>\n");  fflush(stdout);

  /*
   * collect information
   */
  // queryString
  if(queryString == NULL) {
    printf("No information passed from cgi.<br/>\n");
    fflush(stdout);
    return 1;
  }

  // xmlFile
  if((xmlFile = getHtmlFieldValue("xmlFile", queryString)) == NULL){
    printf("No .mzXML/.mzData file specified!<br/>\n");
    fflush(stdout);
    return 1;
  }
  fixPath(xmlFile,1); // pretty up the path seperators etc - expect existence

  // scanNum
  if((tmpValue1 = getHtmlFieldValue("scanNum", queryString)) == NULL){
    printf("No scan number specified!<br/>\n");
    fflush(stdout);
    free(xmlFile);
    return 1;
  }
  else {
    sscanf(tmpValue1, "%d", scanNum);
    free(tmpValue1);
  }

  // scanNum
  if((tmpValue = getHtmlFieldValue("scanNum2", queryString)) == NULL){
    printf("No scan number specified!<br/>\n");
    fflush(stdout);
    free(xmlFile);
    return 1;
  }
  else {
    sscanf(tmpValue, "%d", scanNum+1);
    free(tmpValue);
  }

  // charge
  if((tmpValue = getHtmlFieldValue("chrg", queryString)) != NULL){
    sscanf(tmpValue, "%d", scanNum+2);
    free(tmpValue);
  }
  else 
    scanNum[2] = 0;
  // origFile
  if((origFile = getHtmlFieldValue("origFile", queryString)) == NULL) {
    idIndx = 0;
    strcpy(dir, xmlFile);
  }
  else {
    idIndx = 1;
    fixPath(origFile,1); // pretty up the path seperators etc - expect existence
    strcpy(dir, origFile);
  }
  fixPath(dir,0); // pretty up the path seperators etc

  if((tmpValue = findRightmostPathSeperator(dir)) != NULL){
    dir[strlen(dir)-strlen(tmpValue)] = '\0';
  } 
  else if(safepath_getcwd(dir, 1000) == NULL) {
    dir[0] = '\0';
  }

  /*
   * collect cidSpect
   */
  getCidSpect(&mz, &et, &cidSpect, xmlFile, scanNum);
  if(cidSpect.size < 0){
    free(xmlFile);
    if(origFile != NULL)
      free(origFile);
    return 1;
  }

#ifdef USING_RELATIVE_WEBSERVER_PATH  // for example, in win32 understand /foo/bar as /Inetpub/wwwroot/foo/bar
  char szWebserverRoot[512];
  char *pStr;
  char *curr_dir;
  
  curr_dir =  (char *) calloc(strlen(dir)+1, sizeof(char));
  strcpy(curr_dir, dir); 
#ifdef WINDOWS_CYGWIN
  char szBuf[512];
  char szCommand[512];
  FILE *fp;
  sprintf(szCommand, "cygpath -u '%s'", dir);
  if ((fp=popen(szCommand,"r"))!=NULL)
  {
    char szBuf[512];
    fgets(szBuf, 512,fp);
    pclose(fp);
    szBuf[strlen(szBuf)-1]=0;
    strcpy(dir, szBuf);
  }
#endif
  pStr=getenv("WEBSERVER_ROOT");
  if (pStr==NULL)
  {
    printf("<PRE> Environment variable WEBSERVER_ROOT does not exist.\n\n");
    printf(" For Windows users, you can set this environment variable\n");
    printf(" through the Advanced tab under System Properties when you\n");
    printf(" right-mouse-click on your My Computer icon.\n\n");
    
    printf(" Set this environment variable to your webserver's document\n");
    printf(" root directory such as c:\\inetpub\\wwwroot for IIS or\n");
    printf(" c:\\website\\htdocs or WebSite Pro.\n\n");
    printf(" Exiting.\n");
    exit(0);
  }
  else
  {
#ifdef WINDOWS_CYGWIN
    // must first pass to cygpath program
    sprintf(szCommand, "cygpath -u '%s'", pStr);
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
      strcpy(szWebserverRoot, szBuf);
    }
#else
      strcpy(szWebserverRoot, pStr);
#endif
  }
  free(curr_dir);
#endif

  strcpy(pngFileBase, dir);

  if (pngFileBase[strlen(pngFileBase)-1] == '/') 
  {
    strcat(pngFileBase, "Pep3D_");
  }
  else {
    strcat(pngFileBase, "/Pep3D_");
  }


  /*
   * collect id information
   */

  /* 
   * for Andy
   */
  SpectrumParser* parser = NULL;

  if(idIndx == 1) {
    // get basename from xmlFile
    char* basename = new char[strlen(xmlFile)+1];
    strcpy(basename, xmlFile);
    char* result = rampTrimBaseName(basename);
    if(result == NULL) {
      cout << "error: cannot get basename from " << xmlFile << endl;
      exit(1);
    }

    parser = new SpectrumParser(origFile, basename, scanNum[0], scanNum[1], scanNum[2]);
    //cout << "done here" << endl;
    //idStr = new char[1];
    //idStr = getPepIdStr(origFile, xmlFile, scanNum[0], scanNum[1], scanNum[2]);
    delete[] basename;
  }

  /*
   * get CID plot
   */

  // get pngFile
  strcpy(pngFile, pngFileBase);
  (void) time(&currTime);
  srand48((long) currTime);
  sprintf(pngFile, "%s_%d.png", pngFileBase, (int)lrand48());
  getCidPlot(pngFile, cidSpect, mz);
  

  /*
   * display
   */
  printf("<ul>\n");

  if (scanNum[0] < scanNum[1])
    printf("<li>scan number: %d -- %d <br/> m/z: %f <br/> time: %f</li>\n\n", 
	   scanNum[0], scanNum[1], mz, et);
  else
    printf("<li>scan number: %d <br/> m/z: %f <br/> time: %f</li>\n\n", scanNum[0], mz, et);

  //printf("idIndx: %d\n", idIndx);
  //if(idStr == NULL)
  //  printf("null idStr\n");
  
  if(idIndx == 1) { // && idStr != NULL) {
    printf("<li>ID: <br/>");
    if(parser != NULL && parser->found())
      parser->write(cout);
    else
      printf("<li>no info available</li>\n\n");

    printf("</li>\n\n");
  }
  else
    printf("<li>ID: none</li>\n\n");
  printf("</ul>\n");

  char *pngFileRef;
#ifdef USING_RELATIVE_WEBSERVER_PATH  // for example, in win32 understand /foo/bar as /Inetpub/wwwroot/foo/bar  // fix up the image path
  pngFileRef = strdup(translate_relative_webserver_root_path_to_absolute_filesystem_path(pngFile).c_str());
#else
  pngFileRef = strdup(pngFile);
#endif
  printf("<img src=\"%s\"/>\n", makeTmpPNGFileSrcRef(pngFileRef).c_str());
  free(pngFileRef);

  // save to dta
  if((tmpValue = getenv("SCRIPT_NAME")) != NULL) {
    strcpy(cgiAction, tmpValue);
    if((tmpValue = findRightmostPathSeperator(cgiAction)) != NULL) {
      strcpy(tmpValue+1, "getCidData_xml.cgi");

      printf("<form method=\"POST\" action=\"%s\">\n\n", cgiAction);
      
      // xmlFile
      printf("<input type=\"hidden\" name=\"xmlFile\" value=\"%s\" />\n",
	     xmlFile);
      
      // directory
      printf("<input type=\"hidden\" name=\"dir\" value=\"%s\" />\n", dir);
      
      // scanNum
      printf("<input type=\"hidden\" name=\"scanNum\" value=\"%d\" />\n",
	     scanNum[0]);
      
      // scanNum
      printf("<input type=\"hidden\" name=\"scanNum2\" value=\"%d\" />\n",
	     scanNum[1]);

      // submit
      printf("<input type=\"submit\" name=\"submit\" value=\"Export to dta\" />\t");

      // charge 
      printf("charge: ");
      for (i = 0; i < _MXQ_; ++i)
	printf("<input type=\"checkbox\" name=\"charge%d\"/>+%d\t", i+1, i+1);
      printf("\n");
      
      printf("</form><br/>\n");
    } // if((tmpValue = strrchr(cgiAction, '/')) != NULL) {
  } //   if((tmpValue = getenv("SCRIPT_NAME")) != NULL) {

  printf("<br/>Spectrum is plotted by <a href=\"http://www.gnuplot.info/\">Gnuplot</a>.<br/>\n");
  printf("</BODY></HTML>\n");
  fflush(stdout);

  // clean up
  free(xmlFile);
  if(origFile != NULL)
    free(origFile);
  //if(idStr != NULL)
  //  free(idStr);
  if(parser != NULL)
    delete parser;

  return 0;
}


/************************************************************************
 *
 * void getCidPlot(char *pngFile, spectStrct cidSpect, double mz)
 *
 *   This function generates CID plot.
 *
 ************************************************************************/
void getCidPlot(char *pngFile, const spectStrct &cidSpect, double mz)
{
  void filterByNoise(spectStrct * peakList, double passValue, int msType);
  void getStrPeaks(spectStrct *peakSpect, double pkRz);
 
  // variables
  FILE *file;
  char txtFile[1000], cidFile[1000];
  double mxValue;
  spectStrct strSpect;
  int i;

  // print precursor m/z
  sprintf(cidFile, "%s.cid", pngFile);

  mxValue = 0.;
  for (i = 0; i < cidSpect.size; ++i){
    if(cidSpect.yval[i] > mxValue)
      mxValue = cidSpect.yval[i];
  }

  if((file = fopen(cidFile, "w")) == NULL) {
    printf("Cannot Write to File \"%s\"!<br/>\n", cidFile);
    fflush(stdout);
    return;
  }
  fprintf(file, "%f 0.\n", mz);
  fprintf(file, "%f %f\n", mz, 0.05*mxValue);  
  fclose(file);

  // print data into a file
  sprintf(txtFile, "%s.txt", pngFile);
  
  if((file = fopen(txtFile, "w")) == NULL) {
    printf("Cannot Write to File \"%s\"!<br/>\n", txtFile);
    fflush(stdout);
    return;
  }

  for (i = 0; i < cidSpect.size; ++i)
    fprintf(file, "%f %f\n", cidSpect.xval[i], cidSpect.yval[i]);
  fclose(file);

  // get strong ions
  strSpect = cidSpect;
  filterByNoise(&strSpect, 2.5, 2);
  getStrPeaks(&strSpect, 1.e-2);
  if(strSpect.size < 10) {
    strSpect = cidSpect;
    filterByNoise(&strSpect, 0., 2);
    getStrPeaks(&strSpect, 1.e-2);
  }
  if(strSpect.size < 10) {
    strSpect = cidSpect;
    getStrPeaks(&strSpect, 1.e-2);
  }

  for (i = 0; i < strSpect.size; ++i) {
    strSpect.yval[i] = strSpect.yval[i] < mxValue ? 
      strSpect.yval[i] : mxValue; 
  }

#ifdef PIPEIT
  // printf data using gnuplot
  if((file = tpplib_popen(GNUPLOT_BINARY, "w")) == NULL) {
    printf("Cannot start \"%s\"!<br/>\n",GNUPLOT_BINARY);
    fflush(stdout);
    return;
  }
#else
  char *gpFile = (char *)malloc(strlen(pngFile)+4);
  strcpy(gpFile,pngFile);
  strcat(gpFile,".gp");
  file = fopen(gpFile,"w");
#endif
  fprintf(file, "set terminal png\n");
  fprintf(file, "set output \'%s\'\n", pngFile);
  fprintf(file, "set nokey\n");
  fprintf(file, "set yrange [0:%f]\n", 1.2*mxValue);
  fprintf(file, "set format y \"%%8.2g\"\n");
  fprintf(file, "set xlabel \"m/z\"\n");
  for (i = 0; i < strSpect.size; ++i)
    fprintf(file, "set label \"%.2f\" at %.2f, %.2f left rotate font \"Helvetica, 2\"\n",
	    strSpect.xval[i], strSpect.xval[i], strSpect.yval[i]);
  fprintf(file, "pl '%s'w l lt 3 lw 3, '%s'w i lt 1 lw 3, '%s'\n", 
	  cidFile, txtFile, cidFile);
  fprintf(file, "quit\n");  
#ifdef PIPEIT
  pclose(file);
#else
  fclose(file);
  char *cmd=(char *)malloc(strlen(GNUPLOT_BINARY)+strlen(gpFile)+3);
  sprintf(cmd,"%s %s",GNUPLOT_BINARY,gpFile);
  verified_system(cmd); // system() with verbose error check
  verified_unlink(gpFile);
  free(cmd);
  free(gpFile);
#endif
  // clean up
  verified_unlink(cidFile);
  verified_unlink(txtFile);

  return;
}


/************************************************************************
 *
 *  This function collects strongest peak within a continuous spectrum.
 *
 ************************************************************************/
void getStrPeaks(spectStrct *peakSpect, double pkRz) 
{
  double range; 
  int *peakIndx;
  int pkNum;
  int *eqPks;
  int eqPkNum;
  int left, right;
  int i, j;
  
  // assume all strong peaks
  peakIndx = (int *) calloc(peakSpect->size, sizeof(int));
  for (i = 0; i < peakSpect->size; ++i) 
    peakIndx[i] = 1; 

  // get strong peaks
  eqPks = (int *) calloc(peakSpect->size, sizeof(int));
  pkNum = 0;
  for (i = 0; i < peakSpect->size; ++i) {
    if(peakIndx[i] == 0)
      continue;
    
    range = peakSpect->xval[i]*pkRz;

    left = i;
    while(left > -1 
	  && peakSpect->xval[i]-peakSpect->xval[left] 
	  <= range)
      --left;
    ++left;

    right = i;
    while(right < peakSpect->size
	  && peakSpect->xval[right]-peakSpect->xval[i] 
	  <= range)
      ++right;
    --right;

    eqPkNum = 0;
    for (j = left; j <= right; ++j) { 
      if(peakSpect->yval[j] > peakSpect->yval[i])
	peakIndx[i] = 0;
      else if(peakSpect->yval[j] < peakSpect->yval[i])
	peakIndx[j] = 0;
      else
	eqPks[eqPkNum++] = j;
    }
    
    if(eqPkNum > 1 && peakIndx[i] == 1) {
      for(j = 0; j < eqPkNum; ++j) {
	peakIndx[eqPks[j]] = 0;
      }
      peakIndx[eqPks[(eqPkNum-1)/2]] = 1;
    }
  } // for (i = 0; i < peakSpect->size; ++i) {
  free(eqPks);

  // collect strong peaks
  pkNum = 0;
  for(i = 0; i < peakSpect->size; ++i) {
    if(peakIndx[i] == 1) { 
      peakSpect->xval[pkNum] = peakSpect->xval[i];
      peakSpect->yval[pkNum] = peakSpect->yval[i];
      ++pkNum;
    }
  }
  peakSpect->size = pkNum;
  free(peakIndx);

  return;
}


/************************************************************************
 *
 * This function filters "peakList" by noise.
 *
 * msType = 1 MS; other, CID. 
 *
 ************************************************************************/
void filterByNoise(spectStrct * peakList, double passValue, int msType)
{
  void getBackground(const spectStrct &spectrum, int dtIndx, int msType, 
		     double *ave, double *var);
  void massSort(double *xval, double *yval, int size);
 
  spectStrct strPeakList;
  int peakNum, fnlPeakNum, tempPeakNum;
  int lower, upper;
  double level;
  double *tempAve, *tempVar;
  int i;

  passValue = passValue < 2.5 ? passValue : 2.5;

  // get strPeaklist
  strPeakList.size = peakList->size;
  strPeakList.xval = (double *) calloc(strPeakList.size, sizeof(double));
  strPeakList.yval = (double *) calloc(strPeakList.size, sizeof(double));

  // get all peaks above "level" noise level
  tempAve = (double *) calloc(peakList->size, sizeof(double));
  tempVar = (double *) calloc(peakList->size, sizeof(double));
  level = 2.5;
  fnlPeakNum = 0;
  do {
    tempPeakNum = fnlPeakNum;
    lower = 0;
    upper = 0;
    for (i = 0; i < peakList->size; ++i) {
      getBackground(*peakList, i, msType, tempAve+i, tempVar+i);
    }

    peakNum = 0;
    for (i = 0; i < peakList->size; ++i) {
      if(peakList->yval[i] >=  tempAve[i]+level*tempVar[i]) {
	strPeakList.xval[fnlPeakNum] = peakList->xval[i];
	strPeakList.yval[fnlPeakNum] = peakList->yval[i];
	++fnlPeakNum;
      }
      else {
	peakList->xval[peakNum] = peakList->xval[i];
	peakList->yval[peakNum] = peakList->yval[i];
	++peakNum;
      }
    }
    peakList->size = peakNum;
  }
  while(tempPeakNum != fnlPeakNum);

  // collect peaks above noise level
  level = passValue;
  for (i = 0; i < peakList->size; ++i) {
    if(peakList->yval[i] >=  tempAve[i]+level*tempVar[i]) {
      strPeakList.xval[fnlPeakNum] = peakList->xval[i];
      strPeakList.yval[fnlPeakNum] = peakList->yval[i];
      ++fnlPeakNum;
    }
  }
  strPeakList.size = fnlPeakNum;


  // collect peaks in "strPeakList"
  peakNum = 0;
  for (i = 0; i < strPeakList.size; ++i) {
    peakList->xval[peakNum] = strPeakList.xval[i];
    peakList->yval[peakNum] = strPeakList.yval[i];
    ++peakNum;
  }
  peakList->size = peakNum;

  // sort peakList
  massSort(peakList->xval, peakList->yval, peakList->size);

  free(tempAve);
  free(tempVar);

  return;
}


/************************************************************************
 *
 * This function gets average and variance of peak heights within a M/Z range.
 *
 ************************************************************************/
void getBackground(const spectStrct &spectrum, int dtIndx, int mzType, 
		   double *ave, double *var)
{

  int dataRange = 50;
  int lower, upper;

  int i;

  if(mzType == 1) {
    lower = dtIndx-dataRange > 0 ? dtIndx-dataRange : 0;
    upper = dtIndx+dataRange < spectrum.size ? dtIndx+dataRange : spectrum.size-1;
  }
  else {
    lower = 0;
    upper = spectrum.size - 1;
  }
  
  *ave = 0.;
  *var = 0.;
  for (i = lower; i < upper; ++i) {
    *ave += spectrum.yval[i];
    *var += spectrum.yval[i]*spectrum.yval[i];
  }
  *ave /= upper - lower;
  *var = sqrt((*var-(upper-lower)*(*ave)*(*ave))/(upper-lower-1.));
  
  return;
}  


/************************************************************************
 *
 * This function sorts "peakList" by mass.
 *
 ************************************************************************/
void massSort(double *xval, double *yval, int size) 
{
  double xData, yData;
  int index1, index2;

  index1 = 1;
  index2 = size-1;

  if(size <= 1) return;

  if(size == 2) {
    if(xval[0] > xval[1]) {
      xData = xval[1];
      yData = yval[1];
      xval[1] = xval[0];
      yval[1] = yval[0];
      xval[0] = xData;
      yval[0] = yData;
    }
    return;
  }

  while(index1 < index2) {
    while(index1 < size && xval[index1] <= xval[0]) ++index1;
    while(index2 > 0 && xval[index2] >= xval[0]) --index2;

    if(index1 < index2) {
      xData = xval[index1];
      yData = yval[index1];
      xval[index1] = xval[index2];
      yval[index1] = yval[index2];
      xval[index2] = xData;
      yval[index2] = yData;
    }
  }

  if(index1 > 1) {
    xData = xval[index1-1];
    yData = yval[index1-1];
    xval[index1-1] = xval[0];
    yval[index1-1] = yval[0];
    xval[0] = xData;
    yval[0] = yData;
  }

  massSort(xval, yval, index1-1); 
  massSort(xval+index1, yval+index1, size-index1); 

  return;
}

