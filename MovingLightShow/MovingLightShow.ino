/**********************************************************************
 *
 * MovingLightShow package - Synchronized LED strips for musicians
 * https://MovingLightShow.art
 *
 * @file  MovingLightShow.ino
 * @brief Main file of the MovingLightShow package
 *
 * MovingLightShow can be used by up to 200 musicians simultaneously. Some light
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
 * @version   1.0.1.0
 * @date      2021-10-22
 * @since     2021-01-01
 * @copyright (c) 2020-2021 Showband Les Armourins
 * @copyright GNU Lesser General Public License
 * 
 * Compatible boards:
 * - ESP32 Arduino / TTGO LoRa32-OLED v2.1.6 ESP32 PICO-D4 (ttgo-lora32-v21new)
 *   (board definition has been adjusted in boards.txt to add OTA support)
 * - ESP32 Arduino / EP32 Dev module (esp32)
 *
 * ESP32 Partition organization:
 * - Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)
 *
 * Changes log:
 *   2021-10-22 1.0.1.0 [ENH] New implemented effects and modifiers
 *                      [ENH] Rotary encoder implementation
 *   2021-10-17 1.0.0.6 [ENH] Better customization options
 *                      [ENH] Debug Library integrated
 *                      [ENH] Online update optimization
 *                      [ENH] Non-volatile storage of options
 *   2021-10-14 1.0.0.1 [ENH] Code unification, one code for all boards
 *   2021-10-13 1.0.0.0 [ENH] OTA is working with various boards :-)
 *   2021-01-01 0.0.0.1 first try and first ideas
 *   
 **********************************************************************/
const String actual_firmware = "1.0.1.0";

// https://github.com/espressif/arduino-esp32/issues/595
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

// https://github.com/RobTillaart/CRC
#include "CRC8.h"
#include "CRC.h"

// https://github.com/igorantolic/ai-esp32-rotary-encoder
#include "AiEsp32RotaryEncoder.h"

#ifdef ARDUINO_TTGO_LoRa32_v21new
  // https://github.com/ThingPulse/esp8266-oled-ssd1306
  #include "SSD1306.h"
  #include "mls_oled_logo.h"
#endif

#include <Wire.h>

#include <esp_now.h>
#include <WiFi.h>
#include "esp_wifi.h"

#include "MovingLightShow.h"
#include "mls_tools.h"
#include "mls_ota.h"
#include "mls_light_effects.h"


/// Classes instantiation /// Classes instantiation /// Classes instantiation /// Classes instantiation ///
MlsOta mlsota(OTA_URL, actual_firmware);
MlsTools mlstools;
MlsLightEffects mlslighteffects(NUM_LEDS_PER_STRIP, leftLeds, rightLeds);

AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);

#ifdef ARDUINO_TTGO_LoRa32_v21new
  SSD1306 display(0x3c, SDA, SCL);
#endif

void IRAM_ATTR readEncoderISR() {
  rotaryEncoder.readEncoder_ISR();
}


/// PERMANENT TASK /// PERMANENT TASK /// PERMANENT TASK /// PERMANENT TASK ///
void TaskUpdateLight( void * pvParameters ){
  while(true) {
    mlslighteffects.updateLight();
    yield();
    delayMicroseconds(100);
  }
}


/// SETUP /// SETUP /// SETUP /// SETUP  ///
void setup() {

  pinMode(0, INPUT); // on board boot button, used to force firmware update

  pinMode(MASTER_PIN, INPUT_PULLDOWN);
  masterMode = (1 == digitalRead(MASTER_PIN));

  #ifdef ONBOARD_LED
    pinMode(ONBOARD_LED, OUTPUT);
  #endif

  FastLED.addLeds<LED_TYPE, LEFT_LEDS_PIN, LED_COLOR_ORDER>(leftLeds, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<LED_TYPE, RIGHT_LEDS_PIN, LED_COLOR_ORDER>(rightLeds, NUM_LEDS_PER_STRIP);
  FastLED.clear();

  FastLED.setBrightness(LED_TEST_BRIGHTNESS);
  mlslighteffects.setLightData(millis(), (LIGHT_PACKET){EFFECT_PROGRESS_RAINBOW, MODIFIER_REPEAT, millis(), 1500, 0, 255, 0, 12, 5, 12, 0, 255, 0, 12, 5, 12}); // RAINBOW 1500

  #ifdef DEBUG_MLS
    // Define the integrated serial port speed for debug monitoring
    Serial.begin(115200);
  #endif
  DEBUG_PRINTLN();
  DEBUG_PRINTLN("========================================");
  DEBUG_PRINTLN();
  DEBUG_PRINTLN("MovingLightShow.art " + actual_firmware + " (" + __DATE__ +", " + __TIME__ + ")");
  DEBUG_PRINTLN("(c) 2020-2021 Showband Les Armourins");
  DEBUG_PRINTLN();
  DEBUG_PRINTLN("Board: " + String(ARDUINO_BOARD));

  if (masterMode) {
    DEBUG_PRINTLN("Mode: master");
  } else {
    DEBUG_PRINTLN("Mode: slave");
  }

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

  TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
  TIMERG0.wdt_feed=1;
  TIMERG0.wdt_wprotect=0;

  // Start the TaskUpdateLight if needed 
  if (TaskUpdateLightHandle == NULL) {
    DEBUG_PRINTLN("Start TaskUpdateLightHandle");
    xTaskCreatePinnedToCore(
                      TaskUpdateLight,        // Task function.
                      "TaskUpdateLight",      // name of task.
                      10000,                  // Stack size of task
                      NULL,                   // parameter of the task
                      1,                      // priority of the task
                      &TaskUpdateLightHandle, // Task handle to keep track of created task
                      1);                     // Do the TaskLed job on the separate core 1
  }

}


/// LOOP /// LOOP /// LOOP /// LOOP ///
void loop() {

  // STATE_START // STATE_START // STATE_START // STATE_START //

  // Initialization
  if (state == STATE_START) {

    // Activate Wifi client mode
    WiFi.mode(WIFI_STA);
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    WiFi.setSleep(false);
    esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("WiFi MAC address: " + String(WiFi.macAddress()));
  
    // Reset Wifi detection if encoder button is pressed during boot
    #ifdef ROTARY_ENCODER_A_PIN
      rotaryEncoder.begin();
      rotaryEncoder.setup(readEncoderISR);
      rotaryEncoder.disableAcceleration();
      if (rotaryEncoder.isEncoderButtonDown()) {
        DEBUG_PRINTLN("Reset Wifi detection");
        mlstools.config.ssid1validated = 0;
        mlstools.config.ssid2validated = 0;
      }
    #endif
    
    state = STATE_WIFI_SCAN;
    wifiStep = WIFI_SCAN_START;
    stateStartTime = micros();
    ssidScanStartTime = stateStartTime;
  }

  // STATE_WIFI_SCAN // STATE_WIFI_SCAN // STATE_WIFI_SCAN // STATE_WIFI_SCAN //

  // Scan Wifi, with internal steps
  if (state == STATE_WIFI_SCAN) {
    // WIFI_SCAN_START // WIFI_SCAN_START // WIFI_SCAN_START //
    if (wifiStep == WIFI_SCAN_START) {
      wifiStep = WIFI_SCAN_SSID1;
      ssidLastTrialTime = micros();

      if (strcmp(mlstools.config.ssid1, "") != 0) {
        strcpy(mlsota.ssid,   mlstools.config.ssid1);
        strcpy(mlsota.secret, mlstools.config.secret1);
        DEBUG_PRINTLN("Try to connect Wifi AP with SSID " + String(mlsota.ssid));
        ssidLastTrialTime = micros();
        WiFi.begin(mlsota.ssid, mlsota.secret);
      } else {
        ssidLastTrialTime = ssidLastTrialTime - SSID_TRIAL_MAX_TIME;
      }
    }

    if ((micros() - ssidLastTrialTime) >= SSID_TRIAL_MAX_TIME) {
      WiFi.disconnect();
      ssidLastTrialTime = micros();
      if (wifiStep == WIFI_SCAN_DEFAULT) {
        DEBUG_PRINTLN("No Wifi connection detected");
        state = STATE_WIFI_FINISHED;
      } else if (wifiStep == WIFI_SCAN_SSID2) {
        if (mlstools.useDefaultSsid()) {
          wifiStep = WIFI_SCAN_DEFAULT;
          strcpy(mlsota.ssid,   DEFAULT_WIFI_SSID);
          strcpy(mlsota.secret, DEFAULT_WIFI_SECRET);
          DEBUG_PRINTLN("Try to connect Wifi AP with SSID " + String(mlsota.ssid));
          WiFi.begin(mlsota.ssid, mlsota.secret);
        } else {
          DEBUG_PRINTLN("No Wifi connection detected");
          state = STATE_WIFI_FINISHED;
        }
      } else if (wifiStep == WIFI_SCAN_SSID1) {
        wifiStep = WIFI_SCAN_SSID2;
        if (strcmp(mlstools.config.ssid2, "") != 0) {
          strcpy(mlsota.ssid,   mlstools.config.ssid2);
          strcpy(mlsota.secret, mlstools.config.secret2);
          DEBUG_PRINTLN("Try to connect Wifi AP with SSID " + String(mlsota.ssid));
          WiFi.begin(mlsota.ssid, mlsota.secret);
        } else {
          ssidLastTrialTime = ssidLastTrialTime - SSID_TRIAL_MAX_TIME;
        }
      }
    }

    if ((micros() - ssidLastTrialTime) >= SSID_REPEAT_TRIAL_TIME) {
      if (WiFi.status() == WL_CONNECTED) {
        DEBUG_PRINTF("Device connected to AP after %d seconds\n",((micros()-ssidScanStartTime)/1000000));
        state = STATE_WIFI_CONNECTED;
        stateStartTime = micros();
      }
    }
  }

  // STATE_WIFI_CONNECTED // STATE_WIFI_CONNECTED // STATE_WIFI_CONNECTED // STATE_WIFI_CONNECTED //
  
  if (state == STATE_WIFI_CONNECTED) {
      FastLED.setBrightness(LED_CONFIG_BRIGHTNESS);
      mlslighteffects.stopUpdate();
      delay(10);
      mlslighteffects.fillAll(CRGB::Green);
      FastLED.show();
      mlslighteffects.setLightData(millis(), (LIGHT_PACKET){EFFECT_FLASH, MODIFIER_REPEAT, millis(), 300, 0, 255, 0, 12, 5, 12, 0, 255, 0, 12, 5, 12}); // GREEN/GREEN FLASH (wave) 300
    if (wifiStep == WIFI_SCAN_SSID1) {
      mlstools.config.ssid1validated = 1;
    } else if (wifiStep == WIFI_SCAN_SSID2) {
      mlstools.config.ssid2validated = 1;
    }
    DEBUG_PRINTLN("Wifi SSID connected: " + String(mlsota.ssid));
    DEBUG_PRINT("Wifi IP address: ");
    DEBUG_PRINTLN(WiFi.localIP());
    DEBUG_PRINTLN();
    #ifdef ARDUINO_TTGO_LoRa32_v21new
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.setFont(ArialMT_Plain_10);
      display.drawString(0, 53, "Wifi OK");
      display.display();
    #endif

    result = mlsota.checkOtaUpdates(mlstools.config.iid);
    if (result || forceFirmwareUpdate) {
      DEBUG_PRINTLN("OTA update will be started");
      if (forceFirmwareUpdate) {
        DEBUG_PRINTLN("Firmware update forced");
        /*
        if (TaskUpdateLightHandle != NULL) {
          DEBUG_PRINTLN("Stop TaskUpdateLightHandle");
          vTaskDelete(TaskUpdateLightHandle);
          TaskUpdateLightHandle = NULL;
        }
        */
      }
      #ifdef ARDUINO_TTGO_LoRa32_v21new
        display.clear();
        display.setTextAlignment(TEXT_ALIGN_CENTER);
        display.setFont(ArialMT_Plain_16);
        display.drawString(64, 23, "- Firmware update -");
        display.display();
      #endif
      FastLED.setBrightness(LED_CONFIG_BRIGHTNESS);
      mlslighteffects.fillAll(CRGB::Blue);
      FastLED.show();
      mlslighteffects.setLightData(millis(), (LIGHT_PACKET){EFFECT_PROGRESS4, MODIFIER_REPEAT, millis(), 600, 0, 0, 255, 45, 10, 45, 0, 0, 255, 45, 10, 45}); // BLUE/BLUE PROGRESS4 600
      mlsota.otaUpdates();
      mlslighteffects.fillAll(CRGB::Black);
      FastLED.show();
      mlslighteffects.stopUpdate();
    }
    mlstools.importConfiguration(mlsota.otaDownloadOptions(mlstools.config));

    state = STATE_WIFI_FINISHED;
    stateStartTime = micros();
  }

  if (state < STATE_WIFI_FINISHED) {
    // This is to skip the Wifi search by pressing the encoder button
    #ifdef ROTARY_ENCODER_A_PIN
      if (rotaryEncoder.isEncoderButtonClicked()) {
        DEBUG_PRINTLN("Skip Wifi sequence");
        state = STATE_WIFI_FINISHED;
      }
    #endif
    flashEnable = ((millis() % 400) < 200);
    if (flashLastState != flashEnable) {
      flashLastState = flashEnable;
      if (flashEnable) {
        #ifdef ONBOARD_LED
          digitalWrite(ONBOARD_LED, LED_BUILDIN_ON);
        #endif
      } else {
        #ifdef ONBOARD_LED
          digitalWrite(ONBOARD_LED, LED_BUILDIN_OFF);
        #endif
      }
    }
    // FastLED.setBrightness(LED_TEST_BRIGHTNESS - ((millis() / (5 * (255 / LED_TEST_BRIGHTNESS))) % (LED_TEST_BRIGHTNESS - 16)));
    // FastLED.show();
  }

  // STATE_WIFI_FINISHED // STATE_WIFI_FINISHED // STATE_WIFI_FINISHED // STATE_WIFI_FINISHED //

  if (state == STATE_WIFI_FINISHED) {
    DEBUG_PRINTLN();
    #ifdef ARDUINO_TTGO_LoRa32_v21new
      display.setTextAlignment(TEXT_ALIGN_RIGHT);
      display.setFont(ArialMT_Plain_10);
      display.drawString(127, 0, mlstools.config.iid);
      display.display();
    #endif
    
    #ifdef ROTARY_ENCODER_A_PIN
      rotaryEncoder.setBoundaries(0, 4, false);
      rotaryEncoder.setEncoderValue(mlstools.config.column);
      mlslighteffects.stopUpdate();
      delay(10);
      FastLED.setBrightness(LED_CONFIG_BRIGHTNESS);
      mlslighteffects.fillRight(CRGB::Blue);
      mlslighteffects.setValueLeft(rotaryEncoder.readEncoder(), 4, FADED_BLUE, FADED_BLUE, CRGB::Green, CRGB::Red);
      FastLED.show();
    #endif

    WiFi.disconnect();

    state = STATE_CONFIG;
    configStep = CONFIG_COLUMN;
    stateStartTime = micros();
  }

  if (state > STATE_WIFI_FINISHED) {
    forceFirmwareUpdate = forceFirmwareUpdate || (0 == digitalRead(0));
    if (forceFirmwareUpdate) {
      mlstools.config.ssid1validated = 0;
      mlstools.config.ssid2validated = 0;
      FastLED.setBrightness(LED_CONFIG_BRIGHTNESS);
      mlslighteffects.fillAll(DARK_ORANGE);
      FastLED.show();
      mlslighteffects.setLightData(millis(), (LIGHT_PACKET){EFFECT_PROGRESS, MODIFIER_FLIP_FLOP, millis(), 300, 255, 140, 0, 45, 10, 45, 255, 140, 0, 45, 10, 45}); // DARK_ORANGE
      DEBUG_PRINTLN();
      DEBUG_PRINTLN("Force firmware update");
      state = STATE_START;
    }
  }

  // STATE_CONFIG // STATE_CONFIG // STATE_CONFIG // STATE_CONFIG //

  // Setup options
  if (state == STATE_CONFIG) {

    // CONFIG_COLUMN // CONFIG_COLUMN // CONFIG_COLUMN // CONFIG_COLUMN //
    if (configStep == CONFIG_COLUMN) {
      #ifdef ROTARY_ENCODER_A_PIN
        if (rotaryEncoder.encoderChanged()) {
          DEBUG_PRINTLN("Column: " + String(rotaryEncoder.readEncoder()));
          mlslighteffects.setValueLeft(rotaryEncoder.readEncoder(), 4, FADED_BLUE, FADED_BLUE, CRGB::Green, CRGB::Red);
          FastLED.show();
        }
        if (rotaryEncoder.isEncoderButtonClicked()) {
          DEBUG_PRINTLN("Column selected: " + String(rotaryEncoder.readEncoder()));
          mlstools.config.column = rotaryEncoder.readEncoder();
          #ifdef ROTARY_ENCODER_A_PIN
            rotaryEncoder.setBoundaries(0, NUM_LEDS_PER_STRIP, false);
            rotaryEncoder.setEncoderValue(mlstools.config.rank);
            mlslighteffects.setValueThreeLeft(rotaryEncoder.readEncoder(), NUM_LEDS_PER_STRIP, FADED_BLUE, CRGB::White, CRGB::Green, CRGB::Red);
            FastLED.show();
          #endif
          configStep = CONFIG_RANK;
        }
      #endif
      // CONFIG_TIMEOUT // CONFIG_TIMEOUT // CONFIG_TIMEOUT // CONFIG_TIMEOUT //
      if ((micros() - stateStartTime) >= CONFIG_MAX_TIME) {
        DEBUG_PRINTLN("Config skipped after timeout");
        stateStartTime = micros();
        state = STATE_CONFIG_DONE;
      }
    }

    // CONFIG_RANK // CONFIG_RANK // CONFIG_RANK // CONFIG_RANK //
    if (configStep == CONFIG_RANK) {
      #ifdef ROTARY_ENCODER_A_PIN
        if (rotaryEncoder.encoderChanged()) {
          DEBUG_PRINTLN("Rank: " + String(rotaryEncoder.readEncoder()));
          mlslighteffects.setValueThreeLeft(rotaryEncoder.readEncoder(), NUM_LEDS_PER_STRIP, FADED_BLUE, CRGB::White, CRGB::Green, CRGB::Red);
          FastLED.show();
        }
        if (rotaryEncoder.isEncoderButtonClicked()) {
          DEBUG_PRINTLN("Rank selected: " + String(rotaryEncoder.readEncoder()));
          mlstools.config.rank = rotaryEncoder.readEncoder();
          state = STATE_CONFIG_DONE;
        }
      #endif
    }
  }

  // STATE_CONFIG_DONE // STATE_CONFIG_DONE // STATE_CONFIG_DONE // STATE_CONFIG_DONE //

  // Save options
  if (state == STATE_CONFIG_DONE) {
    mlslighteffects.stopUpdate();
    delay(10);
    FastLED.clear();
    FastLED.show();
    if (mlstools.saveConfiguration()) {
      if (WiFi.status() == WL_CONNECTED) {
        tempString = mlsota.otaDownloadOptions(mlstools.config);
      }
    }
    if (WiFi.status() == WL_CONNECTED) {
      WiFi.disconnect();
    }
    /*
    #ifdef ROTARY_ENCODER_A_PIN
      detachInterrupt(digitalPinToInterrupt(ROTARY_ENCODER_A_PIN));
      detachInterrupt(digitalPinToInterrupt(ROTARY_ENCODER_B_PIN));
    #endif
    */
    state = STATE_SUBSCRIBE;

  // STATE_SUBSCRIBE // STATE_SUBSCRIBE // STATE_SUBSCRIBE // STATE_SUBSCRIBE //


    runningBrightness = LED_MAX_BRIGHTNESS;
    #ifdef ROTARY_ENCODER_A_PIN
      uint8_t min_brightness = (LED_MIN_BRIGHTNESS + 1)/16;
      if (min_brightness < 1) {
        min_brightness = 1;
      }
      rotaryEncoder.setBoundaries(min_brightness, (LED_MAX_BRIGHTNESS + 1)/16, false);
      rotaryEncoder.setEncoderValue((runningBrightness + 1) / 16);
    #endif
    FastLED.setBrightness(runningBrightness);

    state = STATE_RUNNING;
  }

  // STATE_RUNNING // STATE_RUNNING // STATE_RUNNING // STATE_RUNNING //

  if (state == STATE_RUNNING) {


    #ifdef ROTARY_ENCODER_A_PIN
      if (rotaryEncoder.encoderChanged()) {
        runningBrightness = ((rotaryEncoder.readEncoder() * 16) - 1);
        FastLED.setBrightness(runningBrightness);
      }
    #endif

    if (((millis() / 2000) % 10) == 0) {
      if (simulatorEffect != EFFECT_PROGRESS) {
        simulatorEffect = EFFECT_PROGRESS;
        mlslighteffects.setLightData(millis(), (LIGHT_PACKET){EFFECT_PROGRESS, MODIFIER_FLIP_FLOP, millis(), 300, 0, 0, 255, 45, 10, 45, 0, 255, 0, 45, 10, 45}); // BLUE/GREEN
      }
    } else if (((millis() / 2000) % 10) == 2) {
      if (simulatorEffect != EFFECT_STROBE) {
        simulatorEffect = EFFECT_STROBE;
        mlslighteffects.setLightData(millis(), (LIGHT_PACKET){EFFECT_STROBE, 0, millis(), 100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
      }
    } else if (((millis() / 2000) % 10) == 4) {
      if (simulatorEffect != EFFECT_FLASH) {
        simulatorEffect = EFFECT_FLASH;
        mlslighteffects.setLightData(simulatorBeat, (LIGHT_PACKET){EFFECT_FLASH, MODIFIER_REPEAT, simulatorBeat, 500, 255, 255, 0, 0, 3, 45, 255, 255, 0, 0, 3, 45}); // YELLOW LONG FLASH REPEAT
      }
    } else if (((millis() / 2000) % 10) == 6) {
      if (simulatorEffect != EFFECT_FLASH) {
        simulatorEffect = EFFECT_FLASH;
        mlslighteffects.setLightData(millis(), (LIGHT_PACKET){EFFECT_PROGRESS_RAINBOW, MODIFIER_REPEAT, millis(), 500, 0, 255, 0, 12, 5, 12, 0, 255, 0, 12, 5, 12}); // RAINBOW 500
      }
    } else {
        simulatorEffect = EFFECT_NONE;
    }
    
    if (simulatorEffect == EFFECT_NONE) {
      uint32_t simulatorBeat = (millis() / simulatorBeatSpeed);
      if (simulatorLastBeat != simulatorBeat) {
        simulatorLastBeat = simulatorBeat;
        DEBUG_PRINT("Simulated beat packet: ");
        DEBUG_PRINT(simulatorBeat);
        DEBUG_PRINT(" uxTaskGetStackHighWaterMark: ");
        DEBUG_PRINTLN(uxTaskGetStackHighWaterMark(TaskUpdateLightHandle));
        //if (0 == (simulatorBeat %2 )) {
          mlslighteffects.setLightData(simulatorBeat, (LIGHT_PACKET){EFFECT_FLASH, MODIFIER_FLIP_FLOP, simulatorBeat, 0, 255, 0, 0, 0, 3, 35, 0, 255, 0, 0, 3, 35}); // RED/GREEN FLASH FLIP-FLOP
        //} else {
        //}
      }
    }
  }

  yield();

}
