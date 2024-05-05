//C++
#include <iostream>
#include <string>
//Arduino
#include "ArduinoOTA.h"
#include <WiFiUdp.h>
#include <WakeOnLan.h>
WiFiUDP UDP;
WakeOnLan WOL(UDP);  // Pass WiFiUDP class
//#include <WiFiManager.h>

#include <ESP8266mDNS.h>
#include <ESP8266httpUpdate.h>
#include <FS.h>
#include "LittleFS.h"
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <AceRoutine.h>
#include <AsyncElegantOTA.h>
#include <ESPAsyncWiFiManager.h>

//LCD
#include <SPI.h>
#include <TFT_eSPI.h>
#include "Back.h"
#include "Wifi0.h"
#include "Wifi1.h"
#include "Wifi2.h"
#include "Wifi3.h"
#include "Wifi4.h"
#include "Wifi5.h"
#include "Wifi6.h"

//Sensor
#include <OneWire.h>
#include <DallasTemperature.h>
//dht11
#include <DHT.h>

#define ONE_WIRE_BUS 5  //port sensor
#define DHTPIN 5
#define DHTTYPE DHT11

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DHT dht(DHTPIN, DHTTYPE);


using namespace ace_routine;


AsyncWebServer server = AsyncWebServer(80);
DNSServer dns;
WebSocketsServer webSocket = WebSocketsServer(81);
AsyncWiFiManager wifiManager(&server, &dns);

TFT_eSPI tft = TFT_eSPI();

bool websocketStarted;
unsigned long nextWebSocketUpdateTime = 0;
bool isConnected;
float temp;
float hum;
int SetHourse = 0;
int SetMinutes = 0;

void setup() {
  Serial.begin(19200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  //Start LittleFS
  if (!LittleFS.begin()) {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }

  //LCD
  tft.init();                 // инициализируем дисплей
  tft.setRotation(2);         // поворачиваем дисплей на 90 градусов
  tft.fillScreen(TFT_BLACK);  // очищаем экран черным цветом*/
  PngRead();

  displog(".");

  //подключение wifi
  //WiFi.begin("Wi_Fi", "Evolve239638");
  wifiManager.setDebugOutput(true);
  isConnected = wifiManager.autoConnect("LOLIN");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  displog("..");

  AsyncElegantOTA.begin(&server);

  MDNS.begin("portal");
  MDNS.addService("http", "tcp", 80);
  startWebServer();
  startWebSocket();

  displog("...");

  //запуск OTA
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  displog("....");

  dht.begin();      // запускаем датчик влаги
  sensors.begin();  // запускаем датчик температуры

  displog(".....");

  delay(3000);

  displog("Status:OK");
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  webSocket.loop();
  MDNS.update();
  ArduinoOTA.handle();
  dispUpd();
  Clock();
}

void dispUpd() {
  static uint32_t tmr;
  float tempold;
  float humold;
  float memold;
  float memcur;

  if (millis() - tmr >= 30000 || tmr == 0) {

    tmr = millis();

    hum = dht.readHumidity();  //запрос влаги

    sensors.requestTemperatures();            //запрос температуры
    delay(1000);
    temp = sensors.getTempCByIndex(0);  //получение температуры

    float memcurmat = ESP.getFreeHeap();  //получение FreeRAM
    memcur = memcurmat / 8000;

    int rssi = WiFi.RSSI();
    WifiSignal(rssi);

    // загружаем шрифт
    if (temp != tempold) {
      tft.loadFont("bahnschrift16", LittleFS);
      tft.fillRect(68, 22, 46, 17, 0x4982);
      tft.setTextColor(0xFD49);  //текст
      tft.setCursor(70, 24);
      tft.print(temp);
      tempold = temp;
      tft.unloadFont();
    }
    if (hum != humold) {
      tft.loadFont("bahnschrift16", LittleFS);
      tft.fillRect(40, 45, 70, 30, 0x4982);
      tft.setTextColor(0xFD49);  //текст
      tft.setCursor(70, 56);
      tft.print(hum);
      humold = hum;
      tft.unloadFont();
    }
    if (memcur != memold) {
      tft.loadFont("couriernew12", LittleFS);
      tft.fillRect(125, 126, 85, 35, 0x4982);
      tft.setTextColor(0xFD49);
      tft.setCursor(130, 135);
      tft.print(String(memcur) + " KB");
      tft.unloadFont();
      memold = memcur;
    }
  }
}

void displog(String text) {

  text.replace("::", "");

  tft.loadFont("couriernew12", LittleFS);
  tft.fillRect(14, 85, 100, 29, 0x4982);
  tft.setTextColor(0xFD49);  //текст
  tft.setCursor(15, 93);
  tft.print(text);
  tft.unloadFont();
}

void WakePC() {
  const char *MACAddress = "00:D8:61:53:A7:69";
  WOL.sendMagicPacket(MACAddress);
}

void PngRead() {
  tft.setSwapBytes(true);
  tft.pushImage(0, 0, 240, 240, Back);
  tft.setSwapBytes(false);
}

void WifiSignal(int sig)
{
  tft.setSwapBytes(true);
  if(sig > -30){ tft.pushImage(128, 189, 30, 40, Wifi6); }
  if(sig < -55){ tft.pushImage(128, 189, 30, 40, Wifi5); }
  if(sig < -67){ tft.pushImage(128, 189, 30, 40, Wifi4); }
  if(sig < -70){ tft.pushImage(128, 189, 30, 40, Wifi3); }
  if(sig < -80){ tft.pushImage(128, 189, 30, 40, Wifi2); }
  if(sig < -90){ tft.pushImage(128, 189, 30, 40, Wifi1); }
  tft.setSwapBytes(false);
}

void ConnectedStatusOnline() {
  tft.fillCircle(169, 179, 10, TFT_GREEN);
}

void ConnectedStatusDisconnected() {
  tft.fillCircle(169, 179, 10, TFT_RED);
}

void Clock() {
  static uint32_t tmrCl;

  if (millis() - tmrCl >= 60000) {
    tmrCl = millis();
    char clockstr[16];
    uint32_t sec = millis() / 1000ul;      // полное количество секунд
    int Hours = (sec / 3600ul);        // часы
    int Mins = (sec % 3600ul) / 60ul;  // минуты
    int Day = (Hours % 3600ul) / 24ul;
    tft.fillRect(30, 135, 85, 20, 0x4982);
    tft.loadFont("couriernew12", LittleFS);
    tft.setTextColor(0xFD49);  //текст
    tft.setCursor(30, 135);
    if(Day > 0) { Hours -= Day * 24; }
    sprintf(clockstr,"%02u d %02u:%02u", Day, Hours, Mins);
    //tft.print(String(Day) + "d " + String(Hours) + ":" + String(Mins));
    tft.print(clockstr);
    tft.unloadFont();
  }
}

void SendToWeb() {
  float memcurmat = ESP.getFreeHeap();
  float mem = memcurmat / 8000;
  long rssi = WiFi.RSSI();
  String Data = String(temp, 2) + "_" + String(hum, 2) + "_" + String(mem, 2) + "_" + String(rssi, 2);

  webSocket.broadcastTXT(Data);
}