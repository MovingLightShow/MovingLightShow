/**********************************************************************
 * 
 * MovingLightShow package - Synchronized LED strips for musicians
 * https://MovingLightShow.art - contact@movinglightshow.art
* (c) 2020-2021 Showband Les Armourins
 * 
 * @file  MovingLightShow.ino
 * @brief Main file of the MovingLightShow package
 * 
 * MovingLightShow can be used by up to 200 musicians simultaneously. Some light
 *  effects are synchronized with the bass drum, and some others are just visual effects
 *  with specific timing. Effects are programmed to works in 4 and 6 columns layout.
 *  6 columns layout position is calculated automagically from the 4 columns layout,
 *  based on the Armourins' practice.
 * 
 * The MovingLightShow concept is based on the following elements:
 * - ESP32 boards with ESP-Now support
 * - ESP32 boards with LoRa support and OLED display (for remote control)
 * - Rotary Digital Encoder Push Button (like EC11) for the setup
 * - Intelligent LED strips (like WS2812B)
 * - I2S MEMS microphone (like INMP441)
 * - piezo vibration sensor module (like 52Pi)
 * - custom PCB for easier mass production
 * - Bluetooth LE connection to select the effects on the master or from the remote
 * - Remote controler through LoRa
 * - PWA with web Bluetooth API for effects selection (compatible with Chrome on Android or Bluefy on iOS)
 * - 5V powerbank for each musician, with at least 25 Wh of energy (10'000 mAh)
 * - MLSmesh proprietary interconnection protocol using ESP-Now,
 *   with dynamic repeaters selection based on RSSI analysis (12 repeaters)
 * - automatic OTA (over-the-air) firmware upgrade during boot,
 *   if a known access point is available
 * 
 * The Drum major is always the sender ID 1 and must be set-up on rank 0.
 * The Bass Drum instrument is always the master, with the sender ID 0.
 * The device with ID 0 is for the instrument only, with dedicated LED strips.
 * The bass drum player must have a separate device like any other players.
 *
 * This package is the result of a *LOT* of work. If you are happy using this
 * package, contact us for a donation to support this project.
 * 
 * !!!!!!!!!!!!!!!!!!!! WARNING !!!!!!!!!!!!!!!!!!!!!!!
 * ! PIN 12 must NEVER be at Vcc during boot sequence !
 * ! ADC2 pins cannot be used when Wi-Fi is used      !
 * !!!!!!!!!!!!!!!!!!!! WARNING !!!!!!!!!!!!!!!!!!!!!!!
 * 
 * @author    Andre Liechti, MovingLightShow.art / Showband Les Armourins <contact@movinglightshow.art>
 * @version   1.0.4.0
 * @date      2021-11-10
 * @since     2021-01-01
 * @copyright (c) 2020-2021 Showband Les Armourins, Neuchatel, Switzerland
 * @copyright GNU Lesser General Public License
 * 
 * Board manager : https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 * Compatible boards (using esp32 by Espressif Systems version 1.0.6)
 * - ESP32 Arduino / TTGO LoRa32-OLED v2.1.6 ESP32 PICO-D4 (ttgo-lora32-v21new)
 *   (board definition adjusted in boards.txt, PartitionScheme added for OTA support)
 * - ESP32 Arduino / ESP32 Dev Module (esp32)
 * 
 * ESP32 Partition organization:
 * - Minimal SPIFFS (1.9MB APP with OTA/190KB SPIFFS)
 * 
 * Please check MovingLightShow.h for option definitions
 * 
 * Changes log:
 *   2021-11-09 1.0.4.0 [ENH] Disable sleep mode before lights activity (less latency)
 *                      [ENH] ESPNOW remote actions (force update and reboot)
 *   2021-11-08 1.0.3.6 [ENH] LoRa remote control integration
 *                      [ENH] PWA remote control
 *   2021-10-30 1.0.2.0 [ENH] Bluetooth LE integration
 *   2021-10-24 1.0.1.5 [ENH] Centralized reboot of all clients
 *                      [ENH] ESPNOW integration and optimization
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
const String ACTUAL_FIRMWARE = "1.0.4.0";

// https://github.com/FastLED/FastLED

// https://arduinojson.org/

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
  #include "mls_lora.h"
#endif

#include <Wire.h>

#include <esp_now.h>
#include <WiFi.h>
#include "esp_wifi.h"
#if defined(I2S_WS_PIN) && defined(I2S_SCK_PIN) && defined(I2S_SD_PIN)
  #include "driver/i2s.h"
#endif

#include "MovingLightShow.h"
#include "mls_tools.h"
#include "mls_ota.h"
#include "mls_mesh.h"
#include "mls_light_effects.h"

#ifdef BLE_SERVER
  #include <BLEDevice.h>
  #include <BLEServer.h>
  #include <BLEUtils.h>
  #include <BLE2902.h>
  
  BLEServer *pServer = NULL;
  BLECharacteristic * pTxCharacteristic;

  bool deviceConnected = false;
  bool oldDeviceConnected = false;
  uint8_t txValue = 0;
  
  class bleServerCallbacks: public BLEServerCallbacks {
      void onConnect(BLEServer* pServer) {
        deviceConnected = true;
      };
  
      void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
      }
  };
  
  class bleCallbacks: public BLECharacteristicCallbacks {
      void onWrite(BLECharacteristic *pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();
  
        if (rxValue.length() > 0) {
          DEBUG_PRINTLN("*********");
          DEBUG_PRINT("BLE: received Value: ");
          for (int i = 0; i < rxValue.length(); i++) {
            bleParams[i] = rxValue[i];
            DEBUG_PRINT(rxValue[i]);
          }
          bleParams[rxValue.length()] = (char) 0;
          DEBUG_PRINTLN();
          DEBUG_PRINTLN("*********");

          #ifdef ARDUINO_TTGO_LoRa32_v21new
            #ifdef LORA_BAND
              // send packet
              LoRa.beginPacket();
              LoRa.print(bleParams);
              LoRa.endPacket();
            #endif
          #endif
        }
      }
  };
#endif

/// Classes instantiation /// Classes instantiation /// Classes instantiation /// Classes instantiation ///
MlsOta mlsota(OTA_URL, ACTUAL_FIRMWARE);
MlsTools mlstools;
MlsLightEffects mlslighteffects(NUM_LEDS_PER_STRIP, leftLeds, rightLeds);

AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);

#ifdef ARDUINO_TTGO_LoRa32_v21new
  SSD1306 display(0x3c, SDA, SCL);

  #ifdef LORA_BAND
    // callback when LoRa data is received
    void loraCbk(int packetSize) {
      lora_packet = "";
      lora_packSize = String(packetSize, DEC);
      for (int i = 0; i < packetSize; i++) {lora_packet += (char) LoRa.read(); }
      lora_rssi = String(LoRa.packetRssi(), DEC);
      // display.clear();
      // display.setTextAlignment(TEXT_ALIGN_LEFT);
      // display.setFont(ArialMT_Plain_10);
      // display.drawStringMaxWidth(0 , 26 , 128, lora_packet);
      // display.drawString(0, 0, lora_rssi); 
      // display.display();
      loraCommand = 1;
      DEBUG_PRINT("LoRa: loraCbk packSize: ");
      DEBUG_PRINTLN(lora_packSize);
      DEBUG_PRINT("LoRa: loraCbk packet: ");
      DEBUG_PRINTLN(lora_packet);
      DEBUG_PRINT("LoRa: loraCbk RSSI: ");
      DEBUG_PRINTLN(lora_rssi);
    }
  #endif
#endif


void IRAM_ATTR readEncoderISR() {
  rotaryEncoder.readEncoder_ISR();
}


void promiscuous_rx_cb(void *buf, wifi_promiscuous_pkt_type_t type) {
  // All espnow traffic uses action frames which are a subtype of the mgmnt frames so filter out everything else.
  if (type != WIFI_PKT_MGMT)
      return;

  static const uint8_t ACTION_SUBTYPE = 0xd0;
  static const uint8_t ESPRESSIF_OUI[] = {0x18, 0xfe, 0x34};

  typedef struct {
    unsigned frame_ctrl:16;
    unsigned duration_id:16;
    uint8_t addr1[6]; /* receiver address */
    uint8_t addr2[6]; /* sender address */
    uint8_t addr3[6]; /* filtering address */
    unsigned sequence_ctrl:16;
    union {
      uint8_t addr4[6]; /* optional */
      struct {
        uint8_t ctrl;
        uint8_t oui[3];
        uint16_t type;
      };
    };
  } wifi_ieee80211_mac_hdr_2t;

  typedef struct {
    wifi_ieee80211_mac_hdr_2t hdr;
    uint8_t payload[200]; /* network data ended with 4 bytes csum (CRC32) */
  } wifi_ieee80211_packet_2t;

  const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buf;
  const wifi_ieee80211_packet_2t *ipkt = (wifi_ieee80211_packet_2t *)ppkt->payload;
  const wifi_ieee80211_mac_hdr_2t *hdr = &ipkt->hdr;


  // Only continue processing if this is an action frame containing the Espressif OUI.
  if ((ACTION_SUBTYPE == (hdr->frame_ctrl & 0xFF)) &&
      (memcmp(hdr->oui, ESPRESSIF_OUI, 3) == 0)) {

      int8_t rssi = ppkt->rx_ctrl.rssi;

    char macStr[18];

    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
    hdr->addr2[0], hdr->addr2[1], hdr->addr2[2], hdr->addr2[3], hdr->addr2[4], hdr->addr2[5]);

/*
    for (uint8_t i = 0; i < 100; i++) {
      bool equal = true;
      for (int j=0; j<6; j++) {
        if (idMac[i][j] != hdr->addr2[j]) {
          equal = false;
        }
      }
      if (equal) {
        // real near : -17 - pretty far : -85 (wifi : -30dBm à -90dBm) - limite basse : -80
        rssiArray[i] = rssi;
        DEBUG_PRINT("RSSI level: ");
        DEBUG_PRINTLN(rssi);
        break;
      }
    }
*/
    DEBUG_PRINT("promiscuous_rx_cb running on core ");
    DEBUG_PRINTLN(xPortGetCoreID());
  
    DEBUG_PRINT("Sender RSSI: ");
    DEBUG_PRINTLN(macStr);

    DEBUG_PRINT("RSSI level: ");
    DEBUG_PRINTLN(rssi);

    //for (int i = 0; i <= 199; i++) {
    //  Serial.print(char(ppkt->payload[i]));
    //}
  }
}


boolean sendMeshPacket(const uint8_t packetType, const uint16_t packetId, const uint8_t *data) {
  bool sendResult = false;
  struct MLS_PACKET mls_packet;
  memcpy(mls_packet.IID, mlstools.config.iid, 3);
  mls_packet.TYPE = packetType;
  mls_packet.PACKET_ID = packetId;
  memcpy(mls_packet.DATA, data, MLS_DATA_SIZE);
  esp_err_t result = esp_now_send(espnowBroadcastAddress, (uint8_t *) &mls_packet.raw, MLS_PACKET_SIZE);
  sendResult = (result == ESP_OK);
  #ifdef DEBUG_MLS
    if (result != ESP_OK) {
      DEBUG_PRINT("ESPNOW: Error sending data of type ");
      DEBUG_PRINTLN(packetType);
    }
  #endif
  return sendResult;
}


// callback when ESPNOW data is received
void OnEspNowDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {

    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    DEBUG_PRINT("ESPNOW: OnEspNowDataRecv packet of ");
    DEBUG_PRINT(len);
    DEBUG_PRINT(" bytes received from: ");
    DEBUG_PRINT(macStr);
    DEBUG_PRINT(", time: ");
    DEBUG_PRINT(micros()/1000);
    DEBUG_PRINTLN("ms");
  
  // TODO enhance the packet detection
  if (len != MLS_PACKET_SIZE) {
    DEBUG_PRINTLN("ESPNOW: wrong packet size");
  } else if (memcmp(mlstools.config.iid, incomingData, 3) != 0) {
    DEBUG_PRINTLN("ESPNOW: packet received is not a valid MLS packet (bad header)");
} else {
    struct MLS_PACKET mls_received_packet;
    memcpy(mls_received_packet.raw, incomingData, MLS_PACKET_SIZE);
    DEBUG_PRINT("ESPNOW: packet received is a valid MLS packet of type ");
    DEBUG_PRINTLN(mls_received_packet.TYPE);

    // LIGHT DATA and ACK LIGHT DATA
    if ((MLS_TYPE_LIGHT_DATA == mls_received_packet.TYPE) || (MLS_TYPE_ACK_LIGHT_DATA == mls_received_packet.TYPE)) {
      struct LIGHT_PACKET receivedLightPacket;
      if (!MLS_masterMode) {
        memcpy(receivedLightPacket.raw, mls_received_packet.DATA, LIGHT_PACKET_SIZE);
        mlslighteffects.setLightData(millis(), receivedLightPacket); // Max latency: 39000, not used yet.
      }
    } else if (MLS_TYPE_ACTION_DATA == mls_received_packet.TYPE) {
      struct ACTION_PACKET receivedActionPacket;
      memcpy(receivedActionPacket.raw, mls_received_packet.DATA, ACTION_PACKET_SIZE);
      if (receivedActionPacket.action == MLS_ACTION_FORCE_UPDATE) {
        DEBUG_PRINTLN("ESPNOW: Force firmware updated received");
        forceFirmwareUpdate = true;
      } else if (receivedActionPacket.action == MLS_ACTION_REBOOT) {
        ESP.restart();
      }
    }
  }
}

 
// Callback when ESPNOW data is sent
void OnEspNowDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  #ifdef DEBUG_MLS
    if (status == ESP_NOW_SEND_SUCCESS) {
      DEBUG_PRINTLN("ESPNOW: Delivery Success");
    } else {
      DEBUG_PRINTLN("ESPNOW: Delivery Fail");
    }
  #endif
}


/// PERMANENT TASK /// PERMANENT TASK /// PERMANENT TASK /// PERMANENT TASK ///
void TaskUpdateLight( void * pvParameters ){
  while(true) {
    mlslighteffects.updateLight();
    yield();
    delayMicroseconds(50); // Changed from 100 microseconds to 50 microseconds (2021-11-09)
  }
}


/// SETUP /// SETUP /// SETUP /// SETUP  ///
void setup() {

  #ifdef DEBUG_MLS
    // Define the integrated serial port speed for debug monitoring
    Serial.begin(115200);
  #endif
  DEBUG_PRINTLN();
  DEBUG_PRINTLN("============================================================");
  DEBUG_PRINTLN();
  DEBUG_PRINTLN("MovingLightShow.art " + ACTUAL_FIRMWARE + " (" + __DATE__ +", " + __TIME__ + ")");
  DEBUG_PRINTLN("(c) 2020-2021 Showband Les Armourins, Neuchatel, Switzerland");
  DEBUG_PRINTLN("contact@movinglightshow.art");
  DEBUG_PRINTLN();
  DEBUG_PRINTLN("============================================================");
  DEBUG_PRINTLN();
  DEBUG_PRINT("Light packet size: ");
  DEBUG_PRINTLN(LIGHT_PACKET_SIZE);
  DEBUG_PRINT("MLS packet size: ");
  DEBUG_PRINTLN(MLS_PACKET_SIZE);

  #ifdef ARDUINO_ESP32_DEV
    pinMode(0, INPUT); // on board boot button, used to force firmware update
  #endif

  #if MASTER_PIN > 0
    pinMode(MASTER_PIN, INPUT_PULLUP);
  #endif

  #ifdef ONBOARD_LED
    pinMode(ONBOARD_LED, OUTPUT);
  #endif

  FastLED.addLeds<LED_TYPE, LEFT_LEDS_PIN, LED_COLOR_ORDER>(leftLeds, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<LED_TYPE, RIGHT_LEDS_PIN, LED_COLOR_ORDER>(rightLeds, NUM_LEDS_PER_STRIP);
  FastLED.clear();

  FastLED.setBrightness(LED_TEST_BRIGHTNESS);

  // Initialize SPIFFS
  mlstools.spiffs_init();

  // Load non-volatile configuration
  mlstools.loadConfiguration(INITIAL_IID);

  #ifdef ARDUINO_TTGO_LoRa32_v21new
    // Initialize the 128x64 OLED display using Wire library

    display.init();
    display.flipScreenVertically();
    display.clear();
    display.drawXbm(0, 0, logo_width, logo_height, logo_bits);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, ACTUAL_FIRMWARE);
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(127, 47, "MLS");
    display.drawString(126, 46, "MLS");
    display.display();

    #ifdef LORA_BAND
      DEBUG_PRINTLN("SETUP: LoRa: begin initialization");
      SPI.begin(LORA_SCK_PIN,LORA_MISO_PIN,LORA_MOSI_PIN,LORA_SS_PIN);
DEBUG_PRINTLN("SETUP: LoRa: DEBUG 1");
      LoRa.setPins(LORA_SS_PIN,LORA_RST_PIN,LORA_DI0_PIN);  
DEBUG_PRINTLN("SETUP: LoRa: DEBUG 2");
      LoRa.beginPacket(true); // must be implicit header mode for SF6
DEBUG_PRINTLN("SETUP: LoRa: DEBUG 3");
      delay(1000);
      LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR);
DEBUG_PRINTLN("SETUP: LoRa: DEBUG 4");
      LoRa.setTxPower(LORA_TX_POWER, PA_OUTPUT_PA_BOOST_PIN);
DEBUG_PRINTLN("SETUP: LoRa: DEBUG 5");
      LoRa.setSignalBandwidth(LORA_SIGNAL_BANDWIDTH);
DEBUG_PRINTLN("SETUP: LoRa: DEBUG 6");
      LoRa.setCodingRate4(LORA_CODING_RATE_4);
DEBUG_PRINTLN("SETUP: LoRa: DEBUG 7");
      LoRa.setPreambleLength(LORA_PREAMBLE_LENGTH);
DEBUG_PRINTLN("SETUP: LoRa: DEBUG 8");
      LoRa.disableCrc(); // disable is default
DEBUG_PRINTLN("SETUP: LoRa: DEBUG 9");
      LoRa.endPacket(true); //Async mode enabled (default is false)      
DEBUG_PRINTLN("SETUP: LoRa: DEBUG 10");
      if (!LoRa.begin(LORA_BAND)) {
        LoraIsUp = false;
        DEBUG_PRINTLN("SETUP: LoRa: ERROR! Begin failed!");
      } else {
        LoraIsUp = true;
        LoRa.onReceive(loraCbk);
        LoRa.receive(); // Receiver mode activated
        DEBUG_PRINTLN("SETUP: LoRa: successful initialization");
      }
    #endif

  #endif

  TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
  TIMERG0.wdt_feed=1;
  TIMERG0.wdt_wprotect=0;

  // Start the TaskUpdateLight if needed 
  if (TaskUpdateLightHandle == NULL) {
    DEBUG_PRINTLN("SETUP: Start TaskUpdateLightHandle");
    xTaskCreatePinnedToCore(
                      TaskUpdateLight,        // Task function.
                      "TaskUpdateLight",      // name of task.
                      10000,                  // Stack size of task
                      NULL,                   // parameter of the task
                      1,                      // priority of the task
                      &TaskUpdateLightHandle, // Task handle to keep track of created task
                      1);                     // Do the TaskLed job on the separate core 1
  }
  #ifdef DEBUG_MLS
    // Additional tests at boot
    DEBUG_PRINTLN("PROGRESS4 test for 2 seconds");
    mlslighteffects.setLightData(millis(), (LIGHT_PACKET){EFFECT_PROGRESS4, MODIFIER_REPEAT + MODIFIER_FLIP_FLOP, millis(), 400, 0, 255, 255, 0, 45, 10, 45, 0, 255, 255, 45, 10, 45});
    delay(2000);
    DEBUG_PRINTLN("PROGRESS test for 2 seconds");
    mlslighteffects.setLightData(millis(), (LIGHT_PACKET){EFFECT_PROGRESS, MODIFIER_FLIP_FLOP, millis(), 100, 0, 255, 140, 0, 45, 10, 45, 255, 140, 0, 45, 10, 45});
    delay(2000);
    DEBUG_PRINTLN("STROBE test for 1 seconds");
    mlslighteffects.setLightData(millis(), (LIGHT_PACKET){EFFECT_STROBE, 0, millis(), 100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
    delay(1000);
    mlslighteffects.setLightData(millis(), (LIGHT_PACKET){EFFECT_PROGRESS_RAINBOW, MODIFIER_REPEAT, millis(), 1500, 0, 0, 255, 0, 12, 5, 12, 0, 255, 0, 12, 5, 12}); // RAINBOW 1500
  #endif

  delay(1000);

  #ifdef FORCE_SLAVE
    MLS_masterMode = false;
  #else
    #ifdef FORCE_MASTER
      MLS_masterMode = true;
    #else
      if (MASTER_PIN > 0) {  
        MLS_masterMode = (0 == digitalRead(MASTER_PIN));
	    }
    #endif
  #endif

  DEBUG_PRINTLN();
  DEBUG_PRINTLN("Board: " + String(ARDUINO_BOARD));
  
  if (MLS_masterMode) {
    DEBUG_PRINTLN("Mode: MASTER");
    #ifdef ARDUINO_TTGO_LoRa32_v21new
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.setFont(ArialMT_Plain_10);
      display.drawString(0, 26, "Master");
      display.display();
    #endif
  } else {
    DEBUG_PRINTLN("Mode: SLAVE");
    #ifdef ARDUINO_TTGO_LoRa32_v21new
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.setFont(ArialMT_Plain_10);
      display.drawString(0, 26, "Slave");
      display.display();
    #endif
  }
  
  if (MLS_masterMode) {
    #if defined(I2S_WS_PIN) && defined(I2S_SCK_PIN) && defined(I2S_SD_PIN)
      if ((I2S_WS_PIN > 0) && (I2S_SCK_PIN > 0) && (I2S_SD_PIN > 0)) {

        // I2S initialization // I2S initialization // I2S initialization // I2S initialization //
        esp_err_t i2s_err;

        const i2s_config_t i2s_config = {
            .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), // Receive, not transfer
            .sample_rate = I2S_SAMPLE_RATE,                    // 16KHz -> 32Khz
            .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // could only get it to work with 32bits
            .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,  // use left channel
            .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,     // Interrupt level 1
            .dma_buf_count = I2S_BUFFERS,                 // number of buffers
            .dma_buf_len = I2S_BLOCK_SIZE                 // samples per buffer (minimum 8)
        };
      
        const i2s_pin_config_t pin_config = {
            .bck_io_num   = I2S_SCK_PIN,       // Serial Clock (SCK)
            .ws_io_num    = I2S_WS_PIN,        // Word Select (WS)
            .data_out_num = I2S_PIN_NO_CHANGE, // not used
            .data_in_num  = I2S_SD_PIN         // Serial Data (SD)
        };
      
        DEBUG_PRINTLN("SETUP: I2S: Try to install I2S driver.");
        i2s_err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
        if (i2s_err != ESP_OK) {
          i2sEnabled = false;
          DEBUG_PRINTF("SETUP: I2S: Failed installing driver: %d\n", i2s_err);
        } else {
          i2s_err = i2s_set_pin(I2S_PORT, &pin_config);
          if (i2s_err != ESP_OK) {
            i2sEnabled = false;
            DEBUG_PRINTF("SETUP: I2S: Failed setting pin: %d\n", i2s_err);
          } else {
            i2sEnabled = true;
            DEBUG_PRINTLN("SETUP: I2S: driver installed.");
          }
        }
      }
    #endif
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

    #ifdef BLE_SERVER
      // To allow BLE in the same time, we need to allow WiFi sleep (was WiFi.setSleep(false); before, and esp_wifi_set_ps(WIFI_PS_NONE))
      // WiFi.setSleep(false);
      ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_MIN_MODEM));
    #else
      WiFi.setSleep(false);
    #endif

    esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
    DEBUG_PRINTLN();
    DEBUG_PRINTLN("LOOP: STATE_START: WiFi MAC address: " + String(WiFi.macAddress()));

    // Reset Wifi detection if encoder button is pressed during boot
    #ifdef ROTARY_ENCODER_A_PIN
      rotaryEncoder.begin();
      rotaryEncoder.setup(readEncoderISR);
      rotaryEncoder.disableAcceleration();
      if (rotaryEncoder.isEncoderButtonDown()) {
        DEBUG_PRINTLN("LOOP: STATE_START: Reset Wifi detection");
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
        DEBUG_PRINTLN("LOOP: STATE_WIFI_SCAN: Try to connect Wifi AP with SSID " + String(mlsota.ssid));
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
        DEBUG_PRINTLN("LOOP: STATE_WIFI_SCAN: No Wifi connection detected");
        state = STATE_WIFI_FINISHED;
      } else if (wifiStep == WIFI_SCAN_SSID2) {
        if (mlstools.useDefaultSsid()) {
          wifiStep = WIFI_SCAN_DEFAULT;
          strcpy(mlsota.ssid,   DEFAULT_WIFI_SSID);
          strcpy(mlsota.secret, DEFAULT_WIFI_SECRET);
          DEBUG_PRINTLN("LOOP: STATE_WIFI_SCAN: Try to connect Wifi AP with SSID " + String(mlsota.ssid));
          WiFi.begin(mlsota.ssid, mlsota.secret);
        } else {
          DEBUG_PRINTLN("LOOP: STATE_WIFI_SCAN: No Wifi connection detected");
          state = STATE_WIFI_FINISHED;
          forceFirmwareUpdateTrial++;
          if (forceFirmwareUpdateTrial > MAX_UPDATE_FIRMWARE_TRIALS) {
            forceFirmwareUpdate = false; // To avoid infinite loop if Wifi is not present
          }
        }
      } else if (wifiStep == WIFI_SCAN_SSID1) {
        wifiStep = WIFI_SCAN_SSID2;
        if (strcmp(mlstools.config.ssid2, "") != 0) {
          strcpy(mlsota.ssid,   mlstools.config.ssid2);
          strcpy(mlsota.secret, mlstools.config.secret2);
          DEBUG_PRINTLN("LOOP: STATE_WIFI_SCAN: Try to connect Wifi AP with SSID " + String(mlsota.ssid));
          WiFi.begin(mlsota.ssid, mlsota.secret);
        } else {
          ssidLastTrialTime = ssidLastTrialTime - SSID_TRIAL_MAX_TIME;
        }
      }
    }

    if ((micros() - ssidLastTrialTime) >= SSID_REPEAT_TRIAL_TIME) {
      if (WiFi.status() == WL_CONNECTED) {
        DEBUG_PRINTF("LOOP: STATE_WIFI_SCAN: Device connected to AP after %d seconds\n",((micros()-ssidScanStartTime)/1000000));
        state = STATE_WIFI_CONNECTED;
        stateStartTime = micros();
      }
    }
  }

  // STATE_WIFI_CONNECTED // STATE_WIFI_CONNECTED // STATE_WIFI_CONNECTED // STATE_WIFI_CONNECTED //
  
  if (state == STATE_WIFI_CONNECTED) {
      if (MLS_masterMode) {
        mlstools.config.master = 1;
      } else {
        mlstools.config.master = 0;
      }
      FastLED.setBrightness(LED_CONFIG_BRIGHTNESS);
      mlslighteffects.stopUpdate();
      delay(10);
      mlslighteffects.fillAll(CRGB::Green);
      FastLED.show();
      mlslighteffects.setLightData(millis(), (LIGHT_PACKET){EFFECT_FLASH, MODIFIER_REPEAT, millis(), 300, 0, 0, 255, 0, 12, 5, 12, 0, 255, 0, 12, 5, 12}); // GREEN/GREEN FLASH (wave) 300
    if (wifiStep == WIFI_SCAN_SSID1) {
      mlstools.config.ssid1validated = 1;
    } else if (wifiStep == WIFI_SCAN_SSID2) {
      mlstools.config.ssid2validated = 1;
    }
    DEBUG_PRINTLN("LOOP: STATE_WIFI_CONNECTED: Wifi SSID connected: " + String(mlsota.ssid));
    DEBUG_PRINTLN();
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
      DEBUG_PRINTLN("LOOP: STATE_WIFI_CONNECTED: OTA update will be started");
      if (forceFirmwareUpdate) {
        DEBUG_PRINTLN("LOOP: STATE_WIFI_CONNECTED: Firmware update forced");
        if (MLS_masterMode) {
          DEBUG_PRINTLN("LOOP: STATE_WIFI_CONNECTED: Firmware update forced dispatched to other devices");
          action_packet.action = MLS_ACTION_FORCE_UPDATE;
          boolean sendResult = sendMeshPacket(MLS_TYPE_ACTION_DATA, millis(), (uint8_t *) &action_packet);
        }
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
      mlslighteffects.setLightData(millis(), (LIGHT_PACKET){EFFECT_PROGRESS4, MODIFIER_REPEAT, millis(), 600, 0, 0, 0, 255, 45, 10, 45, 0, 0, 255, 45, 10, 45}); // BLUE/BLUE PROGRESS4 600
      delay(1000);
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
        DEBUG_PRINTLN("LOOP: <STATE_WIFI_FINISHED: Skip Wifi sequence");
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

    MLS_remoteControl = mlstools.config.remote;

    // No TaskUpdateLight for the remote control
    if (MLS_remoteControl && (TaskUpdateLightHandle != NULL)) {
      DEBUG_PRINTLN("Stop TaskUpdateLightHandle");
      vTaskDelete(TaskUpdateLightHandle);
      TaskUpdateLightHandle = NULL;
    }

    #ifdef ARDUINO_TTGO_LoRa32_v21new
    if (LoraIsUp) {
      if (MLS_remoteControl) {
        DEBUG_PRINTLN("LOOP: STATE_WIFI_FINISHED: Remote control: ACTIVATED");
        display.setTextAlignment(TEXT_ALIGN_RIGHT);
        display.setFont(ArialMT_Plain_10);
        display.drawString(127, 26, "LoRa RC");
        display.display();
      } else {
        DEBUG_PRINTLN("LOOP: STATE_WIFI_FINISHED: Remote control: RECEIVER ONLY");
        display.setTextAlignment(TEXT_ALIGN_RIGHT);
        display.setFont(ArialMT_Plain_10);
        display.drawString(127, 26, "LoRa OK");
        display.display();
      }
    }
  #endif
    
    #ifdef ARDUINO_TTGO_LoRa32_v21new
      display.setTextAlignment(TEXT_ALIGN_RIGHT);
      display.setFont(ArialMT_Plain_10);
      display.drawString(127, 0, mlstools.config.uniqueid);
      display.display();
    #endif
    
    #ifdef ROTARY_ENCODER_A_PIN
      rotaryEncoder.setBoundaries(0, 4, false);
      rotaryEncoder.setEncoderValue(mlstools.config.column);
      mlslighteffects.stopUpdate();
      delay(10);
      FastLED.setBrightness(LED_CONFIG_BRIGHTNESS);
      mlslighteffects.setValueAll(rotaryEncoder.readEncoder(), 4, MLS_FADED_BLUE, MLS_FADED_BLUE, CRGB::Green, CRGB::Red);
      FastLED.show();
    #endif

    WiFi.disconnect();

    #ifdef BLE_SERVER
      // Create the BLE Device
      char ble_service_name[80];
      strcpy(ble_service_name, BLE_PREFIX_NAME);
      strcat(ble_service_name, " ");
      strcat(ble_service_name, mlstools.config.uniqueid);
      BLEDevice::init(ble_service_name);

      // Create the BLE Server
      pServer = BLEDevice::createServer();
      pServer->setCallbacks(new bleServerCallbacks());
    
      // Create the BLE Service
      BLEService *pService = pServer->createService(SERVICE_UUID);
    
      // Create a BLE Characteristic
      pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
                          
      pTxCharacteristic->addDescriptor(new BLE2902());
    
      BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                           CHARACTERISTIC_UUID_RX,
                          BLECharacteristic::PROPERTY_WRITE
                        );
    
      pRxCharacteristic->setCallbacks(new bleCallbacks());
    
      // Start the service
      pService->start();
    
      // Start advertising
      pServer->getAdvertising()->start();
      DEBUG_PRINT("LOOP: STATE_WIFI_FINISHED: BLE server enabled: ");
      DEBUG_PRINTLN(BLE_PREFIX_NAME);
      DEBUG_PRINTLN("LOOP: STATE_WIFI_FINISHED: Waiting a BLE client connection to notify...");
    #endif

    state = STATE_CONFIG;
    configStep = CONFIG_COLUMN;
    stateStartTime = micros();
  }

  if (state > STATE_WIFI_FINISHED) {
    #ifdef ARDUINO_ESP32_DEV
      forceFirmwareUpdate = forceFirmwareUpdate || (0 == digitalRead(0));
    #endif
    if (forceFirmwareUpdate) {
      mlstools.config.ssid1validated = 0;
      mlstools.config.ssid2validated = 0;
      FastLED.setBrightness(LED_CONFIG_BRIGHTNESS);
      mlslighteffects.fillAll(MLS_DARK_ORANGE);
      FastLED.show();
      mlslighteffects.setLightData(millis(), (LIGHT_PACKET){EFFECT_PROGRESS, MODIFIER_FLIP_FLOP, millis(), 300, 0, 255, 140, 0, 45, 10, 45, 255, 140, 0, 45, 10, 45}); // MLS_DARK_ORANGE
      DEBUG_PRINTLN();
      DEBUG_PRINTLN("LOOP: STATE_WIFI_FINISHED: Force firmware update");
      state = STATE_START;
    }
  }

  // STATE_CONFIG // STATE_CONFIG // STATE_CONFIG // STATE_CONFIG //

  // No config state yet for remote control
  if (MLS_remoteControl && (state == STATE_CONFIG)) {
    state = STATE_CONFIG_DONE;
  }

  // Setup options
  if (state == STATE_CONFIG) {

    #ifdef SKIP_CONFIG
      state = STATE_CONFIG_DONE;
    #endif

    // CONFIG_COLUMN // CONFIG_COLUMN // CONFIG_COLUMN // CONFIG_COLUMN //
    if (configStep == CONFIG_COLUMN) {
      #ifdef ROTARY_ENCODER_A_PIN
        if (rotaryEncoder.encoderChanged()) {
          DEBUG_PRINTLN("Column: " + String(rotaryEncoder.readEncoder()));
          mlslighteffects.setValueAll(rotaryEncoder.readEncoder(), 4, MLS_FADED_BLUE, MLS_FADED_BLUE, CRGB::Green, CRGB::Red);
          configMaxTime = (micros() - stateStartTime) + CONFIG_MAX_TIME;
          FastLED.show();
        }
        if (rotaryEncoder.isEncoderButtonClicked()) {
          DEBUG_PRINTLN("Column selected: " + String(rotaryEncoder.readEncoder()));
          mlstools.config.column = rotaryEncoder.readEncoder();
          #ifdef ROTARY_ENCODER_A_PIN
            rotaryEncoder.setBoundaries(0, NUM_LEDS_PER_STRIP, false);
            rotaryEncoder.setEncoderValue(mlstools.config.rank);
            // Set already the next selection color
            mlslighteffects.setValueThreeLeft(rotaryEncoder.readEncoder(), NUM_LEDS_PER_STRIP, MLS_FADED_BLUE, MLS_WHITE192, CRGB::Green, MLS_RED224);
            FastLED.show();
          #endif
          configStep = CONFIG_RANK;
        }
      #endif
      // CONFIG_TIMEOUT // CONFIG_TIMEOUT // CONFIG_TIMEOUT // CONFIG_TIMEOUT //
      if ((micros() - stateStartTime) >= configMaxTime) {
        DEBUG_PRINTLN("LOOP: STATE_CONFIG: Config skipped after timeout");
        stateStartTime = micros();
        state = STATE_CONFIG_DONE;
      }
    }

    // CONFIG_RANK // CONFIG_RANK // CONFIG_RANK // CONFIG_RANK //
    if (configStep == CONFIG_RANK) {
      #ifdef ROTARY_ENCODER_A_PIN
        if (rotaryEncoder.encoderChanged()) {
          DEBUG_PRINTLN("Rank: " + String(rotaryEncoder.readEncoder()));
          mlslighteffects.setValueThreeLeft(rotaryEncoder.readEncoder(), NUM_LEDS_PER_STRIP, MLS_FADED_BLUE, MLS_WHITE192, CRGB::Green, MLS_RED224);
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

  if (state == STATE_CONFIG_DONE) {
    mlslighteffects.stopUpdate();
    delay(10);
    FastLED.clear();
    FastLED.show();
    // Save options
    if (mlstools.saveConfiguration()) {
      if (WiFi.status() == WL_CONNECTED) {
        tempString = mlsota.otaDownloadOptions(mlstools.config);
      }
    }

    if (!MLS_remoteControl) {
      #ifdef BLE_SERVER
        // De-init BLE server if needed
        if (BLEDevice::getInitialized() == true){
          DEBUG_PRINTLN("LOOP: STATE_SUBSCRIBE: BLE server disabled");
          BLEDevice::deinit(false);
        }
      #endif
      // Deactivate WiFi sleep for less latency in ESPNOW
      WiFi.setSleep(false);
      ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    }
  


    
    if (WiFi.status() == WL_CONNECTED) {
      WiFi.disconnect();
    }

    // This will desactivate the rotary encoder
    #ifdef ROTARY_ENCODER_A_PIN
      #ifndef ROTARY_CHANGE_BRIGHTNESS
        detachInterrupt(digitalPinToInterrupt(ROTARY_ENCODER_A_PIN));
        detachInterrupt(digitalPinToInterrupt(ROTARY_ENCODER_B_PIN));
      #endif
    #endif

    // No subscribe state yet for remote control
    if (MLS_remoteControl) {
      state = STATE_RUNNING;
    } else {
      state = STATE_SUBSCRIBE;
    }
  }

  // STATE_SUBSCRIBE // STATE_SUBSCRIBE // STATE_SUBSCRIBE // STATE_SUBSCRIBE //

  if (state == STATE_SUBSCRIBE) {
    if (lastState != state) {
      DEBUG_PRINTLN("STATE_SUBSCRIBE");
    }

    runningBrightness = LED_MAX_BRIGHTNESS;
    #ifdef ROTARY_ENCODER_A_PIN
      #ifdef ROTARY_CHANGE_BRIGHTNESS
        uint8_t min_brightness = (LED_MIN_BRIGHTNESS + 1)/16;
        if (min_brightness < 1) {
          min_brightness = 1;
        }
        rotaryEncoder.setBoundaries(min_brightness, (LED_MAX_BRIGHTNESS + 1)/16, false);
        rotaryEncoder.setEncoderValue((runningBrightness + 1) / 16);
      #endif
    #endif
    FastLED.setBrightness(runningBrightness);

    // Initializing ESPNOW for MLS

    WiFi.mode(WIFI_STA);
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B);

    // WiFi.setSleep(false);
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_MIN_MODEM)); // To allow BLE

    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);

    if (esp_now_init() != ESP_OK) {
      DEBUG_PRINTLN("LOOP: STATE_SUBSCRIBE: Error initializing ESP-NOW");
    } else {
      #ifdef DEBU_MLS
        esp_now_register_send_cb(OnEspNowDataSent);
      #endif
      esp_now_register_recv_cb(OnEspNowDataRecv);
  
      // register peer
      esp_now_peer_info_t peerInfo = {}; // peerInfo must be initialized, otherwise it doesn't always work ! (ESPNOW: Peer interface is invalid)
      peerInfo.channel = ESPNOW_CHANNEL;  
      peerInfo.encrypt = false;
    
      // register first peer  
      memcpy(peerInfo.peer_addr, espnowBroadcastAddress, 6);
      if (esp_now_add_peer(&peerInfo) != ESP_OK){
        DEBUG_PRINTLN("LOOP: STATE_SUBSCRIBE: ESPNOW: dailed to add peer");
      } else {
        DEBUG_PRINTLN("LOOP: STATE_SUBSCRIBE: ESPNOW: broadcast peer added");
      }
    }

    if (MLS_masterMode) {
      delay(1000);
      DEBUG_PRINTLN("LOOP: STATE_SUBSCRIBE: Send reboot request");
      action_packet.action = MLS_ACTION_REBOOT;
      boolean sendResult = sendMeshPacket(MLS_TYPE_ACTION_DATA, millis(), (uint8_t *) &action_packet);
    }

    state = STATE_RUNNING;
  }

  // STATE_RUNNING // STATE_RUNNING // STATE_RUNNING // STATE_RUNNING //

  if (state == STATE_RUNNING) {

    if (lastState != state) {
      DEBUG_PRINTLN("STATE_RUNNING");
    }

    #if defined(I2S_WS_PIN) && defined(I2S_SCK_PIN) && defined(I2S_SD_PIN)
      if (i2sEnabled) {
        // https://github.com/maspetsberger/esp32-i2s-mems/tree/master/examples/NoiseLevel
        // https://diyi0t.com/i2s-sound-tutorial-for-esp32/
        int32_t samples[I2S_BLOCK_SIZE];
        int32_t num_bytes_read = i2s_read_bytes(I2S_PORT, 
                                                (char *)samples, 
                                                I2S_BLOCK_SIZE, // the doc says bytes, but its elements.
                                                portMAX_DELAY); // no timeout
        if (num_bytes_read > 0) {
          int samples_read = num_bytes_read / 8;
          int32_t mean = 0;
          for (int i = 0; i < samples_read; ++i) {
            mean += (abs(samples[i]) / samples_read);
          }
          mean = abs(mean);
          // Ignore incorrect values
          if (mean > 0x10000000) {
            mean = 0x10000000;
          }
          if (mean < 1) {
            mean = 1;
          }
          
          if (mean > i2s_biggestInput) {
            i2s_biggestInput = mean;
          }

          i2s_rolling_mean[i2s_sampleCounter % I2S_ROLLING_MEAN_SIZE] = mean;
          // i2s_long_term_mean = ((I2S_ROLLING_MEAN_SIZE * i2s_long_term_mean) + mean) / (I2S_ROLLING_MEAN_SIZE + 1);
          i2s_long_term_mean = 0;
          for (int i = 0; i < I2S_ROLLING_MEAN_SIZE; ++i) {
            i2s_long_term_mean = i2s_long_term_mean + (i2s_rolling_mean[i] / I2S_ROLLING_MEAN_SIZE);
            if (i2s_rolling_mean[i] > i2s_rollingmaxInput) {
              i2s_rollingmaxInput = i2s_rolling_mean[i];
            }
          }

          if (mean > i2s_maxInput) {
            i2s_maxInput = mean;
            i2s_maxInputTime = micros();
            i2s_secondtMaxInput = 0;
          } else if (((micros() - i2s_maxInputTime) > (i2s_maxInputFlushTime / 2)) && (mean > i2s_secondtMaxInput)) {
            i2s_secondtMaxInput = mean;
          }
          if ((micros() - i2s_maxInputTime) > i2s_maxInputFlushTime) {
            if (i2s_secondtMaxInput < (0.5 * i2s_maxInput)) {
              i2s_secondtMaxInput = 0.5 * i2s_maxInput;
            } else {
              i2s_maxInput = i2s_secondtMaxInput;
            }
            i2s_maxInputTime = micros();
            i2s_secondtMaxInput = 0;
          } else if ((micros() - i2s_maxInputTime) > i2s_inputFlushTime) {
            if (i2s_secondtMaxInput > (0.5 * i2s_maxInput)) {
              i2s_maxInput = i2s_secondtMaxInput;
              i2s_maxInputTime = micros();
              i2s_secondtMaxInput = 0;
            }
          }

          if (i2s_maxInput < (i2s_biggestInput / 4)) {
            i2s_maxInput = i2s_biggestInput / 4;
          }

          if (mean < lastEdgeDectionLevel) {
            overLastEdgeDectionLevel = false;
          }

          // if ((((!overLastEdgeDectionLevel) && ((micros() - lastEdgeDectionTime) > minEdgeDetectionGap)) || ((micros() - lastEdgeDectionTime) > maxEdgeDetectionGap)) && (((mean / i2s_long_term_mean) > i2S_EDGE_RATIO_LONG_TERM) || (mean > (i2s_maxInput / i2S_EDGE_RATIO_INSTANT)))) {
          if ((((!overLastEdgeDectionLevel) && ((micros() - lastEdgeDectionTime) > minEdgeDetectionGap)) || ((micros() - lastEdgeDectionTime) > maxEdgeDetectionGap)) && ((i2s_rollingmaxInput / mean) < i2S_EDGE_ROLLING_MAX)) {

            lastEdgeDectionLevel = mean;
            overLastEdgeDectionLevel = true;
            lastEdgeDectionTime = micros();
            DEBUG_PRINT("BOOM :-), time (ms): ");
            DEBUG_PRINTLN(micros()/1000);
            light_packet = (LIGHT_PACKET){EFFECT_FLASH, MODIFIER_FLIP_FLOP, detectedBeatCounter, 0, 0, 255, 0, 0, 0, 3, 35, 0, 255, 0, 0, 3, 35}; // RED/GREEN FLASH FLIP-FLOP
            mlslighteffects.setLightData(detectedBeatCounter, light_packet);
            boolean sendResult = sendMeshPacket(MLS_TYPE_LIGHT_DATA, millis(), (uint8_t *) &light_packet);
            detectedBeatCounter++;
            /*
            DEBUG_PRINT("BOOM :-) ");
            DEBUG_PRINT(detectedBeatCounter);
            DEBUG_PRINT(" value: ");
            DEBUG_PRINT(mean);
            DEBUG_PRINT(", biggest: ");
            DEBUG_PRINT(i2s_biggestInput);
            DEBUG_PRINT(", max: ");
            DEBUG_PRINT(i2s_maxInput);
            DEBUG_PRINT(", 2max: ");
            DEBUG_PRINT(i2s_secondtMaxInput);
            DEBUG_PRINT(", long term mean: ");
            DEBUG_PRINT(i2s_long_term_mean);
            DEBUG_PRINT(", ratio: ");
            DEBUG_PRINTLN((mean / i2s_long_term_mean));
            */
          }
            // DEBUG_PRINTLN(mean);
          i2s_sampleCounter++;
        }
      }
    #endif

	#ifdef ROTARY_ENCODER_A_PIN
      #ifdef ROTARY_CHANGE_BRIGHTNESS
        if (rotaryEncoder.encoderChanged()) {
          runningBrightness = ((rotaryEncoder.readEncoder() * 16) - 1);
          FastLED.setBrightness(runningBrightness);
        }
      #endif
    #endif

/*
    if (((millis() / 2000) % 10) == 0) {
      if (simulatorEffect != EFFECT_PROGRESS) {
        simulatorEffect = EFFECT_PROGRESS;
        mlslighteffects.setLightData(millis(), (LIGHT_PACKET){EFFECT_PROGRESS, MODIFIER_FLIP_FLOP, millis(), 300, 0, 0, 0, 255, 45, 10, 45, 0, 255, 0, 45, 10, 45}); // BLUE/GREEN
      }
    } else if (((millis() / 2000) % 10) == 2) {
      if (simulatorEffect != EFFECT_STROBE) {
        simulatorEffect = EFFECT_STROBE;
        mlslighteffects.setLightData(millis(), (LIGHT_PACKET){EFFECT_STROBE, 0, millis(), 100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
      }
    } else if (((millis() / 2000) % 10) == 4) {
      if (simulatorEffect != EFFECT_FLASH) {
        simulatorEffect = EFFECT_FLASH;
        mlslighteffects.setLightData(simulatorBeat, (LIGHT_PACKET){EFFECT_FLASH, MODIFIER_REPEAT, simulatorBeat, 500, 0, 255, 255, 0, 0, 3, 45, 255, 255, 0, 0, 3, 45}); // YELLOW LONG FLASH REPEAT
      }
    } else if (((millis() / 2000) % 10) == 6) {
      if (simulatorEffect != EFFECT_FLASH) {
        simulatorEffect = EFFECT_FLASH;
        mlslighteffects.setLightData(millis(), (LIGHT_PACKET){EFFECT_PROGRESS_RAINBOW, MODIFIER_REPEAT, millis(), 500, 0, 0, 255, 0, 12, 5, 12, 0, 255, 0, 12, 5, 12}); // RAINBOW 500
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
          mlslighteffects.setLightData(simulatorBeat, (LIGHT_PACKET){EFFECT_FLASH, MODIFIER_FLIP_FLOP, simulatorBeat, 0, 0, 255, 0, 0, 0, 3, 35, 0, 255, 0, 0, 3, 35}); // RED/GREEN FLASH FLIP-FLOP
        //} else {
        //}
      }
    }
*/

    simulatorBeat = (millis() / simulatorBeatSpeed);

    #ifdef DEBUG_MLS
    /*
      if (simulatorLastBeat != simulatorBeat) {
        DEBUG_PRINT(" uxTaskGetStackHighWaterMark: ");
        DEBUG_PRINTLN(uxTaskGetStackHighWaterMark(TaskUpdateLightHandle));
      }
    */
    #endif

    #ifdef ARDUINO_TTGO_LoRa32_v21new
      if (0 != loraCommand) {
        light_packet = (LIGHT_PACKET){EFFECT_FLASH, MODIFIER_FLIP_FLOP, millis(), 0, 0, 255, 0, 0, 0, 3, 35, 0, 255, 0, 0, 3, 35}; // RED/GREEN FLASH FLIP-FLOP
        mlslighteffects.setLightData(millis(), light_packet);
        boolean sendResult = sendMeshPacket(MLS_TYPE_LIGHT_DATA, millis(), (uint8_t *) &light_packet);
        DEBUG_PRINT("LOOP: STATE_RUNNING: LoRa: loraCommand: ");
        DEBUG_PRINTLN(loraCommand);
        loraCommand = 0;
      }
    #endif

    // Bluetooth LE handling
    #ifdef BLE_SERVER
      if (MLS_remoteControl) {
        // BLE notification
        if (simulatorLastBeat != simulatorBeat) {
          if (deviceConnected) {
            char cstr[16];
            itoa(announced_devices + simulatorBeat, cstr, 10);
            // pTxCharacteristic->setValue(&simulatorBeat, 1);
            pTxCharacteristic->setValue((uint8_t*)&cstr, strlen(cstr));
            pTxCharacteristic->notify();
          }
        }

        if (!deviceConnected && oldDeviceConnected) {
          // disconnecting
          if (simulatorLastBeat != simulatorBeat) {
            pServer->startAdvertising(); // restart advertising
            DEBUG_PRINTLN("LOOP: STATE_RUNNING: BLE: start advertising");
            oldDeviceConnected = deviceConnected;
          }
        }
        if (deviceConnected && !oldDeviceConnected) {
        // connecting
        // do stuff here on connecting
          oldDeviceConnected = deviceConnected;
        }
      }
    #endif


    if (simulatorLastBeat != simulatorBeat) {
      simulatorLastBeat = simulatorBeat;
    }

    lastState = state;
  }

  yield();

}
