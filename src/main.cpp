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


//========================================================= GLOBALS
#define PIN_CLOCK  4
#define PIN_DATA   5

// the number used to divide the int value to get
// a fixed decimal float value
// 100 = 2 decimals
// 10  = 1 decimal
#define DECIMALS_DIVIDE   100

// if enabled it prints the data bits sequence
// #define DEBUG_DATA_BITS

//========================================================= CALIPER READOUT DEFINITIONS
// stores the value read from the caliper
float caliperValue;
// reads the value from the caliper
void readCaliper();

// condition for clock high
#define IS_CLK_HIGH  (digitalRead(PIN_CLOCK)==HIGH)
// condition for clock low
#define IS_CLK_LOW   (!IS_CLK_HIGH)
// the minimum number of microseconds that signal the start sequence
#define CLK_START_SEQUENCE 500
// waits until the start of the data sequence
void grabStart();

// condition for data high
#define IS_DATA_HIGH (digitalRead(PIN_DATA)==HIGH)
// condition for data low
#define IS_DATA_LOW (!IS_DATA_HIGH)
// returns the value of the current data bit
#define GET_BIT_VALUE IS_DATA_LOW

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

// returns the next data bit
bool grabPulse();
// the value that identifies a high pulse
#define PULSE_HIGH true
// the value that identifies a low pulse
#define PULSE_LOW false


//========================================================= SETUP
void setup() {
  Serial.begin(9600);
  pinMode(PIN_CLOCK, INPUT);
  pinMode(PIN_DATA, INPUT);
}

//========================================================= MAIN LOOP
void loop() {
  readCaliper();
}

//========================================================= IMPLEMENTATIONS

void grabStart() {
  unsigned long m;
  bool start = false;
  while(!start) {
    while(IS_CLK_HIGH) {}
    m = micros();
    while(IS_CLK_LOW) {}
    if(micros() - m > CLK_START_SEQUENCE) start = true;
  }
}

bool grabPulse() {
  while(IS_CLK_HIGH) {}
  while(IS_CLK_LOW) {}
  return GET_BIT_VALUE;
}

void printDataBits(bool *data) {\
  Serial.println("Data bits:");
  for (unsigned int i=0; i<DATA_BITS; i++) {
    Serial.print(data[i]);
  }
  Serial.println();
}

void readCaliper() {
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
    float value = float(valueTemp) / float(DECIMALS_DIVIDE);
    if (data[SIGN_INDEX-1]) {
      value *= -1.0f;
    }
    Serial.println(value);
  }
}