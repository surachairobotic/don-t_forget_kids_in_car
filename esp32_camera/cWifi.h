#pragma once

#define DEBUG_WEBSOCKETS
#define DEBUG_ESP_PORT Serial

//#include <WebSockets.h>
//#include <WebSocketsClient.h>
#include <WiFi.h>
//#include <WiFiMulti.h>
#include <WiFiClient.h>
#include "time.h"
#include <esp_wifi.h>

const String ssid = "OnlyFans"
  , password = "m1111111";
const String server = "192.168.1.147";
const int port = 8088;

const String ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7*3600;
const int   daylightOffset_sec = 0;

//void webSocketEvent(WStype_t type, uint8_t * payload, size_t payloadlength);

class cWifi{
private:
  WiFiClient client;
protected:
  bool b_connect;

public:
  cWifi():b_connect(false){}

  void start(){
    //wifiMulti.addAP(ssid.c_str(), password.c_str());
    WiFi.mode(WIFI_MODE_STA);
    Serial.print(F("MAC:"));
    Serial.println(WiFi.macAddress());
    Serial.println(F("Connecting"));

//    while(wifiMulti.run() != WL_CONNECTED){
    while(1){
      Serial.println(F("Connecting"));
      WiFi.mode(WIFI_STA);
      esp_wifi_set_ps(WIFI_PS_NONE);
      WiFi.begin(ssid.c_str(), password.c_str());
      bool b_ok = true;
      unsigned long t = millis();
      while (WiFi.status() != WL_CONNECTED) {
        mysleep(1000);
        Serial.print(F("WiFi status:"));
        Serial.println(WiFi.status());
        if( (unsigned long)(millis()-t) > 10000 ){
          b_ok = false;
          break;
        }
      }
      if( b_ok )
        break;
      else{
        WiFi.disconnect(true);
        delay(1000);
      }
    }

    Serial.print(F("WiFi connected : "));
    b_connect = true;
    Serial.println(WiFi.localIP());
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer.c_str());
  }

  void stop(){
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  }

  // get real time from NTP server
  bool get_time(struct tm &timeinfo, const uint32_t timeout_ms=0){
    uint32_t t = millis();
    while(!getLocalTime(&timeinfo)){
      if( timeout_ms>0 && uint32_t(millis() - t) >= timeout_ms ){
        Serial.println("get_time(): timeout");
        return false;
      }
      mysleep(200);
    }
    Serial.printf("time : %d:%d\n", timeinfo.tm_hour, timeinfo.tm_min);
    return true;
  }

  // sleep until the next image will be sent
  void sleep_until(const uint8_t hour, const uint8_t min, bool b_image_sent){
    struct tm timeinfo;
    get_time(timeinfo);
    unsigned long t1 = timeinfo.tm_hour * 60 + timeinfo.tm_min
      , t2 = (unsigned long)hour * 60 + min
      , dt;
    if( t2 < t1 ){
      t2+= 24*60;
    }
    dt = t2 - t1;
    // sleep until the next day if an image was sent
    if( b_image_sent && dt < 180 ){
      dt+= 24 * 60;
    }
    // time to sleep in sec
    unsigned long t = dt*60;
    // add some random time
    randomSeed( t1*60 + timeinfo.tm_sec );
    t+= random(1200);

    esp_sleep_enable_timer_wakeup( (uint64_t)t * 1000000 );

    unsigned long h = t / 3600, m = (t-h*3600)/60;
    Serial.printf("Sleep : %ld h %ld m\n", h, m);
    delay(500);
    esp_deep_sleep_start();
  }

  // sleep for some minutes
  void sleep_min(const uint32_t min){
    esp_sleep_enable_timer_wakeup( uint64_t(min)
      *60 * 1000000 );
    Serial.printf("Sleep for %d min\n", min);
    delay(500);
    esp_deep_sleep_start();
  }

  virtual WiFiClient &get_client(){ return client; };

};
