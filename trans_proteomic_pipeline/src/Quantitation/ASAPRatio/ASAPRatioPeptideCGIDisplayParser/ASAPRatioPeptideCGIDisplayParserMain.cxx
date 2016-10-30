/*

Program       : ASAPRatio
Author        : Xiao-jun Li <xli@systemsbiology.org>                                                      
Date          : 09.17.02 

CGI program for displaying and modifying ASAPRatio peptide relative abundance.

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
 * Web-based ASAPRatio (Date: 09-17-2002)
 * Developed by: Xiao-jun Li (xli@systemsbiology.org)
 * UI enhancements by L.Mendoza (2006)
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#ifndef _MSC_VER
#include <sys/param.h>
#endif
#include <sys/stat.h> 
#include "common/constants.h"
#include "common/util.h"

#include "common/TPPVersion.h" // contains version number, name, revision

#include "Quantitation/ASAPRatio/ASAPRatio_Fns/ASAPRatio_txtFns.h"
#include "Quantitation/ASAPRatio/ASAPRatio_Fns/ASAPRatio_numFns.h"
#include "Quantitation/ASAPRatio/ASAPRatio_Fns/ASAPRatio_pepFns.h"
#include "ASAPRatioPeptideCGIDisplayParser.h"
#include "ASAPRatioPeptideUpdateParser.h"
#include "Parsers/Parser/Tag.h"
#include "common/util.h"


Array<Tag*>* generateXML(pepDataStrct data_, int index, Boolean cover, Boolean data) {
  Array<Tag*>* output = new Array<Tag*>;

  if(data_.indx == -1)
    return output; // done

  Tag* next;
  char text[100];
  const char* lcpeakNames[] = {"asapratio_lc_lightpeak", "asapratio_lc_heavypeak"};

  if(cover) {
    if(data)
      next = new Tag("asapratio_result", True, False);
    else
      next = new Tag("asapratio_result", True, True);
    // fill it up
    if(index) {
      sprintf(text, "%d", index);
      next->setAttributeValue("index", text);
    }
    if(data_.pepRatio[0] == -2) {
      next->setAttributeValue("mean", "-1");
      sprintf(text, "%0.2f", data_.pepRatio[1]);
      next->setAttributeValue("error", text);
      next->setAttributeValue("heavy2light_mean", "-1");
      next->setAttributeValue("heavy2light_error", "-1");
    }
    else if(data_.pepRatio[0] == -1) {
      next->setAttributeValue("mean", "999.");
      sprintf(text, "%0.2f", data_.pepRatio[1]);
      next->setAttributeValue("error", text);
      next->setAttributeValue("heavy2light_mean", "0.00");
      next->setAttributeValue("heavy2light_error", "0.00");
    }
    else {
      sprintf(text, "%0.2f", data_.pepRatio[0]);
      next->setAttributeValue("mean", text);
      sprintf(text, "%0.2f", data_.pepRatio[1]);
      next->setAttributeValue("error", text);
      sprintf(text, "%0.2f", data_.pepH2LRatio[0]);
      next->setAttributeValue("heavy2light_mean", text);
      sprintf(text, "%0.2f", data_.pepH2LRatio[1]);
      next->setAttributeValue("heavy2light_error", text);
    }

    //next->write(cout);
    output->insertAtEnd(next);

    // now the light mass
    //    sprintf(text, "%f", data_.msLight);
    //    next->setAttributeValue("light_mass", text);

  }
  if(data) {
    next = new Tag("asapratio_peptide_data", True, False);
    if(index) {
      sprintf(text, "%d", index);
      next->setAttributeValue("index", text);
    }
    sprintf(text, "%d", data_.indx);
    next->setAttributeValue("status", text);
    sprintf(text, "%d", data_.cidIndx);
    next->setAttributeValue("cidIndex", text);
    sprintf(text, "%0.4f", data_.msLight);
    next->setAttributeValue("light_mass", text);
    sprintf(text, "%0.4f", data_.msHeavy);
    next->setAttributeValue("heavy_mass", text);
    sprintf(text, "%d", data_.areaFlag);
    next->setAttributeValue("area_flag", text);
    output->insertAtEnd(next);
    for(int k = 0; k < _ASAPRATIO_MXQ_; k++) {
      next = new Tag("asapratio_contribution", True, False);
      sprintf(text, "%0.4f", data_.pkRatio[k]);
      next->setAttributeValue("ratio", text);
      sprintf(text, "%0.4f", data_.pkError[k]);
      next->setAttributeValue("error", text);
      sprintf(text, "%d", k+1);
      next->setAttributeValue("charge", text);
      sprintf(text, "%d", data_.pkCount[k]);
      next->setAttributeValue("use", text);
      output->insertAtEnd(next);
      for(int j = 0; j < 2; j++) {
	next = new Tag(lcpeakNames[j], True, True);
	sprintf(text, "%d", data_.peaks[k][j].indx);
	next->setAttributeValue("status", text);
	sprintf(text, "%d", data_.peaks[k][j].valley[0]);
	next->setAttributeValue("left_valley", text);
	sprintf(text, "%d", data_.peaks[k][j].valley[1]);
	next->setAttributeValue("right_valley", text);
	sprintf(text, "%0.2e", data_.peaks[k][j].bckgrnd);
	next->setAttributeValue("background", text);
	sprintf(text, "%0.2e", data_.peaks[k][j].area[0]);
	next->setAttributeValue("area", text);
	sprintf(text, "%0.2e", data_.peaks[k][j].area[1]);
	next->setAttributeValue("area_error", text);
	sprintf(text, "%0.4f", data_.peaks[k][j].time[0]);
	next->setAttributeValue("time", text);
	sprintf(text, "%0.4f", data_.peaks[k][j].time[1]);
	next->setAttributeValue("time_width", text);
	sprintf(text, "%d", data_.peaks[k][j].peak);
	next->setAttributeValue("is_heavy", text);
	output->insertAtEnd(next);
      } // light/heavy
      next = new Tag("asapratio_contribution", False, True);
      output->insertAtEnd(next);
    } // next precursor ion charge

    next = new Tag("asapratio_peptide_data", False, True);
    output->insertAtEnd(next);

    if(cover) {
      next = new Tag("asapratio_result", False, True);
      output->insertAtEnd(next);
    } // if also cover
  } // if data
  return output;
}


/*
 * This function gets a pepDataStrct from a queryString.
 */
pepDataStrct *getPepDataStrctFromQueryString(char *queryString)
{
  pepDataStrct *peptide;
  char *tmpValue;
  char tmpField[100];
  int i, j, k;

  peptide = (pepDataStrct *) calloc(1, sizeof(pepDataStrct));

  // indx
  if ((tmpValue = getHtmlFieldValue("peptide_indx", queryString)) != NULL) {
    sscanf(tmpValue, "%d", &(peptide->indx));
    free(tmpValue);
  }
  else {
    free(peptide);
    return NULL;
  }

  // scan
  if ((tmpValue = getHtmlFieldValue("peptide_scan", queryString)) != NULL) {
    sscanf(tmpValue, "%ld", &(peptide->scan));
    free(tmpValue);
  }
  else {
    free(peptide);
    return NULL;
  }

  // chrg
  if ((tmpValue = getHtmlFieldValue("peptide_chrg", queryString)) != NULL) {
    sscanf(tmpValue, "%d", &(peptide->chrg));
    free(tmpValue);
  }
  else {
    free(peptide);
    return NULL;
  }

  // cidIndx
  if ((tmpValue = getHtmlFieldValue("peptide_cidIndx", queryString)) 
      != NULL) {
    sscanf(tmpValue, "%d", &(peptide->cidIndx));
    free(tmpValue);
  }
  else {
    free(peptide);
    return NULL;
  }

  // msLight
  if ((tmpValue = getHtmlFieldValue("peptide_msLight", queryString)) 
      != NULL) {
    sscanf(tmpValue, "%lf", &(peptide->msLight));
    free(tmpValue);
  }
  else {
    free(peptide);
    return NULL;
  }

  // msHeavy
  if ((tmpValue = getHtmlFieldValue("peptide_msHeavy", queryString)) 
      != NULL) {
    sscanf(tmpValue, "%lf", &(peptide->msHeavy));
    free(tmpValue);
  }
  else {
    free(peptide);
    return NULL;
  }

  // eltn
  if ((tmpValue = getHtmlFieldValue("peptide_eltn", queryString)) != NULL) {
    sscanf(tmpValue, "%d", &(peptide->eltn));
    free(tmpValue);
  }
  else {
    free(peptide);
    return NULL;
  }

  // areaFlag
  if ((tmpValue = getHtmlFieldValue("peptide_areaFlag", queryString)) 
      != NULL) {
    if(strcmp(tmpValue, "raw") == 0) 
      peptide->areaFlag = 1;
    else if(strcmp(tmpValue, "fitting") == 0) 
      peptide->areaFlag = 2;
    else
      peptide->areaFlag = 0;
    free(tmpValue);
  }
  else {
    free(peptide);
    return NULL;
  }

  // peaks
  for (i = 0; i < _ASAPRATIO_MXQ_; ++i) {
    for (j = 0; j < 2; ++j) {
      // indx
      sprintf(tmpField, "peptide_peaks_%d_%d_indx", i, j);
      if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) != NULL) {
	sscanf(tmpValue, "%d", &(peptide->peaks[i][j].indx));
	free(tmpValue);
      }
      else {
	free(peptide);
	return NULL;
      }
      // peak
      sprintf(tmpField, "peptide_peaks_%d_%d_peak", i, j);
      if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) != NULL) {
	sscanf(tmpValue, "%d", &(peptide->peaks[i][j].peak));
	free(tmpValue);
      }
      else {
	free(peptide);
	return NULL;
      }
      // valley
      for(k = 0; k < 2; ++k) {
	sprintf(tmpField, "peptide_peaks_%d_%d_valley_%d", i, j, k);
	if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) != NULL) {
	  sscanf(tmpValue, "%d", &(peptide->peaks[i][j].valley[k]));
	  free(tmpValue);
	}
	else {
	  free(peptide);
	  return NULL;
	}
      }
      // bckgrnd
      sprintf(tmpField, "peptide_peaks_%d_%d_bckgrnd", i, j);
      if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) != NULL) {
	sscanf(tmpValue, "%lf", &(peptide->peaks[i][j].bckgrnd));
	free(tmpValue);
      }
      else {
	free(peptide);
	return NULL;
      }
      // area
      for(k = 0; k < 2; ++k) {
	sprintf(tmpField, "peptide_peaks_%d_%d_area_%d", i, j, k);
	if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) != NULL) {
	  sscanf(tmpValue, "%lf", &(peptide->peaks[i][j].area[k]));
	  free(tmpValue);
	}
	else {
	  free(peptide);
	  return NULL;
	}
      }
      // time
      for(k = 0; k < 2; ++k) {
	sprintf(tmpField, "peptide_peaks_%d_%d_time_%d", i, j, k);
	if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) != NULL) {
	  sscanf(tmpValue, "%lf", &(peptide->peaks[i][j].time[k]));
	  free(tmpValue);
	}
	else {
	  free(peptide);
	  return NULL;
	}
      }
    }
  } //   for (i = 0; i < _ASAPRATIO_MXQ_; ++i) {

  // pkRatio
  for (i = 0; i < _ASAPRATIO_MXQ_; ++i) {
    sprintf(tmpField, "peptide_pkRatio_%d", i);
    if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) != NULL) {
      sscanf(tmpValue, "%lf", &(peptide->pkRatio[i]));
      free(tmpValue);
    }
    else {
      free(peptide);
      return NULL;
    }
  }

  // pkError
  for (i = 0; i < _ASAPRATIO_MXQ_; ++i) {
    sprintf(tmpField, "peptide_pkError_%d", i);
    if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) != NULL) {
      sscanf(tmpValue, "%lf", &(peptide->pkError[i]));
      free(tmpValue);
    }
    else {
      free(peptide);
      return NULL;
    }
  }

  // pkCount
  for (i = 0; i < _ASAPRATIO_MXQ_; ++i) {
    sprintf(tmpField, "peptide_pkCount_%d", i);
    if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) != NULL) {
      sscanf(tmpValue, "%d", &(peptide->pkCount[i]));
      free(tmpValue);
    }
    else {
      free(peptide);
      return NULL;
    }
  }

  // pepRatio
  for (i = 0; i < 2; ++i) {
    sprintf(tmpField, "peptide_pepRatio_%d", i);
    if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) != NULL) {
      sscanf(tmpValue, "%lf", &(peptide->pepRatio[i]));
      free(tmpValue);
    }
    else {
      free(peptide);
      return NULL;
    }
  }

  // pepTime
  for (i = 0; i < 2; ++i) {
    for (j = 0; j < 2; ++j) {
      sprintf(tmpField, "peptide_pepTime_%d_%d", i, j);
      if ((tmpValue = getHtmlFieldValue(tmpField, queryString)) != NULL) {
	sscanf(tmpValue, "%lf", &(peptide->pepTime[i][j]));
	free(tmpValue);
      }
      else {
	free(peptide);
	return NULL;
      }
    }
  }
  
  // pepArea
  if ((tmpValue = getHtmlFieldValue("peptide_pepArea", queryString)) 
      != NULL) {
    sscanf(tmpValue, "%lf", &(peptide->pepArea));
    free(tmpValue);
  }
  else {
    free(peptide);
    return NULL;
  }

  return peptide;
}


/*
 * This function displays a pepDataStrct in cgi program.
 */
void displayPepDataStrctInCgi(pepDataStrct peptide, char *cgiAction, 
			      char *xmlFile, char *baseName,
			      int index, int ratioType,
			      double *accRatio, char *pngFileLink, 
			      char *pngFilePath, char* spectrumName) {
  void displayPepDataStrctInCgi(pepDataStrct peptide, char *cgiAction, 
			      char *xmlFile, char *baseName,
			      int index, int ratioType,
			      double *accRatio, char *pngFileLink, 
			      char *pngFilePath, char* spectrumName, 
				Boolean quantHighBG, Boolean zeroBG, double mzBound);
  displayPepDataStrctInCgi( peptide, cgiAction, 
			    xmlFile, baseName,
			    index,  ratioType,
			    accRatio, pngFileLink, 
			    pngFilePath,  spectrumName, 
			    False,  False, -1);
}

void displayPepDataStrctInCgi(pepDataStrct peptide, char *cgiAction, 
			      char *xmlFile, char *baseName,
			      int index, int ratioType,
			      double *accRatio, char *pngFileLink, 
			      char *pngFilePath, char* spectrumName, 
			      Boolean quantHighBG, Boolean zeroBG, double mzBound)
{  
void displayPepDataStrctInCgi(pepDataStrct peptide, char *cgiAction, 
			      char *xmlFile, char *baseName,
			      int index, int ratioType,
			      double *accRatio, char *pngFileLink, 
			      char *pngFilePath, char* spectrumName, 
			      Boolean quantHighBG, Boolean zeroBG, double mzBound, bool wavelet);
  displayPepDataStrctInCgi( peptide, cgiAction, 
			    xmlFile, baseName,
			    index,  ratioType,
			    accRatio, pngFileLink, 
			    pngFilePath,  spectrumName, 
			    quantHighBG,  zeroBG, mzBound, false);
  
}

void displayPepDataStrctInCgi(pepDataStrct peptide, char *cgiAction, 
			      char *xmlFile, char *baseName,
			      int index, int ratioType,
			      double *accRatio, char *pngFileLink, 
			      char *pngFilePath, char* spectrumName, 
			      Boolean quantHighBG, Boolean zeroBG, double mzBound, bool wavelet)
{
  char *mzXMLFile;
  char **ratioStrings;
  char tmpField[1000];
  double tmpRatio[2];
  time_t currTime;
  int i, j, k;
  int randn;

  /*
   * create .png files
   */

  // pngFileBase
  (void) time(&currTime);
  srand48((long) currTime);
  randn = lrand48();
  sprintf(pngFilePath+strlen(pngFilePath), "_%d", randn);
  sprintf(pngFileLink+strlen(pngFileLink), "_%d", randn);

  // plot data
  int len;
  mzXMLFile = (char *) calloc(len=strlen(baseName)+strlen(xmlFile)+10, sizeof(char));
  rampConstructInputPath(mzXMLFile,len,xmlFile,baseName); // .mzXML or .mzData
  rampValidateOrDeriveInputFilename(mzXMLFile,len,spectrumName);
 
  if(peptide.indx >= 0) { // if valid data
    getPepDataStrct(&peptide, mzXMLFile, pngFilePath, 1, quantHighBG, zeroBG, mzBound, wavelet);
  }
  free(mzXMLFile);


  /*
   * form
   */
  printf("<form method=\"POST\" action=\"%s\">\n\n", cgiAction);
  // hidden "default" submit button - gets submitted when user presses 'enter' anywhere on form
  // does not work on IE.  Too bad...
  printf("<div class=\"hideit\"><input name=\"submit\" value=\"Evaluate_Ratio\" type=\"submit\"/></div>\n");

  /*
   * display manual
   */
  printf("<table cellspacing=\"0\">\n");
  printf("<tr>\n");
  printf("<td class=\"banner_cid\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Spectrum: <font color=\"#ff8700\">%s</font>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>\n", spectrumName);
  printf("</tr>\n");
  printf("</table>\n\n");

  printf("<div class=\"formentry\">\n");
  printf("<table border=\"0\" cellpadding=\"2\" cellspacing=\"0\">\n");
  printf("<tbody>\n");

  fflush(stdout);

  // accepted ratio
  ratioStrings = ASAPRatio_ratioOutput(accRatio, ratioType);
  printf("<tr>\n<th align=\"left\">Accepted  Ratio");
  if(ratioType == 0)
    printf("(L/H)");
  else if(ratioType == 1)
    printf("(H/L)");
  //else if(ratioType == 2) 
  //  printf("");
  printf(": </th>\n<td><h2>%s +- %s (%s%%)</h2></td>", ratioStrings[0], ratioStrings[1], ratioStrings[2]);

  // spacer
  printf("<td rowspan=\"2\" align=\"center\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>\n");

  // buttons
  printf("<td rowspan=\"2\" align=\"center\" class=\"navselected\">\n");
  printf("Set Accepted Ratio to:<br/>\n");
  printf("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n");
  printf("<input name=\"submit\" value=\"Interim_Ratio\" type=\"submit\"/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n");
  printf("<input name=\"submit\" value=\"0:1\" type=\"submit\"/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n");
  printf("<input name=\"submit\" value=\"1:0\" type=\"submit\"/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n");
  printf("<input name=\"submit\" value=\"Unknown\" type=\"submit\"/>\n");
  printf("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n");
  printf("<input name=\"submit\" value=\"Next_Peptide\" type=\"submit\"/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
  printf("<input name=\"submit\" value=\"Prev_Peptide\" type=\"submit\"/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\n");
  printf("</td>\n");
  printf("</tr>\n"); 

  fflush(stdout);
  freeMtrx((void **)ratioStrings, 3);

  // interim ratio
  ratioStrings = ASAPRatio_ratioOutput(peptide.pepRatio, ratioType);
  printf("<tr>\n<th align=\"left\">Interim Ratio");
  if(ratioType == 0) 
    printf("(L/H)");
  else if(ratioType == 1) 
    printf("(H/L)");
  //else if(ratioType == 2) 
  //  printf("");
  printf(": </th>\n<td><h3><i>%s +- %s (%s%%)</i></h3></td>\n", ratioStrings[0], ratioStrings[1], ratioStrings[2]);

  fflush(stdout);
  freeMtrx((void **)ratioStrings, 3);

  // run parameters hidden div
  printf("<tr><td class=\"params\" colspan=\"4\"><a onclick=\"toggleParams()\"><u id=\"paramshead\">Show Run Parameters...</u></a></td></tr>\n");
  printf("<tr id=\"parameters1\" class=\"hideit\">");
  printf("<td class=\"params\" align=\"right\">QuantHighBG :</td>");
  if (quantHighBG)
    printf("<td class=\"params\">True</td>");
  else
    printf("<td class=\"params\">False</td>");
  printf("</tr>\n");
  printf("<tr id=\"parameters2\" class=\"hideit\">");
  printf("<td class=\"params\" align=\"right\">ZeroBG :</td>");
  if (zeroBG)
    printf("<td class=\"params\">True</td>");
  else
    printf("<td class=\"params\">False</td>");
  printf("</tr>\n");
  printf("<tr id=\"parameters3\" class=\"hideit\">");
  printf("<td class=\"params\" align=\"right\">mzBound :</td>");
  printf("<td class=\"params\">%f</td>",mzBound);
  printf("</tr>\n");

  // close table and div
  printf("</tbody>\n");
  printf("</table>\n");
  printf("</div>\n\n<br/>\n\n");
  fflush(stdout);


  /*
   * display individual peaks
   */

  // nav tabs and animation checkboxes
  printf("<table>\n");
  printf("<tr>\n");

  for (i = 0; i < _ASAPRATIO_MXQ_; ++i){
    printf("<td align=\"middle\"><input name=\"UIanim_chg\" id=\"UIanim_chg%d\" value=\"%d\" type=\"checkbox\"",i,i);
    if(peptide.pkCount[i] == 1) {
      printf(" checked=\"checked\"></td>\n");
    } else {
      printf("></td>\n");
    }
  }

  printf("<td></td>\n");
  printf("<td id=\"animate_nav\">Animate: \n");
  printf("<input id=\"UIanim_fast\" type=\"button\" onClick=\"anim_ctrl('fast');\" value=\"Fast\">\n");
  printf("<input id=\"UIanim_slow\" type=\"button\" onClick=\"anim_ctrl('slow');\" value=\"Slow\">\n");
  printf("<input id=\"UIanim_stop\" type=\"button\" onClick=\"anim_ctrl('stop');\" value=\"Stop\" disabled=\"disabled\"></td>\n");

  printf("</tr>\n\n<tr>\n");

  for (i = 0; i < _ASAPRATIO_MXQ_; ++i){
    // navselect only the charge state where CID was made
    if(i+1 == peptide.chrg) {
      printf("<td class=\"navselected\" ");
    } else {
      printf("<td class=\"nav\" ");
    }
    printf("id=\"plus%d_nav\"><a onclick=\"display('plus%d')\">&nbsp;&nbsp;&nbsp;&nbsp;",
	   i+1,i+1);

    if(peptide.peaks[i][0].indx >= 0 || peptide.peaks[i][1].indx >= 0) {
      printf("+%d",i+1);
    } else {
      printf("[ +%d ]",i+1);
    }

    // ...and add a little asterisk
    if(i+1 == peptide.chrg)
      printf("*");

    printf("&nbsp;&nbsp;&nbsp;&nbsp;</a></td>\n");
  }
  printf("<td id=\"showall_nav\" align=\"right\"><a onclick=\"displayAll()\">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<u>Display All</u></a></td>\n");

  printf("</tr>\n");
  printf("</table>\n");


  // table
  printf("\n\n<table border=\"1\" cellpadding=\"2\" bgcolor=\"#dddddd\">\n<tbody>\n");

  // Evaluate Ratio button and areaFlag select
  printf("<tr>\n<td align=\"right\">&nbsp;<input name=\"submit\" value=\"Evaluate_Ratio\" type=\"submit\"/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>\n");
  printf("<td>Area type: ");
  printf("<select size=\"1\" name=\"peptide_areaFlag\">");
  if(peptide.areaFlag == 1) {
    printf("<option selected>raw</option>");
    printf("<option>average</option>");
    printf("<option>fitting</option>");
  }
  else if(peptide.areaFlag == 2) {
    printf("<option selected>fitting</option>");
    printf("<option>average</option>");
    printf("<option>raw</option>");
  }
  else {
    printf("<option selected>average</option>");
    printf("<option>raw</option>");
    printf("<option>fitting</option>");
  }
  printf("</select></td>\n</tr>\n\n");
  fflush(stdout);

  for (i = 0; i < _ASAPRATIO_MXQ_; ++i){
    if(peptide.peaks[i][0].indx >= 0
       || peptide.peaks[i][1].indx >= 0) {
      // pngFile
      sprintf(tmpField, "%s_%d.png", pngFileLink, i+1);

      if(i+1 == peptide.chrg) {
	printf("<tr class=\"showit\" ");
      } else {
	printf("<tr class=\"hideit\" ");
      }
      printf("id=\"plus%d_tr\">\n<td><img src=\"%s\"/></td>\n\n", i+1, makeTmpPNGFileSrcRef(tmpField).c_str());

      // table
      if(peptide.pkCount[i] == 1) {
	printf("<td valign=\"top\" class=\"accepted\" id=\"plus%d\">\n", i+1);
      } else {
	printf("<td valign=\"top\" class=\"rejected\" id=\"plus%d\">\n", i+1);
      }
      printf("<table><tbody>\n<tr><th><h1>+%d</h1></th></tr>\n", i+1);

      /*
       * charge state
       */

      // ratio
      tmpRatio[0] = peptide.pkRatio[i];
      tmpRatio[1] = peptide.pkError[i];

      ratioStrings = ASAPRatio_ratioOutput(tmpRatio, ratioType);
      printf("<tr class=\"banner1\"><td>Ratio:</td>");
      printf("<td><h4>%s +- %s (%s%%)</h4></td></tr>\n", 
	     ratioStrings[0], ratioStrings[1], ratioStrings[2]);
      freeMtrx((void **)ratioStrings, 3);

      // weight
      printf("<tr><td>Weight:</td>");
      printf("<td>%g</td></tr>\n", 
	     peptide.peaks[i][0].area[0]+peptide.peaks[i][1].area[0]);

      // acceptance
      printf("<tr><td><b>Acceptance:</b></td>");
      printf("<td><input type=\"radio\" name=\"peptide_pkCount_%d\" onClick=\"highlight('plus%d','yes')\" value=\"1\"", i, i+1);

      if(peptide.pkCount[i] == 1)
	printf(" checked=\"checked\"");

      printf("/>Yes <input type=\"radio\" name=\"peptide_pkCount_%d\" onClick=\"highlight('plus%d','no')\" value=\"0\"", i, i+1);

      if(peptide.pkCount[i] != 1)
	printf(" checked=\"checked\"");

      printf("/>No</td>");
      printf("</tr>\n");

      printf("<tr><td><br/><br/></td><td></td></tr>\n"); 

      /*
       * light
       */
      if(peptide.chrg == i+1 &&
	 peptide.cidIndx == 0) {
	printf("<tr><th class=\"banner_cid\" ");
      } else {
	printf("<tr><th class=\"banner2\" ");
      }
      printf("colspan=\"2\">Light +%d</th></tr>\n", i+1);      

      // light scan
      printf("<tr><td>Scan: </td>");
      printf("<td><input type=\"text\" name=\"peptide_peaks_%d_0_valley_0\" size=\"5\" value=\"%d\"/> ", i, peptide.peaks[i][0].valley[0]);
      printf("<input type=\"text\" name=\"peptide_peaks_%d_0_valley_1\" size=\"5\" value=\"%d\"/></td></tr>\n", i, peptide.peaks[i][0].valley[1]);

      // light bckgrnd
      printf("<tr><td>Background: </td>");
      printf("<td><input type=\"text\" name=\"peptide_peaks_%d_0_bckgrnd\" size=\"8\" value=\"%.2e\"/></td></tr>\n", i, peptide.peaks[i][0].bckgrnd);

      // light elution time
      printf("<tr><td>Time: </td><td>%.2f +- %.2f (min)</td></tr>\n", 
	     peptide.peaks[i][0].time[0], peptide.peaks[i][0].time[1]);

      printf("<tr><td>&nbsp;</td><td>&nbsp;</td></tr>\n");

      /*
       * heavy
       */
      if(peptide.chrg == i+1 &&
	 peptide.cidIndx == 1) {
	printf("<tr><th class=\"banner_cid\" ");
      } else {
	printf("<tr><th class=\"banner2\" ");
      }
      printf("colspan=\"2\">Heavy +%d</th></tr>\n", i+1);      

      // heavy scan
      printf("<tr><td>Scan: </td>");
      printf("<td><input type=\"text\" name=\"peptide_peaks_%d_1_valley_0\" size=\"5\" value=\"%d\"/> ", i, peptide.peaks[i][1].valley[0]);
      printf("<input type=\"text\" name=\"peptide_peaks_%d_1_valley_1\" size=\"5\" value=\"%d\"/></td></tr>\n", i, peptide.peaks[i][1].valley[1]);

      // heavy bckgrnd
      printf("<tr><td>Background: </td> ");
      printf("<td><input type=\"text\" name=\"peptide_peaks_%d_1_bckgrnd\" size=\"8\" value=\"%.2e\"/></td></tr>\n", i, peptide.peaks[i][1].bckgrnd);

      // heavy elution time
      printf("<tr><td>Time: </td><td>%.2f +- %.2f (min)</td></tr>\n", 
	     peptide.peaks[i][1].time[0], peptide.peaks[i][1].time[1]);

      printf("</tbody></table>\n</td>\n</tr>\n\n");
    } //if(peptide.peaks[i][0].indx >= 0
    else {
      printf("<tr class=\"hideit\" id=\"plus%d_tr\">\n", i+1);
      printf("<td class=\"rejected\" align=\"center\" width=\"640\"><h4>-- No Data --</h4></td>\n\n");
      printf("<td width=\"275\" valign=\"top\" class=\"rejected\" id=\"plus%d\">\n", i+1);
      printf("<table width=\"100%%\"><tbody>\n<tr><th align=\"center\"><h1>+%d</h1></th></tr>\n", i+1);
      printf("<tr class=\"banner1\"><td>Ratio:</td>");
      printf("<td><h4>No Ratio</h4></td></tr>\n");
      printf("<tr><td>Weight:</td><td>------</td></tr>\n");
      printf("<tr><td><b>Acceptance:</b></td>");
      printf("<td>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<b>No</b></td>");
      printf("</tr>\n");
      printf("<tr><td><br/><br/></td><td></td></tr>\n");
      printf("</tbody></table>\n</td>\n</tr>\n\n");
    }
    fflush(stdout);
  } //   for (i = 0; i < _ASAPRATIO_MXQ_; ++i){

  printf("<tr>\n<td align=\"right\">&nbsp;<input name=\"submit\" value=\"Evaluate_Ratio\" type=\"submit\"/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</td>\n");
  printf("<td>&nbsp;</td>\n</tr>\n");
  printf("</tbody></table>\n\n\n");
  fflush(stdout);


  /*
   * hidden fields
   */
  // xmlFile
  printf("<input type=\"hidden\" name=\"Xmlfile\" value=\"%s\" />\n",
	 xmlFile);

  // spectrumName
  printf("<input type=\"hidden\" name=\"Spectrum\" value=\"%s\" />\n",
	 spectrumName);

  // baseName
  printf("<input type=\"hidden\" name=\"Basename\" value=\"%s\" />\n",
	 baseName);
  /*
  // pepIndx
  printf("<input type=\"hidden\" name=\"AsapIndex\" value=\"%d\" />\n",
	 pepIndx);
  */
  // Indx
  printf("<input type=\"hidden\" name=\"Indx\" value=\"%d\" />\n",
	 index);

  // ratioType
  printf("<input type=\"hidden\" name=\"ratioType\" value=\"%d\" />\n",
	 ratioType);
  
  // quantHighBG
  printf("<input type=\"hidden\" name=\"quantHighBG\" value=\"%d\" />\n",
	 (int)quantHighBG);
  // zeroBG
  printf("<input type=\"hidden\" name=\"zeroBG\" value=\"%d\" />\n",
	 (int)zeroBG);
  // mzBound
  printf("<input type=\"hidden\" name=\"mzBound\" value=\"%f\" />\n",
	 mzBound);

  // accRatio
  for (i = 0; i < 2; ++i) 
    printf("<input type=\"hidden\" name=\"accRatio_%d\" value=\"%f\" />\n",
	   i, accRatio[i]);
  
  // pepRatio
  for (i = 0; i < 2; ++i) 
    printf("<input type=\"hidden\" name=\"peptide_pepRatio_%d\" value=\"%f\" />\n", i, peptide.pepRatio[i]);
  
  // pepTime
  for (i = 0; i < 2; ++i) 
    for (j = 0; j < 2; ++j)
      printf("<input type=\"hidden\" name=\"peptide_pepTime_%d_%d\" value=\"%f\" />\n", i, j, peptide.pepTime[i][j]);
  
  // pepArea
  printf("<input type=\"hidden\" name=\"peptide_pepArea\" value=\"%f\" />\n",
	 peptide.pepArea);

  // indx
  printf("<input type=\"hidden\" name=\"peptide_indx\" value=\"%d\" />\n",
	 peptide.indx);
  
  // scan
  printf("<input type=\"hidden\" name=\"peptide_scan\" value=\"%ld\" />\n",
	 peptide.scan);
  
  // chrg
  printf("<input type=\"hidden\" name=\"peptide_chrg\" value=\"%d\" />\n",
	 peptide.chrg);
  
  // cidIndx
  printf("<input type=\"hidden\" name=\"peptide_cidIndx\" value=\"%d\" />\n",
	 peptide.cidIndx);
  
  // msLight
  printf("<input type=\"hidden\" name=\"peptide_msLight\" value=\"%f\" />\n",
	 peptide.msLight);

  // msHeavy
  printf("<input type=\"hidden\" name=\"peptide_msHeavy\" value=\"%f\" />\n",
	 peptide.msHeavy);
  
  // eltn
  printf("<input type=\"hidden\" name=\"peptide_eltn\" value=\"%d\" />\n",
	 peptide.eltn);
  fflush(stdout);


  /*
   * peaks
   */
  for (i = 0; i < _ASAPRATIO_MXQ_; ++i){
    for (j = 0; j < 2; ++j) {
      // indx and peak
      printf("<input type=\"hidden\" name=\"peptide_peaks_%d_%d_indx\" value=\"%d\"/>\n", i, j, peptide.peaks[i][j].indx);
      printf("<input type=\"hidden\" name=\"peptide_peaks_%d_%d_peak\" value=\"%d\"/>\n", i, j, peptide.peaks[i][j].peak);
      // valley and bckgrnd
      if(peptide.peaks[i][0].indx < 0
	 && peptide.peaks[i][1].indx < 0) {
	for(k = 0; k < 2; ++k) 
	  printf("<input type=\"hidden\" name=\"peptide_peaks_%d_%d_valley_%d\" value=\"%d\"/>\n", i, j, k, peptide.peaks[i][j].valley[k]);
	printf("<input type=\"hidden\" name=\"peptide_peaks_%d_%d_bckgrnd\" value=\"%f\"/>\n", i, j, peptide.peaks[i][j].bckgrnd);
      } // if(peptide.peaks[i][0].indx < 0
      
      // area and time
      for(k = 0; k < 2; ++k) {
	printf("<input type=\"hidden\" name=\"peptide_peaks_%d_%d_area_%d\" value=\"%f\"/>\n", i, j, k, peptide.peaks[i][j].area[k]);
	printf("<input type=\"hidden\" name=\"peptide_peaks_%d_%d_time_%d\" value=\"%f\"/>\n", i, j, k, peptide.peaks[i][j].time[k]);
      }
    } // for (j = 0; j < 2; ++j) {
    fflush(stdout);
  } //   for (i = 0; i < _ASAPRATIO_MXQ_; ++i) {


  /*
   * pkRatio, pkError, and pkCount
   */
  for (i = 0; i < _ASAPRATIO_MXQ_; ++i){
    // pkRatio
    printf("<input type=\"hidden\" name=\"peptide_pkRatio_%d\" value=\"%f\" />\n", i, peptide.pkRatio[i]);
    
    // pkError
    printf("<input type=\"hidden\" name=\"peptide_pkError_%d\" value=\"%f\" />\n", i, peptide.pkError[i]);

    // pkCount
    if(peptide.peaks[i][0].indx < 0
       && peptide.peaks[i][1].indx < 0) {
      printf("<input type=\"hidden\" name=\"peptide_pkCount_%d\" value=\"%d\" />\n", i, peptide.pkCount[i]);
    }
    fflush(stdout);
  } //   for (i = 0; i < _ASAPRATIO_MXQ_; ++i) {

  printf("\n\n</form>\n\n");

  printf("<h6>Graphs are generated by <a target=\"_new\" href=\"http://www.gnuplot.info\">gnuplot</a>.</h6>\n");
  fflush(stdout);

  return;
}


int main(int argc, char **argv)
{
  hooks_tpp handler(argc,argv); // set up install paths etc
 
  // cgi variables
  char *queryString;
  char cgiAction[1000];
  char szWebserverRoot[1000];
  const char *pStr;
  // parameters
  char *xmlFile=NULL; // .bof file
  char *baseName=NULL; // base name for .mzXML file
  char *timeStamp; 
  int index;
  int ratioType;
  char* spectrumName=NULL; 
  int quantHighBG = 0;
  int zeroBG = 0;
  int wv = 0;
  bool wavelet = false;
  double mzBound = -1;

  // files
  char pngFilePath[1000]; 
  char pngFileLink[1000];

  pStr=getWebserverRoot();
  if (pStr==NULL)
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
    char szCommand[1024];
    FILE *fp;
    sprintf(szCommand, "cygpath '%s'", pStr);
    if((fp = popen(szCommand, "r")) == NULL)
    {
      printf("cygpath error, exiting\n");
      exit(1);
    }
    else
    {
      char szBuf[1024];
      fgets(szBuf, SIZE_BUF, fp);
      pclose(fp);
      szBuf[strlen(szBuf)-1] = 0;
      strcpy(szWebserverRoot, szBuf);
    }
#else
    strcpy(szWebserverRoot, pStr); 
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


  // variables
  pepDataStrct *peptide=NULL;
  char *mzXMLFile;
  char tmpString[1000];
  int wrtIndx;
  double accRatio[2];
  char *tmpValue;

  int i;

  /*
   * html header, style-sheet, and javascript
   */
  printf("Content-type: text/html\n\n");
  printf("<html>\n<head><title>Automated Statistical Analysis on Protein Abundance Ratio (%s)",szTPPVersionInfo);
  printf("</title></head>\n");
  printf("<body bgcolor=\"#c0c0c0\" onload=\"self.focus(); animate();\">\n");

  printf("<style type=\"text/css\">\n");
  printf(".hideit {display:none}\n");
  printf(".showit {display:table-row}\n");
  printf(".accepted {background: #dddddd;}\n");
  printf(".rejected {background: #b5b5b5;}\n");

  printf("h1  {font-family: Helvetica, Arial, Verdana, sans-serif; font-size: 24pt; font-weight:bold; color:#0E207F}\n");
  printf("h2  {font-family: Helvetica, Arial, sans-serif; font-size: 20pt; font-weight: bold; color:#0E207F}\n");
  printf("h3  {font-family: Helvetica, Arial, sans-serif; font-size: 16pt; color:#FF8700}\n");
  printf("h4  {font-family: Helvetica, Arial, sans-serif; font-size: 14pt; color:#0E207F}\n");
  printf("h5  {font-family: Helvetica, Arial, sans-serif; font-size: 10pt; color:#AA2222}\n");
  printf("h6  {font-family: Helvetica, Arial, sans-serif; font-size:  8pt; color:#333333}\n");

  printf("table   {border-collapse: collapse; border-color: #000000;}\n");
  printf("td      {border-collapse: collapse; border-color: #000000;}\n");

  printf(".params       {\n");
  printf("                 font-size:  10pt;\n");
  printf("                 color:      #333333;\n");
  printf("              }\n");
  printf(".banner_cid   {\n");
  printf("                 background: #0e207f;\n");
  printf("                 border: 2px solid #0e207f;\n");
  printf("                 color: #eeeeee;\n");
  printf("                 font-weight:bold;\n");
  printf("              }\n");
  printf(".banner2      {\n");
  printf("                 background: #aaaaaa;\n");
  printf("                 color: black;\n");
  printf("              }\n");
  printf(".banner1      {\n");
  printf("                 background: #FF8700;\n");
  printf("                 color: black;\n");
  printf("              }\n");
  printf(".formentry    {\n");
  printf("                 background: #eeeeee;\n");
  printf("                 border: 2px solid #0e207f;\n");
  printf("                 color: black;\n");
  printf("                 padding: 1em;\n");
  printf("              }\n");
  printf(".nav          {\n");
  printf("                 background: #c0c0c0;\n");
  printf("                 border: 1px solid black;\n");
  printf("                 font-family: Helvetica, Arial, Verdana, sans-serif;\n");
  printf("                 font-weight:bold;\n");
  printf("              }\n");
  printf(".navselected  {\n");
  printf("                 background: #dddddd;\n");
  printf("                 border: 1px solid black;\n");
  printf("                 border-top: 2px solid black;\n");
  printf("                 font-family: Helvetica, Arial, Verdana, sans-serif;\n");
  printf("                 font-weight:bold;\n");
  printf("              }\n");
  printf("</style>\n\n");

  printf("<SCRIPT LANGUAGE=\"JavaScript\">\n");
  printf("    var showParams = true\n");
  printf("    var charges = new Array()\n");

  for (i = 0; i < _ASAPRATIO_MXQ_; ++i){
    printf("    charges[%d] = \"plus%d\";\n",i,i+1);
  }
  printf("\n");

  printf("    function highlight(elementId,yesno){\n");
  printf("      c_element = document.getElementById(elementId);\n");
  printf("      if(yesno == 'yes') {\n");
  printf("        c_element.className = 'accepted';\n");
  printf("       } else {	\n");
  printf("        c_element.className = 'rejected';\n");
  printf("       }	\n");
  printf("    }\n");

  printf("    function display(chargestate){\n");
  printf("      var x;\n");
  printf("      var new_state;\n");
  printf("      var new_nav;\n");
  printf("      for (x in charges) {\n");
  printf("        if (charges[x] == chargestate) {\n");
  printf("	  new_state = 'showit';\n");
  printf("	  new_nav = 'navselected';\n");
  printf("	} else {\n");
  printf("	  new_state = 'hideit';\n");
  printf("	  new_nav = 'nav';\n");
  printf("	}\n");
  printf("	document.getElementById(charges[x] + \"_tr\").className = new_state;\n");
  printf("	document.getElementById(charges[x] + \"_nav\").className = new_nav;\n");
  printf("      }\n");
  printf("    }\n");

  printf("    function displayAll(){\n");
  printf("     for (x in charges) {\n");
  printf("	document.getElementById(charges[x] + \"_tr\").className = 'showit';\n");
  printf("	document.getElementById(charges[x] + \"_nav\").className = 'navselected';\n");
  printf("      }\n");
  printf("    }\n");

  printf("    i = 0;\n");
  printf("    f = 400;\n");
  printf("    animation = false;\n");
  printf("    function animate(){\n");
  printf("      if (++i == %d) i=0; \n", _ASAPRATIO_MXQ_);
  printf("      tt = 0;\n");
  printf("      if (animation && document.getElementById(\"UIanim_chg\"+i).checked) {\n");
  printf("            display(charges[i]);\n");
  printf("	    tt = f;\n");
  printf("      }\n");
  printf("      t = setTimeout(\"animate();\", tt);\n");
  printf("    }\n");

  printf("    function anim_ctrl(rate) {\n");
  printf("      document.getElementById('UIanim_fast').disabled = false;\n");
  printf("      document.getElementById('UIanim_slow').disabled = false;\n");
  printf("      document.getElementById('UIanim_stop').disabled = false;\n");
  printf("      document.getElementById('UIanim_' + rate).disabled = true;\n");
  printf("      if (rate == 'fast') {\n");
  printf("        f = 400;\n");
  printf("        animation=true;\n");
  printf("      }\n");
  printf("      if (rate == 'slow') {\n");
  printf("        f = 750;\n");
  printf("        animation=true;\n");
  printf("      }\n");
  printf("      if (rate == 'stop') {\n");
  printf("        f = 400;\n");
  printf("        animation=false;\n");
  printf("      }\n");
  printf("    }\n");

  printf("    function toggleParams(){\n");
  printf("      var x;\n");
  printf("      var new_state = 'hideit';\n");
  printf("      if (showParams) {\n");
  printf("        showParams = false;\n");
  printf("        new_state = 'showit';\n");
  printf("        document.getElementById('paramshead').innerHTML = 'Hide Run Parameters';\n");
  printf("      } else {\n");
  printf("        showParams = true;\n");
  printf("        document.getElementById('paramshead').innerHTML = 'Show Run Parameters...';\n");
  printf("      }\n");
  printf("      for(var i=1; i<=3; i++) {\n");
  printf("	  document.getElementById(\"parameters\"+i).className = new_state;\n");
  printf("      }\n");
  printf("    }\n");

  printf("</SCRIPT>\n\n");

  printf("<h1>ASAPRatio: Peptide Ratio</h1>\n");
  fflush(stdout);


  /*
   * collect information
   */

  /*
   * initial steps
   */
  // get queryString
  queryString = getQueryString();
  if(queryString == NULL) {
    printf("<font color=\"red\">Error in passing parameters from web.</font><br/>\n");
    printf("</body></html>\n");
    fflush(stdout);
    return 1;
  }

  // get cgiAction
  if((tmpValue = getenv("SCRIPT_NAME")) != NULL) {
    sprintf(cgiAction, "%s", tmpValue);
  }
  else {
    printf("<font color=\"red\">Cannot find SCRIPT_NAME. </font><br/>\n");
    printf("</body></html>\n");
    fflush(stdout);
    free(queryString);
    return 1;
  }


  /*
   * collect parameters from web file
   */
  // collect information from link in XML file 
  if(strcmp(getenv("REQUEST_METHOD"), "GET") == 0) {
    // xmlFile
    if((xmlFile = getHtmlFieldValue("Xmlfile", queryString)) == NULL){
      printf("<font color=\"red\">No input for .bof file!</font><br/>\n");
      printf("</body></html>\n");
      fflush(stdout);
      free(queryString);
      return 1;
    }
    fixPath(xmlFile,1); // tidy up path sep chars etc - expect existence

    // baseName
    if((baseName = getHtmlFieldValue("Basename", queryString)) == NULL){
      printf("<font color=\"red\">No input for .mzXML/.mzData file!</font><br/>\n");
      printf("</body></html>\n");
      fflush(stdout);
      free(queryString);
      free(xmlFile);
      return 1;
    }

    // timeStamp
    if((timeStamp = getHtmlFieldValue("Timestamp", queryString)) == NULL){
      printf("<font color=\"red\">No input for time stamp!</font><br/>\n");
      printf("</body></html>\n");
      fflush(stdout);
      free(queryString);
      free(xmlFile);
      free(baseName);
      return 1;
    }

    // spectrumName
    if((spectrumName = getHtmlFieldValue("Spectrum", queryString)) == NULL){
      printf("<font color=\"red\">No input for spectrum!</font><br/>\n");
      printf("</body></html>\n");
      fflush(stdout);
      free(queryString);
      free(xmlFile);
      free(baseName);
      return 1;
    }
    // quantHighBG
    if((tmpValue = getHtmlFieldValue("quantHighBG", queryString)) != NULL) {
      sscanf(tmpValue, "%d", &quantHighBG);
      free(tmpValue);

    }
    // zeroBG
    if((tmpValue = getHtmlFieldValue("zeroBG", queryString)) != NULL) {
      sscanf(tmpValue, "%d", &zeroBG);
      free(tmpValue);

    }
    // wavelet
    if((tmpValue = getHtmlFieldValue("wavelet", queryString)) != NULL) {
      sscanf(tmpValue, "%d", &wv);
      free(tmpValue);
      if (wv) {
	wavelet = true;
      }
    }
    // mzBound
    if((tmpValue = getHtmlFieldValue("mzBound", queryString)) != NULL) {
      sscanf(tmpValue, "%lf", &mzBound);
      free(tmpValue);

    }
    // Indx
    if((tmpValue = getHtmlFieldValue("Indx", queryString)) != NULL) {
      sscanf(tmpValue, "%d", &index);
      //cout << "just read in: " << index << endl;

      free(tmpValue);
      if(index < 1) {
	printf("<font color=\"red\">Invalid peptide index: \"%d\"!</font><br/>\n", index);
	printf("</body></html>\n");
	fflush(stdout);
	free(queryString);
	free(xmlFile);
	free(baseName);
	free(timeStamp);
	return 1;
      }
    }
    else {
      printf("<font color=\"red\">No peptide index passed!</font><br/>\n");
      printf("</body></html>\n");
      fflush(stdout);
      free(queryString);
      free(xmlFile);
      free(baseName);
      free(timeStamp);
      return 1;
    }

	if((tmpValue = getHtmlFieldValue("submit", queryString)) != NULL) {
		if(strcmp(tmpValue, "Next_Peptide") == 0)	{
		index+=1;
		}
		else if (strcmp(tmpValue, "Prev_Peptide") == 0 && index > 1) {
		index-=1;
		}
	}
    // peptide

    /*** Andy, check this section  ***/
    //cout << "value of index: " << index << " vs asap: " << pepIndx << endl;

    ASAPRatioPeptideCGIDisplayParser* parser = new ASAPRatioPeptideCGIDisplayParser(xmlFile, baseName, timeStamp, index, -2, zeroBG, mzBound);
        
    if(parser == NULL || ! parser->found()) {
      cout << "Error: could not find entry for " << index << " index with basename " << baseName << " in xmlfile: " << xmlFile << endl;
      exit(1);
    }

    peptide = new pepDataStrct();
    *peptide = parser->getPepDataStruct();

    // pepDataStrct
    //    peptide = new CGIDisplayParser(xmlFile, baseName, timeStamp, pepIndx);    

    /*** end check section ***/
    
    // accepted ratio
    accRatio[0] = peptide->pepRatio[0];
    accRatio[1] = peptide->pepRatio[1];
    
    free(timeStamp);
    delete parser;
  } //   if(strcmp(getenv("REQUEST_METHOD"), "GET") == 0) 

  // collect information from CGI
  if(strcmp(getenv("REQUEST_METHOD"), "GET") != 0) {
    // xmlFile
    if((xmlFile = getHtmlFieldValue("Xmlfile", queryString)) == NULL){
      printf("<font color=\"red\">No input for .bof file!</font><br/>\n");
      printf("</body></html>\n");
      fflush(stdout);
      free(queryString);
      return 1;
    }

    // spectrumName
    if((spectrumName = getHtmlFieldValue("Spectrum", queryString)) == NULL){
      printf("<font color=\"red\">No input for spectrum!</font><br/>\n");
      printf("</body></html>\n");
      fflush(stdout);
      free(queryString);
      free(xmlFile);
      return 1;
    }

    // baseName
    if((baseName = getHtmlFieldValue("Basename", queryString)) == NULL){
      printf("<font color=\"red\">No input for .mzXML/.mzData file!</font><br/>\n");
      printf("</body></html>\n");
      fflush(stdout);
      free(queryString);
      free(xmlFile);
      return 1;
    }
    /*
    // pepIndx
    if((tmpValue = getHtmlFieldValue("AsapIndex", queryString)) != NULL) {
      sscanf(tmpValue, "%d", &pepIndx);
      free(tmpValue);
      if(pepIndx < 1) {
	printf("<font color=\"red\">Invalid peptide index: \"%d\"!</font><br/>\n", pepIndx);
	printf("</body></html>\n");
	fflush(stdout);
	free(queryString);
	free(xmlFile);
	free(baseName);
	return 1;
      }
    }
    */
    // pepIndx
    if((tmpValue = getHtmlFieldValue("Indx", queryString)) != NULL) {
      sscanf(tmpValue, "%d", &index);
      free(tmpValue);
      if(index < 1) {
	printf("<font color=\"red\">Invalid peptide index: \"%d\"!</font><br/>\n", index);
	printf("</body></html>\n");
	fflush(stdout);
	free(queryString);
	free(xmlFile);
	free(baseName);
	return 1;
      }
    }
    else {
      printf("<font color=\"red\">No peptide index passed!</font><br/>\n");
      printf("</body></html>\n");
      fflush(stdout);
      free(queryString);
      free(xmlFile);
      free(baseName);
      return 1;
    }

	if((tmpValue = getHtmlFieldValue("submit", queryString)) != NULL) {
		if(strcmp(tmpValue, "Next_Peptide") == 0)	{
		index+=1;
		}
		else if (strcmp(tmpValue, "Prev_Peptide") == 0 && index > 1) {
		index-=1;
		}
	}

    // peptide
    if((peptide = getPepDataStrctFromQueryString(queryString)) == NULL){
      printf("<font color=\"red\">Cannot construct pepDataStrct from CGI.</font><br/>\n");
      printf("</body></html>\n");
      fflush(stdout);
      free(queryString);
      free(xmlFile);
      free(baseName);
      return 1;
    }
    
    // accRatio
    for (i = 0; i < 2; ++i) {
      sprintf(tmpString, "accRatio_%d", i);
      if ((tmpValue = getHtmlFieldValue(tmpString, queryString)) != NULL) {
	sscanf(tmpValue, "%lf", &(accRatio[i]));
	free(tmpValue);
      }
      else
	accRatio[i] = peptide->pepRatio[i];
    }
  } //   if(strcmp(getenv("REQUEST_METHOD"), "GET") != 0) 

  // ratioType
  if ((tmpValue = getHtmlFieldValue("ratioType", queryString)) != NULL) {
    if(sscanf(tmpValue, "%d", &ratioType) != 1
       || ratioType < 0 
       || ratioType > 2) {
      ratioType = 0;
    }
    free(tmpValue);
  }
  else
    ratioType = 0;

  // quantHighBG
  if ((tmpValue = getHtmlFieldValue("quantHighBG", queryString)) != NULL) {
    if(sscanf(tmpValue, "%d", &quantHighBG) != 1
       || quantHighBG < 0 
       || quantHighBG > 1) {
      quantHighBG = 0;
    }
    free(tmpValue);
  }
  else
    quantHighBG = 0;

  // zeroBG
  if ((tmpValue = getHtmlFieldValue("zeroBG", queryString)) != NULL) {
    if(sscanf(tmpValue, "%d", &zeroBG) != 1
       || zeroBG < 0 
       || zeroBG > 1) {
      zeroBG = 0;
    }
    free(tmpValue);
  }
  else
    zeroBG = 0;


  // mzBound
  if ((tmpValue = getHtmlFieldValue("mzBound", queryString)) != NULL) {
    if(sscanf(tmpValue, "%lf", &mzBound) != 1
       || mzBound <= 0 
       || mzBound > 1) {
      mzBound = 0.5;
    }
    free(tmpValue);
  }
  else
    mzBound = 0.5;
  
  strcpy(pngFilePath, xmlFile);
  char* tmpStr = findRightmostPathSeperator(pngFilePath); 
  if (tmpStr++)
     *tmpStr = 0;
  strcat(pngFilePath, "ASAPRatio");
  replace_path_with_webserver_tmp(pngFilePath,sizeof(pngFilePath)); // write this in tmpdir if we have one
  strcpy(pngFileLink, pngFilePath);
  translate_absolute_filesystem_path_to_relative_webserver_root_path(pngFileLink); // so /inetpub/wwwroot/foo becomes /foo

  // submit
  if((tmpValue = getHtmlFieldValue("submit", queryString)) != NULL) {
     int len;
    mzXMLFile = (char *) calloc(len=strlen(baseName)+strlen(xmlFile)+10, sizeof(char));
    rampConstructInputPath(mzXMLFile, len, xmlFile, baseName); // .mzXML or .mzData
    wrtIndx = 1;
    if(strcmp(tmpValue, "Interim_Ratio") == 0) { // accept ratio
      peptide->indx = 2;
      getPepDataStrct(peptide, mzXMLFile, NULL, 0,  quantHighBG, zeroBG, mzBound, wavelet);
    }
    else if(strcmp(tmpValue, "0:1") == 0) { // set to 0:1
      peptide->indx = 2;
      if(ratioType == 1)
	peptide->pepRatio[0] = -1.;
      else
	peptide->pepRatio[0] = 0.;
      peptide->pepRatio[1] = 0.;
    }
    else if(strcmp(tmpValue, "1:0") == 0) { // set to 1:0
      peptide->indx = 2; 
      if(ratioType == 1)
	peptide->pepRatio[0] = 0.;
      else 
	peptide->pepRatio[0] = -1.;
      peptide->pepRatio[1] = 0.;
    }
    else if(strcmp(tmpValue, "Unknown") == 0) { // set to 0:0
      peptide->indx = 2; 
      peptide->pepRatio[0] = -2.;
      peptide->pepRatio[1] = 0.;
    }
    else {
      wrtIndx = 0;
    }
    free(tmpValue);
    free(mzXMLFile);

    // update XML file
    if(wrtIndx == 1) {
      // accepted ratio
      accRatio[0] = peptide->pepRatio[0];
      accRatio[1] = peptide->pepRatio[1];

      /*** Andy, check this section  ***/

      // write pepDataStrct
      //    Parser *parser = new CGIParser(*peptide, xmlFile, baseName, pepIndx);

      //ASAPRatioPeptideUpdateParser* updateParser = new ASAPRatioPeptideUpdateParser(xmlFile, baseName, pepIndx, generateXML(*peptide, pepIndx, True, True));
      ASAPRatioPeptideUpdateParser* updateParser = new ASAPRatioPeptideUpdateParser(xmlFile, baseName, index, generateXML(*peptide, 0, True, True));

      if(updateParser != NULL && updateParser->update())
	cout << "<h5>Changes made to " << xmlFile << ", refresh browser to view</h5>" << endl;
      else
	cout << "<h5>Error: no changes written to " << xmlFile << "</h5>" << endl;

      /*** end check section ***/

    } // if(wrtIndx == 1) {
  } //  if((tmpValue = getHtmlFieldValue("submit", queryString)) != NULL) {

  // free queryString
  free(queryString);


  /*
   *  cgi display
   */
  displayPepDataStrctInCgi(*peptide, cgiAction, xmlFile, baseName,
			   index, ratioType, accRatio, pngFileLink, pngFilePath, spectrumName, quantHighBG, zeroBG, mzBound, wavelet);

  printf("<hr noshade/>\n");
  printf("<h6>(%s)<br/>\n",szTPPVersionInfo);
  printf("This interface is used for evaluating and modifying results on peptide abundance ratios.<br/>\n");
  printf("Developed by Dr. Xiao-jun Li and Luis Mendoza at <a target=\"_new\" href=\"http://www.systemsbiology.org\"> ");
  printf("Institute for Systems Biology. </a> </h6>\n\n");
  printf("</body></html>\n");
  fflush(stdout);

  // free memory
  free(xmlFile);
  free(baseName);
  delete peptide;

  return 1;
}

