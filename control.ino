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

//=====================================================================
//
// Variable Delcarations
//
//=====================================================================

const int USER_INT_PIN = 2;
const int BUMPER_INT_PIN = 3;
const int OPEN_ONE = 4;
const int OPEN_ALL = 5;
const int CLOSE_PIN = 6;
const int LIMIT_FORWARD = 7;
const int LIMIT_BACK = 8;
const int COMMS = 10;
const int SPEED_PIN = 11;
const int DIR_PIN = 12;

const boolean ONE = 0;
const boolean ALL = 1;
const boolean OPEN = 0;
const boolean CLOSE = 1;

/* Flags for interrupts, read as follows:
      [0] == User switch in one
      [1] == User switch in All
      [2] == User switch close All
      [3] == Front bumper switch
      [4] == Rear bumper switch
*/      
volatile boolean flag[5] = {0,0,0,0,0};
//int flagNum;
boolean limitReached;

//=====================================================================
//
// Function Delcarations
//
//=====================================================================


void userInterrupt(){
  for (int i = 0; i < 3; i++){
    flag[i] = !digitalRead(i+4);
  }
  Serial.println("\n\nUser Interrupt");
}

void bumperInterrupt(){
  // bumper should not wake from sleep; should just stop motor
  analogWrite(SPEED_PIN, 0);
  limitReached = 1;
  Serial.println("\n\nBumper Interrupt, Stop Motor");
}

void actuate(boolean dir, boolean scope){
  if (scope == ALL) digitalWrite(COMMS, HIGH);
  digitalWrite(DIR_PIN, dir);
  for (int i = 0; i < 255; i++){
    analogWrite(SPEED_PIN, i);
    delayMicroseconds(1000);
  }  // REPLACE THIS WITH ACCELERATION CODE
  digitalWrite(COMMS, LOW);
}

//=====================================================================
//
// Main Loop
//
//=====================================================================

void setup() {
  for(int i = 2; i < 9; i++){
    pinMode(i,INPUT);
  }
  pinMode(SPEED_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(13, OUTPUT);
  
  Serial.begin(9600);
  
  //Board reset awareness:
  for(int i = 0; i < 10; i++){
    digitalWrite(13, i%2);
    delay(200);
  }
}

void loop() {
  for (int i = 0; i < 5; i++){ flag[i] = 0; } // set flags off
  
  // attach interrupts in prep for sleep
  attachInterrupt(0, userInterrupt, FALLING);
  attachInterrupt(1, bumperInterrupt, FALLING);
  
  // go to sleep
  // TODO write sleep code and/or import library
  
  // On interrupt: 
  detachInterrupt(0); 
  for (int i = 0; i < 5; i++){   // And check flags
    if (flag[i] == 1){ 
      Serial.print("flag read: " + String(i));
      switch(i){                 // call appropriate procedure
        case 0: Serial.println(": open ONE"); actuate(OPEN, ONE); break;
        case 1: Serial.println(": open ALL"); actuate(OPEN, ALL); break;
        case 2: Serial.println(": Close all"); actuate(CLOSE, ALL); break;
        default: bumperInterrupt;  // kills motor
      }
    }
  }
  //TODO: TURN ON LIGHTS
  
}
