/**********************************************************************
 *
 * MovingLightShow package - Synchronized LED strips for musicians
 * https://MovingLightShow.art
 *
 * @file  mls_tools.cpp
 * @brief MLS tools (spiffs, load/import/save configurtion)
 * 
 **********************************************************************/
#include "mls_tools.h"
#include "SPIFFS.h"


MlsTools::MlsTools() {

  this->spiffs_available = false;
  this->disableDefauldSsid = false;

}


void MlsTools::loadConfiguration(String default_iid) {

  char jsonActual[JSON_SIZE];

  // Open file for reading
  File file = SPIFFS.open(spiffs_filename);

  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<JSON_SIZE> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    DEBUG_PRINTLN("Failed to read file, using default configuration");

  serializeJson(doc, jsonRead);

  // Copy values from the JsonDocument to the config
  strlcpy(this->config.iid,
          doc["iid"] | default_iid.c_str(),
          sizeof(this->config.iid));
  strlcpy(this->config.ssid1,
          doc["ssid1"] | "",
          sizeof(this->config.ssid1));
  this->config.ssid1validated = doc["ssid1validated"] | 0;
  strlcpy(this->config.secret1,
          doc["secret1"] | "",
          sizeof(this->config.secret1));
  strlcpy(this->config.ssid2,
          doc["ssid2"] | "",
          sizeof(this->config.ssid2));
  this->config.ssid2validated = doc["ssid2validated"] | 0;
  strlcpy(this->config.secret2,
          doc["secret2"] | "",
          sizeof(this->config.secret2));
  this->config.master = doc["master"] | 0;
  this->config.rank   = doc["rank"]   | 0;
  this->config.column = doc["column"] | 0;

  this->disableDefauldSsid = ((this->config.ssid1validated != 0) || (this->config.ssid2validated != 0));

  // Copy values from the config to the configRead
  strlcpy(this->configRead.iid,
          this->config.iid,
          sizeof(this->configRead.iid));
  strlcpy(this->configRead.ssid1,
          this->config.ssid1,
          sizeof(this->configRead.ssid1));
  this->configRead.ssid1validated = this->config.ssid1validated;
  strlcpy(this->configRead.secret1,
          this->config.secret1,
          sizeof(this->configRead.secret1));
  strlcpy(this->configRead.ssid2,
          this->config.ssid2,
          sizeof(this->configRead.ssid2));
  this->configRead.ssid2validated = this->config.ssid2validated;
  strlcpy(this->configRead.secret2,
          this->config.secret2,
          sizeof(this->configRead.secret2));
  this->configRead.master = this->config.master;
  this->configRead.rank   = this->config.rank;
  this->configRead.column = this->config.column;

  serializeJson(doc, jsonActual);
  DEBUG_PRINTLN("Json content: " + String(jsonActual));

  file.close();

}


void MlsTools::importConfiguration(String jsonImport) {

  char jsonActual[JSON_SIZE];

  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<JSON_SIZE> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, jsonImport);
  if (error)
    DEBUG_PRINTLN("Failed to import configuration");

  // Copy values from the JsonDocument to the Config
  strlcpy(this->config.iid,
          doc["iid"] | this->config.iid,
          sizeof(this->config.iid));
  strlcpy(this->config.ssid1,
          doc["ssid1"] | this->config.ssid1,
          sizeof(this->config.ssid1));
  strlcpy(this->config.secret1,
          doc["secret1"] | this->config.secret1,
          sizeof(this->config.secret1));
  strlcpy(this->config.ssid2,
          doc["ssid2"] | this->config.ssid2,
          sizeof(this->config.ssid2));
  strlcpy(this->config.secret2,
          doc["secret2"] | this->config.secret2,
          sizeof(this->config.secret2));
  this->config.master = doc["master"] | this->config.master,
  this->config.rank   = doc["rank"]   | this->config.rank,
  this->config.column = doc["column"] | this->config.column,

  serializeJson(doc, jsonActual);
  DEBUG_PRINTLN("Json content: " + String(jsonActual));

}


bool MlsTools::saveConfiguration() {

  char jsonActual[JSON_SIZE];

  // Use arduinojson.org/assistant to compute the capacity.
  StaticJsonDocument<JSON_SIZE> doc;

  // Validation must be disabled if an SSID has been changed
  if ((strcmp(this->configRead.ssid1, this->config.ssid1) != 0) || (strcmp(this->configRead.secret1, this->config.secret1) != 0)) {
    this->config.ssid1validated = 0;
  }
  if ((strcmp(this->configRead.ssid2, this->config.ssid2) != 0) || (strcmp(this->configRead.secret2, this->config.secret2) != 0)) {
    this->config.ssid2validated = 0;
  }

  // Set the values in the document
  doc["iid"]            = this->config.iid;
  doc["ssid1"]          = this->config.ssid1;
  doc["ssid1validated"] = this->config.ssid1validated;
  doc["secret1"]        = this->config.secret1;
  doc["ssid2"]          = this->config.ssid2;
  doc["ssid2validated"] = this->config.ssid2validated;
  doc["secret2"]        = this->config.secret2;
  doc["master"]         = this->config.master;
  doc["rank"]           = this->config.rank;
  doc["column"]         = this->config.column;

  // Serialize JSON to file only if changed
  serializeJson(doc, jsonActual);
  if (strcmp(jsonActual, jsonRead) != 0) {

    // Open file for writing
    File file = SPIFFS.open(spiffs_filename, "w");
    if (!file) {
      DEBUG_PRINTLN("Failed to create file");
      return false;
    }
    if (serializeJson(doc, file) == 0) {
      DEBUG_PRINTLN("Failed to write configuration file");
    } else {
      DEBUG_PRINTLN("Configuration file saved");
    }
    // Close the file
    file.close();
    return true;
  } else {
    DEBUG_PRINTLN("Identical configuration unchanged");
    return false;
  }

}


bool MlsTools::useDefaultSsid() {
  
  return (!this->disableDefauldSsid);

}


void MlsTools::spiffs_init() {

  DEBUG_PRINTLN();
  DEBUG_PRINTLN("Initializing SPIFFS");
  if (SPIFFS.begin()) {
    DEBUG_PRINTLN("SPIFFS mounted correctly");
    this->spiffs_available = true;
  } else {
    DEBUG_PRINTLN("Error during SPIFFS mounting");
    bool formatted = SPIFFS.format();
    if (formatted) {
      DEBUG_PRINTLN("SPIFFS formatted successfully");
      if (SPIFFS.begin()) {
        DEBUG_PRINTLN("SPIFFS mounted correctly");
        this->spiffs_available = true;
      } else {
        DEBUG_PRINTLN("Error during SPIFFS mounting");
      }
    } else {
      DEBUG_PRINTLN("Error while formatting SPIFFS");
    }
  }

  /// SPIFFS information /// SPIFFS information /// SPIFFS information /// SPIFFS information ///
  if (this->spiffs_available) {
    DEBUG_PRINTLN("Total SPIFFS space available: " + String(SPIFFS.totalBytes()) + " bytes");
    DEBUG_PRINTLN("Total SPIFFS space used: " + String(SPIFFS.usedBytes()) + " bytes");
  }
  DEBUG_PRINTLN();

}
