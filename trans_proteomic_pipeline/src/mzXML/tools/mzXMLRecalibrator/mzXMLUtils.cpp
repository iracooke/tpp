/*
     Accessories for mzXML file analysis

     Author:  Ted Holzman  (tholzman@fhcrc.org)
     Date:    December, 2004
*/

#include "mzXMLUtils.h"

/* debugging routine.  Output a mass/intensity array to file "out" */
void printMIarray(char *tag, float *arr, FILE *out) {
  char *outTag;
  int i;
  int linecount = 0;

  outTag = (tag == NULL) ? "m/i array" : tag;
  if(arr == NULL || arr[0] == -1) {
    fprintf(out,"No values in %s\n",outTag);
  } else {     
     for(i = 0; arr[i] != -1;) {
        float mass,intensity;

        mass = arr[i++];
        intensity = arr[i++];
        fprintf(
           out,
           "  %d. %s mass = %10.4lf; intensity = %12.4lf\n",
           ++linecount,outTag,(double) mass,(double) intensity
        ); 
     }
  }
}

/* Peaks are in "MI arrays" - float arrays containing pairs of */
/* floats, terminated by a -1.0.  This returns the COUNT (last */
/* index+1) of elements in peak array, INCLUDING terminal -1   */
  
int peakArrSize(float arr[]) {
  int tmp=0;

  if(arr != NULL) {
    while(arr[tmp] != -1.0) tmp++;
  }
  return tmp+1;
}

/* returns the last valid index (excluding -1.0) of a peak array */
int lastValidPeakArrIndex(float arr[]) {
  int tmpCount;

  tmpCount = peakArrSize(arr);
  return ((tmpCount > 1) ? (tmpCount - 2) : 0);
}

float *newMIArrWithoutZeroIntensities(float *oldArr) {
  int i;
  PeakPtr peaks = (PeakPtr) oldArr;
  int n_nonzero_peaks = 0; 
  float *newFloats;
  PeakPtr newPeaks;

  /* count peaks with non-zero intensities */
  for(i=0; peaks[i].mass != -1.00; i++) {
    if(peaks[i].intensity>0) n_nonzero_peaks++;
  } 
  /* allocate space for new array */
  newFloats = (float *) malloc(n_nonzero_peaks*sizeof(Peak)+(1*sizeof(float)));
  if(newFloats == NULL) {
    fprintf(stderr,"Error - can't allocate memory in newMIArrWithoutZeroIntensities\n");
    return NULL;
  }
  /* fill new array with non-zero peaks */
  newPeaks = (PeakPtr) newFloats;
  n_nonzero_peaks = 0;
  for(i=0; peaks[i].mass != -1.00; i++) {
    if(peaks[i].intensity>0) {
      newPeaks[n_nonzero_peaks].mass = peaks[i].mass;
      newPeaks[n_nonzero_peaks].intensity = peaks[i].intensity;
      n_nonzero_peaks++;
    }
  }
  /* emplace terminal -1.00 and return */
  newPeaks[n_nonzero_peaks].mass = -1.00;
  return newFloats;
}

/* convert m/i style array of floats back into base64 */
/* note: allocated output array; user must deallocate */
char *miArrToBase64(float *MIarr_in) {
  int lastMIindex,i;
  char *b64_out;
  
  multiType tmpArr;

  if(MIarr_in == NULL) return NULL;
  lastMIindex = lastValidPeakArrIndex(MIarr_in);
  /* allocate temporary array in which to reverse float-bytes for network */
  tmpArr.qua_float = (float *) malloc((lastMIindex+1)*sizeof(float));
  if(tmpArr.qua_float == NULL) {
    fprintf(stderr,"Cannot allocate memory for temporary array in MIarr2Base64\n");
    exit(1);
  }
  /* copy floats into array and reverse bytes */
  for(i=0;i<=lastMIindex;i++) {tmpArr.qua_float[i] = MIarr_in[i];}
  for(i=0;i<=lastMIindex;i++) tmpArr.qua_int[i] = htonl(tmpArr.qua_int[i]);
  /* allocate output array, estimate 4/3 size increase */
  b64_out = (char *) malloc(((lastMIindex+1)*sizeof(float)*4/3)+4);
  if(b64_out == NULL) {
    fprintf(stderr,"Cannot allocate memory for output array in MIarr2Base64\n");
    exit(1);
  }
  /* copy floats into array and reverse bytes */
  i=b64_encode(b64_out,tmpArr.qua_char,(lastMIindex+1)*sizeof(float));
  b64_out[i] = '\0';
  free(tmpArr.qua_float);
  return b64_out;
}

/* note: allocated output array; user must deallocate */
float *base64ToMIarr(char *b64) {

  multiType schizoIn;
  multiType outArr;  
  int n_decoded; 
  int floatArrSize;  
  int i;

  schizoIn.qua_char = (unsigned char *)malloc((strlen(b64)+1)*sizeof(unsigned char));
  n_decoded = b64_decode_mio(schizoIn.qua_char,b64);
  floatArrSize = (n_decoded/sizeof(float))+1;
  outArr.qua_float = (float *)malloc(floatArrSize*sizeof(float));
  for(i=0;i<floatArrSize;i++){
     outArr.qua_int[i] = ntohl(schizoIn.qua_int[i]);
  }
  outArr.qua_float[floatArrSize-1] = -1.0;
  free(schizoIn.qua_char);
  return outArr.qua_float;
}

/* note: allocated output array; user must deallocate */
/* posn is position file to read from.  posn=-1 means */
/* read from the current position                     */
float *readMIarrFromBase64(FILE *from, off_t posn, unsigned long len){

   char *buffer = (char *)malloc((len+1)*sizeof(char));
   float *retarr;  

   if(posn != -1) fseeko(from,posn,SEEK_SET);
   fread(buffer,sizeof(char),len,from);
   buffer[len] = '\0';
   retarr = base64ToMIarr(buffer);
   free(buffer);
   return(retarr);
}

/* posn is position file to read from.  posn=-1 means */
/* write to the current position                      */
void writeMIarrAsBase64(FILE *to, off_t posn, float *miArr) {
   
   char *outarr = miArrToBase64(miArr);
   int len = lastValidPeakArrIndex(miArr)+1;
  
   if(posn != -1) fseeko(to,posn,SEEK_SET);
   fwrite(outarr,sizeof(char),len*sizeof(float),to);
   free(outarr);
}
