#define _DB_          1     // enable debug info
#define WIFI_SECURE   0     // enable SSL

#if _DB_
  #define db_println(x) Serial.println(x);
  #define db_print(x) Serial.print(x);
#else
  #define db_println(x)
  #define db_print(x)
#endif
#include "cROM.h"

//////////////////////////

const int TIME_SEND_IMAGE_HOUR = 12 // send image to server at 12:00
  , TIME_SEND_IMAGE_MIN = 0;

const unsigned long TIME_RESEND_IMAGE_MIN = 10 // interval to resend image when error in minutes
  , SERIAL_READ_LINE_TIMEOUT = 50;

// EEPROM address to store api key & checksum
const int ADDR_API_KEY = 1
  , MAX_API_KEY_LEN = 12
  , ADDR_API_KEY_CHECKSUM = ADDR_API_KEY + MAX_API_KEY_LEN + 1;

cROM rom;
const int BUF_LEN = 64;
char serial_buf[BUF_LEN];
int n_buf = 0;

void translate_cmd(char *msg);
void send_image();
bool get_api_key(char *str, bool b_silent = false);
void fast_loop();
void mysleep(unsigned long t);

#if WIFI_SECURE
  #include "cWifiSecure.h"
  cWifiSecure wifi;
#else
  #include "cWifi.h"
  cWifi wifi;
#endif
#include "cCamera.h"
cCamera camera;

void setup() {
  Serial.begin(115200);
  Serial.println("Serial begin.");

  wifi.start();
  Serial.println("wifi.start().");

  while( !camera.init() ) {
    Serial.println("camera.init()");
    delay(1000);
  }
  Serial.println("camera pass)");
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  rom.init();

  /*
  // set API key via serial monitor if not found in EEPROM
  char api_key[MAX_API_KEY_LEN+1];
  if( !get_api_key(api_key, true) ){
    uint32_t t = millis();
    Serial.println("No API key found.");
    Serial.println("Please enter your API key by using command \"station [Your API key]\"");
    while(1){
      fast_loop();
      if( millis()-t > 500 ){
        if( get_api_key(api_key, true) )
          break;
        t = millis();
      }
    }
  }
  */

  // send image immediately when esp32 starts
  if( camera.post(wifi) ){
      // sleep until the next time if succeeded
      wifi.sleep_until(TIME_SEND_IMAGE_HOUR, TIME_SEND_IMAGE_MIN, true);
  }
  else{
    // sleep for some minutes before retry if failed
    wifi.sleep_min(TIME_RESEND_IMAGE_MIN);
  }
}

// loop function that does not take time but need to be called as frequently as possible
void fast_loop(){
  unsigned int t_line = millis();
  bool b_end_line = false;
  do {
    while (Serial.available()) {
      char c = Serial.read();
      if ( c == '\n' || c == '\r' ) {
        if ( n_buf > 0 ) {
          serial_buf[n_buf] = 0;
          n_buf = 0;
          translate_cmd(serial_buf);
        }
        b_end_line = true;
        break;
      }
      else {
        if ( n_buf >= BUF_LEN - 1 ) {
          serial_buf[BUF_LEN - 1] = 0;
          Serial.print(F("Too long msg : "));
          Serial.println(serial_buf);
          n_buf = 0;
          return;
        }
        serial_buf[n_buf++] = c;
      }
    }
  }
  while ( (unsigned int)(millis() - t_line) < SERIAL_READ_LINE_TIMEOUT && !b_end_line );
}

// never be called
void loop(){

}

// translate command from serial
void translate_cmd(char *msg) {
  Serial.print(F("CMD : "));
  Serial.println(msg);
  if ( msg[0] == 0 )
    return;
  // set API key
  if( memcmp( "station ", msg, 8) == 0 ) {
    char *str = msg + 8;
    if( strlen(str)!=MAX_API_KEY_LEN ){
      Serial.println(F("Invalid api key len"));
      return;
    }
    rom.write_string( str, ADDR_API_KEY, ADDR_API_KEY_CHECKSUM, MAX_API_KEY_LEN, true, false );

  }
  else {
    Serial.println("invalid cmd");
  }
}

// read api key from EEPROM
bool get_api_key(char *str, bool b_silent){
  return rom.read_string( str, ADDR_API_KEY, ADDR_API_KEY_CHECKSUM, MAX_API_KEY_LEN
    , true, false, b_silent );
}

// not sleep but do other jobs
void mysleep(unsigned long t_sleep){
  unsigned long t = millis();
  do{
    fast_loop();
    delay(1);
  }
  while( (unsigned long)(millis()-t) < t_sleep );
}
