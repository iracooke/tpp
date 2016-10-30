/*
Program       : Web-based Pep3D
Author        : Xiao-jun Li <xli@systemsbiology.org>
Date          : 10.08.02 

CGI program displaying LC-ESI-MS data stored in mzXML format in a Pep3D image.

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h> 

#include <gd.h>
#include <gdfonts.h>
#include <gdfontmb.h>

#include "Visualization/Pep3D/Pep3D_functions.h"

#include "mzParser.h"

#include "Visualization/Pep3D/Pep3DParser.h"
#include "Visualization/Pep3D/BasenameParser.h"
#include "common/TPPVersion.h" // contains version number, name, revision
#include "common/util.h"
#include "common/spectStrct.h"

/************************************************************************
 *
 * constants
 *
 ************************************************************************/
# define _MXSTRLEN_ 10000 // maximum string length
# define XUL 70 // x-coordinate for up-left corner of frame
# define YUL 20 // y-coordinate for up-left corner of frame
# define OFFSET 20 // distance between (XUL, YUL) and the corner for real data
# define HTML_OUTPUT 1
# define XML_OUTPUT 2

/************************************************************************
 *
 * globals and structures
 *
 ************************************************************************/
int inMsgPane = 0;
int iOutputType = HTML_OUTPUT;  //default

// CID data structure 
typedef struct { 
  int s[3];
  double x;
  double y;
  double z;
} cidDataStrct;

// range structure 
typedef struct { 
  double bnd[2]; 
  double grd; 
} rangeStrct;

// mzXML filepath structure
typedef struct {
  char * expLbl;  // experimental Label
  char * path;  // file path
} get_mzXML_pathes_FilePathStrct;


int fillColor(double z, rangeStrct z1Range, rangeStrct z2Range, 
	      int totColor, int sectIndx);
int getOutputType(const char *queryString);
void reportError(const char *szMessage, int iExitCode);
void prntMsg(const char *message);
void prntFrontPage(FILE *file, const char *queryString);
void prntPageFooter(FILE *file);
void Pep3D(const char *queryString);
static int spectPeak(const spectStrct &spectrum, int position, int direction);
static void smoothSpect(spectStrct * spectrum, int range, int repeats);


/************************************************************************
 *
 * main
 *
 ************************************************************************/
int main(int argc, char **argv)
{
  hooks_tpp handler(argc,argv); // set up install paths etc

  char *queryString;
  char *tmpValue;

  queryString = getQueryString();
  iOutputType = getOutputType(queryString);

  if(iOutputType == HTML_OUTPUT) {
    printf("Content-type: text/html\n\n");
    fflush(stdout);

    // front page
    prntFrontPage(stdout, queryString);

  } else if(iOutputType == XML_OUTPUT) {
    printf("Content-type: text/xml\n\n");
    printf("<XML>\n");
    printf("<TPP Version=\"%s\" />\n", szTPPVersionInfo);
  }

  // generate plot
  if(queryString != NULL
     && (tmpValue = getHtmlFieldValue("submit", queryString)) != NULL){

    if(iOutputType == HTML_OUTPUT) {
      printf("<br/>\n");
      prntMsg("Generating images...");
    }

    fflush(stdout);
    Pep3D(queryString);

    if(iOutputType == XML_OUTPUT)
      printf("<STATUS>OK</STATUS>\n");

    free(tmpValue);
    free(queryString);
  } else if(iOutputType == XML_OUTPUT) {
    // no action
    printf("<MESSAGE>Submit action not passed in parameters!</MESSAGE>\n");
    printf("<STATUS>ERROR</STATUS>\n");
  }

  // end response
  if(iOutputType == HTML_OUTPUT)
    prntPageFooter(stdout);
  else if(iOutputType == XML_OUTPUT)
    printf("</XML>\n");


  fflush(stdout);
  return 0;
}


/************************************************************************
 *
 * This function prints the front page interface for the program
 *
 ************************************************************************/
void prntFrontPage(FILE *file, const char *queryString)
{
  char *tmpValue;

  // open page
  fprintf(file,"<html>\n"); 
  fprintf(file,"<head>\n");
  fprintf(file,"<title>Pep3D Image for LC-ESI-MS Data (%s)</title>\n",szTPPVersionInfo);

  // style-sheet
  fprintf(file,"<style type=\"text/css\">\n");
  fprintf(file,".hideit {display:none}\n");
  fprintf(file,".showit {display:table-row}\n");
  fprintf(file,".accepted {background: #dddddd;}\n");
  fprintf(file,".rejected {background: #b5b5b5;}\n");
  fprintf(file,"body{font: small sans-serif; }h1  {font-family: Helvetica, Arial, Verdana, sans-serif; font-size: 24pt; font-weight:bold; color:#0E207F}\n");
  fprintf(file,"h2  {font-family: Helvetica, Arial, sans-serif; font-size: 20pt; font-weight: bold; color:#0E207F}\n");
  fprintf(file,"h3  {font-family: Helvetica, Arial, sans-serif; font-size: 16pt; color:#FF8700}\n");
  fprintf(file,"h4  {font-family: Helvetica, Arial, sans-serif; font-size: 14pt; color:#0E207F}\n");
  fprintf(file,"h5  {font-family: Helvetica, Arial, sans-serif; font-size: 10pt; color:#AA2222}\n");
  fprintf(file,"h6  {font-family: Helvetica, Arial, sans-serif; font-size:  8pt; color:#333333}\n");
  fprintf(file,"table   {border-collapse: collapse; border-color: #000000;}\n");
  fprintf(file,"td      {border-collapse: collapse; border-color: #000000;}\n");
  fprintf(file,".banner_cid   {\n");
  fprintf(file,"                 background: #0e207f;\n");
  fprintf(file,"                 border: 2px solid #0e207f;\n");
  fprintf(file,"                 color: #eeeeee;\n");
  fprintf(file,"                 font-weight:bold;\n");
  fprintf(file,"              }\n");
  fprintf(file,".banner2      {\n");
  fprintf(file,"                 background: #aaaaaa;\n");
  fprintf(file,"                 color: black;\n");
  fprintf(file,"              }\n");
  fprintf(file,".banner1      {\n");
  fprintf(file,"                 background: #FF8700;\n");
  fprintf(file,"                 color: black;\n");
  fprintf(file,"              }\n");
  fprintf(file,".formentry    {\n");
  fprintf(file,"                 background: #eeeeee;\n");
  fprintf(file,"                 border: 2px solid #0e207f;\n");
  fprintf(file,"                 color: black;\n");
  fprintf(file,"                 padding: 1em;\n");
  fprintf(file,"              }\n");
  fprintf(file,".image        {\n");
  fprintf(file,"                 background: #ffffff;\n");
  fprintf(file,"                 border: 2px solid #0e207f;\n");
  fprintf(file,"                 color: black;\n");
  fprintf(file,"                 padding: 1em;\n");
  fprintf(file,"              }\n");
  fprintf(file,".messages     {\n");
  fprintf(file,"                 background: #ffffff;\n");
  fprintf(file,"                 border: 2px solid #FF8700;\n");
  fprintf(file,"                 color: black;\n");
  fprintf(file,"                 padding: 1em;\n");
  fprintf(file,"              }\n");
  fprintf(file,".nav          {\n");
  fprintf(file,"                 background: #c0c0c0;\n");
  fprintf(file,"                 font-family: Helvetica, Arial, Verdana, sans-serif;\n");
  fprintf(file,"                 font-weight:bold;\n");
  fprintf(file,"              }\n");
  fprintf(file,".navselected  {\n");
  fprintf(file,"                 background: #dddddd;\n");
  fprintf(file,"              }\n");
  fprintf(file,"</style>\n");

  // javascript
  fprintf(file,"<script language=\"JavaScript\">\n");
  fprintf(file,"    function showdiv(div_id,nothidden){\n");
  fprintf(file,"	if (document.getElementById(div_id).className == nothidden) {\n");
  fprintf(file,"	  new_state = 'hideit';\n");
  fprintf(file,"	} else {\n");
  fprintf(file,"	  new_state = nothidden;\n");
  fprintf(file,"      }\n");
  fprintf(file,"	document.getElementById(div_id).className = new_state;\n");
  fprintf(file,"    }\n");

  fprintf(file,"    function hilight(tr_id,yesno) {\n");
  fprintf(file,"        if (yesno == 'yes') {\n");
  fprintf(file,"	        document.getElementById(tr_id).className = 'navselected';\n");
  fprintf(file,"	} else {\n");
  fprintf(file,"	        document.getElementById(tr_id).className = '';\n");
  fprintf(file,"	}\n");
  fprintf(file,"    }\n");

  fprintf(file,"    function change_selected(select_id) {\n");
  fprintf(file,"	document.getElementById(select_id).selectedIndex = 1;\n");
  fprintf(file,"    }\n");
  fprintf(file,"</script>\n");

  fprintf(file,"</head>\n\n");

  // open body; print page title
  fprintf(file,"<body bgcolor=\"#c0c0c0\">\n");
  fprintf(file, "<h1>Pep3D Image for LC-ESI-MS Data</h1>\n");

  fprintf(file, "<table cellspacing=\"0\" width=\"100%%\">\n");
  fprintf(file, "<tbody>\n");
  fprintf(file, "<tr>\n");
  fprintf(file, "<td class=\"banner_cid\">&nbsp;&nbsp;&nbsp;Specify parameters</td>\n");
  fprintf(file, "<td align=\"left\">&nbsp;&nbsp;&nbsp;&nbsp;<u><a onclick=\"showdiv('params','formentry')\">[ Show | Hide ]</a></u></td>\n");
  fprintf(file, "<td class=\"nav\" align=\"right\">");

  // form
  if((tmpValue = getenv("SCRIPT_NAME")) != NULL) {
    fprintf(file, "<form method=\"POST\" action=\"%s\">", tmpValue);
    fflush(file);
  }
  else {
    printf("</tr>\n");
    printf("</tbody></table>\n");
    prntMsg("Cannot find SCRIPT_NAME.");
    prntPageFooter(stdout);
    exit(1);
  }
  fprintf(file, "<input type=\"submit\" name=\"submit\" value=\"Reset Pep3D form\" />");
  fprintf(file, "</form></td>\n");
  fprintf(file, "</tr>\n");
  fprintf(file, "</tbody></table>\n");

  // form
  fprintf(file, "<div id=\"params\" class=\"formentry\">\n");
  
  fprintf(file, "<form method=\"POST\" action=\"%s\">\n\n", tmpValue);
  fflush(file);

  /*
   * fields
   */
  fprintf(file, "<table border=\"0\" cellpadding=\"2\">\n");

  // xmlFile
  fprintf(file, "<tr id=\"path\" onmouseover=\"hilight('path','yes')\" onmouseout=\"hilight('path','no')\">\n");  
  fprintf(file, "<td><li title=\"Full path of search result xmlfile or .mzXML/.mzData file. Example: /data2/search/xiaojun/ASAPRatio_demo/interact.xml  Or: /data2/search/dan/17_mix_folder/*.mzXML\">File Path:</li></td>\n");
  fprintf(file, "<td colspan=\"6\">");

  if(queryString != NULL
     && (tmpValue = getHtmlFieldValue("htmFile", queryString)) != NULL) { 
    fprintf(file, "<input type=\"text\" name=\"htmFile\" size=\"120\" value=\"%s\"/>", tmpValue);
    free(tmpValue);
  }
  else
    fprintf(file, "<input type=\"text\" name=\"htmFile\" size=\"120\"/>");

  fprintf(file, "</td>\n</tr>\n");


  // m/z range
  fprintf(file, "<tr id=\"mzrange\" onmouseover=\"hilight('mzrange','yes')\" onmouseout=\"hilight('mzrange','no')\">\n");
  fprintf(file, "<td><li>M/Z range:</li></td>\n");

  fprintf(file, "<td><select size=\"1\" name=\"mzRange\" id=\"mzRange\">");
  if (queryString != NULL
      && (tmpValue = getHtmlFieldValue("mzRange", queryString)) != NULL) {
    if(strcmp(tmpValue, "Selected") == 0) {
      fprintf(file, "<option>Full</option>");
      fprintf(file, "<option selected>Selected</option>");
    }
    else {
      fprintf(file, "<option selected>Full</option>");
      fprintf(file, "<option>Selected</option>");
    }
    free(tmpValue);
  }
  else {
    fprintf(file, "<option selected>Full</option>");
    fprintf(file, "<option>Selected</option>");
  }
  fprintf(file, "</select></td>\n");

  if (queryString != NULL
      && (tmpValue = getHtmlFieldValue("mzLower", queryString)) != NULL) {
    fprintf(file, "<td><input type=\"text\" name=\"mzLower\" size=\"6\" onChange=\"change_selected('mzRange')\" value=\"%s\"/></td>\n", tmpValue);
    free(tmpValue);
  }
  else
    fprintf(file, "<td><input type=\"text\" name=\"mzLower\" size=\"6\" onChange=\"change_selected('mzRange')\"/></td>\n");

  fprintf(file, "<td>=&lt; m/z =&lt;</td>\n");
  if (queryString != NULL
      && (tmpValue = getHtmlFieldValue("mzUpper", queryString)) != NULL) {
    fprintf(file, "<td><input type=\"text\" name=\"mzUpper\" size=\"6\" onChange=\"change_selected('mzRange')\" value=\"%s\"/></td>\n", tmpValue);
    free(tmpValue);
  }
  else
    fprintf(file, "<td><input type=\"text\" name=\"mzUpper\" size=\"6\" onChange=\"change_selected('mzRange')\"/></td>\n");

  fprintf(file, "<td title=\"Increment to plot, in Thompsons\">Plot "); 
  if (queryString != NULL
      && (tmpValue = getHtmlFieldValue("mzGrid", queryString)) != NULL) {
    fprintf(file, "<input type=\"text\" name=\"mzGrid\" size=\"4\" value=\"%s\"/>", tmpValue);
    free(tmpValue);
  }
  else
    fprintf(file, "<input type=\"text\" name=\"mzGrid\" size=\"4\" value=\"2\"/>");
  fprintf(file, "units of m/z,</td>\n"); 

  fprintf(file, "<td title=\"Size on image, in pixels\">over "); 
  if (queryString != NULL
      && (tmpValue = getHtmlFieldValue("mzImgGrid", queryString)) != NULL) {
    fprintf(file, "<input type=\"text\" name=\"mzImgGrid\" size=\"4\" value=\"%s\"/>", tmpValue);
    free(tmpValue);
  }
  else
    fprintf(file, "<input type=\"text\" name=\"mzImgGrid\" size=\"4\" value=\"1\"/>");
  fprintf(file, "pixels</td>\n</tr>\n");
  fflush(file);


  // elution time range
  fprintf(file, "<tr id=\"time\" onmouseover=\"hilight('time','yes')\" onmouseout=\"hilight('time','no')\">\n");
  fprintf(file, "<td><li title=\"in minutes\">Elution time range:</li></td>\n");

  fprintf(file, "<td><select size=\"1\" name=\"scanRange\" id=\"scanRange\">");

  if (queryString != NULL
      && (tmpValue = getHtmlFieldValue("scanRange", queryString)) != NULL) {
    if(strcmp(tmpValue, "Selected") == 0) {
      fprintf(file, "<option>Full</option>");
      fprintf(file, "<option selected>Selected</option>");
    }
    else {
      fprintf(file, "<option selected>Full</option>");
      fprintf(file, "<option>Selected</option>");
    }
    free(tmpValue);
  }
  else {
    fprintf(file, "<option selected>Full</option>");
    fprintf(file, "<option>Selected</option>");
  }
  fprintf(file, "</select></td>\n");

  if (queryString != NULL
      && (tmpValue = getHtmlFieldValue("scanLower", queryString)) != NULL) {
    fprintf(file, "<td><input type=\"text\" name=\"scanLower\" size=\"6\" onChange=\"change_selected('scanRange')\" value=\"%s\"/></td>\n", tmpValue);
    free(tmpValue);
  }
  else
    fprintf(file, "<td><input type=\"text\" name=\"scanLower\" size=\"6\" onChange=\"change_selected('scanRange')\"/></td>\n");

  fprintf(file, "<td>=&lt; time =&lt;</td>\n");
  if (queryString != NULL
      && (tmpValue = getHtmlFieldValue("scanUpper", queryString)) != NULL) {
    fprintf(file, "<td><input type=\"text\" name=\"scanUpper\" size=\"6\" onChange=\"change_selected('scanRange')\" value=\"%s\"/></td>\n", tmpValue);
    free(tmpValue);
  }
  else
    fprintf(file, "<td><input type=\"text\" name=\"scanUpper\" size=\"6\" onChange=\"change_selected('scanRange')\"/></td>\n");

  fprintf(file, "<td title=\"Increment to plot, in minutes\">Plot "); 
  if (queryString != NULL
      && (tmpValue = getHtmlFieldValue("scanGrid", queryString)) != NULL) {
    fprintf(file, "<input type=\"text\" name=\"scanGrid\" size=\"4\" value=\"%s\"/>", tmpValue);
    free(tmpValue);
  }
  else
    fprintf(file, "<input type=\"text\" name=\"scanGrid\" size=\"4\" value=\"0.5\"/>");
  fprintf(file, "minutes,</td>\n");

  fprintf(file, "<td title=\"Size on image, in pixels\">over "); 
  if (queryString != NULL
      && (tmpValue = getHtmlFieldValue("scanImgGrid", queryString)) != NULL) {
    fprintf(file, "<input type=\"text\" name=\"scanImgGrid\" size=\"4\" value=\"%s\"/>", tmpValue);
    free(tmpValue);
  }
  else
    fprintf(file, "<input type=\"text\" name=\"scanImgGrid\" size=\"4\" value=\"2\"/>");
  fprintf(file, "pixels</td>\n</tr>\n");
  fflush(file);


  // intensity
  fprintf(file, "<tr id=\"intensity\" onmouseover=\"hilight('intensity','yes')\" onmouseout=\"hilight('intensity','no')\">\n");
  fprintf(file, "<td><li>Intensity range:</li></td>\n");

  fprintf(file, "<td><select size=\"1\" name=\"peakRange\" id=\"peakRange\">");
  if (queryString != NULL
      && (tmpValue = getHtmlFieldValue("peakRange", queryString)) != NULL) {
    if(strcmp(tmpValue, "Absolute") == 0) {
      fprintf(file, "<option selected>Absolute</option>");
      fprintf(file, "<option>unit of background</option>");
    }
    else {
      fprintf(file, "<option selected>unit of background</option>");
      fprintf(file, "<option>Absolute</option>");
    }
    free(tmpValue);
  }
  else {
    fprintf(file, "<option selected>unit of background</option>");
    fprintf(file, "<option>Absolute</option>");
  }
  fprintf(file, "</select></td>\n");

  if (queryString != NULL
      && (tmpValue = getHtmlFieldValue("peakLower", queryString)) != NULL) {
    fprintf(file, "<td><input type=\"text\" name=\"peakLower\" size=\"6\" value=\"%s\"/></td>\n", tmpValue);
    free(tmpValue);
  }
  else
    fprintf(file, "<td><input type=\"text\" name=\"peakLower\" size=\"6\" value=\"1\"/></td>\n");

  fprintf(file, "<td>=&lt; int. =&lt;</td>\n");
  if (queryString != NULL
      && (tmpValue = getHtmlFieldValue("peakUpper", queryString)) != NULL) {
    fprintf(file, "<td><input type=\"text\" name=\"peakUpper\" size=\"6\" value=\"%s\"/></td>\n", tmpValue);
    free(tmpValue);
  }
  else
    fprintf(file, "<td><input type=\"text\" name=\"peakUpper\" size=\"6\" value=\"20\"/></td>\n");
  fprintf(file, "</td>\n</tr>\n");


  // peptide list
  fprintf(file, "<tr id=\"peptides\" onmouseover=\"hilight('peptides','yes')\" onmouseout=\"hilight('peptides','no')\">\n");
  fprintf(file, "<td><li>Display peptides:</li></td>\n");

  fprintf(file, "<td><select size=\"1\" name=\"pepDisplay\" id=\"pepDisplay\">");
  if (queryString != NULL
      && (tmpValue = getHtmlFieldValue("pepDisplay", queryString)) != NULL) {
    if(strcmp(tmpValue, "All") == 0) {
      fprintf(file, "<option selected>All</option>");
      fprintf(file, "<option>None</option>");
      fprintf(file, "<option>CID</option>");
      fprintf(file, "<option>Peptide</option>");
    }
    else if(strcmp(tmpValue, "CID") == 0) {
      fprintf(file, "<option selected>CID</option>");
      fprintf(file, "<option>None</option>");
      fprintf(file, "<option>Peptide</option>");
      fprintf(file, "<option>All</option>");
    }
    else if(strcmp(tmpValue, "Peptide") == 0) {
      fprintf(file, "<option selected>Peptide</option>");
      fprintf(file, "<option>None</option>");
      fprintf(file, "<option>CID</option>");
      fprintf(file, "<option>All</option>");
    }
    else {
      fprintf(file, "<option selected>None</option>");
      fprintf(file, "<option>CID</option>");
      fprintf(file, "<option>Peptide</option>");
      fprintf(file, "<option>All</option>");
    }
    free(tmpValue);
  }
  else {
    fprintf(file, "<option selected>None</option>");
    fprintf(file, "<option>CID</option>");
    fprintf(file, "<option>Peptide</option>");
    fprintf(file, "<option>All</option>");
  }
  fprintf(file, "</select></td>\n");
  fprintf(file, "<td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td>\n");

  fprintf(file, "<td title=\"Size on image, in pixels\">Size:"); 
  if (queryString != NULL
      && (tmpValue = getHtmlFieldValue("pepImgGrid", queryString)) != NULL) {
    fprintf(file, "<input type=\"text\" name=\"pepImgGrid\" size=\"4\" value=\"%s\"/>", tmpValue);
    free(tmpValue);
  }
  else
    fprintf(file, "<input type=\"text\" name=\"pepImgGrid\" size=\"4\" value=\"2\"/>");
  fprintf(file, "pixels</td>\n</tr>\n"); 


  // Score type and range
  fprintf(file, "<tr id=\"score\" onmouseover=\"hilight('score','yes')\" onmouseout=\"hilight('score','no')\">\n");
  fprintf(file, "<td><li>Score type:</li></td>\n");

  fprintf(file, "<td><select size=\"1\" name=\"scoreType\">");
  if (queryString != NULL
      && (tmpValue = getHtmlFieldValue("scoreType", queryString)) != NULL) {
    if(strcmp(tmpValue, "SEQUEST Xcorr") == 0) {
      fprintf(file, "<option>PeptideProphet probability</option>");
      fprintf(file, "<option selected>SEQUEST Xcorr</option>");
    }
    else {
      fprintf(file, "<option selected>PeptideProphet probability</option>");
      fprintf(file, "<option>SEQUEST Xcorr</option>");
    }
    free(tmpValue);
  }
  else {
    fprintf(file, "<option selected>PeptideProphet probability</option>");
    fprintf(file, "<option>SEQUEST Xcorr</option>");
  }
  fprintf(file, "</select></td>\n");

  if (queryString != NULL
      && (tmpValue = getHtmlFieldValue("scoreLower", queryString)) != NULL) {
    fprintf(file, "<td><input type=\"text\" name=\"scoreLower\" size=\"6\" value=\"%s\"/></td>", tmpValue);
    free(tmpValue);
  }
  else
    fprintf(file, "<td><input type=\"text\" name=\"scoreLower\" size=\"6\" value=\"0.5\"/></td>\n");

  fprintf(file, "<td>=&lt; score =&lt;</td>"); 
  if (queryString != NULL
      && (tmpValue = getHtmlFieldValue("scoreUpper", queryString)) != NULL) {
    fprintf(file, "<td><input type=\"text\" name=\"scoreUpper\" size=\"6\" value=\"%s\"/></td>\n", tmpValue);
    free(tmpValue);
  }
  else
    fprintf(file, "<td><input type=\"text\" name=\"scoreUpper\" size=\"6\"/></td>\n");
  fprintf(file, "</tr>\n");
  fflush(file);


  // mapping function
  fprintf(file, "<tr id=\"mapping\" onmouseover=\"hilight('mapping','yes')\" onmouseout=\"hilight('mapping','no')\">\n");
  fprintf(file, "<td><li>Mapping function:</li></td>\n");

  fprintf(file, "<td><select size=\"1\" name=\"function\">");
  if (queryString != NULL
      && (tmpValue = getHtmlFieldValue("function", queryString)) != NULL) {
    if(strcmp(tmpValue, "Log (base 10)") == 0) {
      fprintf(file, "<option selected>Log (base 10)</option>");
      fprintf(file, "<option>Linear</option>");
    }
    else {
      fprintf(file, "<option selected>Linear</option>");
      fprintf(file, "<option>Log (base 10)</option>");
    }
    free(tmpValue);
  }
  else {
    fprintf(file, "<option selected>Linear</option>");
    fprintf(file, "<option>Log (base 10)</option>");
  }
  fprintf(file, "</select></td>\n</tr>\n");
  fflush(file);


  // Image type
  fprintf(file, "<tr id=\"imgtype\" onmouseover=\"hilight('imgtype','yes')\" onmouseout=\"hilight('imgtype','no')\">\n");
  fprintf(file, "<td><li>Image type:</li></td>\n");

  fprintf(file, "<td><select size=\"1\" name=\"image\">");
  if (queryString != NULL
      && (tmpValue = getHtmlFieldValue("image", queryString)) != NULL) {
    if(strcmp(tmpValue, "Section") == 0) {
      fprintf(file, "<option selected>Section</option>");
      fprintf(file, "<option>Full</option>");
    }
    else {
      fprintf(file, "<option selected>Full</option>");
      fprintf(file, "<option>Section</option>");
    }
    free(tmpValue);
  }
  else {
    fprintf(file, "<option selected>Full</option>");
    fprintf(file, "<option>Section</option>");
  }
  fprintf(file, "</select></td>\n</tr>\n");
  fflush(file);


  // submit
  fprintf(file, "<tr>\n<td>&nbsp;</td>\n");
  fprintf(file, "<td><input type=\"submit\" name=\"submit\" value=\"Generate Pep3D image\" /></td>\n");
  fprintf(file, "<td>&nbsp;</td>\n");

  fprintf(file, "<td colspan=\"3\"><input type=\"submit\" name=\"submit\" value=\"Save as\" />\n");
  if (queryString != NULL
      && (tmpValue = getHtmlFieldValue("saveFile", queryString)) != NULL) {
    fprintf(file, "<input type=\"text\" name=\"saveFile\" size=\"20\" value=\"%s\"/>", tmpValue);
    free(tmpValue);
  }
  else
    fprintf(file, "<input type=\"text\" name=\"saveFile\" size=\"20\" value=\"Pep3D.htm\"/>");

  fprintf(file, "</td>\n</tr>\n");
  fprintf(file, "</table>\n</div>\n");
  fflush(file);

  return;
}


/************************************************************************
 *
 * Report an application error and exit if iExitCode is non-zero
 *
 ************************************************************************/
void reportError(const char *szMessage, int iExitCode)
{

  if(iOutputType == HTML_OUTPUT)
    prntMsg(szMessage);
  else if(iOutputType == XML_OUTPUT) {
    printf("<STATUS>ERROR</STATUS>\n");
    printf("<MESSAGE>%s</MESSAGE>\n", szMessage);
  }

  fflush(stdout);

  if (iExitCode != 0) {
    if(iOutputType == HTML_OUTPUT)
      prntPageFooter(stdout);
    else if(iOutputType == XML_OUTPUT)
      printf("</XML>");

    fflush(stdout);
    exit(iExitCode);

  } else
    return;
}


/************************************************************************
 *
 * This function prints a message in an html pane
 *
 ************************************************************************/
void prntMsg(const char *message)
{
  if (!inMsgPane) {
    printf("<table cellspacing=\"0\">\n");
    printf("<tbody>\n<tr>\n");
    printf("<td class=\"banner1\">&nbsp;&nbsp;&nbsp;Messages&nbsp;&nbsp;&nbsp;</td>\n");
    printf("<td align=\"left\">&nbsp;&nbsp;&nbsp;&nbsp;<u><a onclick=\"showdiv('msgs','messages')\">[ Show | Hide ]</a></u></td>\n");
    printf("</tr></tbody>\n</table>\n");
    printf("<div id=\"msgs\" class=\"messages\">\n");
    printf("</div>\n\n");
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


/************************************************************************
 *
 * This function determines the output type  (HTML, XML)
 *
 ************************************************************************/
int getOutputType(const char *queryString)
{
  char *tmpValue;

  if (queryString != NULL
      && (tmpValue = getHtmlFieldValue("output", queryString)) != NULL) {
    if(strcmp(tmpValue, "xml") == 0)
      return XML_OUTPUT;
    else
      return HTML_OUTPUT;
  }

  return HTML_OUTPUT; // default
}


/************************************************************************
 *
 * This function prints the html page footer
 *
 ************************************************************************/
void prntPageFooter(FILE *file)
{

  fprintf(file,"<hr noshade=\"noshade\">\n");
  fprintf(file,"<h6>(%s)<br/>\n",szTPPVersionInfo);
  fprintf(file,"This program displays LC-ESI-MS data stored in mzXML format in a Pep3D image.<br/>\n");
  fprintf(file,"Developed by Dr. Xiao-jun Li at <a href=\"http://www.systemsbiology.org/\"> Institute for Systems Biology. </a> </h6>\n");
  //  fprintf(file, "Date: April 7, 2004<br/><br/>\n");
  fprintf(file,"</body>\n</html>\n");

  return;
}


/************************************************************************
 *
 * void Pep3D(const char *queryString)
 * This function generates a Pep3D gel image.
 *
 ************************************************************************/
void Pep3D(const char *queryString)
{
  void prntFrontPage(FILE *file, const char *queryString);
  get_mzXML_pathes_FilePathStrct *getMzXMLPathes(const char *htmlFile, int *xmlFileNum);
  void freeMzXMLPathes(get_mzXML_pathes_FilePathStrct *xmlFiles, 
		       int xmlFileNum);
  char **getPngFile(int *linkNum, const char *pngFile, const char *htmlFile, 
		    get_mzXML_pathes_FilePathStrct &xmlFile,
		    const rangeStrct &mzRange, const rangeStrct &scanRange, 
		    const rangeStrct &peakRange,const rangeStrct &scoreRange, 
		    int mzImgGrid, int scanImgGrid, int pepImgGrid, 
		    int peakIndx, int pepIndx, int scoreIndx, int fnIndx,
		    int sectIndx, Boolean makeLinks);

  char tmpMsg[500];

  // parameters
  char *htmlFile; // interact-data.htm file
  int mzIndx, scanIndx, peakIndx, pepIndx, scoreIndx, fnIndx, sectIndx; 
  rangeStrct mzRange, scanRange, peakRange, scoreRange;
  int mzImgGrid, scanImgGrid, pepImgGrid;
  char *saveFile; // saved html file

  // file
  int fileIndx, saveIndx;
  char pngFile[512]; // png file
  char pngFileBase[512];// = "/tmp/Pep3D_";
  char expLbl[512];

  // variables
  FILE *file, *pep3d=NULL;
  char *directory;
  char *tmpValue;
  int lngth;
  get_mzXML_pathes_FilePathStrct *xmlFiles=NULL;
  int xmlFileNum;
  char cmd[4100];
  char tmpString[4096];
  time_t currTime;
  char **links;
  int linkNum;

  int i, j;
  Boolean display_all = False;
  Boolean make_links = True;

  /*
   * collect parameters from web
   */

  /*
   * interact-data.htm file
   */
  // input
  if((htmlFile = getHtmlFieldValue("htmFile", queryString)) == NULL){
    reportError("<b>No input file!</b>",2);
    fflush(stdout);
    return;
  }

  fixPath(htmlFile,1); // pretty up the path seperators etc - expect existence


  // full path
  if(!isAbsolutePath(htmlFile)) { 
    free(htmlFile);
    reportError("<b>Error:</b> Provide <b>FULL path</b> for input file!",4);
    return;
  }

  pngFileBase[0]='\0';

#ifdef USING_RELATIVE_WEBSERVER_PATH  // for example, in win32 understand /foo/bar as /Inetpub/wwwroot/foo/bar
  char szWebserverRoot[512];
  const char *pStr;
  strcat(pngFileBase, "/Pep3D_");
  pStr=getWebserverRoot();
  if (pStr==NULL)
  {
    if(iOutputType == HTML_OUTPUT) {
      prntMsg("");
      prntMsg("<b>Configuration Error!</b>");

      prntMsg(" Environment variable <code>WEBSERVER_ROOT</code> does not exist.");
      prntMsg(" For Windows users, you can set this environment variable");
      prntMsg(" through the Advanced tab under <code>System Properties</code> when you");
      prntMsg(" right-mouse-click on your <code>My Computer</code> icon.");
      prntMsg("");
      prntMsg(" Set this environment variable to your webserver's document");
      prntMsg(" root directory such as <code>c:\\inetpub\\wwwroot</code> for IIS or");
      prntMsg(" <code>c:\\website\\htdocs</code> or WebSite Pro.");
      prntMsg(" Exiting.");

      prntPageFooter(stdout);
      exit(6);

    } else if(iOutputType == XML_OUTPUT) {
      reportError("Environment variable WEBSERVER_ROOT does not exist.",6);
    }

  }
  else
  {
#ifdef CYGWIN_WINDOWS
    FILE *fp;
    char szBuf[512];
    // must first pass to cygpath program
    sprintf(szCommand, "cygpath -u '%s'", pStr);
    if((fp = popen(szCommand, "r")) == NULL)
    {
      reportError("cygpath error, exiting",8);
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
#ifdef CYGWIN_WINDOWS
  /*
   * transform input to Unix path
   */
  FILE *fp;
  char htmlFileOrig[512];
  char szCommand[512];
  sprintf(szCommand, "cygpath -u '%s'", htmlFile);
  
  if ((fp=popen(szCommand,"r"))!=NULL)
    {
      char szBuf[512];
      
      fgets(szBuf, 512,fp);
      pclose(fp);
      szBuf[strlen(szBuf)-1]=0;
      strcpy(htmlFileOrig, htmlFile);
      htmlFile = (char *) realloc(htmlFile, strlen(szBuf)*sizeof(char));
      strcpy(htmlFile, szBuf);
    }
  else
    {
      sprintf(tmpMsg," Error with command %s", szCommand);
      reportError(tmpMsg,10);
    }
#endif
#endif


  // check on xml file
  if(hasValidPepXMLFilenameExt(htmlFile) != NULL) {
    if ((tmpValue = getHtmlFieldValue("orgIndx", queryString)) != NULL) {
      if(sscanf(tmpValue, "%d", &fileIndx) != 1
	 || fileIndx < 0 || fileIndx > 1)
	fileIndx = 0;
      else 
	fileIndx = 1 - fileIndx;
      free(tmpValue);
    }
    else
      fileIndx = 0;
  }
  else if(rampValidFileType(htmlFile) != NULL)
    fileIndx = 2;
  else {
    free(htmlFile);
    reportError("<b>Error:</b> Only pepxml, .mzData or .mzXML file is accepted!",12);
    return;   
  }

  if((tmpValue = getHtmlFieldValue("submit", queryString)) != NULL
     && (strcmp(tmpValue, "Save as") == 0 || strcmp(tmpValue, "Generate Pep3D image") )) {
    saveIndx = 1;
  } else {
     saveIndx = 0;
  }

  // directory
  lngth = (int)(strlen(htmlFile) + (getWebserverRoot()?strlen(getWebserverRoot()):0))+1;
  directory = (char *) calloc(lngth, sizeof(char));
  strcpy(directory, htmlFile);
  if (!saveIndx) { // do all our work in designated tmp dir
    replace_path_with_webserver_tmp(directory,lngth);
  }
  char *slash = findRightmostPathSeperator(directory);
  if (slash) {
	  *slash = 0;
  }
#ifdef WINDOWS_CYGWIN
  char *curr_dir;
  char szCommand[1024];
  curr_dir =  (char *) calloc(lngth+1, sizeof(char));
  strcpy(curr_dir, directory); 
  sprintf(szCommand, "cygpath -u '%s'", directory);
  FILE *fp;
  if ((fp=popen(szCommand,"r"))!=NULL)
  {
    char szBuf[512];
    fgets(szBuf, 512,fp);
    pclose(fp);
    szBuf[strlen(szBuf)-1]=0;
    directory = (char *) realloc(directory, strlen(szBuf)*sizeof(char));
    strcpy(directory, szBuf);
  }
#endif
  strcpy(pngFileBase, directory);
  if (isPathSeperator(pngFileBase[strlen(pngFileBase)-1])) 
  {
    strcat(pngFileBase, "Pep3D_");
  }
  else {
    strcat(pngFileBase, "/Pep3D_");
  }
  if (!saveIndx) { // possibly working in a common tmpdir, create a unique basename
	  strcat(pngFileBase,"XXXXXX");
	  safe_fclose(FILE_mkstemp(pngFileBase)); // create then close a uniquely named file
	  unlink(pngFileBase); // we just want its name, doesn't need to exist
  }

  if (chdir(directory) != 0) {
    sprintf(tmpMsg,"<b>Error:</b> Cannot access directory %s!", directory);
    free(htmlFile);
    free(directory);
    reportError(tmpMsg,14);
    return;   
  }

  /*
   * output files
   */
  if(saveIndx) {
    snprintf(pngFileBase, sizeof(pngFileBase), "%s/Pep3D_", directory);
    if ((saveFile = getHtmlFieldValue("saveFile", queryString)) != NULL){
      if(strncmp(saveFile, "Pep3D", 5) != 0
	 || strstr(saveFile, ".htm") == NULL){
	saveFile = (char *) realloc(saveFile, 10*sizeof(char));
	strcpy(saveFile, "Pep3D.htm");
      }
    }
    else {
      saveFile = (char *) calloc(10, sizeof(char));
      strcpy(saveFile, "Pep3D.htm");
    }

    if((pep3d = fopen(saveFile, "w")) == NULL) {
      sprintf(tmpMsg,"<b>Error:</b> Cannot open %s for writing!", saveFile);
      free(saveFile);
      free(tmpValue);
      free(htmlFile);
      free(directory);
      reportError(tmpMsg,18);
      return;
    }
    else {
      prntFrontPage(pep3d, queryString);
    }
    free(saveFile);
  } //   if((tmpValue = getHtmlFieldValue("submit", queryString)) != NULL
    free(tmpValue);
  free(directory);

  /*
   * m/z range
   */
  // m/z index
  if ((tmpValue = getHtmlFieldValue("mzRange", queryString)) != NULL) {
    if(strcmp(tmpValue, "Selected") == 0) 
      mzIndx = 1;
    else
      mzIndx = 0;
    free(tmpValue);
  }
  else
    mzIndx = 0;

  // m/z range
  if(mzIndx == 1) {
    if ((tmpValue = getHtmlFieldValue("mzLower", queryString)) != NULL) {
      if(sscanf(tmpValue, "%lf", &(mzRange.bnd[0])) != 1
	 || mzRange.bnd[0] < 0.) 
	mzRange.bnd[0] = -1.;
      free(tmpValue);
    }
    else
      mzRange.bnd[0] = -1.;

    if ((tmpValue = getHtmlFieldValue("mzUpper", queryString)) != NULL) {
      if(sscanf(tmpValue, "%lf", &(mzRange.bnd[1])) != 1
	 || mzRange.bnd[1] < 0.) 
	mzRange.bnd[1] = -1.;
      free(tmpValue);
    }
    else
      mzRange.bnd[1] = -1.;
  }
  else {
    mzRange.bnd[0] = -1.;
    mzRange.bnd[1] = -1.;    
  }
  
  // m/z grid
  if ((tmpValue = getHtmlFieldValue("mzGrid", queryString)) != NULL) {
    if(sscanf(tmpValue, "%lf", &mzRange.grd) != 1
       || mzRange.grd < 0.) 
      mzRange.grd = 0.5;
    free(tmpValue);
  }
  else
    mzRange.grd = 0.5;

  // m/z image grid
  if ((tmpValue = getHtmlFieldValue("mzImgGrid", queryString)) != NULL) {
    if(sscanf(tmpValue, "%d", &mzImgGrid) != 1
       || mzImgGrid < 1) 
      mzImgGrid = 1;
    free(tmpValue);
  }
  else
    mzImgGrid = 1;

  /*
   * scan range
   */
  // scan index
  if ((tmpValue = getHtmlFieldValue("scanRange", queryString)) != NULL) {
    if(strcmp(tmpValue, "Selected") == 0) 
      scanIndx = 1;
    else
      scanIndx = 0;
    free(tmpValue);
  }
  else
    scanIndx = 0;

  // scan range
  if(scanIndx == 1) {
    if ((tmpValue = getHtmlFieldValue("scanLower", queryString)) != NULL) {
      if(sscanf(tmpValue, "%lf", &(scanRange.bnd[0])) != 1
	 || scanRange.bnd[0] < 0.) 
	scanRange.bnd[0] = -1.;
      free(tmpValue);
    }
    else
      scanRange.bnd[0] = -1.;

    if ((tmpValue = getHtmlFieldValue("scanUpper", queryString)) != NULL) {
      if(sscanf(tmpValue, "%lf", &(scanRange.bnd[1])) != 1
	 || scanRange.bnd[1] < 0.) 
	scanRange.bnd[1] = -1.;
      free(tmpValue);
    }
    else
      scanRange.bnd[1] = -1.;
  }
  else {
    scanRange.bnd[0] = -1.;
    scanRange.bnd[1] = -1.;    
  }
  
  // scan grid
  if ((tmpValue = getHtmlFieldValue("scanGrid", queryString)) != NULL) {
    if(sscanf(tmpValue, "%lf", &scanRange.grd) != 1
       || scanRange.grd < 0.) 
      scanRange.grd = 0.5;
    free(tmpValue);
  }
  else
    scanRange.grd = 0.5;

  // scan image grid
  if ((tmpValue = getHtmlFieldValue("scanImgGrid", queryString)) != NULL) {
    if(sscanf(tmpValue, "%d", &scanImgGrid) != 1
       || scanImgGrid < 1) 
      scanImgGrid = 1;
    free(tmpValue);
  }
  else
    scanImgGrid = 1;

  /*
   * intensity range
   */
  // peak index
  if ((tmpValue = getHtmlFieldValue("peakRange", queryString)) != NULL) {
    if(strcmp(tmpValue, "Absolute") == 0) 
      peakIndx = 1;
    else
      peakIndx = 0;
    free(tmpValue);
  }
  else
    peakIndx = 0;

  // peak range
  if ((tmpValue = getHtmlFieldValue("peakLower", queryString)) != NULL) {
    if(sscanf(tmpValue, "%lf", &(peakRange.bnd[0])) != 1
       || peakRange.bnd[0] < 0.) 
      peakRange.bnd[0] = 1.;
    free(tmpValue);
  }
  else
    peakRange.bnd[0] = 1.;

  if ((tmpValue = getHtmlFieldValue("peakUpper", queryString)) != NULL) {
    if(sscanf(tmpValue, "%lf", &(peakRange.bnd[1])) != 1
       || peakRange.bnd[1] < 0.) 
      peakRange.bnd[1] = -1.;
    free(tmpValue);
  }
  else
    peakRange.bnd[1] = -1.;

  
  /*
   * peptide display
   */
  // pepIndx
  if ((tmpValue = getHtmlFieldValue("pepDisplay", queryString)) != NULL) {
    if(strcmp(tmpValue, "All") == 0) 
      pepIndx = 3;
    else if(strcmp(tmpValue, "Peptide") == 0) 
      pepIndx = 2;
    else if(strcmp(tmpValue, "CID") == 0) 
      pepIndx = 1;
    else
      pepIndx = 0;
    free(tmpValue);
  }
  else
    pepIndx = 0;

  // pepImgGrid
  if((tmpValue = getHtmlFieldValue("pepImgGrid", queryString)) != NULL) {
    if(sscanf(tmpValue, "%d", &pepImgGrid) != 1
       || pepImgGrid < 1) 
      pepImgGrid = 2;
    free(tmpValue);
  }
  else
    pepImgGrid = 2;

  // scoreIndx
  if ((tmpValue = getHtmlFieldValue("scoreType", queryString)) != NULL) {
    if(strcmp(tmpValue, "SEQUEST Xcorr") == 0) 
      scoreIndx = 1;
    else
      scoreIndx = 0;
    free(tmpValue);
  }
  else
    scoreIndx = 0;

  // score range
  if ((tmpValue = getHtmlFieldValue("scoreLower", queryString)) != NULL) {
    if(sscanf(tmpValue, "%lf", &(scoreRange.bnd[0])) != 1
       || scoreRange.bnd[0] < 0.)
      scoreRange.bnd[0] = 0.5;
    free(tmpValue);
  }
  else
    scoreRange.bnd[0] = 0.5;

  // display_all
  if ((tmpValue = getHtmlFieldValue("display_all", queryString)) != NULL) {
    display_all = True;
    free(tmpValue);
  }


  if ((tmpValue = getHtmlFieldValue("scoreUpper", queryString)) != NULL) {
    if(sscanf(tmpValue, "%lf", &(scoreRange.bnd[1])) != 1
       || scoreRange.bnd[1] < scoreRange.bnd[0]) 
      scoreRange.bnd[1] = -1.;
    free(tmpValue);
  }
  else
    scoreRange.bnd[1] = -1.;


  /*
   * function index
   */
  if ((tmpValue = getHtmlFieldValue("function", queryString)) != NULL) {
    if(strcmp(tmpValue, "Log (base 10)") == 0) 
      fnIndx = 1;
    else
      fnIndx = 0;
    free(tmpValue);
  }
  else
    fnIndx = 0;

  
  /*
   * section index
   */
  if ((tmpValue = getHtmlFieldValue("image", queryString)) != NULL) {
    if(strcmp(tmpValue, "Section") == 0) 
      sectIndx = 1;
    else
      sectIndx = 0;
    free(tmpValue);
  }
  else
    sectIndx = 0;

  
  /* 
   * generate png file
   */
  // get all .mzXML files
  // get xmlFiles
  if(fileIndx < 2){
    if((xmlFiles = getMzXMLPathes(htmlFile, &xmlFileNum)) == NULL){
      free(htmlFile);
      if(saveIndx == 1)
	fclose(pep3d);
      reportError("No valid .mzXML/.mzData file found.",18);
      return;
    }
  }
  else if(fileIndx == 2) {
    sprintf(cmd, "ls %s", htmlFile);
    if((file = popen(cmd, "r")) == NULL) {
      if(saveIndx == 1)
	fclose(pep3d);
      pclose(file);
      sprintf(tmpMsg,"<b>Error:</b> No such .mzXML/.mzData file %s!", htmlFile);
      free(htmlFile);
      reportError(tmpMsg,20);
      return;
    }

    xmlFileNum = 0;
    while(!feof(file)
	  && fgets(tmpString, 1000, file) != NULL){
      getRidOfSpace(tmpString);
      if((tmpValue = rampValidFileType(tmpString)) != NULL){
	++xmlFileNum;
      }
    }
    pclose(file);

    if(xmlFileNum < 1){
      if(saveIndx == 1)
	fclose(pep3d);
      sprintf(tmpMsg,"<b>Error:</b> No such .mzXML/.mzData file %s!", htmlFile);
      free(htmlFile);
      reportError(tmpMsg,22);
      return;
    }

    xmlFiles = (get_mzXML_pathes_FilePathStrct *) 
      calloc(xmlFileNum, sizeof(get_mzXML_pathes_FilePathStrct));

    if((file = popen(cmd, "r")) == NULL) {
      if(saveIndx == 1)
	fclose(pep3d);
      sprintf(tmpMsg,"<b>Error:</b> No such .mzXML/.mzData file %s!", htmlFile);
      free(htmlFile);
      reportError(tmpMsg,24);
      return;
    }

    xmlFileNum = 0;
    while(!feof(file)
	  && fgets(tmpString, 1000, file) != NULL){ 
      getRidOfSpace(tmpString);
      if((tmpValue = rampValidFileType(tmpString)) != NULL){
	lngth = strlen(tmpString);
	xmlFiles[xmlFileNum].path = strdup(tmpString);
	strcpy(expLbl, tmpString);
	expLbl[lngth-strlen(tmpValue)] = '\0';
	if((tmpValue = findRightmostPathSeperator(expLbl)) != NULL) {
	  strcpy(expLbl, tmpValue+1);
	}
	xmlFiles[xmlFileNum].expLbl = strdup(expLbl);
	++xmlFileNum;
      }
    } // while(!feof(file)
    pclose(file);
  } // else if(fileIndx == 2) 

  /*
   * generate png file for each .mzXML file
   */

  char display_tag[200];

  int num_checkboxes = 0;
  for (i = 0; i < xmlFileNum; ++i) {
    sprintf(display_tag, "xmlfile_%d", i+1);
    if(getHtmlFieldValue(display_tag, queryString) != NULL)
      num_checkboxes++;
  }
  if(num_checkboxes == 0)
    display_all = True;

  if( (num_checkboxes > 1) || ((num_checkboxes == 0) && (xmlFileNum > 1)) || (iOutputType == XML_OUTPUT) )
    make_links = False;

  if ( (!make_links) && (pepIndx != 0) ) {
    if(iOutputType == HTML_OUTPUT) {
      prntMsg("More than one mzXML file in input; links on images *not* active");
    } else if(iOutputType == XML_OUTPUT) {
      printf("<MESSAGE>Links on images *not* active</MESSAGE>\n");
    }
  }

  for (i = 0; i < xmlFileNum; ++i) {
    sprintf(display_tag, "xmlfile_%d", i+1);
    // put the checkbox check here...
    if(! display_all && getHtmlFieldValue(display_tag, queryString) == NULL){
      continue;
    }

    if(iOutputType == HTML_OUTPUT) {
      printf("<br/><br/>\n\n<table cellspacing=\"0\" width=\"100%%\">\n<tbody>\n<tr>\n");
      printf("<td class=\"banner_cid\">&nbsp;&nbsp;&nbsp;<input type=\"checkbox\" name=\"%s\" CHECKED/>  ", display_tag);
      printf("%s&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>\n", xmlFiles[i].expLbl);
      printf("<td align=\"left\">&nbsp;&nbsp;&nbsp;&nbsp;<u>");
      printf("<a onclick=\"showdiv('div_%s','image')\">[ Show | Hide ]</a></u></td>\n",display_tag);

      if(saveIndx == 1) {
	fprintf(pep3d,"<br/><br/>\n\n<table cellspacing=\"0\" width=\"100%%\">\n<tbody>\n<tr>\n");
	fprintf(pep3d,"<td class=\"banner_cid\">&nbsp;&nbsp;&nbsp;<input type=\"checkbox\" name=\"%s\" CHECKED/>  ", display_tag);
	fprintf(pep3d,"%s&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>\n", xmlFiles[i].expLbl);
	fprintf(pep3d,"<td align=\"left\">&nbsp;&nbsp;&nbsp;&nbsp;<u>");
	fprintf(pep3d,"<a onclick=\"showdiv('div_%s','image')\">[ Show | Hide ]</a></u></td>\n",display_tag);
      }
    } else if(iOutputType == XML_OUTPUT) {
      printf("<IMAGE seq=\"%d\">\n", i+1);
      printf("  <SRC_FILE>%s</SRC_FILE>\n", xmlFiles[i].expLbl);
    }
    fflush(stdout);

    // get pngFile
    strcpy(pngFile, pngFileBase);
    sprintf(pngFile+strlen(pngFile), "%s_", xmlFiles[i].expLbl);

    //DDS: THIS flush is important !!!
    fflush(stdout);

    (void) time(&currTime);
    srand48((long) currTime);
    sprintf(pngFile+strlen(pngFile), "%d.png", (int)lrand48());

    links = getPngFile(&linkNum, pngFile, htmlFile, xmlFiles[i],
		       mzRange, scanRange, peakRange, scoreRange, 
		       mzImgGrid, scanImgGrid, pepImgGrid, 
		       peakIndx, pepIndx, scoreIndx, fnIndx, sectIndx, make_links);

    if(links != NULL){
      if(iOutputType == HTML_OUTPUT) {
	printf("<td class=\"nav\" align=\"right\">%s</td>\n", links[0]);
      } else if(iOutputType == XML_OUTPUT) {
	printf("  %s", links[0]);
      }
      if(saveIndx == 1) 
	fprintf(pep3d, "<td class=\"nav\" align=\"right\">%s</td>\n", links[0]);
      free(links[0]);
    }

    if(iOutputType == HTML_OUTPUT) {
      printf("</tr>\n</tbody>\n</table>\n");
      printf("<div id=\"div_%s\" class=\"image\">\n",display_tag);
      if(saveIndx == 1) {
	fprintf(pep3d,"</tr>\n</tbody>\n</table>\n");
	fprintf(pep3d,"<div id=\"div_%s\" class=\"image\">\n",display_tag);
      }
    }

#ifdef USING_RELATIVE_WEBSERVER_PATH  // for example, in win32 understand /foo/bar as /Inetpub/wwwroot/foo/bar  // fix up the image path
	std::string tmpPng=translate_relative_webserver_root_path_to_absolute_filesystem_path(pngFile);
    strcpy(pngFile, tmpPng.c_str());
    //#endif
#else
    // jmt
    // adding code to deal with paths on linux/apache, borrowed from 
    // plot-msms
    {
      char szWebserverRoot[512];

      if (getenv("WEBSERVER_ROOT") != NULL) {
	strcpy(szWebserverRoot, getenv("WEBSERVER_ROOT"));
	char* result = strstri(pngFile, szWebserverRoot);
	if (result != NULL)
	  {
	    if (strlen(result) > strlen(szWebserverRoot) && result[strlen(szWebserverRoot)] != '/')
	      sprintf(pngFile, "/%s", result + strlen(szWebserverRoot));
	    else
	      strcpy(pngFile, result + strlen(szWebserverRoot));
	  }
      }
    }

#endif

    // print link
    if(iOutputType == HTML_OUTPUT) {
      printf("<img src=\"%s\" border=\"0\" usemap=\"#%smap\"/>\n", 
	     makeTmpPNGFileSrcRef(pngFile,(saveIndx == 1)).c_str(), xmlFiles[i].expLbl);
      printf("<map name=\"%smap\">\n", xmlFiles[i].expLbl);
      if(saveIndx == 1) {
	fprintf(pep3d, "<img src=\"%s\" border=\"0\" usemap=\"#%smap\"/>\n", 
		pngFile, xmlFiles[i].expLbl);
	fprintf(pep3d, "<map name=\"%smap\">\n", xmlFiles[i].expLbl);
      }
      if(links != NULL) {
	for (j = 1; j < linkNum; ++j) {
	  printf("%s\n", links[j]);
	  if(saveIndx == 1) 
	    fprintf(pep3d, "%s\n", links[j]);
	  free(links[j]);
	}
	free(links);
      }
      printf("</map>\n</div>\n");
      if(saveIndx == 1) 
	fprintf(pep3d, "</map>\n</div>\n");

    } else if(iOutputType == XML_OUTPUT) {

      printf("  <URL>%s</URL>\n", pngFile);
      printf("</IMAGE>\n");

    }
    fflush(stdout);

  } //for (i = 0; i < xmlFileNum; ++i) {

  // end form here
  if(iOutputType == HTML_OUTPUT) {
    printf("</form>\n\n");
    if(saveIndx == 1) {
      fprintf(pep3d, "</form>\n\n");
    }

  } else if(iOutputType == XML_OUTPUT) {
      printf("<NUM_IMAGES>%d</NUM_IMAGES>\n", xmlFileNum);
  }

  fflush(stdout);


  /*
   * cleanup
   */
  freeMzXMLPathes(xmlFiles, xmlFileNum);
  free(htmlFile);

  if(iOutputType == HTML_OUTPUT) {
    prntMsg("Done!");
    printf("<h6>Graphs are generated by using <a href=\"http://www.boutell.com/gd/\">GD</a> graphics library.</h6>\n");
    if(saveIndx == 1) {
      fprintf(pep3d, "<h6>Graphs are generated by using <a href=\"http://www.boutell.com/gd/\">GD</a> graphics library.</h6>\n");
      prntPageFooter(pep3d);
      fclose(pep3d);
    }
  }

  return;
}


/************************************************************************
 *
 * This function collects all unique ramp-supported mass spec file paths 
 * within a "interact-data.html" file.
 *
 ************************************************************************/
get_mzXML_pathes_FilePathStrct *getMzXMLPathes(const char *htmlFile, int *xmlFileNum)
{
  void freeMzXMLPathes(get_mzXML_pathes_FilePathStrct *xmlFiles, 
		       int xmlFileNum);

  get_mzXML_pathes_FilePathStrct *mzXMLFiles;
  char **xmlPathes;
  char tmpLbl[1000];
  const char *tmpValue;
  //int tmpNum;
  int i;

  // collect mzXML filepaths from xml
  BasenameParser* baseparser = new BasenameParser(htmlFile);

  if(baseparser != NULL) {
    Array<char*>* files = baseparser->getMzXMLFiles();
    *xmlFileNum = files->length();
    if(files->length() == 0) {
      reportError("<b>error</b>",1);
      exit(1);
    }

    xmlPathes = new char*[files->size()];
    for(int k = 0; k < files->size(); k++) {
      xmlPathes[k] = strdup((*files)[k]);
    }

    delete baseparser;
    delete files;
  }
  else {
    reportError("<b>error with baseparser</b>",1);
    exit(1);
  }

  // convert into FILEPathStrct
  mzXMLFiles = (get_mzXML_pathes_FilePathStrct *) 
    calloc(*xmlFileNum, sizeof(get_mzXML_pathes_FilePathStrct));
  for ( i = 0; i < *xmlFileNum; ++i){
    mzXMLFiles[i].path = xmlPathes[i];
    if((tmpValue = findRightmostPathSeperator(xmlPathes[i])) != NULL)
      strncpy(tmpLbl, tmpValue+1, sizeof(tmpLbl));
    else
      strncpy(tmpLbl, xmlPathes[i], sizeof(tmpLbl));
    if((tmpValue = hasValidPepXMLFilenameExt(tmpLbl)) != NULL)
      tmpLbl[strlen(tmpLbl)-strlen(tmpValue)] = '\0';
    mzXMLFiles[i].expLbl = strdup(tmpLbl);
  } //   for ( i = 0; i < tmpNum; ++i)

  delete[] xmlPathes;

  return mzXMLFiles;
}


/************************************************************************
 *
 * This function frees memory allocated to "xmlFiles"
 *
 ************************************************************************/
void freeMzXMLPathes(get_mzXML_pathes_FilePathStrct *xmlFiles, 
		     int xmlFileNum)
{
  int i;
  
  for (i = 0; i < xmlFileNum; ++i) {
    free(xmlFiles[i].expLbl);
    free(xmlFiles[i].path);
  }
  free(xmlFiles);

  return;
}


/************************************************************************
 *
 * char **getPngFile(int *linkNum, char *pngFile, char *htmlFile, 
 *	             get_mzXML_pathes_FilePathStrct xmlFile,
 *		     rangeStrct mzRange, rangeStrct scanRange, 
 *	             rangeStrct peakRange, rangeStrct scoreRange, 
 *	             int mzImgGrid, int scanImgGrid, int pepImgGrid, 
 *	             int peakIndx, int pepIndx, int fnIndx,
 *                   int sectIndx, Boolean makeLinks)
 *
 *  This function generates a png file from a .mzXML file.
 *    Also returns all links to CIDs if makeLinks == True.
 *
 ************************************************************************/
  int pepProbCmp(const cidDataStrct *a, const cidDataStrct *b) {
    if (a->z > b->z) 
      return 1;
    else if (a->z < b->z) 
      return -1;
    else 
      return 0;
  }
  int cidCmp(const cidDataStrct *a, const cidDataStrct *b) {
    if (a->s[0] > b->s[0]) 
      return 1;
    else if (a->s[0] < b->s[0]) 
      return -1;
    else 
      return 0;
  }

char **getPngFile(int *linkNum, const char *pngFile, const char *htmlFile, 
		  get_mzXML_pathes_FilePathStrct &xmlFile,
		  const rangeStrct &mzRange, const rangeStrct &scanRange, 
		  const rangeStrct &peakRange, const rangeStrct &scoreRange_in, 
		  int mzImgGrid, int scanImgGrid, int pepImgGrid, 
		  int peakIndx, int pepIndx, int scoreIndx, int fnIndx,
		  int sectIndx, Boolean makeLinks)
{
  void readMsBnds(double *mzBnd, double *scanBnd, 
		  RAMPFILE *pFI, ramp_fileoffset_t *pScanIndex, int iLastScan, int iMsLevel);
  cidDataStrct *getPep3DList(int *pepNum, const char *htmlFile, const char *mzXMLFile, int scoreIndx,
			     RAMPFILE *pFI, ramp_fileoffset_t *pScanIndex);
  cidDataStrct *getCIDList(int *cidNum, RAMPFILE *pFI, ramp_fileoffset_t *pScanIndex, int iLastScan);
  double getSpectBckNoise(double *intensity, int dataSize);
  void plotPep3DImg(const char *pngFile, cidDataStrct *pepList, int pepNum,
		    cidDataStrct *cidList, int cidNum,
		    int dataNum, double *xData, double *yData, double *zData, 
		    const rangeStrct &xRange, const rangeStrct &yRange, 
		    const rangeStrct &zRange, const rangeStrct &scoreRange, int pepIndx, int scoreIndx,  
		    int xGrdSize, int yGrdSize, int pepGrdSize,
		    int sectIndx, const char *zLbl);
  void getCidCoors(int *coors, cidDataStrct &data, 
		   const rangeStrct &xRange, const rangeStrct &yRange, 
		   int xFrmGrdSize, int yFrmGrdSize, 
		   int xDataGrdSize, int yDataGrdSize);

  rangeStrct scoreRange = scoreRange_in;
  char **links=NULL;
  RAMPFILE *pFI;
  ramp_fileoffset_t *pScanIndex;
  int iLastScan;
  struct ScanHeaderStruct scanHeader;
  ramp_fileoffset_t indexOffset;
  rangeStrct mzBnd, scanBnd, peakBnd;//, peak2Bnd;
  double *mzList;
  int mzSize;
  double *scanList;
  int scanSize;
  double *intensity;
  int dataSize;
  int *scanCounts;
  RAMPREAL *pPeaks;
  int mzIndx, scanIndx, fileIndx;
  double mxValue;//, mnValue;
  double bckValue;
  cidDataStrct *pepList, *cidList;
  int pepNum, cidNum, tmpNum;
  int startIndx;
  int coors[4];
  char cgiAction[1000], tmpString[1000];
  char *tmpValue;
  char tmpMsg[500];

  int i, j;

  if(rampValidFileType(htmlFile) != NULL)
    fileIndx = 1;
  else
    fileIndx = 0;


  /*
   * initialize file reading
   */
  if((pFI = rampOpenFile (xmlFile.path)) == NULL) {
    sprintf (tmpMsg,"<b>Error:</b> Could not open input file %s.", xmlFile.path);
    reportError(tmpMsg,0);
    return NULL;
  }

  // Read the scan index into a vector, get LastScan
  indexOffset = getIndexOffset(pFI);
  pScanIndex = readIndex(pFI, indexOffset, &iLastScan);

  /*
   * get mzBnd, etc
   */
  readMsBnds(mzBnd.bnd, scanBnd.bnd, pFI, pScanIndex, iLastScan, 1);
  if(mzBnd.bnd[0] >= mzBnd.bnd[1] || scanBnd.bnd[0] >= scanBnd.bnd[1]) {
    sprintf(tmpMsg,"Empty spectrum (no MS1 data?) in %s.", xmlFile.path);
    reportError(tmpMsg,0);

    prntMsg(" Attempting to use MS2 data to establish plot range...");
    readMsBnds(mzBnd.bnd, scanBnd.bnd, pFI, pScanIndex, iLastScan, 2);
    if(mzBnd.bnd[0] > mzBnd.bnd[1] || scanBnd.bnd[0] > scanBnd.bnd[1]) {

      sprintf(tmpMsg," ...failed. Empty spectra (no MS2 data?) in %s.", xmlFile.path);
      reportError(tmpMsg,0);
      rampCloseFile(pFI);
      free(pScanIndex);
      return NULL;

    } else {
      sprintf(tmpMsg," ...success!");

      if ( peakIndx == 0 ) {
	strcat(tmpMsg, " (using absolute intensity range)");
	printf("<SCRIPT LANGUAGE=\"JavaScript\" TYPE=\"text/javascript\">\n");
	printf("change_selected('peakRange')</SCRIPT>\n");
	peakIndx = 1;
      }

      if ( pepIndx == 0 ) {
	strcat(tmpMsg, " (enabling CID display)");
	printf("<SCRIPT LANGUAGE=\"JavaScript\" TYPE=\"text/javascript\">\n");
	printf("change_selected('pepDisplay')</SCRIPT>\n");
	pepIndx = 1;
      }

      prntMsg(tmpMsg);
    }
  }

  // get mzSize
  if(mzRange.bnd[0] > mzBnd.bnd[0])
    mzBnd.bnd[0] = mzRange.bnd[0];
  if(mzRange.bnd[1] >= mzBnd.bnd[0] && mzRange.bnd[1] < mzBnd.bnd[1])
    mzBnd.bnd[1] = mzRange.bnd[1];
  if(mzRange.grd <= 0.)
    mzBnd.grd = 0.5;
  else
    mzBnd.grd = mzRange.grd;
  mzBnd.bnd[0] = ((int)(mzBnd.bnd[0]/mzBnd.grd+0.5))*mzBnd.grd;
  mzBnd.bnd[1] = ((int)(mzBnd.bnd[1]/mzBnd.grd+0.5))*mzBnd.grd;
  mzSize = (int)(mzBnd.bnd[1]/mzBnd.grd+0.5) 
    - (int)(mzBnd.bnd[0]/mzBnd.grd+0.5) + 1;
  if(mzSize < 1) {
    sprintf(tmpMsg,"Invalid M/Z range for %s.", xmlFile.expLbl);
    reportError(tmpMsg,0);
    rampCloseFile(pFI);
    free(pScanIndex);
    return NULL;
  }

  // get scanSize
  if(scanRange.bnd[0] > scanBnd.bnd[0])
    scanBnd.bnd[0] = scanRange.bnd[0];
  if(scanRange.bnd[1] >= scanBnd.bnd[0] && scanRange.bnd[1] < scanBnd.bnd[1])
    scanBnd.bnd[1] = scanRange.bnd[1];
  if(scanRange.grd <= 0.)
    scanBnd.grd = 0.5;
  else
    scanBnd.grd = scanRange.grd;
  scanBnd.bnd[0] = ((int)(scanBnd.bnd[0]/scanBnd.grd+0.5))*scanBnd.grd;
  scanBnd.bnd[1] = ((int)(scanBnd.bnd[1]/scanBnd.grd+0.5))*scanBnd.grd;
  scanSize = (int)(scanBnd.bnd[1]/scanBnd.grd+0.5) 
    - (int)(scanBnd.bnd[0]/scanBnd.grd+0.5) + 1;
  if(scanSize < 1) {
    sprintf(tmpMsg,"Invalid elution range for %s.", xmlFile.expLbl);
    reportError(tmpMsg,0);
    rampCloseFile(pFI);
    free(pScanIndex);
    return NULL;
  }    

  // mzList, scanList
  dataSize = mzSize*scanSize;
  mzList = (double *) calloc(dataSize, sizeof(double));
  scanList = (double *) calloc(dataSize, sizeof(double));
  for (i = 0; i < scanSize; ++i) {
    for (j = 0; j < mzSize; ++j){
      mzList[i*mzSize+j] = mzBnd.bnd[0] + j*mzBnd.grd;
      scanList[i*mzSize+j] = scanBnd.bnd[0] + i*scanBnd.grd;
    }
  }


  /*
   * get intensity
   */
  intensity = (double *) calloc(dataSize, sizeof(double));
  scanCounts = (int *) calloc(scanSize, sizeof(int));


  for (i = 1; i <= iLastScan; ++i) {
    readHeader (pFI, pScanIndex[i], &scanHeader);

    if(scanHeader.msLevel != 1 || strcasecmp(scanHeader.scanType, "Zoom")==0)
      continue;

    scanIndx = (int)((scanHeader.retentionTime/60.-scanBnd.bnd[0])/scanBnd.grd + 0.5);
    if(scanIndx < 0 || scanIndx >= scanSize)
      continue;
    ++scanCounts[scanIndx];

    pPeaks = readPeaks (pFI, pScanIndex[i]);
    for (j = 0; j < scanHeader.peaksCount; ++j) {
      mzIndx = (int)((pPeaks[2*j]-mzBnd.bnd[0])/mzBnd.grd + 0.5);
      if(mzIndx >= 0 && mzIndx < mzSize)
	intensity[scanIndx*mzSize+mzIndx] += pPeaks[2*j+1];
    }  
    free(pPeaks);
  }
  for (i = 0; i < scanSize; ++i) {
    for (j = 0; j < mzSize; ++j) {
      if(scanCounts[i] > 0)
	intensity[i*mzSize+j] /= scanCounts[i];
      else
	intensity[i*mzSize+j] = 0.;
    }
  }
  free(scanCounts);


  /*
   * get image intensity
   */
  // get bckValue
  if(peakIndx == 1)
    bckValue = 1.;
  else
    bckValue = getSpectBckNoise(intensity, dataSize);


  // get peakRange
  if(peakRange.bnd[0] < 0. 
     || (peakRange.bnd[1] > 0. && peakRange.bnd[0] >= peakRange.bnd[1]))
    peakBnd.bnd[0] = bckValue;
  else
    peakBnd.bnd[0] = peakRange.bnd[0]*bckValue;
  if(peakRange.bnd[1] < 0. || peakRange.bnd[0] >= peakRange.bnd[1]) {
    mxValue = intensity[0];
    for (i = 0; i < dataSize; ++i) {
      mxValue = mxValue > intensity[i] ? mxValue : intensity[i];
    }
    peakBnd.bnd[1] = mxValue;
  }
  else
    peakBnd.bnd[1] = peakRange.bnd[1]*bckValue;
  if(fnIndx == 1) {
    if (peakBnd.bnd[0] <= 0.)
      peakBnd.bnd[0] = 0.;
    else
      peakBnd.bnd[0] = log10(peakBnd.bnd[0]);
    if (peakBnd.bnd[1] <= 0.)
      peakBnd.bnd[1] = 0.;
    else
      peakBnd.bnd[1] = log10(peakBnd.bnd[1]);
  }
  if(peakBnd.bnd[1] <= peakBnd.bnd[0]) {
    sprintf(tmpMsg,"Invalid intensity range for %s.", xmlFile.expLbl);
    reportError(tmpMsg,0);
    rampCloseFile(pFI);
    free(pScanIndex);
    free(mzList);
    free(scanList);
    free(intensity);
    return NULL;
  }

  // get image intensity
  if(fnIndx == 1) {
    for (i = 0; i < dataSize; ++i) {
      if (intensity[i] < 1.)
	intensity[i] = 0.;
      else
	intensity[i] = log10(intensity[i]);
    }
  }

  /*
   * get peptide list
   */
  if(fileIndx != 1 && pepIndx > 1) {
    pepList = getPep3DList(&pepNum, htmlFile, xmlFile.path, scoreIndx, pFI, pScanIndex);
    // scoreRange.bnd[1]
    if(scoreRange.bnd[1] <= scoreRange.bnd[0]){
      for (i = 0; i < pepNum; ++i){
	scoreRange.bnd[1] = scoreRange.bnd[1] > pepList[i].z ? 
	  scoreRange.bnd[1] : pepList[i].z; 
      }
    } // if(scoreRange.bnd[1] == -1.){

    // filter
    tmpNum = 0;
    for (i = 0; i < pepNum; ++i){
      if(pepList[i].x >= scanBnd.bnd[0] && pepList[i].x <= scanBnd.bnd[1]
	 && pepList[i].y >= mzBnd.bnd[0] && pepList[i].y <= mzBnd.bnd[1]
	 && pepList[i].z >= scoreRange.bnd[0] && pepList[i].z <= scoreRange.bnd[1])
	pepList[tmpNum++] = pepList[i];
    }
    pepNum = tmpNum;
    if(pepList != NULL && pepNum < 1){
      free(pepList);
      pepList = NULL;
    }
  }
  else {
    pepList = NULL;
    pepNum = 0;
  }

  if(pepIndx == 1 || pepIndx == 3) {
    cidList = getCIDList(&cidNum, pFI, pScanIndex, iLastScan);
    // filter
    tmpNum = 0;
    for (i = 0; i < cidNum; ++i){
      if(cidList[i].x >= scanBnd.bnd[0] && cidList[i].x <= scanBnd.bnd[1]
	 && cidList[i].y >= mzBnd.bnd[0] && cidList[i].y <= mzBnd.bnd[1])
	cidList[tmpNum++] = cidList[i];
    }
    cidNum = tmpNum;
    if(cidList != NULL && cidNum < 1){
      free(cidList);
      cidList = NULL;
    }
  }
  else {
    cidList = NULL;
    cidNum = 0;
  }
  rampCloseFile(pFI);
  free(pScanIndex);

  if(iOutputType == HTML_OUTPUT) {
    if(cidNum > 0 && pepNum > 0) 
      sprintf(tmpString, "peptide/CID (%d/%d = %f)", pepNum, cidNum, pepNum/((double)cidNum));
    else if(pepNum > 0)
      sprintf(tmpString, "peptide (%d)", pepNum);
    else if(cidNum > 0)
      sprintf(tmpString, "CID (%d)", cidNum);
    else
      sprintf(tmpString, "&nbsp;");

  } else if(iOutputType == XML_OUTPUT) {

    if(cidNum > 0 && pepNum > 0) 
      sprintf(tmpString, "<NUM_PEPTIDES>%d</NUM_PEPTIDES>\n<NUM_CIDS>%d</NUM_CIDS>\n", pepNum, cidNum);
    else if(pepNum > 0)
      sprintf(tmpString, "<NUM_PEPTIDES>%d</NUM_PEPTIDES>\n", pepNum);
    else if(cidNum > 0)
      sprintf(tmpString, "<NUM_CIDS>%d</NUM_CIDS>\n", cidNum);
    else
      sprintf(tmpString, " ");
  }

  //remove pepList from cidList
  if(pepList != NULL && cidList != NULL) {
    qsort(pepList, pepNum, sizeof(cidDataStrct), 
	  (int(*)(const void*, const void*))cidCmp);
    qsort(cidList, cidNum, sizeof(cidDataStrct), 
	  (int(*)(const void*, const void*))cidCmp);

    tmpNum = 0;
    startIndx = 0;
    for (i = 0; i < pepNum; ++i) {
      for (j = startIndx; j < cidNum; ++j) {
	if(cidList[j].s[0] < pepList[i].s[0]) {
	  cidList[tmpNum++] = cidList[j];
	}
	else if(cidList[j].s[0] == pepList[i].s[0]){
	  startIndx = j + 1;
	  while(startIndx < cidNum 
		&& cidList[startIndx].s[0] <= pepList[i].s[1])
	    ++startIndx;
	  break;
	}
	else if(cidList[j].s[0] > pepList[i].s[1]) {
	  startIndx = j;
	  break;
	}
      }
    }
    for (j = startIndx; j < cidNum; ++j) {
      cidList[tmpNum++] = cidList[j];
    }
    cidNum = tmpNum;
    if(cidNum < 1){
      free(cidList);
      cidList = NULL;
    }
  } // if(pepList != NULL && cidList != NULL) {

  // sort by prob
  if(pepList != NULL)
    qsort(pepList, pepNum, sizeof(cidDataStrct), 
	  (int(*)(const void*, const void*))pepProbCmp);

  /*
   * generate density plot
   */
  if(fnIndx == 1)
    plotPep3DImg(pngFile, pepList, pepNum, cidList, cidNum, 
		 dataSize, scanList, mzList, intensity,
		 scanBnd, mzBnd, peakBnd, scoreRange, pepIndx, scoreIndx,
		 scanImgGrid, mzImgGrid, pepImgGrid,
		 sectIndx, "intensity (log10)");
  else
    plotPep3DImg(pngFile, pepList, pepNum, cidList, cidNum, 
		 dataSize, scanList, mzList, intensity,
		 scanBnd, mzBnd, peakBnd, scoreRange, pepIndx, scoreIndx, 
		 scanImgGrid, mzImgGrid, pepImgGrid,
		 sectIndx, "intensity");

  /*
   * generate cgi links
   */
  *linkNum = pepNum + cidNum + 1;
  links = (char **) calloc(max(*linkNum,1), sizeof(char *));
  links[0] = (char *) calloc(strlen(tmpString)+1, sizeof(char));
  strcpy(links[0], tmpString);

  if(*linkNum > 1) {
    if((tmpValue = getenv("SCRIPT_NAME")) != NULL) {
      strcpy(cgiAction, tmpValue);
      if((tmpValue = findRightmostPathSeperator(cgiAction)) != NULL) {
	strcpy(tmpValue+1, "plot-msms-js.cgi");
      }
    }
    else
      cgiAction[0] = '\0';
    links[0] = strdup(tmpString);
  } // if(*linkNum > 1) {

  *linkNum = 1;
  if (makeLinks) {
    if(pepList != NULL) {
      for (i = pepNum-1; i >= 0; --i) {
	getCidCoors(coors, pepList[i], scanBnd, mzBnd, 
		    scanImgGrid, mzImgGrid, pepImgGrid, pepImgGrid);
	if(coors[0] < 0)
	  continue;

        sprintf(tmpString, "<area target=\"win2\" shape=rect coords=\"%d, %d, %d, %d\" href=\"%s?File=%s&amp;ScanStart=%d&amp;ScanEnd=%d&amp;PrecMz=%lf&amp;Charge=%d&amp;origFile=%s\">", 
		coors[0], coors[1], coors[2], coors[3], cgiAction, xmlFile.path, 
		pepList[i].s[0], pepList[i].s[0], pepList[i].y, pepList[i].s[2], htmlFile);
	links[*linkNum] = (char *) calloc(strlen(tmpString)+1, sizeof(char));
	strcpy(links[*linkNum], tmpString);
	++(*linkNum);
      }
    }

    if(cidList != NULL) {
      for (i = 0; i < cidNum; ++i) {
	getCidCoors(coors, cidList[i], scanBnd, mzBnd, 
		    scanImgGrid, mzImgGrid, pepImgGrid, pepImgGrid);
	if(coors[0] < 0)
	  continue;
	if(fileIndx != 0)
	  sprintf(tmpString, "<area target=\"win2\" shape=rect coords=\"%d, %d, %d, %d\" href=\"%s?File=%s&amp;ScanStart=%d&amp;ScanEnd=%d&amp;PrecMz=%lf&amp;Charge=%d\">",
		  coors[0], coors[1], coors[2], coors[3], cgiAction, xmlFile.path, 
		  cidList[i].s[0], cidList[i].s[0], cidList[i].y, cidList[i].s[2]);
	else
	  sprintf(tmpString, "<area target=\"win2\" shape=rect coords=\"%d, %d, %d, %d\" href=\"%s?File=%s&amp;ScanStart=%d&amp;ScanEnd=%d&amp;PrecMz=%lf&amp;Charge=%d&amp;origFile=%s\">", 
		  coors[0], coors[1], coors[2], coors[3], cgiAction, xmlFile.path, 
		  cidList[i].s[0], cidList[i].s[0], cidList[i].y, cidList[i].s[2], htmlFile);
	links[*linkNum] = (char *) calloc(strlen(tmpString)+1, sizeof(char));
	strcpy(links[*linkNum], tmpString);
	++(*linkNum);
      }
    }
  } // if (makeLinks)

  free(mzList);
  free(scanList);
  free(intensity);
  if(pepList != NULL)
    free(pepList);
  if(cidList != NULL)
    free(cidList);

  return links;
}


/************************************************************************
 *
 * This function finds the mzBnd and scanBnd of the experiment.
 *
 ************************************************************************/
void readMsBnds(double *mzBnd, double *scanBnd, 
		RAMPFILE *pFI, ramp_fileoffset_t *pScanIndex, int iLastScan, int iMsLevel)
{
  int  i;
  struct ScanHeaderStruct scanHeader;
  int indx = 0;
  
  /* Initialize the boundries */
  mzBnd[0] = 0.0;
  mzBnd[1] = 0.0;
  scanBnd[0] = 0.0;
  scanBnd[1] = 0.0;

  for (i = 1; i <= iLastScan; ++i) {
    readHeader(pFI, pScanIndex[i], &scanHeader);

    if(scanHeader.msLevel == iMsLevel 
       && scanHeader.peaksCount > 0){
      if(indx == 0) {
	mzBnd[0] = scanHeader.lowMZ;
	mzBnd[1] = scanHeader.highMZ;
	scanBnd[0] = scanHeader.retentionTime/60.;
	indx = 1;
      }
      else {
	mzBnd[0] = mzBnd[0] < scanHeader.lowMZ ?
	  mzBnd[0] : scanHeader.lowMZ;	  
	mzBnd[1] = mzBnd[1] > scanHeader.highMZ ?
	  mzBnd[1] : scanHeader.highMZ;	  
	scanBnd[1] = scanHeader.retentionTime/60.;
      }
    }
  }

  return;
}


/************************************************************************
 *
 * This function gets the background noise level of the whole spectrum,
 * which is used for normalizing spectrum.
 *
 ************************************************************************/
double getSpectBckNoise(double *intensity, int dataSize)
{

  double logGrid = 0.01;
  double mxValue, mnValue, cutoff, bckValue;
  int *denCounts;
  int size, totCount;
  int indx;
  spectStrct spect;
  double ave;

  int i;

  if (dataSize < 1)
    return 0.0;

  // get mxValue and mnValue
  mnValue = intensity[0];
  mxValue = intensity[0];

  for (i = 0; i < dataSize; ++i) {
    mnValue = mnValue < intensity[i] ? mnValue : intensity[i];
    mxValue = mxValue > intensity[i] ? mxValue : intensity[i];
  }

  if (mxValue < 1.)
    return 0.0;

  mxValue = ((int)(log10(mxValue)/logGrid+1.))*logGrid;
  if(mnValue < 1.)
    mnValue = 0.;
  else {
    mnValue = ((int)(log10(mnValue)/logGrid))*logGrid;
    mnValue = mnValue > 0. ? mnValue : 0.;
  }
  cutoff = pow(10., mnValue-0.5*logGrid);

  // get denCounts
  size = ((int)((mxValue-mnValue)/logGrid+0.5)) + 1;
  denCounts = (int *) calloc(size, sizeof(int));
  for (i = 0; i < dataSize; ++i) {
    if(intensity[i] > cutoff) {
      indx = (int)((log10(intensity[i])-mnValue)/logGrid+0.5);
      ++denCounts[indx];
    }
  }
  
  // get spectStrct
  spect.size = size;
  spect.xval = (double *) calloc(size, sizeof(double));
  spect.yval = (double *) calloc(size, sizeof(double));
  ave = 0.;
  totCount = 0;

  for(i = 0; i < size; ++i) {
    spect.xval[i] = mnValue + i*logGrid;
    spect.yval[i] = (double) denCounts[i];
    ave += spect.xval[i]*spect.yval[i];
    totCount += denCounts[i];
  }
  ave /= totCount;
  smoothSpect(&spect, 50, 4);

  // get bckValue
  indx = 0;
  while(indx < spect.size && spect.xval[indx] < ave)
    ++indx;
  indx = spectPeak(spect, indx, -1);
  if(indx > 0)
    bckValue = pow(10., spect.xval[indx]);
  else
    bckValue = pow(10., ave);

  free(denCounts);
  return bckValue;
}


/************************************************************************
 *
 * This function smooths rough spectrum.
 *
 ************************************************************************/
void smoothSpect(spectStrct * spectrum, int range, int repeats)
{
  double smoothDataPt(const spectStrct &spectrum, int dtIndx, 
		      int range, double threshold);

  int size = spectrum->size;
  double *temp;
  double threshold;
  int i, j;

  temp = (double *) calloc(size, sizeof(double));
  threshold = spectrum->yval[0];

  for (i = 0; i < size; ++i) {
    if(spectrum->yval[i] < threshold) threshold = spectrum->yval[i];
  }

  for (j = 0; j < repeats; ++j) {
    for (i = 0; i < size; ++i) {
      temp[i] = smoothDataPt(*spectrum, i, range, threshold);
    }
    spectrum->set_yvals(temp);
  }

  free(temp);

  return;
}


/************************************************************************
 *
 * This function smooths rough spectrum at a specific point.
 *
 ************************************************************************/
double smoothDataPt(const spectStrct &spectrum, int dtIndx, 
		    int range, double threshold)
{

  int order = 4;
  int lower, upper;
  double value;


  if (dtIndx < range) {
    lower = 0;
    upper = 2*range;
  }
  else if (dtIndx > spectrum.size-range-1) {
    lower = spectrum.size-2*range-1;
    upper = spectrum.size-1;
  }
  else {
    lower = dtIndx - range;
    upper = dtIndx + range;
  }

  // get filter value
  order = order < upper-lower-1 ? order : upper-lower-1;
  if(order < 1)
    return spectrum.yval[dtIndx];
  else
    value = spectrum.dataFilter(dtIndx, lower, upper, order); 

  if(value > threshold) 
    return value;
  else
    return threshold;

}




/************************************************************************
 *
 * This function finds the peak at "position" in a spectrum.  
 * If "position" is a valley, then find the left peak (when 
 * "direction = -1") or the right one (when "direction = 1").
 *
 ************************************************************************/
int spectPeak(const spectStrct &spectrum, int position, int direction)
{
  int right, left;
  int i;

  // determine slope on the right hand side 
  i = 1;
  while(position+i < spectrum.size &&
	spectrum.yval[position] == spectrum.yval[position+i]){
    ++i;
  } 
  if (position+i >= spectrum.size) {
    right = 0;
  }
  else if (spectrum.yval[position] < spectrum.yval[position+i]){
    right = 1;
  }
  else
    right = -1;

  // determine slope on the left hand side 
  i = -1;
  while(position+i >= 0 &&
	spectrum.yval[position] == spectrum.yval[position+i]){
    --i;
  } 
  if (position+i < 0) {
    left = 0;
  }
  else if (spectrum.yval[position] < spectrum.yval[position+i]){
    left = 1;
  }
  else
    left = -1;

  // determine "direction"
  if((right == -1 && left == -1) || (right == 0 && left == 0)) {
    return position;
  }
  else if (right == -1 && left == 0) {
    return 0;
  }
  else if (right == 0 && left == -1) {
    return spectrum.size-1;
  }
  else if (right <= 0 && left == 1) {
    direction = -1;
  }
  else if (right == 1 && left <= 0) {
    direction = 1;
  }

  // search for peak
  while (position+direction >= 0 && position+direction < spectrum.size
	 && spectrum.yval[position] <= spectrum.yval[position+direction]) {
    position += direction;
  }

  // return index for peak
  if (position+direction < 0) 
    return 0;
  else if (position+direction >= spectrum.size) 
    return spectrum.size-1;
  else 
    return position;
}


/************************************************************************
 *
 * This function gets (m/z, time, prob) coodinates of all peptides in an experiment.
 *
 ************************************************************************/
cidDataStrct *getPep3DList(int *pepNum, const char *htmlFile, const char *mzXMLFile, int scoreIndx, 
			   RAMPFILE *pFI, ramp_fileoffset_t *pScanIndex)
{
  Pep3D_dataStrct *peptides;
  cidDataStrct *pepList;
  struct ScanHeaderStruct scanHeader;
  double mass, time;
  int count;
  int i, j;

  // collect peptide information
  const char** next_mzxml = new const char*[1];
  next_mzxml[0] = mzXMLFile;
  Pep3DParser* parser = new Pep3DParser(htmlFile, next_mzxml, 1);
 
  if(parser != NULL) {
    *pepNum = parser->getNumDataEntries();
    peptides = parser->getPep3DDataStrct();
    delete parser;
  }
  else {
    reportError("<b>error: no Pep3DParser</b>",1);
    exit(1);
  }

  //  peptides = getPep3DDataStrct(pepNum, mzXMLFile, htmlFile);
  //  if(*pepNum < 1){
  //    free(peptides);
  //    return NULL;
  //  }

  // collect pepList
  pepList = (cidDataStrct *) calloc(*pepNum, sizeof(cidDataStrct));
  for (i = 0; i < *pepNum; ++i){
    pepList[i].s[0] = peptides[i].startScan;
    pepList[i].s[1] = peptides[i].endScan;
    pepList[i].s[2] = peptides[i].charge;

    mass = 0.;
    time = 0.;
    count = 0;
    for (j = peptides[i].startScan; j <= peptides[i].endScan; ++j) {
	
      readHeader (pFI, pScanIndex[j], &scanHeader);
  
      if(scanHeader.msLevel == 2){
	mass += scanHeader.precursorMZ;
	time += scanHeader.retentionTime/60.;
	++count;
      }
    }
    if(count > 0){
      mass /= count;
      time /= count;
    }
    pepList[i].x = time;
    pepList[i].y = mass;

    if(scoreIndx == 1) // Xcorr
      pepList[i].z = peptides[i].score;
    else
      pepList[i].z = peptides[i].prob;
  }

  free(peptides);

  return pepList;
}


/************************************************************************
 *
 * This function gets (scan, m/z, time) coodinates of all cid attemps in an experiment.
 *
 ************************************************************************/
cidDataStrct *getCIDList(int *cidNum, RAMPFILE *pFI, ramp_fileoffset_t *pScanIndex, int iLastScan)
{
  cidDataStrct *cidList;
  struct ScanHeaderStruct scanHeader;
  int tmpNum;

  long i;

  *cidNum = 0;

  cidList = (cidDataStrct *) calloc(iLastScan, sizeof(cidDataStrct));
  tmpNum = 0;
  for (i = 1; i <= iLastScan; ++i){
    readHeader (pFI, pScanIndex[i], &scanHeader);
    if(scanHeader.msLevel == 2){
      cidList[tmpNum].s[0] = i;
      cidList[tmpNum].s[2] = scanHeader.precursorCharge;
      cidList[tmpNum].x = scanHeader.retentionTime/60.;
      cidList[tmpNum].y = scanHeader.precursorMZ;
      cidList[tmpNum].z = 1.;
      ++tmpNum;
    }
  }
  if(tmpNum < 1) {
    free(cidList);
    return NULL;
  }
  else {
    cidList = (cidDataStrct *) realloc(cidList, tmpNum*sizeof(cidDataStrct));
  }
  *cidNum = tmpNum;

  return cidList;
}


/************************************************************************
 *
 * This function gets the coordinates of a cidDataStrct.
 *
 ************************************************************************/
void getCidCoors(int *coors, cidDataStrct &data, 
		 const rangeStrct &xRange, const rangeStrct &yRange, 
		 int xFrmGrdSize, int yFrmGrdSize, 
		 int xDataGrdSize, int yDataGrdSize)
{
  int xOffset, yOffset; // image offset
  int xcPos, ycPos; // position of center
  
  // out of range
  if(data.x < xRange.bnd[0] || data.x > xRange.bnd[1]
     || data.y < yRange.bnd[0] || data.y > yRange.bnd[1]){
    coors[0] = -1;
    return;
  }

  // offsets
  xOffset = XUL + OFFSET + xFrmGrdSize/2; 
  yOffset = YUL + OFFSET - yFrmGrdSize/2
    + ((int)(yRange.bnd[1]/yRange.grd+0.5)-(int)(yRange.bnd[0]/yRange.grd+0.5)
       +1)*yFrmGrdSize;
  
  // coordinates
  xcPos = xOffset + ((int)((data.x-xRange.bnd[0])/xRange.grd+0.5))*xFrmGrdSize;
  ycPos = yOffset - ((int)((data.y-yRange.bnd[0])/yRange.grd+0.5))*yFrmGrdSize;
  coors[0] = xcPos-xDataGrdSize/2;
  coors[1] = ycPos-yDataGrdSize/2; 
  coors[2] = xcPos+xDataGrdSize-xDataGrdSize/2; 
  coors[3] = ycPos+yDataGrdSize-yDataGrdSize/2;
  
  return;
}


/************************************************************************
 *
 * void plotPep3DImg(const char *pngFile, cidDataStrct *pepList, int pepNum,
 *		     cidDataStrct *cidList, int cidNum,
 *		     int dataNum, double *xData, double *yData, double *zData, 
 *		     rangeStrct xRange, rangeStrct yRange, 
 *		     rangeStrct zRange, rangeStrct scoreRange, int pepIndx, 
 *		     int xGrdSize, int yGrdSize, int pepGrdSize,
 *		     int sectIndx, const char *zLbl)
 *
 * This function plots a list of (x, y, z) data in density plot.
 *
 ************************************************************************/
void plotPep3DImg(const char *pngFile, cidDataStrct *pepList, int pepNum,
		  cidDataStrct *cidList, int cidNum,
		  int dataNum, double *xData, double *yData, double *zData, 
		  const rangeStrct &xRange, const rangeStrct &yRange, 
		  const rangeStrct &zRange, const rangeStrct &scoreRange, int pepIndx, int scoreIndx, 
		  int xGrdSize, int yGrdSize, int pepGrdSize,
		  int sectIndx, const char *zLbl)
{
  void plotData(gdImagePtr im, double x, double y, double z, int flColor, 
		const rangeStrct &xRange, const rangeStrct &yRange, 
		int xFrmGrdSize, int yFrmGrdSize, 
		int xDataGrdSize, int yDataGrdSize);
  void plotDataUnfilled(gdImagePtr im, double x, double y, double z, 
			int lblColor, 
			const rangeStrct &xRange, const rangeStrct &yRange, 
			int xFrmGrdSize, int yFrmGrdSize, 
			int xDataGrdSize, int yDataGrdSize);
  void plotImageFrame(gdImagePtr im, 
		      const rangeStrct &xRange, const rangeStrct &yRange, 
		      const char *xLbl, const char *yLbl, 
		      int lblColor, int xFrmGrdSize, int yFrmGrdSize);
  void plotZTicsUp(gdImagePtr im, int xPos, int yPos, 
		   const rangeStrct &z1Range, const rangeStrct &z2Range, const char *zLbl,
		   int *color, int totColor, int sectIndx, int lblColor);
  void plotZTicsDn(gdImagePtr im, int xPos, int yPos, 
		   const rangeStrct &z1Range, const rangeStrct &z2Range, const char *zLbl,
		   int *color, int totColor, int sectIndx, int lblColor);
  void detColor(gdImagePtr im, int *color, int totColor, char type, int bnd);
  int fillColor(double z, rangeStrct z1Range, rangeStrct z2Range, 
		int totColor, int sectIndx); 

  char xLbl[] = "time (min)";
  char yLbl[] = "m/z";
 
  char tmpMsg[500];

  // image
  gdImagePtr im;
  FILE *pngout;
  int xSize, ySize;

  // color
  int totColor = 253;
  int *color; //color[0] transparent
  int lblColor, flColor, cidColor;
  int vldColor, indx;
  rangeStrct z2Range;

  int i;

  // vldColor
  if(pepIndx > 1)
    vldColor = totColor/2 + 1;
  else
    vldColor = totColor;

  // image
  xSize = XUL + 2*OFFSET + xGrdSize
    + (int)(xRange.bnd[1]/xRange.grd+0.5)*xGrdSize
    - (int)(xRange.bnd[0]/xRange.grd+0.5)*xGrdSize;
  ySize = YUL + 2*OFFSET + yGrdSize
    + (int)(yRange.bnd[1]/yRange.grd+0.5)*yGrdSize
    - (int)(yRange.bnd[0]/yRange.grd+0.5)*yGrdSize;
  im = gdImageCreate(xSize+150, ySize+150);


  // color
  color = (int *) calloc(totColor, sizeof(int));
  color[0] = gdImageColorAllocate(im, 255, 255, 255); // background color
  lblColor = gdImageColorAllocate(im, 0, 0, 0); // color for labeling     
  cidColor = gdImageColorAllocate(im, 0, 0, 255); // cid color
  detColor(im, color, vldColor, 'B', 1);
  if(pepIndx > 1) {
    for (i = vldColor; i < totColor; ++i) {
      indx = (int)(2*255-255*(i-2.)/(totColor/2-1.)+0.5);
      color[i] = gdImageColorAllocate(im, 255-indx, indx, 0);
    }
  }


  // plot frame
  plotImageFrame(im, xRange, yRange, xLbl, yLbl, lblColor, xGrdSize, yGrdSize); 
  z2Range.bnd[0] = 0.;
  z2Range.bnd[1] = -1.;
  plotZTicsUp(im, xSize+10, (YUL+ySize)/2, zRange, z2Range, zLbl, 
	      color, vldColor, sectIndx, lblColor); 

  // plot data
  for (i = 0; i < dataNum; ++i) {
    // fill color
    flColor = fillColor(zData[i], zRange, z2Range, vldColor, sectIndx);
    if(flColor < 0)
      continue;
    else
      plotData(im, xData[i], yData[i], zData[i], color[flColor],
	       xRange, yRange, xGrdSize, yGrdSize, xGrdSize, yGrdSize);
  }

  
  // plot cid list
  if((pepIndx == 1 || pepIndx == 3) && cidList != NULL) {
    for (i = 0; i < cidNum; ++i) {
      plotDataUnfilled(im, cidList[i].x, cidList[i].y, 1., 
		       cidColor, xRange, yRange, xGrdSize, yGrdSize, 
		       pepGrdSize, pepGrdSize);
    }
    
    gdImageString(im, gdFontMediumBold, (XUL+xSize)/2+totColor/2+50-4*strlen("CID"), ySize+50, (unsigned char*)("CID"), lblColor);
    gdImageRectangle(im, (XUL+xSize)/2+totColor/2+50-5, ySize+70,
		     (XUL+xSize)/2+totColor/2+50+5, ySize+80, cidColor);
  }


  // plot peptide list
  if(pepIndx > 1 && pepList != NULL) {
    for (i = 0; i < pepNum; ++i) {
      flColor = fillColor(pepList[i].z, scoreRange, z2Range, vldColor, 0);
      if(flColor > 0)
	plotDataUnfilled(im, pepList[i].x, pepList[i].y, pepList[i].z, 
			 color[vldColor+flColor-1], xRange, yRange, xGrdSize, yGrdSize, 
			 pepGrdSize, pepGrdSize);
    }

    if(scoreIndx == 1)
      plotZTicsDn(im, (XUL+xSize)/2, ySize+50, scoreRange, z2Range, "peptide Xcorr",
		  color+vldColor, totColor/2, 0, lblColor); 
    else
      plotZTicsDn(im, (XUL+xSize)/2, ySize+50, scoreRange, z2Range, "peptide probability",
		  color+vldColor, totColor/2, 0, lblColor); 
  }


  // output
  if((pngout = fopen(pngFile, "wb")) == NULL) {
    sprintf(tmpMsg,"<b>Cannot open %s for writing!</b>", pngFile);
    reportError(tmpMsg,1);
    exit(1);
  }
  // seem to need the memory based version for MSVC to avoid a crash
  int sz;
  char *img = (char *)gdImagePngPtr(im, &sz); 
  if (img) {
   size_t fwrote = fwrite(img, 1, sz, pngout);
   free(img);
  } else {
    printf("error writing image file \"%s\"! \n", pngFile);
    exit(0);
  }
  fclose(pngout);

  // delete image
  gdImageDestroy(im);

  free(color);

  return;
}


/************************************************************************
 *
 * This function plots the frame of the image.
 *
 ************************************************************************/
void plotImageFrame(gdImagePtr im, 
		    const rangeStrct &xRange, const rangeStrct &yRange, 
		    const char *xLbl, const char *yLbl, 
		    int lblColor, int xFrmGrdSize, int yFrmGrdSize)
{
  void getAxisTics(double *tics, const rangeStrct &data, int majTics);

  int majTics;
  int minTicsSize = 5;
  int majTicsSize = 10;
  int x1, y1;
  int xImSize, yImSize;
  int x2, y2;
  int start, end;
  double tics[2];
  double value;
  int pos;
  char ticsTxt[100];

  // plot frame

  // upper left
  x1 = XUL;
  y1 = YUL;

  // image size
  xImSize = ((int)(xRange.bnd[1]/xRange.grd+0.5)
	     -(int)(xRange.bnd[0]/xRange.grd+0.5)+1)*xFrmGrdSize;
  yImSize = ((int)(yRange.bnd[1]/yRange.grd+0.5)
	     -(int)(yRange.bnd[0]/yRange.grd+0.5)+1)*yFrmGrdSize;

  // lower right
  x2 = x1 + xImSize + 2*OFFSET; 
  y2 = y1 + yImSize + 2*OFFSET;

  // frame
  gdImageRectangle(im, x1, y1, x2, y2, lblColor);
  
  
  // xtics
  majTics = xImSize/50 > 1 ? xImSize/50 : 1;
  getAxisTics(tics, xRange, majTics);

  // 1st tics
  value = xRange.bnd[0];
  while(fmod(value, tics[1]) > 0.5*xRange.grd
	&& tics[1]-fmod(value, tics[1]) > 0.5*xRange.grd){
    value += xRange.grd;
  }
  pos = x1 + OFFSET + xFrmGrdSize/2 + ((int)(value/xRange.grd+0.5))*xFrmGrdSize
    - ((int)(xRange.bnd[0]/xRange.grd+0.5))*xFrmGrdSize;

  // plot xtics
  do {
    // bottom
    start = y2;
    end = start - minTicsSize;
    if(fmod(value, tics[0]) < 0.5*tics[1]
       || tics[0]-fmod(value, tics[0]) < 0.5*tics[1]){
      end = start - majTicsSize;
      sprintf(ticsTxt, "%g", value);
      gdImageString(im, gdFontMediumBold, pos-4*strlen(ticsTxt), start+5, 
		    (unsigned char*)ticsTxt, lblColor);
    }
    gdImageLine(im, pos, start, pos, end, lblColor);
    // top
    start = y1;
    end = start + minTicsSize;
    if(fmod(value, tics[0]) < 0.5*tics[1]
      || tics[0]-fmod(value, tics[0]) < 0.5*tics[1]) {
      end = start + majTicsSize;
    }
    gdImageLine(im, pos, start, pos, end, lblColor);
    value += tics[1];
    pos += ((int)(tics[1]/xRange.grd+0.5))*xFrmGrdSize;
  }
  while(value <= xRange.bnd[1]);


  // ytics
  majTics = yImSize/50 > 1 ? yImSize/50 : 1;
  getAxisTics(tics, yRange, majTics);

  // 1st tics
  value = yRange.bnd[0];
  while(fmod(value, tics[1]) > 0.5*yRange.grd
	&& tics[1]-fmod(value, tics[1]) > 0.5*yRange.grd) {
    value += yRange.grd;
  }
  pos = y2 - OFFSET - yFrmGrdSize/2 - ((int)(value/yRange.grd+0.5))*yFrmGrdSize
    + ((int)(yRange.bnd[0]/yRange.grd+0.5))*yFrmGrdSize;

  // plot ytics
  do {
    // left
    start = x1;
    end = start + minTicsSize;
    if(fmod(value, tics[0]) < 0.5*tics[1]
       || tics[0]-fmod(value, tics[0]) < 0.5*tics[1]) {
      end = start + majTicsSize;
      sprintf(ticsTxt, "%g", value);
      gdImageString(im, gdFontMediumBold, start-10*strlen(ticsTxt), pos-8, 
		    (unsigned char*)ticsTxt, lblColor);
    }
    gdImageLine(im, start, pos, end, pos, lblColor);
    // right
    start = x2;
    end = start - minTicsSize;
    if(fmod(value, tics[0]) < 0.5*tics[1]
       || tics[0]-fmod(value, tics[0]) < 0.5*tics[1]) {
      end = start - majTicsSize;
    }
    gdImageLine(im, start, pos, end, pos, lblColor);
    value += tics[1];
    pos -= ((int)(tics[1]/yRange.grd+0.5))*yFrmGrdSize;
  }
  while(value <= yRange.bnd[1]);


  // x label
  gdImageString(im, gdFontMediumBold, (x1+x2)/2-4*strlen(xLbl), y2+25, 
		(unsigned char*)xLbl, lblColor);

  // y label
  gdImageStringUp(im, gdFontMediumBold, x1-65, (y1+y2)/2+4*strlen(yLbl), 
		  (unsigned char*)yLbl, lblColor);


  return;
}


/************************************************************************
 *
 * This function determines the major and minor tics of an axis.
 *
 ************************************************************************/
void getAxisTics(double *tics, const rangeStrct &data, int majTics)
{
  int minTics = 5;
  int dataSize;
  double tmpNum, unit;
  int order;
  int ticsNum;

  dataSize = (int)(data.bnd[1]/data.grd+0.5)
    - (int)(data.bnd[0]/data.grd+0.5) + 1;
  ticsNum = (int)((dataSize-1)/majTics/minTics);
  if (ticsNum < 1) {
    tics[0] = minTics*data.grd;
    tics[1] = data.grd;
    return;
  }
  order = (int)log10((double)ticsNum);
  unit = pow(10., order);
  tmpNum = ticsNum/unit;
  if(tmpNum < 1.5)
    ticsNum = 1;
  else if(tmpNum < 3.)
    ticsNum = 2;
  else if(tmpNum < 7.5)
    ticsNum = 5;
  else
    ticsNum = 10;
  tics[1] = ticsNum*unit*data.grd;
  tics[0] = tics[1]*minTics;

  return;
}


/************************************************************************
 *
 * This function plots ztics vertically.
 *
 ************************************************************************/
void plotZTicsUp(gdImagePtr im, int xPos, int yPos, 
		 const rangeStrct &z1Range, const rangeStrct &z2Range, const char *zLbl,
		 int *color, int totColor, int sectIndx, int lblColor)
{
  void getAxisTics(double *tics, const rangeStrct &data, int majTics);

  rangeStrct zRange;
  int majTics;
  double tics[2];
  double value;
  int flColor;
  int start, end;
  char ticsTxt[100];
  int size;

  // label
  gdImageStringUp(im, gdFontMediumBold, xPos, yPos+4*strlen(zLbl), (unsigned char*)zLbl, lblColor);
  xPos += 20;

  // zRange
  if(z2Range.bnd[0] >= z2Range.bnd[1]) {
    zRange.bnd[0] = z1Range.bnd[0];
    zRange.bnd[1] = z1Range.bnd[1];
    zRange.grd = (z1Range.bnd[1]-z1Range.bnd[0])/totColor;
  }
  else {
    zRange.bnd[0] = z1Range.bnd[0];
    zRange.bnd[1] = z2Range.bnd[1];
    zRange.grd = (z2Range.bnd[1]-z1Range.bnd[0])/totColor;
  }
  zRange.grd = pow(10., floor(log10(zRange.grd)));
  while((size = (int)(zRange.bnd[1]/zRange.grd+0.5)-(int)(zRange.bnd[0]/zRange.grd+0.5)+1)
	> 2*totColor) {
    zRange.grd *= 2.;
  }
  
  // tics
  majTics = ((int)(zRange.bnd[1]/zRange.grd+0.5)
	     -(int)(zRange.bnd[0]/zRange.grd+0.5)+1)/50;
  majTics = majTics > 1 ? majTics : 1;
  getAxisTics(tics, zRange, majTics);
    
  // tics
  value = floor(zRange.bnd[0]/zRange.grd)*zRange.grd;
  if(value < zRange.bnd[0])
    value += zRange.grd;
  
  start = yPos + ((int)(zRange.bnd[1]/zRange.grd+0.5)
		  -(int)(zRange.bnd[0]/zRange.grd+0.5)+1)/2;
  while(value < zRange.bnd[1]+0.5*zRange.grd) {
    flColor = fillColor(value, z1Range, z2Range, totColor, sectIndx);
    if(flColor >= 0) {
      end = start - (int)(value/zRange.grd+0.5) 
	+ (int)(zRange.bnd[0]/zRange.grd+0.5);
      gdImageFilledRectangle(im, xPos, end-1, xPos+10, end,
			     color[flColor]); 
      if(fmod(value, tics[1]) < 0.5*zRange.grd
	 || fabs(fmod(value, tics[1])-tics[1]) < 0.5*zRange.grd) {
	gdImageLine(im, xPos+10, end, xPos+12, end, lblColor);
      }
      if(fmod(value, tics[0]) < 0.5*zRange.grd
	 || fabs(fmod(value, tics[0])-tics[0]) < 0.5*zRange.grd) {
	gdImageLine(im, xPos+10, end, xPos+15, end, lblColor);
	sprintf(ticsTxt, "%g", value);
	gdImageString(im, gdFontMediumBold, xPos+20, end-8,
		      (unsigned char*)ticsTxt, lblColor);
      }
    }
    value += zRange.grd;
  }

  return;
}

/************************************************************************
 *
 * This function plots ztics horizontally.
 *
 ************************************************************************/
void plotZTicsDn(gdImagePtr im, int xPos, int yPos, 
		 const rangeStrct &z1Range, const rangeStrct &z2Range, const char *zLbl,
		 int *color, int totColor, int sectIndx, int lblColor)
{
  void getAxisTics(double *tics, const rangeStrct &data, int majTics);

  rangeStrct zRange;
  int majTics;
  double tics[2];
  double value;
  int flColor;
  int start, end;
  char ticsTxt[100];
  int size;

  // label
  gdImageString(im, gdFontMediumBold, xPos-4*strlen(zLbl), yPos, (unsigned char*)zLbl, lblColor);
  yPos += 20;

  // zRange
  if(z2Range.bnd[0] >= z2Range.bnd[1]) {
    zRange.bnd[0] = z1Range.bnd[0];
    zRange.bnd[1] = z1Range.bnd[1];
    zRange.grd = (z1Range.bnd[1]-z1Range.bnd[0])/totColor;
  }
  else {
    zRange.bnd[0] = z1Range.bnd[0];
    zRange.bnd[1] = z2Range.bnd[1];
    zRange.grd = (z2Range.bnd[1]-z1Range.bnd[0])/totColor;
  }
  zRange.grd = pow(10., floor(log10(zRange.grd)));
  while((size = (int)(zRange.bnd[1]/zRange.grd+0.5)-(int)(zRange.bnd[0]/zRange.grd+0.5)+1)
	> 2*totColor) {
    zRange.grd *= 2.;
  }
  
  // tics
  majTics = ((int)(zRange.bnd[1]/zRange.grd+0.5)
	     -(int)(zRange.bnd[0]/zRange.grd+0.5)+1)/50;
  majTics = majTics > 1 ? majTics : 1;
  getAxisTics(tics, zRange, majTics);
    
  // tics
  value = floor(zRange.bnd[0]/zRange.grd)*zRange.grd;
  if(value < zRange.bnd[0])
    value += zRange.grd;

  start = xPos - ((int)(zRange.bnd[1]/zRange.grd+0.5)
		  -(int)(zRange.bnd[0]/zRange.grd+0.5)+1)/2;
  while(value < zRange.bnd[1]+0.5*zRange.grd) {
    flColor = fillColor(value, z1Range, z2Range, totColor, sectIndx);
    if(flColor >= 0) {
      end = start + (int)(value/zRange.grd+0.5) 
	- (int)(zRange.bnd[0]/zRange.grd+0.5);
      gdImageFilledRectangle(im, end, yPos, end+1, yPos+10,
			     color[flColor]); 
      if(fmod(value, tics[1]) < 0.5*zRange.grd
	 || fabs(fmod(value, tics[1])-tics[1]) < 0.5*zRange.grd) {
	gdImageLine(im, end, yPos+10, end, yPos+12, lblColor);
      }
      if(fmod(value, tics[0]) < 0.5*zRange.grd
	 || fabs(fmod(value, tics[0])-tics[0]) < 0.5*zRange.grd) {
	gdImageLine(im, end, yPos+10, end, yPos+15, lblColor);
	sprintf(ticsTxt, "%g", value);
	gdImageString(im, gdFontMediumBold, end-4*strlen(ticsTxt), yPos+20,
		      (unsigned char*)ticsTxt, lblColor);
      }
    }
    value += zRange.grd;
  }

  return;
}


/************************************************************************
 *
 * This function marks individual data with an unfilled rectangle of a given color.
 *
 ************************************************************************/
void plotDataUnfilled(gdImagePtr im, double x, double y, double z, 
		      int lblColor, 
		      const rangeStrct &xRange, const rangeStrct &yRange, 
		      int xFrmGrdSize, int yFrmGrdSize, 
		      int xDataGrdSize, int yDataGrdSize)
{
  int xOffset, yOffset; // image offset
  int xcPos, ycPos; // position of center
  int x1, y1;
  int x2, y2;
  
  // out of range
  if(x < xRange.bnd[0] || x > xRange.bnd[1]
     || y < yRange.bnd[0] || y > yRange.bnd[1])
    return;

  // offsets
  xOffset = XUL + OFFSET + xFrmGrdSize/2; 
  yOffset = YUL + OFFSET - yFrmGrdSize/2
    + ((int)(yRange.bnd[1]/yRange.grd+0.5)-(int)(yRange.bnd[0]/yRange.grd+0.5)
       +1)*yFrmGrdSize;
  
  // coordinates
  xcPos = xOffset + ((int)((x-xRange.bnd[0])/xRange.grd+0.5))*xFrmGrdSize;
  ycPos = yOffset - ((int)((y-yRange.bnd[0])/yRange.grd+0.5))*yFrmGrdSize;
  x1 = xcPos-xDataGrdSize/2;
  y1 = ycPos-yDataGrdSize/2; 
  x2 = xcPos+xDataGrdSize-xDataGrdSize/2; 
  y2 = ycPos+yDataGrdSize-yDataGrdSize/2;

  // data
  gdImageRectangle(im, x1, y1, x2, y2, lblColor);
  
  return;
}


/************************************************************************
 *
 * This function plots individual data with a given color.
 *
 ************************************************************************/
void plotData(gdImagePtr im, double x, double y, double z, int flColor, 
	      const rangeStrct &xRange, const rangeStrct &yRange, 
	      int xFrmGrdSize, int yFrmGrdSize, 
	      int xDataGrdSize, int yDataGrdSize)
{
  int xOffset, yOffset; // image offset
  int xcPos, ycPos; // position of center
  int x1, y1;
  int x2, y2;
  
  // out of range
  if(x < xRange.bnd[0] || x > xRange.bnd[1]
     || y < yRange.bnd[0] || y > yRange.bnd[1])
    return;

  // offsets
  xOffset = XUL + OFFSET + xFrmGrdSize/2; 
  yOffset = YUL + OFFSET - yFrmGrdSize/2
    + ((int)(yRange.bnd[1]/yRange.grd+0.5)-(int)(yRange.bnd[0]/yRange.grd+0.5)
       +1)*yFrmGrdSize;
  
  // coordinates
  xcPos = xOffset + ((int)((x-xRange.bnd[0])/xRange.grd+0.5))*xFrmGrdSize;
  ycPos = yOffset - ((int)((y-yRange.bnd[0])/yRange.grd+0.5))*yFrmGrdSize;
  x1 = xcPos-xDataGrdSize/2;
  y1 = ycPos-yDataGrdSize/2; 
  x2 = xcPos+xDataGrdSize-xDataGrdSize/2; 
  y2 = ycPos+yDataGrdSize-yDataGrdSize/2;

  // data
  gdImageFilledRectangle(im, x1, y1, x2, y2, flColor);
  
  return;
}


/************************************************************************
 *
 * This function determines the color code in a plot.
 * If 2 bands, bnd = 2.
 *
 ************************************************************************/
void detColor(gdImagePtr im, int *color, int totColor, char type, int bnd)
{
  int indx;
  int i;

  // transparent color
  //  color[0] = gdImageColorAllocate(im, 255, 255, 255); //white

  // color image
  if(toupper(type) == 'C') { 
    if(bnd == 2) { // 2 bands
      for (i = 1; i <= totColor/2; ++i) {
	indx = (int)(255*(i-1.)/(totColor/2-1.)+0.5);
	color[i] = gdImageColorAllocate(im, 255, indx, indx);
      }
      for (i = totColor/2+1; i < totColor; ++i) {
	indx = (int)(2*255-255*(i-2.)/(totColor/2-1.)+0.5);
	color[i] = gdImageColorAllocate(im, indx, indx, 255);
      }
    }
    else { // 1 bnd
      for (i = 1; i < totColor; ++i) {
	indx = (int)(255*(totColor-1.-i)/(totColor-2.)+0.5);
	color[i] = gdImageColorAllocate(im, 255, indx, indx);
      }
    }
  } // if(toupper(type) == 'C') { 
  else { // gray image
    for (i = 1; i < totColor; ++i) {
      indx = (int)(255*(totColor-1.-i)/(totColor-2.)+0.5);
      color[i] = gdImageColorAllocate(im, indx, indx, indx);
    }
  } // else { // gray image
  
  return;
}


/************************************************************************
 *
 * This function determines the fill color of a data.
 * invalid: return -1; totColor: odd; sectIndx = 1: out range transparent.
 * If only 1 band, z2Range.bnd[0] > z2Range.bnd[1].
 *
 ************************************************************************/
int fillColor(double z, rangeStrct z1Range, rangeStrct z2Range, 
	      int totColor, int sectIndx) 
{
  int flColor;

  // get zRange.grd
  if(z1Range.bnd[0] >= z1Range.bnd[1])
    return -1;
  if(z2Range.bnd[0] >= z2Range.bnd[1])
    z2Range.grd = -1.;
  else
    z2Range.grd = 1.;

  // flColor
  if(z2Range.grd > 0.) { // 2 bands valid
    z1Range.grd = (double)(z1Range.bnd[1]-z1Range.bnd[0])/(double)(totColor/2-1.);
    z2Range.grd = (double)(z2Range.bnd[1]-z2Range.bnd[0])/(double)(totColor/2-1.);
    if (z < z1Range.bnd[0]) {
      if(sectIndx == 1)
	flColor = 0;
      else
	flColor = 1;
    }
    else if(z > z2Range.bnd[1]){
      if(sectIndx == 1)
	flColor = 0;
      else
	flColor = totColor - 1;
    }
    else if(z > z1Range.bnd[1] && z < z2Range.bnd[0])
      flColor = 0;
    else if(z <= z1Range.bnd[1])
      flColor = (int)((z-z1Range.bnd[0])/z1Range.grd+1.5);
    else 
      flColor = (int)((z-z2Range.bnd[0])/z2Range.grd+1.5)+totColor/2;
  }
  else { // only 1st band valid
    z1Range.grd = (z1Range.bnd[1]-z1Range.bnd[0])/(totColor-2.);
    if (z < z1Range.bnd[0]) {
      if(sectIndx == 1)
	flColor = 0;
      else
	flColor = 1;
    }
    else if(z > z1Range.bnd[1]){
      if(sectIndx == 1)
	flColor = 0;
      else
	flColor = totColor - 1;
    }
    else {
      flColor = (int)((z-z1Range.bnd[0])/z1Range.grd+1.5);
    }
  }

  return flColor;
}
