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

  #define DEBUG_MLS
  #include "DebugTools.h"

  #include <WString.h>
  
  #define JSON_SIZE 1024
  const String spiffs_filename = "/mls.ini";
  
  // https://arduinojson.org/
  #include <ArduinoJson.h>
  
  #include <Arduino.h>
  #include <WString.h>


  class MlsTools {
    
    public:
      MlsTools();
      struct Config {
        char iid[4]; // 3 chars and /0
        char ssid1[64];
        int ssid1validated;
        char secret1[64];
        char ssid2[64];
        int ssid2validated;
        char secret2[64];
        int master;
        int rank;
        int column;
      };
      Config config;
      Config configRead;
      void spiffs_init();
      void loadConfiguration(String default_iid);
      bool saveConfiguration();
      void importConfiguration(String jsonImport);
      bool useDefaultSsid();
  
    private:
      char jsonRead[JSON_SIZE];
      bool spiffs_available;
      bool disableDefauldSsid;
  
  };

#endif
