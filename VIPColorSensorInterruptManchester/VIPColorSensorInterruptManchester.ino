#include <Wire.h>
#include "Adafruit_TCS34725.h"

// Pick analog outputs, for the UNO these three work well
// use ~560  ohm resistor between Red & Blue, ~1K for green (its brighter)
#define redpin 3
#define greenpin 5
#define bluepin 6
#define MINIMUM 24                    //On/Off threshold for added RGBC values
#define HANDSHAKE_LEN 10              //The number of bits used in handshake
#define FREQ 150                      //Desired frequency of sensor -- Must be <= 150?
#define SYS_CLOCK 16000000            //Frequecny of the system clock
#define PRESCALER 64                  //System clock prescaler
#define SECONDS_TO_EXIT_HANDSHAKE 1   //Number of seconds without transition to exit handshake
#define PARITY_LEN 1                  //The number of bytes per parity bit
#define PARITY_MODE true

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_2_4MS, TCS34725_GAIN_4X);

//Enum describes the different modes the sensor can be in
enum mode {
  OFF,          //Bubl is off, waiting for handshake
  ON,           //Bulb is on, waiting for handshake
  HANDSHAKE,    //Handshake is happening
  SYNC_STOP,    //Start bit (used to account for desynced clocks)
  SYNC_START,   //Stop bit  (used to account for desynced clocks)
  MESSAGE       //Message is being transmitted
};
int mode = OFF;

//global variables
int samplesSinceLastBit = 0;//samples since last bit
int samplesPerBit;          //samples per bit
int sum = 0;                //sum of the samples
int colorSum = 0;           //sum of the color values
float handshakeSum = 0;     //sum of interrupts since handshake started
int handshakeStage = 0;     //how many times bulb has changed since handshake started
int bits = 0;               //number of transmitted bits since last sync
int manchBit = 0;           //index of manchester array
int manchester[2];          //array used to determine transition for manchester encoding
int continuous0s = 0;       //used to check if transmission has stopped
int continuous1s = 0;       //used to check if transmission has stopped
int lastSum = 0;            //handshakeSum last time the bulb changed 
int sync;                   //a count of interrupts stop bit occurred
int onOrOff;                //Whether bulb was on or off when handshake started
uint16_t red, green, blue, clear;
//What the prescaled timer1 has to count to for an interrupt to occur
int comp_reg = SYS_CLOCK/((uint16_t)PRESCALER * (uint16_t)FREQ) - 1;
int handshakeSwitch = 0;    //0 when bulb is on during handshake, 1 when bulb is off
int thresh = 0;             //the number sum must be greater than for samples to count as 1 bit

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

/*
 * Setup for entering handshake mode after waiting.
 * onOrOff is 0 if handshake begun with light turning on,
 *            1 if handshake begun with light turning off
 */
void enterHandshake(){
  //Reset HANDSHAKE variables
  mode = HANDSHAKE;
  handshakeSwitch = !onOrOff;
  handshakeSum = 0;
  handshakeStage = onOrOff;
  lastSum = 0;

  //Reset MESSAGE variables
  sum = 0;
  continuous0s = 0;
  continuous1s = 0;
  samplesSinceLastBit = 0;
  manchBit = 0;
  bits = 0;
}

/*
 * Increments the state of the handshake.
 * If handshakeStage is greater than HANDSHAKE_LEN,
 * the handshake is over and we enter SYNC_STOP mode.
 * Called when lightbulb changes state during handshake.
 */
void incrementHandshake() {
  handshakeStage++;
  handshakeSwitch = !handshakeSwitch;
  lastSum = handshakeSum;
  if (handshakeStage >= HANDSHAKE_LEN) {
    samplesPerBit = (int) round(handshakeSum / ((HANDSHAKE_LEN - onOrOff)*2) + 0.2);
    thresh = (int) round(((float)samplesPerBit) / 2);
    Serial.println(handshakeSum);
    Serial.println(samplesPerBit);
    mode = SYNC_STOP;
    sync = 0;
  }
}

//Timer1 interrupt, set to FREQ
ISR(TIMER1_COMPA_vect) {
  switch(mode) {
    //Off Mode. The sensor waits for the handshake to occur.
    //Handshake mode is entered when the bulb first turns on.
    case OFF:
      if (colorSum >= MINIMUM) {
        onOrOff = 0;
        enterHandshake();
      }
      break;
    //On Mode. The sensor waits for the handshake to occur.
    //Handshake mode is entered when the bulb first turns off.
    case ON:
      if (colorSum < MINIMUM) {
        onOrOff = 1;
        enterHandshake();
      }
      break;
    //Handshake mode. Used to determine the correct samples/bit
    case HANDSHAKE:
      //The bulb is on. When it turns off, we increase handshakeStage
      //changing handshakeSwitch
      if (handshakeSwitch == 1) {
        //We also check that handshakeSum has incremented enough
        //since handshakeSwitch changed to avoid immediate switches
        if (colorSum < MINIMUM && handshakeSum > lastSum + 3) {
          incrementHandshake();
        }
      }
      //The bulb is off. When it turns on, we increase handshakeStage,
      //changing handshakeSwitch. If handshakeStage is greater than
      //HANDSHAKE_LEN when the bulb is turned on, the hand shake is
      //over and we enter Stop bit mode.
      else {
        if (colorSum >= MINIMUM && handshakeSum > lastSum + 3) {
          incrementHandshake();
        }
      }
      //handshakeSum is incremented every interrupt in handshake mode
      handshakeSum++;
      //If the bulb has not changed state in the time specified by
      //SECONDS_TO_EXIT_HANDSHAKE, the bulb returns to waiting for
      //a handshake by entering either ON or OFF mode (based on
      //current state of the bulb, handshakeSwitch)
      if (handshakeSum - lastSum > FREQ * SECONDS_TO_EXIT_HANDSHAKE) {
        mode = handshakeSwitch;
        Serial.println("Handshake exited.");
      }
      break;
    //Stop bit. Move on to start bit once light is turned off
    //and sync is greater than half the determined samples/bit.
    //The second condition stops us from entering stop mode
    //if we enter start mode while the bulb is still off
    //and the stop bit has not yet appeared
    case SYNC_STOP:
      sync++;
      if (colorSum < MINIMUM && sync > samplesPerBit/2) {
        mode = SYNC_START;
        sync = 0;
      } else if (sync > samplesPerBit * 5) {
        mode = ON;
      }
      break;
    //Start bit. We are resynced once we enter this mode,
    //so we wait for the determined number of samples/bit
    //and then move back to started mode.
    case SYNC_START:
      sync = (sync+1) % samplesPerBit;
      if (sync == 0){
        mode = MESSAGE;
      }
      break;
    case MESSAGE:
      if (colorSum >= MINIMUM) {
        sum++;
      }
      //When the determined number of samples/bit have been taken,
      //the bit is considered a 1 if sum is greater than thresh, 0 otherwise
      samplesSinceLastBit = (samplesSinceLastBit + 1) % samplesPerBit;
      if (samplesSinceLastBit == 0) {
        //The value of the bit is added to the manchester array
        if (sum >= thresh) {
          manchester[manchBit] = 1;
        } else {
          manchester[manchBit] = 0;
        }
        //When the array is full, we determine the data of the past two bits
        manchBit = (manchBit + 1) % 2;
        if (manchBit == 0) {
          //The data is 1 if we transitioned from 0 to 1
          if (manchester[0] == 0 && manchester[1] == 1) {
            Serial.println("1");
            continuous0s = 0;
            continuous1s = 0;
          //The data is 0 if we transitioned from 1 to 0
          } else if (manchester[0] == 1 && manchester[1] == 0) {
            Serial.println("0");
            continuous0s = 0;
            continuous1s = 0;
          //When we stay low, we guess 0 and check if transmission has stopped
          } else if (manchester[0] == 0 && manchester[1] == 0) {
            Serial.println("0");
            continuous0s++;
            if (continuous0s >=6) {
              mode = OFF;
            }
            continuous1s = 0;
          //When we stay high, we guess 1
          } else {
            Serial.println("1");
            continuous1s++;
            if (continuous1s >= 6) {
              mode = ON;
            }
            continuous0s = 0;
          }
        }
        if (PARITY_MODE) {
          bits = (bits + 1) % 8;
        } else {
          //Every PARITY_LEN*2*7 + 1 transmitted bits (+1 for parity bit), sync mode is entered
          bits = (bits + 1) % (PARITY_LEN*2*7 + 1);
        }
        if (bits == 0) {
          mode = SYNC_STOP;
        }
        sum = 0;
      }
      break;
  }
}

//The loop gets color data from the sensor as fast as possible.
void loop() {

  tcs.getRawData(&red, &green, &blue, &clear);
  colorSum = red+green+blue+clear;

}
