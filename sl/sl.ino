#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

const char* ssid = "{Your SSID}";          
const char* password = "{Your Password}";         
const char* key = "{API KEY}";      
const char* location = "{City Name}";
unsigned long previousTime = 0;
const char* host = "api.seniverse.com";
size_t MAX_CONTENT_SIZE = 1024;
WiFiClient client;

struct UserData  {
  char cityName[16];
  char weather[25];
  char temperature[16];
  
};

bool sendRequest(const char* key, const char* host, const char* location) {
  //https://api.seniverse.com/v3/weather/now.json?key=SvapyYFkj7EvAAqfG&location=beijing&language=zh-Hans&unit=c
  //api.openweathermap.org/data/2.5/weather?id={city id}&appid={your api key}
  String getUrl = "/v3/weather/now.json?key=";
  getUrl += key;
  getUrl += "&language=en&location=";
  getUrl += location;

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

  strcpy(userData.cityName, root["results"][0]["location"]["name"]);
  strcpy(userData.weather, root["results"][0]["now"]["text"]);
  strcpy(userData.temperature, root["results"][0]["now"]["temperature"]);

  Serial.print("CityName: ");
  Serial.println(userData.cityName);
  Serial.print("Weather: ");
  Serial.println(userData.weather);
  Serial.print("Temperature: ");
  Serial.println(userData.temperature);
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

  delay(500);
  
  while(!client.connect(host, 80))
  {
      Serial.println("Failed to connecting to host!");
      return;
  }

  if( sendRequest(key, host, location) && skipHeaders() )
  {
      /* Read response from server*/
      char content[MAX_CONTENT_SIZE];
      readResponse(content, sizeof(content)); 
      UserData userData;
      parseUserData(content, userData);
  }

  previousTime = millis();
}

void loop() {
  unsigned long currentTime = millis();
  if( currentTime - previousTime >= 5*60000)
  {
      /* Update per 5 minutes*/
      while(!client.connect(host, 80))
      {
          Serial.println("Failed to connecting to host!");
          return;
      }
    
      if( sendRequest(key, host, location) && skipHeaders() )
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
