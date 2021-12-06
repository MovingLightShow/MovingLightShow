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
 * @version   1.1.1.1
 * @date      2021-11-26
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
 *   2021-11-26 1.1.1.1 [ENH] Back to basic MLS-MESH implementation for the show tonight
 *                      [ENH] LoRa Remote control additional commands
 *                      [ENH] LoRa packets are sent asynchronously
 *                      [ENH] Better update and startup handling
 *                      [ENH] Check, heartbeat and breath effects implemented
 *   2021-11-14 1.0.5.1 [FIX] Better Wifi handling
 *   2021-11-13 1.0.5.0 [FIX] MLS-MESH RSSI "compression" algorithm
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
const String ACTUAL_FIRMWARE = "1.1.1.1";

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

  boolean deviceConnected = false;
  boolean oldDeviceConnected = false;
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
        if (rxValue.length() > 3) {
          DEBUG_PRINTLN("*********");
          DEBUG_PRINT("BLE: received Value (length: ");
          DEBUG_PRINT(rxValue.length());
          DEBUG_PRINT("): ");
          for (int i = 0; i < rxValue.length(); i++) {
            bleParams[i] = rxValue[i];
            DEBUG_PRINT(rxValue[i]);
          }
          bleParams[rxValue.length()] = (char) 0;
          DEBUG_PRINTLN();

          if (memcmp(gIID, bleParams, 3) != 0) {
            DEBUG_PRINTLN("BLE: packet received is not a valid packet (bad header)");
          } else {
            DEBUG_PRINTLN("*********");
  
            #ifdef ARDUINO_TTGO_LoRa32_v21new
              #ifdef LORA_BAND
                // Send LoRa packet
                DEBUG_PRINTLN("BLE: BEGIN SENDING LORA PACKET");
                LoRa.beginPacket(LORA_IMPLICIT_HEADER);
                // LoRa.print(bleParams);
                // LoRa.write(bleParams, rxValue.length());
                for (int i = 0; i < rxValue.length(); i++) {
                  LoRa.write(rxValue[i]);
                }
                LoRa.endPacket(false); // Do NOT send asynchronously
                LoRa.receive();
                DEBUG_PRINTLN("BLE: END SENDING LORA PACKET");
              #endif
            #endif
          }
        } else {
          DEBUG_PRINT("BLE: packet received is not a valid packet (too small)");
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
    void lora_receive_cb(int packetSize) {
      char one_char;
      String loraInfo = "";
      uint8_t crc_received;
      lora_packet = "";
      loraCommand = "";
      lora_packSize = String(packetSize, DEC);

      // Packet size must be at least 7 bytes (IID + command + extension + CRC),but not more than 80
      if ((packetSize <= 80) && (packetSize >= 7)) {
        for (int i = 0; i < (packetSize-1); i++) {
          one_char = (char) LoRa.read();
          lora_packet += one_char;
          if ((i > 2) & (i < 6)) {
            loraCommand += one_char;
          } else if (i >= 6) {
            if (loraCommand.toInt() == EFFECT_FEEDBACK_INFO) {
              loraInfo += one_char;
            } else {
              loraExtended[i-6] = one_char;
            }
          }
        }

        if (memcmp(gIID, lora_packet.c_str(), 3) != 0) {
          DEBUG_PRINT("LoRa: invalid packet received");
        } else {
          DEBUG_PRINT("LoRa: loraCommand (length: "); DEBUG_PRINT(packetSize); DEBUG_PRINT("): "); DEBUG_PRINTLN(loraCommand);
  
          // CRC is the last char
          crc_received = (char) LoRa.read();
          
          lora_rssi = String(LoRa.packetRssi(), DEC);
    
          if (MLS_remoteControl) {
            strcpy(bleLastCmdInfo, loraInfo.c_str());
            DEBUG_PRINT("LoRa info for BLE: "); DEBUG_PRINTLN(bleLastCmdInfo);
          } else { // MLS_relay in the band
            // display.clear();
            // display.setTextAlignment(TEXT_ALIGN_LEFT);
            // display.setFont(ArialMT_Plain_10);
            // display.drawStringMaxWidth(0 , 26 , 128, lora_packet);
            // display.drawString(0, 0, lora_rssi); 
            // display.display();
    
            cmd_to_send_ts =  micros();
            cmd_to_send = loraCommand.toInt();
    
            switch(cmd_to_send) {
              case EFFECT_REBOOT:
                action_packet.action = MLS_ACTION_REBOOT;
                mlsmesh_send_packet(MLS_TYPE_ACTION_DATA, (uint8_t *) &action_packet);
                break;
              case 999:
                break;
              default:
                break;
            }
            /*
            if ((mls_received_packet.COMMAND != 0) && (mls_received_packet.COMMAND_PACKET_ID != LastLoraCommandPacketId)) {
              if (
            }
            */
      
            loraReceived = 1;
            DEBUG_PRINT("LoRa: lora_receive_cb packSize: "); DEBUG_PRINT(lora_packSize); DEBUG_PRINT("packet: "); DEBUG_PRINTLN(lora_packet);
            DEBUG_PRINT("LoRa: lora_receive_cb RSSI: "); DEBUG_PRINTLN(lora_rssi);
            DEBUG_PRINT("LoRa: loraCommand: "); DEBUG_PRINTLN(loraCommand);
          }
        }
      } // if (packetSize >= 7)
    }
  #endif
#endif


void IRAM_ATTR readEncoderISR() {
  rotaryEncoder.readEncoder_ISR();
}


void mlsmesh_promiscuous_rx_cb(void *buf, wifi_promiscuous_pkt_type_t type) {
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
      boolean equal = true;
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
    DEBUG_PRINT("mlsmesh_promiscuous_rx_cb running on core ");
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


boolean mlsmesh_send_packet(const uint8_t packetType, const uint8_t *data) {
  if (MLS_masterMode) {
    mlsmeshLastPackedId++;
  }
  mlsmeshLastPacketSentMs = millis();

  boolean sendResult = false;
  struct MLS_PACKET mls_packet;
  memcpy(mls_packet.IID, mlstools.config.iid, 3);
  mls_packet.TYPE = packetType;
  mls_packet.PACKET_ID = mlsmeshLastPackedId;
  mls_packet.SENDER_ID = my_device.id;
  mls_packet.NUMBER_OF_COLUMNS = mlslighteffects.getColumns();
  mls_packet.NUMBER_OF_RANKS = mlslighteffects.getRanks();
  mls_packet.COMMAND = lastCommand;
  mls_packet.COMMAND_SENDER_ID = lastCommandSenderId;
  mls_packet.COMMAND_PACKET_ID = lastCommandPacketId;

  memcpy(mls_packet.DATA, data, MLS_DATA_SIZE);
  esp_err_t result = esp_now_send(espnowBroadcastAddress, (uint8_t *) &mls_packet.raw, MLS_PACKET_SIZE);
  sendResult = (result == ESP_OK);
  if (sendResult) {
  } else {
    #ifdef DEBUG_MLS
      DEBUG_PRINT("ESPNOW: Error sending data of type ");
      DEBUG_PRINTLN(packetType);
    #endif
  }
  return sendResult;
}


// callback when ESPNOW data is received
void mlsmesh_receive_packet_cb(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
    uint8_t received_mac[6];
    boolean isNewCommand = false;

    memcpy(received_mac, mac_addr, 6);

    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    DEBUG_PRINT("ESPNOW: mlsmesh_receive_packet_cb packet of ");
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
    OneEspNowPacketReceived = true;
    struct MLS_PACKET mls_received_packet;
    memcpy(mls_received_packet.raw, incomingData, MLS_PACKET_SIZE);
    DEBUG_PRINT("ESPNOW: packet received is a valid MLS packet of type ");
    DEBUG_PRINTLN(mls_received_packet.TYPE);

    // Update the last command if it is a new one
    if ((mls_received_packet.COMMAND_PACKET_ID - lastCommandPacketId) > 0) {
      isNewCommand = true;
      lastCommand = mls_received_packet.COMMAND;
      lastCommandSenderId = mls_received_packet.COMMAND_SENDER_ID;
      lastCommandPacketId = mls_received_packet.COMMAND_PACKET_ID;
    }
    DEBUG_PRINT("Received ESPNOW PACKET_ID: ");
    DEBUG_PRINTLN(mls_received_packet.PACKET_ID);
    if ((mls_received_packet.PACKET_ID > mlsmeshLastPackedId) || (abs(mls_received_packet.PACKET_ID - mlsmeshLastPackedId) > 10000)) {
      mlsmeshLastPackedId = mls_received_packet.PACKET_ID;
    }

    struct LIGHT_PACKET receivedLightPacket;

    // LIGHT DATA
    if (MLS_TYPE_LIGHT_DATA == mls_received_packet.TYPE) {

      if (last_packet_played != mls_received_packet.PACKET_ID) {
        last_packet_played = mls_received_packet.PACKET_ID;

        // Extract number of columns and ranks
        if (!MLS_masterMode) {
          mlslighteffects.setColumns(mls_received_packet.NUMBER_OF_COLUMNS);
          mlslighteffects.setRanks(mls_received_packet.NUMBER_OF_RANKS);
        }
          
        if (!MLS_masterMode) {
          memcpy(receivedLightPacket.raw, mls_received_packet.DATA, LIGHT_PACKET_SIZE);
          #ifdef DEBUG_MLS
            DEBUG_PRINT("ESPNOW: MLS_TYPE_LIGHT_DATA MLS effect: ");
            DEBUG_PRINTLN(receivedLightPacket.effect);
            DEBUG_PRINT("LIGHT INFO: Left color (RGB): ");
            DEBUG_PRINT(receivedLightPacket.left_color_r);
            DEBUG_PRINT(" ");
            DEBUG_PRINT(receivedLightPacket.left_color_g);
            DEBUG_PRINT(" ");
            DEBUG_PRINTLN(receivedLightPacket.left_color_b);
            DEBUG_PRINT("LIGHT INFO: Left on time: ");
            DEBUG_PRINTLN(receivedLightPacket.left_on_time);
          #endif
          if (last_effect_played != current_beat_effect) {
            detectedBeatCounter = 0;
          }
          mlslighteffects.setLightData(millis(), &receivedLightPacket); // Max latency: 39000, not used yet.
          detectedBeatCounter++;
        }
      } // if (last_packet_played != mls_received_packet.PACKET_ID)

    // ACK LIGHT DATA
    } else if (MLS_TYPE_ACK_LIGHT_DATA == mls_received_packet.TYPE) {
      if (MLS_masterMode) {
        if (isNewCommand) {
          // Is the effect of the command synced with bass drum ?
          if ((lastCommand >= 100) && (lastCommand <= 199)) {
            current_beat_effect = lastCommand;
          } else {
            if (EFFECT_CHECK == lastCommand) {
              receivedLightPacket = (LIGHT_PACKET){lastCommand, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            } else {
              receivedLightPacket = (LIGHT_PACKET){lastCommand, 0, millis(), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            }
            boolean sendResult = mlsmesh_send_packet(MLS_TYPE_LIGHT_DATA, (uint8_t *) &receivedLightPacket);
            mlslighteffects.setLightData(millis(), &receivedLightPacket); // Max latency: 39000, not used yet.
            current_beat_effect = EFFECT_KEEP_ALIVE;
          }
        }
      }
    } else if (MLS_TYPE_ACTION_DATA == mls_received_packet.TYPE) {
      struct ACTION_PACKET receivedActionPacket;
      memcpy(receivedActionPacket.raw, mls_received_packet.DATA, ACTION_PACKET_SIZE);
      if (receivedActionPacket.action == MLS_ACTION_FORCE_UPDATE) {
        DEBUG_PRINTLN("ESPNOW: Force firmware updated received");
        forceFirmwareUpdate = true;
        forceFirmwareUpdateTrial = 0;
      } else if (receivedActionPacket.action == MLS_ACTION_REBOOT) {
        action_packet.action = MLS_ACTION_REBOOT;
        boolean sendResult = mlsmesh_send_packet(MLS_TYPE_ACTION_DATA, (uint8_t *) &action_packet);
        delay(500);
        ESP.restart();
      }
    } else if (MLS_TYPE_TOPOLOGY_DATA == mls_received_packet.TYPE) {
      struct TOPOLOGY_PACKET receivedTopologyPacket;
      uint8_t device_id;
      memcpy(receivedTopologyPacket.raw, mls_received_packet.DATA, ACTION_PACKET_SIZE);
      DEBUG_PRINT("receivedTopologyPacket.type: ");DEBUG_PRINTLN(receivedTopologyPacket.type);
      DEBUG_PRINT("receivedTopologyPacket.device_id: ");DEBUG_PRINTLN(receivedTopologyPacket.device_id);
      DEBUG_PRINT("receivedTopologyPacket.rank: ");DEBUG_PRINTLN(receivedTopologyPacket.rank);
      DEBUG_PRINT("receivedTopologyPacket.column: ");DEBUG_PRINTLN(receivedTopologyPacket.column);
      if ((MLS_masterMode) && (MLS_TOPOLOGY_REQUEST == receivedTopologyPacket.type)) {
        device_id = searchDevice(devices, announced_devices, received_mac);
        if (0xFF == device_id) {
          memcpy(devices[announced_devices].mac, receivedTopologyPacket.mac, 6);
          devices[announced_devices].rank   = receivedTopologyPacket.rank;
          devices[announced_devices].column = receivedTopologyPacket.column;
          announced_devices++;
          device_id = announced_devices; // We do that after ++ because the master as the id 0 !
          if (receivedTopologyPacket.column > mlslighteffects.getColumns()) {
            mlslighteffects.setColumns(receivedTopologyPacket.column);
          }
          if (receivedTopologyPacket.rank > mlslighteffects.getRanks()) {
            mlslighteffects.setRanks(receivedTopologyPacket.rank);
          }
        }
        // Send back the id info with the mac address;
        topology_packet.device_id = device_id;
        memcpy(topology_packet.mac, devices[device_id].mac, 6);
        topology_packet.type   = MLS_TOPOLOGY_REPLY;
        topology_packet.rank   = devices[device_id].rank;
        topology_packet.column = devices[device_id].column;
        boolean sendResult = mlsmesh_send_packet(MLS_TYPE_TOPOLOGY_DATA, (uint8_t *) &topology_packet);
      } else if ((!MLS_masterMode) && (MLS_TOPOLOGY_REPLY == receivedTopologyPacket.type)) {
        if (0xFF == my_device.id) {
          memcpy(devices[receivedTopologyPacket.device_id].mac, receivedTopologyPacket.mac, 6);
          devices[receivedTopologyPacket.device_id].rank   = receivedTopologyPacket.rank;
          devices[receivedTopologyPacket.device_id].column = receivedTopologyPacket.column;
          if (memcmp(my_device.mac, receivedTopologyPacket.mac, 6) == 0) { // It's for me :-)
            my_device.id = receivedTopologyPacket.device_id;
          }
        }
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

  #ifdef FORCE_FIRMWARE_UPDATE_PIN
    #if FORCE_FIRMWARE_UPDATE_PIN > -1
      pinMode(FORCE_FIRMWARE_UPDATE_PIN, INPUT); // on board boot button, used to force firmware update
      pinMode(FORCE_FIRMWARE_UPDATE_PIN, INPUT_PULLDOWN);
    #endif
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
DEBUG_PRINT("SETUP: LoRa: DEBUG 0");
      LoRa.setPins(LORA_SS_PIN,LORA_RST_PIN,LORA_DI0_PIN);  
DEBUG_PRINT(", 1");
      LoRa.setSyncWord(LORA_SYNC_WORD);
DEBUG_PRINT(", 2");
      LoRa.beginPacket(LORA_IMPLICIT_HEADER); // must be implicit header mode for SF6
DEBUG_PRINT(", 3");
      delay(1000);
      LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR);
DEBUG_PRINT(", 4");
      LoRa.setTxPower(LORA_TX_POWER, PA_OUTPUT_PA_BOOST_PIN);
DEBUG_PRINT(", 5");
      LoRa.setSignalBandwidth(LORA_SIGNAL_BANDWIDTH);
DEBUG_PRINT(", 6");
      LoRa.setCodingRate4(LORA_CODING_RATE_4);
DEBUG_PRINT(", 7");
      LoRa.setPreambleLength(LORA_PREAMBLE_LENGTH);
DEBUG_PRINT(", 8");
      #ifdef LORA_ENABLE_CRC
        LoRa.enableCrc();
      #else
        LoRa.disableCrc(); // disable is default
      #endif
DEBUG_PRINT(", 9");
      LoRa.endPacket(true); //Async mode enabled (default is false)      
DEBUG_PRINTLN(", 10");
      if (!LoRa.begin(LORA_BAND)) {
        LoraIsUp = false;
        DEBUG_PRINTLN("SETUP: LoRa: ERROR! Begin failed!");
      } else {
        LoraIsUp = true;
        LoRa.onReceive(lora_receive_cb);
        LoRa.receive(); // Receiver mode activated
        DEBUG_PRINTLN("SETUP: LoRa: successful initialization");
      }
    #endif

  #endif

  // Default is 0xFF (unregistered)
  my_device.id = 0xFF;

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

    struct LIGHT_PACKET test_light_packet;
    if (LIGHT_PACKET_SIZE != sizeof(test_light_packet.raw)) {
      DEBUG_PRINT("Light packet size error!");
      while(true);
    }

    struct MLS_PACKET test_mls_packet;
    if (MLS_PACKET_SIZE != sizeof(test_mls_packet.raw)) {
      DEBUG_PRINT("MLS packet size error!");
      while(true);
    }

    struct STRIP_DATA test_strip_data;
    if (STRIP_DATA_SIZE != sizeof(test_strip_data.raw)) {
      DEBUG_PRINT("Strip data size error!");
      while(true);
    }

    struct FLIP_DATA test_flip_data;
    if (FLIP_DATA_SIZE != sizeof(test_flip_data.raw)) {
      DEBUG_PRINT("Flip data size error!");
      while(true);
    }

    #ifdef ARDUINO_TTGO_LoRa32_v21new
      struct REMOTE_CONTROL_PACKET test_remote_control_packet;
      if (REMOTE_CONTROL_PACKET_SIZE != sizeof(test_remote_control_packet.raw)) {
        DEBUG_PRINT("Remote control packet size error!");
        while(true);
      }
    #endif

    struct ACTION_PACKET test_action_packet;
    if (ACTION_PACKET_SIZE != sizeof(test_action_packet.raw)) {
      DEBUG_PRINT("Action packet size error!");
      while(true);
    }

    // Some light effects at the beginning...
    /*
    DEBUG_PRINTLN("PROGRESS4 test for 2 seconds");
    light_packet = (LIGHT_PACKET){EFFECT_PROGRESS4, MODIFIER_REPEAT + MODIFIER_FLIP_FLOP, millis(), 400, 0, 255, 255, 0, 45, 10, 45, 0, 255, 255, 45, 10, 45};
    mlslighteffects.setLightData(millis(), &light_packet);
    delay(2000);
    DEBUG_PRINTLN("PROGRESS test for 2 seconds");
    light_packet = (LIGHT_PACKET){EFFECT_PROGRESS, MODIFIER_FLIP_FLOP, millis(), 100, 0, 255, 140, 0, 45, 10, 45, 255, 140, 0, 45, 10, 45};
    mlslighteffects.setLightData(millis(), &light_packet);
    delay(2000);
    DEBUG_PRINTLN("STROBE test for 1 seconds");
    light_packet = (LIGHT_PACKET){EFFECT_STROBE, 0, millis(), 120, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    mlslighteffects.setLightData(millis(), &light_packet);
    delay(1000);
    */
  #endif

  // Rainbow check for at least 1 second + WIFI connection time
  light_packet = (LIGHT_PACKET){EFFECT_PROGRESS_RAINBOW, MODIFIER_REPEAT, millis(), 1212, 0, 0, 255, 0, 12, 5, 12, 0, 255, 0, 12, 5, 12}; // RAINBOW 1212
  mlslighteffects.setLightData(millis(), &light_packet);
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
    my_device.id = 0;
    
    DEBUG_PRINTLN("Mode: Bass drum (ESPNOW master)");
    #ifdef ARDUINO_TTGO_LoRa32_v21new
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.setFont(ArialMT_Plain_10);
      display.drawString(0, 26, "Master");
      display.display();
    #endif
  } else {
    DEBUG_PRINTLN("Mode: Musician (ESPNOW slave)");
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

  // Initialize some randomness
  #ifdef RANDOM_INIT_PIN
    #if RANDOM_INIT_PIN > -1
      randomSeed(analogRead(RANDOM_INIT_PIN));
    #endif
  #endif
}


/// LOOP /// LOOP /// LOOP /// LOOP ///
void loop() {

  // Always : initial ESPNOW sync check in acceptable delay
  if ((!MLS_masterMode) && (!MLS_remoteControl) && (!OneEspNowPacketReceived) && (millis() > MLSMESH_MAX_MS_FIRST_PACKET)) {
    DEBUG_PRINT("WARNING: Device restarted , still no ESPNOW packet received ");
    DEBUG_PRINT(MLSMESH_MAX_MS_FIRST_PACKET);
    DEBUG_PRINTLN(" ms after boot");
    ESP.restart();
  }

  // Always : ESPNOW keep alive packet from the master if needed
  if ((MLS_masterMode) && ((millis() - mlsmeshLastPacketSentMs) > MLSMESH_MASTER_TIMEOUT_MS)) {
    mlsmeshLastPacketSentMs = mlsmeshLastPacketSentMs + 2000; // Set next trial in 2 seconds
    DEBUG_PRINTLN("LOOP: ESPNOW keep alive packet sent, because timeout check time is now over.");
    action_packet.action = MLS_ACTION_KEEP_ALIVE;
    boolean sendResult = mlsmesh_send_packet(MLS_TYPE_ACTION_DATA, (uint8_t *) &action_packet);
  }

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

    WiFi.macAddress(my_device.mac);
    if (MLS_masterMode) {
      announced_devices = 1;
      my_device.id = 0;
      WiFi.macAddress(devices[my_device.id].mac);
    } else {
      WiFi.macAddress(my_device.mac);
    }

    // Reset Wifi detection and force firmware update if encoder button is pressed before boot
    #ifdef ROTARY_ENCODER_A_PIN
      rotaryEncoder.begin();
      rotaryEncoder.setup(readEncoderISR);
      rotaryEncoder.disableAcceleration();
      if (rotaryEncoder.isEncoderButtonDown()) {
        DEBUG_PRINTLN("LOOP: STATE_START: Reset Wifi detection");
        mlstools.config.ssid1validated = 0;
        mlstools.config.ssid2validated = 0;
        forceFirmwareUpdate = true;
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
          if (forceFirmwareUpdateTrial > MAX_FORCE_FIRMWARE_TRIALS) {
            forceFirmwareUpdate = false; // To avoid infinite loop if Wifi is not present
          } else if (forceFirmwareUpdateTrial > 1) {
            delay(random(MIN_FORCE_FIRMWARE_WAIT_MS, MAX_FORCE_FIRMWARE_WAIT_MS));
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
      mlslighteffects.fill(CRGB::Green, NUM_LEDS_PER_STRIP, leftLeds);
      mlslighteffects.fill(CRGB::Green, NUM_LEDS_PER_STRIP, rightLeds);
      mlslighteffects.showLeds();
      light_packet = (LIGHT_PACKET){EFFECT_FLASH, MODIFIER_REPEAT, millis(), 300, 0, 0, 255, 0, 12, 5, 12, 0, 255, 0, 12, 5, 12}; // GREEN/GREEN FLASH (wave) 300
      mlslighteffects.setLightData(millis(), &light_packet);
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
          boolean sendResult = mlsmesh_send_packet(MLS_TYPE_ACTION_DATA, (uint8_t *) &action_packet);
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
      mlslighteffects.stopUpdate();
      delay(10);
      mlslighteffects.fill(CRGB::Blue, NUM_LEDS_PER_STRIP, leftLeds);
      mlslighteffects.fill(CRGB::Blue, NUM_LEDS_PER_STRIP, rightLeds);
      mlslighteffects.showLeds();
      light_packet = (LIGHT_PACKET){EFFECT_PROGRESS4, MODIFIER_REPEAT, millis(), 600, 0, 0, 0, 255, 45, 10, 45, 0, 0, 255, 45, 10, 45}; // BLUE/BLUE PROGRESS4 600
      mlslighteffects.setLightData(millis(), &light_packet);
      delay(1000);
      mlsota.otaUpdates();
      mlslighteffects.stopUpdate();
      delay(10);
      mlslighteffects.fill(CRGB::Black, NUM_LEDS_PER_STRIP, leftLeds);
      mlslighteffects.fill(CRGB::Black, NUM_LEDS_PER_STRIP, rightLeds);
      mlslighteffects.showLeds();
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
    // mlslighteffects.showLeds();
  }

  // STATE_WIFI_FINISHED // STATE_WIFI_FINISHED // STATE_WIFI_FINISHED // STATE_WIFI_FINISHED //

  if (state == STATE_WIFI_FINISHED) {
    DEBUG_PRINTLN();
    mlstools.saveConfiguration();
    mlslighteffects.stopUpdate();
    delay(100);
    FastLED.setBrightness(LED_CONFIG_BRIGHTNESS);

    memcpy(gIID, mlstools.config.iid, 3);
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
      display.setTextAlignment(TEXT_ALIGN_RIGHT);
      display.setFont(ArialMT_Plain_10);
      display.drawString(127, 0, mlstools.config.uniqueid);
      display.display();
    #endif
    
    #ifdef ROTARY_ENCODER_A_PIN
      mlslighteffects.stopUpdate();
      delay(10);
      rotaryEncoder.setBoundaries(0, 4, false);
      rotaryEncoder.setEncoderValue(mlstools.config.column);
      mlslighteffects.setValue(mlstools.config.column, INITIAL_COLUMNS, MLS_FADED_BLUE, MLS_FADED_BLUE, CRGB::Green, CRGB::Red, leftLeds);
      mlslighteffects.setValue(mlstools.config.column, INITIAL_COLUMNS, MLS_FADED_BLUE, MLS_FADED_BLUE, CRGB::Green, CRGB::Red, rightLeds);
      mlslighteffects.showLeds();
    #endif

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
    // This is not working anymore, depending how port 0 is used
      // forceFirmwareUpdate = forceFirmwareUpdate || (0 == digitalRead(FORCE_FIRMWARE_UPDATE_PIN));
    #endif
    if (forceFirmwareUpdate) {
      mlstools.config.ssid1validated = 0;
      mlstools.config.ssid2validated = 0;
      FastLED.setBrightness(LED_CONFIG_BRIGHTNESS);
      mlslighteffects.fill(MLS_DARK_ORANGE, NUM_LEDS_PER_STRIP, leftLeds);
      mlslighteffects.fill(MLS_DARK_ORANGE, NUM_LEDS_PER_STRIP, rightLeds);
      mlslighteffects.showLeds();
      light_packet = (LIGHT_PACKET){EFFECT_PROGRESS, MODIFIER_FLIP_FLOP, millis(), 300, 0, 255, 140, 0, 45, 10, 45, 255, 140, 0, 45, 10, 45}; // MLS_DARK_ORANGE
      mlslighteffects.setLightData(millis(), &light_packet);
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
          mlslighteffects.setValue(rotaryEncoder.readEncoder(), INITIAL_COLUMNS, MLS_FADED_BLUE, MLS_FADED_BLUE, CRGB::Green, CRGB::Red, leftLeds);
          mlslighteffects.setValue(rotaryEncoder.readEncoder(), INITIAL_COLUMNS, MLS_FADED_BLUE, MLS_FADED_BLUE, CRGB::Green, CRGB::Red, rightLeds);
          mlslighteffects.showLeds();
          #ifdef DEBUG_MLS
            configTimeOutTime = (micros() - stateStartTime) + CONFIG_TIMEOUT_TIME_DEBUG;
          #else
            configTimeOutTime = (micros() - stateStartTime) + CONFIG_TIMEOUT_TIME;
          #endif
        }
        if (rotaryEncoder.isEncoderButtonClicked()) {
          DEBUG_PRINTLN("Column selected: " + String(rotaryEncoder.readEncoder()));
          mlstools.config.column = rotaryEncoder.readEncoder();
          #ifdef ROTARY_ENCODER_A_PIN
            rotaryEncoder.setBoundaries(0, NUM_LEDS_PER_STRIP, false);
            rotaryEncoder.setEncoderValue(mlstools.config.rank);
            // Set already the next selection color
            mlslighteffects.setValueThree(mlstools.config.rank, NUM_LEDS_PER_STRIP, MLS_FADED_BLUE, MLS_WHITE192, CRGB::Green, MLS_RED224, leftLeds);
            mlslighteffects.showLeds();
          #endif
          configStep = CONFIG_RANK;
        }
      #endif
      // CONFIG_TIMEOUT // CONFIG_TIMEOUT // CONFIG_TIMEOUT // CONFIG_TIMEOUT //
      if ((micros() - stateStartTime) >= configTimeOutTime) {
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
          mlslighteffects.setValueThree(rotaryEncoder.readEncoder(), NUM_LEDS_PER_STRIP, MLS_FADED_BLUE, MLS_WHITE192, CRGB::Green, MLS_RED224, leftLeds);
          mlslighteffects.showLeds();
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
    mlslighteffects.setMyColumn(mlstools.config.column);
    mlslighteffects.setMyRank(mlstools.config.rank);
    mlslighteffects.stopUpdate();
    delay(10);
    mlslighteffects.clearLeds();
    mlslighteffects.showLeds();
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
          DEBUG_PRINTLN("LOOP: STATE_CONFIG_DONE: BLE server disabled");
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
      startStateRunningTS = millis();
    } else {
      state = STATE_SUBSCRIBE;
    }
  }

  // STATE_SUBSCRIBE // STATE_SUBSCRIBE // STATE_SUBSCRIBE // STATE_SUBSCRIBE //

  if (state == STATE_SUBSCRIBE) {
    if (lastState != state) {
      DEBUG_PRINTLN("STATE_SUBSCRIBE");

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
  
      // Initializing ESPNOW for MLS, except for remote control
      if (!MLS_remoteControl) {
        WiFi.mode(WIFI_STA);
        WiFi.setTxPower(WIFI_POWER_19_5dBm);
        esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_11B);
    
        // WiFi.setSleep(false);
        ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_MIN_MODEM)); // To allow BLE
    
        esp_wifi_set_promiscuous(true);
        esp_wifi_set_promiscuous_rx_cb(&mlsmesh_promiscuous_rx_cb);
    
        if (esp_now_init() != ESP_OK) {
          DEBUG_PRINTLN("LOOP: STATE_SUBSCRIBE: Error initializing ESP-NOW");
          ESP.restart();
        } else {
          #ifdef DEBU_MLS
            esp_now_register_send_cb(OnEspNowDataSent);
          #endif
          esp_now_register_recv_cb(mlsmesh_receive_packet_cb);
      
          // register peer
          esp_now_peer_info_t peerInfo = {}; // peerInfo must be initialized, otherwise it doesn't always work ! (ESPNOW: Peer interface is invalid)
          peerInfo.channel = MLSMESH_CHANNEL;  
          peerInfo.encrypt = false;
        
          // register first peer  
          memcpy(peerInfo.peer_addr, espnowBroadcastAddress, 6);
          if (esp_now_add_peer(&peerInfo) != ESP_OK){
            DEBUG_PRINTLN("LOOP: STATE_SUBSCRIBE: ESPNOW: refused to add peer");
            ESP.restart();
          } else {
            DEBUG_PRINTLN("LOOP: STATE_SUBSCRIBE: ESPNOW: broadcast peer added");
          }
        }
      }
  
      if (MLS_masterMode) {
        delay(1000);
        DEBUG_PRINTLN("LOOP: STATE_SUBSCRIBE: Send reboot request");
        action_packet.action = MLS_ACTION_REBOOT;
        boolean sendResult = mlsmesh_send_packet(MLS_TYPE_ACTION_DATA, (uint8_t *) &action_packet);
      }
  
      /*
      DEBUG_PRINTLN("CHECK test for 30 seconds");
      for (uint8_t i = 0; i < 30; i++) {
        light_packet = (LIGHT_PACKET){EFFECT_CHECK, 0, i, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        mlslighteffects.setLightData(millis(), &light_packet);
        delay(1000);
      }
      */

    }

    if (my_device.id != 0xFF) {
      DEBUG_PRINT("my_device.id: "); DEBUG_PRINT(my_device.id);
      state = STATE_RUNNING;
      startStateRunningTS = millis();
    } else {
      if ((millis() - lastSubscribeTimeMs) > SUBSCRIBE_RETRY_TIME_MS) {
        topology_packet.type = MLS_TOPOLOGY_REQUEST;
        topology_packet.device_id = my_device.id;
        memcpy(topology_packet.mac, my_device.mac, 6);
        topology_packet.rank = mlslighteffects.getMyRank();
        topology_packet.column = mlslighteffects.getMyColumn();
        sendResult = mlsmesh_send_packet(MLS_TYPE_TOPOLOGY_DATA, (uint8_t *) &topology_packet);
      }
      lastSubscribeTimeMs = millis();
    }
  }

  // STATE_RUNNING // STATE_RUNNING // STATE_RUNNING // STATE_RUNNING //

  if (state == STATE_RUNNING) {

    if (lastState != state) {
      DEBUG_PRINTLN("STATE_RUNNING");
    }

    #ifdef ARDUINO_TTGO_LoRa32_v21new
      if ((millis() - lastdisplayUpdateTime) > OLED_INFO_REFRESH_TIME) {
        uint32_t seconds = millis() / 1000;
        uint32_t minutes = seconds / 60;
        uint32_t hours = minutes / 60;
        display.setColor(BLACK);
        display.fillRect(0, 26, 50, 10);
        sprintf(tempStr, "%02d:%02d:%02d", hours%24, minutes%60, seconds%60);
        display.setColor(WHITE);
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_10);
        display.drawString(0, 26, tempStr);
        display.display();
        lastdisplayUpdateTime = millis();
      }
    #endif


    // Repeat last CHECK command
    if (MLS_masterMode) {
      if (EFFECT_CHECK == lastCommand) {
        if ((millis() - mlsmeshLastPacketSentMs) > CHECK_RESEND_TIME_MS) {
          checkCounter++;
          light_packet = (LIGHT_PACKET){lastCommand, 0, checkCounter, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
          boolean sendResult = mlsmesh_send_packet(MLS_TYPE_LIGHT_DATA, (uint8_t *) &light_packet);
        }
      } else {
        checkCounter = 0;
      }
    }

    #if defined(I2S_WS_PIN) && defined(I2S_SCK_PIN) && defined(I2S_SD_PIN)
      if (MLS_masterMode) {
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
  
            // if ((((!overLastEdgeDectionLevel) && ((micros() - lastEdgeDectionTime) > minEdgeDetectionGap)) || ((micros() - lastEdgeDectionTime) > maxEdgeDetectionGap)) && (((mean / i2s_long_term_mean) > I2S_EDGE_RATIO_LONG_TERM) || (mean > (i2s_maxInput / I2S_EDGE_RATIO_INSTANT)))) {
            if ((((!overLastEdgeDectionLevel) && ((micros() - lastEdgeDectionTime) > minEdgeDetectionGap)) || ((micros() - lastEdgeDectionTime) > maxEdgeDetectionGap)) && ((i2s_rollingmaxInput / mean) < I2S_EDGE_ROLLING_MAX)) {
  
              lastEdgeDectionLevel = mean;
              overLastEdgeDectionLevel = true;
              lastEdgeDectionTime = micros();
              DEBUG_PRINT("BOOM :-), time (ms): ");
              DEBUG_PRINT(micros()/1000);
              DEBUG_PRINT(", level: ");
              DEBUG_PRINTLN(mean);
              DEBUG_PRINT("detectedBeatCounter: ");
              DEBUG_PRINTLN(detectedBeatCounter);

              if (last_effect_played != current_beat_effect) {
                detectedBeatCounter = 0;
              }
              last_effect_played = current_beat_effect;
              light_packet = (LIGHT_PACKET){current_beat_effect, 0, detectedBeatCounter, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
              // light_packet = (LIGHT_PACKET){EFFECT_FLASH, MODIFIER_FLIP_FLOP, detectedBeatCounter, 0, 0, 255, 0, 0, 0, 3, 35, 0, 255, 0, 0, 3, 35}; // RED/GREEN FLASH FLIP-FLOP
              mlslighteffects.setLightData(detectedBeatCounter, &light_packet);
              boolean sendResult = mlsmesh_send_packet(MLS_TYPE_LIGHT_DATA, (uint8_t *) &light_packet);
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
      } // if (MLS_masterMode)
    #endif

	#ifdef ROTARY_ENCODER_A_PIN
      #ifdef ROTARY_CHANGE_BRIGHTNESS
        if (rotaryEncoder.encoderChanged()) {
          runningBrightness = ((rotaryEncoder.readEncoder() * 16) - 1);
          FastLED.setBrightness(runningBrightness);
        }
      #endif
    #endif

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
      if (0 != loraReceived) {

        lastCommand = cmd_to_send;
        lastCommandSenderId = my_device.id;
        lastCommandPacketId = mlsmeshLastPackedId;

        if (!MLS_masterMode) {
          // Send the command as in an ACK LIGHT packet
          light_packet = (LIGHT_PACKET){lastCommand, 0, millis(), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
          mlsmesh_send_packet(MLS_TYPE_ACK_LIGHT_DATA, (uint8_t *) &light_packet);
          DEBUG_PRINT("MLS_TYPE_ACK_LIGHT_DATA packet sent based on LoRa, command: ");
          DEBUG_PRINTLN(cmd_to_send);
        } else {
          // Command synced with bass drum
          if ((lastCommand >= 100) && (lastCommand <= 199)) {
            current_beat_effect = lastCommand;
          } else {
            // Direct command 
            if (EFFECT_CHECK == lastCommand) {
              light_packet = (LIGHT_PACKET){lastCommand, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            } else {
              light_packet = (LIGHT_PACKET){lastCommand, 0, millis(), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            }
            boolean sendResult = mlsmesh_send_packet(MLS_TYPE_LIGHT_DATA, (uint8_t *) &light_packet);
            DEBUG_PRINT("MLS_TYPE_LIGHT_DATA packet sent based on LoRa, command: ");
            DEBUG_PRINTLN(cmd_to_send);
            mlslighteffects.setLightData(millis(), &light_packet); // Max latency: 39000, not used yet.
            current_beat_effect = EFFECT_KEEP_ALIVE;
          }
        }
        
        if (!MLS_remoteControl) {
          DEBUG_PRINTLN("Send BLE info back through Lora");
          LoRa.beginPacket(LORA_IMPLICIT_HEADER);
          LoRa.print("253" + loraCommand); // SendAck
          LoRa.endPacket(false); // Do NOT send asynchronously
          LoRa.receive();
        }

        DEBUG_PRINT("LOOP: STATE_RUNNING: LoRa: loraReceived and resent: ");
        DEBUG_PRINT(loraReceived);
        DEBUG_PRINT(" loraCommand: ");
        DEBUG_PRINTLN(loraCommand);
        loraReceived = 0;

        if (cmd_to_send == EFFECT_REBOOT) {
          delay(500);
          ESP.restart();
        }
      }
    #endif

    // Bluetooth LE handling
    #ifdef BLE_SERVER
      if (MLS_remoteControl) {
        // BLE notification : announced devices, current packet and last effect sent
        if (deviceConnected) {
          if ((millis() - lastBleNotificationTS) > BLE_NOTIF_HEARTBEAT_MS) {
            lastBleNotificationTS = millis();
            itoa(announced_devices, tempStr, 10);
            strcpy(bleFeedback, tempStr);
            strcat(bleFeedback, ",");
            strcat(bleFeedback, bleLastCmdInfo);
            strcat(bleFeedback, ",");
            itoa(mlsmeshLastPackedId, tempStr, 10);
            strcat(bleFeedback, tempStr);
            pTxCharacteristic->setValue((uint8_t*)&bleFeedback, strlen(bleFeedback));
            pTxCharacteristic->notify();
            DEBUG_PRINT("LOOP: STATE_RUNNING: BLE: notified ");
            DEBUG_PRINTLN(bleFeedback);
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
          DEBUG_PRINTLN("LOOP: STATE_RUNNING: BLE: connecting device");
        }
      }
    #endif

    #ifdef DEBUG_MLS
      #ifdef MLS_DEMO
        if (millis() - startStateRunningTS < (( (4*5) * 4) * 1000)) {
          if (simulatorLastBeat != simulatorBeat) {
            if (MLS_masterMode) {
              demoStep = (int(simulatorBeat / (4*5)) % 4);
              switch (demoStep) {
                case 0:
                  light_packet = (LIGHT_PACKET){EFFECT_HEARTBEAT, 0, millis(), simulatorBeatSpeed, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // HEARTBEAT
                  mlslighteffects.setLightData(simulatorBeat, &light_packet);
                  sendResult = mlsmesh_send_packet(MLS_TYPE_LIGHT_DATA, (uint8_t *) &light_packet);
                  break;
                case 1:
                  if (0 == (simulatorBeat % 5)) {
                    light_packet = (LIGHT_PACKET){EFFECT_BREATH, 0, millis(), 5 * simulatorBeatSpeed, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // BLUE BREATH
                    mlslighteffects.setLightData(simulatorBeat, &light_packet);
                    sendResult = mlsmesh_send_packet(MLS_TYPE_LIGHT_DATA, (uint8_t *) &light_packet);
                  }
                  break;
                case 2:
                  light_packet = (LIGHT_PACKET){EFFECT_CHECK, 0, simulatorBeat, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // CHECK
                  mlslighteffects.setLightData(simulatorBeat, &light_packet);
                  sendResult = mlsmesh_send_packet(MLS_TYPE_LIGHT_DATA, (uint8_t *) &light_packet);
                  break;
                case 3:
                  light_packet = (LIGHT_PACKET){EFFECT_FLASH_ALTERNATE, MODIFIER_FLIP_FLOP, simulatorBeat, 0, 0, 0, 0, 0, 0, 3, 35, 0, 0, 0, 0, 3, 35}; // RED/GREEN FLASH FLIP-FLOP
                  mlslighteffects.setLightData(simulatorBeat, &light_packet);
                  sendResult = mlsmesh_send_packet(MLS_TYPE_LIGHT_DATA, (uint8_t *) &light_packet);
                  break;
                default:
                  break;
              }
            }
            simulatorLastBeat = simulatorBeat;
          }
        } else {
          light_packet = (LIGHT_PACKET){EFFECT_BLANK, 0, simulatorBeat, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // CHECK
          mlslighteffects.setLightData(simulatorBeat, &light_packet);
          sendResult = mlsmesh_send_packet(MLS_TYPE_LIGHT_DATA, (uint8_t *) &light_packet);
          break;
        }
      #endif // MLS_DEMO
    #endif // DEBUG_MLS
  }
  lastState = state; // Memorize the current state of the state machine in the lastState variable

  yield();

}
