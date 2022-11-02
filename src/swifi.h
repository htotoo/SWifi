#ifndef __SWIFI_H
#define __SWIFI_H

#include <stdint.h>
#include <vector>
#include <ArduinoOTA.h>
#include <Wifi.h>


typedef struct {
  char * ssid;
  char * passphrase;
} WifiAPlist_t;


class SWifi
{
  public:
    static void SetWifiMode(bool sta, bool ap);
    static void SetAP(const char* ssid, const char* password, int channel = 8, int ssid_hidden = 0, int max_connection = 5);
    static bool AddWifi(const char* ssid, const char *passphrase = NULL);
    static void SetHostname(String hostname);
    static uint8_t Connect();
    static bool IsConnected();
    static void Loop();
    static void InitOTA(String password = "");
	static void SetWatchdogTimeout(uint8_t minutes);
  private:
    static std::vector<WifiAPlist_t> APlist;
    static unsigned long lastCheck;
    static unsigned long lastConnOk;
    static bool otaEnabled;
	static uint8_t wdTimeMin;
};


#endif
