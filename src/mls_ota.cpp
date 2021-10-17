/**********************************************************************
 *
 * MovingLightShow package - Synchronized LED strips for musicians
 * https://MovingLightShow.art
 *
 * @file  mls_ota.cpp
 * @brief OTA update and configuration
 * 
 **********************************************************************/
#include "mls_ota.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClient.h>


MlsOta::MlsOta(String ota_url, String actual_firmware) {

  this->ota_url = ota_url;
  this->actual_firmware = actual_firmware;
  uint8_t mac[6];
  WiFi.macAddress(mac);
  sprintf(this->macAddr,"%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]); /// small letters at MAC address

}


bool MlsOta::checkOtaUpdates(String ota_iid) {
  WiFiClient client;
  bool ota_check_result = false;

  this->ota_iid = ota_iid;
  String fwURL = this->ota_url;
  fwURL.concat("?mac=");
  fwURL.concat(this->macAddr);
  fwURL.concat("&iid=");
  fwURL.concat(this->ota_iid);
  fwURL.concat("&board=");
  fwURL.concat(ARDUINO_VARIANT);
  fwURL.concat("&firmware=");
  fwURL.concat(this->actual_firmware);
  DEBUG_PRINTLN("Check for firmware update using this URL: " + fwURL);

  HTTPClient httpClient;
  httpClient.begin(client, fwURL);
  int httpCode = httpClient.GET();
  if (httpCode == 200) {
    String http_result = httpClient.getString().c_str();
    String online_version = this->getValue(http_result,':',0);
    String online_size = this->getValue(http_result,':',1);
    DEBUG_PRINTLN("Online firmware version: " + online_version + " (" + online_size + " bytes)");
    DEBUG_PRINTLN("Local firmware version: " + this->actual_firmware);

    online_version.trim();
    if (!online_version.equals(this->actual_firmware)) {
      fwURL.concat("&download=");
      fwURL.concat(online_version);
      ota_check_result = true;
      strcpy(this->new_firmware_url, fwURL.c_str());
      DEBUG_PRINTLN("Firmware uppdate available");
    } else {
      DEBUG_PRINTLN("Firmware is already up to date");
    }
  } else {
    DEBUG_PRINTF("Http error (%d)", httpCode);
    DEBUG_PRINTLN();
  }
  httpClient.end();
  return ota_check_result;
}


void MlsOta::otaUpdates() {
  WiFiClient client;
  t_httpUpdate_return ret = httpUpdate.update(client, this->new_firmware_url);
  switch(ret) {
    case HTTP_UPDATE_FAILED:
      DEBUG_PRINTFF("HTTP_UPDATE_FAILED Error (%d): %s", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      DEBUG_PRINTFF("HTTP_UPDATE_NO_UPDATES (%d): %s", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;
  }
}


String MlsOta::otaDownloadOptions(MlsTools::Config config) {

  WiFiClient client;
  String fwURL = this->ota_url;
  String http_result;
  fwURL.concat("?mac=");
  fwURL.concat(this->macAddr);
  fwURL.concat("&iid=");
  fwURL.concat(this->ota_iid);
  fwURL.concat("&board=");
  fwURL.concat(this->urlencode(ARDUINO_VARIANT));
  fwURL.concat("&firmware=");
  fwURL.concat(this->actual_firmware);
  fwURL.concat("&cnf=1");
  fwURL.concat("&ssid1=");
  fwURL.concat(this->urlencode(config.ssid1));
  fwURL.concat("&ssid2=");
  fwURL.concat(this->urlencode(config.ssid2));
  fwURL.concat("&master=");
  fwURL.concat(config.master);
  fwURL.concat("&rank=");
  fwURL.concat(config.rank);
  fwURL.concat("&column=");
  fwURL.concat(config.column);
  DEBUG_PRINTLN("Configuration information sent: " + fwURL);

  HTTPClient httpClient;
  httpClient.begin(client, fwURL);

  int httpCode = httpClient.GET();
  if (httpCode == 200) {
    http_result = httpClient.getString().c_str();
    DEBUG_PRINTLN("Configuration information received: " + String(http_result));
  } else {
    http_result = "";
  }
  httpClient.end();
  return http_result;

}


// https://stackoverflow.com/questions/9072320/split-string-into-string-array
String MlsOta::getValue(String data, char separator, int index) {

  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";

}


// https://github.com/zenmanenergy/ESP8266-Arduino-Examples/
String MlsOta::urlencode(String str) {

  String encodedString="";
  char c;
  char code0;
  char code1;
  char code2;
  for (int i =0; i < str.length(); i++){
    c=str.charAt(i);
    if (c == ' '){
      encodedString+= '+';
    } else if (isalnum(c)){
      encodedString+=c;
    } else{
      code1=(c & 0xf)+'0';
      if ((c & 0xf) >9){
          code1=(c & 0xf) - 10 + 'A';
      }
      c=(c>>4)&0xf;
      code0=c+'0';
      if (c > 9){
          code0=c - 10 + 'A';
      }
      code2='\0';
      encodedString+='%';
      encodedString+=code0;
      encodedString+=code1;
      //encodedString+=code2;
    }
    yield();
  }
  return encodedString;

}
