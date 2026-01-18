#include <Arduino.h>

#define STATE_IDLE       0
#define STATE_READY         1
#define STATE_CHOOSETARGET  2
#define STATE_TARGETON      3
#define STATE_TARGETOFF     4
#define STATE_WAIT          5
#define STATE_FINISH        6
#define STATE_STOP          7

uint8_t i;
uint32_t now;
uint8_t numberoftargets;
uint8_t currenttargetnumber;
uint8_t newtargetnumber;
uint8_t prevtargetnumber;
uint8_t numberoftargetswitching;
uint8_t state;
uint32_t serialinput;

uint32_t delay_readytostart;
uint32_t delay_targetengage;
uint32_t delay_targetswitch;
uint8_t  target_array[10];

uint32_t time_targeton;
uint32_t time_targetoff;
uint32_t time_readystart;

float adc_voltage;

// Constants
const float V_REF = 3.3;     // Analog reference voltage (e.g., 5V or 3.3V)
const float R_BITS = 10.0;   // ADC resolution (bits)
const float ADC_STEPS = (1 << int(R_BITS)) - 1; // Number of steps (2^R_BITS - 1)


//====================================================================
// Initialise
//====================================================================
void setup() {
  // put your setup code here, to run once:
  // UART0 will be used to log information into Serial Monitor
  Serial.begin(115200);

  Serial.println("Setup done");
  numberoftargets = 5;
  currenttargetnumber = 1;
  newtargetnumber = 1;
  prevtargetnumber = 1;
  numberoftargetswitching = 10;
  state = STATE_IDLE;

  delay_readytostart = 3000;

  // Setup ADC
  // ADC0 => GPIO36 => analogRead(36)
  // ADC3 => GPIO39 => analogRead(39)
  //pinMode(A0, OUPUT);
  digitalWrite(36, LOW);
  pinMode(36, INPUT);

  // Setup Target Array
  target_array[0] = 25; pinMode(target_array[0], OUTPUT);
  target_array[1] = 26; pinMode(target_array[1], OUTPUT);
  target_array[2] = 27; pinMode(target_array[2], OUTPUT);
  target_array[3] = 14; pinMode(target_array[3], OUTPUT);
  target_array[4] = 12; pinMode(target_array[4], OUTPUT);
  target_array[5] = 13; pinMode(target_array[5], OUTPUT);
  target_array[6] = 25; pinMode(target_array[6], OUTPUT);
  target_array[7] = 25; pinMode(target_array[7], OUTPUT);
  target_array[8] = 25; pinMode(target_array[8], OUTPUT);
  target_array[9] = 25; pinMode(target_array[9], OUTPUT);
  
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
  //now = millis();

  //Serial.printf("%03d\r\n",now);
  while( (millis() - now) < 1000){}

  switch(state){
    case STATE_IDLE:
      //GetNewTarget();
      //Serial.printf("%d ",newtargetnumber);
      //Serial.printf("%03d\r\n",now);

      adc_voltage = ((analogRead(36)/ADC_STEPS)*V_REF);
      Serial.println(adc_voltage,3);

      i++;
      if(i>9)
        i = 0;
      digitalWrite(target_array[i],HIGH);
      delay(250);
      digitalWrite(target_array[i],LOW);
      delay(250);

      break;
    case STATE_READY:
      now = millis();
      if((now-time_readystart)>delay_readytostart){
        state = STATE_IDLE;
        Serial.println("Change State to: STATE_IDLE");
      }
      break;
    case STATE_CHOOSETARGET:
      GetNewTarget();
      state = STATE_TARGETON;
      time_targeton = millis();
      break;
    case STATE_TARGETON:
      //now = millis();
      //Serial.println("Target ON");

      //if((now-time_readystart)>delay_readytostart){
      //  state = STATE_CHOOSETARGET;
      //}
      break;
    case STATE_TARGETOFF:
      break;
    case STATE_WAIT:
      break;
    case STATE_FINISH:
      break;
    case STATE_STOP:
      break;
  }

  if(Serial.available()>0){
    serialinput = Serial.read();
    Serial.print("Received: ");
    Serial.println((int)serialinput);
    switch(serialinput){
      case '1':
        state = STATE_READY;
        time_readystart = millis();
        Serial.println("Change State to: STATE_READY");
        break;
      case '2':
        Serial.println("Case 2");
        break;
      case '3':
        Serial.println("Case 3");
        break;
      case '4':
        Serial.println("Case 4");
        break;
      case '5':
        Serial.println("Case 5");
        break;
      case '6':
        Serial.println("Case 6");
        break;
        
    }
  }
  
}
