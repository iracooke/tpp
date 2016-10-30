#include <stdio.h>
#include "mzXMLRecalibratorRamp.h"

#define SMALLEST_POSSIBLE_INTERVAL 10

int val_within_tolerance(double val, double goldstd, double tolerancePct);
int guessCalibInterval(const char *mzFname, double goldStandard, double tolerancePct);
