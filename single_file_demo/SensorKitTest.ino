#include <Arduino_SensorKit.h>
#include <Arduino_SensorKit_BMP280.h>
#include <Arduino_SensorKit_LIS3DHTR.h>

#define LED 6
#define Environment Environment_I2C

//unsigned char tempread = 0;
float temp = 0.0;
float hum  = 0.0;

void setup() {
  Environment.begin();
  // put your setup code here, to run once:
  //pinMode(LED,OUTPUT);    //Sets the pinMode to Output 
  Oled.begin();
  Oled.setFlipMode(true); // Sets the rotation of the screen
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  //digitalWrite(LED, HIGH); //Sets the voltage to high 
  //delay(1000);					   //Waits for 1000 milliseconds 
  //digitalWrite(LED, LOW);  //Sets the voltage to low
  //delay(1000);             //Waits for 1000 milliseconds

  int random_value = analogRead(A0);   //read value from A0
  int random_value2 = analogRead(A3);   //read value from A0
  
  //tempread++;
  //if(tempread>2){
    float temp = Environment.readTemperature();
    float hum = Environment.readHumidity();
    //tempread = 0;
  //}

  Oled.setFont(u8x8_font_chroma48medium8_r); 
  Oled.setCursor(0, 0);    // Set the Coordinates 
  Oled.print("Volt :     ");
  Oled.setCursor(7, 0);    // Set the Coordinates
  Oled.print(random_value); // Print the Values  
  //Oled.print(rowi); // Print the Values
  
  Oled.setCursor(0, 2);    // Set the Coordinates 
  Oled.print("Light:     ");
  Oled.setCursor(7, 2);    // Set the Coordinates
  Oled.print(random_value2); // Print the Values

  Oled.setCursor(0, 4);    // Set the Coordinates 
  Oled.print("Temp :       ");
  Oled.setCursor(7, 4);    // Set the Coordinates
  Oled.print(temp); // Print the Values
  Oled.print("C");

  Serial.print("Temperature = ");
  Serial.print(Environment.readTemperature()); //print temperature
  Serial.println(" C");

  Oled.setCursor(0, 6);    // Set the Coordinates 
  Oled.print("Hum  :       ");
  Oled.setCursor(7, 6);    // Set the Coordinates
  Oled.print(hum); // Print the Values
  Oled.print("%");

  Oled.refreshDisplay();    // Update the Display

  delay(600);
}
