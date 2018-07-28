# ArduinoESPAT
A library to control esp-8266 from Arduino by AT commands easie.

# Usage
## Definition
You need SSID and password for your Access Point.
```c
ESPAT espat(<ssid>, <password>);
```

## begin(): bool
Caution!!: You should call this method before other methods on this library.    
```c
ESPAT espat(<ssid>, <password>);  

void setup(){
  espat.begin();
}
```
## checkAT(): bool
This method tell you AT command is Available or Not.

```c
espat.checkAT();
```

## changeMode(uint8_t mode): bool
This method send AT command(AT+CWMODE).  

Arguments:  
* mode => Wifi mode. (0: station, 1: softAP, 2: station + softAP)

```c
espat.changeMode(1); // Wifi is station mode.
```

## tryConnectAP(): bool
This method connect with AP.  

Caution!!: You should set correct ssid and passwd at Definition.  
Caution!!: You should set station mode by changeMode mothod.
```c
espat.changeMode(1) // set station mode
espat.tryConnectAP();
```

## clientIP(): String
This method tell you client IP address.

Returns:  
* "192.168.xx.xx" => Correct IP address.
* "" => If failed to get IP address, you get empty.    


You can check IP address by:
```c
if(espat.clientIP() != ""){
  // do something
}else{
  // do something for error
}
```

## get(String uri): String
This method send GET request.

Caution!!: You should call tryConnectAP before this method.  
Caution!!: You also hove to check client IP address is valid by call clientIP().  

```c
espat.get("www.google.co.jp"); // Return response.
```
