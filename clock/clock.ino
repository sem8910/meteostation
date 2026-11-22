#define _LCD_TYPE 1

#include <GyverBME280.h>
#include <I2C_LiquidCrystal_RUS.h>
#include <Wire.h>
#include <RTClib.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

I2C_LiquidCrystal_RUS lcd(0x27, 16, 2, 2); 
GyverBME280 bme;
RTC_DS3231 rtc;

const int backlightPin = 2; 
const int relayPin = 14;
const int powerIndPin = 0;
const int photoresPin = A0;
const char* ap_ssid = "Wemos_AP";
const char* ap_password = "password123";
bool menuState = HIGH;
float tempOffset = -3.0;
int correctionBaro = 14;

ESP8266WebServer server(80);

void controlBacklight(int photoValue) {
  int bright = map(photoValue, 650, 1023, 255, 20);
  bright = constrain(bright, 20, 255);
  analogWrite(backlightPin, bright);
}

void display(int temp, int baro, int hud) {
  lcd.setCursor(0, 0);
  lcd.print("t: ");
  lcd.print(String(temp));
  lcd.write(223);
  lcd.print("C");
  lcd.print(" P: ");
  lcd.print(String(baro));
  lcd.print("mm      ");
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

void handleRoot() {
  int t = bme.readTemperature() + tempOffset;
  int p = bme.readPressure() * 0.00750062 + 14;
  DateTime now = rtc.now();
  char timeStr[9];
  sprintf(timeStr, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<title>Метеостанция</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 20px; }";
  html += "input { width: 80px; padding: 5px; }";
  html += "button { padding: 5px 10px; }";
  html += "</style></head><body>";
  html += "<h2>Метеостанция</h2>";
  html += "<p>Температура: <b>" + String(t) + " &deg;C</b></p>";
  html += "<p>Давление: <b>" + String(p) + " мм рт. ст.</b></p>";
  html += "<p>Время до синхронизации: " + String(timeStr) + "</p>";
  html += "<p id='syncStatus' style='color: #888; font-weight: bold;'>Синхронизация времени...</p>";
  html += "<hr>";
  html += "<form action='/save' method='POST'>";
  html += "Поправка температуры: <input type='number' step='0.1' name='offset' value='" + String(tempOffset, 1) + "'> ";
  html += "<button type='submit'>ОК</button>";
  html += "</form>";
  html += "<script>";
  html += "window.onload = function() {";
  html += "  let d = new Date();";
  html += "  let url = `/sync?y=${d.getFullYear()}&m=${d.getMonth()+1}&d=${d.getDate()}&h=${d.getHours()}&min=${d.getMinutes()}&s=${d.getSeconds()}`;";
  html += "  let st = document.getElementById('syncStatus');";
  html += "  fetch(url).then(r => {";
  html += "    if (r.ok) {";
  html += "      st.innerText = 'Время синхронизировано (' + d.toLocaleTimeString() + ')';";
  html += "      st.style.color = 'green';";
  html += "    } else {";
  html += "      st.innerText = 'Ошибка синхронизации';";
  html += "      st.style.color = 'red';";
  html += "    }";
  html += "  }).catch(() => {";
  html += "    st.innerText = 'Нет связи с платой';";
  html += "    st.style.color = 'red';";
  html += "  });";
  html += "};";
  html += "</script>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleSave() {
  if (server.hasArg("offset")) {
    tempOffset = server.arg("offset").toFloat();
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSync() {
  if (server.hasArg("y") && server.hasArg("m") && server.hasArg("d") &&
      server.hasArg("h") && server.hasArg("min") && server.hasArg("s")) {
    int y = server.arg("y").toInt();
    int m = server.arg("m").toInt();
    int d = server.arg("d").toInt();
    int h = server.arg("h").toInt();
    int min = server.arg("min").toInt();
    int s = server.arg("s").toInt();
    rtc.adjust(DateTime(y, m, d, h, min, s));
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Error");
  }
}

void setup() {
  Wire.begin();
  pinMode(backlightPin, OUTPUT);
  pinMode(powerIndPin, OUTPUT);
  digitalWrite(powerIndPin, HIGH);
  digitalWrite(relayPin, HIGH);
  WiFi.softAP(ap_ssid, ap_password);
  Serial.begin(9600);
  Serial.println(WiFi.softAPIP());
  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/sync", handleSync);
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
  int temp = bme.readTemperature() + tempOffset;
  int baro = bme.readPressure() * 0.00750062 + correctionBaro;
  server.handleClient();
  display(temp, baro, 1);
  displayTime();
  delay(1000);
}
