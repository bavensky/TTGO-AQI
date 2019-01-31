#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "SSD1306.h"

const char *ssid = "T123456";
const char *password = "V4bTaMaTo";

//OLED pins to ESP32 GPIOs via this connecthin:
//OLED_SDA -- GPIO4
//OLED_SCL -- GPIO15
//OLED_RST -- GPIO16
SSD1306 display(0x3c, 4, 15);

uint32_t pevTime = 0;

void getAQI(String web, int timeDelay)
{
  uint32_t curTime = millis();
  if (curTime - pevTime > (timeDelay * 1000))
  {
    pevTime = curTime;
    if (WiFi.status() == WL_CONNECTED)
    { //Check WiFi connection status
      HTTPClient http; //Declare an object of class HTTPClient
      //    http.begin("http://jsonplaceholder.typicode.com/users/1");  //Specify request destination

      //   http://aqicn.org/city/thailand/chiangmai---cmis/
      http.begin(web);

      int httpCode = http.GET(); //Send the request
      if (httpCode > 0)
      { //Check the returning code
        String payload = http.getString(); //Get the request response payload
        Serial.println(payload);           //Print the response payload

        // Allocate JsonBuffer
        // Use arduinojson.org/assistant to compute the capacity.
        //        const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_ARRAY_SIZE(2) + 60;
        const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(3) + 12 * JSON_OBJECT_SIZE(1) + 4 * JSON_OBJECT_SIZE(2) + 2 * JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(8) + JSON_OBJECT_SIZE(11);
        DynamicJsonBuffer jsonBuffer(capacity);
        //        StaticJsonBuffer<1000> jsonBuffer;
        JsonObject &root = jsonBuffer.parseObject(payload);
        if (!root.success())
        {
          Serial.println(F("Parsing failed!"));
          return;
        }
        Serial.println(F("Parsing success!"));

        const char* status = root["status"]; // "ok"
        JsonObject& data = root["data"];
        int data_aqi = data["aqi"]; // 152

        JsonObject& data_city = data["city"];
        float data_city_geo0 = data_city["geo"][0]; // 18.787747
        float data_city_geo1 = data_city["geo"][1]; // 98.9931284
        const char* data_city_name = data_city["name"]; // "Chiang Mai"

        Serial.println("ESP32 AQI ");
        Serial.print("status : ");
        Serial.println(status);
        Serial.print("name : ");
        Serial.print(data_city_name);
        Serial.print(" AQI : ");
        Serial.println(data_aqi);
        Serial.print("Location : ");
        Serial.print(data_city_geo0);
        Serial.print(",");
        Serial.println(data_city_geo1);

        display.clear();
        display.setFont(ArialMT_Plain_24);
        display.drawString(0, 0, "ESP32 AQI");
        display.setFont(ArialMT_Plain_16);
        display.drawString(0, 24, "status = ");
        display.drawString(64, 24, root["status"].as<char *>());
        display.drawString(0, 40, "AQI = ");
        display.drawString(48, 40, root["data"]["aqi"].as<char *>());
        display.display();
      }
      http.end(); //Close connection
    }
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(16, OUTPUT);
  digitalWrite(16, LOW); // set GPIO16 low to reset OLED
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
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("WiFi Connected...");
}

void loop()
{
  getAQI("http://api.waqi.info/feed/chiangmai/?token=c215825b5de34b7c75cd570d1cf2c1e957acc963", 30);
}
