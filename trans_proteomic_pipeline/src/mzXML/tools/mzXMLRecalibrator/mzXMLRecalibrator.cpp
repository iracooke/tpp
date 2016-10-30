/**
/*  mzXMLRecalibrator
/* 
/*  strip lockspray scans from mzXML files generated from Waters machines
/*  recalibrate data scans
/*  several other minor mzXML utilities
/*
/*  Author
/*
/*  Ted Holzman
/*  FHCRC
/*  tholzman@fhcrc.org
/*  Feb 2005
/*
**/

#include <stdio.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/xmlmemory.h>
#include <libxml/xmlsave.h>

#include "mzXMLRecalibrator.h"
#define USERDATA ((userDataPtr)(((xmlParserCtxtPtr)ctx)->_private))
#define OUTFILE USERDATA->outFilePtr


/**** utility routines ****/

void usage() {
   fprintf(stderr,"Usage:\n    mzXMLRecalibrator [<options>] <inputfilename> [<outputfilename>]\n");
   fprintf(stderr,"Options are: \n");
   fprintf(stderr,"   -i nnn  calibration scan interval    (default nnn = %d)\n",DEFAULT_INTERVAL);
   fprintf(stderr,"   -r      don't recalibrate            (default = %s)\n",    DEFAULT_DONT_CALIBRATE ?      "ON" : "OFF");
   fprintf(stderr,"   -m fff  calibration M/Z              (default fff = %f)\n",DEFAULT_CALIBRATION_MZ);
   fprintf(stderr,"   -c      calibration scans only       (default = %s)\n",    DEFAULT_CALIB_SCANS_ONLY ?    "ON" : "OFF");
   fprintf(stderr,"   -s      don't strip calib scans      (default = %s)\n",    DEFAULT_DONT_STRIP ?          "ON" : "OFF");
   fprintf(stderr,"   -x      use mzXML to get calib scans (default = %s)\n",    DEFAULT_INTERNAL_RECOG ?      "ON" : "OFF");
   fprintf(stderr,"   -z      strip peaks of intensity 0   (default = %s)\n",    DEFAULT_STRIP_0_INTENSITIES ? "ON" : "OFF");
   fprintf(stderr,"   -w nnn  size of recalibration window (default nnn = %d)\n",DEFAULT_RUNNINGMEAN_WINDOW);
   fprintf(stderr,"   -2      strip msLevel 2 and higher   (default = %s)\n",    DEFAULT_STRIP_LEVEL2 ?        "ON" : "OFF");
   fprintf(stderr,"   -g      guess calibration interval   (default = %s)\n",    DEFAULT_GUESS_INTERVAL ?      "ON" : "OFF");
}

/* set a sax handler struct to initial */
void initializeSAXHandler(xmlSAXHandlerPtr theHandler) {
   memset(theHandler,0,sizeof(xmlSAXHandler));
   theHandler->initialized=1;
}

/* set a sax handler struct to nominal */
void setNominalSAXHandler(xmlSAXHandlerPtr theHandler) {
   initializeSAXHandler(theHandler);
   theHandler->startElement       = *(myStartElementSAXFunc);
   theHandler->endElement         = *(myEndElementSAXFunc);
   theHandler->characters         = *(myCharactersSAXFunc);
   theHandler->setDocumentLocator = *(mySetDocumentLocatorSAXFunc);
   theHandler->startDocument      = *(myStartDocumentSAXFunc);
   theHandler->endDocument        = *(myEndDocumentSAXFunc);
}

/* obtain the value of key from k/v/null-term style arrays */
char *getValueFromKey(const char *key,const xmlChar **attrs) {
    int index=0;
   
   if(attrs == NULL) return NULL; 
    while(attrs[index] != 0) {
       if(strcmp((char *)attrs[index],key)==0) { 
         return (char *) attrs[++index];
      } else {
         index += 2;
      }
   }   
    return NULL;
}

/* return index of key's value in attrs */
int getIndexFromKey(const char *key,const xmlChar **attrs) {
   int index=0;
   
   if(attrs == NULL) return -1; 
      while(attrs[index] != 0) {
         if(strcmp((char *)attrs[index],key)==0) { 
            return ++index;
         } else {
         index += 2;
      }
   }   
   return -1;
}

/* make a new attrs array */
xmlChar **copyAttributes(const xmlChar **attrs) {

   int i;
   xmlChar **newAttrs;

   if(attrs == NULL) return NULL;
   for(i=0; attrs[i] != 0; i++);
   newAttrs = (xmlChar **)malloc((i+1)*sizeof(xmlChar *));
   for(i=0; attrs[i] != 0; i++) {
      newAttrs[i] = (xmlChar *) attrs[i];
   }
   newAttrs[i] = 0;
   return newAttrs;
}

/* replace key's value with value in attrs */
int replaceAttribute(const xmlChar **attrs, const char *key, const char *value){
   int index = getIndexFromKey(key,attrs);
   if(index == -1) {
      return 0;
   } else {
      attrs[index] = value;
     return 1;
   }
}

/* analysis-like routines */

/* look at current peaks.  Strip zeros.  Rewrite.  Truncate remainder of OUTFILE */
/* store appropriate information in scanTagListTail. Return allocated space      */

void stripZeroIntensities(void *ctx) {
  /* look */
  int filDesc = fileno(OUTFILE);
  float *fullOfZeros = 
     readMIarrFromBase64(
        OUTFILE,
        USERDATA->scanTagListTail->peaksDataStartsAt,
        USERDATA->scanTagListTail->peaksDataLength
     );
  /* strip */
  float *emptyOfZeros = newMIArrWithoutZeroIntensities(fullOfZeros);
  char *b64nonZero = miArrToBase64(emptyOfZeros);
  int n_of_nonzero_peaks = (lastValidPeakArrIndex(emptyOfZeros)+1)/2;

  /* rewrite */
  fseeko(OUTFILE,USERDATA->scanTagListTail->peaksDataStartsAt,SEEK_SET);
  fprintf(OUTFILE,"%s",b64nonZero);
  fflush(OUTFILE);

  /* truncate */
  ftruncate(filDesc,ftello(OUTFILE));

  /* store */
  USERDATA->scanTagListTail->peaksCount = n_of_nonzero_peaks;
  USERDATA->scanTagListTail->peaksDataLength = strlen(b64nonZero);

  /* return space */
  free(fullOfZeros);
  free(emptyOfZeros);
  free(b64nonZero);
}

/**** SAX parser handlers ****/

void myStartDocumentSAXFunc(void *ctx){
   const char *version = DEFAULT_XML_VERSION; 
   xmlParserCtxtPtr pParserContext = (xmlParserCtxtPtr)ctx;
   const char *encoding = NULL;
   
   if(pParserContext != NULL) {
      version = pParserContext->version;
      encoding= pParserContext->encoding;
   } 
   fprintf(OUTFILE,"<?xml version=\"%s\" ",version);
   if(encoding != NULL) {
      fprintf(OUTFILE,"encoding=\"%s\" ",encoding);
   }
   fprintf(OUTFILE,"?>\n");
}

void myEndDocumentSAXFunc(void *ctx) {
   fprintf(OUTFILE,"\n");
}

// Is current scan within tolerance?
// i.e. is it likely to be a "real" calibration scan?
int scan_within_tolerance(void *ctx, const xmlChar **attrs) {
  double val = atof(getValueFromKey("basePeakMz",attrs));
  double goldstd = USERDATA->calibrationMZ;
  return val_within_tolerance(val,goldstd,DEFAULT_CALIBRATION_TOLERANCE);
}

int isCalibrationScan(
   void *ctx,
   const xmlChar **attrs) {

   int retVal = 0;   
   int reportedScanNumber;
   /* -x takes precedence over -i */
   if(USERDATA->mzXMLinternalCalibFlag) {
      char *scanType = getValueFromKey("scanType",attrs);
      retVal = scanType != NULL && (strcmp(scanType,CALIBRATION_SCANTYPE) == 0);
   } else {
      reportedScanNumber = atoi(getValueFromKey("num",attrs));
      retVal = (reportedScanNumber % USERDATA->calibrationInterval) == 1;
   }
	   
   return retVal;
}

/* return 1 if we should process the scan, 0 ifn't */
int useScan(
   void *ctx,
   const xmlChar **attrs) {
   int reportedScanNumber = atoi(getValueFromKey("num",attrs));

   /* strip non-msLevel=1 scans?  -2 takes precedence over -s */
   if(USERDATA->stripLevel2Flag) {
      char *msLevel = getValueFromKey("msLevel",attrs);
      if(msLevel != NULL && strcmp(msLevel,"1") != 0) return 0;
   }

   /* dontStripFlag takes precedence over rest */
   if(USERDATA->dontStripFlag) return 1;

   if(USERDATA->calibrationScansOnlyFlag){
      return isCalibrationScan(ctx,attrs);
   } else {
      return !isCalibrationScan(ctx,attrs);
   }
}

int processScan(   
   void *ctx,
   const xmlChar **attrs) {

   char *numVal;
   int isCalib;

   numVal = getValueFromKey("num",attrs);
   if(numVal == NULL) return 0;   /* don't use scans without a valid scan number */

   /* process calibration scans */
   if(isCalib = isCalibrationScan(ctx,attrs)) {
     char *bpmz = getValueFromKey("basePeakMz",attrs);
     if(bpmz != NULL) {
        calibNode *curCalib = (calibNode *)malloc(sizeof(calibNode));
        curCalib->basePeakMz = (double) atof(bpmz);
        curCalib->next = NULL;
        if(USERDATA->calibListHead == NULL) {   //first node
	   USERDATA->calibListHead = curCalib;
           curCalib->prev = NULL;
           USERDATA->mostRecentCalib = curCalib;
        } else {                                //not first node
           curCalib->prev = USERDATA->mostRecentCalib;
           USERDATA->mostRecentCalib->next = curCalib;
           USERDATA->mostRecentCalib = curCalib;
	}
        if(!USERDATA->dontCalibrateFlag && isCalib && !scan_within_tolerance(ctx,attrs)) {
           fprintf(
              stderr,
              "Warning - Scan %d is considered a calibration scan but is more than %6.2f%% different from %f (gold standard)\n",
              atoi(numVal),DEFAULT_CALIBRATION_TOLERANCE,USERDATA->calibrationMZ
           );
        }
      }      
   }   

   /* if scan is good and we're not skipping it */
   if(useScan(ctx,attrs)) {
       int reportedScanNumber = atoi(numVal);
       scanTagNode *curFilePos = (scanTagNode *) malloc(sizeof(scanTagNode));
 
       USERDATA->curScanNumber++;
       fflush(OUTFILE);
       curFilePos->filePosition = ftello(OUTFILE);
       curFilePos->originalScanNo = reportedScanNumber;
       curFilePos->trimmedScanNo = USERDATA->curScanNumber;
       curFilePos->isCalib = isCalib;
       curFilePos->next = NULL;
       curFilePos->mostRecentCalibration = USERDATA->mostRecentCalib;
       if(USERDATA->scanTagListHead == NULL) {
          USERDATA->scanTagListHead = curFilePos;
       } else {
          USERDATA->scanTagListTail->next = curFilePos;
       }
       USERDATA->scanTagListTail = curFilePos;
       return 1;
    } else {
       return 0;
    }
}  

void WriteNewIndexTagAndOffsets(void *ctx) {
   scanTagNode *curPtr = USERDATA->scanTagListHead;
    
   fseeko(OUTFILE,USERDATA->indexTagPos,SEEK_SET);
   fprintf(OUTFILE,"  <index name=\"scan\">\n");
   while(curPtr != NULL) {
      fprintf(
         OUTFILE,
         "    <offset id=\"%d\">%llu</offset>\n",
         curPtr->trimmedScanNo,
         curPtr->filePosition
      );
      curPtr = curPtr->next;
   }
   fprintf(OUTFILE,"  </index>\n");
   fflush(OUTFILE);
}   

void processIndex(void *ctx) {
   fflush(OUTFILE);
   USERDATA->indexTagPos = ftello(OUTFILE);
   if(!USERDATA->dontCalibrateFlag) {
      recalibrate(USERDATA);
   }

   WriteNewIndexTagAndOffsets(ctx);
}

void processIndexOffset(void *ctx) {
   fprintf(OUTFILE,"  <indexOffset>%llu</indexOffset>\n",USERDATA->indexTagPos);
}

void reWriteScanCount(void *ctx) {
   off_t position;
   char buffer[20];

   position=(USERDATA->scanCountPos);
   sprintf(buffer,"\"%d\"",USERDATA->curScanNumber);
   fseeko(OUTFILE,position,SEEK_SET);
   fprintf(OUTFILE,"%-12s",buffer);
   fflush(OUTFILE);
   fseeko(OUTFILE,0L,SEEK_END);
}

void reWriteCurrentPeaksCount(void *ctx) {
  off_t position;
  char buffer[20];

  position=USERDATA->scanTagListTail->peaksCountStartsAt;
  sprintf(buffer,"\"%d\"",USERDATA->scanTagListTail->peaksCount);
  fseeko(OUTFILE,position,SEEK_SET);
  fprintf(OUTFILE,"%-12s",buffer);
  fflush(OUTFILE);
  fseeko(OUTFILE,(off_t) 0L,SEEK_END);
}

#define BUF_SIZE 1000
void processSHA1(void *ctx) {

   char SHA1[BUF_SIZE];
  
   reWriteScanCount(ctx);
   fprintf(OUTFILE,"  <sha1>");
   fflush(OUTFILE);
   SHA1[0] = '\0';
   sha1_hashFile(USERDATA->outFileName,SHA1);
   fprintf(OUTFILE,"%s</sha1>\n",SHA1);
}

/* This routine writes out the info for the element 'name' with attributes 'attrs'   */
/* The 'returnThisAttrPos' is kindof dumb but saves a lot of coding.  When writing   */
/* out the attribute whose name is stored in 'returnThisAttrPos', writeElement stores*/
/* the file position of this attribute and later returns it as a function return     */
/* What happens if you need the file position of 2 or more attributes?  You rethink  */
/* this kludge.                                                                      */

off_t writeElement(
   void *ctx,
   const xmlChar *name,
   const xmlChar **attrs,
   const char *returnThisAttrPos) {

   int i=0;
   off_t attrPos = (off_t) 0;

   fprintf(OUTFILE,"<%s",name);
 
   if(attrs != NULL) {
      while(attrs[i] != 0) {
         if(attrs[i] != NULL){
            fprintf(OUTFILE," %s=",attrs[i]);
         } else {
            break;
         }
         fflush(OUTFILE);
         if(attrs[i] != NULL){
	    if(returnThisAttrPos != NULL && strcmp(returnThisAttrPos,attrs[i]) == 0) {
	      attrPos = ftello(OUTFILE);
            }
            i++;
            fprintf(OUTFILE,"\"%s\"",attrs[i++]);
         } else { 
            break;
	 }
      }
   }
   fprintf(OUTFILE,">");
   return attrPos;
}

void myStartElementSAXFunc(
   void *ctx,
   const xmlChar *name,
   const xmlChar **attrs) {

   if((USERDATA->skipTag) != NULL) return;

   /* process <scan> tags */
   if(xmlStrcmp(name,(xmlChar *)"scan")==0){
      if(!processScan(ctx,attrs)){
         USERDATA->skipTag = "scan";
      } else {
         xmlChar **newAttrs = copyAttributes(attrs);
         char newScanNo[10];

         /* Leave space in output file for new peakscount    */
         /* in case we do something to change it (e.g. -z)   */
         /* 10 spaces (1 billion peaks) ought to be enough   */
         char *newPeaksCount = "          ";         
         
         sprintf(newScanNo,"%d",USERDATA->curScanNumber);
         if(!replaceAttribute((const xmlChar **) newAttrs,"num",newScanNo)) {
            fprintf(stderr,"Warning - no scan number found in <scan> tag.\n");
            return;
         }

         replaceAttribute((const xmlChar **) newAttrs,"peaksCount",newPeaksCount);
         USERDATA->scanTagListTail->peaksCountStartsAt = 
            writeElement(ctx,name,(const xmlChar **) newAttrs,"peaksCount");

         /* save current peaksCount */
         sscanf((char *)attrs[getIndexFromKey("peaksCount",attrs)],"%d",&(USERDATA->scanTagListTail->peaksCount));

         free(newAttrs);
      }
      return;
   }

   /* process <msRun> tag -- leave space large enough to replace scancount when we know it */
   if(xmlStrcmp(name,(xmlChar *)"msRun") == 0) {
     xmlChar **newAttrs = copyAttributes(attrs); 
     char *newScanCount = "          ";  /* 10 spaces (1 billion scans) ought to be enough */

     if(replaceAttribute((const xmlChar **) newAttrs,"scanCount",newScanCount)) {
         USERDATA->scanCountPos =
	   writeElement(ctx,name,(const xmlChar **)newAttrs,"scanCount");
     } else {
        fprintf(stderr,"Warning - no scanCount found in msRun element.\n");
     }
     return;
   }

   /* process <msManufacturer> tags */
   if(xmlStrcmp(name,(xmlChar *)"msManufacturer") == 0) {
     char *value = getValueFromKey("value",attrs);
     if(value != NULL && strcmp(value,"Waters") != 0) {
        fprintf(stderr,"Warning - stripping calibration scans, but manufacturer is not 'Waters'\n");
     }
   } 

  /* process <index> tags */
  if(xmlStrcmp(name,(xmlChar *)"index")==0){
     processIndex(ctx);
     USERDATA->skipTag = "index";
     return;
  }

  /* process <indexOffset> tags */
  if(xmlStrcmp(name,(xmlChar *)"indexOffset")==0){
     processIndexOffset(ctx);
     USERDATA->skipTag = "indexOffset";
     return;
  }

  /* process SHA1 */
  if(xmlStrcmp(name,(xmlChar *)"sha1")==0){
     processSHA1(ctx);
     USERDATA->skipTag = "sha1";
     return;
  }

  /* process <peaks> tag */
  if(xmlStrcmp(name,(xmlChar *)"peaks")==0){
     USERDATA->peaksTagJustProcessed = 1;
  }

  /* default behavior */
  writeElement(ctx,name,attrs,NULL);
  fflush(OUTFILE);

  /* peaks tag postprocess; more must be done at "characters" processing */
  if(USERDATA->peaksTagJustProcessed) {
     USERDATA->scanTagListTail->peaksDataStartsAt = ftello(OUTFILE);
     USERDATA->scanTagListTail->peaksDataLength = 0;
  }
}

void myEndElementSAXFunc(
   void *ctx,
   const xmlChar *name) {
   if((USERDATA->skipTag) != NULL) {
      if(xmlStrcmp(name,USERDATA->skipTag) == 0) {
         USERDATA->skipTag = NULL;
      }
      return;
   }



   /* peaks element just ended */
   if(xmlStrcmp(name,(xmlChar *)"peaks") == 0) {
      USERDATA->peaksTagJustProcessed = 0;
      if(USERDATA->stripZeroIntensitiesFlag) {
	stripZeroIntensities(ctx);
      }
      /* write in (possibly new) peaksCount */
      reWriteCurrentPeaksCount(ctx);
   }

   /* default behavior */
   fprintf(OUTFILE,"</%s>",name);
   fflush(OUTFILE);

   /* add a dataprocessing entry for this program (if necessary) */
   if(xmlStrcmp(name,(xmlChar *)"dataProcessing") == 0  &&
      !USERDATA->dataProcessingElementAdded) {
       USERDATA->dataProcessingElementAdded = 1;   
       fprintf(OUTFILE,"\n  <dataProcessing>\n");
       fprintf(OUTFILE,"    <software type=\"processing\" name=\"mzXMLRecalibrator\" version=\"%s\"></software>\n",VERSION);
       fprintf(OUTFILE,"  </dataProcessing>\n");
   }
}

void myCharactersSAXFunc(
   void *ctx,
   const xmlChar *chars,
   int len) {

   if((USERDATA->skipTag) != NULL) return;

   /* if we are writing out the characters from a <peaks> tag, */
   /* save the length of the base64 string                     */                                             
   if(USERDATA->peaksTagJustProcessed) {
      USERDATA->scanTagListTail->peaksDataLength += len;
   }  

   fprintf(OUTFILE,"%.*s",len,chars);
}

void mySetDocumentLocatorSAXFunc(
   void *ctx,
   xmlSAXLocatorPtr loc){
}

/**** main parser driver and minor helpers ****/

/**
 * readPacket:
 * @mem: array to store the packet
 * @size: the packet size
 *
 * read at most @size bytes from the document and store it in @mem
 *
 * Returns the number of bytes read
 */
static int readPacket(char *mem, int size, FILE *desc) {
   int res;

   res = fread(mem, 1, size, desc);
   return(res);
}


void destroyFilePtrList(userData *ud) {
   scanTagNode *curPtr = ud->scanTagListHead;

   while(curPtr != NULL) {
      scanTagNode *tmp; 
       
      tmp = curPtr -> next;
      free(curPtr);
      curPtr = tmp;
   }
   ud->scanTagListHead = NULL;
   ud->scanTagListTail = NULL;
}   

void destroyCalibList(userData *ud) {
   calibNode *curPtr = ud->calibListHead;

   while(curPtr != NULL) {
      calibNode *tmp; 
       
      tmp = curPtr -> next;
      free(curPtr);
      curPtr = tmp;
   }
   ud->calibListHead   = NULL;
   ud->mostRecentCalib = NULL;
}   

void initializeUserContext(const char *infname, FILE *inf, const char *outfname, FILE *outf, userDataPtr udptr) {
   udptr->outFileName = outfname;
   udptr->outFilePtr = outf;
   udptr->inFileName = infname;
   udptr->inFilePtr = inf;
   udptr->curScanNumber = 0;
   udptr->scanTagListHead = NULL;
   udptr->calibrationInterval = DEFAULT_INTERVAL;
   udptr->skipTag = NULL;
   udptr->dataProcessingElementAdded = 0;
   udptr->dontCalibrateFlag = DEFAULT_DONT_CALIBRATE;
   udptr->stripLevel2Flag = DEFAULT_STRIP_LEVEL2;
   udptr->calibrationMZ = DEFAULT_CALIBRATION_MZ;
   udptr->calibrationScansOnlyFlag = DEFAULT_CALIB_SCANS_ONLY;
   udptr->dontStripFlag = DEFAULT_DONT_STRIP;
   udptr->mzXMLinternalCalibFlag = DEFAULT_INTERNAL_RECOG;
   udptr->calibListHead = NULL;
   udptr->mostRecentCalib = NULL;
   udptr->peaksTagJustProcessed = 0;
   udptr->runningMeanWindow = DEFAULT_RUNNINGMEAN_WINDOW;
   udptr->guessIntervalFlag = DEFAULT_GUESS_INTERVAL;
   udptr->stripZeroIntensitiesFlag = DEFAULT_STRIP_0_INTENSITIES;
}

void loadOptions(userDataPtr udptr,optionStruct options[]) {
  int optionIndex;

  for(optionIndex=0; options[optionIndex].option != 0; optionIndex++) {
     if(strcmp(options[optionIndex].option,"-i") == 0) {
        udptr->calibrationInterval = atoi(options[optionIndex].value);
        continue;
     }
     if(strcmp(options[optionIndex].option,"-r") == 0) {
        udptr->dontCalibrateFlag = !DEFAULT_DONT_CALIBRATE;
        continue;
     }
     if(strcmp(options[optionIndex].option,"-2") == 0) {
        udptr->stripLevel2Flag = !DEFAULT_STRIP_LEVEL2;
        continue;
     }
     if(strcmp(options[optionIndex].option,"-m") == 0) {
        udptr->calibrationMZ = atof(options[optionIndex].value);
        continue;
     }
     if(strcmp(options[optionIndex].option,"-w") == 0) {
        udptr->runningMeanWindow = atoi(options[optionIndex].value);
        continue;
     }
     if(strcmp(options[optionIndex].option,"-c") == 0) {
        udptr->calibrationScansOnlyFlag = !DEFAULT_CALIB_SCANS_ONLY;
        continue;
     }
     if(strcmp(options[optionIndex].option,"-s") == 0) {
        udptr->dontStripFlag = !DEFAULT_DONT_STRIP;
        continue;
     }
     if(strcmp(options[optionIndex].option,"-g") == 0) {
        udptr->guessIntervalFlag = !DEFAULT_GUESS_INTERVAL;
        continue;
     }
     if(strcmp(options[optionIndex].option,"-x") == 0) {
        udptr->mzXMLinternalCalibFlag = !DEFAULT_INTERNAL_RECOG;
        continue;
     }
     if(strcmp(options[optionIndex].option,"-z") == 0) {
        udptr->stripZeroIntensitiesFlag = !DEFAULT_STRIP_0_INTENSITIES;
        continue;
     }
  }
  if(udptr->guessIntervalFlag) {
    int guessedInterval = 
       guessCalibInterval(
          udptr->inFileName,
          udptr->calibrationMZ,
          DEFAULT_CALIBRATION_TOLERANCE
       );
    if(guessedInterval == 0) {
       fprintf(stderr,"Warning - could not guess the calibration interval in %s\n",udptr->inFileName);
    } else {
       if(guessedInterval != udptr->calibrationInterval) {
          fprintf(stderr, "Warning - changing calibration interval from %d to %d\n",udptr->calibrationInterval,guessedInterval);
          udptr->calibrationInterval = guessedInterval;
       }
    }
  }
}

/**
 * parserDriver:
 * @inFilename: a filename or an URL
 * @infile: a FILE * open on filename
 * @outFilename: output file name
 * @outfile: a FILE* open on the output file
 * @options: array of parser options
 * Parse and recalibrate
 */
#define CHUNKSIZE 5000
void parserDriver(const char *inFilename, FILE *infile, const char *outFilename, FILE *outfile, optionStruct options[]) {
   xmlParserCtxtPtr ctxt;
   char chars[CHUNKSIZE];
   int res;
   userData myUserData;
   xmlSAXHandler myHandler = {
    NULL, /* internalSubsetSAXFunc */
    NULL, /* isStandaloneSAXFunc */
    NULL, /* hasInternalSubsetSAXFunc */
    NULL, /* hasExternalSubsetSAXFunc */
    NULL, /* resolveEntitySAXFunc */
    NULL, /* getEntitySAXFunc */
    NULL, /* entityDeclSAXFunc */
    NULL, /* notationDeclSAXFunc */
    NULL, /* attributeDeclSAXFunc */
    NULL, /* elementDeclSAXFunc */
    NULL, /* unparsedEntityDeclSAXFunc */
    NULL, /* setDocumentLocatorSAXFunc */
    NULL, /* startDocumentSAXFunc */
    NULL, /* endDocumentSAXFunc */
    NULL, /* startElementSAXFunc */
    NULL, /* endElementSAXFunc */
    NULL, /* referenceSAXFunc */
    NULL, /* charactersSAXFunc */
    NULL, /* ignorableWhitespaceSAXFunc */
    NULL, /* processingInstructionSAXFunc */
    NULL, /* commentSAXFunc */
    NULL, /* warningSAXFunc */
    NULL, /* errorSAXFunc */
    NULL, /* fatalErrorSAXFunc */
    NULL, /* getParameterEntitySAXFunc */
    NULL, /* cdataBlockSAXFunc */
    NULL, /* externalSubsetSAXFunc */
    1,    /* initialized (int) */
    NULL, /* _private (void *) */
    NULL, /* startElementNsSAXFunc */
    NULL, /* endElementNsSAXFunc */
    NULL  /* xmlStructuredErrorFunc */
   };

   /* initialize parser */
   LIBXML_TEST_VERSION
   setNominalSAXHandler(&myHandler);
   xmlKeepBlanksDefault(0);

   /* initialize local user context */
   initializeUserContext(inFilename,infile,outFilename,outfile,&myUserData);
   loadOptions(&myUserData,options);

   /*
    * Read a few bytes to check the input used for the
    * encoding detection at the parser level.
    */
   res = readPacket(chars, CHUNKSIZE, infile);
   if (res <= 0) {
      fprintf(stderr, "Failed to parse %s\n", inFilename);
      return;
   }

   /*
    * Create a progressive parsing context, We also pass the first bytes of the 
    * document to allow encoding detection when creating the parser.   
    */
   ctxt = xmlCreatePushParserCtxt(&myHandler, NULL, chars, res, inFilename);
   
   /* insert user data stuff into context */

   ctxt->_private = &myUserData;
   if (ctxt == NULL) {
      fprintf(stderr, "Failed to create parser context !\n");
      return;
   }

   /*
    * loop on the input getting the document data
    */
   while ((res = readPacket(chars, CHUNKSIZE, infile)) > 0) {
      xmlParseChunk(ctxt, chars, res, 0);
   }

   /*
    * there is no more input, indicate the parsing is finished.
    */
   xmlParseChunk(ctxt, chars, 0, 1);

   if((ctxt->wellFormed) != 1) {
      fprintf(stderr, "Warning - document is not well-formed\n"); 
   }

   destroyFilePtrList(&myUserData);
   destroyCalibList(&myUserData); 
   xmlFreeParserCtxt(ctxt);
   xmlCleanupParser();
}

#define MAX_OPTIONS 20

static optionInfo legalOptions[] = {
   {"-i",1,NULL,NULL},{"-r",0,NULL,NULL},{"-m",1,NULL,NULL},{"-c",0,NULL,NULL},   
   {"-s",0,NULL,NULL},{"-x",0,NULL,NULL},{"-w",1,NULL,NULL},{"-2",0,NULL,NULL},
   {"-g",0,NULL,NULL},{"-z",0,NULL,NULL},
   {NULL,0,NULL,NULL}
};

int main(int argc, char **argv) {
   FILE *infile;
   FILE *outfile;

   char defaultOutFileName[1000];
   char *outFileName;
   char *inFileName;

   optionStruct options[MAX_OPTIONS];
   int nextOption = 0;

   memset(options,0,MAX_OPTIONS*sizeof(optionStruct));
   if((nextOption=getOptions(options,legalOptions,argc,argv))==-1 || (argc-nextOption) < 1 || (argc-nextOption) > 2 ){
      usage();
      return(1);
   }    

   inFileName = argv[nextOption++];
   if((infile = fopen(inFileName, "rb")) == NULL) {
      fprintf(stderr,"ERROR -- cannot open mzXML file: %s\n",inFileName);
      return 1;
   }

   /* our outfilename or yours? */
   if((argc-nextOption) == 1) {
      outFileName = argv[nextOption];
   } else {
      outFileName = createOutFileName(inFileName,defaultOutFileName,"recalibrated-",1);
   }
   if((outfile = fopen(outFileName,"wb+")) == NULL) {
     fprintf(stderr,"ERROR -- cannot open output file: %s\n",outFileName);
     return 1;
   }

   parserDriver(inFileName,infile,outFileName,outfile,options);

   fclose(infile);
   fclose(outfile);
   return(0);
}

