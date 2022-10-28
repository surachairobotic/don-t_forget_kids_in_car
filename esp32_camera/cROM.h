
#include "EEPROM.h"

const unsigned int EEPROM_SIZE = 20;

class cROM{
public:
  cROM(){}

  void init(){
    if (!EEPROM.begin(EEPROM_SIZE)){
      Serial.println(F("init EEPROM failed"));
      while(1);
    }
  }

  void write(const unsigned int addr, const void *buf, const unsigned int len) {
    if ( addr + len >= EEPROM_SIZE ) {
      Serial.print(F("rom_write() : invalid addr "));
      Serial.print(addr);
      Serial.print(',');
      Serial.print(len);
      Serial.print('/');
      Serial.println(EEPROM_SIZE);
      return;
    }
    db_print(F("rom write : addr = "));
    db_print(addr);
    db_print(F(", len = "));
    db_println(len);
    for (int i = len - 1; i >= 0; i--) {
      EEPROM.write(addr + i, ((char*)buf)[i]);
    }
    EEPROM.commit();
  }

  void read(unsigned int addr, void *buf, int len) {
/*    db_print("rom read : addr = ");
    db_print(addr);
    db_print(", len = ");
    db_println(len);*/
    for (int i = len - 1; i >= 0; i--) {
      ((char*)buf)[i] = EEPROM.read(addr + i);
    }
  }

  void reset(unsigned int addr_start, unsigned int addr_end){
    if ( addr_start >= EEPROM_SIZE || addr_end >= EEPROM_SIZE ) {
      Serial.print(F("rom.reset() : invalid addr "));
      Serial.print(addr_start);
      Serial.print(',');
      Serial.println(addr_end);
      return;
    }
    for(unsigned int i = addr_start ; i < addr_end; i++) {
      EEPROM.write(i, 0);
    }
    EEPROM.commit();
  }

  bool check_only_alphabet_number(const char *str){
    int n = strlen(str);
    for(int i=0;i<n;i++){
      char c = str[i];
      if( !(c>='A' && c<='Z') && !(c>='a' && c<='z') && !(c>='0' && c<='9') ){
        db_print(F("check_only_alphabet_number() failed : "));
        db_println(str);
        return false;
      }
    }
    return true;
  }

  bool check_only_number(const char *str){
    int n = strlen(str);
    for(int i=0;i<n;i++){
      char c = str[i];
      if( !(c>='0' && c<='9') ){
        db_print(F("check_only_number() failed : "));
        db_println(str);
        return false;
      }
    }
    return true;
  }


  bool read_string(char *str, const unsigned int addr, const unsigned int addr_checksum
      , const unsigned int max_len
      , bool b_check_only_alphabet_number, bool b_check_only_number, bool b_silent = false){
    this->read( addr, str, max_len + 1 );
    int i;
    for(i=0;i<=max_len;i++){
      char c = str[i];
      if( c==0 )
        break;
/*      else if( !(c>='A' && c<='Z') && !(c>='a' && c<='z') && !(c>='0' && c<='9') ){
        db_print(F("invalid api key char : "));
        db_println(c);
        api[0] = 0;
        return false;
      }*/
    }
    if( i>max_len ){
      if( !b_silent )
        db_println(F("invalid string len"));
      return false;
    }

    {
      bool ret = true;
      if( b_check_only_alphabet_number )
        ret = check_only_alphabet_number(str);
      if( b_check_only_number )
        ret = check_only_number(str);
      if( !ret ){
        if( !b_silent ){
          Serial.print(F("invalid string : "));
          Serial.println(str);
        }
        return false;
      }
    }

    uint16_t cs;
    this->read( addr_checksum, &cs, 2 );

    if( checksum(str, strlen(str))!=cs ){
      if( !b_silent )
        db_println(F("invalid string checksum"));
      return false;
    }
    return true;
  }

  uint16_t checksum( const char *str, unsigned int len ){
    uint16_t cs = 21251;
    for(int i=0;i<len;i++){
      cs+= (uint16_t)str[i]*i+i+7;
    }
    return cs;
  }


  bool write_string(const char *str, const unsigned int addr, const unsigned int addr_checksum
      , const unsigned int max_len
      , bool b_check_only_alphabet_number, bool b_check_only_number){
    int n = strlen(str);
    if( n>max_len ){
      Serial.print(F("Too long string in ROM : "));
      Serial.println(str);
      return false;
    }
    if( b_check_only_alphabet_number && !check_only_alphabet_number(str) ){
      Serial.println("invalid string in ROM");
      return false;
    }
    if( b_check_only_number && !check_only_number(str) ){
      Serial.println("invalid string in ROM");
      return false;
    }

    this->write( addr, str, n + 1 );
    uint16_t cs = checksum( str, n );
    this->write( addr_checksum, &cs, 2 );
//    Serial.printf("write : %s, %d\n", str, cs);
    return true;
  }
};
