/*
 * This software is released under the GNU GPL or LPGL license
 * 
 * Copyright 2004, Jimmy Eng, 10/21/2004
 *
 *
 * Patrick 2004_12_14:
 * - Changed HAS_INDEX() to return the offset of the index
 *   instead of looking for the presence of the indexOffset
 *   element.
 *
 * Patrick 2004_12_14:
 * - Removed '\r' chars from the copied part of the mzXML
 *   file.
 *
 * Patrick 2004_12_14:
 * - removed iNumScans dead variable.
 *
 * Patrick 2004_12_14:
 * - Added GENERATE_INDEX() sub_routine to recicle code
 *   btw ADD_INDEX and VALIDATE_INDEX
 *
 * Patrick 2004_12_14:
 * - Added index validation.
 *
 * Patrick 2004_12_15:
 * - Stopped ADD_INDEX from copying corrupted indexes
 *   or NULL indexOffset elements. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>

#include "indexmzXMLSHA1Wrapper.hpp"

#define SIZE_BUF  8192
#define SIZE_FILE 512
#define CHUNK_SIZE 500  // How many index slots to add on each realloc

#include "common/sysdepend.h"

// want a minimal RAMP implementation, just for crossplatform longfile handling
#undef RAMP_HAVE_GZ_INPUT
#undef HAVE_PWIZ_MZML_LIB
#undef TPPLIB
#include "ramp.h"
#include "ramp.cpp"
#include "ramp_base64.cpp"

#define STREAMOFFSET ramp_fileoffset_t



STREAMOFFSET HAS_INDEX(const char *szInput, RAMPFILE * pFP);
STREAMOFFSET atooff(const char *s);  /* string conversion */
void ADD_INDEX(const char *szInput, RAMPFILE * pFP);
long GENERATE_INDEX(RAMPFILE * pFP, STREAMOFFSET ** pScanIndex);
void VALIDATE_INDEX(const char *szInput, RAMPFILE * pFP, STREAMOFFSET indexOffset);
std::string formatOffset(STREAMOFFSET off) {
	std::stringstream out;
	out << off;
	std::string result = out.str();
	return result;
}

int main(int argc, char **argv)
{
   int i;

   if (argc < 2)
   {
      printf("\n");
      printf(" indexmzXML\n");
      printf("\n");
      printf(" USAGE:  %s *.mzXML\n", argv[0]);
      printf("\n");
      printf(" This program adds the optional index element to mzXML files that don't\n");
      printf(" have the index.  If the index element is present, this program will\n");
      printf(" check if the index is valid and updates the index if it is not.\n");
      printf("\n");
      exit(0);
   }
   else
   {
      printf("\n");
      printf(" indexmzXML\n");
      printf("\n");
   }

   if (sizeof(STREAMOFFSET) < 8) {
	   puts("program is incorrectly compiled, will not handle files >2GB");
   };

   for (i = 1; i < argc; i++)
   {
      char   szInput[SIZE_FILE];

      strcpy(szInput, argv[i]);

      /*
       * Check mzXML file; should look into file and not rely on extension
       */
      if (!strcasecmp(szInput + strlen(szInput) - 6, ".mzXML"))
      {
         RAMPFILE  *pFP;
         STREAMOFFSET indexOffset;

         if ((pFP = rampOpenFile(szInput)) == NULL)
         {
            printf(" ERROR:  cannot open file %s, skipping ...\n", szInput);
         }
         else
         {
            printf(" Analyzing:  %s\n", szInput);
            fflush(stdout);

            if (!(indexOffset = HAS_INDEX(szInput, pFP)))
               ADD_INDEX(szInput, pFP);
            else
               VALIDATE_INDEX(szInput, pFP, indexOffset);

            rampCloseFile(pFP);
         }
      }
      else
      {
         printf(" Wrong extension, skipping: %s\n", szInput);
      }
   }

   printf("\n");
   exit(0);
} /*main*/


/*
 * Check if an index element exists in the mzXML file.
 * The stream is reset at the beginning of the file
 * before exiting.
 * @return: The offset of the index element.
 *
 */
STREAMOFFSET HAS_INDEX(const char *szInput, RAMPFILE * pFP)
{
   char   szBuf[SIZE_BUF];
   char  *indexOffset;
   char  *pStr;

   ramp_fseek(pFP, -200, SEEK_END);
   ramp_fread(szBuf, SIZE_BUF, pFP);
   ramp_fseek(pFP, 0, SEEK_SET);

   indexOffset = strstr(szBuf, "<indexOffset>");

   if (indexOffset == NULL)
      return 0;

   pStr=strchr(indexOffset+strlen("<indexOffset>"), '<');
   *pStr='\0';
/*
printf("*** indexOffset: %s\n", indexOffset);
printf("*** offset: %s\n", indexOffset + strlen("<indexOffset>"));
printf("*** atoll: %lld\n", atoll(indexOffset + strlen("<indexOffset>")));
printf("*** atooff: %lld\n", atooff(indexOffset + strlen("<indexOffset>")));
printf("*** sizeof(STREAMOFFSET): %d\n", sizeof(STREAMOFFSET));
*/
   return atooff(indexOffset + strlen("<indexOffset>"));

} /*HAS_INDEX*/


STREAMOFFSET atooff(const char *s)
{
	return atoll(s);
}


void ADD_INDEX(const char *szInput, RAMPFILE * pFP)
{
   long   i;
   long   reallocSize = 100000;
   char   szBuf[SIZE_BUF];
   char   szOutput[SIZE_BUF];
   char   szHash[SIZE_BUF];
   char   lf = '\n';    //0xA;
   STREAMOFFSET *pScanIndex;
   STREAMOFFSET  indexOffset=0;
   FILE  *pFP2;

   printf(" Adding index to: %s.new\n", szInput);

   if ((pScanIndex = (STREAMOFFSET *)calloc(reallocSize, sizeof(STREAMOFFSET))) == NULL)
   {
      printf(" ERROR:  cannot allocate memory\n\n");
      exit(1);
   }

   /*
    * write out new mzXML file with index
    */
   sprintf(szOutput, "%s.new", szInput);
   if ((pFP2 = fopen(szOutput, "wb")) == NULL)
   {
      printf(" ERROR:  cannot open temporary output file %s to write\n\n", szOutput);
      exit(1);
   }

   /*
    * duplicate the mzXML file up to (and not including)
    * the index or the indexOffset tag (whichever comes
    * first)
    */
   char  *stopCopy;

   ramp_fseek(pFP,0,SEEK_SET);

   while (ramp_fgets(szBuf, SIZE_BUF, pFP))
   {
      if (*(szBuf + strlen(szBuf) - 2) == '\r')
      {
         *(szBuf + strlen(szBuf) - 2) = '\n';
         *(szBuf + strlen(szBuf) - 1) = '\0';
      }
      if (stopCopy = strstr(szBuf, "<index"))
      {
         *stopCopy = '\0';
         fputs(szBuf,pFP2);
		 indexOffset+=strlen(szBuf);
         break;
      }
      fputs(szBuf,pFP2);
      indexOffset+=strlen(szBuf);
   }

   /*
    *  add in the index ; first note index location in new file
    */
   strcpy(szBuf,"  ");
   fputs(szBuf,pFP2);
   indexOffset+=strlen(szBuf);         
   fputs("<index name=\"scan\">\n",pFP2);

   ramp_fseek(pFP,0,SEEK_SET);

   fclose(pFP2);

   /*
    * Generate index on new file
    */
   RAMPFILE *pFP3;
   if ((pFP3 = rampOpenFile(szOutput)) == NULL)
   {
      printf(" ERROR:  cannot open temporary output file %s to read\n\n", szOutput);
      exit(1);
   }
   reallocSize = GENERATE_INDEX(pFP3, &pScanIndex);
   rampCloseFile(pFP3);
  
   /*
    * Append index to new file
    */
   if ((pFP2 = fopen(szOutput, "ab")) == NULL)
   {
      printf(" ERROR:  cannot open temporary output file %s to append\n\n", szOutput);
      exit(1);
   }

   for (i = 0; i < reallocSize; i++)
   {
      if (pScanIndex[i] > 0)
      {
         fprintf(pFP2, "    <offset id=\"%ld\">%s</offset>\n", i, formatOffset(pScanIndex[i]).c_str());
      }
      fflush(pFP2);
   }

   free(pScanIndex);

   fprintf(pFP2, "  </index>\n");
   fprintf(pFP2, "  <indexOffset>%s</indexOffset>\n", formatOffset(indexOffset).c_str());

   /*
    *  add in the sha1 checksum
    */
   fprintf(pFP2, "  <sha1>");
   fclose(pFP2);

   szHash[0] = '\0';
   if (sha1_hashFile(szOutput, szHash) == 0)
   {
      if ((pFP2 = fopen(szOutput, "a")) == NULL)
      {
         printf(" ERROR:  cannot open temporary output file %s to append\n\n",
                szOutput);
         exit(1);
      }
      fprintf(pFP2, "%s</sha1>\n", szHash);
   }
   else
   {
      if ((pFP2 = fopen(szOutput, "a")) == NULL)
      {
         printf(" ERROR:  cannot open temporary output file %s to append\n\n",
                szOutput);
         exit(1);
      }
   }

   /*
    *  print out the end tag
    */
   fprintf(pFP2, "</mzXML>\n");

   fclose(pFP2);

} /*ADD_INDEX */


long GENERATE_INDEX(RAMPFILE * pFP, STREAMOFFSET ** pScanIndex)
{
   long   reallocSize = 100000;
   char   szBuf[SIZE_BUF];
   STREAMOFFSET  indexOffset=0;

   /*
    * loop through input mzXML file
    */
   while (ramp_fgets(szBuf, SIZE_BUF, pFP))
   {
      /*
       * Find scan tag; indexOffset is correct file offset value
       */
      if (strstr(szBuf, "<scan"))
      {
         long    iScanNum = 0;
         char  *pStr;
         STREAMOFFSET offsetDiff = 0;

         // We need the offset to point to opening char of "<scan" element
         while (strncmp(szBuf+offsetDiff, "<scan", 5))
            offsetDiff++;

         // Get the scan number; no assumption on sequential scan #s and no
         // assumption that num attribute on same line as scan element tag
         // or else this part is useless and we can just increment iScanNum
         if (!(pStr = strstr(szBuf, "num=\"")))
         {
            while (ramp_fgets(szBuf, SIZE_BUF, pFP))
               if ((pStr = strstr(szBuf, "num=\"")))
                  sscanf(pStr, "num=\"%ld\"", &iScanNum);
         }
         else
         {
            sscanf(pStr, "num=\"%ld\"", &iScanNum);
         }

         if (iScanNum >= reallocSize)
         {
            STREAMOFFSET *pTmp;
            long newSize = reallocSize + CHUNK_SIZE;

            pTmp = (STREAMOFFSET *) realloc(*pScanIndex, newSize * sizeof(STREAMOFFSET));

            if (pTmp == NULL)
            {
               printf(" ERROR:  cannot realloc memory\n\n");
               exit(1);
            }

            // zero out the added CHUNK_SIZE*sizeof(STREAMOFFSET) bytes of newly alloc'd memory
            memset(pTmp + reallocSize, 0, CHUNK_SIZE*sizeof(STREAMOFFSET));
            *pScanIndex = pTmp;
            reallocSize = newSize;
         }

         (*pScanIndex)[iScanNum] = indexOffset + offsetDiff;
      }                         // look for scan tag
      indexOffset += strlen(szBuf);

   }

   ramp_fseek(pFP,0,SEEK_SET);
   return reallocSize;

} /*GENERATE_INDEX*/


void VALIDATE_INDEX(const char *szInput, RAMPFILE * pFP, STREAMOFFSET indexOffset)
{
   long    n;
   long    iScanNum = 0;
   long    iMaxScanNum = 0;
   long    reallocSize = 100000;
   char   szBuf[SIZE_BUF];
   char  *pStr;
   STREAMOFFSET *pScanIndex;
   STREAMOFFSET *pScanIndexToValidate;


   // Generate real index

   if ((pScanIndex = (STREAMOFFSET *)calloc(reallocSize, sizeof(STREAMOFFSET))) == NULL)
   {
      printf(" ERROR:  cannot allocate memory\n\n");
      exit(1);
   }

   GENERATE_INDEX(pFP, &pScanIndex);

   // Read instance document index
   printf(" Validate index: %s\n", szInput);

   if ((pScanIndexToValidate = (STREAMOFFSET *)calloc(reallocSize, sizeof(STREAMOFFSET))) == NULL)
   {
      printf(" ERROR:  cannot allocate memory\n\n");
      exit(1);
   }

   // check that indexOffset is correct
   ramp_fseek(pFP, indexOffset, SEEK_SET);
   ramp_fgets(szBuf, SIZE_BUF, pFP);

   /*
    * indexOffset must point to '<' of index tag ; historically our converters
    * pointed to the beginning of the line so allow that offset to be valid also.
    */
   if (strncmp(szBuf, "<index name=\"scan\">", 19) && strncmp(szBuf, "  <index name=\"scan\">", 21) && strncmp(szBuf, "<index name=\"scan\" >", 20))
   {
      printf(" The index offset [%s] is incorrect\n", formatOffset(indexOffset).c_str());

      szBuf[500]='\0';
      printf("The offset points to: %s\n\n", szBuf);
      ADD_INDEX(szInput, pFP);
   }
   else
   {
      ramp_fseek(pFP, indexOffset, SEEK_SET);

      while (ramp_fgets(szBuf, SIZE_BUF, pFP))
      {
         if (strstr(szBuf, "<offset"))
         {
            if (!(pStr = strstr(szBuf, "id=\"")))
            {
               while (ramp_fgets(szBuf, SIZE_BUF, pFP))
                  if ((pStr = strstr(szBuf, "id=\"")))
                     sscanf(pStr, "id=\"%ld\"", &iScanNum);
            }
            else
            {
               sscanf(pStr, "id=\"%ld\"", &iScanNum);
            }
   
            if (iScanNum > iMaxScanNum)
               iMaxScanNum = iScanNum;

            if (iScanNum >= reallocSize)
            {
               STREAMOFFSET *pTmp;
               long newSize = reallocSize + CHUNK_SIZE;

               pTmp = (STREAMOFFSET *) realloc(pScanIndexToValidate, newSize * sizeof(STREAMOFFSET));

               if (pTmp == NULL)
               {
                  printf(" ERROR:  cannot realloc memory\n\n");
                  exit(1);
               }

               // zero out the added CHUNK_SIZE*sizeof(STREAMOFFSET) bytes of newly alloc'd memory
               memset(pTmp + reallocSize, 0, CHUNK_SIZE*sizeof(STREAMOFFSET));
               pScanIndexToValidate = pTmp;
               reallocSize = newSize;
            }
   
            if (!(pStr = strstr(szBuf, ">")))
            {                      // look for the end of the offset element
               while (ramp_fgets(szBuf, SIZE_BUF, pFP))
                  if ((pStr = strstr(szBuf, ">")))
                     pScanIndexToValidate[iScanNum] = atooff(pStr + strlen(">"));
            }
            else
            {
               pScanIndexToValidate[iScanNum] = atooff(pStr + strlen(">"));
            }
         }
      }

      ramp_fseek(pFP,0,SEEK_SET);

      // Compare the two indexes
      for (n = 0; n <= iMaxScanNum; n++)
      {
         if (pScanIndexToValidate[n] != pScanIndex[n])
         {
            printf("The index is corrupted: (scan %ld) %s vs %s\n",
                  n, formatOffset(pScanIndexToValidate[n]).c_str(), formatOffset(pScanIndex[n]).c_str());
            ADD_INDEX(szInput, pFP);
            printf("Done\n");
            break;
         }
      }
   }

   free(pScanIndexToValidate);
   free(pScanIndex);

} /*VALIDATE_INDEX*/
