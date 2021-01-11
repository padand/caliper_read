#include <Arduino.h>
#include<caliper.h>

Caliper caliper(4,5,2);

void setup() {
  Serial.begin(9600);
}

float v;
void loop() {
  if(caliper.read(&v)) {
    Serial.println(v);
  } else {
    Serial.println("readout error");
  }
}