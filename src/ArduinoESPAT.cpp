#include "ArduinoESPAT.h"

ESPAT::ESPAT(String ssid, String pass){
  SSID = ssid;
  PASS = pass;
}

void ESPAT::atdelay(int limit){
  int cnt = 0;

  ss->listen();

  while(!ss->available() && cnt < limit){
    cnt += 1;
    delay(1);
  }
}

bool ESPAT::waitResp(uint8_t limit){
  uint8_t cnt = 0;

  ss->listen();

  while(true){
    atdelay(500);
    String data = ss->readString();

    Serial.println(data);

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
  ss->listen();

  ss->begin(115200);
  // Serial.begin(115200);
  ss->println("AT+RESTORE");
  delay(2000);
  ss->readString();
  ss->println("AT+UART_CUR=9600,8,1,0,0");
  atdelay(1000);
  ss->readString();
  ss->println("AT+CWDHCP_DEF=1,1");
  atdelay(1000);
  ss->readString();
  ss->begin(9600);
  delay(500);
  INIT = checkAT();
  changeMode(1);
  ss->readString();
  return INIT;
}

bool ESPAT::checkAT(){
  ss->listen();

  ss->println("AT");
  atdelay(500);
  return checkStrByOk(ss->readString());
}

bool ESPAT::changeMode(uint8_t mode){
  ss->listen();

  if(!INIT) return false;
  ss->println("AT+CWMODE_CUR=" + String(mode));

  return waitResp(5);
}

bool ESPAT::tryConnectAP(){
  ss->listen();

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

String ESPAT::get(String host, String path, int port = 80){
  ss->listen();
  String result;
  uint16_t cnt = 0;
  bool flag = false;
  bool skipCheck = false;

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
          while(ss->available() && cnt < RESP_BUFF){
            char readData = (char)ss->read();

            if(skipCheck){
              // Serial.print(readData);
              result += readData;
              cnt += 1;
            }else{
              if(readData == '\n'){
                if(!flag){
                  flag = true;
                }
              }else if(readData == '\r' && flag){
                skipCheck = true;
              }else{
                flag = false;
              }
            }
            atdelay(3000);
          }
          ss->print("+++");
          atdelay(1000);
          ss->readString(); // reset buff
          ss->println("AT+CIPCLOSE");
          atdelay(1000);
          ss->readString(); // reset buff
          // Serial.println(ss->readString());
          return result;
        }else{
          return "";
        }
      }else{
        return "";
      }
    }else{
      return "";
    }
  }else{
    return "";
  }
}

bool ESPAT::advGet(String host, String path, int port = 80, void (*ptf)(char) = nullptr){ // host path port callback || host path callback
  ss->listen();

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
  ss->listen();

  if(!INIT) return "";
  String resp = "";
  String result = "";
  int pos = 0;

  ss->println("AT+CIFSR");
  atdelay(2000);
  while(ss->available()){
    resp += char(ss->read());
    atdelay(2000);
  }
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
  ss->listen();

  String line = "";

  if(!SERVER && INIT){
    if(clientIP() == NOIP){
      changeMode(1);
      waitResp(5);
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
        ss->listen();
        while(SERVER){
          while(ss->available()){
            char c = ss->read();

            if(SERVER_FORCE_SHUT){
              SERVER = false;
              SERVER_FORCE_SHUT = false;
              return true;
            }

            line += c;
            if(c == '\n'){
              // Serial.println(line);
              if(line.indexOf("GET") >= 0 && line.indexOf("+IPD,") == 0){
                String path = "";
                String queryStr = "";
                String query[2] = {"", ""};
                String response = "";
                String html = "404";
                int8_t id = 0;
                void (*callback)(String, String) = nullptr;

                path = line.substring(line.indexOf("GET") + 4);
                path = path.substring(0, path.indexOf("HTTP/1") - 1);
                id = s2i(line.substring(5, 6));

                if(path.indexOf("?") >= 0){
                  uint8_t queryStart = path.indexOf("?");
                  queryStr = path.substring(queryStart + 1);
                  path = path.substring(0, queryStart);

                  if(queryStr.indexOf("=") >= 0){
                    uint8_t equal = queryStr.indexOf("=");

                    query[0] = queryStr.substring(0, equal);
                    query[1] = queryStr.substring(equal + 1);
                  }
                }

                Serial.println(path);
                Serial.println(id);
                Serial.println(query[0]);
                Serial.println(query[1]);

                for(int i = 0;  i < GetRecieveEventsNext; i++){
                  if(GetRecieveEvents[i].path == path){
                    callback = GetRecieveEvents[i].access;
                    html = GetRecieveEvents[i].html;
                    break;
                  }
                }

                response = "HTTP/1.0 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nKeep-Alive: timeout=15, max=100\r\n\r\n" + html;
                ss->listen();
                ss->println("AT+CIPSEND=" + String(id) + "," + response.length());
                delay(500);
                ss->println(response);
                delay(500);
                ss->println("AT+CIPCLOSE=" + String(id));

                if(callback != nullptr) callback(query[0], query[1]);
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

void ESPAT::breakServer(){
  ss->listen();

  if(SERVER){
    SERVER_FORCE_SHUT = true;
  }
}

void ESPAT::setGetRecieveEvents(String path, String html, void (*access)()){
  struct GetRecieveEvent event;

  ss->listen();

  if(GetRecieveEventsNext < GET_RECV_EVENTS_LIMIT){
    event.access = access;
    event.path = path;
    event.html = html;
    GetRecieveEvents[GetRecieveEventsNext] = event;
    GetRecieveEventsNext += 1;
  }
}

bool ESPAT::checkStrByOk(String s){
  ss->listen();

  if(s.indexOf("OK") >= 0){
    return true;
  }else{
    return false;
  }
}

String ESPAT::sendComm(String comm, int wait = 2000){
  ss->listen();

  if(!INIT) return "";
  ss->println(comm);
  atdelay(wait);
  return ss->readString();
}

bool ESPAT::analysisUri(String *buff, String uri){
  String host = "";
  String url = "";

  ss->listen();

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
  ss->listen();

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
