#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/*
 * This program uses readmzXML and gnuplot to generate a PNG image
 * containing base peak chromatogram (BPC) and total ion chromatogram (TIC)
 * for input mzXML files.
 */

#define SIZE_FILE  256
#define SIZE_BUF  4096

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 1
#endif
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 0
#endif


char *command;

void USAGE(int failure,
        int iColorTIC,
        int iColorBPC);
void SET_OPTION(char *arg,
        int *bStdOut,
        int *iColorTIC,
        int *iColorBPC);


int main(int argc, char **argv)
{
   char *arg,
        szBaseName[SIZE_FILE],
        szRawFile[SIZE_FILE],
        szCommand[SIZE_BUF],
        szTextFile[SIZE_BUF],
        szTmp[SIZE_BUF];
   int  i,
        iNumArg,
        iStartArgc,
        iColorTIC,
        iColorBPC,
        bStdOut;    /* send image to standard out */
   FILE *fp;
 
   command = argv[0];
   iColorTIC=8;
   iColorBPC=1;

   if (argc<2)
   {
      USAGE(0, iColorTIC, iColorBPC);
   }

   iStartArgc=1;
   iNumArg=1;
   bStdOut=0;
   arg = argv[iNumArg];

   while (iNumArg < argc)
   {
      if (arg[0] == '-')
         SET_OPTION(arg, &bStdOut, &iColorTIC, &iColorBPC);
      else
         break;

      iStartArgc++;
      arg = argv[++iNumArg];
   }

   /* no input files specified */
   if (argc-iStartArgc == 0)
      exit(0);

   for (i=iStartArgc; i<argc; i++)
   {
      strcpy(szRawFile, argv[i]);

      if (!strncasecmp(szRawFile+strlen(szRawFile)-6, ".mzXML", 6))
      {
         strcpy(szBaseName, szRawFile);

         szBaseName[strlen(szBaseName)-6]='\0';
         sprintf(szCommand, "readmzXML -h %s >> /dev/null", szRawFile);
	 int errCheckMe = -1;
         errCheckMe = system(szCommand);
         sprintf(szTextFile, "%s.txt", szBaseName);

         /* if text file exists, create chromatogram plots */
         if ( (fp=fopen(szTextFile, "r"))!=NULL)
         {
            fclose(fp);
            sprintf(szTmp, "%s.gp", szBaseName);
            if ( (fp=fopen(szTmp, "w"))!=NULL)
            {
               fprintf(fp, "set terminal png\n");
               if (bStdOut)
                  fprintf(fp, "set output\n");
               else
                  fprintf(fp, "set output \"%s.png\"\n", szBaseName);
               fprintf(fp, "set size 1.0,0.75\n");
               fprintf(fp, "set title \"%s\"\n", szBaseName);
               fprintf(fp, "set xlabel \"scan number\"\n");
               fprintf(fp, "set ylabel \"relative abundance\"\n");
               fprintf(fp, "set format y \"%%3.1e\"\n");
               fprintf(fp, "set multiplot\n");
               /*
               fprintf(fp, "set size 1.0,0.5\n");
               fprintf(fp, "set origin 0.0,0.5\n");

               fprintf(fp, "plot [0:] \"%s\" using \"%%lf%%*lf%%*f%%lf\" title \"Basepeak\" with lines 9\n", szTextFile);
               */

               fprintf(fp, "set size 1.0,0.75\n");
               fprintf(fp, "set origin 0.0,0.0\n");

               fprintf(fp, "plot [0:] \"%s\" using \"%%lf%%*lf%%*f%%*f%%lf\" title \"TIC\" with lines %d, ", szTextFile, iColorTIC);
               fprintf(fp, "\"%s\" using \"%%lf%%*lf%%*f%%lf\" title \"BPC\" with lines %d\n", szTextFile, iColorBPC);
               fclose(fp);

               sprintf(szCommand, "gnuplot %s; rm -f %s",  szTmp, szTmp);
	       int errCheckMe = -1;
	       errCheckMe = system(szCommand);
               if (!bStdOut)
                  printf(" Created %s.png\n", szBaseName);
            }

            unlink(szTextFile);
         }
      }
  }
   
   printf("\n");
   return(EXIT_SUCCESS);

} /*main*/


void USAGE(int failure,
      int iColorTIC,
      int iColorBPC)
{
   printf("\n");
   printf(" PLOTTIC usage:  %s [options] [.mzXML files]\n", command);
   printf("\n");
   printf(" This program generates a total ion current (TIC) chromatograms and base-peak\n");
   printf(" chromatogram (BPC) plot.\n");
   printf("\n");
   printf("       options:  -s  image to standard out, as opposed to default png file.\n");
   printf("                 -b<num>  apply gnuplot <num> to color BPC (default=%d)\n", iColorBPC);
   printf("                 -c<num>  apply gnuplot <num> to color TIC (default=%d)\n", iColorTIC);
   printf("\n");
   printf(" Note:  requires readmzXML and gnuplot\n");
   printf("\n");

   exit(EXIT_FAILURE);

} /*USAGE*/


void SET_OPTION(char *arg,
        int *bStdOut,
        int *iColorTIC,
        int *iColorBPC)
{
   int iTmp;

   switch (arg[1])
   {
      case 's':
         *bStdOut = 1;
         break;
      case 'c':
         if (sscanf(&arg[2], "%d", &iTmp) != 1)
            printf("Bad color number: '%s' ... ignored\n", &arg[2]);
         else
            *iColorTIC = iTmp;
         break;
      case 'b':
         if (sscanf(&arg[2], "%d", &iTmp) != 1)
            printf("Bad color number: '%s' ... ignored\n", &arg[2]);
         else
            *iColorBPC = iTmp;
         break;
      default:
         break;
   }
   arg[0] = '\0';
}
