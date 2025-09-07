```c
/*******************
Connections:
SCLK  -> D13 (SCK)
SDATA -> D11 (MOSI) MasterOutSlaveIn (COPI)ControllerOutPeripheralOut

Rev 4:
10(CS), 
11(COPI), 
SDATA   ->  12(CIPO), 
SCLK    ->   13(SCK)
*******************/

#include <SPI.h>

const int RST   = 8;
const int FSYN1 = 9;
const int FSYN2 = 10;

unsigned long previousMillis  = 0;
const long interval           = 20000;
unsigned char freq            = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(RST,    OUTPUT);
  pinMode(FSYN1,  OUTPUT);
  pinMode(FSYN2,  OUTPUT);

  digitalWrite(RST,   HIGH);
  digitalWrite(FSYN1, HIGH);
  digitalWrite(FSYN2, HIGH);

  SPI.begin();
  //SPI.setDataMode(SPI_MODE2);
  //SPI.setBitOrder(MSBFIRST);
  // Sets the SPI clock divider relative to the system clock. 
  //    On AVR based boards, the dividers available are 2, 4, 8, 
  //    16, 32, 64 or 128. The default setting is SPI_CLOCK_DIV4, 
  //    which sets the SPI clock to one-quarter the frequency of 
  //    the system clock (4 Mhz for the boards at 16 MHz).
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE2));

  AD9834_RESET();
  AD9834_FREQ();
  AD9834_ENABLE();

  delay(2000);

  // Take AD9834 out of hardware reset.
  digitalWrite(RST, LOW);
}

/* RESET: ----------------------------------------------------------------------------
Control Register write. Datasheeet recommends putting part in reset after power up. 
See Datasheet pg. 19, for details on Control Register write.*/

void AD9834_RESET()
{
  digitalWrite(FSYN1, LOW);
  SPI.transfer16(0b0010000100000010);
  digitalWrite(FSYN1, HIGH);
}

/* FREQUENCY + PHASE: ---------------------------------------------------
FREQUENCY WORD: 
Is 28 bits, sent in two 14 bit parts LSB sent first. 
In both parts B15 and B16 address the frequency register.
01 -> FREQ0  |-------|  10 -> FREQ1
So, for FREQ1 the word should be 01xx xxxx xxxx xxxx for both parts. 
Where x are the 14 bits of the frequency word. 

PHASE WORD:
Is 12 bits. B14, B15 and B16 are set to 11 to address the phase register.
110 -> PHASE0
111 -> PHASE1
So, for PHASE0 the word should be 110x xxxx xxxx xxxx. B0 - B11 are the phase word and B12 is "don't care"
0xC000 sets a ZERO phase adjustment. */

void AD9834_FREQ()
{ 
  //FREQUENCY
  digitalWrite(FSYN1, LOW);
  SPI.transfer16(0b0111010000001110);
  SPI.transfer16(0b0100001101101001);

  //PHASE
  SPI.transfer(0b11000000);
  SPI.transfer(0b00000000);             // 0b00000000 equals ZERO phase.
  digitalWrite(FSYN1, HIGH);
  Serial.println("FREQUENCY Done");
}

/* ENABLE: ----------------------------------------------------------------------------
Control Register write. Enable the AD9834 and set PIN/SW to 1. This enables Chip pin reset (hardware reset). 
The AD9834 will be left in a hardware reset state dcue to the HIGH on the RST pin. 
This is cleared at the end of the void setup().
To enable the PIN/SW pin, set control B9 to HIGH. */

void AD9834_ENABLE()
{ 
  digitalWrite(FSYN1, LOW);
  SPI.transfer(0x22);
  SPI.transfer(0x00);
  digitalWrite(FSYN1, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  //unsigned long currentMillis = millis();
  //if(currentMillis - previousMillis >= interval){
  //  if(freq == 1){
  //    freq = 0;
  //    digitalWrite(FSYN1, LOW);
  //    SPI.transfer16(0b0111010000001110); // 0111 0100 0000 1110  740E
  //    SPI.transfer16(0b0100001101101001); // 0100 0011 0110 1001  4369
  //    Serial.println("FREQUENCY 0");
  //  }else{
  //    freq = 1;
  //    digitalWrite(FSYN1, LOW);
  //    SPI.transfer16(0b0111010000011110);
  //    SPI.transfer16(0b0100001101101001);
  //    Serial.println("FREQUENCY 0");
  //  }
  //
  //}
}

```
