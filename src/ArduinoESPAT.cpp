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
    atdelay(500);
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
  Serial.println(String(mode));
  ss->println("AT+CWMODE_CUR=" + String(mode));
  atdelay(2000);
  return checkStrByOk(ss->readString());
}

bool ESPAT::tryConnectAP(){
  changeMode(1);
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

bool ESPAT::openServer(int port, void (*opened)()){
  String line = "";

  if(!SERVER){
    if(clientIP() == NOIP){
      if(!tryConnectAP()){
        return false;
      }
    }
    ss->println("AT+CIPMUX=1");
    if(waitResp(5)){
      ss->println("AT+CIPSERVER=1," + String(port));
      if(waitResp(5)){
        atdelay(1000);
        SERVER = true;
        if(opened != nullptr) opened();
        while(SERVER){
          while(ss->available()){
            char c = ss->read();

            line += c;
            if(c == '\n'){
              Serial.println(line);
              if(line.indexOf("GET") >= 0 && line.indexOf("+IPD,") == 0){
                String path = "";
                String response = "";
                String html = "";
                int8_t id = 0;

                path = line.substring(line.indexOf("GET") + 4);
                path = path.substring(0, path.indexOf("HTTP/1") - 1);
                id = s2i(line.substring(5, 6));

                Serial.println(path);
                Serial.println(id);

                for(int i = 0;  i < GetRecieveEventsNext; i++){
                  if(GetRecieveEvents[i].path == path){
                    GetRecieveEvents[i].access();
                    html = GetRecieveEvents[i].html;
                  }
                }

                response = "HTTP/1.0 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nKeep-Alive: timeout=15, max=100\r\n\r\n" + html;
                ss->println("AT+CIPSEND=" + String(id) + "," + response.length());
                // waitResp(3);
                delay(500);
                ss->println(response);
                // waitResp(5);
                delay(500);
                ss->println("AT+CIPCLOSE=" + String(id));
              }
              line = "";
            }
            atdelay(100000);
          }
        }
      }else{
        return false;
      }
    }else{
      return false;
    }
  }
}

void ESPAT::setGetRecieveEvents(String path, String html, void (*access)()){
  struct GetRecieveEvent event;

  event.access = access;
  event.path = path;
  event.html = html;
  GetRecieveEvents[GetRecieveEventsNext] = event;
  GetRecieveEventsNext += 1;
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

int ESPAT::s2i(String str){
  if(str == ""){
    return -1;
  }
  for(int i = 0; i < str.length(); i++){
    if(String((int)(str[i] - '0')) != String(str[i])){
      return -1;
    }
  }
  return str.toInt();
}
