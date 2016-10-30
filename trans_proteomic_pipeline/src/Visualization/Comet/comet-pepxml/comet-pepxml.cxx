#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <sstream>
//#include <ctype.h>
//#include <sys/stat.h>

#include "common/util.h"
#include "common/constants.h"
#include "Visualization/Comet/Comet.h"
#include "common/TPPVersion.h" // contains version number, name, revision

#define SIZE_BIG  81920


char szAttributeVal[SIZE_FILE];

struct ParametersStruct
{
   std::string mass_type_parent;
   std::string mass_type_fragment;
   std::string precursor_neutral_mass;
   std::string strInputFile;
} pParams;


char *GET_VAL(char *szSpectrumQuery, const char *szAttribute);
void INITIALIZE(void);
void EXTRACT_CGI_QUERY(void);
bool READ_ENTRY_XML(char *szSpectrumQuery,
      char *szSpectrum);
void PRINT_SEARCH_HITS(char *szSpectrumQuery);


int main(void)
{
   int i;
   int iScanNum;
   int iCharge;
   FILE *fp;
   char szBuf[SIZE_BUF];
   char szSpectrumQuery[SIZE_BIG];
   char szBaseName[SIZE_FILE];
   char szPepXMLFile[SIZE_FILE];
   char szSpectrum[SIZE_FILE];
   char *pStr;


   INITIALIZE();
   EXTRACT_CGI_QUERY();

   printf("Content-type: text/html\n\n");
   printf("<html>\n<head>\n");
   printf("  <title>comet-pepxml</title>\n");
   printf("  <link rel=\"stylesheet\" type=\"text/css\" href=\"/tpp/html/css/PepXMLViewer.css\" />\n");

   printf("\
  <script language=\"javascript\">\n\
    function toggle() {\n\
      var ele = document.getElementById(\"toggleText\");\n\
      var text = document.getElementById(\"displayText\");\n\
      if(ele.style.display == \"block\") {\n\
        ele.style.display = \"none\";\n\
        text.innerHTML = \"show search parameters\";\n\
      }\n\
      else {\n\
        ele.style.display = \"block\";\n\
        text.innerHTML = \"hide search parameters\";\n\
      }\n\
    } \n\
  </script>\n");

   printf("</head>\n<body>\n");
   printf("<div id=\"body\">\n");


   // decode input file name to get pep.xml file, scan #, charge

   strcpy(szBaseName, pParams.strInputFile.c_str());
   pStr = strrchr(szBaseName, '/');
   if ( pStr == NULL )
       pStr = strrchr(szBaseName, '\\');  // try a Window's style path

   if (pStr == NULL)
   {
      printf(" Error -  expecting full file path w/spectrum: %s\n\n", pParams.strInputFile.c_str());
      exit(1);
   }

   strcpy(szSpectrum, pStr + 1);

   *pStr = 0;

   strcpy(szPepXMLFile, szBaseName);

   if ( (fp=fopen(szPepXMLFile, "r"))==NULL)
   {
      printf(" Error - cannot read input .pep.xml file %s\n", szPepXMLFile);
      exit(1);
   }
   pParams.mass_type_parent = "1";
   pParams.mass_type_fragment = "1";

   bool bFoundEntry = false;
   while (fgets(szBuf, SIZE_BUF, fp))
   {
      if (strstr(szBuf, "mass_type_parent"))
         pParams.mass_type_parent = GET_VAL(szBuf, " value");
      if (strstr(szBuf, "mass_type_fragment"))
         pParams.mass_type_fragment = GET_VAL(szBuf, " value");

      // grab entire spectrum query tag
      if (strstr(szBuf, "<spectrum_query spectrum"))
      {
         strcpy(szSpectrumQuery, szBuf);

         // append subsequent lines together
         while (fgets(szBuf, SIZE_BUF, fp))
         {
            if (!strstr(szBuf, "alternative_protein"))
               strcat(szSpectrumQuery, szBuf);

            if (strstr(szBuf, "</spectrum_query>"))
            {
               bFoundEntry = READ_ENTRY_XML(szSpectrumQuery, szSpectrum);
               break;
            }
         }
      }
      if (bFoundEntry)
         break;

   }

   rewind(fp);

   printf("\n<p><a id=\"displayText\" href=\"javascript:toggle();\">show search parameters</a>\n");
   printf("<div id=\"toggleText\" style=\"display: none\"><tt>\n");
   while (fgets(szBuf, SIZE_BUF, fp))
   {
      if (strstr(szBuf, "<parameter "))
      {
         printf("<br>%s = ", GET_VAL(szBuf, "name"));
         printf("%s\n", GET_VAL(szBuf, "value"));
      }

      if (strstr(szBuf, "</search_summary>"))
         break;
   }
   printf("</tt></div>\n");

   fclose(fp);


   printf("</div></body></html>\n");
   return(0);

}


// for a given pepXML entry, parse info out
bool READ_ENTRY_XML(char *szSpectrumQuery,
      char *szSpectrum)
{
   char szTmp[SIZE_FILE];
   char szModPeptide[SIZE_FILE];
   char szPeptide[SIZE_FILE];
   char szCurrentSpectrum[SIZE_FILE];
   int x;
   int iLenDiff;

   bool bFoundEntry = false;

   strcpy(szCurrentSpectrum, GET_VAL(szSpectrumQuery, " spectrum"));

   iLenDiff = strlen(szCurrentSpectrum) - strlen(szSpectrum); // not sure if this is needed

   if (!strcmp(szSpectrum, szCurrentSpectrum + iLenDiff))
   {
      pParams.precursor_neutral_mass = GET_VAL(szSpectrumQuery, " precursor_neutral_mass");

      printf("<table><tr><td>\n");
      printf("\
  <table cellspacing=\"0\" width=\"100%%\">\n\
    <tbody>\n\
      <tr>\n\
        <td class=\"banner_cid\">&nbsp;&nbsp;&nbsp; %s &nbsp; &nbsp; &nbsp; mass %s\n\
        </td>\n\
      </tr>\n\
    </tbody>\n\
  </table>\n", szSpectrum, pParams.precursor_neutral_mass.c_str());

      printf("  <div class=\"formentry\">\n");

      printf("  <table class=\"data\">\n");

      printf("    <tr>");
      printf("      <td style=\"font-family: monospace;\">#</td>\n");
      printf("      <td style=\"font-family: monospace;\">peptide</td>\n");
      printf("      <td style=\"font-family: monospace;\">E-value</td>\n");
      printf("      <td style=\"font-family: monospace;\">xcorr</td>\n");
      printf("      <td style=\"font-family: monospace;\">dCn</td>\n");
      printf("      <td style=\"font-family: monospace;\">Sp</td>\n");
      printf("      <td style=\"font-family: monospace;\">rankSp</td>\n");
      printf("      <td style=\"font-family: monospace;\">neutral mass</td>\n");
      printf("      <td style=\"font-family: monospace;\">ions</td>\n");
      printf("      <td style=\"font-family: monospace;\">protein</td>\n");
      printf("    </tr>\n");

      PRINT_SEARCH_HITS(szSpectrumQuery);
      printf("  </table>\n");

      printf("  </div>\n");
      printf("</td></tr></table>\n");

      bFoundEntry= true;
   }

   return bFoundEntry;
}


void PRINT_SEARCH_HITS(char *szSpectrumQuery)
{
   char szSearchHit[SIZE_BIG];
   char szQuery[SIZE_BIG];

   strcpy(szQuery, szSpectrumQuery);

   int i=0;
   while (1)
   {
      char *pStr;
      char szPeptide[256];
      char szModifiedPeptide[1024];
      char szPrevAA[256];
      char szNextAA[256];
      char szTmp[256];
      int iNumTotProteins;
      std::string strLink="";
      std::string tmp;

      std::string script_name = getenv("SCRIPT_NAME");
      int slashPos = findRightmostPathSeperator(script_name);

      if ( (pStr = strstr(szQuery, "<search_hit")) == NULL)
      {
         break;
      }

      // copy individual search hit
      strcpy(szSearchHit, pStr);
      pStr = strstr(szSearchHit, "</search_hit>");

      strcpy(szQuery, pStr);
      *pStr = 0;


      szPeptide[0]='\0';
      szModifiedPeptide[0]='\0';
      strcpy(szPeptide, GET_VAL(szSearchHit, " peptide"));


      strLink = "<a target=\"Win1\" href=\"";
      strLink += script_name.substr(0, slashPos+1) + "plot-msms-js.cgi?PrecursorMassType=";
      strLink += pParams.mass_type_parent;
      strLink += "&amp;FragmentMassType=";
      strLink += pParams.mass_type_fragment;
      strLink += "&amp;PepMass=" + pParams.precursor_neutral_mass;

      // add N/C term mods if they exist
      if (strlen(GET_VAL(szSearchHit, "mod_nterm_mass"))>1) {
         tmp = GET_VAL(szSearchHit, "mod_nterm_mass");
         strLink += "&amp;ModN=" + tmp;
      }

      if (strlen(GET_VAL(szSearchHit, "mod_cterm_mass"))>1) {
         tmp = GET_VAL(szSearchHit, "mod_cterm_mass");
         strLink += "&amp;ModC=" + tmp;
      }

      // handle AA mods
      if (strstr(szSearchHit, "mod_aminoacid_mass"))
      {
         char szQuery[SIZE_BIG];
         char *pStr;

         strcpy(szQuery, szSearchHit);

         pStr = strstr(szQuery, "<mod_aminoacid_mass ");

         while (pStr)
         {
            int iPos;
            double dMass;
            char szPos[4];
            char szMass[64];

            sscanf(pStr, "<mod_aminoacid_mass position=\"%d\" mass=\"%lf\" />", &iPos, &dMass);
            sprintf(szPos, "%d", iPos);
            sprintf(szMass, "%0.6f", dMass);

            strLink += "&amp;Mod";
            strLink += szPos;
            strLink += "=";
            strLink += szMass;

            strcpy(szQuery, pStr+20);
            pStr = strstr(szQuery, "<mod_aminoacid_mass ");
         }
      }

      strLink += "&amp;Pep=";
      strLink += szPeptide;

      strLink += "&amp;Dta=";
      strLink += pParams.strInputFile + ".dta";

      strLink += "\">";

      tmp = GET_VAL(szSearchHit, " num_matched_ions");
      strLink += tmp;
      strLink += "/";
      tmp = GET_VAL(szSearchHit, " tot_num_ions");
      strLink += tmp;

      strLink += "</a>";


      strcpy(szModifiedPeptide, GET_VAL(szSearchHit, " modified_peptide"));
      strcpy(szPrevAA, GET_VAL(szSearchHit, " peptide_prev_aa"));
      strcpy(szNextAA, GET_VAL(szSearchHit, " peptide_next_aa"));

      printf("    <tr class=\"%s\">", i%2?"even":"odd");
      printf("<td class=\"center\">%s</td>", GET_VAL(szSearchHit, " hit_rank"));
      printf("<td class=\"left\">%s.%s.%s</td>", szPrevAA, (szModifiedPeptide[0]!='-'?szModifiedPeptide:szPeptide), szNextAA);
      printf("<td class=\"center\">%s</td>", GET_VAL(szSearchHit, "expect\" value"));
      printf("<td class=\"center\">%s</td>", GET_VAL(szSearchHit, "xcorr\" value"));
      printf("<td class=\"center\">%s</td>", GET_VAL(szSearchHit, "deltacn\" value"));
      printf("<td class=\"center\">%s</td>", GET_VAL(szSearchHit, "spscore\" value"));
      printf("<td class=\"center\">%s</td>", GET_VAL(szSearchHit, "sprank\" value"));
      printf("<td class=\"center\">%s</td>", GET_VAL(szSearchHit, " calc_neutral_pep_mass"));
      printf("<td class=\"center\">%s</td>", strLink.c_str());

      strcpy(szTmp, GET_VAL(szSearchHit, " num_tot_proteins"));
      sscanf(szTmp, "%d", &iNumTotProteins);

      if (iNumTotProteins > 1)
         printf("<td class=\"left\">%s +%d</td>", GET_VAL(szSearchHit, " protein"), iNumTotProteins);
      else
         printf("<td class=\"left\">%s</td>", GET_VAL(szSearchHit, " protein"));

      printf("</tr>\n");
      i++;
   }
}



// for a given attribute, return the attribute value 
char *GET_VAL(char *szSpectrumQuery, const char *szAttribute)
{
   char *pStr;

   if ( (pStr=strstr(szSpectrumQuery, szAttribute))!=NULL)
   {
      strncpy(szAttributeVal, pStr+strlen(szAttribute)+2, SIZE_FILE);  // +2 to skip ="

      if ( (pStr=strchr(szAttributeVal, '"'))!=NULL)
      {
         *pStr='\0';
         return(szAttributeVal);
      }
      else
      {
         //printf(" Error - expecting an end quote in %s\n", szAttributeVal);
         return((char *)"-");
      }
   }
   else
   {
      //printf("Error - cannot find %s in %s\n", szAttribute, szSpectrumQuery);
      return((char *)"-");
      getchar();
   }
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
         pParams.strInputFile = szWord;
      }
      else
      {
         getword(szWord, pQS, '&'); plustospace(szWord); unescape_url(szWord);
      }
   }
}


void INITIALIZE(void)
{
   int i;

   pParams.strInputFile[0]='\0';

}
