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
 
*/

#include "LowPower.h"

//=====================================================================
//
// Variable Delcarations
//
//=====================================================================

const int PS_EN = 1;
const int BUMPER_INT_PIN = 2;
const int USER_INT_PIN = 3;
const int OPEN_PIN = 4;
const int CLOSE_PIN = 5;
const int WC_FORWARD = 6;
const int WC_LIMIT_FRONT = 7;
const int WC_LIMIT_BACK = 8;
const int WC_BACKWARD = 9;
const int DR_FORWARD = 10;
const int DR_BACKWARD = 11;
const int DR_LIMIT_FRONT = 12;
const int DR_LIMIT_BACK = 13;

const int OPEN = 0;
const int CLOSE = 1;
const int FRONT = 0;
const int BACK = 1;
const int MIDDLE = 2;

int lastAction;
volatile int action;
volatile byte limitReached;
unsigned long restartTime;
int WC_posn, DR_posn;

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

boolean checkPinLow(int pin){
  // returns true if pin is low
  if (!digitalRead(pin)){
    delayMicroseconds(100);
    return (!digitalRead(pin));
  }
  return 0;
}

void bumperPoll(){
  // Check limit switches for position.
  if (checkPinLow(WC_LIMIT_FRONT)){
    WC_posn = FRONT;
  } else if (checkPinLow(WC_LIMIT_BACK)){
    WC_posn = BACK;
  } else {
    WC_posn = MIDDLE;
  }
  
  if (checkPinLow(DR_LIMIT_FRONT)){
    DR_posn = FRONT;
  } else if (checkPinLow(DR_LIMIT_BACK)){
    DR_posn = BACK;
  } else {
    DR_posn = MIDDLE;
  }
}

void stopWC(){
  digitalWrite(WC_FORWARD, LOW);
  digitalWrite(WC_BACKWARD, LOW);
}

void stopDR(){
  digitalWrite(DR_FORWARD, LOW);
  digitalWrite(DR_BACKWARD, LOW);
}

void stopAll(){
  stopWC();
  stopDR();
}

void actuate(){
      /*  
      actuate() checks to make sure that the user isn't
      pressing the same button over and over, and checks to 
      make sure we aren't already moving in some direction. 
      Then, it accelerates to full in the direction selected.
      */
  lastAction = action;
  
  if (action == OPEN){

    // Accelerate over 1.5s:
    for (int i = 100; i < 255; i++){
      // First, check if user has changed mind:
      if (action != lastAction) { 
        stopAll();
        return; 
      }
      
      // Then check position, and act accordingly:
      bumperPoll();
      if (WC_posn == FRONT){ stopWC(); }
      else { analogWrite(WC_FORWARD, i); }
      if (DR_posn == FRONT){ stopDR(); }
      else { analogWrite(DR_FORWARD, i); }
      if (WC_posn == FRONT && DR_posn == FRONT){
        stopAll();
        return;
      }  
      delay(10);
    }
    
    // Set high (since pulse sounds bad):
    digitalWrite(WC_FORWARD, HIGH);
    digitalWrite(DR_FORWARD, HIGH);
    do{
      bumperPoll();
      if (WC_posn == FRONT){ stopWC(); }
      if (DR_posn == FRONT){ stopDR(); }
      if (action != lastAction) { break; } //in case user changes mind
    } while (WC_posn != FRONT || DR_posn != FRONT);
    
    stopAll();
  
  } else if (action == CLOSE){
   
    // Accelerate over 1.5s:
    for (int i = 100; i < 255; i++){
      bumperPoll();
      if (action != lastAction) { 
        stopAll();
        return; 
      }
      if (WC_posn == BACK){ stopWC(); }
      else { analogWrite(WC_BACKWARD, i); }
      if (DR_posn == BACK){ stopDR(); }
      else { analogWrite(DR_BACKWARD, i); }
      if (WC_posn == BACK && DR_posn == BACK){
        stopAll();
        return;
      }  
      delay(10);
    }
    
    // Set high (since pulse sounds bad):
    digitalWrite(WC_BACKWARD, HIGH);
    digitalWrite(DR_BACKWARD, HIGH);
    do{
      bumperPoll();
      if (WC_posn == BACK){ stopWC(); }
      if (DR_posn == BACK){ stopDR(); }
      if (action != lastAction) { break; } // in case user changes mind
    } while (WC_posn != BACK || DR_posn != BACK);
    
    stopAll();
  }
}


//=====================================================================
//
// Main Loop
//
//=====================================================================

void setup() {
  for(int i = 2; i <  14; i++) { pinMode(i,INPUT); }
  int pinList[5] = {1, 6, 9, 10, 11};
  for(int i = 0; i < 5; i++){ pinMode(pinList[i],OUTPUT); }
  
  //attachInterrupt(0, bumperPoll, FALLING);
  attachInterrupt(1, userInterrupt, FALLING);
  action = 3;
  lastAction = 4;
}

void loop() {
  // On interrupt/wakeup: 
  digitalWrite(PS_EN, HIGH);  //enable power supply
  
  actuate();
  
  // go to sleep
  action = 3;
  lastAction = 4;
  delay(20); // so the serial println flushes
  digitalWrite(PS_EN, LOW);  //disable power supply
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}
