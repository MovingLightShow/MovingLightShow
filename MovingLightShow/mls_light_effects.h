/**********************************************************************
 *
 * MovingLightShow package - Synchronized LED strips for musicians
 * https://MovingLightShow.art
 *
 * @file  mls_light_effects.h
 * @brief Light effects
 * 
 **********************************************************************/
#ifndef MLS_LIGHT_EFFECTS_H
#define MLS_LIGHT_EFFECTS_H

  #include "mls_config.h"
  #include "DebugTools.h"

  #include "FastLED.h"
  #include <stdint.h>

  #define EFFECT_KEEP_ALIVE          0 // Effect is ignored, keep alive only
  #define EFFECT_NONE                1 // Effect is "none", do nothing, but don't flush the LED strips
  #define EFFECT_BLANK               2 // Blank, strips cleared
  #define EFFECT_CHECK               3 // TODO Columns are looping in green on left strip, ranks are looping in red on right strip, it changes every second
  #define EFFECT_FIXED              10 // Fixed color(s), with fadein
  #define EFFECT_STROBE             11 // White stroboscop, 40ms on, repeat, every 100ms (can be defined in speed)
  #define EFFECT_PROGRESS           12 // Auto progress bar, group of three leds by default (or as setup in option if option > 0), every 300ms (can be defined in speed)
  #define EFFECT_PROGRESS4          13 // Auto progress bar, group of four leds, every 300ms (can be defined in speed)
  #define EFFECT_PROGRESS_RAINBOW   14 // Rainbow progress on all leds, every 300 ms (can be defined in speed)
  #define EFFECT_BREATH             15 // TODO
  #define EFFECT_HEARTBEAT          16 // TODO
  #define EFFECT_LARSON             17 // TODO
  #define EFFECT_WAVE_BACK          18 // TODO
  #define EFFECT_WAVE_FORTH         19 // TODO
  #define EFFECT_WAVE_BACK_FORTH    20 // TODO
  #define EFFECT_VUE_METER          21 // TODO
  #define EFFECT_3_STEPS            22 // TODO
  #define EFFECT_FIREFLY            23 // TODO
  #define EFFECT_FLASH             100 // Drum based flashed color(s), with fadein, on, fadeout
  #define EFFECT_RAINBOW_BEAT      101 // TODO Drum based Rainbow colors, same color for all on each beat, selection of 7 colors (or as setup in option)
  #define EFFECT_RAINBOW_RANK_BEAT 102 // TODO Drum based Rainbow colors, rolling colors per rank on each beat, selection of 7 rolling colors (or as setup in option)

  #define MODIFIER_IGNORE_LEFT       1 // Don't send the data of the effect to the left strip
  #define MODIFIER_IGNORE_RIGHT      2 // Don't send the data of the effect to the right strip
  #define MODIFIER_REPEAT            4 // Repeat the effect
  #define MODIFIER_FLIP_FLOP         8 // Alternate the right/left colors on each beat
  #define MODIFIER_IGNORE_LATENCY   16 // Execute the effect as soon as possible, don't use the latency information (except for specific effects like STROBE)
  #define MODIFIER_MASTER_INCLUDED  32 // This effect must also be played on the master in any case
  #define MODIFIER_6_COLUMNS_MODE  128 // TODO

  const struct CRGB MLS_ORANGE      = CRGB(255,165,  0);
  const struct CRGB MLS_DARK_ORANGE = CRGB(255,140,  0);
  const struct CRGB MLS_FADED_BLUE  = CRGB(  0,  0, 63);
  const struct CRGB MLS_WHITE192    = CRGB(192,192,192);
  const struct CRGB MLS_RED224      = CRGB(224,  0,  0);

  // https://learn.adafruit.com/led-tricks-gamma-correction/the-quick-fix
  const uint8_t PROGMEM gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };
  
  struct CURRENT_DATA {
    union {
      struct {
        int32_t delta_time_micros;    // Delta time (including optional latency) since start for the current effect of the current strip (in microseconds)
        int32_t start_time_micros;    // Start time (including optional latency) for the current effect of the current strip (in microseconds)
        uint32_t fadein_time_micros;  // Fade in time for the current effect of the current strip (in microseconds)
        uint32_t on_time_micros;      // On time for the current effect of the current strip (in microseconds)
        uint32_t fadeout_time_micros; // Fade out time for the current effect of the current strip (in microseconds)
        union {
          struct {
            uint8_t color_r;          // Red component for the current strip
            uint8_t color_g;          // Green component for the current strip
            uint8_t color_b;          // Blue component for the current strip
          };
          uint8_t color_raw[3];       // RGB color for the current strip
        };
      };
      uint8_t raw[23];                // Full raw data for the current strip
    };
  };
  const uint8_t CURRENT_DATA_SIZE = 23;


  struct LIGHT_PACKET {               // Light packet (LIGHT DATA payload)
    union {
      struct {
        uint8_t effect;               // Effection (see constants for possible values)
        uint8_t effect_modifier;      // Effect modifier (see constants for possible values)
        uint16_t repeat_counter;      // Repeat counter sent by the sender
	      uint16_t duration;            // Duration of the effect (in ms)
	      uint16_t option;              // Option of the effect
        union {
          struct {
            uint8_t left_color_r;     // Red component for the left strip
            uint8_t left_color_g;     // Green component for the left strip
            uint8_t left_color_b;     // Blue component for the left strip
          };
          uint8_t left_color_raw[3];  // RGB color for the left strip
        };
        uint8_t left_fadein_time;     // Fade in time for the left strip (in step of 10ms)
        uint8_t left_on_time;         // On time for the left strip (in step of 10ms)
        uint8_t left_fadeout_time;    // Fade out time for the left strip (in step of 10ms)
        union {
          struct {
            uint8_t right_color_r;    // Red component for the right strip
            uint8_t right_color_g;    // Green component for the right strip
            uint8_t right_color_b;    // Blue component for the right strip
          };
          uint8_t right_color_raw[3]; // RGB color for the right strip
        };
        uint8_t right_fadein_time;    // Fade in time for the right strip (in step of 10ms)
        uint8_t right_on_time;        // On time for the right strip (in step of 10ms)
        uint8_t right_fadeout_time;   // Fade out time for the right strip (in step of 10ms)
      };
      uint8_t raw[20];                // Full raw data of the packet
    };
  };
  const uint8_t LIGHT_PACKET_SIZE = 20;


  class MlsLightEffects {
    
    private:
      bool lightPacketReceived = false;
      bool currentlightPacketReceived = false;
      uint32_t currentPackedReceivedTimeMicros = 0;
      uint16_t idPacketReceived = 0;
      struct LIGHT_PACKET light_packet;
      struct CURRENT_DATA left_data;
      struct CURRENT_DATA right_data;
      uint32_t receivedLatencyTimeMicros = 0;
      uint32_t currentLatencyTimeMicros = 0;
      uint32_t effect_start_time_micros;
      uint16_t received_packet;
      uint16_t current_packet;
      uint16_t current_effect;
      uint16_t play_counter;
      uint16_t current_play_counter;
      uint16_t leds_per_strip;
      int8_t last_light_step = 0;
  	  uint8_t number_of_columns;
	    uint8_t number_of_ranks;
      struct CRGB *left_strip;
      struct CRGB *right_strip;
      void fill(struct CRGB color, struct CRGB *strip);
      void setValue(uint16_t value, uint16_t maxValue, struct CRGB colorOffMin, struct CRGB colorOffMax, struct CRGB colorOnMin, struct CRGB colorOnMax, struct CRGB *strip);
      void setValueThree(uint16_t value, uint16_t maxValue, struct CRGB colorOff, struct CRGB colorOn1, struct CRGB colorOn2, struct CRGB colorOn3, struct CRGB *strip);
      void rainbow(struct CRGB *strip);
      void progressRainbow(struct CRGB *strip);

    public:
      struct LIGHT_PACKET received_light_packet;
      struct LIGHT_PACKET internal_light_packet;
      MlsLightEffects(uint16_t leds_per_strip, struct CRGB *left_strip, struct CRGB *right_strip);
      void effectFixed();
      void effectFlash();
      void effectProgress();
      void effectProgressRainbowAll();
      void effectRainbowAll();
      void effectRainbowLeft();
      void effectRainbowRight();
      void setLightData(uint16_t packetId, struct LIGHT_PACKET lightData);
      void setLightData(uint16_t packetId, struct LIGHT_PACKET lightData, uint32_t receivedLatencyTimeMicros);
      void updateLight();
      struct CRGB adjustBrightness(struct CRGB color, uint8_t brightness);
      void fillAll(struct CRGB color);
      void fillLeft(struct CRGB color);
      void fillRight(struct CRGB color);
      void setValueAll(uint16_t value, uint16_t maxValue, struct CRGB colorOffMin, struct CRGB colorOffMax, struct CRGB colorOnMin, struct CRGB colorOnMax);
      void setValueLeft(uint16_t value, uint16_t maxValue, struct CRGB colorOffMin, struct CRGB colorOffMax, struct CRGB colorOnMin, struct CRGB colorOnMax);
      void setValueRight(uint16_t value, uint16_t maxValue, struct CRGB colorOffMin, struct CRGB colorOffMax, struct CRGB colorOnMin, struct CRGB colorOnMax);
      void setValueThreeAll(uint16_t value, uint16_t maxValue, struct CRGB colorOff, struct CRGB colorOn1, struct CRGB colorOn2, struct CRGB colorOn3);
      void setValueThreeLeft(uint16_t value, uint16_t maxValue, struct CRGB colorOff, struct CRGB colorOn1, struct CRGB colorOn2, struct CRGB colorOn3);
      void setValueThreeRight(uint16_t value, uint16_t maxValue, struct CRGB colorOff, struct CRGB colorOn1, struct CRGB colorOn2, struct CRGB colorOn3);
      void stopUpdate();
  };

#endif
