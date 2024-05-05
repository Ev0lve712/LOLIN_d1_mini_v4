void startWebServer()
{

  server.serveStatic("/", LittleFS, "/");


  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(LittleFS, "index.html", String(), false, processor);
  });

  server.onNotFound([](AsyncWebServerRequest * request)
  {
    if (request->method() == HTTP_OPTIONS) {
      request->send(200);
    } else {
      request->send(404);
    }
  });

  /*server.on("/wol", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    if (request->hasParam("wake"))
    {
      WakePC();
      AsyncWebParameter *LogText = request->getParam("wake");
      String log = LogText->value();
      request->send(200);
    } else {
      request->send(404);
    }
  });*/

  server.on("/reset_wifi", HTTP_GET, [](AsyncWebServerRequest * request) {
    
    request->send(200, "text/html", "Wifi reset");
  });

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  server.begin();
}

String processor(const String &var)
{
  if (var == "IP")
    return WiFi.localIP().toString();
  return String();
}