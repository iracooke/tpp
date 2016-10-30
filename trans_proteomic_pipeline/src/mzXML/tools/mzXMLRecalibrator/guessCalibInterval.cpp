/**
/*  guessCalibInterval
/* 
/*  Guess the interval at which lock-spray scans occur in mzXML
/*  files generated from Waters mass spectrometers
/*  
/*  Collect basePeakMZ values from scans.  Compare them to a
/*  gold standard.  Note the ones that fall within a given
/*  tolerance of the gold standard.  Return the (smallest) interval 
/*  at which the tolerated basePeakMZ occur.  
/*
/*  Uses RAMP
/*
/*  Author
/*
/*  Ted Holzman
/*  FHCRC
/*  tholzman@fhcrc.org
/*  Feb 2005
/*
**/

#include <math.h>
#include "guessCalibInterval.h"

int val_within_tolerance(double val, double goldstd, double tolerancePct) {
  double delta = fabs(goldstd-val);
  double tolerance = goldstd*tolerancePct/100.0;
  return (delta<=tolerance);
}


#define FAILURE_SCORE_WEIGHT 2

int guessCalibInterval(const char *mzFname, double goldStandard, double tolerancePct) {

  unsigned char *withinTolScans;
  int scanCount = 0;
  off_t indexOffset;
  off_t *scanLocs;
  unsigned int i;
  int bestInterval = 0;
  int currentInterval;
  int bestIntervalCount = 0;

  // open file
  FILE *mzFILE = fopen(mzFname,"r");
  if(mzFILE == NULL) return 0;

  // find out how many scans there are
  indexOffset = getIndexOffset(mzFILE);  
  if(indexOffset == (off_t) 0) {
    fprintf(stderr,"Warning - getIndexOffset failed in guessCalibInterval\n");
    return 0;
  }

  scanLocs = readIndex( mzFILE , indexOffset, &scanCount );
  if(scanLocs == NULL) {
    fprintf(stderr,"Warning - readIndex failed in guessCalibInterval\n");
    return 0;
  }
  
  // allocate array to hold scan-meets-tolerance booleans
  withinTolScans = (unsigned char *) malloc(sizeof(unsigned char)*scanCount); 
  if(withinTolScans == NULL) {
    fprintf(stderr,"Warning - malloc failed in guessCalibInterval\n");
    return 0;
  }

  // loop through scans, fill array
  for(i=0; i<scanCount; i++) {
     struct ScanHeaderStruct scanHeader;

     readHeader(mzFILE, scanLocs[i], &scanHeader);
     withinTolScans[i] = val_within_tolerance(scanHeader.basePeakMz,goldStandard,tolerancePct);
  }
  // loop through guesses SMALLEST_POSSIBLE_INTERVAL through scanCount/2
  // find maximum n of meets-tolerances at these guesses

  for(currentInterval=SMALLEST_POSSIBLE_INTERVAL; currentInterval < scanCount/2; currentInterval++) {
    int currentIntervalTolCount = 0;
    int scanIterator;
    int nwrong = 0;
    
    for(scanIterator=1; scanIterator<scanCount; scanIterator+=currentInterval) {
       currentIntervalTolCount+=withinTolScans[scanIterator];
       nwrong += !withinTolScans[scanIterator];
    }

    //fprintf(stdout,"Interval: %d; currentIntervalCount: %d; number wrong: %d; right-wrong: %d; current best interval: %d\n",
    //	currentInterval,currentIntervalTolCount,nwrong,currentIntervalTolCount-(nwrong*FAIURE_SCORE_WEIGHT),bestInterval);

    currentIntervalTolCount -= (nwrong*FAILURE_SCORE_WEIGHT);
    if(currentIntervalTolCount>bestIntervalCount) {
       bestIntervalCount = currentIntervalTolCount;
       bestInterval = currentInterval;
    }
  }

  // clean up - close file - free memory
  fclose(mzFILE);
  free(withinTolScans);
  free(scanLocs);

  // return interval with max n of meets-tolerances
  //    0 means failure
  return bestInterval;
}
