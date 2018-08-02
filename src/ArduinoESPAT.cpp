#include "ArduinoESPAT.h"

ESPAT::ESPAT(String ssid, String pass){
  SSID = ssid;
  PASS = pass;
}

void ESPAT::atdelay(int limit){
  int cnt = 0;

  while(!ss->available() && cnt < limit){
    cnt += 1;
    delay(1);
  }
}

bool ESPAT::begin(){
  ss->begin(115200);
  // Serial.begin(115200);
  ss->println("AT+RST");
  atdelay(3000);
  ss->readString();
  ss->println("AT+UART_CUR=9600,8,1,0,0");
  atdelay(1000);
  ss->readString();
  ss->println("AT+CWDHCP_DEF=1,1");
  atdelay(1000);
  ss->readString();

  ss->begin(9600);
  atdelay(10);
  if(checkAT()){
    INIT = true;
  }
  return checkAT();
}

bool ESPAT::checkAT(){
  ss->println("AT");
  atdelay(500);
  return checkStrByOk(ss->readString());
}

bool ESPAT::changeMode(uint8_t mode){
  if(!INIT) return false;
  ss->println("AT+CWMODE_CUR=" + String(mode));
  atdelay(2000);
  return checkStrByOk(ss->readString());
}

bool ESPAT::tryConnectAP(){
  if(!INIT) return false;
  if(true){ // clientIP() == ""
    ss->println("AT+CWJAP_CUR=\"" + SSID + "\",\"" + PASS + "\"");
    
    atdelay(10000);
    ss->readString(); // reset buff
    // Serial.println(ss->readString());
    if(clientIP() != ""){
      return true;
    }else{
      return false;
    }
  }else{
    return false;
  }
}

String ESPAT::get(String uri){
  if(!INIT) return "";
  String result = "";
  String buff[2] = {};

  analysisUri(buff, uri);
  String host = buff[0];
  String url = buff[1];

  if(clientIP() != "no IP"){
    ss->println("AT+CIPSTART=\"TCP\",\""+ host +"\",80");
    atdelay(4000);
    if(checkStrByOk(ss->readString())){
      // Serial.println("TCP OK");
      ss->println("AT+CIPMODE=1");
      atdelay(500);
      if(checkStrByOk(ss->readString())){
        // Serial.println("MODE OK");
        ss->println("AT+CIPSEND");
        atdelay(1000);
        if(checkStrByOk(ss->readString())){
          // Serial.println("SEND READY");
          ss->println("GET "+ url +" HTTP/1.1\nHOST:"+ host +"\n");
          atdelay(4000);
          while(ss->available()){
            result += (char)ss->read();
          }
          // Serial.println(result);
          ss->print("+++");
          atdelay(1000);
          ss->readString(); // reset buff
          ss->println("AT+CIPCLOSE");
          atdelay(1000);
          ss->readString(); // reset buff
          // Serial.println(ss->readString());
          return result;
        }else{
          return result;
        }
      }else{
        return result;
      }
    }else{
      return result;
    }
  }else{
    return result;
  }
}

String ESPAT::clientIP(){
  if(!INIT) return "";
  String resp = "";
  String result = "";
  int pos = 0;

  ss->println("AT+CIFSR");
  atdelay(2000);
  resp = ss->readString();
  pos = resp.indexOf("+CIFSR:STAIP,\"");

  if(pos >= 0){
    char c = ' ';
    uint8_t cnt = 0;

    while(true){
      c = resp[cnt + pos + 14];

      if(c == '\"') break;
      result += c;
      cnt += 1;
    }
    return result;
  }else{
    return "";
  }
}

bool ESPAT::checkStrByOk(String s){
  if(s.indexOf("OK") >= 0){
    return true;
  }else{
    return false;
  }
}

String ESPAT::sendComm(String comm, int wait = 2000){
  if(!INIT) return "";
  ss->println(comm);
  atdelay(wait);
  return ss->readString();
}

bool ESPAT::analysisUri(String *buff, String uri){
  String host = "";
  String url = "";

  for(int i = 0; i < uri.length(); i++){
    if(uri[i] == '/'){
      break;
    }else{
      host += uri[i];
    }
  }
  if(host.length() == uri.length()){
    return false;
  }
  url = uri.substring(host.length());
  *(buff) = host;
  *(buff + 1) = url;

  return true;
}
