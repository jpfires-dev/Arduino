#include <Arduino.h>

#define STATE_CONFIG 0
#define STATE_WAIT   1
#define STATE_START  2
#define STATE_READY  3
#define STATE_STOP   4

uint32_t now;
uint8_t numberoftargets;
uint8_t newtargetnumber;
uint8_t prevtargetnumber;
uint8_t numberoftargetswitching;
uint8_t state;
uint32_t serialinput;

uint8_t delay_readytostart;
uint8_t delay_targetengage;
uint8_t delay_targetswitch;

//====================================================================
// Initialise
//====================================================================
void setup() {
  // put your setup code here, to run once:
  // UART0 will be used to log information into Serial Monitor
  Serial.begin(115200);

  Serial.println("Setup done");
  numberoftargets = 5;
  newtargetnumber = 1;
  prevtargetnumber = 1;
  numberoftargetswitching = 10;
  state = STATE_CONFIG;

}

//====================================================================
// Funtions
//====================================================================
void GetNewTarget(){
  
  prevtargetnumber = newtargetnumber;
  newtargetnumber = random(1,numberoftargets+1);

  while(newtargetnumber==prevtargetnumber){
    newtargetnumber = random(1,numberoftargets+1);
  }
  
}

//====================================================================
// Main
//====================================================================
void loop() {
  // put your main code here, to run repeatedly:
  now = millis();

  //Serial.printf("%03d\r\n",now);
  while( (millis() - now) < 1000){}

  switch(state){
    case STATE_CONFIG:
      GetNewTarget();
      Serial.printf("%d ",newtargetnumber);
      //Serial.printf("%03d\r\n",now);
      break;
    case STATE_WAIT:
      break;
    case STATE_READY:
      break;
    case STATE_STOP:
      break;
  }

  if(Serial.available()>0){
    serialinput = Serial.read();
    Serial.print("Received: ");
    Serial.println(serialinput);
  }
  
}
