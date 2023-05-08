#ifndef Measurement_h
#define Measurement_h

#include "ValuesCo2.h"
#include "ValuesBme.h"
#include "ValuesBat.h"

typedef struct {
  uint32_t secondstime;
  ValuesCo2 valuesCo2; 
  ValuesBme valuesBme; 
  ValuesBat valuesBat;
  bool publishable;
} Measurement;

#endif