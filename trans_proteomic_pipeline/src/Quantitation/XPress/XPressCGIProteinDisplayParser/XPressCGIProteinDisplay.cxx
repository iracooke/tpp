/*

Program       : XPressProteinDisplay                                                  
Author        : J.Eng and Andrew Keller <akeller@systemsbiology.org>, Robert Hubley, and 
                open source code                                                       
Date          : 11.27.02 

Primary data object holding all mixture distributions for each precursor ion charge

Copyright (C) 2003 Andrew Keller

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

Andrew Keller
Institute for Systems Biology
401 Terry Avenue North 
Seattle, WA  98109  USA
akeller@systemsbiology.org

*/

#include "XPressCGIProteinDisplay.h"

#include "common/TPPVersion.h" // contains version number, name, revision

#include "common/util.h"
#include <errno.h>

XPressCGIProteinDisplay::XPressCGIProteinDisplay(const char* inputfiles, char* peptides, const char* protein, 
			  const char* cgihome, const char* protxmlfile, 
			  double minpepprob, Boolean heavy2light, const char* xslt, const char* mark_aas, Boolean glyc) {


  //cout << "peptides: xx" << peptides << "xx" << endl;


  mark_aas_ = new char[strlen(mark_aas)+1];
  strcpy(mark_aas_, mark_aas);
  glyc_ = glyc;

  inputfiles_ = parse(inputfiles, ' ');
  inputlinks_ = parse(inputfiles, ' ');
  
  // first replace all '~' with '#'
  int k;
  for(k = 0; peptides[k]; k++) {
    if(peptides[k] == '~')
      peptides[k] = '#';
  }

  Array<const char*>*peps = parse(peptides, '+');
  peptides_ = new peplist;
  for (int p=0;p<peps->size();p++) {
     peptides_->add((*peps)[p]);
  }
  delete peps;

  minpepprob_ = minpepprob;
  xslt_ = new char[strlen(xslt)+1];
  strcpy(xslt_, xslt);

  cgihome_ = new char[strlen(cgihome)+1];
  strcpy(cgihome_, cgihome);

  parser_ = new XPressProteinRatioParser(*inputfiles_, *peptides_, minpepprob);
  if(parser_ != NULL) {
    pRatio_ = parser_->getRatio();
    delete parser_;
  }
  else {
    cout << "Error: null parser for inputfiles: ";
    int k;
    for(k = 0; k < inputfiles_->length(); k++)
      cout << (*inputfiles_)[k] << " ";
    cout << " and peptides: ";
    for(k = 0; k < peptides_->size(); k++) {
       if(k) {
          cout << " ";
       }
       cout << peptides_->getNthCharptr(k); // write out in order read in
    }
    cout << endl;
    exit(1);
  }

  // set up xslfile
  char suffix[] = ".tmp.xsl";
  char* xslfile = new char[strlen((*inputfiles_)[0]) + strlen(suffix) + 1];
  strcpy(xslfile, (*inputfiles_)[0]);
  strcat(xslfile, suffix);
  unlink(xslfile); // clear any previous file

  //cout << "writing to xsl " << xslfile << endl;
  //writeXSLFile(xslfile, minpepprob);

  char command[3000];
  FILE* fp;
  const int line_width = 50000; // extra wide
  char next_line[line_width];

#ifdef USING_RELATIVE_WEBSERVER_PATH  // for example, in win32 understand /foo/bar as /Inetpub/wwwroot/foo/bar
  char szWebserverRoot[256], *pStr;
  // Get webserver root path to remove it from the link
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
  else  {
#ifdef WINDOWS_CYGWIN
   char szCommand[4096], szBuf[4096];
    // must first pass to cygpath program
    sprintf(szCommand, "cygpath '%s'", pStr);
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
#ifdef WINDOWS_CYGWIN
  for(k = 0; k < strlen(szWebserverRoot); k++) {
    szWebserverRoot[k] = tolower(szWebserverRoot[k]);
  }
  if(szWebserverRoot[strlen(szWebserverRoot)] == '/') {
    szWebserverRoot[strlen(szWebserverRoot)] = '\0';
  }
#endif
#endif

  //printf("Content-type: text/html\n\n");
  printf("<HTML>\n<HEAD><TITLE>%s (%s)</TITLE></HEAD>\n", "XPRESSDisplay", szTPPVersionInfo);
  printf("<BODY BGCOLOR=\"#FFFFFF\" OnLoad=\"self.focus();\">\n");
  printf("<PRE>");
  printf("<b>XPressCGIProteinDisplay (%s)</b>\n\n", szTPPVersionInfo);
  if(heavy2light) 
    printf("XPRESS ratio in ProteinProphet: heavy:light  (%0.2f &plusmn; %0.2f)  <b>%s</b> %d entries used\n", pRatio_.dh2lRatio, pRatio_.dh2lStdDev, protein, pRatio_.iNumPeptides);
  else
    printf("XPRESS ratio in ProteinProphet: light:heavy  (%0.2f &plusmn; %0.2f)  <b>%s</b> %d entries used</CENTER>", pRatio_.dRatio, pRatio_.dStdDev, protein, pRatio_.iNumPeptides);

  if(heavy2light) {

    printf("                                 %0.2f:%0.2f\n", pRatio_.dh2lRatio, 1.0);
    printf("                                 %0.2f:%0.2f", 1.0, pRatio_.dRatio);

  }
  else {
    printf("                                 %0.2f:%0.2f\n", pRatio_.dRatio, 1.0);
    printf("                                 %0.2f:%0.2f", 1.0, pRatio_.dh2lRatio);
  }

  printf("\n\n");
  printf("<HR>");

  for(k = 0; k < inputfiles_->length(); k++) {
    
#ifdef USING_RELATIVE_WEBSERVER_PATH  // for example, in win32 understand /foo/bar as /Inetpub/wwwroot/foo/bar
    //remove webserver path from link
    const char *pStr = strstri((*inputlinks_)[k], szWebserverRoot);

    if(pStr != NULL) {
     if(strlen((*inputlinks_)[k]) > strlen(szWebserverRoot) && pStr[strlen(szWebserverRoot)] != '/') {
       sprintf((char *)(*inputlinks_)[k], "/%s", pStr + strlen(szWebserverRoot));
     }
     else {
       strcpy((char *)(*inputlinks_)[k], pStr + strlen(szWebserverRoot));
     }
    }

#endif

    writeXSLFile(xslfile, (*inputfiles_)[k], heavy2light);

    const char *ext = hasValidPepXMLFilenameExt((*inputlinks_)[k]);
    if (ext) {
      printf("<A TARGET=\"Win3\" HREF=\"");
      for(const char *j = (*inputlinks_)[k]; j<ext;j++) {
         printf("%c", *j);
      }
      printf(".pep.shtml\">");
    }
    else
      printf("<A TARGET=\"Win3\" HREF=\"%s\">", (*inputlinks_)[k]);

    printf("<font color=\"red\">%s</font></A><p>\n", (*inputfiles_)[k]);
    fflush(stdout);
    printf("<TABLE cellpadding=\"2\" bgcolor=\"white\" style=\"font-family: 'Courier New', Courier, mono; font-size: 10pt;\">");
    command[0] = 0;

    if(strstr(xslt, "xsltproc") != NULL) {
      strcat(command, xslt);
      if (!strstr(xslt,"-novalid")) {
      strcat(command, " --novalid ");
      } else {
         strcat(command," ");
      }
      strcat(command, xslfile);
      strcat(command, " ");
      strcat(command, (*inputfiles_)[k]);
    }
    else { // conventional case
      strcat(command, xslt);
      strcat(command, " ");
      strcat(command, (*inputfiles_)[k]);
      strcat(command, " ");
      strcat(command, xslfile);
    }

    if((fp = tpplib_popen(command, "r")) == NULL) {
      cout << "error: cannot open pipe for " << command << endl;
      exit(1);
    }

    char *fgot=fgets(next_line, line_width, fp); // waste first line    

    char cginame[] = "Blast.cgi";
    char prefix[] = "<font size=\"-2\">";
    char suffix[] = "</font>";
    char nextmass[line_width];

    while (fgets(next_line, line_width, fp)) {
      char* match = strstr(next_line, cginame);
      if(match != NULL) {
	// check for mods
 
	// get peptide start and stop locations
	int pep_start = -1;
	int pep_stop = -1;

	char* query = strstr(next_line, "QUERY=");
	if(query != NULL) {
	  char* start = strchr(query, '>');
	  if(start != NULL && strlen(start) > 1) {
	    pep_start = strlen(next_line) - strlen(start) + 1;
	    char* end = strchr(start, '<');
	    if(end != NULL) {
	      pep_stop = strlen(next_line) - strlen(end) - 1;
	    }
	  }
	} // not null

	Boolean color = False;
   int l;
	for(l = 0; next_line[l]; l++) {

	  if(k >= pep_start && k <= pep_stop) {
	    if(! color && strchr(mark_aas_, next_line[l]) != NULL) { // color it
	      printf("<font color=\"red\">");
	      color = True;
	    }
	    else if(color && next_line[l] >= 'A' && next_line[l] <= 'Z' &&
		    strchr(mark_aas_, next_line[l]) == NULL) { // uncolor it
	      printf("</font>");
	      color = False;
	    }

	  }

	  if(next_line[l] == '[') {
	    // look ahead to see if of form: [2343.2342]
	    nextmass[0] = 0;
	    int skip = 0;
	    for(int j = l+1; j < (int)strlen(next_line); j++) {
	      if((next_line[j] >= '0' && next_line[j] <= '9') || next_line[j] == '.')
		nextmass[j-l-1] = next_line[j];
	      else if(next_line[j] == ']') { // done
		nextmass[j-l-1] = 0;
		//printf("%s%s%s", prefix, nextmass, suffix);
		skip = strlen(nextmass) + 1;
		break;
	      }
	      else
		break; // done
	    } // next lookahead
	    if(skip) 
		printf("%s%s%s", prefix, nextmass, suffix);
	    else
	      printf("%c", next_line[l]);
	    l += skip;
	  }
	  else
	    printf("%c", next_line[l]);
	} // next char
	printf("\n");	
      }
      else
	printf("%s\n", next_line); // display it
    }
    
    pclose(fp);
    printf("</TABLE>");
    printf("<HR>");
  } // next inputfile

  printf("</PRE>");

  //printf("</TABLE>");
  //unlink(xslfile);

  printf("<p>");
  printf("<TT><B> Click RELOAD in your browser to re-evaluate ratio if changes have been made to the XPRESS quantitation in any of the entries above.  This will update what you see in the current ratio just above.");
  printf("<p>To accept the current ratio back to ProteinProphet, click on the button below.");


  printf("<CENTER><FORM METHOD=POST ACTION=\"%sxpress-prophet-update.cgi\" TARGET=\"Win1\">\n", cgihome);

  printf("<INPUT TYPE=\"hidden\" NAME=\"protein\" VALUE=\"%s\">\n", protein);
  printf("<INPUT TYPE=\"hidden\" NAME=\"xmlfile\" VALUE=\"%s\">\n", protxmlfile);
  printf("<INPUT TYPE=\"hidden\" NAME=\"stddev\" VALUE=\"%0.4f\">\n", pRatio_.dStdDev);
  printf("<INPUT TYPE=\"hidden\" NAME=\"ratio\" VALUE=\"%0.4f\">\n", pRatio_.dRatio);
  printf("<INPUT TYPE=\"hidden\" NAME=\"h2l_ratio\" VALUE=\"%0.4f\">\n", pRatio_.dh2lRatio);
  printf("<INPUT TYPE=\"hidden\" NAME=\"h2l_stddev\" VALUE=\"%0.4f\">\n", pRatio_.dh2lStdDev);
  printf("<INPUT TYPE=\"hidden\" NAME=\"num\" VALUE=\"%d\">\n", pRatio_.iNumPeptides);
  if(heavy2light)
    printf("<INPUT TYPE=\"hidden\" NAME=\"heavy2light\" VALUE=\"%d\">\n", heavy2light);
  printf("<INPUT TYPE=\"submit\" VALUE=\"Update ProteinProphet ratio\"></FORM></CENTER>\n");

  printf("</BODY></HTML>");

}


Array<const char*>* XPressCGIProteinDisplay::parse(const char* input, char separator) {
  Array<const char*>* output = new Array<const char*>;
  int start = 0;
  char* next = NULL;
  for(int k = 0; k <= (int)strlen(input); k++)
    if(k == (int)strlen(input) || input[k] == separator) {
      next = new char[k-start+1];
      strncpy(next, input+start, k-start);
      next[k-start] = 0;
      if (' '==separator) { // files
         fixPath(next,1); // pretty up the path seperators etc - expect existence
      }
      output->insertAtEnd(next);
      start = k+1;
    }

  return output;

}

void XPressCGIProteinDisplay::writeXSLFile(const char* xslfile, const char* xmlfile, Boolean heavy2light) {
  FILE* fp;


  if ( (fp=fopen(xslfile, "w"))==NULL)
     {
       printf("Error - cannot open %s: %s", xslfile, strerror(errno));
       return;
     }

  const int SIZE = 10000;

  char index[SIZE], probability[SIZE], spectrum[SIZE], peptide_sequence[SIZE], protein[SIZE], xpress[SIZE];
  char table_spacer[] = "<xsl:text>     </xsl:text>";
  char ions[SIZE];
  char asapratio[4 * SIZE];
  char asap_ref[SIZE];
  char plusmn[] = "+-" ; //"&#177;";
  sprintf(index, "%s%s%s", "<td><xsl:value-of select=\"@index\"/>", table_spacer, "</td>");


  //my $spec_ref = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'sequest-tgz-out.cgi?OutFile={$basename}/{$xpress_spec}.out">';
  //$display{'spectrum'} = '<td>' . $spec_ref . '<xsl:value-of select="@spectrum"/></A>' . $table_spacer . '</td>';

  //my $spec_ref = '<xsl:choose><xsl:when test="parent::node()/@search_engine=\'SEQUEST\'"><A TARGET="Win1" HREF="' . $CGI_HOME . 'sequest-tgz-out.cgi?OutFile={$basename}/{$xpress_spec}.out"><xsl:value-of select="@spectrum"/></A></xsl:when><xsl:when test="parent::node()/@search_engine=\'MASCOT\'"><A TARGET="Win1" HREF="' . $CGI_HOME . 'mascotout.pl?OutFile={$basename}/{$xpress_spec}.out"><xsl:value-of select="@spectrum"/></A></xsl:when><xsl:when test="parent::node()/@search_engine=\'COMET\'"><A TARGET="Win1" HREF="' . $CGI_HOME . 'cometresult.cgi?TarFile={$basename}.cmt.tar.gz&amp;File=./{$xpress_spec}.cmt"><xsl:value-of select="@spectrum"/></A></xsl:when><xsl:otherwise><xsl:value-of select="@spectrum"/></xsl:otherwise></xsl:choose>
  sprintf(spectrum,  "%s%s%s%s%s%s%s%s%s", "<td><xsl:choose><xsl:when test=\"parent::node()/pepx:search_summary/@search_engine='SEQUEST'\"><A TARGET=\"Win3\" HREF=\"", cgihome_, "sequest-tgz-out.cgi?OutFile={$basename}/{$xpress_spec}.out\"><xsl:value-of select=\"@spectrum\"/></A></xsl:when><xsl:when test=\"parent::node()/pepx:search_summary/@search_engine='MASCOT'\"><A TARGET=\"Win3\" HREF=\"", cgihome_, "mascotout.pl?OutFile={$basename}/{$xpress_spec}.out\"><xsl:value-of select=\"@spectrum\"/></A></xsl:when><xsl:when test=\"parent::node()/pepx:search_summary/@search_engine='COMET'\"><A TARGET=\"Win3\" HREF=\"", cgihome_, "cometresult.cgi?TarFile={$basename}.cmt.tar.gz&amp;File=./{$xpress_spec}.cmt\"><xsl:value-of select=\"@spectrum\"/></A></xsl:when><xsl:otherwise><xsl:value-of select=\"@spectrum\"/></xsl:otherwise></xsl:choose>", table_spacer, "</td>");


  //<A TARGET=\"Win1\" HREF=\"", cgihome_, "sequest-tgz-out.cgi?OutFile={$basename}/{$xpress_spec}.out\"><xsl:value-of select=\"@spectrum\"/></A>", table_spacer, "</td>");




  // sprintf(spectrum,  "%s%s%s%s%s", "<td><A TARGET=\"Win1\" HREF=\"", cgihome_, "sequest-tgz-out.cgi?OutFile={$basename}/{$xpress_spec}.out\"><xsl:value-of select=\"@spectrum\"/></A>", table_spacer, "</td>");

 //sprintf(spectrum, "%s%s%s", "<td><xsl:value-of select=\"@spectrum\"/>", table_spacer, "</td>");
 //  sprintf(peptide_sequence, "%s%s%s", "<td><xsl:if test=\"search_hit[@hit_rank='1']/@peptide_prev_aa\"><xsl:value-of select=\"search_hit[@hit_rank='1']/@peptide_prev_aa\"/>.</xsl:if><xsl:value-of select=\"search_hit[@hit_rank='1']/@peptide\"/><xsl:if test=\"search_hit[@hit_rank='1']/@peptide_next_aa\">.<xsl:value-of select=\"search_hit[@hit_rank='1']/@peptide_next_aa\"/></xsl:if>", table_spacer, "</td>");

  //my $pep_ref = '<A TARGET="Win1" HREF="http://www.ncbi.nlm.nih.gov/blast/Blast.cgi?CMD=Web&amp;LAYOUT=TwoWindows&amp;AUTO_FORMAT=Semiauto&amp;ALIGNMENTS=50&amp;ALIGNMENT_VIEW=Pairwise&amp;CDD_SEARCH=on&amp;CLIENT=web&amp;COMPOSITION_BASED_STATISTICS=on&amp;DATABASE=nr&amp;DESCRIPTIONS=100&amp;ENTREZ_QUERY=(none)&amp;EXPECT=1000&amp;FILTER=L&amp;FORMAT_OBJECT=Alignment&amp;FORMAT_TYPE=HTML&amp;I_THRESH=0.005&amp;MATRIX_NAME=BLOSUM62&amp;NCBI_GI=on&amp;PAGE=Proteins&amp;PROGRAM=blastp&amp;SERVICE=plain&amp;SET_DEFAULTS.x=41&amp;SET_DEFAULTS.y=5&amp;SHOW_OVERVIEW=on&amp;END_OF_HTTPGET=Yes&amp;SHOW_LINKOUT=yes&amp;QUERY={$Peptide}">';

//$display{'peptide_sequence'} = '<td><xsl:if test="search_hit[@hit_rank=\'1\']/@peptide_prev_aa"><xsl:value-of select="search_hit[@hit_rank=\'1\']/@peptide_prev_aa"/>.</xsl:if>' . $pep_ref . '<xsl:value-of select="search_hit[@hit_rank=\'1\']/@peptide"/></A><xsl:if test="search_hit[@hit_rank=\'1\']/@peptide_next_aa">.<xsl:value-of select="search_hit[@hit_rank=\'1\']/@peptide_next_aa"/></xsl:if>' . $table_spacer . '</td>';


#ifdef USE_STD_MODS
 sprintf(peptide_sequence, "%s%s%s", "<td><xsl:if test=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/@peptide_prev_aa\"><xsl:value-of select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/@peptide_prev_aa\"/>.</xsl:if><A TARGET=\"Win1\" HREF=\"http://www.ncbi.nlm.nih.gov/blast/Blast.cgi?CMD=Web&amp;LAYOUT=TwoWindows&amp;AUTO_FORMAT=Semiauto&amp;ALIGNMENTS=50&amp;ALIGNMENT_VIEW=Pairwise&amp;CDD_SEARCH=on&amp;CLIENT=web&amp;COMPOSITION_BASED_STATISTICS=on&amp;DATABASE=nr&amp;DESCRIPTIONS=100&amp;ENTREZ_QUERY=(none)&amp;EXPECT=1000&amp;FILTER=L&amp;FORMAT_OBJECT=Alignment&amp;FORMAT_TYPE=HTML&amp;I_THRESH=0.005&amp;MATRIX_NAME=BLOSUM62&amp;NCBI_GI=on&amp;PAGE=Proteins&amp;PROGRAM=blastp&amp;SERVICE=plain&amp;SET_DEFAULTS.x=41&amp;SET_DEFAULTS.y=5&amp;SHOW_OVERVIEW=on&amp;END_OF_HTTPGET=Yes&amp;SHOW_LINKOUT=yes&amp;QUERY={$StrippedPeptide}\"><xsl:if test=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:modification_info\"><xsl:if test=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:modification_info/@modified_peptide\"><xsl:value-of select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:modification_info/@modified_peptide\"/></xsl:if></xsl:if><xsl:if test=\"not(pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:modification_info)\"><xsl:value-of select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/@peptide\"/></xsl:if></A><xsl:if test=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/@peptide_next_aa\">.<xsl:value-of select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/@peptide_next_aa\"/></xsl:if>", table_spacer, "</td>");
#endif

#ifndef USE_STD_MODS
 sprintf(peptide_sequence, "%s%s%s", "<td><xsl:if test=\"search_hit[@hit_rank='1']/@peptide_prev_aa\"><xsl:value-of select=\"search_hit[@hit_rank='1']/@peptide_prev_aa\"/>.</xsl:if><A TARGET=\"Win1\" HREF=\"http://www.ncbi.nlm.nih.gov/blast/Blast.cgi?CMD=Web&amp;LAYOUT=TwoWindows&amp;AUTO_FORMAT=Semiauto&amp;ALIGNMENTS=50&amp;ALIGNMENT_VIEW=Pairwise&amp;CDD_SEARCH=on&amp;CLIENT=web&amp;COMPOSITION_BASED_STATISTICS=on&amp;DATABASE=nr&amp;DESCRIPTIONS=100&amp;ENTREZ_QUERY=(none)&amp;EXPECT=1000&amp;FILTER=L&amp;FORMAT_OBJECT=Alignment&amp;FORMAT_TYPE=HTML&amp;I_THRESH=0.005&amp;MATRIX_NAME=BLOSUM62&amp;NCBI_GI=on&amp;PAGE=Proteins&amp;PROGRAM=blastp&amp;SERVICE=plain&amp;SET_DEFAULTS.x=41&amp;SET_DEFAULTS.y=5&amp;SHOW_OVERVIEW=on&amp;END_OF_HTTPGET=Yes&amp;SHOW_LINKOUT=yes&amp;QUERY={$StrippedPeptide}\"><xsl:value-of select=\"search_hit[@hit_rank='1']/@peptide\"/></A><xsl:if test=\"search_hit[@hit_rank='1']/@peptide_next_aa\">.<xsl:value-of select=\"search_hit[@hit_rank='1']/@peptide_next_aa\"/></xsl:if>", table_spacer, "</td>");
#endif


  sprintf(protein, "%s%s%s", "<td><xsl:value-of select=\"pepx:search_result/pepx:search_hit[@hit_rank = '1']/@protein\"/><xsl:for-each select=\"pepx:search_result/pepx:search_hit[@hit_rank = '1']/pepx:alternative_protein\"><xsl:text> </xsl:text><xsl:value-of select=\"@protein\"/></xsl:for-each>", table_spacer, "</td>");
  //sprintf(probability, "%s%s%s%s%s%s%s%s%s", "<td><xsl:if test=\"search_hit[@hit_rank='1']/peptideprophet_result\"><A TARGET=\"Win1\" HREF=\"", cgihome_, "ModelParser.cgi?Xmlfile=", xmlfile, "&amp;Timestamp={$pepproph_timestamp}&amp;Spectrum={$xpress_spec}&amp;Scores={$scores}&amp;Prob={$prob}\"><xsl:value-of select=\"search_hit[@hit_rank='1']/peptideprophet_result/@probability\"/></A></xsl:if><xsl:if test=\"not(search_hit[@hit_rank='1']/peptideprophet_result)\">N_A</xsl:if>", table_spacer, "</td>");
  sprintf(probability, "%s%s%s%s%s%s%s", "<td><xsl:if test=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:analysis_result[@analysis='peptideprophet']\"><A TARGET=\"Win1\" HREF=\"", cgihome_, "ModelParser.cgi?Xmlfile=", "{$summaryxml}", "&amp;Timestamp={$pepproph_timestamp}&amp;Spectrum={$xpress_spec}&amp;Scores={$scores}&amp;Prob={$prob}\"><xsl:if test=\"pepx:search_result/search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='peptideprophet']/pepx:peptideprophet_result/@analysis and pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='peptideprophet']/pepx:peptideprophet_result/@analysis=\'adjusted\'\"><font color=\"#FF00FF\"><xsl:value-of select=\"pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='peptideprophet']/pepx:peptideprophet_result/@probability\"/></font></xsl:if><xsl:if test=\"not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='peptideprophet']/pepx:peptideprophet_result/@analysis) or not(pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='peptideprophet']/pepx:peptideprophet_result/@analysis=\'adjusted\')\"><xsl:value-of select=\"pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='peptideprophet']/pepx:peptideprophet_result/@probability\"/></xsl:if></A></xsl:if><xsl:if test=\"not(pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:analysis_result[@analysis='peptideprophet'])\">N_A</xsl:if>", table_spacer, "</td>");

  //<xsl:value-of select=\"search_hit[@hit_rank='1']/peptideprophet_result/@probability\"/>

  //</A></xsl:if><xsl:if test=\"not(search_hit[@hit_rank='1']/peptideprophet_result)\">N_A</xsl:if>", table_spacer, "</td>");




  // quant
  if(heavy2light) {
    sprintf(xpress,"%s%s%s%s%s%s%s", "<td><xsl:if test=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:analysis_result[@analysis='xpress']\"><A TARGET=\"Win1\" HREF=\"", cgihome_, "XPressPeptideUpdateParser.cgi?LightFirstScan={$light_first_scan}&amp;LightLastScan={$light_last_scan}&amp;HeavyFirstScan={$heavy_first_scan}&amp;HeavyLastScan={$heavy_last_scan}&amp;XMLFile={$basename}.mzXML&amp;ChargeState={$xpress_charge}&amp;LightMass={$LightMass}&amp;HeavyMass={$HeavyMass}&amp;MassTol={$MassTol}&amp;PpmTol={$PpmTol}&amp;index={$xpress_index}&amp;xmlfile=", xmlfile, "&amp;bXpressLight1={$xpress_display}&amp;OutFile={$xpress_spec}\"><nobr><xsl:value-of select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:analysis_result[@analysis='xpress']/pepx:xpressratio_result/@heavy2light_ratio\"/></nobr></A></xsl:if>", table_spacer, "</td>");
    sprintf(asap_ref, "%s%s%s%s%s%d%s", "<A TARGET=\"Win1\" HREF=\"", cgihome_, "ASAPRatioPeptideCGIDisplayParser.cgi?Xmlfile=", xmlfile, "&amp;Basename={$basename}&amp;Indx={$xpress_index}&amp;Timestamp={$asap_time}&amp;Spectrum={$xpress_spec}&amp;ratioType=", 1, "&amp;quantHighBG={$asap_quantHighBG}&amp;zeroBG={$asap_zeroBG}&amp;mzBound={$asap_mzBound}\">");
    sprintf(asapratio, "%s%s%s%s%s%s%s", "<xsl:if test=\"/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis='asapratio']\"><td>", asap_ref, "<xsl:if test=\"pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='asapratio']/pepx:asapratio_result/@mean&lt;\'0\'\">N_A</xsl:if><xsl:if test=\"pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='asapratio']/pepx:asapratio_result/@mean&gt;\'-1\'\"><xsl:choose><xsl:when test=\"pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='asapratio']/pepx:asapratio_result/@mean=\'0\' or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='asapratio']/pepx:asapratio_result/@heavy2light_mean=\'999\' or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='asapratio']/pepx:asapratio_result/@heavy2light_error &gt; 0.5 * pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='asapratio']/pepx:asapratio_result/@heavy2light_mean\"><font color=\"red\"><nobr><xsl:value-of select=\"pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='asapratio']/pepx:asapratio_result/@heavy2light_mean\"/><xsl:text> </xsl:text>", plusmn, "<xsl:text> </xsl:text><xsl:value-of select=\"pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='asapratio']/pepx:asapratio_result/@heavy2light_error\"/></nobr></font></xsl:when><xsl:otherwise><nobr><xsl:value-of select=\"pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='asapratio']/pepx:asapratio_result/@heavy2light_mean\"/><xsl:text> </xsl:text>", plusmn, "<xsl:text> </xsl:text><xsl:value-of select=\"pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='asapratio']/pepx:asapratio_result/@heavy2light_error\"/></nobr></xsl:otherwise></xsl:choose></xsl:if></A></td></xsl:if>");
  }
  else {
    sprintf(xpress,"%s%s%s%s%s%s%s", "<td><xsl:if test=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:analysis_result[@analysis='xpress']\"><A TARGET=\"Win1\" HREF=\"", cgihome_, "XPressPeptideUpdateParser.cgi?LightFirstScan={$light_first_scan}&amp;LightLastScan={$light_last_scan}&amp;HeavyFirstScan={$heavy_first_scan}&amp;HeavyLastScan={$heavy_last_scan}&amp;XMLFile={$basename}.mzXML&amp;ChargeState={$xpress_charge}&amp;LightMass={$LightMass}&amp;HeavyMass={$HeavyMass}&amp;MassTol={$MassTol}&amp;PpmTol={$PpmTol}&amp;index={$xpress_index}&amp;xmlfile=", xmlfile, "&amp;bXpressLight1={$xpress_display}&amp;OutFile={$xpress_spec}\"><nobr><xsl:value-of select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:analysis_result[@analysis='xpress']/pepx:xpressratio_result/@ratio\"/></nobr></A></xsl:if>", table_spacer, "</td>");
    sprintf(asap_ref, "%s%s%s%s%s%d%s", "<A TARGET=\"Win1\" HREF=\"", cgihome_, "ASAPRatioPeptideCGIDisplayParser.cgi?Xmlfile=", xmlfile, "&amp;Basename={$basename}&amp;Indx={$xpress_index}&amp;Timestamp={$asap_time}&amp;Spectrum={$xpress_spec}&amp;ratioType=", 0, "&amp;quantHighBG={$asap_quantHighBG}&amp;zeroBG={$asap_zeroBG}&amp;mzBound={$asap_mzBound}\">");
    sprintf(asapratio, "%s%s%s%s%s%s%s", "<xsl:if test=\"/pepx:msms_pipeline_analysis/pepx:analysis_summary[@analysis='asapratio']\"><td>", asap_ref, "<xsl:if test=\"pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='asapratio']/pepx:asapratio_result/@mean&lt;\'0\'\">N_A</xsl:if><xsl:if test=\"pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='asapratio']/pepx:asapratio_result/@mean&gt;\'-1\'\"><xsl:choose><xsl:when test=\"pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='asapratio']/pepx:asapratio_result/@mean=\'0\' or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='asapratio']/pepx:asapratio_result/@mean=\'999\' or pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='asapratio']/pepx:asapratio_result/@error &gt; 0.5 * pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='asapratio']/pepx:asapratio_result/@mean\"><font color=\"red\"><nobr><xsl:value-of select=\"pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='asapratio']/pepx:asapratio_result/@mean\"/><xsl:text> </xsl:text>", plusmn, "<xsl:text> </xsl:text><xsl:value-of select=\"pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='asapratio']/pepx:asapratio_result/@error\"/></nobr></font></xsl:when><xsl:otherwise><nobr><xsl:value-of select=\"pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='asapratio']/pepx:asapratio_result/@mean\"/><xsl:text> </xsl:text>", plusmn, "<xsl:text> </xsl:text><xsl:value-of select=\"pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='asapratio']/pepx:asapratio_result/@error\"/></nobr></xsl:otherwise></xsl:choose></xsl:if></A></td></xsl:if>");


  }

  // sprintf(ions, "%s%s%s%s%s", "<td><A TARGET=\"Win1\" HREF=\"", cgihome_, "sequest-tgz-plot.cgi?MassType={$masstype}&amp;NumAxis=1&amp;Pep={$Peptide}&amp;Dta={$basename}/{$xpress_spec}.dta\"><nobr><xsl:value-of select=\"search_hit[@hit_rank='1']/@num_matched_ions\"/>/<xsl:value-of select=\"search_hit[@hit_rank='1']/@tot_num_ions\"/></nobr></A>", table_spacer, "</td>");


 sprintf(ions, "%s%s%s%s%s%s%s", "<td><xsl:choose><xsl:when test=\"parent::node()/pepx:search_summary/@search_engine='COMET'\"><A TARGET=\"Win3\" HREF=\"", cgihome_, "plot-msms.cgi?MassType={$masstype}&amp;NumAxis=1&amp;{$PeptideMods2}Pep={$StrippedPeptide}&amp;Dta={$basename}/{$xpress_spec}.dta&amp;COMET=1\"><nobr><xsl:value-of select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/@num_matched_ions\"/>/<xsl:value-of select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/@tot_num_ions\"/></nobr></A></xsl:when><xsl:otherwise><A TARGET=\"Win3\" HREF=\"", cgihome_, "plot-msms.cgi?MassType={$masstype}&amp;NumAxis=1&amp;{$PeptideMods2}Pep={$StrippedPeptide}&amp;Dta={$basename}/{$xpress_spec}.dta\"><nobr><xsl:value-of select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/@num_matched_ions\"/>/<xsl:value-of select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/@tot_num_ions\"/></nobr></A></xsl:otherwise></xsl:choose>", table_spacer, "</td>");


 // sprintf(ions, "%s%s%s%s%s%s%s", "<td><xsl:choose><xsl:when test=\"parent::node()/@search_engine='COMET'\"><A TARGET=\"Win3\" HREF=\"", cgihome_, "cometplot.cgi?TarFile={$basename}.cmt.tar.gz&amp;File=./{$xpress_spec}.dta&amp;Xmin=0&amp;Xmax=0&amp;Ymin=2&amp;Ymax=3&amp;LabelType=0&amp;NumAxis=1&amp;Pep={$StrippedPeptide}&amp;ConfigFile=comet.def&amp;MD5={$comet_md5_check_sum}&amp;PepMass={$pep_mass}&amp;ShowB=1&amp;ShowY=1&amp;AAMods={$aa_mods}&amp;TerminalMods={$term_mods}\"><nobr><xsl:value-of select=\"search_hit[@hit_rank='1']/@num_matched_ions\"/>/<xsl:value-of select=\"search_hit[@hit_rank='1']/@tot_num_ions\"/></nobr></A></xsl:when><xsl:otherwise><A TARGET=\"Win1\" HREF=\"", cgihome_, "sequest-tgz-plot.cgi?MassType={$masstype}&amp;NumAxis=1&amp;Pep={$StrippedPeptide}&amp;Dta={$basename}/{$xpress_spec}.dta\"><nobr><xsl:value-of select=\"search_hit[@hit_rank='1']/@num_matched_ions\"/>/<xsl:value-of select=\"search_hit[@hit_rank='1']/@tot_num_ions\"/></nobr></A></xsl:otherwise></xsl:choose>", table_spacer, "</td>");

 //<A TARGET=\"Win1\" HREF=\"", cgihome_, "sequest-tgz-plot.cgi?MassType={$masstype}&amp;NumAxis=1&amp;Pep={$Peptide}&amp;Dta={$basename}/{$xpress_spec}.dta\"><nobr><xsl:value-of select=\"search_hit[@hit_rank='1']/@num_matched_ions\"/>/<xsl:value-of select=\"search_hit[@hit_rank='1']/@tot_num_ions\"/></nobr></A>", table_spacer, "</td>");





 //$display{'ions'} = '<td>' . $ions_ref . '<nobr><xsl:value-of select="search_hit[@hit_rank=\'1\']/@num_matched_ions"/>/<xsl:value-of select="search_hit[@hit_rank=\'1\']/@tot_num_ions"/></nobr></A>' . $table_spacer . '</td>';
 
  //my $xpress_ref = '<A TARGET="Win1" HREF="' . $CGI_HOME . 'XPressPeptideUpdateParser.cgi?LightFirstScan={$light_first_scan}&amp;LightLastScan={$light_last_scan}&amp;HeavyFirstScan={$heavy_first_scan}&amp;HeavyLastScan={$heavy_last_scan}&amp;XMLFile={$basename}.mzXML&amp;ChargeState={$xpress_charge}&amp;LightMass={$LightMass}&amp;HeavyMass={$HeavyMass}&amp;MassTol={$MassTol}&amp;index={$xpress_index}&amp;xmlfile=' . $xmlfile . '&amp;bXpressLight1=0&amp;OutFile={$xpress_spec}">';


  //$display{'xpress'} = '<td><xsl:if test="search_hit[@hit_rank=\'1\']/xpressratio_result">' . $xpress_ref . '<nobr><xsl:value-of select="search_hit[@hit_rank=\'1\']/xpressratio_result/@ratio"/></nobr></A></xsl:if></td>';


  fprintf(fp, "<?xml version=\"1.0\"?>\n");
  fprintf(fp, "<xsl:stylesheet version=\"1.0\" xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\" xmlns:pepx=\"%s\">\n",PEPXML_NAMESPACE);

  // fill in the rest (copies from pepxml2html.pl)
  fprintf(fp, "<xsl:key name=\"search_engine\" match=\"/pepx:msms_pipeline_analysis//pepx:msms_run_summary/pepx:search_summary/@search_engine\" use=\".\"/>");

  fprintf(fp, "<xsl:template match=\"pepx:msms_pipeline_analysis\">\n");

  // put the header here.......
  fprintf(fp, "<tr>");
  fprintf(fp, "%s%s%s", "<td><font color=\"brown\"><b>index</b></font>", table_spacer, "</td>");
  fprintf(fp, "%s%s%s", "<td><font color=\"brown\"><b>prob</b></font>", table_spacer, "</td>");
  fprintf(fp, "%s%s%s", "<td><font color=\"brown\"><b>spectrum</b></font>", table_spacer, "</td>");
  fprintf(fp, "%s%s%s", "<td><font color=\"brown\"><b>m ions</b></font>", table_spacer, "</td>");
  fprintf(fp, "%s%s%s", "<td><font color=\"brown\"><b>peptide</b></font>", table_spacer, "</td>");

  // quantitation
  if(heavy2light) {
    fprintf(fp, "%s%s%s", "<td><font color=\"brown\"><b>(H/L) XPRESS</b></font>", table_spacer, "</td>");
    fprintf(fp, "%s%s%s", "<xsl:if test=\"/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:analysis_timestamp[@analysis='asapratio']\"><td><font color=\"brown\"><b>(H/L) ASAPRatio</b></font>", table_spacer, "</td></xsl:if>");
  }
  else {
    fprintf(fp, "%s%s%s", "<td><font color=\"brown\"><b>XPRESS</b></font>", table_spacer, "</td>");
    fprintf(fp, "%s%s%s", "<xsl:if test=\"/pepx:msms_pipeline_analysis/pepx:msms_run_summary/pepx:analysis_timestamp[@analysis='asapratio']\"><td><font color=\"brown\"><b>ASAPRatio</b></font>", table_spacer, "</td></xsl:if>");
  }

  fprintf(fp, "</tr>");



  fprintf(fp, "<xsl:apply-templates select=\"pepx:msms_run_summary/pepx:spectrum_query\">\n");

  fprintf(fp, "<xsl:sort select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/@peptide\"/>");
  fprintf(fp, "</xsl:apply-templates>");

  fprintf(fp, "</xsl:template>\n");
  
  fprintf(fp, "<xsl:template match=\"pepx:spectrum_query\">\n");

  fprintf(fp, "<xsl:if test=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/@peptide='%s'", peptides_->getNthCharptr(0));
  for (int k=1;k<peptides_->size();k++)
    fprintf(fp, " or pepx:search_result/pepx:search_hit[@hit_rank='1']/@peptide='%s'", peptides_->getNthCharptr(k));
  fprintf(fp, "\">\n");
  if(minpepprob_ > 0.0)
    fprintf(fp, "<xsl:if test=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:analysis_result[@analysis='peptideprophet']/pepx:peptideprophet_result/@probability &gt;='%0.2f'\">", minpepprob_);

  //  fprintf(fp, "<xsl:variable name=\"light_first_scan\"><xsl:value-of select=\"search_hit[@hit_rank='1']/xpressratio_result/@light_firstscan\"/></xsl:variable>");
  fprintf(fp, "<xsl:variable name=\"light_first_scan\" select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:analysis_result[@analysis='xpress']/pepx:xpressratio_result/@light_firstscan\"/>");

  //  fprintf(fp, "<xsl:variable name=\"light_last_scan\"><xsl:value-of select=\"search_hit[@hit_rank='1']/xpressratio_result/@light_lastscan\"/></xsl:variable>");
  fprintf(fp, "<xsl:variable name=\"light_last_scan\" select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:analysis_result[@analysis='xpress']/pepx:xpressratio_result/@light_lastscan\"/>");

  //  fprintf(fp, "<xsl:variable name=\"heavy_first_scan\"><xsl:value-of select=\"search_hit[@hit_rank='1']/xpressratio_result/@heavy_firstscan\"/></xsl:variable>");
  fprintf(fp, "<xsl:variable name=\"heavy_first_scan\" select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:analysis_result[@analysis='xpress']/pepx:xpressratio_result/@heavy_firstscan\"/>");

  //  fprintf(fp, "<xsl:variable name=\"heavy_last_scan\"><xsl:value-of select=\"search_hit[@hit_rank='1']/xpressratio_result/@heavy_lastscan\"/></xsl:variable>");
  fprintf(fp, "<xsl:variable name=\"heavy_last_scan\" select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:analysis_result[@analysis='xpress']/pepx:xpressratio_result/@heavy_lastscan\"/>");

  //  fprintf(fp, "<xsl:variable name=\"basename\"><xsl:value-of select=\"parent::node()/@base_name\"/></xsl:variable>");
  fprintf(fp, "<xsl:variable name=\"basename\" select=\"parent::node()/@base_name\"/>");

  //  fprintf(fp, "<xsl:variable name=\"xpress_charge\"><xsl:value-of select=\"@assumed_charge\"/></xsl:variable>");
  fprintf(fp, "<xsl:variable name=\"xpress_charge\" select=\"@assumed_charge\"/>");

  //  fprintf(fp, "<xsl:variable name=\"xpress_display\"><xsl:value-of select=\"parent::node()/xpressratio_timestamp/@display_ref\"/></xsl:variable>");
  //  fprintf(fp, "<xsl:variable name=\"xpress_display\" select=\"parent::node()/xpressratio_timestamp/@display_ref\"/>");
  fprintf(fp, "<xsl:variable name=\"xpress_display\" select=\"parent::node()/pepx:analysis_timestamp[@analysis='xpress']/pepx:xpressratio_timestamp/@xpress_light\"/>");

  //  fprintf(fp, "<xsl:variable name=\"LightMass\"><xsl:value-of select=\"search_hit[@hit_rank='1']/xpressratio_result/@light_mass\"/></xsl:variable>");
  fprintf(fp, "<xsl:variable name=\"LightMass\" select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:analysis_result[@analysis='xpress']/pepx:xpressratio_result/@light_mass\"/>");

  //  fprintf(fp, "<xsl:variable name=\"HeavyMass\"><xsl:value-of select=\"search_hit[@hit_rank='1']/xpressratio_result/@heavy_mass\"/></xsl:variable>");
  fprintf(fp, "<xsl:variable name=\"HeavyMass\" select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:analysis_result[@analysis='xpress']/pepx:xpressratio_result/@heavy_mass\"/>");

  //  fprintf(fp, "<xsl:variable name=\"MassTol\"><xsl:value-of select=\"search_hit[@hit_rank='1']/xpressratio_result/@mass_tol\"/></xsl:variable>");
  fprintf(fp, "<xsl:variable name=\"MassTol\" select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:analysis_result[@analysis='xpress']/pepx:xpressratio_result/@mass_tol\"/>");

  fprintf(fp, "<xsl:variable name=\"PpmTol\" select=\"parent::node()/parent::node()/pepx:analysis_summary[@analysis='xpress']/pepx:xpressratio_summary/@ppmtol\"/>");

  //  fprintf(fp, "<xsl:variable name=\"summaryxml\"><xsl:value-of select=\"/msms_pipeline_analysis/@summary_xml\"/></xsl:variable>");
  fprintf(fp, "<xsl:variable name=\"summaryxml\" select=\"/pepx:msms_pipeline_analysis/@summary_xml\"/>");

  //  fprintf(fp, "<xsl:variable name=\"xpress_index\"><xsl:value-of select=\"@index\"/></xsl:variable>");
  fprintf(fp, "<xsl:variable name=\"xpress_index\" select=\"@index\"/>");

  //  fprintf(fp, "<xsl:variable name=\"xpress_spec\"><xsl:value-of select=\"@spectrum\"/></xsl:variable>");
  fprintf(fp, "<xsl:variable name=\"xpress_spec\" select=\"@spectrum\"/>");
  //  fprintf(fp, "<xsl:variable name=\"asap_index\"><xsl:value-of select=\"search_hit[@hit_rank=\'1\']/asapratio_result/@index\"/></xsl:variable>");
  //  fprintf(fp, "<xsl:variable name=\"asap_time\"><xsl:value-of select=\"parent::node()/asapratio_timestamp/@time\"/></xsl:variable>");
  //  fprintf(fp, "<xsl:variable name=\"asap_time\" select=\"parent::node()/parent::node()/pepx:analysis_summary[@analysis='asapratio']/@time\"/>");
  fprintf(fp, "<xsl:variable name=\"asap_time\" select=\"parent::node()/pepx:analysis_timestamp[@analysis='asapratio']/@time\"/>");
  //fprintf(fp, "<xsl:variable name=\"asap_quantHighBG\" select=\"parent::node()/parent::node()/pepx:analysis_summary[@analysis='asapratio']/pepx:parameter[@name=quantHighBG]/@value\"/>");
  //fprintf(fp, "<xsl:variable name=\"asap_zeroBG\" select=\"parent::node()/parent::node()/pepx:analysis_summary[@analysis='asapratio']/pepx:parameter[@name=zeroBG]/@value\"/>");
  //fprintf(fp, "<xsl:variable name=\"asap_mzBound\" select=\"parent::node()/parent::node()/pepx:analysis_summary[@analysis='asapratio']/pepx:parameter[@name=mzBound]/@value\"/>");

  fprintf(fp, "<xsl:variable name=\"asap_quantHighBG\">");
  fprintf(fp, "			<xsl:choose>");
  fprintf(fp, "				<xsl:when test=\"parent::node()/parent::node()/pepx:analysis_summary[@analysis='asapratio']/pepx:parameter[@name='quantHighBG']\">");
  fprintf(fp, "			                      <xsl:choose>");
  fprintf(fp, "				                      <xsl:when test=\"parent::node()/parent::node()/pepx:analysis_summary[@analysis='asapratio']/pepx:parameter[@value='True']\">");
  fprintf(fp, "					                       <xsl:value-of select=\"1\"/>");
  fprintf(fp, "				                      </xsl:when>");
  fprintf(fp, "				                      <xsl:otherwise>");
  fprintf(fp, "					                       <xsl:value-of select=\"0\"/>");
  fprintf(fp, "				                      </xsl:otherwise>");
  fprintf(fp, "			                       </xsl:choose>"); 
  fprintf(fp, "				</xsl:when>");
  fprintf(fp, "				<xsl:otherwise>");
  fprintf(fp, "					<xsl:value-of select=\"0\"/>");
  fprintf(fp, "				</xsl:otherwise>");
  fprintf(fp, "			</xsl:choose>");
  fprintf(fp, "		</xsl:variable>");

  fprintf(fp, "<xsl:variable name=\"asap_zeroBG\">");
  fprintf(fp, "			<xsl:choose>");
  fprintf(fp, "				<xsl:when test=\"parent::node()/parent::node()/pepx:analysis_summary[@analysis='asapratio']/pepx:parameter[@name='zeroBG']\">");
  fprintf(fp, "			                      <xsl:choose>");
  fprintf(fp, "				                      <xsl:when test=\"parent::node()/parent::node()/pepx:analysis_summary[@analysis='asapratio']/pepx:parameter[@value='True']\">");
  fprintf(fp, "					                       <xsl:value-of select=\"1\"/>");
  fprintf(fp, "				                      </xsl:when>");
  fprintf(fp, "				                      <xsl:otherwise>");
  fprintf(fp, "					                       <xsl:value-of select=\"0\"/>");
  fprintf(fp, "				                      </xsl:otherwise>");
  fprintf(fp, "			                       </xsl:choose>"); 
  fprintf(fp, "				</xsl:when>");
  fprintf(fp, "				<xsl:otherwise>");
  fprintf(fp, "					<xsl:value-of select=\"0\"/>");
  fprintf(fp, "				</xsl:otherwise>");
  fprintf(fp, "			</xsl:choose>");
  fprintf(fp, "		</xsl:variable>");
 
  fprintf(fp, "<xsl:variable name=\"asap_mzBound\">");
  fprintf(fp, "			<xsl:choose>");
  fprintf(fp, "				<xsl:when test=\"parent::node()/parent::node()/pepx:analysis_summary[@analysis='asapratio']/pepx:parameter[@name='mzBound']\">");
  fprintf(fp, "				       <xsl:value-of select=\"parent::node()/parent::node()/pepx:analysis_summary[@analysis='asapratio']/pepx:parameter[@name='mzBound']/@value\"/>");
  fprintf(fp, "				</xsl:when>");
  fprintf(fp, "				<xsl:otherwise>");
  fprintf(fp, "					<xsl:value-of select=\"0.5\"/>");
  fprintf(fp, "				</xsl:otherwise>");
  fprintf(fp, "			</xsl:choose>");
  fprintf(fp, "		</xsl:variable>");


  //  fprintf(fp, "<xsl:variable name=\"pepproph_timestamp\"><xsl:value-of select=\"parent::node()/peptideprophet_timestamp/@time\"/></xsl:variable>");
  fprintf(fp, "<xsl:variable name=\"pepproph_timestamp\" select=\"parent::node()/pepx:analysis_timestamp[@analysis='peptideprophet']/@time\"/>");

  //  fprintf(fp, "<xsl:variable name=\"Peptide\"><xsl:value-of select=\"search_hit[@hit_rank='1']/@peptide\"/></xsl:variable>");
  fprintf(fp, "<xsl:variable name=\"Peptide\" select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/@peptide\"/>");
#ifdef USE_STD_MODS
  //  fprintf(fp, "<xsl:variable name=\"StrippedPeptide\"><xsl:value-of select=\"search_hit[@hit_rank='1']/@peptide\"/></xsl:variable>");
  fprintf(fp, "<xsl:variable name=\"StrippedPeptide\" select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/@peptide\"/>");
  fprintf(fp, "<xsl:variable name=\"PeptideMods2\"><xsl:if test=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:modification_info\"><xsl:if test=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:modification_info/@mod_nterm_mass\">ModN=<xsl:value-of select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:modification_info/@mod_nterm_mass\"/>&amp;</xsl:if><xsl:if test=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:modification_info/@mod_cterm_mass\">ModC=<xsl:value-of select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:modification_info/@mod_cterm_mass\"/>&amp;</xsl:if><xsl:for-each select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:modification_info/pepx:mod_aminoacid_mass\">Mod<xsl:value-of select=\"@position\"/>=<xsl:value-of select=\"@mass\"/>&amp;</xsl:for-each></xsl:if></xsl:variable>");


#endif
#ifndef USE_STD_MODS
  //  fprintf(fp, "<xsl:variable name=\"StrippedPeptide\"><xsl:value-of select=\"search_hit[@hit_rank='1']/@stripped_peptide\"/></xsl:variable>");
  fprintf(fp, "<xsl:variable name=\"StrippedPeptide\" select=\"pepx:search_hit[@hit_rank='1']/@stripped_peptide\"/>");
#endif
  fprintf(fp, "<xsl:variable name=\"prob\"><xsl:value-of select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:analysis_result[@analysis='peptideprophet']/pepx:peptideprophet_result/@probability\"/><xsl:if test=\"pepx:search_result/pepx:search_hit[@hit_rank=\'1\']/pepx:analysis_result[@analysis='peptideprophet']/pepx:peptideprophet_result/@analysis='adjusted'\">a</xsl:if></xsl:variable>");

 fprintf(fp, "<xsl:variable name=\"masstype\"><xsl:if test=\"parent::node()/pepx:search_summary/@precursor_mass_type='average'\">0</xsl:if><xsl:if test=\"not(parent::node()/search_summary/@precursor_mass_type='average')\">1</xsl:if></xsl:variable>");
 fprintf(fp, "<xsl:variable name=\"fragmasstype\"><xsl:if test=\"parent::node()/pepx:search_summary/@fragment_mass_type='average'\">0</xsl:if><xsl:if test=\"not(parent::node()/search_summary/@fragment_mass_type='average')\">1</xsl:if></xsl:variable>");
 fprintf(fp, "<xsl:variable name=\"comet_md5_check_sum\"><xsl:if test=\"parent::node()/pepx:search_summary/@search_engine='COMET'\"><xsl:value-of select=\"parent::node()/pepx:search_summary/pepx:parameter[@name='md5_check_sum']/@value\"/></xsl:if></xsl:variable>");

 // fprintf(fp, "<xsl:variable name=\"pep_mass\"><xsl:value-of select=\"search_hit[@hit_rank='1']/@calc_neutral_pep_mass\"/></xsl:variable>");
 fprintf(fp, "<xsl:variable name=\"pep_mass\" select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/@calc_neutral_pep_mass\"/>");

 fprintf(fp, "<xsl:variable name=\"aa_mods\"><xsl:for-each select=\"parent::node()/pepx:search_summary/pepx:aminoacid_modification\"><xsl:value-of select=\"@aminoacid\"/><xsl:if test=\"@symbol\"><xsl:value-of select=\"@symbol\"/></xsl:if>-<xsl:value-of select=\"@mass\"/>:</xsl:for-each></xsl:variable>");
 fprintf(fp, "<xsl:variable name=\"term_mods\"><xsl:for-each select=\"parent::node()/pepx:search_summary/pepx:terminal_modification\"><xsl:value-of select=\"@terminus\"/><xsl:if test=\"@symbol\"><xsl:value-of select=\"@symbol\"/></xsl:if>-<xsl:value-of select=\"@mass\"/>:</xsl:for-each></xsl:variable>");



 //<xsl:value-of select=\"search_hit[@hit_rank='1']/peptideprophet_result/@probability\"/></xsl:variable>");
 //  fprintf(fp, "<xsl:variable name=\"scores\"><xsl:if test=\"search_hit[@hit_rank='1']/peptideprophet_result/search_score_summary\">fval:<xsl:value-of select=\"search_hit[@hit_rank='1']/peptideprophet_result/search_score_summary/@fval\"/><xsl:text> </xsl:text>ntt:<xsl:value-of select=\"search_hit[@hit_rank='1']/peptideprophet_result/search_score_summary/@ntt\"/><xsl:text> </xsl:text>massd:<xsl:value-of select=\"search_hit[@hit_rank='1']/peptideprophet_result/search_score_summary/@massd\"/><xsl:text> </xsl:text><xsl:if test=\"search_hit[@hit_rank='1']/peptideprophet_result/search_score_summary/@nmc\">nmc:<xsl:value-of select=\"search_hit[@hit_rank='1']/peptideprophet_result/search_score_summary/@nmc\"/><xsl:text> </xsl:text></xsl:if><xsl:if test=\"search_hit[@hit_rank='1']/peptideprophet_result/search_score_summary/@icat\">icat:<xsl:value-of select=\"search_hit[@hit_rank='1']/peptideprophet_result/search_score_summary/@icat\"/><xsl:text> </xsl:text></xsl:if><xsl:if test=\"search_hit[@hit_rank='1']/peptideprophet_result/search_score_summary/@glyc\">glyc:<xsl:value-of select=\"search_hit[@hit_rank='1']/peptideprophet_result/search_score_summary/@glyc\"/><xsl:text> </xsl:text></xsl:if></xsl:if></xsl:variable>");
  // alternative
  fprintf(fp, "<xsl:variable name=\"scores\"><xsl:if test=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:analysis_result[@analysis='peptideprophet']/pepx:peptideprophet_result/pepx:search_score_summary\"><xsl:for-each select=\"pepx:search_result/pepx:search_hit[@hit_rank='1']/pepx:analysis_result[@analysis='peptideprophet']/pepx:peptideprophet_result/pepx:search_score_summary/pepx:parameter\"><xsl:value-of select=\"@name\"/>:<xsl:value-of select=\"@value\"/><xsl:text> </xsl:text></xsl:for-each></xsl:if></xsl:variable>");


  // here write the output format
  fprintf(fp, "<tr>");
  fprintf(fp, "%s\n", index);
  fprintf(fp, "%s\n", probability);
  fprintf(fp, "%s\n", spectrum);
  //printf("%s\n", scores);
  fprintf(fp, "%s\n", ions);
  fprintf(fp, "%s\n", peptide_sequence);
  //fprintf(fp, "%s\n", protein);
  fprintf(fp, "%s\n", xpress);
  fprintf(fp, "%s\n", asapratio);
  //printf("%s\n", fval);
  fprintf(fp, "</tr><xsl:text>\n</xsl:text>");

  if(minpepprob_ > 0.0)
    fprintf(fp, "</xsl:if>");
  fprintf(fp, "</xsl:if>\n");

  fprintf(fp, "</xsl:template>\n");


fprintf(fp, "</xsl:stylesheet>\n");


fclose(fp);

}

