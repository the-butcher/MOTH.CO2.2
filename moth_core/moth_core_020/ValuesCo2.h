#ifndef ValuesCo2_h
#define ValuesCo2_h

typedef struct {
  int co2;
  float temperature;
  float humidity;
  int co2Raw; // low pass filtered co2 value
} ValuesCo2;

#endif