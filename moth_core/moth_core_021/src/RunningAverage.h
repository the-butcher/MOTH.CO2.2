#ifndef RunningAverage_h
#define RunningAverage_h
#include <Arduino.h>

class RunningAverage {
  
  private:
    int dim = 6;
    float valueArray[6];
    int valueIndex;
    bool dirty;
    float avg;
    // float std;
    // float var;
    void recalculate();
    
  public:
    RunningAverage();
    void push(float value);
    void reset();
    float getAvg();
    // float getStd();
    // float getVar();
};

#endif