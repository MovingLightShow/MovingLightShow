/**********************************************************************
 *
 * MovingLightShow package - Synchronized LED strips for musicians
 * https://MovingLightShow.art
 *
 * @file  mls_ota.h
 * @brief OTA update and configuration header
 * 
 **********************************************************************/
#ifndef MLS_OTA_H
#define MLS_OTA_H

  #define DEBUG_MLS
  #include "DebugTools.h"
  
  #include <Arduino.h>
  #include <WString.h>
  #include "mls_tools.h"
  
  class MlsOta {
    
    private:
      String ota_iid;
      String ota_url;
      String actual_firmware;
      char new_firmware_url[1000];
      String getValue(String data, char separator, int index);
  
    public:
      char macAddr[14];
      char ssid[64];
      char secret[64];
      MlsOta(String ota_url, String actual_firmware);
      bool checkOtaUpdates(String ota_iid);
      void otaUpdates();
      String otaDownloadOptions(MlsTools::Config config);
      String urlencode(String str);
  
  };

#endif
