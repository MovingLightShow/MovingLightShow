/**********************************************************************
 *
 * MovingLightShow package - Synchronized LED strips for musicians
 * https://MovingLightShow.art
 *
 * @file  mls_lora.h
 * @brief LoRa helpers
 * 
 **********************************************************************/
#ifndef MLS_LORA_H
#define MLS_LORA_H

  #include "mls_config.h"
  #include "DebugTools.h"

  #include <SPI.h>

  #ifdef ARDUINO_TTGO_LoRa32_v21new
    // https://github.com/sandeepmistry/arduino-LoRa
    #include <LoRa.h>
  #endif

  struct REMOTE_CONTROL_PACKET {
    union {
      struct {
        union {
          struct {
            char IID[3];                // Installation ID
            uint16_t CONTROL_PACKET_ID; // Original packet number
            uint8_t COMMAND;            // Command sent (0: no new command. FF: Enhanced command, the command is a full LIGHT_PACKET. The same command is repeated at least 10x)
            uint8_t DATA[20];           // Enhanced command data
          } __attribute__((__packed__));
          uint8_t CRC_DATA[26];         // Data on which to calculate the CRC
        };
        uint8_t CRC;                    // CRC control
      } __attribute__((__packed__));
      uint8_t raw[27];                  // Full raw data of the packet
    };
  } __attribute__((__packed__));
  const uint8_t REMOTE_CONTROL_PACKET_SIZE = sizeof(REMOTE_CONTROL_PACKET);
 
#endif
