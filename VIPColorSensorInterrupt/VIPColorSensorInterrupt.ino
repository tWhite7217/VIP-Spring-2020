#include <Wire.h>
#include "Adafruit_TCS34725.h"

// Pick analog outputs, for the UNO these three work well
// use ~560  ohm resistor between Red & Blue, ~1K for green (its brighter)
#define redpin 3
#define greenpin 5
#define bluepin 6
#define MINIMUM 24    //On/Off threshold for added RGBC values
#define FREQ 150       //Desired frequency of sensor -- Must be <= 150?
#define SYS_CLOCK 16000000  //Frequecny of the system clock
#define PRESCALER 64   //System clock prescaler
//What the prescaled timer1 has to count to for an interrupt to occur
//#define COMP_REG SYS_CLOCK/(PRESCALER * FREQ) - 1
#define ARR_SIZE 10
// for a common anode LED, connect the common pin to +5V
// for common cathode, connect the common to ground

// set to false if using a common cathode LED
#define commonAnode true

// our RGB -> eye-recognized gamma color
byte gammatable[256];

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_2_4MS, TCS34725_GAIN_4X);

//global variables
int ind = 0;
int arr_size = 0;
int sum = 0;
int colorSum = 0;
int handshakeSum = 0;
int handshakeStage = 0;
int bits = 0;
int past[ARR_SIZE];
boolean started = false;
boolean handshake = false;
uint16_t red, green, blue, clear;
int comp_reg = SYS_CLOCK/((uint16_t)PRESCALER * (uint16_t)FREQ) - 1;

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

  // thanks PhilB for this gamma table!
  // it helps convert RGB colors to what humans see
  for (int i = 0; i < 256; i++) {
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
  if (started) {
    if (colorSum >= MINIMUM) {
      sum++;
    }
    ind = (ind + 1) % ARR_SIZE;
    if (ind == 0) {
      if (sum >= ARR_SIZE / 2 + 1) {
        Serial.println('1');
      } else {
        Serial.println('0');
      }
      bits = (bits + 1) % 2;
      sum = 0;
    }
  }
  else {
    if (colorSum >= MINIMUM) {
      started = true;
      sum = 1;
      ind++;
    }
  }
}

void loop() {

  tcs.getRawData(&red, &green, &blue, &clear);
  colorSum = red+green+blue+clear;

}
