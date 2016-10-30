#include "mzXMLRecalibratorOptions.h"

char *createOutFileName(
   const char *source,
   char *buffer,
   const char *addendum,
   int pre){

   const char *basename = source;
   const char *temp = source;

   /* find last token in source delimited '/' or '\\' or ':'*/
   while(temp != NULL) {
      temp = strpbrk(basename,PATH_DELIMS);
      if(temp != NULL) {
         basename = ++temp;
      }
   }
   if(basename == NULL) basename = "";       
   sprintf(buffer,"%s%s",(pre ? addendum : basename),(pre ? basename : addendum));
   return buffer;
}


int whichLegalOption(char *opt,optionInfo legal_options[]) {
   int index = 0;

   while(legal_options[index].option != NULL) {
      if(strcmp(opt,legal_options[index].option) == 0) {
         return index;
      } else {
        index++;
      }
   }
   return -1;
}

/* load options from argv into curOptions.  Return index of last option */
/* or -1 for error (illegal option or missing parameter.   Exit when    */
/* argv is exhausted or parameter with no leading "-"                   */

int getOptions(
   optionStruct curOptions[], 
   optionInfo legal_options[], 
   int argc, 
   char **argv) {
   
   int legalOptionIndex = -1;
   int argvIndex = 1;
   int optionIndex = 0;
   
   while(argvIndex < argc && *argv[argvIndex] == '-') {
      /* is current index pointing a legal option? */
      legalOptionIndex = whichLegalOption(argv[argvIndex],legal_options);
      if(legalOptionIndex == -1) {
	      fprintf(stderr,"ERROR -- %s is not a legal option\n",argv[argvIndex]);
         return -1;
      }
      /* ok, it's legal, install it into curOptions and see if it takes a parameter */
      curOptions[optionIndex].option = argv[argvIndex];
      curOptions[optionIndex].value = NULL;
      if(legal_options[legalOptionIndex].n_arguments == 1) {
	   if(++argvIndex >= argc || *argv[argvIndex] == '-') {
	      fprintf(stderr,"ERROR -- %s requires an argument\n",argv[argvIndex-1]);
         return -1;
      }
      curOptions[optionIndex].value = (void *)argv[argvIndex];
      }
      ++argvIndex;
      ++optionIndex;
   }
   return argvIndex;
}

