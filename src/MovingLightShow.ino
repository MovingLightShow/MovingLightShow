/**********************************************************************
 *
 * MovingLightShow package - Synchronized LED strips for musicians
 * https://MovingLightShow.art
 *
 * @file  MovingLightShow.ino
 * @brief Main file of the MovingLightShow package
 *
 * MovingLightShow can be used by up to 100 musicians simultaneously. Some light
 * effects are synchronized with the bass drum, and some others are just visual effects
 * with specific timing. Effects are programmed to works in 4 and 6 columns layout.
 * 6 columns layout position is calculated automagically from the 4 columns layout,
 * based on the Armourins' practice.
 *
 * The MovingLightShow concept is based on the following elements:
 * - ESP32 boards with ESP-Now support
 * - ESP32 boards with LoRa support and OLED display (remote control from outside of the band)
 * - Rotary Digital Encoder Push Button for the setup
 * - WS2812 intelligent LEds strips
 * - MIDI launchpad with 64 PDAD to select the effects (experimental)
 * - 5V powerbank for each musician, with at least 25 Wh of energy (10'000 mAh)
 * - MLSmesh proprietary interconnection protocol using ESP-Now,
 *   with dynamic repeaters selection based on RSSI analysis
 * - automatic OTA (over-the-air) firmware upgrade during boot,
 *   if a known access point is available
 *
 * This package is the result of a *LOT* of work. If you are happy using this
 * package, contact us for a donation to support this project.
 *
 * @author    MovingLightShow, Showband Les Armourins <contact@movinglightshow.art>
 * @version   1.0.0.6
 * @date      2021-10-17
 * @since     2021-01-01
 * @copyright (c) 2020-2021 Showband Les Armourins
 * @copyright GNU Lesser General Public License
 * 
 * Compatible boards:
 * - ESP32 Arduino / TTGO LoRa32-OLED v2.1.6 (ttgo-lora32-v21new)
 *   (board definition has been adjusted in boards.txt to add OTA support)
 * - ESP32 Arduino / EP32 Dev module (esp32)
 *
 * ESP32 Partition organization:
 * - Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)
 *
 * Changes log:
 *   2021-10-17 1.0.0.6 [ENH] Better customization options
 *                      [ENH] Debug Library integrated
 *                      [ENH] Online update optimization
 *                      [ENH] Non-volatile storage of options
 *   2021-10-14 1.0.0.1 [ENH] Code unification, one code for all boards
 *   2021-10-13 1.0.0.0 [ENH] OTA is working with various boards :-)
 *   2021-01-01 0.0.0.1 first try and first ideas
 *   
 **********************************************************************/
const String actual_firmware = "1.0.0.6";

#include "MovingLightShow.h"
#include "mls_tools.h"
#include "mls_ota.h"

// https://github.com/espressif/arduino-esp32/issues/595
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

// https://github.com/RobTillaart/CRC
#include "CRC8.h"
#include "CRC.h"

// https://github.com/FastLED/FastLED
#define FASTLED_ALLOW_INTERRUPTS 0
#include "FastLED.h"

#ifdef ARDUINO_TTGO_LoRa32_v21new
  // https://github.com/ThingPulse/esp8266-oled-ssd1306
  #include "SSD1306.h"
  #include "mls_oled_logo.h"
#endif

#include <Wire.h>

#include <esp_now.h>
#include <WiFi.h>
#include "esp_wifi.h"


/// Classes instantiation /// Classes instantiation /// Classes instantiation /// Classes instantiation ///
MlsOta mlsota(OTA_URL, actual_firmware);
MlsTools mlstools;

#ifdef ARDUINO_TTGO_LoRa32_v21new
  SSD1306 display(0x3c, SDA, SCL);
#endif


/// SETUP /// SETUP /// SETUP /// SETUP  ///
void setup() {

  #ifdef ONBOARD_LED
    pinMode (ONBOARD_LED, OUTPUT);
  #endif

  #ifdef DEBUG_MLS
    // Define the integrated serial port speed for debug monitoring
    Serial.begin(115200);
  #endif
  DEBUG_PRINTLN("Booting (setup)...");
  DEBUG_PRINTLN("Board: " + String(ARDUINO_BOARD));

  // Activate Wifi client mode
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  WiFi.setSleep(false);
  esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
  DEBUG_PRINTLN("WiFi MAC address: " + String(WiFi.macAddress()));

  // Initialize SPIFFS
  mlstools.spiffs_init();

  // Load non-volatile configuration
  mlstools.loadConfiguration(INITIAL_IID);

  #ifdef ARDUINO_TTGO_LoRa32_v21new
    // Initialize the 128x64 OLED display using Wire library
    DEBUG_PRINTLN("OLED display initialized");
    display.init();
    display.flipScreenVertically();
    display.clear();
    display.drawXbm(0, 0, logo_width, logo_height, logo_bits);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, actual_firmware);
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(127, 47, "MLS");
    display.display();
  #endif

}


/// LOOP /// LOOP /// LOOP /// LOOP ///
void loop() {

  // Initialization
  if (mlsState == STATE_START) {
    mlsState = STATE_WIFI_SCAN;
    ssidStep = WIFI_SCAN_START;
    mlsStateStart = micros();
    mlsScanStart = mlsStateStart;
  }

  if (mlsState == STATE_WIFI_SCAN) {
    if (ssidStep == WIFI_SCAN_START) {
      ssidStep = WIFI_SCAN_SSID1;
      mlsLastTrial = micros();

      if (strcmp(mlstools.config.ssid1, "") != 0) {
        strcpy(mlsota.ssid,   mlstools.config.ssid1);
        strcpy(mlsota.secret, mlstools.config.secret1);
        DEBUG_PRINTLN("Try to connect Wifi AP with SSID " + String(mlsota.ssid));
        mlsLastTrial = micros();
        WiFi.begin(mlsota.ssid, mlsota.secret);
      } else {
        mlsLastTrial = mlsLastTrial - SSID_TRIAL_MAX_TIME;
      }
    }

    if ((micros() - mlsLastTrial) >= SSID_TRIAL_MAX_TIME) {
      WiFi.disconnect();
      mlsLastTrial = micros();
      if (ssidStep == WIFI_SCAN_DEFAULT) {
        DEBUG_PRINTLN("No Wifi connection detected");
        mlsState = STATE_WIFI_FINISHED;
      } else if (ssidStep == WIFI_SCAN_SSID2) {
        if (mlstools.useDefaultSsid()) {
          ssidStep = WIFI_SCAN_DEFAULT;
          strcpy(mlsota.ssid,   mls_default_ssid);
          strcpy(mlsota.secret, mls_default_secret);
          DEBUG_PRINTLN("Try to connect Wifi AP with SSID " + String(mlsota.ssid));
          WiFi.begin(mlsota.ssid, mlsota.secret);
        } else {
          DEBUG_PRINTLN("No Wifi connection detected");
          mlsState = STATE_WIFI_FINISHED;
        }
      } else if (ssidStep == WIFI_SCAN_SSID1) {
        ssidStep = WIFI_SCAN_SSID2;
        if (strcmp(mlstools.config.ssid2, "") != 0) {
          strcpy(mlsota.ssid,   mlstools.config.ssid2);
          strcpy(mlsota.secret, mlstools.config.secret2);
          DEBUG_PRINTLN("Try to connect Wifi AP with SSID " + String(mlsota.ssid));
          WiFi.begin(mlsota.ssid, mlsota.secret);
        } else {
          mlsLastTrial = mlsLastTrial - SSID_TRIAL_MAX_TIME;
        }
      }
    }

    if ((micros() - mlsLastTrial) >= SSID_ONE_TRIAL_TIME) {
      if (WiFi.status() == WL_CONNECTED) {
        DEBUG_PRINTF("Device connected to AP after %d seconds\n",((micros()-mlsScanStart)/1000000));
        mlsState = STATE_WIFI_CONNECTED;
      }
    }
  }

  if (mlsState == STATE_WIFI_CONNECTED) {
    if (ssidStep == WIFI_SCAN_SSID1) {
      mlstools.config.ssid1validated = 1;
    } else if (ssidStep == WIFI_SCAN_SSID2) {
      mlstools.config.ssid2validated = 1;
    }
    DEBUG_PRINTLN("Wifi SSID connected: " + String(mlsota.ssid));
    DEBUG_PRINT("Wifi IP address: ");
    DEBUG_PRINTLN(WiFi.localIP());
    #ifdef ARDUINO_TTGO_LoRa32_v21new
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.setFont(ArialMT_Plain_10);
      display.drawString(0, 53, "Wifi OK");
      display.display();
    #endif

    // OTA update and configuration import - TODO: must be enhanced (parallelized with the user setup at boot)
    if (mlsota.checkOtaUpdates(mlstools.config.iid)) {
      DEBUG_PRINT("OTA update will be started.");
      #ifdef ARDUINO_TTGO_LoRa32_v21new
        display.clear();
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.setFont(ArialMT_Plain_16);
        display.drawString(64, 23, "- Firmware update -");
        display.display();
      #endif
      mlsota.otaUpdates();
    }
    mlstools.importConfiguration(mlsota.otaDownloadOptions(mlstools.config));

    WiFi.disconnect();
    mlsState = STATE_WIFI_FINISHED;
  }

  if (mlsState == STATE_WIFI_FINISHED) {
    #ifdef ARDUINO_TTGO_LoRa32_v21new
      display.setTextAlignment(TEXT_ALIGN_RIGHT);
      display.setFont(ArialMT_Plain_10);
      display.drawString(127, 0, mlstools.config.iid);
      display.display();
    #endif
    
    // This last state is set by the end of the user boot configuration
    mlsState = STATE_CONFIG_DONE;
  }

  // Setup options, and after, save them

  if (mlsState == STATE_CONFIG_DONE) {
    mlstools.saveConfiguration();
    mlsState = STATE_RUNNING;
  }

  if ((mlsState != STATE_CONFIG_DONE) && (mlsState != STATE_RUNNING)) {
    #ifdef ONBOARD_LED
      testCounter++;
      if ((testCounter % 100) > 49) {
        if ((testCounter % 50) == 0) {
          digitalWrite(ONBOARD_LED, LED_ON);
        }
      } else {
        if ((testCounter % 50) == 0) {
          digitalWrite(ONBOARD_LED, LED_OFF);
        }
      }
    delay(5);
    #endif
  }

  if (mlsState == STATE_RUNNING) {
    int counter = 0;
    while (true) {
      DEBUG_PRINT(".");
      counter++;
      if (0 == (counter % 40)) {
        DEBUG_PRINTLN();
      }
      #ifdef ONBOARD_LED
        if ((counter % 2) > 0) {
          digitalWrite(ONBOARD_LED, LED_ON);
        } else {
          digitalWrite(ONBOARD_LED, LED_OFF);
        }
      #endif
      delay(1000);
    }
  }
}
