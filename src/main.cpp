#include <Arduino.h>

//========================================================= GLOBALS
#define PIN_CLOCK  4
#define PIN_DATA   5

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
// waits until the start of the data sequence; returns the first data bit
bool grabStart();

// condition for data high
#define IS_DATA_HIGH (digitalRead(PIN_DATA)==HIGH)
// condition for data low
#define IS_DATA_LOW (!IS_DATA_HIGH)
// the number of the data bits in each sequence
#define DATA_BITS 24
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
  Serial.println("Start read");
  readCaliper();
  Serial.println("End read");
  delay(1000);
}

//========================================================= IMPLEMENTATIONS

bool grabStart() {
  unsigned long m;
  bool start = false;
  while(!start) {
    while(IS_CLK_HIGH) {}
    m = micros();
    while(IS_CLK_LOW) {}
    if(micros() - m > CLK_START_SEQUENCE) start = true;
  }
  return IS_DATA_HIGH;
}

bool grabPulse() {
  while(IS_CLK_HIGH) {}
  while(IS_CLK_LOW) {}
  return IS_DATA_HIGH;
}

void readCaliper() {
  bool data[DATA_BITS];
  unsigned int dataIdx = 0;
  // scan for the start sequence position and grab the first bit
  data[0] = grabStart();
  // grab each data bit
  do {
    dataIdx ++;
    data[dataIdx] = grabPulse();
  } while(dataIdx < DATA_BITS-1);
  // print data bits
  for (dataIdx=0; dataIdx<DATA_BITS; dataIdx++) {
    Serial.print(data[dataIdx]);
  }
  Serial.println();
}