#ifndef _CALIBRATION_ROUTINES_

#define _CALIBRATION_ROUTINES_
#include "mzXMLRecalibrator.h"

double calibrationFactor(scanTagNodePtr curScan, userDataPtr ud);
void recalibrate(userDataPtr ud);

#endif
