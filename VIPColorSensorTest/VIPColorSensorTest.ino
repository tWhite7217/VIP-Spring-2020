#include <Wire.h>
#include "Adafruit_TCS34725.h"

// Pick analog outputs, for the UNO these three work well
// use ~560  ohm resistor between Red & Blue, ~1K for green (its brighter)
#define redpin 3
#define greenpin 5
#define bluepin 6
#define MINIMUM 24
#define DELAY_TIME 16
#define ARR_SIZE 1

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_2_4MS, TCS34725_GAIN_4X);

int past[ARR_SIZE];
int ind = 0;
boolean started = false;
int sum = 0;


void setup() {
  Serial.begin(9600);

  if (tcs.begin()) {
    //Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1); // halt!
  }

  pinMode(redpin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  pinMode(bluepin, OUTPUT);
}


void loop() {

  uint16_t red, green, blue, clear;

  //delay(DELAY_TIME);  // takes 50ms to read

  tcs.getRawData(&red, &green, &blue, &clear);


  if (started) {
    if (clear + red + green + blue < MINIMUM) {
      past[ind] = 0;
    } else {
      past[ind] = 1;
    }
    ind = (ind + 1) % ARR_SIZE;
    if (ind == 0) {
      for (int i = 0; i < ARR_SIZE; i++) {
        sum += past[i];
      }
      if (sum >= ARR_SIZE/2 + 1) {
        Serial.println("1");
      } else {
        Serial.println("0");
      }
      sum = 0;
   }
 } 
 else {
   if (clear + red + green + blue >= MINIMUM) {
      started = true;
      past[ind] = 1;
      ind++;
   }
 }
}
