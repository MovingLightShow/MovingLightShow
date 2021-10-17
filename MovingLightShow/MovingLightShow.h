/**********************************************************************

   MovingLightShow package - Synchronized LED strips for musicians
   https://MovingLightShow.art

   @file  MovingLightShow.h
   @brief Main header file

 **********************************************************************/
#ifndef MOVING_LIGHT_SHOW_H
#define MOVING_LIGHT_SHOW_H
  
  #define DEBUG_MLS
  #include "DebugTools.h"

  #define INITIAL_IID "MLS"
  #define OTA_URL "http://movinglightshow.art/"

  const char mls_default_ssid[64]   = "IOT_NETWORK";
  const char mls_default_secret[64] = "gzfh-dkse-6943-dfrt";

  #define LEDS_TYPE WS2812B
  #define NUM_LEDS_PER_STRIP 18 // (18 LEDs, 30cm)
  
  #ifdef ARDUINO_TTGO_LoRa32_v21new
    #define ONBOARD_LED LED_BUILTIN
    #define LED_ON HIGH
    #define LED_OFF LOW
  #endif
  
  #define LEFT_LEDS_PIN 14
  #define RIGHT_LEDS_PIN 26
  
  #define STATE_START          0
  #define STATE_WIFI_SCAN      1
  #define STATE_WIFI_CONNECTED 2
  #define STATE_WIFI_FINISHED  3
  #define STATE_CONFIG_DONE    4
  #define STATE_RUNNING        5

  #define WIFI_SCAN_START   0
  #define WIFI_SCAN_SSID1   1
  #define WIFI_SCAN_SSID2   2
  #define WIFI_SCAN_DEFAULT 3

  #define SSID_ONE_TRIAL_TIME 100000  // 0.1s = 100000 microseconds
  #define SSID_TRIAL_MAX_TIME 5000000 // 5s

  int mlsState = STATE_START;
  unsigned long mlsStateStart = 0;
  unsigned long mlsScanStart = 0;
  unsigned long mlsLastTrial = 0;
  int ssidStep = 0;
  int ssidTrial = 0;

  int testCounter = 0;
  
#endif
