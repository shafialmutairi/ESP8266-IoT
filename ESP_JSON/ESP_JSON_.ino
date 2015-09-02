#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Adafruit_NeoPixel.h>
#include <Ticker.h>
#include <ArduinoJson.h>

Ticker flipper;

int count = 0;

void flip() {
  int state = digitalRead(13);  // get the current state of GPIO1 pin
  digitalWrite(13, !state);     // set pin to the opposite state

  if (++count >= 2)
    flipper.detach();
}



#define PIN 5
Adafruit_NeoPixel strip = Adafruit_NeoPixel(3, PIN, NEO_GRB + NEO_KHZ800);
void colorWipe(uint32_t c) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
  }
}

const char* ssid = "Z3";
const char* password = "0503209205";
MDNSResponder mdns;

ESP8266WebServer server(80);

void handleRoot() {

  StaticJsonBuffer<500> jsonBuffer;  // JSON Buffer
  char buff[512];   // receive & send buff ..
  String message = "";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  Serial.println(message);

  // ... Check Args Received ...
  for (uint8_t i = 0; i < server.args(); i++) {
    if (server.argName(i).equals("seek")) {
      int value = server.arg(i).toInt();
      analogWrite(4, value);
    }else if (server.argName(i).equals("gpio4")) {
      int value = server.arg(i).toInt();
      if (value == 1)
        strip.setPixelColor(0, strip.Color(0, 0, 50));
      else
        strip.setPixelColor(0, strip.Color(50, 0, 0));
      digitalWrite(4, value);
      strip.show();
    }    
    else if (server.argName(i).equals("gpio12")) {
      int value = server.arg(i).toInt();
      if (value == 1)
        strip.setPixelColor(1, strip.Color(0, 0, 50));
      else
        strip.setPixelColor(1, strip.Color(50, 0, 0));
      digitalWrite(12, value);
      strip.show();
    }
    else if (server.argName(i).equals("gpio13")) {
      int value = server.arg(i).toInt();
      if (value == 1)
        strip.setPixelColor(2, strip.Color(0, 0, 50));
      else
        strip.setPixelColor(2, strip.Color(50, 0, 0));
      digitalWrite(13, value);
      strip.show();

    }
    if (server.argName(i).equals("gpio14")) {
      int value = server.arg(i).toInt();
      if (value == 1)
        strip.setPixelColor(2, strip.Color(0, 0, 50));
      else
        strip.setPixelColor(2, strip.Color(50, 0, 0));
      digitalWrite(14, value);
      strip.show();
    }
    else if (server.argName(i).equals("hex")) {
      int value = server.arg(i).toInt();
      colorWipe(value);
    }
    else if (server.argName(i).equals("plain")) {
      //// Receive Json Format ///////
      server.arg(0).toCharArray(buff, sizeof(buff));
      JsonObject& root = jsonBuffer.parseObject(buff);
      if (!root.success()) {
        Serial.println("parseObject() failed");
        return;
      }
      const char* sensor = root["sensor"];
      long time = root["time"];
      Serial.print("Sensor:"); Serial.println(sensor);
      Serial.print("time:"); Serial.println(time);
    }
  }

  ////////  Send Json Format ////////
  JsonObject& rootSend = jsonBuffer.createObject();
  rootSend["name"]    = "Shafi S.Almutairi";
  rootSend["sensor"]  = "gps";
  rootSend["time"]    = 1351824120;
  JsonArray& data = rootSend.createNestedArray("data");
  data.add(48.756080, 6);
  data.add(2.302038, 6);
  rootSend.printTo(buff, sizeof(buff));


  server.send(200, "text/plain", buff);
}

void handleNotFound() {
  server.send(404, "text/plain", "Wrong address Access...");
}


void StartServerMode() {

  strip.setPixelColor(0, strip.Color(50, 0, 50));
  strip.setPixelColor(1, strip.Color(50, 0, 50));
  strip.setPixelColor(2, strip.Color(50, 0, 50));
  strip.show();
  Serial.println("Finishd Sequence...");
}
void ServerMode() {

  long currenttime = millis();
  long limittime = 5000; // 5 second
  long diff = 0;
  boolean trigger = false;
  while (digitalRead(14) == HIGH && trigger == false) {
    diff = millis() - currenttime;
    if (diff >= limittime)
      trigger = true;
  }
  int value = digitalRead(4);
  digitalWrite(4,!value);
  Serial.print("Trigger Value: ");
  Serial.print(trigger);
  Serial.print(" time: ");
  Serial.println(diff);
  if (trigger == true)
    StartServerMode();

}
void setup(void) {
  pinMode(4, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(16, OUTPUT);

  pinMode(14, INPUT);

  digitalWrite(4, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
  digitalWrite(16, LOW);

  attachInterrupt(14, ServerMode, RISING);
  strip.begin();
  strip.show();
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // flip the pin every 0.3s
  flipper.attach(0.1, flip);
  // Wait for connection
  colorWipe(strip.Color(50, 0, 0)); // set RED Color
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  colorWipe(strip.Color(0, 50, 0)); // set GREEN Color

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  if (WiFi.status() != WL_CONNECTED) {
    // Wait for connection
    colorWipe(strip.Color(50, 0, 0)); // set RED Color
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    colorWipe(strip.Color(0, 50, 0)); // set GREEN Color

  }
}
