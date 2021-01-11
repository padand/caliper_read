/*
  caliper.cpp - Library for reading digital caliper with Arduino.
  Created by Andrei Paduraru, January 11, 2021.
  Released into the public domain (MIT license).
*/

#include <caliper.h>
#include <Arduino.h>

/*
Data format:
20 bits value
1  bits sign
3  bits error check

                     __sign (1 = negative)
                    |
 __value (base 2)__ | ____error check
|                  ||| |
000000000000000000001001
*/

//========================================================= CONFIG

// uncomment to print the binary data sequence
// #define DEBUG_DATA_BITS

// the minimum number of microseconds that signal the start sequence
#define CLK_START_SEQUENCE 500

// the number of data bits in each sequence
#define DATA_BITS  24
// the number of value bits
#define VALUE_BITS 20
// the number of sign bits, should always be 1
#define SIGN_BITS 1
// the number of verification bits
#define VERIFY_BITS 3

// sanity check
#if VALUE_BITS + ERROR_MASK_BITS + SIGN_BITS + VERIFY_BITS != DATA_BITS
  #error "Invalid data format"
#endif

#define SIGN_INDEX (VALUE_BITS + SIGN_BITS)

// decimal result of bitwise ERROR_MASK_BITS || VERIFY_BITS
// useful for checking if there was an error in acquiring data,
// like skipping bits for example
#define VERIFY_SUCCESS 4

//========================================================= CONSTRUCTOR

Caliper::Caliper(const unsigned int clkPin, const unsigned int dataPin, const unsigned int decimals) {
    this->clkPin = clkPin;
    this->dataPin = dataPin;
    this->decimalsDivide = 1;
    for(unsigned int i=0; i<decimals; i++) {
        this->decimalsDivide *= 10;
    }
    pinMode(clkPin, INPUT);
    pinMode(dataPin, INPUT);
}

inline bool Caliper::isClkHigh() {
    return digitalRead(clkPin)==HIGH;
}

inline bool Caliper::isDataHigh() {
    return digitalRead(dataPin)==HIGH;
}

void Caliper::grabStart(){
    unsigned long m;
    bool start = false;
    while(!start) {
        while(isClkHigh()) {}
        m = micros();
        while(!isClkHigh()) {}
        if(micros() - m > CLK_START_SEQUENCE) start = true;
    }
}

bool Caliper::grabPulse() {
    while(isClkHigh()) {}
    while(!isClkHigh()) {}
    return !isDataHigh();
}

bool Caliper::read(float *value) {
  bool data[DATA_BITS];
  unsigned int dataIdx = 0;
  // scan for the start sequence
  grabStart();
  // grab each data bit
  do {
    data[dataIdx] = grabPulse();
    dataIdx ++;
  } while(dataIdx < DATA_BITS);
  // print data bits
  #ifdef DEBUG_DATA_BITS
    printDataBits(data);
  #endif
  // verify data ok
  dataIdx = VALUE_BITS + SIGN_BITS;
  unsigned int dataVerify = 0;
  for(unsigned int i=0; i<VERIFY_BITS; i++) {
    dataVerify |= data[dataIdx+i]<<i;
  }
  if(dataVerify == VERIFY_SUCCESS) {
    // success; convert value to float and print
    unsigned long valueTemp = 0;
    for(unsigned int i=0; i<VALUE_BITS; i++) {
      valueTemp |= data[i]<<i;
    }
    *value = float(valueTemp) / float(decimalsDivide);
    if (data[SIGN_INDEX-1]) {
      *value *= -1.0f;
    }
    return true;
  }
  return false;
}

void Caliper::printDataBits(bool *data) {
  for (unsigned int i=0; i<DATA_BITS; i++) {
    Serial.print(data[i]);
  }
  Serial.println();
}