/**********************************************************************
 *
 * MovingLightShow package - Synchronized LED strips for musicians
 * https://MovingLightShow.art
 *
 * @file  mls_mesh.h
 * @brief MLSmesh implementation (on the top of ESPNOW)
 * 
 **********************************************************************/
#ifndef MLS_MESH_H
#define MLS_MESH_H

  #include "mls_config.h"
  #include "DebugTools.h"
  #include "mls_light_effects.h"
  #include <stdint.h>
  #include <esp_now.h>
  #include <WiFi.h>
  #include <Wire.h>
  #include "esp_wifi.h"

  #define MLS_ACTION_KEEP_ALIVE     0   // Packet is ignored, keep alive only
  #define MLS_ACTION_REBOOT         99  // Reset the device
  #define MLS_ACTION_FORCE_UPDATE   199 // Force the firmware update

  #define MLS_TOPOLOGY_KEEP_ALIVE   0
  #define MLS_TOPOLOGY_REQUEST      1
  #define MLS_TOPOLOGY_REPLY        2

  #define MLS_TYPE_TOPOLOGY_DATA    1
  #define MLS_TYPE_ACTION_DATA      2
  #define MLS_TYPE_LIGHT_DATA       3
  #define MLS_TYPE_ACK_LIGHT_DATA   4
  #define MLS_TYPE_MODIFIER_UNICAST 0x20
  #define MLS_TYPE_MODIFIER_GROUP   0x40

  #define MLS_DATA_SIZE             20


  struct DEVICE_INFO {
    uint8_t mac[6];
    uint8_t id;
    uint8_t rank;
    uint8_t column;
    int8_t rssi;
    uint32_t rssi_time; // last time rssi was measured (in ms)
  };


  struct MLS_PACKET {
    union {
      struct {
        union {
          struct {
            char IID[3];                 // Installation ID
            uint8_t TYPE;                // Packet type: 0x01: TOPOLOGY DATA (payload is TOPOLOGY_PACKET)
                                         //              0x02: ACTION DATA (payload is ACTION_PACKET)
                                         //              0x03: LIGHT DATA  (payload is LIGHT_PACKET)
                                         //              0x04: ACK LIGHT DATA (like LIGHT DATA, but sent back from all devices)
                                         //             +0x20: UNICAST destination packet (Device ID is in DESTINATION_ID)
                                         //             +0x40: GROUP destination packet (Group ID is in DESTINATION_ID)
            uint8_t RESERVED;            // Reserved for future use
            uint8_t SENDER_ID;           // Sender ID: 0x00: master, 0x01-FE registered clients, 0xFF unregistered client
            uint8_t DESTINATION_ID;      // Destination ID (only used for 0x20 or 0x40 bit in packet type)
            uint16_t PACKET_ID;          // Original packet number. Repeaters repeat always the original packet number for every forwarded packet
            uint8_t REPEATER_POSITION;   // LSB: repeater position, MSB: repeater organisation (1x/2x/3x/4x/6x/Cx)   0-30 0-90
            uint8_t REPEATERS_ID[12];    // All repeaters,  in airtime order (is filled by repeaters for the next repeater)
            uint8_t ANNOUNCED_DEVICES;   // Number of devices currently in the network (including the master)
            uint8_t NUMBER_OF_COLUMNS;   // Number of columns currently in the network (including the master)
            uint8_t NUMBER_OF_RANKS;     // Number of ranks currently in the network (including the master)
            int8_t RSSI0;                // RSSI of the master for this sender
            uint8_t RSSI_SHIFTER;        // LSB: shifter (0-8)
                                         // MSB: RSSI organisation :
                                         //  1: regular (value up to -126, -127: no signal)
                                         //  2: Rescaled RSSI in 2 x 4 bits
                                         //     0: 0 to -40 dBm , and -3dBm per step (1: down to -34dBm, 2: down to -38dBm, ... 14: down to -82dBm, 15 : -83dBm and lower
                                         //  4: Rescaled RSSI in 4 x 2 bits
                                         //     0: 0 to -40 dBm, 1: -41 to -70dBm, 2: -71 TO -80dBm, 3: -81dBm and lower
            int8_t RSSI[24];             // 0 to -120 dBm (-70dBm Minimum signal strength for reliable packet delivery,
                                         //   -80dBm Minimum signal strength for basic connectivity. Packet delivery may be unreliable.)
            uint8_t FIRST_SENDER_ID;     // First sender ID which have sent the last packet
            uint8_t FIRST_REPEATER_SLOT; // First repeater slot which have sent the last packet
            uint8_t BETTER_SENDER_ID;    // Sender ID of the better sender RSSI for the last packet
            uint8_t BETTER_REPEATER_ID;  // Sender ID of the better repeater RSSI for the last packet
            uint8_t COMMAND;             // Command sent (0: no new command. FF: Enhanced command, the command is a full LIGHT_PACKET. The same command is repeated at least 10x)
            uint8_t COMMAND_SENDER_ID;   // Sender ID which has sent the command (0: no sender, master cannot send himself command packet)
            uint16_t COMMAND_PACKET_ID;  // Packet ID of the command
            uint8_t DATA[MLS_DATA_SIZE]; // Effective payload of the packet
          } __attribute__((__packed__));
          uint8_t CRC_DATA[79];          // Data on which to calculate the CRC
        };
        uint8_t CRC;                     // CRC control
      } __attribute__((__packed__));
      uint8_t raw[80];                   // Full raw data of the packet
    };
  } __attribute__((__packed__));
  const uint8_t MLS_PACKET_SIZE = sizeof(MLS_PACKET);


  struct TOPOLOGY_PACKET {
    union {
      struct {
        uint8_t type;
        uint8_t device_id;
        uint8_t mac[6];
        uint8_t rank;
        uint8_t column;
        uint8_t topology_data[10];
      } __attribute__((__packed__));
      uint8_t raw[MLS_DATA_SIZE];     // Full raw data of the packet
    };
  } __attribute__((__packed__));
  const uint8_t TOPOLOGY_PACKET_SIZE = sizeof(TOPOLOGY_PACKET);


  struct ACTION_PACKET {               // Action packet (ACTION DATA payload)
    union {
      struct {
        uint8_t action;               // Action (see constants for possible values)
        uint8_t action_data[19];      // Action payload
      } __attribute__((__packed__));
      uint8_t raw[MLS_DATA_SIZE];     // Full raw data of the packet
    };
  } __attribute__((__packed__));
  const uint8_t ACTION_PACKET_SIZE = sizeof(ACTION_PACKET);

  const uint8_t espnowBroadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  uint8_t searchDevice(struct DEVICE_INFO *all_devices, uint8_t number_of_devices, uint8_t *mac);

#endif
