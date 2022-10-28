#pragma once

// https://github.com/mudassar-tamboli/ESP32-OV7670-WebSocket-Camera
// https://github.com/Links2004/arduinoWebSockets

#include "ESP32CAM.h"

const framesize_t FRAME_SIZE = FRAMESIZE_VGA;
//const pixformat_t PIXEL_FORMAT = PIXFORMAT_JPEG;
const int IMG_WIDTH = 640
  , IMG_HEIGHT = 480;
const String cam_mode_name = "VGA";

class cCamera{
private:
  ESP32CAM camera;

public:
  cCamera():camera(){}

  bool init(){
    Serial.println("init()");
    if (camera.init() != ESP_OK){
      Serial.println("camera.init() failed");
      return false;
    }
    Serial.println("init() pass");
    sensor_t *s = camera.sensor();
    s->set_framesize(s, FRAME_SIZE);
    Serial.println("camera.sensor() pass");
    //s->set_pixformat(s, PIXEL_FORMAT);
    return true;
  }

  // post image to server
  bool post(cWifi &wifi){
    db_println(F("post image"));

    auto &client = wifi.get_client();

    /*
    // get api key
    char api_key[MAX_API_KEY_LEN+1];
    if( !get_api_key(api_key) ){
      Serial.println(F("Cannot get api key"));
      return true;
    }
    */

    // get image
    camera_fb_t *fb = camera.capture();
    if (!fb){
      Serial.println(F("Camera capture failed"));
      return false;
    }

    // send to server
    char contentType[100];
    char boundary[32] = "--";
    const int fileSize = fb->len;

    //boundary
    randomSeed(micros());
    for (int i = 0; i < 3; i++) {
      ltoa(random(0x7FFFFFF), boundary + strlen(boundary), 16);
    }
    strcpy(contentType, "multipart/form-data; boundary=");
    strcat(contentType, boundary);

    String payloadHeaderFormat = ""
                     "--%s\r\n"
                     "Content-Disposition: form-data; name=\"image\"; filename=\"%s.%s.jpg\"\r\n"
                     "Content-Type: application/octet-stream\r\n"
                     "Content-Transfer-Encoding: binary\r\n"
                     "\r\n"
                     ;
    char payloadHeader[200] = {0};
    sprintf(payloadHeader,
                payloadHeaderFormat.c_str(),
                boundary,
                cam_mode_name.c_str()); // add api key to image file name to identify the camera
    /*
    sprintf(payloadHeader,
                payloadHeaderFormat.c_str(),
                boundary,
                cam_mode_name.c_str(),
                api_key); // add api key to image file name to identify the camera
    */
    
    char payloadFooter[50] = {0};
    sprintf(payloadFooter, "\r\n--%s--\r\n", boundary);

    db_println("--- request --- ");

    String bodyHeader = String(payloadHeader);
    String bodyFooter = String(payloadFooter);
    int contentLength = bodyHeader.length() + fileSize + bodyFooter.length();

    db_print(F("contentLength "));    db_println(contentLength);
    db_print(F("bodyHeaderLength ")); db_println(bodyHeader.length());
    db_print(F("bodyFooterLength ")); db_println(bodyFooter.length());
    db_print(F("fileSize "));         db_println(fileSize);

    bool b_ret = false;
    if (client.connect(server.c_str(),WIFI_SECURE ? 443 : port)){
      client.printf("POST /api/add_image HTTP/1.1\n");
      client.print(F("Host: "));client.println(server);
      client.println(F("Accept: application/json"));
      client.println(F("Connection: close"));
      client.print(F("Content-Type: "));client.println(contentType);
      client.print(F("Content-Length: "));client.println(contentLength);
      client.println();
      client.print(bodyHeader.c_str());
      client.flush();

      db_println(F("write ---"));
      client.write(fb->buf, fb->len);
      client.flush();
      db_println(F("--- write"));
      camera.clearMemory(fb);

      client.print(bodyFooter.c_str());
      //client.flush();

      db_println(F("response ---"));
      unsigned long t = millis();
      while (client.available() == 0) {
        if((unsigned long)(millis()-t) > 30000 ) {
          Serial.println(F("Client Timeout !"));
          client.stop();
          return false;
        }
      }
      db_println(F("--- response"));
      String line = "";
      while(client.available()){
        line = client.readStringUntil('\r');
        db_print(line);
        if( line.indexOf("{\"result\":\"success\"}")>=0 ){
          b_ret = true;
        }
      }
      client.stop();
    }
    else{
      Serial.println(F("Cannot connect sever"));
    }
    db_println("\nend post");
    return b_ret;
  }
};
