void startWebSocket() {
  // Start a WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);  // if there's an incomming websocket message, go to function 'webSocketEvent'
  websocketStarted = true;
  Serial.println("WebSocket server started.");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t lenght) {
  // When a WebSocket message is received
  switch (type) {
    case WStype_ERROR:
      Serial.printf("Error: [%f]", payload);
      break;
    case WStype_BIN:

      break;
    case WStype_TEXT:

      if (strcmp((char *)payload, "wake") == 0) {
        WakePC();
      }

      if (strncmp((char *)payload, "::", 2) == 0) {
        
        displog((char *)payload);
      }
      
      if (strcmp((char *)payload, "refresh") == 0) {
        SendToWeb();
      }

      if(strncmp((char *)payload, "setClock", 8) == 0) {
        int a;
        int b;
        scanf("%d:%d", &a, &b);
        SetHourse = a;
        SetMinutes = b;
      }

      break;
    case WStype_DISCONNECTED:  // if the websocket is disconnected
      ConnectedStatusDisconnected();
      break;
    case WStype_CONNECTED:  // if a new websocket connection is established
      ConnectedStatusOnline();
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      break;
  }
}