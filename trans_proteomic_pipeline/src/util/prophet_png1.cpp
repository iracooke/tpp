/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library or "Lesser" General Public      *
 *   License (LGPL) as published by the Free Software Foundation;          *
 *   either version 2 of the License, or (at your option) any later        *
 *   version.                                                              *
 *                                                                         *
 ***************************************************************************/


/*
 * This program takes in a PeptideProphet model file as input on the command
 * line and creates the individual error/sensitivity and distribution plots
 * by calling gnuplot.  Modified from the plotmodel.cgi program distributed
 * by the peptideprophet.sourceforge.net project
 *
 * 2007/06/13 Update to read in pepXML file
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE_FILE   256
#define SIZE_BUF    8192

#define POSITIVE_COLOR "#A020F0"
#define NEGATIVE_COLOR "#0000FF"


char szAttributeVal[SIZE_FILE];

char *GET_VAL(char *szBuf,
      const char *szAttribute);


int main(int argc, char **argv)
{
   char szModelFile[SIZE_FILE];
   // unused:
   // szCommand[SIZE_FILE];
   // double dProbability=0.0;
   FILE *fp;

   if (argc<2)
   {
      printf("\n");
      printf(" Please enter a PeptideProphet pepXML file as input on the command line.\n");
      printf(" The results are 4 PNG images with the same basename as the input model file.\n\n");
      exit(1);
   }

   strcpy(szModelFile, argv[1]);

   if (!strstr(szModelFile, ".xml"))
   {
      printf(" Error - input file (%s) doesn't end in .xml\n", szModelFile);
      printf(" Exiting.\n");
   }
   else if ((fp=fopen(szModelFile, "r"))==NULL)
   {
      printf(" Error - cannot open model file %s\n", szModelFile);
      printf(" Exiting.\n");
   }
   else
   {
      char szBuf[SIZE_BUF],
           szTmpPlotFile[SIZE_FILE],
           szTmpImageFile[SIZE_FILE],
           szTmpDataFile[SIZE_FILE];
      FILE *fpTmp;
      // unused:
      // time_t tStartTime;

      strcpy(szTmpPlotFile, szModelFile);
      strcpy(szTmpDataFile, szModelFile);
      strcat(szTmpPlotFile, ".gp");
      strcat(szTmpDataFile, ".data");

      if ( (fpTmp=fopen(szTmpDataFile, "w"))==NULL)
      {
         printf("Error - cannot create 1st image data file.");
      }
      else
      {
         int bBreak=0;

         while (fgets(szBuf, SIZE_BUF, fp))
         {
            if (!strncmp(szBuf, "<roc_data_point", 15))
            {
               do
               {
                  double dProbability;
                  double dError;
                  double dSensitivity;

                  if (strncmp(szBuf, "<roc_data_point", 15))
                  {
                     bBreak=1;
                     break;
                  }
  
                  sscanf(GET_VAL(szBuf, " min_prob"), "%lf", &dProbability);
                  sscanf(GET_VAL(szBuf, " sensitivity"), "%lf", &dSensitivity);
                  sscanf(GET_VAL(szBuf, " error"), "%lf", &dError);

                  fprintf(fpTmp, "%0.4f\t%0.4f\t%0.4f\n", dProbability, dSensitivity, dError);
               } while (fgets(szBuf, SIZE_BUF, fp));
            }
            if (bBreak)
               break;
         }
         fclose(fpTmp);

         sprintf(szTmpImageFile, "%s.0.png", szModelFile);

         /*
          * Create the first image's gnuplot .gp file
          */
         if ( (fpTmp=fopen(szTmpPlotFile, "w"))==NULL)
         {
            printf("Error - cannot create 1st image file.");
         }
         else
         {
            fprintf(fpTmp, "set terminal png;\n");
            fprintf(fpTmp, "set output \"%s\";\n", szTmpImageFile);
            fprintf(fpTmp, "set border;\n");
            fprintf(fpTmp, "set title \"Sensitivity & Error Rates\";\n");
            fprintf(fpTmp, "set xlabel \"Min Probability Threshhold (MPT) To Accept\";\n");
            fprintf(fpTmp, "set ylabel \"Sensitivity & Error\";\n");
            fprintf(fpTmp, "set grid;\n");
            fprintf(fpTmp, "set size 0.5,0.6;\n");
            fprintf(fpTmp, "plot \"%s\" using 1:2 title 'sensitivity' with lines, \\\n", szTmpDataFile);
            fprintf(fpTmp, "     \"%s\" using 1:3 title 'error' with lines\n", szTmpDataFile);

            fclose(fpTmp);

            sprintf(szBuf, "gnuplot %s; rm -f %s %s\n", szTmpPlotFile, szTmpPlotFile, szTmpDataFile);
            int errCheckMe = -1;
            errCheckMe = system(szBuf);
         }

         rewind(fp);
      }
      
      strcpy(szTmpPlotFile, szModelFile);
      strcpy(szTmpDataFile, szModelFile);
      strcat(szTmpPlotFile, ".gp");
      strcat(szTmpDataFile, ".data");

      if ( (fpTmp=fopen(szTmpDataFile, "w"))==NULL)
      {
         printf("Error - cannot create 2nd image data file.");
      }
      else
      {
         int iChargeState;
         int bBreak=0;

         /*
          * Create the fval curves
          */
         while (fgets(szBuf, SIZE_BUF, fp))
         {
            if (!strncmp(szBuf, "<distribution_point", 19))
            {
               do
               {
                  double dFval,
                         d1Pos, d1Neg,
                         d2Pos, d2Neg,
                         d3Pos, d3Neg,
                         d4Pos, d4Neg,
                         d5Pos, d5Neg,
                         d6Pos, d6Neg,
                         d7Pos, d7Neg ;
                  int iNum1, iNum2, iNum3, iNum4, iNum5, iNum6, iNum7;

                  if (strncmp(szBuf, "<distribution_point", 19))
                  {
                     bBreak=1;
                     break;
                  }

                  sscanf(GET_VAL(szBuf, " fvalue"), "%lf", &dFval);

                  sscanf(GET_VAL(szBuf, " model_1_pos_distr"), "%lf", &d1Pos);
                  sscanf(GET_VAL(szBuf, " model_1_neg_distr"), "%lf", &d1Neg);
                  sscanf(GET_VAL(szBuf, " obs_1_distr"), "%d", &iNum1);
                  sscanf(GET_VAL(szBuf, " model_2_pos_distr"), "%lf", &d2Pos);
                  sscanf(GET_VAL(szBuf, " model_2_neg_distr"), "%lf", &d2Neg);
                  sscanf(GET_VAL(szBuf, " obs_2_distr"), "%d", &iNum2);
                  sscanf(GET_VAL(szBuf, " model_3_pos_distr"), "%lf", &d3Pos);
                  sscanf(GET_VAL(szBuf, " model_3_neg_distr"), "%lf", &d3Neg);
                  sscanf(GET_VAL(szBuf, " obs_3_distr"), "%d", &iNum3);
                  sscanf(GET_VAL(szBuf, " model_4_pos_distr"), "%lf", &d4Pos);
                  sscanf(GET_VAL(szBuf, " model_4_neg_distr"), "%lf", &d4Neg);
                  sscanf(GET_VAL(szBuf, " obs_4_distr"), "%d", &iNum4);
                  sscanf(GET_VAL(szBuf, " model_5_pos_distr"), "%lf", &d5Pos);
                  sscanf(GET_VAL(szBuf, " model_5_neg_distr"), "%lf", &d5Neg);
                  sscanf(GET_VAL(szBuf, " obs_5_distr"), "%d", &iNum5);
                  sscanf(GET_VAL(szBuf, " model_6_pos_distr"), "%lf", &d6Pos);
                  sscanf(GET_VAL(szBuf, " model_6_neg_distr"), "%lf", &d6Neg);
                  sscanf(GET_VAL(szBuf, " obs_6_distr"), "%d", &iNum6);
                  sscanf(GET_VAL(szBuf, " model_7_pos_distr"), "%lf", &d7Pos);
                  sscanf(GET_VAL(szBuf, " model_7_neg_distr"), "%lf", &d7Neg);
                  sscanf(GET_VAL(szBuf, " obs_7_distr"), "%d", &iNum7);

                  fprintf(fpTmp, "%0.2f\t", dFval);
                  fprintf(fpTmp, "%d\t%0.2f\t%0.2f\t", iNum1, d1Pos, d1Neg);
                  fprintf(fpTmp, "%d\t%0.2f\t%0.2f\t", iNum2, d2Pos, d2Neg);
                  fprintf(fpTmp, "%d\t%0.2f\t%0.2f\t", iNum3, d3Pos, d3Neg);
                  fprintf(fpTmp, "%d\t%0.2f\t%0.2f\t", iNum4, d4Pos, d4Neg);
                  fprintf(fpTmp, "%d\t%0.2f\t%0.2f\t", iNum5, d5Pos, d5Neg);
                  fprintf(fpTmp, "%d\t%0.2f\t%0.2f\t", iNum6, d6Pos, d6Neg);
                  fprintf(fpTmp, "%d\t%0.2f\t%0.2f\n", iNum7, d7Pos, d7Neg);

               } while (fgets(szBuf, SIZE_BUF, fp));
            }
            if (bBreak)
               break;
         }
         fclose(fpTmp);

         for (iChargeState=1; iChargeState <=7; iChargeState++)
         {
            sprintf(szTmpImageFile, "%s.%d.png", szModelFile, iChargeState);
            if ( (fpTmp=fopen(szTmpPlotFile, "w"))==NULL)
            {
               printf("Error - cannot create %d+ image file.", iChargeState);
            }
            else
            {
               /*
                * Create the gnuplot .gp file
                */
               fprintf(fpTmp, "set terminal png;\n");
               fprintf(fpTmp, "set output \"%s\";\n", szTmpImageFile);
               fprintf(fpTmp, "set title \"%d+\";\n", iChargeState);
               fprintf(fpTmp, "set border;\n");
               fprintf(fpTmp, "set xlabel \"Discriminant Score (F-value)\";\n");
               fprintf(fpTmp, "set ylabel \"# of Spectra\";\n");
               fprintf(fpTmp, "set grid;\n");
               fprintf(fpTmp, "set size 0.4,0.5;\n");
               fprintf(fpTmp, "plot \"%s\" using 1:%d title '%d+ distr' with lines lc rgb \"#000000\", \\\n",
                     szTmpDataFile, iChargeState*2 + iChargeState-1, iChargeState);
               fprintf(fpTmp, "     \"%s\" using 1:%d title '%d+ pos' with lines lc rgb \"%s\", \\\n",
                     szTmpDataFile, iChargeState*2 + iChargeState-1+1, iChargeState, POSITIVE_COLOR);
               fprintf(fpTmp, "     \"%s\" using 1:%d title '%d+ neg' with lines lc rgb \"%s\"\n",
                     szTmpDataFile, iChargeState*2 + iChargeState-1+2, iChargeState, NEGATIVE_COLOR);
   
               fclose(fpTmp);
   
               sprintf(szBuf, "gnuplot %s; rm -f %s\n", szTmpPlotFile, szTmpPlotFile);
               int errCheckMe = -1;
               errCheckMe = system(szBuf);
            }
         }
         sprintf(szBuf, "rm -f %s\n", szTmpDataFile);
         int errCheckMe = -1;
         errCheckMe = system(szBuf);
      }

      fclose(fp);
   }

   return(0);

} /*main*/


/* for a given attribute, return the attribute value */
char *GET_VAL(char *szBuf, const char *szAttribute)
{
   char *pStr;
   
   if ( (pStr=strstr(szBuf, szAttribute))!=NULL)
   {
      strncpy(szAttributeVal, pStr+strlen(szAttribute)+2, SIZE_FILE);  /* +2 to skip =" */

      if ( (pStr=strchr(szAttributeVal, '"'))!=NULL)
      {
         *pStr='\0';
         return(szAttributeVal);
      }
      else
      {
         //printf(" Error - expecting an end quote in %s\n", szAttributeVal);
         strcpy(szAttributeVal, "0");
         return(szAttributeVal);
      }
   }
   else
   {
      //printf("Error - cannot find %s in %s\n", szAttribute, szBuf);
      strcpy(szAttributeVal, "0");
      return(szAttributeVal);
   }
}

