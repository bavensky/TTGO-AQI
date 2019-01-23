#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "SSD1306.h"

const char* ssid = "T123456";
const char* password = "V4bTaMaTo";

//OLED pins to ESP32 GPIOs via this connecthin:
//OLED_SDA -- GPIO4
//OLED_SCL -- GPIO15
//OLED_RST -- GPIO16
SSD1306  display(0x3c, 4, 15);

uint32_t pevTime = 0;

void getAQI(String web, int timeDelay) {
  uint32_t curTime = millis();
  if (curTime - pevTime > (timeDelay * 1000)) {
    pevTime = curTime;
    if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
      HTTPClient http;  //Declare an object of class HTTPClient
      //    http.begin("http://jsonplaceholder.typicode.com/users/1");  //Specify request destination

      //   http://aqicn.org/city/thailand/chiangmai---cmis/
      http.begin(web);

      int httpCode = http.GET();                                                                  //Send the request
      if (httpCode > 0) { //Check the returning code
        String payload = http.getString();   //Get the request response payload
        Serial.println(payload);                     //Print the response payload

        // Allocate JsonBuffer
        // Use arduinojson.org/assistant to compute the capacity.
        const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
        DynamicJsonBuffer jsonBuffer(capacity);
        JsonObject& root = jsonBuffer.parseObject(payload);
        if (!root.success()) {
          Serial.println(F("Parsing failed!"));
          return;
        }
        Serial.println("Response : ");
        Serial.print("Status : ");
        Serial.println(root["status"].as<char*>());
        Serial.print("aqi : ");
        Serial.print(root["data"]["aqi"].as<char*>());
        Serial.print(" idx : ");
        Serial.println(root["data"]["idx"].as<char*>());

        display.setFont(ArialMT_Plain_24);
        display.drawString(0, 0, "ESP32 AQI");
        display.setFont(ArialMT_Plain_16);
        display.drawString(0, 24, "status = ");
        display.drawString(64, 24, root["status"].as<char*>());
        display.drawString(0, 40, "AQI = ");
        display.drawString(48, 40, root["data"]["aqi"].as<char*>());
        display.display();
      }
      http.end();   //Close connection
    }
  }
}

void setup () {
  Serial.begin(115200);

  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high

  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 0, "ESP32 AQI");
  display.display();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("WiFi Connected...");
}

void loop() {
  getAQI("http://api.waqi.info/feed/shanghai/?token=demo", 30);
}
