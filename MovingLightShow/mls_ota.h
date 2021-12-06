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

  #include "mls_config.h"
  #include "DebugTools.h"
  
  #include <Arduino.h>
  #include <WString.h>
  #include "mls_tools.h"
  
  class MlsOta {
    private:
      String ota_iid;
      String ota_url;
      String actual_firmware;
      char new_firmware_url[URL_CHAR_SIZE];
      String getValue(String data, char separator, int index);
  
    public:
      char macAddr[MAC_ADDR_CHAR_SIZE];
      char ssid[SSID_CHAR_SIZE];
      char secret[SECRET_CHAR_SIZE];
      MlsOta(String ota_url, String actual_firmware);
      boolean checkOtaUpdates(String ota_iid);
      void otaUpdates();
      void otaUpdates(boolean forced);
      String otaDownloadOptions(MlsTools::Config config);
      String urlencode(String str);
  };

#endif
