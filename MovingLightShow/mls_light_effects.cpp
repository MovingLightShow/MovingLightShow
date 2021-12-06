/**********************************************************************

   MovingLightShow package - Synchronized LED strips for musicians
   https://MovingLightShow.art

   @file  mls_light_effects.cpp
   @brief Light effects

 **********************************************************************/
#include "mls_light_effects.h"


// MlsLightEffects constructor
MlsLightEffects::MlsLightEffects(uint16_t leds_per_strip, struct CRGB *left_strip, struct CRGB *right_strip) {
  this->setLedsPerStrip(leds_per_strip);
  this->setStrips(left_strip, right_strip);
    for (uint8_t lr = 0; lr < 2; lr++) {
    this->data_actual[lr].packet = 0;
    this->data_actual[lr].effect = 0;
    this->data_received[lr].packet = 0;
    this->data_received[lr].effect = 0;
  }
}


// Set the LEDs per strip
void MlsLightEffects::setLedsPerStrip(uint16_t leds_per_strip) {
  this->leds_per_strip = leds_per_strip;
}


// Set the strips memory addresses
void MlsLightEffects::setStrips(struct CRGB *left_strip, struct CRGB *right_strip) {
  this->left_strip = left_strip;
  this->right_strip = right_strip;
}


// Get the number of columns
uint8_t MlsLightEffects::getColumns() {
  return this->number_of_columns;
}


// Get the number of ranks
uint8_t MlsLightEffects::getRanks() {
  return this->number_of_ranks;
}


// Set the number of columns
void MlsLightEffects::setColumns(uint8_t number_of_columns) {
  this->number_of_columns = number_of_columns;
}


// Set the number of ranks
void MlsLightEffects::setRanks(uint8_t number_of_ranks) {
  this->number_of_ranks = number_of_ranks;
}


uint8_t MlsLightEffects::getMyColumn() {
  return this->my_column;
}


// Set my rank
uint8_t MlsLightEffects::getMyRank() {
  return this->my_rank;
}


// Set my column
void MlsLightEffects::setMyColumn(uint8_t column) {
  this->my_column = column;
}


// Set my rank
void MlsLightEffects::setMyRank(uint8_t rank) {
  this->my_rank = rank;
}


// Calculate the effective brightness of the LEDs (gamma correction is done when writing in the strips memory addresses)
struct CRGB MlsLightEffects::adjustBrightness(struct CRGB color, uint8_t brightness) {
  return CRGB((brightness * color.r) / 255, (brightness * color.g) / 255, (brightness * color.b) / 255);
}


// Set the strip data for the HEARTBEAT effect
void MlsLightEffects::effectHeartbeat(struct STRIP_DATA *actual_data, struct CRGB *strip) {
  struct CRGB new_color;
  struct CRGB diastole_color;
  uint32_t systole_fadein_time_micros = 50 * actual_data->duration_ms;    // 5% of total duration
  uint32_t systole_on_time_micros = 100 * actual_data->duration_ms;       // 10% of total duration
  uint32_t systole_fadeout_time_micros = 300 * actual_data->duration_ms;  // 25% of total duration (but diastole is coming before the end)
  uint32_t diastole_time_micros = 350 * actual_data->duration_ms;         // Diastole appears at 35% of whole signal
  uint32_t diastole_fadein_time_micros = 0;
  uint32_t diastole_on_time_micros = 100 * actual_data->duration_ms;      // 10% of total duration
  uint32_t diastole_fadeout_time_micros = 450 * actual_data->duration_ms; // 45% of total duration

  memcpy(new_color.raw, actual_data->color_raw, 3);
  diastole_color = this->adjustBrightness(new_color, actual_data->option);
  if (((actual_data->delta_time_micros) >= diastole_time_micros) && (diastole_fadein_time_micros > 0) && ((actual_data->delta_time_micros - diastole_time_micros) <= (diastole_fadein_time_micros))) {
    new_color = this->adjustBrightness(diastole_color, 255 * (actual_data->delta_time_micros - diastole_time_micros)  / diastole_fadein_time_micros);
  } else if (((actual_data->delta_time_micros) >= diastole_time_micros) && ((actual_data->delta_time_micros - diastole_time_micros) < (diastole_fadein_time_micros + diastole_on_time_micros))) {
    new_color = diastole_color;
  } else if (((actual_data->delta_time_micros) >= diastole_time_micros) && (diastole_fadeout_time_micros > 0) && ((actual_data->delta_time_micros - diastole_time_micros) < (diastole_fadein_time_micros + diastole_on_time_micros + diastole_fadeout_time_micros))) {
    new_color = this->adjustBrightness(diastole_color, 255 * (diastole_time_micros
    + diastole_fadeout_time_micros - ((actual_data->delta_time_micros) - diastole_fadein_time_micros - diastole_on_time_micros)) / diastole_fadeout_time_micros);
  } else if ((actual_data->delta_time_micros) >= (diastole_time_micros + diastole_time_micros + diastole_on_time_micros + diastole_fadeout_time_micros)) {
    new_color = CRGB::Black;
  } else if ((actual_data->delta_time_micros >= 0) && (systole_fadein_time_micros > 0) && (actual_data->delta_time_micros < systole_fadein_time_micros)) {
    new_color = this->adjustBrightness(new_color, 255 * actual_data->delta_time_micros / systole_fadein_time_micros);
  } else if ((actual_data->delta_time_micros >= 0) && (actual_data->delta_time_micros < (systole_fadein_time_micros + systole_on_time_micros))) {
  } else if ((actual_data->delta_time_micros >= 0) && (systole_fadeout_time_micros > 0) && (actual_data->delta_time_micros < (systole_fadein_time_micros + systole_on_time_micros + systole_fadeout_time_micros))) {
    new_color = this->adjustBrightness(new_color, 255 * (systole_fadeout_time_micros - (actual_data->delta_time_micros - systole_fadein_time_micros - systole_on_time_micros)) / systole_fadeout_time_micros);
  } else {
    new_color = CRGB::Black;
  }
  this->fill(new_color, actual_data->leds_per_strip, strip);
}


// Set the strip data for the FLASH effect
void MlsLightEffects::effectBreath(struct STRIP_DATA *actual_data, struct CRGB *strip) {
  struct CRGB new_color;
  uint32_t breath_fadein_time_micros = 350 * actual_data->duration_ms;
  uint32_t breath_on_time_micros = 100 * actual_data->duration_ms;
  uint32_t breath_fadeout_time_micros = 350 * actual_data->duration_ms;

  memcpy(new_color.raw, actual_data->color_raw, 3);
  if ((actual_data->delta_time_micros >= 0) && (breath_fadein_time_micros > 0) && (actual_data->delta_time_micros < breath_fadein_time_micros)) {
    new_color = this->adjustBrightness(new_color, 63 + (192 * actual_data->delta_time_micros / breath_fadein_time_micros));
  } else if ((actual_data->delta_time_micros >= 0) && (actual_data->delta_time_micros < (breath_fadein_time_micros + breath_on_time_micros))) {
  } else if ((actual_data->delta_time_micros >= 0) && (breath_fadeout_time_micros > 0) && (actual_data->delta_time_micros < (breath_fadein_time_micros + breath_on_time_micros + breath_fadeout_time_micros))) {
    new_color = this->adjustBrightness(new_color, 63 + (192 * (breath_fadeout_time_micros - (actual_data->delta_time_micros - breath_fadein_time_micros - breath_on_time_micros)) / breath_fadeout_time_micros));
  } else {
    new_color = this->adjustBrightness(new_color, 63);
  }
  this->fill(new_color, actual_data->leds_per_strip, strip);
}


// Set the strip data for the FIREFLY effect
void MlsLightEffects::effectFirefly(struct STRIP_DATA *actual_data, struct CRGB *strip) {

  FIREFLY_COLOR firefly_color;
  struct CRGB new_color;

  // OK, let's try to create a new firefly
  if (actual_data->delta_time_micros > (1000 * actual_data->duration_ms)) {
    if (0 == random(0,FIREFLIES_PROBABILITY)) {
      actual_data->duration_ms = random(FIREFLIES_LIFE_MINIMUM, FIREFLIES_LIFE_MAXIMUM);
      actual_data->option = 1; // a firefly is born :-)
      actual_data->fadein_time_micros = 1000 * random(FIREFLIES_FADEIN_MINIMUM, FIREFLIES_FADEIN_MAXIMUM);
      actual_data->fadeout_time_micros = 1000 * random(FIREFLIES_FADEOUT_MAXIMUM, FIREFLIES_FADEOUT_MAXIMUM);
      actual_data->on_time_micros = (1000 * actual_data->duration_ms) - actual_data->fadein_time_micros - actual_data->fadeout_time_micros;
      firefly_color.raw = FirefliesColorPalette[random(0, FIREFLIES_COLORS)];
      actual_data->color_r = firefly_color.one[2]; actual_data->color_g = firefly_color.one[1]; actual_data->color_b = firefly_color.one[0];
      DEBUG_PRINT("A firefly is born for "); DEBUG_PRINT(actual_data->duration_ms); DEBUG_PRINTLN(" ms");
    } else {
      actual_data->duration_ms = random(FIREFLIES_GAP_MINIMUM, FIREFLIES_GAP_MAXIMUM);
      actual_data->option = 0;
      actual_data->color_r = 0; actual_data->color_g = 0; actual_data->color_b = 0;
      DEBUG_PRINT("No firefly for "); DEBUG_PRINT(actual_data->duration_ms); DEBUG_PRINTLN(" ms");
    }
    actual_data->start_time_micros = micros();
    actual_data->delta_time_micros = 0;
  }
  if (actual_data->option > 0) {
    memcpy(new_color.raw, actual_data->color_raw, 3);
    if ((actual_data->delta_time_micros >= 0) && (actual_data->fadein_time_micros > 0) && (actual_data->delta_time_micros < actual_data->fadein_time_micros)) {
      new_color = this->adjustBrightness(new_color, 255 * actual_data->delta_time_micros / actual_data->fadein_time_micros);
    } else if ((actual_data->delta_time_micros >= 0) && (actual_data->delta_time_micros < (actual_data->fadein_time_micros + actual_data->on_time_micros))) {
    } else if ((actual_data->delta_time_micros >= 0) && (actual_data->fadeout_time_micros > 0) && (actual_data->delta_time_micros < (actual_data->fadein_time_micros + actual_data->on_time_micros + actual_data->fadeout_time_micros))) {
      new_color = this->adjustBrightness(new_color, 255 * (actual_data->fadeout_time_micros - (actual_data->delta_time_micros - actual_data->fadein_time_micros - actual_data->on_time_micros)) / actual_data->fadeout_time_micros);
    } else {
      new_color = CRGB::Black;
    }
  } else {
    new_color = CRGB::Black;
  }
  this->fill(new_color, actual_data->leds_per_strip, strip);
}


// Set the strip data for the STARS effect
void MlsLightEffects::effectStars(struct STRIP_DATA *actual_data, struct CRGB *strip) {

  struct CRGB new_color;

  // OK, let's try to create a new star
  if (actual_data->delta_time_micros > (1000 * actual_data->duration_ms)) {
    if (0 == random(0,STARS_PROBABILITY)) {
      actual_data->option = 1; // a star is born :-)
      actual_data->fadein_time_micros = 1000 * random(STARS_FADEIN_MINIMUM, STARS_FADEIN_MAXIMUM);
      actual_data->fadeout_time_micros = 1000 * random(STARS_FADEOUT_MAXIMUM, STARS_FADEOUT_MAXIMUM);
      actual_data->on_time_micros = 0;
      actual_data->duration_ms = (actual_data->fadein_time_micros + actual_data->fadeout_time_micros) / 1000;
      DEBUG_PRINT("A star is born for "); DEBUG_PRINT(actual_data->duration_ms); DEBUG_PRINTLN(" ms");
    } else {
      actual_data->duration_ms = random(STARS_GAP_MINIMUM, STARS_GAP_MAXIMUM);
      actual_data->option = 0;
      DEBUG_PRINT("No star for "); DEBUG_PRINT(actual_data->duration_ms); DEBUG_PRINTLN(" ms");
    }
    actual_data->start_time_micros = micros();
    actual_data->delta_time_micros = 0;
  }
  if (actual_data->option > 0) {
    new_color = CRGB::White;
    if ((actual_data->delta_time_micros >= 0) && (actual_data->fadein_time_micros > 0) && (actual_data->delta_time_micros < actual_data->fadein_time_micros)) {
      new_color = this->adjustBrightness(new_color, 255 * actual_data->delta_time_micros / actual_data->fadein_time_micros);
    } else if ((actual_data->delta_time_micros >= 0) && (actual_data->delta_time_micros < (actual_data->fadein_time_micros + actual_data->on_time_micros))) {
    } else if ((actual_data->delta_time_micros >= 0) && (actual_data->fadeout_time_micros > 0) && (actual_data->delta_time_micros < (actual_data->fadein_time_micros + actual_data->on_time_micros + actual_data->fadeout_time_micros))) {
      new_color = this->adjustBrightness(new_color, 255 * (actual_data->fadeout_time_micros - (actual_data->delta_time_micros - actual_data->fadein_time_micros - actual_data->on_time_micros)) / actual_data->fadeout_time_micros);
    } else {
      new_color = CRGB::Black;
    }
  } else {
    new_color = CRGB::Black;
  }
  this->fill(new_color, actual_data->leds_per_strip, strip);
}


// Set the strip data for the CHECK effect
void MlsLightEffects::effectCheck(struct STRIP_DATA *actual_data, struct CRGB *strip, uint8_t lr) {
  struct CRGB new_color;
  struct CRGB strip_color;
  struct CRGB place_color;
  struct CRGB position_color;
  uint8_t my_place;
  uint8_t max_steps;
  uint8_t max_leds;
  uint8_t led_position;
  boolean my_place_now;
  uint32_t reference_time;

  max_steps = this->number_of_ranks + this->number_of_columns;

  actual_data->step = (actual_data->repeat_counter/(CHECK_RANK_TIME_MS/CHECK_RESEND_TIME_MS)) % max_steps;

  if (actual_data->step < this->number_of_ranks) {
    led_position = actual_data->step;
  } else {
    led_position = actual_data->step - this->number_of_ranks;
  }
  my_place_now = false;

  strip_color = MLS_FADED_BLUE;
  position_color = MLS_FADED_BLUE;

  if (lr == 0) {
    // Ranks on left
    max_leds = this->number_of_ranks;
    place_color = CRGB::Green;
    my_place = this->my_rank;
    if (actual_data->step < this->number_of_ranks) { // Added to do ranks first, and columns after
      position_color = MLS_ORANGE;
      if (my_place == (1 + led_position)) {
        my_place_now = true;
        strip_color = CRGB::Green;
      }
    }
  } else {
    // Columns on right
    max_leds = this->number_of_columns;
    place_color = CRGB::Red;
    my_place = this->my_column;
    if (actual_data->step >= this->number_of_ranks) { // Added to do ranks first, and columns after
      position_color = MLS_ORANGE;
      if (my_place == (1 + led_position)) {
        my_place_now = true;
        strip_color = CRGB::Red;
      }
    }
  }

  if (my_place_now) {
    max_leds = actual_data->leds_per_strip;
  }

  for (uint16_t i = 0; i < max_leds; i++) {
    if (my_place == (i + 1)) {
      new_color = place_color;
    }
    else if (i == led_position) {
      new_color = position_color;
    } else {
      new_color = strip_color;
    }
    strip[i] = CRGB(pgm_read_byte(&gamma8[new_color.r]),  pgm_read_byte(&gamma8[new_color.g]),  pgm_read_byte(&gamma8[new_color.b]));
  }
  for (uint16_t i = max_leds; i < actual_data->leds_per_strip; i++) {
    new_color = CRGB::Black;
    strip[i] = CRGB(pgm_read_byte(&gamma8[new_color.r]),  pgm_read_byte(&gamma8[new_color.g]),  pgm_read_byte(&gamma8[new_color.b]));
  }
  
  if (actual_data->last_step != actual_data->step) {
    actual_data->last_step = actual_data->step;
  }
}


// Set the strip data for the PROGRESS effect
void MlsLightEffects::effectProgress(struct STRIP_DATA *actual_data, struct CRGB *strip) {
  struct CRGB new_color;

  actual_data->step = (actual_data->option * actual_data->delta_time_micros / (1000 * actual_data->duration_ms)) % actual_data->option;

  for (uint16_t i = 0; i < actual_data->leds_per_strip; i++) {
    if (actual_data->step == (i % actual_data->option)) {
      new_color = CRGB(actual_data->color_r, actual_data->color_g, actual_data->color_b);
    } else {
      new_color = CRGB::Black;
    }
    strip[i] = CRGB(pgm_read_byte(&gamma8[new_color.r]),  pgm_read_byte(&gamma8[new_color.g]),  pgm_read_byte(&gamma8[new_color.b]));
  }
  if (actual_data->last_step != actual_data->step) {
    actual_data->last_step = actual_data->step;
  }
}


void MlsLightEffects::effectVueMeter(struct STRIP_DATA *actual_data, struct CRGB *strip) {
  struct CRGB new_color;
  uint32_t shift_delay_micros;
  uint32_t fadein_time_micros = 0;
  uint32_t fadeout_time_micros;
  uint32_t on_time_micros;
  uint32_t rank_start_delay_micros;
  uint8_t effective_ranks = this->number_of_ranks;
  uint8_t effective_rank = this->my_rank;

  if (effective_ranks < 2) {
    effective_ranks = 2;
  }
  if (effective_rank < 1) {
    effective_rank = 1;
  }

  shift_delay_micros = 1000 * (actual_data->duration_ms - actual_data->option) / (2 * (effective_ranks - 1));
  rank_start_delay_micros = (effective_rank - 1) * shift_delay_micros;
  fadeout_time_micros = shift_delay_micros;
  on_time_micros = (1000 * actual_data->option) + (2 * (effective_ranks - effective_rank) * shift_delay_micros);

  new_color = CRGB::Green;
  if ((effective_ranks - effective_rank) <= 1) {
    new_color = CRGB::Red;
   fadeout_time_micros = shift_delay_micros;
  }

  if (this->my_rank > 1) {
    fadein_time_micros = shift_delay_micros;
    rank_start_delay_micros = rank_start_delay_micros - fadein_time_micros;
  }

/*
  DEBUG_PRINT("this->my_rank: "); DEBUG_PRINTLN(this->my_rank);
  DEBUG_PRINT("effective_ranks: "); DEBUG_PRINTLN(effective_ranks);
  DEBUG_PRINT("effective_rank: "); DEBUG_PRINTLN(effective_rank);
  DEBUG_PRINT("rank_start_delay_micros: "); DEBUG_PRINTLN(rank_start_delay_micros);
  DEBUG_PRINT("actual_data->leds_per_strip: "); DEBUG_PRINTLN(actual_data->leds_per_strip);
  DEBUG_PRINT("fadein_time_micros: "); DEBUG_PRINTLN(fadein_time_micros);
  DEBUG_PRINT("on_time_micros: "); DEBUG_PRINTLN(on_time_micros);
  DEBUG_PRINT("fadeout_time_micros: "); DEBUG_PRINTLN(fadeout_time_micros);
*/

  if ((actual_data->delta_time_micros >= rank_start_delay_micros) && (fadein_time_micros > 0) && (actual_data->delta_time_micros < (rank_start_delay_micros + fadein_time_micros))) {
    new_color = this->adjustBrightness(new_color, 255 * (actual_data->delta_time_micros - rank_start_delay_micros) / fadein_time_micros);
  } else if ((actual_data->delta_time_micros >= rank_start_delay_micros) && (actual_data->delta_time_micros < (rank_start_delay_micros + fadein_time_micros + on_time_micros))) {
  } else if ((actual_data->delta_time_micros >= rank_start_delay_micros) && (fadeout_time_micros > 0) && (actual_data->delta_time_micros < (rank_start_delay_micros + fadein_time_micros + on_time_micros + fadeout_time_micros))) {
    new_color = this->adjustBrightness(new_color, 255 * (fadeout_time_micros - (actual_data->delta_time_micros - rank_start_delay_micros - fadein_time_micros - on_time_micros)) / fadeout_time_micros);
  } else {
    new_color = CRGB::Black;
  }
  this->fill(new_color, actual_data->leds_per_strip, strip);
}


void MlsLightEffects::effectWaveBack(struct STRIP_DATA *actual_data, struct CRGB *strip) {
  struct CRGB new_color;
  uint32_t shift_delay_micros;
  uint32_t fadein_time_micros = 0;
  uint32_t fadeout_time_micros;
  uint32_t on_time_micros;
  uint32_t rank_start_delay_micros;
  uint8_t effective_ranks = this->number_of_ranks;
  uint8_t effective_rank = this->my_rank;

  if (effective_ranks < 2) {
    effective_ranks = 2;
  }
  if (effective_rank < 1) {
    effective_rank = 1;
  }

  shift_delay_micros = 1000 * (actual_data->duration_ms - actual_data->option) / (effective_ranks - 1);
  rank_start_delay_micros = (effective_rank - 1) * shift_delay_micros;
  fadeout_time_micros = shift_delay_micros;
  on_time_micros = (1000 * actual_data->option) + ((effective_ranks - effective_rank) * shift_delay_micros);
  new_color = CRGB::Blue;

  if ((effective_ranks - effective_rank) <= 1) {
   fadeout_time_micros = shift_delay_micros;
  }

  if (this->my_rank > 1) {
    fadein_time_micros = shift_delay_micros;
    rank_start_delay_micros = rank_start_delay_micros - fadein_time_micros;
  }

  on_time_micros = on_time_micros + rank_start_delay_micros;

  if ((actual_data->delta_time_micros >= rank_start_delay_micros) && (fadein_time_micros > 0) && (actual_data->delta_time_micros < (rank_start_delay_micros + fadein_time_micros))) {
    new_color = this->adjustBrightness(new_color, 255 * (actual_data->delta_time_micros - rank_start_delay_micros) / fadein_time_micros);
  } else if ((actual_data->delta_time_micros >= rank_start_delay_micros) && (actual_data->delta_time_micros < (rank_start_delay_micros + fadein_time_micros + on_time_micros))) {
  } else if ((actual_data->delta_time_micros >= rank_start_delay_micros) && (fadeout_time_micros > 0) && (actual_data->delta_time_micros < (rank_start_delay_micros + fadein_time_micros + on_time_micros + fadeout_time_micros))) {
    new_color = this->adjustBrightness(new_color, 255 * (fadeout_time_micros - (actual_data->delta_time_micros - rank_start_delay_micros - fadein_time_micros - on_time_micros)) / fadeout_time_micros);
  } else {
    new_color = CRGB::Black;
  }
  this->fill(new_color, actual_data->leds_per_strip, strip);
}


// Set the strip data for the FLASH effect
void MlsLightEffects::effectFlash(struct STRIP_DATA *actual_data, struct CRGB *strip) {
  struct CRGB new_color;

  memcpy(new_color.raw, actual_data->color_raw, 3);
  if ((actual_data->delta_time_micros >= 0) && (actual_data->fadein_time_micros > 0) && (actual_data->delta_time_micros < actual_data->fadein_time_micros)) {
    new_color = this->adjustBrightness(new_color, 255 * actual_data->delta_time_micros / actual_data->fadein_time_micros);
  } else if ((actual_data->delta_time_micros >= 0) && (actual_data->delta_time_micros < (actual_data->fadein_time_micros + actual_data->on_time_micros))) {
  } else if ((actual_data->delta_time_micros >= 0) && (actual_data->fadeout_time_micros > 0) && (actual_data->delta_time_micros < (actual_data->fadein_time_micros + actual_data->on_time_micros + actual_data->fadeout_time_micros))) {
    new_color = this->adjustBrightness(new_color, 255 * (actual_data->fadeout_time_micros - (actual_data->delta_time_micros - actual_data->fadein_time_micros - actual_data->on_time_micros)) / actual_data->fadeout_time_micros);
  } else {
    new_color = CRGB::Black;
  }
  this->fill(new_color, actual_data->leds_per_strip, strip);
}


// Set the strip data for the THREE STEPS effect
void MlsLightEffects::effectThreeSteps(struct STRIP_DATA *actual_data, struct CRGB *strip) {
  struct CRGB new_color;

  if (0 == ((300 + actual_data->repeat_counter - this->my_rank) % 3)) {
    memcpy(new_color.raw, actual_data->color_raw, 3);
    if ((actual_data->delta_time_micros >= 0) && (actual_data->fadein_time_micros > 0) && (actual_data->delta_time_micros < actual_data->fadein_time_micros)) {
      new_color = this->adjustBrightness(new_color, 255 * actual_data->delta_time_micros / actual_data->fadein_time_micros);
    } else if ((actual_data->delta_time_micros >= 0) && (actual_data->delta_time_micros < (actual_data->fadein_time_micros + actual_data->on_time_micros))) {
    } else if ((actual_data->delta_time_micros >= 0) && (actual_data->fadeout_time_micros > 0) && (actual_data->delta_time_micros < (actual_data->fadein_time_micros + actual_data->on_time_micros + actual_data->fadeout_time_micros))) {
      new_color = this->adjustBrightness(new_color, 255 * (actual_data->fadeout_time_micros - (actual_data->delta_time_micros - actual_data->fadein_time_micros - actual_data->on_time_micros)) / actual_data->fadeout_time_micros);
    } else {
      new_color = CRGB::Black;
    }
  } else {
    new_color = CRGB::Black;
  }
  this->fill(new_color, actual_data->leds_per_strip, strip);
}


void MlsLightEffects::effectRainbowRankBeat(struct STRIP_DATA *actual_data, struct CRGB *strip) {
  struct CRGB new_color;

  new_color = CHSV(((255 * ((this->my_rank + actual_data->repeat_counter) % (1 + number_of_ranks)) / number_of_ranks)) % 256, 255, 255);
  if ((actual_data->delta_time_micros >= 0) && (actual_data->fadein_time_micros > 0) && (actual_data->delta_time_micros < actual_data->fadein_time_micros)) {
    new_color = this->adjustBrightness(new_color, 255 * actual_data->delta_time_micros / actual_data->fadein_time_micros);
  } else if ((actual_data->delta_time_micros >= 0) && (actual_data->delta_time_micros < (actual_data->fadein_time_micros + actual_data->on_time_micros))) {
  } else if ((actual_data->delta_time_micros >= 0) && (actual_data->fadeout_time_micros > 0) && (actual_data->delta_time_micros < (actual_data->fadein_time_micros + actual_data->on_time_micros + actual_data->fadeout_time_micros))) {
    new_color = this->adjustBrightness(new_color, 255 * (actual_data->fadeout_time_micros - (actual_data->delta_time_micros - actual_data->fadein_time_micros - actual_data->on_time_micros)) / actual_data->fadeout_time_micros);
  } else {
    new_color = CRGB::Black;
  }
  this->fill(new_color, actual_data->leds_per_strip, strip);
}


void MlsLightEffects::effectThreeStepsAlternate(struct STRIP_DATA *actual_data, struct CRGB *strip) {
  struct CRGB new_color;

  if (0 == ((300 + actual_data->repeat_counter - this->my_rank) % 3)) {
    if (0 == (actual_data->repeat_counter % 2)) {
      actual_data->color_r = 0; actual_data->color_g = 255; actual_data->color_b = 0;
    } else {
      actual_data->color_r = 255; actual_data->color_g = 0; actual_data->color_b = 0;
    }

    memcpy(new_color.raw, actual_data->color_raw, 3);
    if ((actual_data->delta_time_micros >= 0) && (actual_data->fadein_time_micros > 0) && (actual_data->delta_time_micros < actual_data->fadein_time_micros)) {
      new_color = this->adjustBrightness(new_color, 255 * actual_data->delta_time_micros / actual_data->fadein_time_micros);
    } else if ((actual_data->delta_time_micros >= 0) && (actual_data->delta_time_micros < (actual_data->fadein_time_micros + actual_data->on_time_micros))) {
    } else if ((actual_data->delta_time_micros >= 0) && (actual_data->fadeout_time_micros > 0) && (actual_data->delta_time_micros < (actual_data->fadein_time_micros + actual_data->on_time_micros + actual_data->fadeout_time_micros))) {
      new_color = this->adjustBrightness(new_color, 255 * (actual_data->fadeout_time_micros - (actual_data->delta_time_micros - actual_data->fadein_time_micros - actual_data->on_time_micros)) / actual_data->fadeout_time_micros);
    } else {
      new_color = CRGB::Black;
    }
  } else {
    new_color = CRGB::Black;
  }
  this->fill(new_color, actual_data->leds_per_strip, strip);
}


// Set the strip data for the FIXED effect
void MlsLightEffects::effectFixed(struct STRIP_DATA *actual_data, struct CRGB *strip) {
  struct CRGB new_color;

  memcpy(new_color.raw, actual_data->color_raw, 3);
  if ((actual_data->delta_time_micros >= 0) && (actual_data->fadein_time_micros > 0) && (actual_data->delta_time_micros < actual_data->fadein_time_micros)) {
    new_color = this->adjustBrightness(new_color, 255 * actual_data->delta_time_micros / actual_data->fadein_time_micros);
  } else if (actual_data->delta_time_micros < 0) {
    new_color = CRGB::Black;
  }
  this->fill(new_color, actual_data->leds_per_strip, strip);
}


// Set the strip data for the PROGRESS_RAINBOW effect
void MlsLightEffects::effectProgressRainbow(struct STRIP_DATA *actual_data, struct CRGB *strip) {
  int8_t light_wave = 255 - ((256 * actual_data->delta_time_micros / (1000 * actual_data->duration_ms)) % 256);
  for (uint16_t i = 0; i < actual_data->leds_per_strip; i++) {
    strip[i] = CHSV((light_wave + map(i % actual_data->leds_per_strip, 0, actual_data->leds_per_strip, 0, 255) % 256), 255, 255);
  }
}


// setLightData without latency informations
void MlsLightEffects::setLightData(uint16_t packetId, struct LIGHT_PACKET *lightPacket) {
  setLightData(packetId, lightPacket, 0);
}


// setLightData with latency informations
void MlsLightEffects::setLightData(uint16_t packetId, struct LIGHT_PACKET *lightPacket, uint32_t latency_micros) {
  if (this->received_packet != packetId) {
    this->received_packet = packetId;
    if (EFFECT_KEEP_ALIVE != lightPacket->effect) {
      // Overwrite some values for some effects
      if (EFFECT_FLASH_ALTERNATE == lightPacket->effect) {
        lightPacket->effect_modifier = MODIFIER_FLIP_FLOP;
        if (0 == (lightPacket->left_color_r + lightPacket->left_color_g +  lightPacket->left_color_b + lightPacket->right_color_r + lightPacket->right_color_g +  lightPacket->right_color_b)) {
          lightPacket->left_color_r = 255; lightPacket->left_color_g = 0; lightPacket->left_color_b = 0;
          lightPacket->right_color_r = 0; lightPacket->right_color_g = 255; lightPacket->right_color_b = 0;
        }
      }
      
      if (MODIFIER_IGNORE_LEFT != (lightPacket->effect_modifier & MODIFIER_IGNORE_LEFT)) {
        // DEBUG_PRINT("Packet color RGB left: ");
        // DEBUG_PRINT(lightPacket->left_color_r); DEBUG_PRINT("/"); DEBUG_PRINT(lightPacket->left_color_g); DEBUG_PRINT("/"); DEBUG_PRINTLN(lightPacket->left_color_b);
        this->data_received[0].received             = false;
        this->data_received[0].received_time_micros = micros();
        this->data_received[0].packet               = packetId;
        this->data_received[0].step                 = 0;
        this->data_received[0].last_step            = 0;
        this->data_received[0].leds_per_strip       = this->leds_per_strip;
        this->data_received[0].latency_micros       = latency_micros;
        this->data_received[0].start_time_micros    = this->data_received[0].received_time_micros + latency_micros;
        this->data_received[0].delta_time_micros    = 0;
        this->data_received[0].effect               = lightPacket->effect;
        this->data_received[0].effect_modifier      = lightPacket->effect_modifier;
        this->data_received[0].repeat_counter       = lightPacket->repeat_counter;
        this->data_received[0].duration_ms          = lightPacket->duration_ms;
        this->data_received[0].option               = lightPacket->option;
        this->data_received[0].color_r              = lightPacket->left_color_r;
        this->data_received[0].color_g              = lightPacket->left_color_g;
        this->data_received[0].color_b              = lightPacket->left_color_b;
        this->data_received[0].fadein_time_micros   = lightPacket->left_fadein_time * 10000;
        this->data_received[0].on_time_micros       = lightPacket->left_on_time * 10000;
        this->data_received[0].fadeout_time_micros  = lightPacket->left_fadeout_time * 10000;
        this->data_received[0].repeat               = (MODIFIER_REPEAT == (lightPacket->effect_modifier  & MODIFIER_REPEAT));
      }
      if (MODIFIER_IGNORE_RIGHT != (lightPacket->effect_modifier & MODIFIER_IGNORE_RIGHT)) {
        // DEBUG_PRINT("Packet color RGB right: ");
        // DEBUG_PRINT(lightPacket->right_color_r); DEBUG_PRINT("/"); DEBUG_PRINT(lightPacket->right_color_g); DEBUG_PRINT("/"); DEBUG_PRINTLN(lightPacket->right_color_b);
        this->data_received[1].received             = false;
        this->data_received[1].received_time_micros = micros();
        this->data_received[1].packet               = packetId;
        this->data_received[1].step                 = 0;
        this->data_received[1].last_step            = 0;
        this->data_received[1].leds_per_strip       = this->leds_per_strip;
        this->data_received[1].latency_micros       = latency_micros;
        this->data_received[1].start_time_micros    = this->data_received[1].received_time_micros + latency_micros;
        this->data_received[1].delta_time_micros    = 0;
        this->data_received[1].effect               = lightPacket->effect;
        this->data_received[1].effect_modifier      = lightPacket->effect_modifier;
        this->data_received[1].repeat_counter       = lightPacket->repeat_counter;
        this->data_received[1].duration_ms          = lightPacket->duration_ms;
        this->data_received[1].option               = lightPacket->option;
        this->data_received[1].color_r              = lightPacket->right_color_r;
        this->data_received[1].color_g              = lightPacket->right_color_g;
        this->data_received[1].color_b              = lightPacket->right_color_b;
        this->data_received[1].fadein_time_micros   = lightPacket->right_fadein_time * 10000;
        this->data_received[1].on_time_micros       = lightPacket->right_on_time * 10000;
        this->data_received[1].fadeout_time_micros  = lightPacket->right_fadeout_time * 10000;
        this->data_received[1].repeat               = (MODIFIER_REPEAT == (lightPacket->effect_modifier  & MODIFIER_REPEAT));
        this->data_received[1].received             = true;
      }
      if (MODIFIER_IGNORE_LEFT != (lightPacket->effect_modifier & MODIFIER_IGNORE_LEFT)) {
        this->data_received[0].received             = true;
      }
    }
  }
}


void MlsLightEffects::updateLight() {
  struct CRGB *current_strip;
  struct STRIP_DATA *actual_data;
  uint8_t effect_changed[2];

  uint16_t option = 0;

  // Loop for both strips (left and right) - prepare flip data
  for (uint8_t lr = 0; lr < 2; lr++) {
    this->data_actual[lr].applied = false;
    if ((this->data_received[lr].received) && ((micros() - this->data_received[lr].start_time_micros) >= 0)) {
      this->data_received[lr].received = false;
      effect_changed[lr] = (this->data_actual[lr].effect != this->data_received[lr].effect);
      memcpy(this->data_actual[lr].raw, this->data_received[lr].raw, STRIP_DATA_SIZE);
      memcpy(this->data_flip[lr].raw, this->data_actual[lr].flip_data, FLIP_DATA_SIZE);
      this->data_actual[lr].applied = true;
      // DEBUG_PRINT("*** updateLight *** "); DEBUG_PRINTLN(lr);
    }
  }

  // Loop for both strips (left and right)
  for (uint8_t lr = 0; lr < 2; lr++) {
    current_strip = NULL;
    if (0 == lr) {
      current_strip  = this->left_strip;
    } else {
      current_strip  = this->right_strip;
    }
    actual_data    = &this->data_actual[lr];

    if (actual_data->effect == EFFECT_NONE) {
      effect_changed[lr] = false;
      actual_data->applied = false;
      actual_data->repeat = false;
    }

    if (actual_data->duration_ms <= 0) {
      actual_data->repeat = false;
    }

    if (effect_changed[lr]) {
      actual_data->last_step = 65535;
      actual_data->start_time_micros = micros() - actual_data->latency_micros;
      this->play_counter[lr] = actual_data->repeat_counter;
      this->current_play_counter[lr] = actual_data->repeat_counter;
    } else if (actual_data->repeat) {
      this->play_counter[lr] = actual_data->repeat_counter + ((micros() - actual_data->start_time_micros) / (1000 * actual_data->duration_ms));
    } else {
      this->play_counter[lr] = actual_data->repeat_counter; // To be sure that the flip is syncrhonized between all musicians
    }

    if (actual_data->repeat) {
      actual_data->delta_time_micros  = (micros() - actual_data->start_time_micros) % (1000 * actual_data->duration_ms);
    } else {
      actual_data->delta_time_micros  = micros() - actual_data->start_time_micros;
    }

    if (effect_changed[lr] || actual_data->applied || (this->play_counter[lr] != this->current_play_counter[lr])) {

      if ((MODIFIER_FLIP_FLOP == (actual_data->effect_modifier & MODIFIER_FLIP_FLOP)) && (0 != (this->play_counter[lr] % 2))) {
        memcpy(actual_data->flip_data, this->data_flip[(lr + 1) % 2].raw, FLIP_DATA_SIZE);
      } else {
        memcpy(actual_data->flip_data, this->data_flip[lr].raw, FLIP_DATA_SIZE);
      }
    }

    this->current_play_counter[lr] = this->play_counter[lr];
    if ((!effect_changed[lr]) && actual_data->applied) {
      this->play_counter[lr]++;
    }

    if (current_strip != NULL) {
      switch (actual_data->effect) {
        case EFFECT_BLANK:
          this->data_actual[lr].effect = EFFECT_NONE;
          actual_data->effect = EFFECT_NONE;
          this->fill(CRGB::Black, actual_data->leds_per_strip, current_strip);
          break;
        case EFFECT_FLASH_YELLOW:
          actual_data->fadein_time_micros = 0;
          actual_data->on_time_micros = 30000;
          actual_data->fadeout_time_micros = 350000;
          actual_data->color_r = 255; actual_data->color_g = 165; actual_data->color_b = 0;
          this->effectFlash(actual_data, current_strip);
          break;
        case EFFECT_FLASH_ALTERNATE:
          actual_data->fadein_time_micros = 0;
          actual_data->on_time_micros = 30000;
          actual_data->fadeout_time_micros = 350000;
          this->effectFlash(actual_data, current_strip);
          break;
        case EFFECT_STROBE:
          actual_data->repeat = true;
          if (actual_data->duration_ms == 0) {
            actual_data->duration_ms = 120;
          }
          actual_data->fadein_time_micros = 0;
          actual_data->on_time_micros = 30000;
          actual_data->fadeout_time_micros = 0;
          actual_data->color_r = 255; actual_data->color_g = 255; actual_data->color_b = 255;
        case EFFECT_FLASH:
          this->effectFlash(actual_data, current_strip);
          break;
        case EFFECT_VUE_METER:
          if (actual_data->option == 0) {
            actual_data->option = 50; // Default minimum on time of the max level in ms 
          }
          if (actual_data->duration_ms == 0) {
            actual_data->duration_ms = 300; // Default effect duration
          }
          this->effectVueMeter(actual_data, current_strip);
          break;
        case EFFECT_WAVE_BACK:
          if (actual_data->option == 0) {
            actual_data->option = 50; // Default minimum on time for all in ms
          }
          if (actual_data->duration_ms == 0) {
            actual_data->duration_ms = 300; // Default effect duration
          }
          this->effectWaveBack(actual_data, current_strip);
          break;
        case EFFECT_3_STEPS_ALTERNATE:
          actual_data->fadein_time_micros = 0;
          actual_data->on_time_micros = 30000;
          actual_data->fadeout_time_micros = 350000;
          this->effectThreeStepsAlternate(actual_data, current_strip);
          break;
        case EFFECT_3_STEPS:
          actual_data->fadein_time_micros = 0;
          actual_data->on_time_micros = 30000;
          actual_data->fadeout_time_micros = 350000;
          if (0 == (actual_data->color_r + actual_data->color_g +  actual_data->color_b)) {
            if (0 == lr) {
              actual_data->color_r = 0; actual_data->color_g = 255; actual_data->color_b = 0;
            } else {
              actual_data->color_r = 255; actual_data->color_g = 0; actual_data->color_b = 0;
            }
          }
          this->effectThreeSteps(actual_data, current_strip);
          break;
        case EFFECT_RAINBOW_RANK_BEAT:
          actual_data->fadein_time_micros = 0;
          actual_data->on_time_micros = 30000;
          actual_data->fadeout_time_micros = 350000;
          this->effectRainbowRankBeat(actual_data, current_strip);
          break;
        case EFFECT_HEARTBEAT:
          actual_data->repeat = true;
          if (actual_data->duration_ms == 0) {
            actual_data->duration_ms = 1000; // Default effect duration
          }
          if (actual_data->option == 0) {
            actual_data->option = 224; // Default diastole brigthtness
          }
          if ((0 == actual_data->color_r) && (0 == actual_data->color_g) && (0 == actual_data->color_b)) {
            actual_data->color_r = 255; actual_data->color_g = 0; actual_data->color_b = 0;
          }
          this->effectHeartbeat(actual_data, current_strip);
          break;
        case EFFECT_PROGRESS4:
          actual_data->option = 4;
        case EFFECT_PROGRESS:
          actual_data->repeat = true;
          if (actual_data->duration_ms == 0) {
            actual_data->duration_ms = 300;
          }
          if (actual_data->option == 0) {
            actual_data->option = 3;
          }
          this->effectProgress(actual_data, current_strip);
          break;
        case EFFECT_PROGRESS_RAINBOW:
          actual_data->repeat = true;
          if (actual_data->duration_ms == 0) {
            actual_data->duration_ms = 300;
          }
          this->effectProgressRainbow(actual_data, current_strip);
          break;
        case EFFECT_FIXED:
          this->effectFixed(actual_data, current_strip);
          break;
        case EFFECT_CHECK:
          this->effectCheck(actual_data, current_strip, lr);
          break;
        case EFFECT_POLICE:
          // this->effectPolice(actual_data, current_strip);
          break;
        case EFFECT_BREATH:
          actual_data->repeat = true;
          if (actual_data->duration_ms == 0) {
            actual_data->duration_ms = 5000; // Default effect duration
          }
          if ((0 == actual_data->color_r) && (0 == actual_data->color_g) && (0 == actual_data->color_b)) {
            actual_data->color_r = 0; actual_data->color_g = 255; actual_data->color_b = 0;
          }
          this->effectBreath(actual_data, current_strip);
          break;
        case EFFECT_FIREFLY:
          if (actual_data->duration_ms == 0) {
            actual_data->duration_ms = random(1, FIREFLIES_GAP_MAXIMUM); // Default effect duration
          }
          this->effectFirefly(actual_data, current_strip);
          break;
        case EFFECT_STARS:
          if (actual_data->duration_ms == 0) {
            actual_data->duration_ms = random(1, STARS_GAP_MAXIMUM); // Default effect duration
          }
          this->effectStars(actual_data, current_strip);
          break;
        default:
          break;
      }
    }
  }
  this->showLeds();
}


// Fill with a specific color the number of LEDs of a strip
void MlsLightEffects::fill(struct CRGB color, uint16_t number_of_leds, struct CRGB *strip) {
  for (uint16_t i = 0; i < number_of_leds; i++) {
    strip[i] = CRGB(pgm_read_byte(&gamma8[color.r]),  pgm_read_byte(&gamma8[color.g]),  pgm_read_byte(&gamma8[color.b]));
  }
}


// Set a value with specific two colors for a strip
void MlsLightEffects::setValue(uint16_t value, uint16_t maxValue, struct CRGB colorOffMin, struct CRGB colorOffMax, struct CRGB colorOnMin, struct CRGB colorOnMax, struct CRGB *strip) {
  for (uint16_t i = 0; i < value; i++) {
    if (i < maxValue / 2) {
      strip[i] = CRGB(pgm_read_byte(&gamma8[colorOnMin.r]),  pgm_read_byte(&gamma8[colorOnMin.g]),  pgm_read_byte(&gamma8[colorOnMin.b]));
    } else {
      strip[i] = CRGB(pgm_read_byte(&gamma8[colorOnMax.r]),  pgm_read_byte(&gamma8[colorOnMax.g]),  pgm_read_byte(&gamma8[colorOnMax.b]));
    }
  }
  for (uint16_t i = value; i < maxValue; i++) {
    if (i < maxValue / 2) {
      strip[i] = CRGB(pgm_read_byte(&gamma8[colorOffMin.r]),  pgm_read_byte(&gamma8[colorOffMin.g]),  pgm_read_byte(&gamma8[colorOffMin.b]));
    } else {
      strip[i] = CRGB(pgm_read_byte(&gamma8[colorOffMax.r]),  pgm_read_byte(&gamma8[colorOffMax.g]),  pgm_read_byte(&gamma8[colorOffMax.b]));
    }
  }
  for (uint16_t i = maxValue; i < this->leds_per_strip; i++) {
    strip[i] = CRGB::Black;
  }
}


// Set a value with specific three colors for a strip
void MlsLightEffects::setValueThree(uint16_t value, uint16_t maxValue, struct CRGB colorOff, struct CRGB colorOn1, struct CRGB colorOn2, struct CRGB colorOn3, struct CRGB *strip) {
  for (uint16_t i = 0; i < value; i++) {
    switch (i % 3) {
      case 0:
        strip[i] = CRGB(pgm_read_byte(&gamma8[colorOn1.r]),  pgm_read_byte(&gamma8[colorOn1.g]),  pgm_read_byte(&gamma8[colorOn1.b]));
        break;
      case 1:
        strip[i] = CRGB(pgm_read_byte(&gamma8[colorOn2.r]),  pgm_read_byte(&gamma8[colorOn2.g]),  pgm_read_byte(&gamma8[colorOn2.b]));
        break;
      case 2:
        strip[i] = CRGB(pgm_read_byte(&gamma8[colorOn3.r]),  pgm_read_byte(&gamma8[colorOn3.g]),  pgm_read_byte(&gamma8[colorOn3.b]));
        break;
    }
  }
  for (uint16_t i = value; i < maxValue; i++) {
    strip[i] = CRGB(pgm_read_byte(&gamma8[colorOff.r]),  pgm_read_byte(&gamma8[colorOff.g]),  pgm_read_byte(&gamma8[colorOff.b]));
  }
  for (uint16_t i = maxValue; i < this->leds_per_strip; i++) {
    strip[i] = CRGB::Black;
  }
}


void MlsLightEffects::showLeds() {
  // Warning! The ledp strip structure is on 3 bytes !!!
  if ((memcmp(this->left_strip, this->last_left_strip, 3 * this->leds_per_strip) != 0) || (memcmp(this->right_strip, this->last_right_strip, 3 * this->leds_per_strip) != 0)) {
    memcpy(last_left_strip,  left_strip,  3 * this->leds_per_strip);
    memcpy(last_right_strip, right_strip, 3 * this->leds_per_strip);
    FastLED.show();
  }
}


void MlsLightEffects::clearLeds() {
  for (uint16_t i = 0; i < this->leds_per_strip; i++) {
    this->left_strip[i]  = CRGB::Black;
    this->right_strip[i] = CRGB::Black;
  }
}


// Stop all updates
void MlsLightEffects::stopUpdate() {
  DEBUG_PRINTLN("MLS Light effect: Stop update");
  struct LIGHT_PACKET blank_packet;
  blank_packet = (LIGHT_PACKET) {EFFECT_BLANK, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  this->setLightData(0, &blank_packet);
}
