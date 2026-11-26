#define _LCD_TYPE 1


#include <GyverBME280.h>
#include <I2C_LiquidCrystal_RUS.h>
#include <Wire.h>
#include <RTClib.h>
#include <DHT.h>  
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

I2C_LiquidCrystal_RUS lcd(0x27, 16, 2, 2); 
GyverBME280 bme;
RTC_DS3231 rtc;

#define DHTPIN 12
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const int backlightPin = 2; // пин подсветки
const int relayPin = 14; // пин реле
const int powerIndPin = 0; // пин индикатора питания.красный светодиод
const int photoresPin = A0; // аналоговый пин фоторезистора
const char* ap_ssid = "meteostation"; // имя точки доступа, которую создает контроллер
const char* ap_password = "1234567890"; // пароль к точке доступа
float tempOffset = -3.0; // начальная поправка температуры
int correctionBaro = 14; // поправка давления
int correctionSeconds = 2; // поправка секунд
ESP8266WebServer server(80);

void controlBacklight(int photoValue) { // ркгулировка подсветки дисплея взависимости от света
  int bright = map(photoValue, 650, 1023, 255, 20);
  bright = constrain(bright, 20, 255);
  analogWrite(backlightPin, bright);
}

void controlHud(int hud) { // включение и отключение увлажнителя воздуха
  if (hud < 40) {
    digitalWrite(relayPin, LOW);
    Serial.println("Вкл");
  }
  else if (hud >= 52) {
      digitalWrite(relayPin, HIGH);
      Serial.print("выкл");
  }
}

void display(int temp, int baro, int hud) { // вывод на дисплей температуры, давления и влажности
  lcd.setCursor(0, 0);
  lcd.print("t: ");
  lcd.print(String(temp));
  lcd.write(223);
  lcd.print("C");
  lcd.print(" P: ");
  lcd.print(String(baro));
  lcd.print("mm      ");
  lcd.setCursor(10, 1);
  lcd.print("h: ");
  lcd.print(String(hud));
  lcd.print("%      ");
}

void displayTime() { // вывод времени
  DateTime now = rtc.now();
  lcd.setCursor(0, 1);
  if (now.hour() < 10) lcd.print("0");
  lcd.print(String(now.hour()));
  lcd.print(":");
  if (now.minute() < 10) lcd.print("0");
  lcd.print(String(now.minute()));
  lcd.print(":");
  if (now.second() + correctionSeconds < 10) lcd.print("0");
  lcd.print(String(now.second()) + " ");
}

void handleRoot() { // прописана основная страница
  int t = bme.readTemperature() + tempOffset;
  int p = bme.readPressure() * 0.00750062 + 14;
  DateTime now = rtc.now();
  char timeStr[9];
  sprintf(timeStr, "%02d:%02d:%02d", now.hour(), now.minute(), now.second() + correctionSeconds); // финальное правильный формат строки
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
  html += "<p id='syncStatus' style='color: #888; font-weight: bold;'>Синхронизация времени...</p>"; // проверка на запрос синхронизации
  html += "<hr>";
  html += "<form action='/save' method='POST'>";
  html += "Поправка температуры: <input type='number' step='0.1' name='offset' value='" + String(tempOffset, 1) + "'> "; // поле для ввода темпратуры
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

void handleSave() { // принимается и перезаписывается поправка температуры
  if (server.hasArg("offset")) {
    tempOffset = server.arg("offset").toFloat();
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSync() { // синхронизация времени с телефоном и полусение данных с него
  if (server.hasArg("y") && server.hasArg("m") && server.hasArg("d") &&
      server.hasArg("h") && server.hasArg("min") && server.hasArg("s")) {
    int y = server.arg("y").toInt();
    int m = server.arg("m").toInt();
    int d = server.arg("d").toInt();
    int h = server.arg("h").toInt();
    int min = server.arg("min").toInt();
    int s = server.arg("s").toInt();
    rtc.adjust(DateTime(y, m, d, h, min, s + correctionSeconds));
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Error");
  }
}

void setup() {
  Wire.begin();
  pinMode(backlightPin, OUTPUT);
  pinMode(powerIndPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
  
  digitalWrite(powerIndPin, HIGH);
  digitalWrite(relayPin, HIGH);
  WiFi.softAP(ap_ssid, ap_password); // запуск точки доступа
  Serial.begin(9600);

  // запуск путей
  server.on("/", handleRoot); 
  server.on("/save", HTTP_POST, handleSave);
  server.on("/sync", handleSync);
  server.begin(); // запуск сервера

  // инициализация i2c
  bme.begin();
  dht.begin();
  rtc.begin();
  lcd.init();

  // очистка дисплея и включение подсветки
  lcd.clear();
  lcd.backlight();
}

void loop() {
  int photo = analogRead(photoresPin); //получение данных с фоторезистора
  controlBacklight(photo);
  int temp = dht.readTemperature() + tempOffset; // получение данных с DHT11  и прибавка к температуре заданную корректироку
  int baro = bme.readPressure() * 0.00750062 + correctionBaro; // получение данных о давлении и корректирование их
  int hud = dht.readHumidity(); // получение влажности
  server.handleClient(); // проверка запросов
  display(temp, baro, hud);
  displayTime();
  controlHud(hud);
  delay(1000); //задержка перед новым циклом
}
