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
  #include "mls_light_effects.h"

  // https://github.com/FastLED/FastLED
  #define FASTLED_ALLOW_INTERRUPTS 0
  #include "FastLED.h"
  
  #define INITIAL_IID "MLS"
  #define OTA_URL "http://movinglightshow.art/"

  #define DEFAULT_WIFI_SSID   "IOT_NETWORK"
  #define DEFAULT_WIFI_SECRET "gzfh-dkse-6943-dfrt"

  #define LED_TYPE WS2812B
  #define LED_COLOR_ORDER GRB
  #define NUM_LEDS_PER_STRIP 18 // (18 LEDs, 30cm)
  #define LED_TEST_BRIGHTNESS    63
  #define LED_CONFIG_BRIGHTNESS  63
  #define LED_MAX_BRIGHTNESS    255
  #define LED_MIN_BRIGHTNESS     15

  #ifdef ARDUINO_ESP32_DEV
    #define ROTARY_ENCODER_BUTTON_PIN 12 // This PIN must NEVER be at 5V during boot !
    #define ROTARY_ENCODER_A_PIN      13
    #define ROTARY_ENCODER_B_PIN      14
    #define MASTER_PIN                34
    #define LEFT_LEDS_PIN             32
    #define RIGHT_LEDS_PIN            33
  #endif

  #ifdef ARDUINO_TTGO_LoRa32_v21new
    #define ROTARY_ENCODER_BUTTON_PIN 12 // This PIN must NEVER be at 5V during boot !
    #define ROTARY_ENCODER_A_PIN      13
    #define ROTARY_ENCODER_B_PIN      14
    #define MASTER_PIN                34
    #define LEFT_LEDS_PIN             36
    #define RIGHT_LEDS_PIN            39

    #define ONBOARD_LED     LED_BUILTIN
    #define LED_BUILDIN_ON         HIGH
    #define LED_BUILDIN_OFF         LOW
  #endif

  #define ROTARY_ENCODER_STEPS    8
  #define ROTARY_ENCODER_VCC_PIN -1

  CRGB leftLeds[NUM_LEDS_PER_STRIP];
  CRGB rightLeds[NUM_LEDS_PER_STRIP];

  #define STATE_START          0
  #define STATE_WIFI_SCAN      1
  #define STATE_WIFI_CONNECTED 2
  #define STATE_WIFI_FINISHED  3
  #define STATE_CONFIG         4
  #define STATE_CONFIG_DONE    5
  #define STATE_SUBSCRIBE      6
  #define STATE_RUNNING        7

  #define WIFI_SCAN_START   0
  #define WIFI_SCAN_SSID1   1
  #define WIFI_SCAN_SSID2   2
  #define WIFI_SCAN_DEFAULT 3

  #define CONFIG_COLUMN 0
  #define CONFIG_RANK   1

  #define SSID_REPEAT_TRIAL_TIME  100000 // 0.1 seconds = 100'000 microseconds
  #define SSID_TRIAL_MAX_TIME    6000000 // 6 seconds

  #define CONFIG_MAX_TIME       30000000 // 30 seconds

  String tempString;

  bool result = false;
  bool forceFirmwareUpdate = false;
  bool masterMode = false;

  uint8_t state = STATE_START;

  uint8_t wifiStep = WIFI_SCAN_START;

  uint8_t configStep = CONFIG_COLUMN;

  uint32_t stateStartTime = 0;
  uint32_t ssidScanStartTime = 0;
  uint32_t ssidLastTrialTime = 0;
  uint32_t ssidTrialTime = 0;

  uint8_t runningBrightness = (LED_MAX_BRIGHTNESS + 1)/16;


  bool flashEnable = false;
  bool flashLastState = false;

  uint32_t simulatorBeat = 0;
  uint32_t simulatorLastBeat = 0;
  uint32_t simulatorBeatSpeed = 500; // (in ms, = 0.5s
  uint8_t simulatorEffect = EFFECT_NONE;

  TaskHandle_t TaskUpdateLightHandle = NULL;
  
#endif
