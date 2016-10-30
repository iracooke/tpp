#ifndef _MZXMLUTILS
#define _MZXMLUTILS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <Winsock2.h>
#define uint32_t unsigned long
#else
#include <netinet/in.h>
#endif

#include "mzXMLRecalibratorBase64.h"

struct _PeakStruct
{
   float  mass;
   float  intensity;
};

typedef struct _PeakStruct Peak;
typedef Peak *PeakPtr;

union char_or_float {
   unsigned char  *qua_char;
   float          *qua_float;
   unsigned long  *qua_int;
};

typedef union char_or_float multiType;

void printMIarray(char *tag, float *arr, FILE *out);
int peakArrSize(float arr[]);
int lastValidPeakArrIndex(float arr[]);
char *miArrToBase64(float *miArr);
float *base64ToMIarr(char *b64);
float *readMIarrFromBase64(FILE *from, off_t posn, unsigned long len);
void writeMIarrAsBase64(FILE *to, off_t posn, float *miArr);
float *newMIArrWithoutZeroIntensities(float *oldArr);
#endif
