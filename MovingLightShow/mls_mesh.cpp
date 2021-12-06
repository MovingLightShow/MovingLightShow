/**********************************************************************
 *
 * MovingLightShow package - Synchronized LED strips for musicians
 * https://MovingLightShow.art
 *
 * @file  mls_mesh.cpp
 * @brief MLSmesh implementation (on the top of ESPNOW)
 * 
 **********************************************************************/
#include "mls_mesh.h"

uint8_t searchDevice(struct DEVICE_INFO *all_devices, uint8_t number_of_devices, byte *mac) {
  uint8_t position = 255;
  #ifdef DEBUG_MLS
    char macStr1[18];
    char macStr2[18];
    snprintf(macStr1, sizeof(macStr1), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  #endif
  for (int i = 0; i < number_of_devices; ++i) {
    if (memcmp(mac, all_devices[i].mac, 6) == 0) {
      #ifdef DEBUG_MLS
        snprintf(macStr2, sizeof(macStr2), "%02x:%02x:%02x:%02x:%02x:%02x", all_devices[i].mac[0], all_devices[i].mac[1], all_devices[i].mac[2], all_devices[i].mac[3], all_devices[i].mac[4], all_devices[i].mac[5]);
        DEBUG_PRINT("Device "); DEBUG_PRINT(i); DEBUG_PRINT(", my MAC: "); DEBUG_PRINTLN(macStr1); DEBUG_PRINT(", MAC: "); DEBUG_PRINTLN(macStr2); 
      #endif
      position = i;
      DEBUG_PRINT("Device found at position "); DEBUG_PRINTLN(position);
      break;
    }
  }
  return position;
}
