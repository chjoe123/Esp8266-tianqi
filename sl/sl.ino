#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

const char* ssid = "";          
const char* password = "";         
const char* api_key = "";      
const char* city_id = "1790633";
unsigned long previousTime = 0;
const char* host = "api.openweathermap.org";
size_t MAX_CONTENT_SIZE = 1024;
WiFiClient client;

struct UserData  {
  char weather[25];
  char temp[16];
  char tempMin[16];
  char tempMax[16];
  char humidity[16];
  
};

bool sendRequest(const char* api_key, const char* host, const char* city_id) {
  //https://api.seniverse.com/v3/weather/now.json?key=SvapyYFkj7EvAAqfG&location=beijing&language=zh-Hans&unit=c
  //api.openweathermap.org/data/2.5/weather?id={city id}&appid={your api key}
  String getUrl = "/data/2.5/weather?id=";
  getUrl += city_id;
  getUrl += "&appid=";
  getUrl += api_key;
  getUrl += "&units=metric";

  client.print("GET " + getUrl + " HTTP/1.1\r\n" +
                "Host: " + host + "\r\n" +
                "Connection: close\r\n\r\n");
  return true;
}

bool skipHeaders() {
  char endOfHeaders[] = "\r\n\r\n";
  client.setTimeout(2100);
  bool ok = client.find(endOfHeaders);
  if(!ok)
  {
      Serial.println("Server not response!");
  }
  return ok;
}

void readResponse(char* content, size_t maxSize) {
  size_t length = client.readBytes(content, maxSize);
  content[length] = '\0';
  Serial.println(content);
}

void parseUserData(char* content, struct UserData userData) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.parseObject(content);

  strcpy(userData.weather, root["weather"][0]["main"]);
  strcpy(userData.temp, root["main"]["temp"]);
  strcpy(userData.tempMin, root["main"]["temp_min"]);
  strcpy(userData.tempMax, root["main"]["temp_max"]);
  strcpy(userData.humidity, root["main"]["humidity"]);

  Serial.print("Weather: ");
  Serial.println(userData.weather);
  Serial.print("Temp: ");
  Serial.println(userData.temp);
  Serial.print("Temp_Min: ");
  Serial.println(userData.tempMin);
  Serial.print("Temp_Max: ");
  Serial.println(userData.tempMax);
  Serial.print("Humidity: ");
  Serial.println(userData.humidity);
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while(!WiFi.status() == WL_CONNECTED)
  {
      delay(500);
      Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi Connected!");
}

void loop() {
  unsigned long currentTime = millis();
  if( currentTime - previousTime >= 10000)
  {
      /* Update per 5 minutes*/
      while(!client.connect(host, 80))
      {
          Serial.println("Failed to connecting to host!");
          return;
      }
    
      if( sendRequest(api_key, host, city_id) && skipHeaders() )
      {
          /* Read response from server*/
          char content[MAX_CONTENT_SIZE];
          readResponse(content, sizeof(content));
          UserData userData;
          parseUserData(content, userData);
      }

      previousTime = currentTime;
  }
}
