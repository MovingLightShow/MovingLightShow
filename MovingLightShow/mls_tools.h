 /**********************************************************************
 *
 * MovingLightShow package - Synchronized LED strips for musicians
 * https://MovingLightShow.art
 *
 * @file  mls_tools.h
 * @brief MLS tools header
 * 
 **********************************************************************/
#ifndef MLS_TOOLS_H
#define MLS_TOOLS_H

  #include "mls_config.h"
  #include "DebugTools.h"

  #include <WString.h>
  
  const String spiffs_filename = "/mls.ini";
  
  // https://arduinojson.org/
  #include <ArduinoJson.h>
  
  #include <Arduino.h>
  #include <WString.h>


  class MlsTools {
    public:
      MlsTools();
      struct Config {
        char iid[4]; // 3 chars + /0
        char uniqueid[UNIQUEID_CHAR_SIZE];
        char ssid1[SSID_CHAR_SIZE];
        int ssid1validated;
        char secret1[SECRET_CHAR_SIZE];
        char ssid2[SSID_CHAR_SIZE];
        int ssid2validated;
        char secret2[SECRET_CHAR_SIZE];
        int master;
        int rank;
        int column;
        int remote;
      } __attribute__((__packed__));
      Config config;
      Config configRead;
      void spiffs_init();
      void loadConfiguration(String default_iid);
      boolean saveConfiguration();
      void importConfiguration(String jsonImport);
      boolean useDefaultSsid();
  
    private:
      char jsonRead[JSON_SIZE];
      char jsonActual[JSON_SIZE];
      boolean spiffs_available;
      boolean disableDefauldSsid;
  };

#endif
