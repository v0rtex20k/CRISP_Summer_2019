#include "DualVNH5019MotorShield.h"
#include <elapsedMillis.h>
DualVNH5019MotorShield md;

#define absolute_zero_button 8
#define outputA 2
#define encoder_output 3

char user_input = 's';
char old_input = 's';
bool set = false;
bool initializing = true;
int encoder_count = 0; 
int bState = 0;
int bLastState = 0;
const int virtual_limit_switch = -780;


void stopIfFault(){ 
  if (md.getM1Fault() or md.getM2Fault()) while(1);
}

void initialization_sequence(){
  //Serial.println("i");
  while(digitalRead(absolute_zero_button) == LOW){ // not pressed
     //Serial.println("INITIALIZING");
     md.setM1Speed(-325);
     button_not_pressed('r');
  }
  initializing = false;
  md.setM1Speed(0);
  encoder_count = 0; 
  button_pressed('f');
  //Serial.print("BACK");
}

void update_encoder_count(){
  //Serial.println('e');
  bState = digitalRead(encoder_output);
  if(bState != bLastState){  
    if(user_input == 'f') encoder_count ++;
    else encoder_count--;
    Serial.println(encoder_count); // PROCESSING
    delay(50);
  } 
}

void button_not_pressed(char input){
  //Serial.println("BNP");
  stopIfFault();
  old_input = user_input = input;
}

void button_pressed(char viable_direction){
  //Serial.println("BP");
  char savior_input = 's';
  while(savior_input != viable_direction and digitalRead(absolute_zero_button) == HIGH){ //checks if still pressed
    //Serial.print("Waiting for "); 
    //Serial.println(viable_direction);
    if(Serial.available() > 0){
      savior_input = Serial.read();
      //Serial.print("GOT ");
      Serial.println(savior_input);
    }
  }
  //Serial.println("below");
  user_input = savior_input;
  md.setM1Speed(325);
  button_not_pressed(user_input);
}

void setup() {
  Serial.begin(9600);
  md.init();
  pinMode(absolute_zero_button, INPUT_PULLUP);
  pinMode(encoder_output,INPUT);
  bLastState = digitalRead(encoder_output); 
}

void loop(){
  //Serial.println("TOP");
  if(initializing) initialization_sequence();
  update_encoder_count();
  if(Serial.available()>1){
    user_input = Serial.read();
    set = true;
  }
  int zeroState = 0;
  zeroState = digitalRead(absolute_zero_button);
  if(zeroState == LOW and set){ // button NOT pressed
       if(user_input == 'f') md.setM1Speed(325);
       else if(user_input == 'r') md.setM1Speed(-325);
       else if(user_input == 's') md.setM1Speed(0);
       //Serial.println('.');
       button_not_pressed(user_input);
       if(encoder_count == virtual_limit_switch) button_pressed('r');
  }
  else if(zeroState == HIGH) button_pressed('f');
  bLastState = bState; 
  delay(1);
}
