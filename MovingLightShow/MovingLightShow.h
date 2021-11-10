/**********************************************************************

   MovingLightShow package - Synchronized LED strips for musicians
   https://MovingLightShow.art

   @file  MovingLightShow.h
   @brief Main header file

 **********************************************************************/
#ifndef MOVING_LIGHT_SHOW_H
#define MOVING_LIGHT_SHOW_H
  
  #include "mls_config.h"
  #include "DebugTools.h"
  #include "mls_mesh.h"
  #include "mls_light_effects.h"
  #include "driver/i2s.h"

  // https://github.com/FastLED/FastLED
  #include "FastLED.h"
  
  /**********************************************************************

                    !!! IMPORTANT PINOUT INFORMATION !!!
  
    PIN 12 must NEVER be at Vcc during boot sequence
    Strapping pin: GPIO0, GPIO2, GPIO5, GPIO12 (MTDI), and GPIO15 (MTDO) are strapping pins.
      For more infomation, please refer to ESP32 datasheet.
    SPI0/1: GPIO6-11 and GPIO16-17 are usually connected to the SPI flash and PSRAM integrated
      on the module and therefore should not be used for other purposes.
    JTAG: GPIO12-15 are usually used for inline debug.
    GPI: GPIO34-39 can only be set as input mode and do not have software-enabled pullup or pulldown functions.
    TXD & RXD are usually used for flashing and debugging.
      ADC2: ADC2 pins cannot be used when Wi-Fi is used. So, if you’re using Wi-Fi and you’re having trouble
      getting the value from an ADC2 GPIO, you may consider using an ADC1 GPIO instead, that should solve
      your problem. For more details, please refer to ADC limitations.

   Based on: https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
             https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html
      
   **********************************************************************/

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

  String tempString;

  const i2s_port_t I2S_PORT = I2S_NUM_0;
  bool i2sEnabled = false;

  uint32_t i2s_rolling_mean[I2S_ROLLING_MEAN_SIZE];
  uint32_t i2s_rollingmaxInput = 0;
  uint32_t i2s_sampleCounter = 0;
  uint32_t i2s_long_term_mean = 0;
  uint32_t i2s_biggestInput = 0;
  uint32_t i2s_maxInput = 0;
  uint32_t i2s_maxInputTime = 0;
  uint32_t i2s_secondtMaxInput = 0;
  uint32_t i2s_inputFlushTime = 1000000; // was 2 seconds, in microseconds
  uint32_t i2s_maxInputFlushTime = 2000000; // was 4 seconds, in microseconds
  uint32_t lastEdgeDectionTime = 0; // in microseconds
  uint32_t lastEdgeDectionLevel = 0;
  bool overLastEdgeDectionLevel = false;
  uint32_t minEdgeDetectionGap = 300000; // was 100 milliseconds, in microseconds
  uint32_t maxEdgeDetectionGap = 600000; // was 300 milliseconds, in microseconds
  uint32_t detectedBeatCounter = 0;

  bool result = false;
  bool forceFirmwareUpdate = false;
  uint8_t forceFirmwareUpdateTrial = 0;

  uint8_t state     = STATE_START;
  uint8_t lastState = STATE_START;

  uint8_t wifiStep = WIFI_SCAN_START;

  uint8_t configStep = CONFIG_COLUMN;

  uint32_t stateStartTime = 0;
  uint32_t ssidScanStartTime = 0;
  uint32_t ssidLastTrialTime = 0;
  uint32_t ssidTrialTime = 0;

  uint32_t configMaxTime = CONFIG_MAX_TIME;

  uint8_t runningBrightness;

  bool flashEnable = false;
  bool flashLastState = false;

  bool MLS_masterMode = false;
  bool MLS_remoteControl = false;

  bool LoraIsUp = false;

  uint8_t bleEffect = 0;
  char bleParams[JSON_SIZE];

  uint8_t loraCommand = 0;

  uint8_t simulatorBeat = 0;
  uint32_t simulatorLastBeat = 0;
  uint32_t simulatorBeatSpeed = 500; // (in ms, = 0.5s
  uint8_t simulatorEffect = EFFECT_NONE;

  TaskHandle_t TaskUpdateLightHandle = NULL;

  String lora_rssi = "--";
  String lora_packSize = "--";
  String lora_packet;

  uint8_t announced_devices = 123;

  struct LIGHT_PACKET light_packet;
  struct ACTION_PACKET action_packet;
  uint8_t dummy_payload[20];

#endif
