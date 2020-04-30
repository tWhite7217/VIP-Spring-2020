#include <Wire.h>
#include "Adafruit_TCS34725.h"

// Pick analog outputs, for the UNO these three work well
// use ~560  ohm resistor between Red & Blue, ~1K for green (its brighter)
#define redpin 3
#define greenpin 5
#define bluepin 6
#define MINIMUM 24    //On/Off threshold for summed RGBC values
#define HANDSHAKE_LEN 10
#define FREQ 150       //Desired frequency of sensor -- Must be <= 150?
#define SYS_CLOCK 16000000  //Frequecny of the system clock
#define PRESCALER 1   //System clock prescaler
//What the prescaled timer1 has to count to for an interrupt to occur
//#define COMP_REG SYS_CLOCK/(PRESCALER * FREQ) - 1
//#define ARR_SIZE 10
// for a common anode LED, connect the common pin to +5V
// for common cathode, connect the common to ground

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_2_4MS, TCS34725_GAIN_4X);

//global variables
//int ind = 0;
//int arr_size;
//int sum = 0;
//int colorSum = 0;
//int handshakeSum = 0;
//int handshakeStage = 0;
//int bits = 0;
//boolean started = false;
//boolean handshake = false;
//uint16_t red, green, blue, clear;
int comp_reg = SYS_CLOCK/((uint16_t)PRESCALER * (uint16_t)FREQ) - 1;
//int handshakeSwitch = 0;
int alternator = 0;

void setup() {

  Serial.begin(2400);

  pinMode(redpin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  pinMode(bluepin, OUTPUT);

  cli();//stop interrupts

  int prescalerBits = 0;
  
  if (PRESCALER == 1) {
    prescalerBits = 1;
  } else if (PRESCALER == 8) {
    prescalerBits = 2;
  } else if (PRESCALER == 64) {
    prescalerBits = 3;
  } else if (PRESCALER == 256) {
    prescalerBits = 4;
  } else { //1024 default
    prescalerBits = 5;
  }
  
  //set timer1 interrupt at FREQ
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for FREQ increments
  OCR1A = comp_reg;// must be <65536
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set 64 prescaler
  TCCR1B = (TCCR1B & 0b11111000) | prescalerBits;
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();//allow interrupts
  
}

ISR(TIMER1_COMPA_vect) { //timer1 interrupt 1Hz toggles pin 13 (LED)
  //Reads most recent reading from loop and adds it to polling array.
  //When array is full, its sum is used to determine if the last reading
  //of the bulb was on or off
  alternator = !alternator;
  Serial.println(alternator);
}

void loop() {
  
}
