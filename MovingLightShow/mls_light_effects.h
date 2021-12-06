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

  // System effects
  #define EFFECT_KEEP_ALIVE          0 // Effect is ignored, keep alive only
  #define EFFECT_NONE                1 // Effect is "none", stop the previous effect, but do nothing else, don't flush the LED strips
  #define EFFECT_BLANK               2 // Blank, strips cleared
  #define EFFECT_CHECK               3 // Ranks are looping in green on left strip, Columns are looping in red on right strip, it changes every second

  // Effects not synced with bass drum
  #define EFFECT_PROGRESS           10 // Auto progress bar, group of three leds by default (or as setup in option if option > 0), every 300ms (can be defined in duration_ms)
  #define EFFECT_PROGRESS4          11 // Auto progress bar, group of four leds, every 300ms (can be defined in speed)
  #define EFFECT_PROGRESS_RAINBOW   12 // Rainbow progress on all leds, every 300 ms (can be defined in speed)
  #define EFFECT_FIXED              13 // Fixed color(s), with fadein
  #define EFFECT_STROBE             14 // White stroboscop, 40ms on, repeat, every 100ms (can be defined in speed)
  #define EFFECT_BREATH             15 // Breath in a specific color, the default rythm of 4 seconds can be changed in the speed (if duration_ms > 0)
  #define EFFECT_HEARTBEAT          16 // Heartbeat in a specific color, the default rythm of 1 second can be changed in the speed (if duration_ms > 0)
  #define EFFECT_LARSON             17 // TODO Larson (K2000) effect, between the first and the last rank
  #define EFFECT_FIREFLY            18 // TODO
  #define EFFECT_POLICE             19 // TODO
  #define EFFECT_RAINBOW            20 // TODO Drum based Rainbow colors, same color for all on each beat, selection of 7 colors (or as setup in option)
  #define EFFECT_STARS              21 // TODO Stars effect (random stars randomly on each bracelet and for each musician)

  // Effects synced with bass drum
  #define EFFECT_FLASH             100 // Drum based flashed color(s), with fadein, on, fadeout
  #define EFFECT_FLASH_ALTERNATE   101 // Drum based flashed color(s), with fadein, on, fadeout
  #define EFFECT_FLASH_YELLOW      102 // Drum based flashed color(s), with fadein, on, fadeout
  #define EFFECT_WAVE_BACK         103 // TODO Wave to the back, the default speed can be changed in speed, and the default width in option
  #define EFFECT_WAVE_FORTH        104 // TODO Wave to the front, the default speed can be changed in speed, and the default width in option
  #define EFFECT_WAVE_BACK_FORTH   105 // TODO Wave back and forth, the default speed can be changed in speed, and the default width in option
  #define EFFECT_VUE_METER         106 // TODO Vuemeter, first ranks in green, two last ranks in red (and the rank before in orange(
  #define EFFECT_3_STEPS           107 // TODO Display every three ranks, rolling on each beat
  #define EFFECT_3_STEPS_ALTERNATE 108 // TODO Display every three ranks with alternate red/green colors, rolling on each beat
  #define EFFECT_RAINBOW_BEAT      109 // TODO Drum based Rainbow colors, same color for all on each beat, selection of 7 colors (or as setup in option)
  #define EFFECT_RAINBOW_RANK_BEAT 110 // TODO Drum based Rainbow colors, rolling colors per rank on each beat, selection of 7 rolling colors (or as setup in option)
  #define EFFECT_SPIN              111 // TODO

  // These effects are special effects, more commands than effects
  #define EFFECT_DRUM_ON           200 // TODO Drum on, will switch on drum LEDs with current colors
  #define EFFECT_DRUM_IN           201 // TODO Drum in, will switch bass drum with all musicians
  #define EFFECT_DRUM_OFF          202 // TODO Drum off, will switch off drum LEDs, AND will also switch off the "drum in" mode
  #define EFFECT_LIGHT_ON          203 // TODO
  #define EFFECT_FEEDBACK_INFO     253 // Some info in feedabck
  #define EFFECT_REBOOT            254 // Reboot effect (sent by the RC)
  #define EFFECT_EXTENDED          255 // Extended effect, see duration_ms and option for details

  // Effect modifiers
  #define MODIFIER_IGNORE_LEFT       1 // Don't send the data of the effect to the left strip
  #define MODIFIER_IGNORE_RIGHT      2 // Don't send the data of the effect to the right strip
  #define MODIFIER_REPEAT            4 // Repeat the effect
  #define MODIFIER_FLIP_FLOP         8 // Alternate the right/left colors on each beat
  #define MODIFIER_MASTER_INCLUDED  16 // TODO: This effect must also be played on the master in any case
  #define MODIFIER_START_WITH_BEAT  32 // TODO: Start a normally unsynced effect on the next beat detection
  #define MODIFIER_YYY              64 // TODO
  #define MODIFIER_6_COLUMNS_MODE  128 // TODO If the modifier is activated, we have to switch in 6 columns mode

  // Stars configuration
  #define STARS_PROBABILITY     3    // Star creation probability (3 means 1/3)
  #define STARS_GAP_MINIMUM     500  // Time before the next creation trial in ms
  #define STARS_GAP_MAXIMUM     1000 // Time before the next creation trial in ms
  #define STARS_FADEIN_MINIMUM  20   // Minimum fadein time of a star in ms
  #define STARS_FADEIN_MAXIMUM  100  // Maximum fadein time of a star in ms
  #define STARS_FADEOUT_MINIMUM 200  // Minimum fadeout time of a star in ms
  #define STARS_FADEOUT_MAXIMUM 1000 // Maximum fadeout time of a star in ms

  // Firefly configuration
  #define FIREFLIES_PROBABILITY     5    // Firefly creation probability (5 means 1/5)
  #define FIREFLIES_GAP_MINIMUM     500  // Time before the next creation trial in ms
  #define FIREFLIES_GAP_MAXIMUM     1000 // Time before the next creation trial in ms
  #define FIREFLIES_LIFE_MINIMUM    1500 // Minimum complete lifetime of a firefly in ms
  #define FIREFLIES_LIFE_MAXIMUM    3500 // Maximum complete lifetime of a firefly in ms
  #define FIREFLIES_FADEIN_MINIMUM  200  // Minimum fadein time of a firefly in ms
  #define FIREFLIES_FADEIN_MAXIMUM  1000 // Maximum fadein time of a firefly in ms
  #define FIREFLIES_FADEOUT_MINIMUM 100  // Minimum fadeout time of a firefly in ms
  #define FIREFLIES_FADEOUT_MAXIMUM 500  // Maximum fadeout time of a firefly in ms
  #define FIREFLIES_COLORS          16   // Amount of fireflies colors in the palette

  const uint32_t PROGMEM FirefliesColorPalette[] = {
    0xff0000, 0x00ff00, 0x0000ff, 0xffff00,
    0xff00ff, 0x00ffff, 0xffff80, 0xff80ff,
    0x80ffff, 0xff4080, 0x40ff80, 0x4080ff,
    0x80ff40, 0xff8040, 0x8040ff, 0xff8080};

  const struct CRGB MLS_ORANGE       = CRGB(255,165,  0);
  const struct CRGB MLS_DARK_ORANGE  = CRGB(255,140,  0);
  const struct CRGB MLS_FADED_BLUE   = CRGB(  0,  0, 63);
  const struct CRGB MLS_WHITE192     = CRGB(192,192,192);
  const struct CRGB MLS_RED224       = CRGB(224,  0,  0);
  const struct CRGB MLS_FADED_GREEN  = CRGB(  0, 63,  0);
  const struct CRGB MLS_FADED_RED    = CRGB( 63,  0,  0);
  const struct CRGB MLS_FADED_ORANGE = CRGB(63,  41,  0);

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

  union FIREFLY_COLOR {
    uint32_t raw;
    uint8_t one[4];
  };


  struct FLIP_DATA {
    union {
      struct {
        union {
          struct {
            uint8_t color_r;           // Red component for the current strip
            uint8_t color_g;           // Green component for the current strip
            uint8_t color_b;           // Blue component for the current strip
          } __attribute__((__packed__));
          uint8_t color_raw[3];        // RGB color for the current strip
        };
        uint32_t fadein_time_micros;   // Fade in time for the current effect of the current strip (in microseconds)
        uint32_t on_time_micros;       // On time for the current effect of the current strip (in microseconds)
        uint32_t fadeout_time_micros;  // Fade out time for the current effect of the current strip (in microseconds)
      } __attribute__((__packed__));
      uint8_t raw[15];                 // Flippable data
    };
  } __attribute__((__packed__));
  const uint8_t FLIP_DATA_SIZE = sizeof(FLIP_DATA);


  struct STRIP_DATA {
    union {
      struct {
        uint8_t received;              // Packet received
        uint32_t received_time_micros; // Fade in time for the current effect of the current strip (in microseconds)
        uint8_t applied;               // Packet applied
        uint8_t repeat;                // Effect with repeat flag
        uint16_t packet;               // Packet ID concerned by this data
        uint16_t step;                 // Light step in one beat (used internally by some effects with several steps)
        uint16_t last_step;            // Last light step in one beat (used internally by some effects with several steps)
        uint16_t leds_per_strip;       // Number of LEDs in the strip
        uint32_t latency_micros;       // Latency for this packet (in microseconds)
        int32_t start_time_micros;     // Start time (including optional latency) for the current effect of the current strip (in microseconds)
        int32_t delta_time_micros;     // Delta time (including optional latency) since start for the current effect of the current strip (in microseconds)
        uint8_t effect;                // Effection (see constants for possible values)
        uint8_t effect_modifier;       // Effect modifier (see constants for possible values)
        uint16_t repeat_counter;       // Repeat counter sent by the sender
        uint16_t duration_ms;          // Duration of the effect (in ms)
        uint16_t option;               // Option of the effect
        union {
          struct {
            union {
              struct {
                uint8_t color_r;             // Red component for the current strip
                uint8_t color_g;             // Green component for the current strip
                uint8_t color_b;             // Blue component for the current strip
              } __attribute__((__packed__));
              uint8_t color_raw[3];          // RGB color for the current strip
            };
            uint32_t fadein_time_micros;     // Fade in time for the current effect of the current strip (in microseconds)
            uint32_t on_time_micros;         // On time for the current effect of the current strip (in microseconds)
            uint32_t fadeout_time_micros;    // Fade out time for the current effect of the current strip (in microseconds)
          } __attribute__((__packed__));
          uint8_t flip_data[FLIP_DATA_SIZE]; // Flippable data
        };
      } __attribute__((__packed__));
      uint8_t raw[50];                 // Full raw data for the current strip
    };
  } __attribute__((__packed__));
  const uint8_t STRIP_DATA_SIZE = sizeof(STRIP_DATA);


  struct LIGHT_PACKET {               // Light packet (LIGHT DATA payload)
    union {
      struct {
        uint8_t effect;               // Effection (see constants for possible values)
        uint8_t effect_modifier;      // Effect modifier (see constants for possible values)
        uint16_t repeat_counter;      // Repeat counter sent by the sender
	      uint16_t duration_ms;         // Duration of the effect (in ms)
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
          } __attribute__((__packed__));
          uint8_t right_color_raw[3]; // RGB color for the right strip
        };
        uint8_t right_fadein_time;    // Fade in time for the right strip (in step of 10ms)
        uint8_t right_on_time;        // On time for the right strip (in step of 10ms)
        uint8_t right_fadeout_time;   // Fade out time for the right strip (in step of 10ms)
      } __attribute__((__packed__));
      uint8_t raw[20];                // Full raw data of the packet
    };
  } __attribute__((__packed__));
  const uint8_t LIGHT_PACKET_SIZE = sizeof(LIGHT_PACKET);


  class MlsLightEffects {
    
    private:
      struct STRIP_DATA data_received[2];
      struct STRIP_DATA data_actual[2];
      struct FLIP_DATA data_flip[2];
      uint16_t play_counter[2];
      uint16_t current_play_counter[2];
      struct CRGB *left_strip;
      struct CRGB *right_strip;
      struct CRGB last_left_strip[NUM_LEDS_PER_STRIP];
      struct CRGB last_right_strip[NUM_LEDS_PER_STRIP];
      uint16_t received_packet;
      uint16_t leds_per_strip;
  	  uint8_t number_of_columns = 4;
	    uint8_t number_of_ranks = 8;
      uint8_t my_column;
      uint8_t my_rank;

    public:
      MlsLightEffects(uint16_t leds_per_strip, struct CRGB *left_strip, struct CRGB *right_strip);
      void clearLeds();
      void effectBreath(struct STRIP_DATA *actual_data, struct CRGB *strip);
      void effectCheck(struct STRIP_DATA *actual_data, struct CRGB *strip, uint8_t lr);
      void effectFirefly(struct STRIP_DATA *actual_data, struct CRGB *strip);
      void effectFixed(struct STRIP_DATA *actual_data, struct CRGB *strip);
      void effectFlash(struct STRIP_DATA *actual_data, struct CRGB *strip);
      void effectHeartbeat(struct STRIP_DATA *actual_data, struct CRGB *strip);
      void effectProgress(struct STRIP_DATA *actual_data, struct CRGB *strip);
      void effectProgressRainbow(struct STRIP_DATA *actual_data, struct CRGB *strip);
      void effectRainbowRankBeat(struct STRIP_DATA *actual_data, struct CRGB *strip);
      void effectStars(struct STRIP_DATA *actual_data, struct CRGB *strip);
      void effectThreeSteps(struct STRIP_DATA *actual_data, struct CRGB *strip);
      void effectThreeStepsAlternate(struct STRIP_DATA *actual_data, struct CRGB *strip);
      void effectVueMeter(struct STRIP_DATA *actual_data, struct CRGB *strip);
      void effectWaveBack(struct STRIP_DATA *actual_data, struct CRGB *strip);
      void fill(struct CRGB color, uint16_t number_of_leds, struct CRGB *strip);
      uint8_t getColumns();
      uint8_t getMyColumn();
      uint8_t getMyRank();
      uint8_t getRanks();
      void setColumns(uint8_t number_of_columns);
      void setLedsPerStrip(uint16_t leds_per_strip);
      void setLightData(uint16_t packetId, struct LIGHT_PACKET *lightData);
      void setLightData(uint16_t packetId, struct LIGHT_PACKET *lightData, uint32_t latency_micros);
      void setMyColumn(uint8_t column);
      void setMyRank(uint8_t rank);
      void setRanks(uint8_t number_of_ranks);
      void setStrips(struct CRGB *left_strip, struct CRGB *right_strip);
      void setValue(uint16_t value, uint16_t maxValue, struct CRGB colorOffMin, struct CRGB colorOffMax, struct CRGB colorOnMin, struct CRGB colorOnMax, struct CRGB *strip);
      void setValueThree(uint16_t value, uint16_t maxValue, struct CRGB colorOff, struct CRGB colorOn1, struct CRGB colorOn2, struct CRGB colorOn3, struct CRGB *strip);
      void showLeds();
      void stopUpdate();
      void updateLight();
      struct CRGB adjustBrightness(struct CRGB color, uint8_t brightness);
  };

#endif
