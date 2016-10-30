#include "common/constants.h"

#define MAX_SCORE_COUNT      500
#define MAX_PEPTIDE_LEN      256
#define MAX_REFERENCE_LEN    256
#define MAX_HEADER_LEN       256
#define MAX_NUM_HEADERS      10
//#define SIZE_BUF             4096
//#define SIZE_FILE            256
#define MAX_MASS             8192

#define szXMLTITLE           "COMET2XML by Jimmy Eng"
#define szTITLE              "COMET by Jimmy Eng"
#define szCOPYRIGHT          "Copyright (c) Institute for Systems Biology, 2000.  All rights reserved"
#define DEFAULT_CONFIG_FILE  "comet.def"

#define BGCOLOR "#DDA0DD"    /* background border color for both plot-msms.cgi and comet-fastadb.cgi */

/* *** in constants.h ***
   #ifdef ISB_VERSION
   #define COMETLINKSDIR        "/tools/bin/"
   #else
   #ifdef __CYGWIN__
   #define COMETLINKSDIR        "c:\\Inetpub\\wwwroot\\"
   #endif
   #endif
   #define COMETLINKSFILE       "cometlinks.def"
*** in constants.h *** */


#define SIMILARITY_CUTOFF     0.90
#define MAX_NUM_VAR_MOD_RES   5

#ifndef TRUE
#define TRUE                  1
#endif
#ifndef FALSE
#define FALSE                 0
#endif
