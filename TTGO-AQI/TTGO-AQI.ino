//#include <WiFi.h>
#include <pm25senses.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "SSD1306.h"

//const char *ssid = "T123456";
//const char *password = "V4bTaMaTo";
const char *ssid = "CMMC_Sinet_2.4G";
const char *passw = "zxc12345";

String sendername;
String lat, lng;
float pm25, pm10;
String response;

pm25senses mydevice;

//OLED pins to ESP32 GPIOs via this connecthin:
//OLED_SDA -- GPIO4
//OLED_SCL -- GPIO15
//OLED_RST -- GPIO16
SSD1306 display(0x3c, 4, 15);

#define DISPLAY_DURATION (5 * 6000)  // send data every 5 minute
typedef void (*screen)(void);
int screenCount = 0;
int counter = 1;

const char* status;
int data_aqi;               // Air Quality Index (AQI)
float data_city_geo0;       // 18.787747
float data_city_geo1;       // 98.9931284
const char* data_city_name; // "ChiangMai"
const char* data_time_s;    // "2019-01-30 10:00:00"
float data_iaqi_pm25_v;       // 152
float data_iaqi_pm10_v;       // 55


#define LED_ONBOARD 16
uint32_t pevTime = 0;

void mainDisplay() {
  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 0, "ESP32 AQI");
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 24, "City = ");
  display.drawString(48, 24, data_city_name);
  display.drawString(0, 40, "AQI = ");
  display.drawString(48, 40, String(data_aqi));
}

void timeDisplay() {
  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 0, "ESP32 AQI");
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 24, "Date ");
  display.drawString(24, 24, data_time_s);
  display.drawString(0, 34, "City ");
  display.drawString(24, 34, data_city_name);
  display.drawString(0, 44, "Location ");
  display.drawString(44, 44, String(data_city_geo0));
  display.drawString(0, 44, ", ");
  display.drawString(74, 44, String(data_city_geo1));
}

void getAQI(String web)
{
  HTTPClient http; //Declare an object of class HTTPClient
  //    http.begin("http://jsonplaceholder.typicode.com/users/1");  //Specify request destination

  //   http://aqicn.org/city/thailand/chiangmai---cmis/
  http.begin(web);

  int httpCode = http.GET(); //Send the request
  if (httpCode > 0)
  { //Check the returning code
    String payload = http.getString(); //Get the request response payload
    //    Serial.println(payload);           //Print the response payload

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

    status = root["status"]; // "ok"
    JsonObject& data = root["data"];
    data_aqi = data["aqi"]; // 152

    JsonObject& data_city = data["city"];
    data_city_geo0 = data_city["geo"][0]; // 18.787747
    data_city_geo1 = data_city["geo"][1]; // 98.9931284
    data_city_name = data_city["name"]; // "Chiang Mai"

    JsonObject& data_time = data["time"];
    data_time_s = data_time["s"]; // "2019-01-30 10:00:00"

    JsonObject& data_iaqi = data["iaqi"];
    data_iaqi_pm25_v = data_iaqi["pm25"]["v"]; // 152
    data_iaqi_pm10_v = data_iaqi["pm10"]["v"]; // 55


    Serial.println("ESP32 AQI ");
    Serial.print("status : ");
    Serial.println(status);
    Serial.print("name : ");
    Serial.println(data_city_name);
    Serial.print("pm2.5 : ");
    Serial.print(data_iaqi_pm25_v);
    Serial.print(", pm10 : ");
    Serial.println(data_iaqi_pm10_v);
    Serial.print("Location : ");
    Serial.print(data_city_geo0, 6);
    Serial.print(",");
    Serial.println(data_city_geo1, 6);
  }
  http.end(); //Close connection
}

void setup()
{
  Serial.begin(115200);

  pinMode(LED_ONBOARD, OUTPUT);
  pinMode(16, OUTPUT);
  digitalWrite(16, LOW); // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high

  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  WiFi.mode(WIFI_STA);
  mydevice.begin(ssid, passw);
  response = mydevice.checkServerReady();
  Serial.println("PM25Senses response :" + String(response));

  //  WiFi.begin(ssid, password);
  //  while (WiFi.status() != WL_CONNECTED)
  //  {
  //    delay(1000);
  //    Serial.print(".");
  //  }
  //  Serial.println("WiFi Connected...");
  getAQI("http://api.waqi.info/feed/chiangmai/?token=c215825b5de34b7c75cd570d1cf2c1e957acc963");
}

screen showOLED[] = {mainDisplay, timeDisplay};
int screenLength = (sizeof(showOLED) / sizeof(screen));
long timeSinceLastModeSwitch = 0;

void loop()
{
  // clear the display
  display.clear();
  // draw the current demo method
  showOLED[screenCount]();

  // write the buffer to the display
  display.display();

  if (millis() - timeSinceLastModeSwitch > DISPLAY_DURATION) {
    getAQI("http://api.waqi.info/feed/chiangmai/?token=c215825b5de34b7c75cd570d1cf2c1e957acc963");

    sendername = "CMMC";
    lat = 18.787747;
    lng = 98.9931284;
    pm25 = data_iaqi_pm25_v;
    pm10 = data_iaqi_pm10_v;

    response = mydevice.reportPM25senses(sendername, lat, lng, pm25, pm10);
    Serial.print("response form server : ");
    Serial.println(response);
    Serial.println();
    Serial.println();

    // blink led when sending finish
    digitalWrite(LED_ONBOARD, !digitalRead(LED_ONBOARD));
    delay(100);
    digitalWrite(LED_ONBOARD, !digitalRead(LED_ONBOARD));
    
    screenCount = (screenCount + 1)  % screenLength;
    timeSinceLastModeSwitch = millis();
  }
  counter++;
  delay(10);

}
