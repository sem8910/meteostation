#include <GyverBME280.h>
#define _LCD_TYPE 1
#include <I2C_LiquidCrystal_RUS.h>
#include <Wire.h>
#include <RTClib.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

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
const char* ap_ssid = "Wemos_AP";
const char* ap_password = "password123";
bool menuState = HIGH;

ESP8266WebServer server(80);


void controlBacklight(int photoValue) {
  int bright = map(photoValue, 650, 1023, 255, 20);
  bright = constrain(bright, 20, 255);
  analogWrite(backlightPin, bright);
}

void handleRoot() {
  int t = bme.readTemperature() - 3;
  int p = bme.readPressure() * 0.00750062;
  
  DateTime now = rtc.now();
String timeStr = "";
if (now.hour() < 10) timeStr += "0";
timeStr += String(now.hour()) + ":";
if (now.minute() < 10) timeStr += "0";
timeStr += String(now.minute()) + ":";
if (now.second() < 10) timeStr += "0";
timeStr += String(now.second());
  String html = "<!DOCTYPE html><html lang='ru'><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<meta http-equiv='refresh' content='5'>";
  html += "<title>Моя метеостанция</title>";
  html += "<style>";
  html += "* { box-sizing: border-box; margin: 0; padding: 0; }";
  html += "body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; background: #121212; color: #ffffff; display: flex; flex-direction: column; align-items: center; min-height: 100vh; padding: 20px; }";
  html += "h1 { font-size: 24px; font-weight: 600; margin: 15px 0 25px; text-align: center; color: #e0e0e0; }";
  html += ".container { width: 100%; max-width: 400px; display: flex; flex-direction: column; gap: 16px; }";
  html += ".card { background: #1e1e1e; border-radius: 16px; padding: 24px; text-align: center; box-shadow: 0 4px 12px rgba(0,0,0,0.4); }";
  html += ".label { font-size: 14px; text-transform: uppercase; letter-spacing: 1px; color: #9e9e9e; margin-bottom: 8px; }";
  html += ".value { font-size: 48px; font-weight: 700; color: #4fc3f7; }";
  html += ".unit { font-size: 22px; font-weight: 400; color: #b0bec5; }";
  html += ".footer { margin-top: 24px; font-size: 14px; color: #757575; }";
  html += "</style></head><body>";
  html += "<h1>Моя метеостанция</h1>";
  html += "<div class='container'>";
  html += "<div class='card'><div class='label'>Температура</div><div class='value'>" + String(t) + "<span class='unit'> °C</span></div></div>";
  html += "<div class='card'><div class='label'>Давление</div><div class='value'>" + String(p) + "<span class='unit'> мм рт. ст.</span></div></div>";
  html += "</div>";
  html += "<div class='footer'>Время станции: " + timeStr + "</div>";
  html += "</body></html>";
  server.send(200, "text/html", html);
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
  WiFi.softAP(ap_ssid, ap_password);
  Serial.println(WiFi.softAPIP());
  server.on("/", handleRoot);
  server.begin();                         
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
  server.handleClient();
  display(temp, baro);
  displayTime();
  delay(1000);
}
 
