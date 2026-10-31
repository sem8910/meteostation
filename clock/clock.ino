#include <GyverBME280.h>
#define _LCD_TYPE 1
#include <I2C_LiquidCrystal_RUS.h>
#include <GyverDS3231.h>
#include <Wire.h>

I2C_LiquidCrystal_RUS lcd(0x27, 16, 2, 2); 
GyverBME280 bme;
GyverDS3231 rtc;

const int backlightPin = 2; 
const int menuButtonPin = 14;
const int hoursButtonInt = 13;
const int minutesButtonPin = 12;
const int powerIndPin = 0;
const int tempIndPin = 15;
const int wetIndPin = 16;
const int photoresPin = A0;

int inputPhotoRes() {
  int photoValue = analogRead(photoresPin);
  return photoValue;
}

void controlBacklight(int photoValue) {
  int bright = map(photoValue, 0, 1023, 200, 1023);
  analogWrite(photoValue, bright);
}



void setup() {
  Wire.begin();
  pinMode(backlightPin, OUTPUT);
  Serial.begin(9600);                         
  bme.begin();  
  lcd.init();
  lcd.clear();
  lcd.backlight();
}

void loop() {

}
