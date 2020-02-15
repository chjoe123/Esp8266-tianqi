#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

WiFiClient client;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char *host = "api.seniverse.com";
const char *ssid = "TP-LINK_903E";          
const char *password = "DmgZg2312";      
const char *city = "xian";
const char *key = "SvapyYFkj7EvAAqfG";
const unsigned long HTTP_TIMEOUT = 2100;
const size_t MAX_CONTENT_SIZE = 2048;


struct UserData {
  char city[16];
  char weather[16];
  char temperature[16];
};


bool skipResponseHeaders() {
  char endOfHeaders[] = "\r\n\r\n";
  client.setTimeout(HTTP_TIMEOUT);
  bool ok = client.find(endOfHeaders);
  if(!ok)
  {
      Serial.println("服务器未响应！");
  }
  return ok;
}

bool sendRequst(const char *host, const char *key, const char *city) {
  //https://api.seniverse.com/v3/weather/now.json?key=SvapyYFkj7EvAAqfG&location=beijing&language=zh-Hans&unit=c
  String url = "/v3/weather/now.json?key=";
  url += key;
  url += "&location=";
  url += city;
  url += "&language=en";

  client.print(String("GET ") + url + " HTTP/1.1\r\n" + 
                "Host: " + host + "\r\n" + 
                "Connection: close\r\n\r\n");
  return true;
}


void readResponseContent(char *content, size_t maxSize) {
  size_t length = client.readBytes(content, maxSize);
  delay(100);
  content[length] = 0;
  Serial.println(content);
}


bool parseUserData(char *content, struct UserData *userData) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(content);

  if(!root.success())
  {
      Serial.println("解析失败！");
      return false;
  }
  strcpy(userData->city, root["results"][0]["location"]["name"]);
  strcpy(userData->weather, root["results"][0]["now"]["text"]);
  strcpy(userData->temperature, root["results"][0]["now"]["temperature"]);
}

void displayText(struct UserData *userData) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("City: ");
  display.println(userData->city);
  display.print("temp: ");
  display.print(userData->temperature);
  display.println(" C");
  display.println("weat: ");
  display.println(userData->weather);
}


void setup() {
  // put your setup code here, to run once:
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);

  //显示连接状态
  while(WiFi.status() != WL_CONNECTED)
  {
      delay(500);
      Serial.print(".");
  }
  Serial.println();
  Serial.println();
  //连接成功
  Serial.println("WiFi Connected!");
  delay(500);
}

void loop() {
  // put your main code here, to run repeatedly:
  //测试是否连接服务器
  if(!client.connect(host, 80))
  {
      Serial.println("连接服务器失败！");
      return;
  }
  if(sendRequst(host, key, city) && skipResponseHeaders())
  {
      char response[MAX_CONTENT_SIZE];
      readResponseContent(response, sizeof(response));
      UserData userData;                          //声明结构体变量
      parseUserData(response, &userData);
      displayText(&userData);
      display.display();
  }
  delay(30000);
}
