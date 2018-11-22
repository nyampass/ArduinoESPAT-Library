#include "ArduinoESPAT.h"

ESPAT espat("SSID", "PASSWD"); 

void setup(){
  Serial.begin(115200);

  if(espat.begin()){
    Serial.println("Initialize Ok");
  }else{
    Serial.println("Initialize Fail");
  }

  if(espat.changeMode(1)){
    Serial.println("Mode OK");
  }else{
    Serial.println("Mode not OK");
  }

  if(espat.tryConnectAP()){
    Serial.println("Connected");
  }else{
    Serial.println("Connect Failed");
  }

  Serial.println(espat.clientIP());

  Serial.println(espat.get("hogehoge.com", "/", 80));
  Serial.println("Finished");
}

void loop(){
}