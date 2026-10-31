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


void controlBacklight(int photoValue) {
  int bright = map(photoValue, 650, 1023, 255, 20);
  bright = constrain(bright, 20, 255);
  analogWrite(backlightPin, bright);
  // Serial.print(bright);
}

int outInfoTemp() {
  int temp = bme.readTemperature();
  return temp;
}

float outInfoWet() {
  float wet = bme.readHumidity();
  return wet;
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

void display(int temp, int baro, int wet, int height) {
  lcd.setCursor(0, 0);
  for (int i = 0; i < 16; i++) {
    lcd.print(" ");
  }
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++) {
  lcd.print(" ");
  }
  lcd.setCursor(0, 0);
  lcd.print("t: ");
  lcd.print(String(temp));
  lcd.write(0xDF);
  lcd.print("C    ");
  lcd.print("h: ");
  lcd.print(String(wet));
  lcd.print("%  ");
  lcd.setCursor(0, 1);
  lcd.print("P: ");
  lcd.print(String(baro));
  lcd.print("mm  ");
}

void loop() {
  int photo = analogRead(photoresPin);
  controlBacklight(photo);
  // display(3, 6, 9, 12);
  delay(1000); 
}

