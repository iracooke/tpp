#ifndef _MZXML_RECALIBRATOR_
#define _MZXML_RECALIBRATOR_
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>

#include <libxml/xmlversion.h>
#include <libxml/xmlIO.h>
#include <libxml/parser.h>
#include <libxml/entities.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlerror.h>

#include "mzXMLRecalibratorOptions.h"
#include "mzXMLUtils.h"
#include "guessCalibInterval.h"

#define VERSION "0.7"
#define DEFAULT_XML_VERSION "1.0"

#define TRUE  1
#define FALSE 0
#define ON    1
#define OFF   0

/* the value of the scanType attribute that indicates the current */
/* scan is a calibration                                          */
#define DEFAULT_CALIBRATION_TOLERANCE 0.1   
/* Percent difference from gold standard calibration before       */
/* generating a warning                                           */
#define CALIBRATION_SCANTYPE "calibration"

typedef struct _calibNode calibNode;

struct _calibNode {
   double basePeakMz;
   calibNode *next;
   calibNode *prev;
};

typedef struct _scanTagNode scanTagNode; 
typedef scanTagNode *scanTagNodePtr;

struct _scanTagNode {
   scanTagNode *next;
   off_t filePosition;
   int originalScanNo;
   int trimmedScanNo;
   calibNode *mostRecentCalibration;
   off_t peaksCountStartsAt;
   int peaksCount;
   off_t peaksDataStartsAt;
   int peaksDataLength;
   short isCalib;
};

/* default values for processing flags */

#define DEFAULT_INTERVAL              20
#define DEFAULT_DONT_CALIBRATE        FALSE
#define DEFAULT_CALIBRATION_MZ        785.8426 /* GFP */
#define DEFAULT_CALIB_SCANS_ONLY      FALSE
#define DEFAULT_DONT_STRIP            FALSE
#define DEFAULT_INTERNAL_RECOG        FALSE
#define DEFAULT_RUNNINGMEAN_WINDOW    6
#define DEFAULT_STRIP_LEVEL2          FALSE
#define DEFAULT_GUESS_INTERVAL        FALSE
#define DEFAULT_STRIP_0_INTENSITIES   FALSE

struct _UserData {
   const char *outFileName;        /* used by SHA1 routines                       */
   FILE *outFilePtr;               /* the (open) output FILE *                    */
   const char *inFileName;         /* input mzXML file name                       */
   FILE *inFilePtr;                /* FILE * open on the mzXML file               */

   int dontCalibrateFlag;          /* TRUE=don't recalibrate                      */
   int stripLevel2Flag;            /* TRUE=strip msLevel="2" and higher           */
   int calibrationScansOnlyFlag;   /* TRUE=show lockspray scans, strip data       */
   int dontStripFlag;              /* TRUE=don't remove lockspray scans           */
   int mzXMLinternalCalibFlag;     /* TRUE=use mzXML info to determine locksprays */
   int guessIntervalFlag;          /* TRUE=try to guess calib interval            */
   int stripZeroIntensitiesFlag;   /* TRUE=remove peaks with 0 intensity          */

   int calibrationInterval;        /* how often do calibration scans occur        */ 
   double calibrationMZ;           /* gold standard m/z for calibration           */
   int curScanNumber;              /* OUR scan number -- excluding stripped scans */ 
   scanTagNode *scanTagListHead;   /* head of the list of pointers to scan tags   */
   scanTagNode *scanTagListTail;   /* tail of the list                            */
   const char *skipTag;            /* skip til end of -- make sure this is global */
   off_t indexTagPos;              /* file position of <index> tag                */
   off_t scanCountPos;             /* file position of beginning of scanCount attr*/
   int dataProcessingElementAdded; /* flag to tell whether we've added our dp tag */
   calibNode *calibListHead;       /* pointer to head of the calibration data list*/
   calibNode *mostRecentCalib;     /* pointer to most recent calibration          */
   int peaksTagJustProcessed;      /* have we just parsed a peaks tag?            */
   int runningMeanWindow;          /* size of the window for recalibration        */
};

typedef struct _UserData userData;
typedef userData *userDataPtr;

void myStartDocumentSAXFunc(void *ctx);
void myStartElementSAXFunc(
   void *ctx,
   const xmlChar *name,
   const xmlChar **attrs
);
void myStartElementSAXFunc(
   void *ctx,
   const xmlChar *name,
   const xmlChar **attrs
);
void myEndElementSAXFunc(void *ctx, const xmlChar *name); 
void myEndDocumentSAXFunc(void *ctx);
void myCharactersSAXFunc(
   void *ctx,
   const xmlChar *chars,
   int len
);
void mySetDocumentLocatorSAXFunc(void *ctx, xmlSAXLocatorPtr loc);
int sha1_hashFile(const char* szFileName , char *szReport );

#endif
