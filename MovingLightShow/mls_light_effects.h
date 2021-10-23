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

  #define DEBUG_MLS
  #include "DebugTools.h"

  #include "FastLED.h"
  #include <stdint.h>

  #define EFFECT_KEEP_ALIVE       0
  #define EFFECT_NONE             1
  #define EFFECT_BLANK            2
  #define EFFECT_FIXED            3
  #define EFFECT_FLASH            4
  #define EFFECT_STROBE           5
  #define EFFECT_BREATH           6
  #define EFFECT_PROGRESS         7
  #define EFFECT_PROGRESS4        8
  #define EFFECT_PROGRESS_RAINBOW 9

  #define MODIFIER_IGNORE_LEFT  1
  #define MODIFIER_IGNORE_RIGHT 2
  #define MODIFIER_REPEAT       4
  #define MODIFIER_FLIP_FLOP    8

  const struct CRGB ORANGE      = CRGB(255,165,  0);
  const struct CRGB DARK_ORANGE = CRGB(255,140,  0);
  const struct CRGB FADED_BLUE  = CRGB(  0,  0, 31);

  #define CURRENT_DATA_SIZE 23
  struct CURRENT_DATA {
    union {
      struct {
        uint32_t delta_time;
        uint32_t start_time;
        uint32_t fadein_time;
        uint32_t on_time;
        uint32_t fadeout_time;
        union {
          struct {
            uint8_t color_r;
            uint8_t color_g;
            uint8_t color_b;
          };
          uint8_t color_raw[3];
        };
      };
      uint8_t raw[CURRENT_DATA_SIZE];
    };
  };

  #define LIGHT_PACKET_SIZE 18
  struct LIGHT_PACKET {
    union {
      struct {
        uint8_t effect;
        uint8_t effect_modifier;
        uint16_t repeat_counter;
        union {
          uint16_t speed;
          uint16_t option;
        };
        union {
          struct {
            uint8_t left_color_r;
            uint8_t left_color_g;
            uint8_t left_color_b;
          };
          uint8_t left_color_raw[3];
        };
        uint8_t left_fadein_time;
        uint8_t left_on_time;
        uint8_t left_fadeout_time;
        union {
          struct {
            uint8_t right_color_r;
            uint8_t right_color_g;
            uint8_t right_color_b;
          };
          uint8_t right_color_raw[3];
        };
        uint8_t right_fadein_time;
        uint8_t right_on_time;
        uint8_t right_fadeout_time;
      };
      uint8_t raw[LIGHT_PACKET_SIZE];
    };
  };

  class MlsLightEffects {
    
    private:
      struct LIGHT_PACKET received_light_packet;
      bool lightPackedReceived = false;
      uint16_t idPacketReceived = 0;
      struct LIGHT_PACKET light_packet;
      struct CURRENT_DATA left_data;
      struct CURRENT_DATA right_data;
      uint32_t effect_start_time;
      uint16_t received_packet;
      uint16_t current_packet;
      uint16_t current_effect;
      uint16_t play_counter;
      uint16_t current_play_counter;
      uint16_t leds_per_strip;
      struct CRGB *left_strip;
      struct CRGB *right_strip;
      void fill(struct CRGB color, struct CRGB *strip);
      void setValue(uint16_t value, uint16_t maxValue, struct CRGB colorOffMin, struct CRGB colorOffMax, struct CRGB colorOnMin, struct CRGB colorOnMax, struct CRGB *strip);
      void setValueThree(uint16_t value, uint16_t maxValue, struct CRGB colorOff, struct CRGB colorOn1, struct CRGB colorOn2, struct CRGB colorOn3, struct CRGB *strip);
      void rainbow(struct CRGB *strip);
      void progressRainbow(struct CRGB *strip);

    public:
      MlsLightEffects(uint16_t leds_per_strip, struct CRGB *left_strip, struct CRGB *right_strip);
      void effectFixed();
      void effectFlash();
      void effectProgress();
      void effectProgress(uint8_t steps);
      void effectProgressRainbowAll();
      void effectRainbowAll();
      void effectRainbowLeft();
      void effectRainbowRight();
      void setLightData(uint16_t packetId, struct LIGHT_PACKET lightData);
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
