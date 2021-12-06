/**********************************************************************
 *
 * MovingLightShow package - Synchronized LED strips for musicians
 * https://MovingLightShow.art
 *
 * @file  DebugTools.h
 * @brief Debug tools used during development, based on https://github.com/cubiwan/Debugino
 *
 * DebugTools usage:
 * 
 *   Debug on:
 *     #define DEBUG
 *     #include "DebugTools.h"
 *   
 *   Debug off:
 *     // #define DEBUG
 *     #include "DebugTools.h"
 *   
 **********************************************************************/
#ifndef DEBUGTOOLS_H
#define DEBUGTOOLS_H

  #ifdef DEBUG_MLS
    #define DEBUG_PRINT(x)               Serial.print   (x)
    #define DEBUG_PRINTF(x,y)            Serial.printf  (x, y)
    #define DEBUG_PRINTFF(x,y,z)         Serial.printf  (x, y, z)
    #define DEBUG_PRINTDEC(x)            Serial.print   (x, DEC)
    #define DEBUG_PRINTHEX(x)            Serial.print   (x, HEX)
    #define DEBUG_PRINTLN(x)             Serial.println (x)
    #define DEBUG_INFO(str)              \
      Serial.print(millis());            \
      Serial.print(": ");                \
      Serial.print(__PRETTY_FUNCTION__); \
      Serial.print(' ');                 \
      Serial.print(__FILE__);            \
      Serial.print(':');                 \
      Serial.print(__LINE__);            \
      Serial.print(' ');                 \
      Serial.println(str);
  #else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTF(x,y)
    #define DEBUG_PRINTFF(x,y,z)
    #define DEBUG_PRINTDEC(x)
    #define DEBUG_PRINTHEX(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_INFO(str)
  #endif

#endif
