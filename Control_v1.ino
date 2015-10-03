/*
Sneak Easy Control Module
Keith Jacobsen
Summer 2015
=========================
Version 0:
This program reacts to user input switches to activate
a secret passageway.  

The switches are:
 - open one
 - open all
 - close all
 
There are also bumper switches for safety and travel limit:
 - limit open
 - safety/limit close
 
 ========================
 Version 1 changes:
 Refactored for readability; eliminated flags in favor of more 
 descriptive solution.
 
 Altered bumper interrupt for a bumper poll routine.
*/

#include "LowPower.h"

//=====================================================================
//
// Variable Delcarations
//
//=====================================================================

const int USER_INT_PIN = 3;
const int BUMPER_INT_PIN = 2;
const int OPEN_PIN = 4;
const int CLOSE_PIN = 5;
const int LIMIT_FRONT = 6;
const int LIMIT_BACK = 7;
const int FORWARD = 10;
const int BACKWARD = 11;
const int PS_EN = 12;

const boolean OPEN = 0;
const boolean CLOSE = 1;
const int FRONT = 0;
const int BACK = 1;
const int MIDDLE = 2;

int lastAction = 4;
volatile boolean limitReached;
unsigned long restartTime;
volatile int action = 3;
int posn;

//=====================================================================
//
// Function Delcarations
//
//=====================================================================


void userInterrupt(){
  // Read user pins
  if (!digitalRead(OPEN_PIN)){ //if the open pin has been pulled low
    action = OPEN;
  } else if (!digitalRead(CLOSE_PIN)){
    action = CLOSE;
  }
}

void bumperPoll(){
  // Check limit switches for position.
  if (!digitalRead(LIMIT_FRONT)){
    posn = FRONT;
  } else if (!digitalRead(LIMIT_BACK)){
    posn = BACK;
  } else {
    posn = MIDDLE;
  }
}

void stopAll(){
  digitalWrite(FORWARD, LOW);
  digitalWrite(BACKWARD, LOW);
}

void actuate(){
  /*  
  actuate() checks to make sure that the user isn't
  pressing the same button over and over, and checks to 
  make sure we aren't already moving in some direction. 
  Then, it accelerates to full in the direction selected.
  */
      
  // If a user is being insistent, ignore him:
  if (lastAction == action) { return; }
  lastAction = action;
  
  if (action == OPEN){
    // Accelerate for a second and a half:
    for (int i = 100; i < 255; i++){
      bumperPoll();
      if (posn == BACK || action != lastAction){ 
        stopAll();
        return; 
      }
      analogWrite(BACKWARD, i);
      delay(10);
    }
    
    // Then switch to full high for noise considerations:
    digitalWrite(BACKWARD, HIGH);
    
    // Keep going until you're done, then stop:
    while (posn != BACK & action == lastAction){
      bumperPoll();
    }
    stopAll();
    
  } else if (action == CLOSE){
    // Accelerate for a second and a half:
    for (int i = 100; i < 255; i++){
      bumperPoll();
      if (posn == FRONT || action != lastAction){ 
        stopAll();
        return; 
      }
      analogWrite(FORWARD, i);
      delay(10);
    }
    
    // Then switch to full high for noise considerations:
    digitalWrite(FORWARD, HIGH);
    
    // Keep going until you're there, then stop:
    while (posn != FRONT & action == lastAction){
      bumperPoll();
    }
    stopAll();
    
  }
}


//=====================================================================
//
// Main Loop
//
//=====================================================================

void setup() {
  for(int i = 2; i <  9; i++) { pinMode(i,INPUT); }
  for(int i = 9; i < 14; i++) { pinMode(i,OUTPUT); }
  
  //attachInterrupt(0, bumperPoll, FALLING);
  attachInterrupt(1, userInterrupt, FALLING);
  
  //Board reset awareness:
  for(int i = 0; i < 11; i++){
    digitalWrite(13, i%2);
    delay(200);
  }
}

void loop() {
  // On interrupt/wakeup: 
  digitalWrite(PS_EN, HIGH);  //enable power supply

  actuate();
   
  // go to sleep
  action = 3;
  lastAction = 4;
  delay(20);
  digitalWrite(PS_EN, LOW);  //disable power supply
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}
