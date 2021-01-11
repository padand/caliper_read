/*
  caliper.h - Library for reading digital caliper with Arduino.
  Created by Andrei Paduraru, January 11, 2021.
  Released into the public domain (MIT license).
*/
#ifndef CALIPER_H
#define CALIPER_H

#include "Arduino.h"

class Caliper
{
  public:
    Caliper(const unsigned int clkPin, const unsigned int dataPin, const unsigned int decimals);
    // reads the value from the caliper
    // returns true if the value is ok
    bool read(float *value);
  private:
    unsigned int clkPin, dataPin, decimalsDivide;
    // returns true if clock is high
    bool isClkHigh();
    // returns true if data is high
    bool isDataHigh();
    // waits until the start of the data sequence
    void grabStart();
    // returns the next data bit
    bool grabPulse();
    // prints the binary data sequence
    void printDataBits(bool *data);
};

#endif