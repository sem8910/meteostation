#include <GyverBME280.h>
#define _LCD_TYPE 1
#include <I2C_LiquidCrystal_RUS.h>
#include <Wire.h>
#include <RTClib.h>

I2C_LiquidCrystal_RUS lcd(0x27, 16, 2, 2); 
GyverBME280 bme;
RTC_DS3231 rtc;

const int backlightPin = 2; 
const int menuButtonPin = 16;
const int hoursButtonPin = 13;
const int minutesButtonPin = 14;
const int powerIndPin = 0;
const int tempIndPin = 15;
const int baroIndPin = 12;
const int photoresPin = A0;
bool menuState = HIGH;



void controlBacklight(int photoValue) {
  int bright = map(photoValue, 650, 1023, 255, 20);
  bright = constrain(bright, 20, 255);
  analogWrite(backlightPin, bright);
}

void display(int temp, int baro) {
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
  lcd.print("C ");
  lcd.print("P: ");
  lcd.print(String(baro));
  lcd.print("mm");
}

void displayTime() {
  DateTime now = rtc.now();
  lcd.setCursor(0, 1);
  if (now.hour() < 10) lcd.print("0");
  lcd.print(String(now.hour()));
  lcd.print(":");
  if (now.minute() < 10) lcd.print("0");
  lcd.print(String(now.minute()));
  lcd.print(":");
  if (now.second() < 10) lcd.print("0");
  lcd.print(String(now.second()));

}

void setup() {
  Wire.begin();
  pinMode(backlightPin, OUTPUT);
  pinMode(menuButtonPin, INPUT_PULLUP);
  pinMode(hoursButtonPin, INPUT_PULLUP);
  pinMode(minutesButtonPin, INPUT_PULLUP);
  pinMode(powerIndPin, OUTPUT);
  pinMode(tempIndPin, OUTPUT);
  pinMode(baroIndPin, OUTPUT);
  digitalWrite(powerIndPin, HIGH);
  Serial.begin(9600);                         
  bme.begin();
  rtc.begin();  
  lcd.init();
  lcd.clear();
  lcd.backlight();
}


void loop() {
  int photo = analogRead(photoresPin);
  controlBacklight(photo);
  int temp = bme.readTemperature() - 3;
  int baro = bme.readPressure() * 0.00750062;

  if (temp > 25 || temp < 20) {
    digitalWrite(tempIndPin, HIGH);
  }
  else {
    digitalWrite(tempIndPin, LOW);
  }
  if (baro < 735 || baro > 755) {
    digitalWrite(baroIndPin, HIGH);
  }
  else {
    digitalWrite(baroIndPin, LOW);
  }
  
  display(temp, baro);
  displayTime();
  delay(1000);
}
 
