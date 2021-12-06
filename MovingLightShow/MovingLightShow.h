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
boolean i2sEnabled = false;

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
boolean overLastEdgeDectionLevel = false;
uint32_t minEdgeDetectionGap = 300000; // was 100 milliseconds, in microseconds
uint32_t maxEdgeDetectionGap = 600000; // was 300 milliseconds, in microseconds
uint16_t detectedBeatCounter = 0;

boolean result = false;
boolean forceFirmwareUpdate = false;
uint8_t forceFirmwareUpdateTrial = 0;

uint8_t state     = STATE_START;
uint8_t lastState = STATE_START;

uint8_t wifiStep = WIFI_SCAN_START;

uint8_t configStep = CONFIG_COLUMN;

uint32_t stateStartTime = 0;
uint32_t ssidScanStartTime = 0;
uint32_t ssidLastTrialTime = 0;
uint32_t ssidTrialTime = 0;

uint32_t configTimeOutTime = CONFIG_TIMEOUT_TIME;

uint8_t runningBrightness;

boolean flashEnable = false;
boolean flashLastState = false;

boolean MLS_masterMode = false;
boolean MLS_remoteControl = false;

boolean LoraIsUp = false;

boolean OneEspNowPacketReceived = false;

uint32_t mlsmeshLastPacketSentMs = 0;

uint8_t bleEffect = 0;
char bleParams[JSON_SIZE];

uint8_t loraReceived = 0;
char loraExtended[256];

uint8_t simulatorBeat = 0;
uint32_t simulatorLastBeat = 0;
uint32_t simulatorBeatSpeed = 1000; // (in ms, = 1s
uint8_t simulatorEffect = EFFECT_NONE;

TaskHandle_t TaskUpdateLightHandle = NULL;

String lora_rssi = "--";
String lora_packSize = "--";
String lora_packet;
String loraCommand;

uint32_t LastLoraCommandPacketId = 0;

uint32_t lastBleNotificationTS = 0; // in ms

uint32_t startStateRunningTS = 0; // in ms

boolean sendResult;

uint8_t announced_devices = 1;
struct DEVICE_INFO devices[210];
struct DEVICE_INFO my_device;

boolean nextEffectBeatEnabled = false;
uint8_t current_beat_effect = 0;

struct LIGHT_PACKET light_packet;
struct LIGHT_PACKET next_light_packet_to_send;
struct LIGHT_PACKET last_light_packet;

struct ACTION_PACKET action_packet;
uint8_t dummy_payload[20];

struct TOPOLOGY_PACKET topology_packet;

uint8_t last_effect_played = 0;
uint16_t last_packet_played = 0;

uint16_t last_packet_sent = 65535;
uint16_t last_packet_received = 65535;

uint32_t cmd_to_send_ts = 0;
uint8_t cmd_to_send = 0;

#ifdef MASTER_WITHOUT_EFFECT
boolean drum_in = false;
#else
boolean drum_in = true;
#endif

uint8_t demoStep = 0;

char bleFeedback[40];
char bleLastCmdInfo[40];
char tempStr[16];

char gIID[3];

uint8_t lastCommand = 0;
uint8_t lastCommandSenderId = 0;
uint16_t lastCommandPacketId = 0;

uint16_t mlsmeshLastPackedId = 0;

uint16_t checkCounter = 0;

uint32_t lastdisplayUpdateTime = 0;

uint32_t lastSubscribeTimeMs = 0;

#endif
