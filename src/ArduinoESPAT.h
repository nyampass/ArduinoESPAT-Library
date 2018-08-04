#ifndef ArduinoESPAT_h
#define ArduinoESPAT_h

#include <Arduino.h>
#include <SoftwareSerial.h>

class SoftwareSerial;
class ESPAT{
  public:
    ESPAT(String ssid, String pass);
    bool begin();
    bool checkAT();
    bool changeMode(uint8_t mode);
    bool tryConnectAP();
    bool get(String host, String path, int port);
    String clientIP();
    String sendComm(String comm, int wait = 2000);

  private:
    String SSID;
    String PASS;
    bool INIT = false;
    bool analysisUri(String *buff, String uri);
    bool checkStrByOk(String s);
    bool waitResp(uint8_t limit);
    void atdelay(int limit);

    SoftwareSerial *ss = new SoftwareSerial(2, 3);
};

#endif
