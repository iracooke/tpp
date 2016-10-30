#include "calibrationRoutines.h"

/**** Calibration routines ****/

/* return calibration factor for data described in scanTagNode */
double calibrationFactor(scanTagNodePtr curScan, userDataPtr ud) {
   int windowSize = ud->runningMeanWindow;
   calibNode *curCalib = curScan->mostRecentCalibration;
   int i,j;
   double tmp = 0.0;   

   /* go backward by windowSize/2 calibrations */
   for(i=1; i<(windowSize/2) && curCalib->prev != NULL; i++) {
      curCalib = curCalib->prev;
   } 
   
   /* go forward by 2*[the number we went backward], summing the calibration factors */
   for(j=1; j<=(i*2) && curCalib->next != NULL; j++) {
      tmp += curCalib->basePeakMz;
      curCalib = curCalib->next;
   }
   j--;
   return ud->calibrationMZ/(tmp/(float)j);
}

/* recalibrate.  Loop (i.e. trace linked list) through all scans.  Determine calib  */
/* factor for each scan.  Recalculate each peak mass.  Rewrite each peak's base64   */
/* data.  DON'T recalibrate calibration scans.                                      */
void recalibrate(userDataPtr ud) {
   scanTagNodePtr curScan = ud->scanTagListHead;

   while(curScan != NULL) {
     if(!curScan->isCalib) {
        double calibFactor = calibrationFactor(curScan,ud);
        float *peakInfo = 
           readMIarrFromBase64(ud->outFilePtr,curScan->peaksDataStartsAt,curScan->peaksDataLength);
        PeakPtr pkArray = (PeakPtr) peakInfo;
        int i;
        
        for(i=0; pkArray[i].mass != -1.0; i++) {
           pkArray[i].mass = ((double) pkArray[i].mass)*calibFactor;
        }
      
        writeMIarrAsBase64(ud->outFilePtr,curScan->peaksDataStartsAt,peakInfo);

        free(peakInfo);  
     }

     curScan = curScan->next;
   }
}
