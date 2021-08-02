# SWifi

Just another ESP32 Wifi client.
Features: AP mode, and multi client mode (one at a time, but can reconnect to the strongest, that you set). Also an interface to Ardiono OTA simple use. Also it is async, so not blocking main logic, while maintains connection.

## Usage:

```cpp
#define ENABLEOTA 1
#define HOSTNAME "ESPSWIFI"

#include <swifi.h>

bool lastState = false;

void setup() {
  Serial.begin(115200);
  SWifi::SetAP("APNAME", "APPASS1234");
  SWifi::AddWifi("homeWifi", "homeWifiPass");
  SWifi::AddWifi("secondWifi", "secondWifiPass");
  SWifi::SetWifiMode(true, true);
  SWifi::SetHostname(HOSTNAME);
  SWifi::Connect();
#ifdef ENABLEOTA
  SWifi::InitOTA(); //to enable OTA updates
#endif

}


void loop() {
  SWifi::Loop(); //to maintain connection
  if (SWifi::IsConnected() != lastState)
  {
      lastState = SWifi::IsConnected();
      if (lastState) Serial.println("Now connected"); else Serial.println("Disconnected");
  }
}
```

Based on Markus Sattler's ESP8266WiFiMulti.h, and also used code from the ArdionoOTA. Thanks for theirs work!