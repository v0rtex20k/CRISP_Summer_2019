
#include <IRremote.h>
#include "viaRemote.h" // my own personal remote's button mappings.
#include <elapsedMillis.h>

#define dirPin 8 // sets direction for BigEasy
#define stpPin 9 // sets step pin for BigEasy
#define btn 10 // sets button pin
#define RECV_PIN 12 // sets IR receiver pin for remote control
#define remoteSize 17 // number of buttons on my personal remote
#define sep '#' // how to distinguish between adjacent packets. Can be changed as needed.


int bS = 0;     // holds the digitalRead of the btn (buttonState)
int spd = 0;    // The current speed in steps/second. Start by not moving.
int d0ir = -1;      // Either 1 for forward, or 0 for reverse (HIGH/LOW).
unsigned int sread = 0; // CRUCIAL for stopping after rotating back to home. Incremented up, so lowest it can be is zero
char sSPEED; // serial speed char (f,r,s) extracted from full serial prompt.
char sDIR; // serial direction char (1,2,3,4) extracted from full serial prompt.
char lastSDIR; // for homing function to know which way to spin.
char joystick; // serial input from joystick/ROS/Raspberry Pi system
bool reset = false; // variable which resets everything after motor has spun (very bottom of loop()) - CRUCIAL to avoid inifinite commands
//String prompt; // Holds the entire message for processing.
IRrecv irrecv(RECV_PIN); // creates a receiver obj reading from RECV_PIN to handle incoming IR signals from remote
decode_results inputs; // decode_results object to transform IR signals into readable input - consult documentation
                       // at https://github.com/z3t0/Arduino-IRremote/blob/2.1.0/IRremote.h 

void setup(){
  Serial.begin(9600);
  pinMode(dirPin, OUTPUT);
  pinMode(stpPin, OUTPUT);
  pinMode(btn, INPUT_PULLUP); // removes signal noise - defaults to LOW == 0.
  irrecv.enableIRIn(); // Begin receiving 
  irrecv.blink13(true); // Blinks LED when new IR signals recieved - debugging. Can be removed.
}


void loop(){  
//  Serial.println(".");
  bool sS = false; // controls speed switch statement below
  bool sD = false; // controls direction switch statement below
 /* Serial.print("^^sDIR is: ");
  Serial.println(sDIR);
  Serial.print("^^sSPEED is: ");
  Serial.println(sSPEED);
  Serial.print("oldSDIR is: ");
  Serial.println(lastSDIR);*/
  while(Serial.available() > 0){ 
    joystick = Serial.read();
   // Serial.print("joystick is: ");
   // Serial.println(joystick);
      if(isalpha(joystick)){
        if(isValid(joystick,1)){
          sD = true;
          sDIR += joystick;
          lastSDIR = sDIR;
          sread++;
          joystick = char (0); // reset it for next input
        }
        else resetter(1);
      }
      else{
        continue;
      }
  }
  if (irrecv.decode(&inputs)){ // returns true if it recieves IR signals, false otherwise
   // Serial.println("DECODE TOP");
    String hexValue = String(inputs.value, HEX);
    hexValue.toUpperCase(); // hex prints with lowercase letters vs header w/ caps.
    if(isdigit(demasker(hexValue))){
     // Serial.println(demasker(hexValue));
      if(isValid(demasker(hexValue), 2)){ // '2' flag set so it checks against valid int values in isValid()
         sS = true;
         sSPEED = demasker(hexValue);
        // Serial.print("sSPEED is now: ");
        // Serial.print(sSPEED);
        // Serial.println("");
         sread++;
      }
      else resetter(1); // eliminates bad input. 
   } 
   else if(demasker(hexValue) == '0') reset = true;
   irrecv.resume(); // re-enables IR receiver, allowing new commands.
 }
 //if(sDIR == (char) 0 or sSPEED == (char) 0) return; // makes sure both direction and speed are entered.
 //Serial.println("OKAY");
 if(isValid(sDIR, 1) and isValid(sSPEED, 2)) sD = sS = true; // needed b/c sometimes one resets in loops^^
 if(checkButton()) activateButton(); // Looks to see if button has been pressed or not. 
 /*Serial.print("@sDIR is: ");
 Serial.println(sDIR);
 Serial.print("@sSPEED is: ");
 Serial.println(sSPEED);*/
  
  // SET DIRECTION   
 if(sD){ 
   //Serial.println("HEY");
   switch (sDIR){
    case 'f':
      Serial.write("FORWARD");
      dir = 1; // HIGH
      lastSDIR = 'f';
      break;
    case 'r':
      Serial.write("REVERSE");
      dir = 0; //LOW
      lastSDIR = 'r';
      break;
    case 's': // must be 's' to pass isValid() check, but still. 
      Serial.write("\t**STOP**");
      dir = -1;
      lastSDIR = 's';
      resetter(5);
      sSPEED = 'E';
      break;
    default:
      Serial.write("**DEF");
      dir = -1; // no effect - easier to debug. technically interpretted as HIGH (aka NOT zero)
      resetter(6);
      sSPEED = 'E';
      lastSDIR = 's';
      spd = 0;
      sS = sD = false;
      break;
   }
 }

int multiplier = 0;
  // SET SPEED
 if(sS){ 
    switch(sSPEED){
      case '1':
        spd = 16382; // SUPER slow - DO NOT GO PAST THIS POINT!!! delayMicro CANNOT handle it.
        multiplier = 5;
        Serial.println(" @  ABSOLUTE SLOWEST SPEED (1)");
        break;
      case '2':
        spd = 14000; // SLOW
        multiplier = 4;
        Serial.println(" @ SECOND SLOWEST SPEED (2)");
        break;
      case '3':
        spd = 12000;
        multiplier = 3;
        Serial.println(" @ THIRD SLOWEST SPEED (3)");
        break;
      case '4':
        spd = 10000;
        multiplier = 2;
        Serial.println(" @ MEDIUM to LOW SPEED (4)");
        break;
      case '5':
        spd = 8000;
        multiplier = 1;
        Serial.println(" @ MEDIUM to HIGH SPEED (5)");
        break;
      case '6':
        spd = 6000;
        multiplier = 1;
        Serial.println(" @  FOURTH FASTEST SPEED (6)");
        break;
      case '7':
        spd = 4000;
        multiplier = 1;
        Serial.println(" @ THIRD FASTEST SPEED (7)");
        break;
      case '8':
        spd = 2000;
        multiplier = 1;
        Serial.println(" @ SECOND FASTEST SPEED (8)");
        break;
      case '9':
        spd = 1000; 
        multiplier = 1;
        Serial.println("@ FASTEST SPEED (9)");
        break;
      case 'E':
        sread = 0; // DO NOT CHANGE! Controls return below in if(sread) -> reset
        spd = 0;
        Serial.println(" @ ZERO (E)");
        break;
      default:
        sread = 0;
        spd = 0;
        Serial.println("bad");
        break;
    }
 }

 if(sread == 0){
   resetter(7);
   return; // SKIP ROTATION STUFF BELOW - CRUCIAL to stop rotation 
 }
 rotator(); // dir and spd are global!
 if(reset){
   resetter(8);
 }
}

void rotator(){
  if(spd == 0) return;
  int timetoHome = 500; // 1000 ms in sec = 1 second. 
  bool yes = false;
  bool go = true;
  elapsedMillis timeElapsed;
  while(timeElapsed < (timetoHome)){
    //Serial.println("ROTATING");
    digitalWrite(dirPin, dir);
    digitalWrite(stpPin, HIGH);
    delayMicroseconds(spd);
    digitalWrite(stpPin, LOW);
    delayMicroseconds(spd);
    if(go){
      yes = checkButton(); 
      go = false;
    }
   if(Serial.available() > 1){
    Serial.println(" \tDO A BARREL ROLL! ");
    char input = Serial.read();
    if(input != sDIR){
      //Serial.println(" SIR YES SIR ! ");
      if(input == 's'){
        reset = true;
        spd = 0;
      }
      lastSDIR = sDIR = input;
    }
    return; 
   }
  }
  if(yes) activateButton();
}

bool checkButton(){
  if(digitalRead(btn) == LOW) return true;
  return false;
}

void activateButton(){
    Serial.println("BUTTON ACTIVATED");
    delay(50);
    sDIR = 's';
    sSPEED = 'E'; // overkill but whatever
    resetter(3);
    Serial.println("HOME");
    goHome(lastSDIR);   // GOHOME() CALLED HERE!!
    resetter(4);
    sDIR = 's';
    sSPEED = 'E';
}

char demasker(String hexValue){
   for(int i = 0; i <  remoteSize; i++){
    if(hexValue == primeHex[i] or hexValue == secHex[i]){
      return buttonLabels[i]; // consult header file
    }
   }
   return '!';
}

bool isValid(char c, int channel){ // sanitize input or indirectly call for a reset
  if(channel == 1){
    for(int i = 13; i < remoteSize-1; i++){ // checks only for f r s 
       if(c == buttonLabels[i]){ 
        //Serial.print("Yes, ");
        //Serial.print(c);
        //Serial.print(" is equal to ");
        //Serial.print(buttonLabels[i]);
        //Serial.println('.');
         if(c == 's'){
          sSPEED = 'E';
         }
        return true;  
       }    
    }
  }
  else if (channel == 2){
    for(int i = 0; i < 9; i++){ // only checks digits 1-9
       if(c == buttonLabels[i]){ 
   //     Serial.print("Yes, ");
   //     Serial.print(c);
   //     Serial.print(" is equal to ");
   //     Serial.print(buttonLabels[i]);
   //     Serial.println('.');
        return true;  
       }
       else if(c == '0'){
        resetter(10);
       }
    }
  }
  return false;
}

void resetter(int ERRORCODE){ // resets all switch controlling variables to avoid re-reads / misc issues w/ unrecognized input.
   // Serial.print("[RESET] ");
   // Serial.println(ERRORCODE);
    sSPEED = lastSDIR = sDIR = joystick = (char) 0;
    reset = false;
}

static unsigned int timetoHome = 14900; // ~15 seconds (14.9 for dead center) in milliseconds for elapsed time on buttonPress
#define goHomeDelay 2500 // don't drop below 1000! Stepper freaks out
// NOTE: It takes the arm ~30 sec @ delay = 5000 and ~15 sec @ delay = 2500 (obv).
void goHome(char oldSDIR){
  elapsedMillis timeElapsed;
  int wayHome = -1;
  switch(oldSDIR){
      case 'f':
         wayHome = 0; //REVERSE - opposite direction
         break;
      case 'r':
         wayHome = 1; //FORWARD - opposite direction
         break;
      default:
         wayHome = -1; // safety - just in case
         resetter(9);
         return;
         break;
    }
  while(timeElapsed < timetoHome){
    digitalWrite(dirPin, wayHome); // go in "wayHome" direction
    digitalWrite(stpPin, HIGH);
    delayMicroseconds(goHomeDelay);
    digitalWrite(stpPin, LOW);
    delayMicroseconds(goHomeDelay);
  }
}
