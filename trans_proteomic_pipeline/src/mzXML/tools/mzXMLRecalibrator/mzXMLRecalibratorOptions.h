#include <stdio.h>
#include <string.h>

#define PATH_DELIMS "/\\:"

struct _optionStruct {
   const char *option;
   void *value;
};

typedef struct _optionStruct optionStruct;
typedef optionStruct *optionStructPtr;

struct _optionInfo {
   const char *option;
   int n_arguments;
   void *default_value;
   int (*syntax_validator)(void *testVal);
};

typedef struct _optionInfo optionInfo;
typedef optionInfo *optionInfoPtr;

/* TEMPLATES */
char *createOutFileName(
   const char *source,
   char *buffer,
   const char *addendum,
   int pre);

int whichLegalOption(char *opt,optionInfo legal_options[]);

int getOptions(
   optionStruct curOptions[], 
   optionInfo legal_options[], 
   int argc, 
   char **argv);
