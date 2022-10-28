#define _DB_          1     // enable debug info
#define WIFI_SECURE   0     // enable SSL

#if _DB_
#define db_println(x) Serial.println(x);
#define db_print(x) Serial.print(x);
#else
#define db_println(x)
#define db_print(x)
#endif

#include "Arduino.h"
#include "esp_camera.h"
#include <WiFi.h>

//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM

#include "camera_pins.h"

const char* ssid = "OnlyFans";
const char* password = "m1111111";
const char* device_name = "CAM3";
int numRestart = 10;

typedef struct {
  size_t size; //number of values used for filtering
  size_t index; //current value index
  size_t count; //value count
  int sum;
  int * values; //array to be filled with values
} ra_filter_t;

static ra_filter_t ra_filter;

void startCameraServer();

static ra_filter_t * ra_filter_init(ra_filter_t * filter, size_t sample_size) {
  memset(filter, 0, sizeof(ra_filter_t));

  filter->values = (int *)malloc(sample_size * sizeof(int));
  if (!filter->values) {
    return NULL;
  }
  memset(filter->values, 0, sample_size * sizeof(int));

  filter->size = sample_size;
  return filter;
}

void setup() {
  Serial.begin(115200);
  Serial.println(device_name);
  delay(1000);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  config.frame_size = FRAMESIZE_UXGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;


#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  //s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if(numRestart == 0) {
      ESP.restart();
    }
    numRestart--;
  }
  Serial.println("");
  Serial.println("WiFi connected");

  Serial.print("IP Address : ");
  Serial.println(WiFi.localIP());
}

void loop() {
  capture_and_post();
  // put your main code here, to run repeatedly:
  delay(5000);
}

bool capture_and_post() {
  camera_fb_t * fb = NULL;
  esp_err_t res = ESP_OK;
  int64_t fr_start = esp_timer_get_time();

  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
    return ESP_FAIL;
  }
  else
    Serial.println("Camera capture pass.");

  if (fb->width >= 320) {
    Serial.println("fb->width > 320 : true");
    if (fb->format == PIXFORMAT_JPEG) {
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
                                   "Content-Disposition: form-data; name=\"image\"; filename=\"%s.jpg\"\r\n"
                                   "Content-Type: application/octet-stream\r\n"
                                   "Content-Transfer-Encoding: binary\r\n"
                                   "\r\n"
                                   ;
      char payloadHeader[200] = {0};
      sprintf(payloadHeader,
              payloadHeaderFormat.c_str(),
              boundary,
              device_name);

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

      WiFiClient client;
      String server = "192.168.1.147";
      int port = 8088;
      if (client.connect(server.c_str(), port)) {
        Serial.println(F("Connected sever\n"));

        client.printf("POST /api/add_image HTTP/1.1\n");
        client.print(F("Host: ")); client.println(server);
        client.println(F("Accept: application/json"));
        client.println(F("Connection: close"));
        client.print(F("Content-Type: ")); client.println(contentType);
        client.print(F("Content-Length: ")); client.println(contentLength);
        client.println();
        client.print(bodyHeader.c_str());
        client.flush();

        db_println(F("write ---\n"));
        client.write(fb->buf, fb->len);
        client.flush();
        db_println(F("--- write\n"));

        esp_camera_fb_return(fb);

        client.print(bodyFooter.c_str());
        //client.flush();

        db_println(F("response ---\n"));
        unsigned long t = millis();
        while (client.available() == 0) {
          if ((unsigned long)(millis() - t) > 30000 ) {
            Serial.println(F("Client Timeout !\n"));
            client.stop();
            ESP.restart();
            return false;
          }
        }
        db_println(F("--- response\n"));

        String line = "";
        while (client.available()) {
          line = client.readStringUntil('\r');
          db_print(line);
          if ( line.indexOf("{\"result\":\"success\"}") >= 0 ) {
            db_println(F("result:success\n"));
          }
          else {
            db_println(F("result: not success\n"));
          }
        }
        client.stop();
      }
      else {
        Serial.println(F("Cannot connect sever\n"));
        ESP.restart();
      }
    }
    else {
      Serial.print("fb->width : ");
      Serial.println(fb->width);
      ESP.restart();
    }
  }
  else {
    Serial.print(F("fb->width > 320 : false : size is "));
    Serial.println(fb->width);
    ESP.restart();
  }
}
