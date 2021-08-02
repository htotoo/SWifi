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