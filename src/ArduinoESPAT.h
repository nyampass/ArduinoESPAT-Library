#ifndef ArduinoESPAT_h
#define ArduinoESPAT_h

#include <Arduino.h>
#include <SoftwareSerial.h>

#define NOIP "0.0.0.0"
#define EVENTS_AMOUNT 10

class SoftwareSerial;
class ESPAT{
  public:
    ESPAT(String ssid, String pass);
    bool begin();
    bool checkAT();
    bool changeMode(uint8_t mode);
    bool tryConnectAP();
    bool get(String host, String path, int port = 80, void (*ptf)(char) = nullptr);
    bool openServer(int port, void (*opened)() = nullptr);
    String clientIP();
    String sendComm(String comm, int wait = 2000);
    void setGetRecieveEvents(String path, String html, void (*access)());

  private:
    String SSID;
    String PASS;
    bool INIT = false;
    bool SERVER = false;
    bool analysisUri(String *buff, String uri);
    bool checkStrByOk(String s);
    bool waitResp(uint8_t limit);
    void atdelay(int limit);
    int s2i(String str);

    uint8_t GetRecieveEventsNext = 0;
    struct GetRecieveEvent{
      void (*access)();
      String path;
      String html;
    };
    struct GetRecieveEvent GetRecieveEvents[EVENTS_AMOUNT];

    SoftwareSerial *ss = new SoftwareSerial(2, 3);
};

#endif
