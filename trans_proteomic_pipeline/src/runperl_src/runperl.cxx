/* A really cheesy hack to get the IIS server
   to run a perl script through CYGWIN */
#include "common/sysdepend.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "common/util.h"

int main(int argc, char** argv) {
  int ret;  
  char *pLink;
  pLink=getenv("PATH_TRANSLATED");
  if ( pLink == NULL )
  {
    printf("Content-type: text/html\n\n");
    printf("ERROR - no value for env var 'PATH_TRANSLATED'");
    exit(EXIT_FAILURE);
  }
#ifndef WINDOWS_CYGWIN
  ret=system(pLink);
#else  
  {
  FILE *fpIn;
  char szBuf[SIZE_BUF];
  char szCmd[SIZE_BUF];

  strcpy(szCmd,"cygpath '");
  strcat(szCmd,pLink);
  strcat(szCmd,"'");

  if ( (fpIn=popen(szCmd,"r"))==NULL)
  {
    printf("Content-type: text/html\n\n");
    printf("Error\n");
    exit(EXIT_FAILURE);
  }
  fgets(szBuf, SIZE_BUF, fpIn);
  /*printf("The converted path is %s\n",szBuf);*/

  strcpy(szCmd, "/bin/perl ");
  strcat(szCmd,szBuf);
  ret = system(szCmd);
  }
#endif
  exit(ret);
}
