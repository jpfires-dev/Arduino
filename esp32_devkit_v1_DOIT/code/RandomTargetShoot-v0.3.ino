#include <Arduino.h>

#include <SPI.h>
#include <Wire.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

/*************************************************************/
/*                                                           */
/* Version 0.3                                               */
/*   Added OLED 0.96 Inch 128 x 64 Pixel OLED LCD Display,   */
/*         1315 Chip Drive, IIC Interface I2C Screen Display */
/*         Module, Compatible with Arduino (Yellow and Blue) */
/* Verions 0.2                                               */
/*   Added extra buttons                                     */
/*   Added a menu via buttons                                */
/*      Output via serial port                               */
/*                                                           */
/* Verions 0.1                                               */
/*   Basic functioning version with long time delays         */
/*************************************************************/
#define VER_MAJOR           0
#define VER_MINOR           3
#define STATE_IDLE          0
#define STATE_READY         1
#define STATE_CHOOSETARGET  2
#define STATE_TARGETON      3
#define STATE_TARGETOFF     4
#define STATE_WAIT          5
#define STATE_FINISH        6
#define STATE_STOP          7

#define BTN_LEFT            36  /* Left Button GIO pin number*/
#define BTN_UP              39  /* Up Button GIO pin number*/
#define BTN_DOWN            34  /* Down Button GIO pin number*/
#define BTN_RIGHT           35  /* Right Button GIO pin number*/

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
/*
  U8g2lib Example Overview:
    Frame Buffer Examples: clearBuffer/sendBuffer. Fast, but may not work with all Arduino boards because of RAM consumption
    Page Buffer Examples: firstPage/nextPage. Less RAM usage, should work with all Arduino boards.
    U8x8 Text Only Example: No RAM usage, direct communication with display controller. No graphics, 8x8 Text only.
    
*/
// Please UNCOMMENT one of the contructor lines below
// U8g2 Contructor List (Frame Buffer)
// The complete list is available here: https://github.com/olikraus/u8g2/wiki/u8g2setupcpp
// Please update the pin numbers according to your setup. Use U8X8_PIN_NONE if the reset pin is not connected
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 22, /* data=*/ 21);   // ESP32 Thing, HW I2C with pin remapping

uint8_t i;
uint32_t now;
uint8_t numberoftargets;
uint8_t currenttargetnumber;
uint8_t newtargetnumber;
uint8_t prevtargetnumber;
uint8_t numberoftargetswitching;
uint8_t targetswitchingcount;
uint8_t state;
uint32_t serialinput;
uint8_t menu_state;           // Menu state variable
uint8_t menu_state_sub;       // Sub Menu state variable
uint8_t menu_exit;            // Variable to stay in menu

//uint32_t delay_targetengage;
//uint32_t delay_targetswitch;

uint32_t  delay_PulseOn;      // Pulsing on delay
uint32_t  delay_PulseOff;     // Pulsing off delay
uint32_t  delay_ReadyState;   // Time to wait in ReadyState
uint32_t  delay_TargetOn;     // Time to light up target
uint32_t  delay_TargetOff;    // Time before next target
uint32_t  delay_FinishState;  // Time to wait in Finish State before next round can start
uint32_t  delay_Debounce;     // Burron Debounce delay
uint8_t   target_array[10];

uint32_t time_targeton;
uint32_t time_targetoff;
uint32_t time_readystart;

uint8_t   lcd_update;

float adc_voltage;

// Constants
const float V_REF = 3.3;     // Analog reference voltage (e.g., 5V or 3.3V)
const float R_BITS = 10.0;   // ADC resolution (bits)
const float ADC_STEPS = (1 << int(R_BITS)) - 1; // Number of steps (2^R_BITS - 1)

/*====================================================================*/
/* Initialise                                                         */
/*====================================================================*/
void setup() {
  // put your setup code here, to run once:
  // UART0 will be used to log information into Serial Monitor
  Serial.begin(115200);
  Serial.printf("RandomTargetShoot %d.%02d\r\n",VER_MAJOR,VER_MINOR);

  /* Initialise Variables */
  delay_PulseOff      = 300;  /**/
  delay_PulseOn       = 300;  /**/
  delay_ReadyState    = 3000; /**/
  delay_TargetOn      = 1000; /**/
  delay_TargetOff     = 1000; /**/
  delay_FinishState   = 3000; /**/

  numberoftargets = 5;
  currenttargetnumber = 1;
  newtargetnumber = 1;
  prevtargetnumber = 1;
  numberoftargetswitching = 10;
  state = STATE_IDLE;

  // Setup Buttons
  pinMode(BTN_LEFT, INPUT);
  pinMode(BTN_UP, INPUT);
  pinMode(BTN_DOWN, INPUT);
  pinMode(BTN_RIGHT, INPUT);
  
  // Setup Target Array
  target_array[0] = 26; pinMode(target_array[0], OUTPUT); /* not used*/
  target_array[1] = 26; pinMode(target_array[1], OUTPUT); /* Target 1 */
  target_array[2] = 27; pinMode(target_array[2], OUTPUT); /* Target 2 */
  target_array[3] = 14; pinMode(target_array[3], OUTPUT); /* Target 3 */
  target_array[4] = 12; pinMode(target_array[4], OUTPUT); /* Target 4 */
  target_array[5] = 13; pinMode(target_array[5], OUTPUT); /* Target 5 */
  target_array[6] = 26; pinMode(target_array[6], OUTPUT); /* not used*/
  target_array[7] = 26; pinMode(target_array[7], OUTPUT); /* not used*/
  target_array[8] = 26; pinMode(target_array[8], OUTPUT); /* not used*/
  target_array[9] = 26; pinMode(target_array[9], OUTPUT); /* not used*/
  
  menu_state      = 0;  /* Menu state variabl     */
  menu_state_sub  = 1;  /* Sub Menu state 0=Dec, 1=NoChange 2=Inc */
  menu_exit       = 0;  /* Stay in menu till set to 0 */
  delay_Debounce  = 250;

  //-----------------------------------------------------------------------------------
  // Setup OLED Display
  // Wait for display
  delay(500);
  //u8g2.setI2CAddress(0x7B<<1);
  u8g2.begin();

  u8g2.clearBuffer();					// clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
  //                 123456789012345678901
  u8g2.drawStr(0,10,"Random Target Shoot  ");	// write something to the internal memory
  u8g2.sendBuffer();					// transfer internal memory to the display
  lcd_update = 1;
  //-----------------------------------------------------------------------------------

  Serial.println("Setup done");
}

/*====================================================================*/
/* Funtions                                                           */
/*====================================================================*/
void GetNewTarget(){
  
  prevtargetnumber = newtargetnumber;
  newtargetnumber = random(1,numberoftargets+1);

  while(newtargetnumber==prevtargetnumber){
    newtargetnumber = random(1,numberoftargets+1);
  }
  
}

void ActionTarget(uint8_t TergetNumber){
  digitalWrite(target_array[TergetNumber],HIGH);
  delay(250);
  digitalWrite(target_array[TergetNumber],LOW);
  delay(250);
}

void TargetOn(){
  digitalWrite(target_array[newtargetnumber],HIGH);
}
void TargetOff(){
  digitalWrite(target_array[newtargetnumber],LOW);
}
void AllTargetsOn(){
  for(int i=0;i<10;i++){
    digitalWrite(target_array[i],HIGH);
  }
}
void AllTargetsOff(){
  for(int i=0;i<10;i++){
    digitalWrite(target_array[i],LOW);
  }
}

void UpdateMenu(){
  menu_exit = 0;
  while(menu_exit==0){
    if(digitalRead(BTN_DOWN)==LOW){
      delay(delay_Debounce);
      while(digitalRead(BTN_DOWN)==LOW){}
      menu_state++;
      lcd_update = 1;
      //UpdateMenu();
    }
    if(digitalRead(BTN_UP)==LOW){
      delay(delay_Debounce);
      while(digitalRead(BTN_UP)==LOW){}
      menu_state--;
      lcd_update = 1;
      //UpdateMenu();
    }
    if((digitalRead(BTN_LEFT)==LOW)&&(digitalRead(BTN_RIGHT)==HIGH)){
      delay(delay_Debounce);
      if((digitalRead(BTN_LEFT)==LOW)&&(digitalRead(BTN_RIGHT)==HIGH)){
        while(digitalRead(BTN_LEFT)==LOW){}
        menu_state_sub--;
        lcd_update = 1;
        //UpdateMenu();
      }
    }
    if((digitalRead(BTN_RIGHT)==LOW)&&(digitalRead(BTN_LEFT)==HIGH)){
      delay(delay_Debounce);
      if((digitalRead(BTN_RIGHT)==LOW)&&(digitalRead(BTN_LEFT)==HIGH)){
        while(digitalRead(BTN_RIGHT)==LOW){}
        menu_state_sub++;
        lcd_update = 1;
        //UpdateMenu();
      }
    }
    switch(menu_state){
      case 0x00:
        Serial.println("Menu");
        if(lcd_update==1){
          //u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
          //                 123456789012345678901
          u8g2.drawStr(0,30,"Menu                 ");	// write something to the internal memory
          u8g2.sendBuffer();					// transfer internal memory to the display
          lcd_update = 0;
        }
        break;
      case 0x01:
        //Serial.println("Target: 1");
        if(menu_state_sub==0){
          Serial.println("Target 1 OFF");
          digitalWrite(target_array[1],LOW);
          if(lcd_update==1){
            //                 123456789012345678901
            u8g2.drawStr(0,30,"Target 1 OFF         ");	// write something to the internal memory
            u8g2.sendBuffer();					// transfer internal memory to the display
            lcd_update = 0;
          }
        }
        if(menu_state_sub==2){
          Serial.println("Target 1 ON");
          digitalWrite(target_array[1],HIGH);
          if(lcd_update==1){
            //                 123456789012345678901
            u8g2.drawStr(0,30,"Target 1 ON          ");	// write something to the internal memory
            u8g2.sendBuffer();					// transfer internal memory to the display
            lcd_update = 0;
          }
        }
        menu_state_sub = 1;
        break;
      case 0x02:
        //Serial.println("Target: 2");
        if(menu_state_sub==0){
          Serial.println("Target 2 OFF");
          digitalWrite(target_array[2],LOW);
          if(lcd_update==1){
            //                 123456789012345678901
            u8g2.drawStr(0,30,"Target 2 OFF          ");	// write something to the internal memory
            u8g2.sendBuffer();					// transfer internal memory to the display
            lcd_update = 0;
          }
        }
        if(menu_state_sub==2){
          Serial.println("Target 2 ON");
          digitalWrite(target_array[2],HIGH);
          if(lcd_update==1){
            //                 123456789012345678901
            u8g2.drawStr(0,30,"Target 2 ON          ");	// write something to the internal memory
            u8g2.sendBuffer();					// transfer internal memory to the display
            lcd_update = 0;
          }
        }
        menu_state_sub = 1;
        break;
      case 0x03:
        Serial.println("Target: 3");
        if(lcd_update==1){
          //                 123456789012345678901
          u8g2.drawStr(0,30,"Target 3             ");	// write something to the internal memory
          u8g2.sendBuffer();					// transfer internal memory to the display
          lcd_update = 0;
        }
        //display.println(millis());
        //display.display();
        delay(500);
        break;
      case 0x04:
        Serial.println("Target: 4");
        if(lcd_update==1){
          //                 123456789012345678901
          u8g2.drawStr(0,30,"Target 4             ");	// write something to the internal memory
          u8g2.sendBuffer();					// transfer internal memory to the display
          lcd_update = 0;
        }
        delay(500);
        break;
      case 0x05:
        Serial.println("Target: 5");
        if(lcd_update==1){
          //                 123456789012345678901
          u8g2.drawStr(0,30,"Target 5             ");	// write something to the internal memory
          u8g2.sendBuffer();					// transfer internal memory to the display
          lcd_update = 0;
        }
        delay(500);
        break;
      case 0x06://delay_TargetOn
        Serial.printf("delay_TargetOn: %d\r\n",delay_TargetOn);
        u8g2.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font
        //u8g2.drawStr(0,30,F("delay_TargetOn: %d\r\n",delay_TargetOn));	// write something to the internal memory
        u8g2.drawStr(0,30,"delay_TargetOn: ");	// write something to the internal memory
        u8g2.sendBuffer();					// transfer internal memory to the display
        delay(500);
        if(menu_state_sub == 0){
          if(delay_TargetOn>100){
            delay_TargetOn -= 100;
          }
        }
        if(menu_state_sub == 2){
          if(delay_TargetOn<3000){
            delay_TargetOn += 100;
          }
        }
        menu_state_sub = 1;
        break;
      case 0x07:// delay_TargetOff
        Serial.printf("delay_TargetOff: %d\r\n",delay_TargetOff);
        if(menu_state_sub == 0){
          if(delay_TargetOff>100){
            delay_TargetOff -= 100;
          }
        }
        if(menu_state_sub == 2){
          if(delay_TargetOff<3000){
            delay_TargetOff += 100;
          }
        }
        menu_state_sub = 1;
        break;
      case 0x08:
        //menu_exit
        Serial.println("Exit Y/N");
        if(menu_state_sub == 0){
          menu_state = 1;
        }
        if(menu_state_sub == 2){
          menu_state = 0;
          menu_exit = 1;
          Serial.println("Exit menu!");
        }
        menu_state_sub = 1;
        break;
      case 0x09:
        Serial.println("Menu");
        menu_state = 0x00;
        break;
      case 0xFF:
        Serial.println("Menu");
        menu_state = 0x00;
        break;
    }
    menu_state_sub = 1;
    }
}

/*====================================================================*/
/* Main                                                               */
/*====================================================================*/
void loop() {
  // put your main code here, to run repeatedly:
  //now = millis();

  //Serial.printf("%03d\r\n",now);
  //while( (millis() - now) < 1000){}

  if((digitalRead(BTN_LEFT)==LOW)&&(digitalRead(BTN_RIGHT)==LOW)){
    delay(delay_Debounce);
    if((digitalRead(BTN_LEFT)==LOW)&&(digitalRead(BTN_RIGHT)==LOW)){
      Serial.println("Left&Right Button pressed: STATE_READY");
      state = STATE_READY;
      delay(delay_Debounce);
    }
  }
  //if(digitalRead(BTN_UP)==LOW){
  //  Serial.println("Up Button pressed!");
  //  delay(200);
  //}
  //if(digitalRead(BTN_DOWN)==LOW){
  //  Serial.println("Down Button pressed!");
  //  delay(200);
  //}
  //if(digitalRead(BTN_RIGHT)==LOW){
  //  Serial.println("Right Button pressed!");
  //  delay(200);
  //}

  /*====================================================================*/
  /* State Machine                                                      */
  /*====================================================================*/
  switch(state){
    case STATE_IDLE:
      targetswitchingcount = numberoftargetswitching;
      break;
    case STATE_READY:
      AllTargetsOn();
      delay(delay_PulseOn);         // delay_PulseOn
      AllTargetsOff();
      delay(delay_PulseOff);        // delay_PulseOff

      Serial.println("   STATE_READY");
      delay(delay_ReadyState);      // delay_ReadyState
      state = STATE_CHOOSETARGET;
      break;
    case STATE_CHOOSETARGET:
      Serial.println("   STATE_CHOOSETARGET");
      GetNewTarget();
      state = STATE_TARGETON;
      Serial.println(newtargetnumber);
      break;
    case STATE_TARGETON:
      Serial.println("   STATE_TARGETON");
      TargetOn();
      delay(delay_TargetOn);        // delay_TargetOn
      state = STATE_TARGETOFF;
      break;
    case STATE_TARGETOFF:
      Serial.println("   STATE_TARGETOFF");
      TargetOff();
      state = STATE_WAIT;
      targetswitchingcount--;
      if((targetswitchingcount>0)&&(targetswitchingcount<10)){
        state = STATE_WAIT;
      }else{
        state = STATE_FINISH;
        Serial.println("Done.");
      }
      break;
    case STATE_WAIT:
      Serial.println("   STATE_WAIT");
      delay(delay_TargetOff);       // delay_TargetOff
      state = STATE_CHOOSETARGET;
      break;
    case STATE_FINISH:
      delay(delay_FinishState);     // delay_FinishState
      AllTargetsOn();
      delay(delay_PulseOn);         // delay_PulseOn
      AllTargetsOff();
      delay(delay_PulseOff);        // delay_PulseOff
      AllTargetsOn();
      delay(delay_PulseOn);         // delay_PulseOn
      AllTargetsOff();
      delay(delay_PulseOff);        // delay_PulseOff

      Serial.println("===================================");
      Serial.println("Result: You shoot bad, try again!");
      state = STATE_IDLE;
      break;
    case STATE_STOP:
      break;
  }

  /*====================================================================*/
  /* Serial Commanding and Debugging                                    */
  /*====================================================================*/
  if(Serial.available()>0){
    serialinput = Serial.read();
    Serial.print("Received: ");
    Serial.println((int)serialinput);
    switch(serialinput){
      case '1':
        state = STATE_READY;
        //time_readystart = millis();
        Serial.println("Change State to: STATE_READY");
        break;
      case '2':
        Serial.println("Going to Idle state.");
        state = STATE_IDLE;
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

  /*====================================================================*/
  /* Enter Setup Menu                                                  */
  /*====================================================================*/
  if(digitalRead(BTN_DOWN)==LOW){
    delay(delay_Debounce);
    while(digitalRead(BTN_DOWN)==LOW){}
    menu_state++;
    UpdateMenu();
  }
  //if(digitalRead(BTN_UP)==LOW){
  //  delay(delay_Debounce);
  //  while(digitalRead(BTN_UP)==LOW){}
  //  menu_state--;
  //  UpdateMenu();
  //}
  //if((digitalRead(BTN_LEFT)==LOW)&&(digitalRead(BTN_RIGHT)==HIGH)){
  //  delay(delay_Debounce);
  //  if((digitalRead(BTN_LEFT)==LOW)&&(digitalRead(BTN_RIGHT)==HIGH)){
  //    while(digitalRead(BTN_LEFT)==LOW){}
  //    menu_state_sub--;
  //    UpdateMenu();
  //  }
  //}
  //if((digitalRead(BTN_RIGHT)==LOW)&&(digitalRead(BTN_LEFT)==HIGH)){
  //  delay(delay_Debounce);
  //  if((digitalRead(BTN_RIGHT)==LOW)&&(digitalRead(BTN_LEFT)==HIGH)){
  //    while(digitalRead(BTN_RIGHT)==LOW){}
  //    menu_state_sub++;
  //    UpdateMenu();
  //  }
  //}

}
