
#include "swifi.h"

std::vector<WifiAPlist_t> SWifi::APlist;
unsigned long SWifi::lastCheck = 0;
bool SWifi::otaEnabled = false;

void SWifi::SetWifiMode(bool sta, bool ap)
{
  if (!sta && ! ap) WiFi.mode(WIFI_OFF);
  if (!sta && ap) WiFi.mode(WIFI_AP);
  if (sta && !ap) WiFi.mode(WIFI_STA);
  if (sta && ap) WiFi.mode(WIFI_AP_STA);
  WiFi.setAutoConnect(false);
}

void SWifi::SetAP(const char* ssid, const char* password, int channel, int ssid_hidden, int max_connection)
{
  WiFi.softAP( ssid,  password,  channel,  ssid_hidden,  max_connection);
}

bool SWifi::AddWifi(const char* ssid, const char *passphrase)
{
  WifiAPlist_t newAP;

  if (!ssid || *ssid == 0x00 || strlen(ssid) > 31) {
    log_e("[SWIFI][AddWifi] no ssid or ssid too long");
    return false;
  }

  if (passphrase && strlen(passphrase) > 63) {
    log_e("[SWIFI][AddWifi] passphrase too long");
    return false;
  }
  newAP.ssid = strdup(ssid);

  if (!newAP.ssid) {
    log_e("[SWIFI][AddWifi] fail to set SSID");
    return false;
  }

  if (passphrase && *passphrase != 0x00) {
    newAP.passphrase = strdup(passphrase);
    if (!newAP.passphrase) {
      log_e("[SWIFI][AddWifi] fail to set passphrase");
      free(newAP.ssid);
      return false;
    }
  } else {
    newAP.passphrase = NULL;
  }

  APlist.push_back(newAP);
  log_i("[SWIFI][AddWifi] added SSID: %s", newAP.ssid);
  return true;
}



uint8_t SWifi::Connect()
{
  int8_t scanResult;
  uint8_t status = WiFi.status();
  
  if (status == WL_CONNECTED) {
    for (uint32_t x = 0; x < APlist.size(); x++) {
      if (WiFi.SSID() == APlist[x].ssid) {
        return status;
      }
    }
    WiFi.disconnect(false, false);
    delay(10);
    status = WiFi.status();
  }
  scanResult = WiFi.scanComplete();
  if (scanResult == WIFI_SCAN_FAILED || status == WL_NO_SSID_AVAIL)
  {
    WiFi.scanDelete();
    scanResult = WiFi.scanNetworks(true);
    return WIFI_SCAN_RUNNING;
  }
  if (scanResult == WIFI_SCAN_RUNNING) {
    // scan is running
    log_i("[SWIFI] Scanning");
    return WL_NO_SSID_AVAIL;
  } else if (scanResult >= 0) {
    // scan done analyze
    log_i("[SWIFI] Scan done");
    WifiAPlist_t bestNetwork { NULL, NULL };
    int bestNetworkDb = INT_MIN;
    uint8_t bestBSSID[6];
    int32_t bestChannel = 0;
    if (scanResult == 0) {
      log_i("[SWIFI] no networks found");
    } else {
      for (int8_t i = 0; i < scanResult; ++i) {
        String ssid_scan;
        int32_t rssi_scan;
        uint8_t sec_scan;
        uint8_t* BSSID_scan;
        int32_t chan_scan;

        WiFi.getNetworkInfo(i, ssid_scan, sec_scan, rssi_scan, BSSID_scan, chan_scan);

        bool known = false;
        for (uint32_t x = 0; x < APlist.size(); x++) {
          WifiAPlist_t entry = APlist[x];
          if (ssid_scan == entry.ssid) { // SSID match
            known = true;
            if (rssi_scan > bestNetworkDb) { // best network
              if (sec_scan == WIFI_AUTH_OPEN || entry.passphrase) { // check for passphrase if not open wlan
                bestNetworkDb = rssi_scan;
                bestChannel = chan_scan;
                memcpy((void*) &bestNetwork, (void*) &entry, sizeof(bestNetwork));
                memcpy((void*) &bestBSSID, (void*) BSSID_scan, sizeof(bestBSSID));
              }
            }
          }
        }
      }
    }
    // clean up ram
    WiFi.scanDelete();

    if (bestNetwork.ssid) {
      log_i("[SWIFI] Connecting");
      WiFi.begin(bestNetwork.ssid, bestNetwork.passphrase, bestChannel, bestBSSID);
      log_i("[SWIFI] Connecting called");
    } else {
      log_i("[SWIFI] No matching Wifi found");
    }
  } else {
    // start scan
    WiFi.disconnect();
    // scan wifi async mode
    WiFi.scanNetworks(true);
  }

  return status;
}

void SWifi::InitOTA(String password)
{
  SWifi::otaEnabled = true;
  if (password.length()>0) ArduinoOTA.setPassword(password.c_str());
  ArduinoOTA
  .onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";
  })
  .onEnd([]() {
  })
  .onProgress([](unsigned int progress, unsigned int total) {
  })
  .onError([](ota_error_t error) {
  });
  ArduinoOTA.begin();
}

void SWifi::Loop()
{
  if (SWifi::otaEnabled) ArduinoOTA.handle();
  if (millis() - SWifi::lastCheck < 2000) return; //2mp
  SWifi::lastCheck = millis();
  uint8_t status = WiFi.status();
  if (status == WL_CONNECTED)
  {
    return; //nothing to do
  }
  log_i("[SWIFI] Not connected, so check Connect");
  SWifi::Connect();
}

bool SWifi::IsConnected()
{
  return (WiFi.status() == WL_CONNECTED);
}

void SWifi::SetHostname(String hostname)
{
  WiFi.setHostname(hostname.c_str());
  ArduinoOTA.setHostname(hostname.c_str());
}
