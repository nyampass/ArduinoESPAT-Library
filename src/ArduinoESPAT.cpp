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

bool ESPAT::waitResp(uint8_t limit){
  uint8_t cnt = 0;

  while(true){
    atdelay(3000);
    String data = ss->readString();

    // Serial.println(data);

    if(cnt > limit){break;}

    if(checkStrByOk(data)){
      return true;
      break;
    }else if(data.indexOf("ERROR") >= 0){
      return false;

      break;
    }
    cnt += 1;
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

    int limit = 10;

    while(true){
      atdelay(3000);
      String data = ss->readString();

      if(checkStrByOk(data) || data.indexOf("ERROR") >= 0){
        break;
      }
    }
    if(clientIP() != ""){
      return true;
    }else{
      return false;
    }
  }else{
    return false;
  }
}

bool ESPAT::get(String host, String path, int port = 80, void (*ptf)(char) = nullptr){ // host path port callback || host path callback
  if(!INIT) return "";

  if(clientIP() != ""){
    ss->println("AT+CIPSTART=\"TCP\",\""+ host +"\"," + String(port));
    // atdelay(4000);
    if(waitResp(5)){
      // Serial.println("TCP OK");
      ss->println("AT+CIPMODE=1");
      // atdelay(500);
      if(waitResp(5)){
        // Serial.println("MODE OK");
        ss->println("AT+CIPSEND");
        // atdelay(1000);
        if(waitResp(5)){
          // Serial.println("SEND READY");
          ss->println("GET "+ path +" HTTP/1.0\nHOST:"+ host +"\n");
          atdelay(4000);
          while(ss->available()){
            char readData = (char)ss->read();

            if(ptf == nullptr){
              Serial.print(readData);
            }else{
              ptf(readData);
            }
            // Serial.print(readData);
            atdelay(3000);
          }
          ss->print("+++");
          atdelay(1000);
          ss->readString(); // reset buff
          ss->println("AT+CIPCLOSE");
          atdelay(1000);
          ss->readString(); // reset buff
          // Serial.println(ss->readString());
          return true;
        }else{
          return false;
        }
      }else{
        return false;
      }
    }else{
      return false;
    }
  }else{
    return false;
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
