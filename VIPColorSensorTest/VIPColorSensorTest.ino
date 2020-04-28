#include <Wire.h>
#include "Adafruit_TCS34725.h"

// Pick analog outputs, for the UNO these three work well
// use ~560  ohm resistor between Red & Blue, ~1K for green (its brighter)
#define redpin 3
#define greenpin 5
#define bluepin 6
#define MINIMUM 24
#define DELAY_TIME 16
#define ARR_SIZE 3
// for a common anode LED, connect the common pin to +5V
// for common cathode, connect the common to ground

// set to false if using a common cathode LED
#define commonAnode true

// our RGB -> eye-recognized gamma color
byte gammatable[256];


Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_2_4MS, TCS34725_GAIN_4X);

int past[ARR_SIZE];
int ind = 0;
boolean started = false;
int sum = 0;


void setup() {
  Serial.begin(9600);
  //Serial.println("Color View Test!");

  if (tcs.begin()) {
    //Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1); // halt!
  }

  // use these three pins to drive an LED
#if defined(ARDUINO_ARCH_ESP32)
  ledcAttachPin(redpin, 1);
  ledcSetup(1, 12000, 8);
  ledcAttachPin(greenpin, 2);
  ledcSetup(2, 12000, 8);
  ledcAttachPin(bluepin, 3);
  ledcSetup(3, 12000, 8);
#else
  pinMode(redpin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  pinMode(bluepin, OUTPUT);
#endif

  // thanks PhilB for this gamma table!
  // it helps convert RGB colors to what humans see
  for (int i=0; i<256; i++) {
    float x = i;
    x /= 255;
    x = pow(x, 2.5);
    x *= 255;

    if (commonAnode) {
      gammatable[i] = 255 - x;
    } else {
      gammatable[i] = x;
    }
    //Serial.println(gammatable[i]);
  }
}


void loop() {

  uint16_t red, green, blue, clear;

  delay(DELAY_TIME);  // takes 50ms to read

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
