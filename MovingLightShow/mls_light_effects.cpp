/**********************************************************************
 *
 * MovingLightShow package - Synchronized LED strips for musicians
 * https://MovingLightShow.art
 *
 * @file  mls_light_effects.cpp
 * @brief Light effects
 * 
 **********************************************************************/
#include "mls_light_effects.h"


MlsLightEffects::MlsLightEffects(uint16_t leds_per_strip, struct CRGB *left_strip, struct CRGB *right_strip) {
  this->leds_per_strip = leds_per_strip;
  this->left_strip = left_strip;
  this->right_strip = right_strip;
  this->light_packet.effect = 0;
  this->current_packet = 0;
  this->current_effect = 0;
}


struct CRGB MlsLightEffects::adjustBrightness(struct CRGB color, uint8_t brightness) {
  return CRGB((brightness * color.r) / 255, (brightness * color.g) / 255, (brightness * color.b) / 255);
}


void MlsLightEffects::effectProgress() {
  this->effectProgress(3);
}


void MlsLightEffects::effectProgress(uint8_t steps) {

  struct CRGB new_color;

  int8_t light_step = (steps * left_data.delta_time / (1000 * this->light_packet.option)) % steps;
 
  if (0 == (this->light_packet.effect_modifier & MODIFIER_IGNORE_LEFT)) {
    for (uint16_t i=0; i < this->leds_per_strip; i++) {
      if (light_step == (i % steps)) {
        new_color = CRGB(left_data.color_r, left_data.color_g, left_data.color_b);
      } else {
        new_color = CRGB::Black;
      }
      this->left_strip[i] = CRGB(new_color);
    }
  }
  if (0 == (this->light_packet.effect_modifier & MODIFIER_IGNORE_RIGHT)) {
    for (uint16_t i=0; i < this->leds_per_strip; i++) {
      if (light_step == ( i %steps)) {
        new_color = CRGB(right_data.color_r, right_data.color_g, right_data.color_b);
      } else {
        new_color = CRGB::Black;
      }
      this->right_strip[i] = CRGB(new_color);
    }
  }
  FastLED.show();
}


void MlsLightEffects::effectFlash() {

  struct CRGB new_color;

  if (0 == (this->light_packet.effect_modifier & MODIFIER_IGNORE_LEFT)) {
    memcpy(new_color.raw, left_data.color_raw, 3);
    if ((this->left_data.fadein_time > 0) && (this->left_data.delta_time < this->left_data.fadein_time)) {
      new_color = this->adjustBrightness(new_color, 255 * this->left_data.delta_time / this->left_data.fadein_time);
    } else if (this->left_data.delta_time < (this->left_data.fadein_time + this->left_data.on_time)) {
    } else if ((this->left_data.fadeout_time > 0) && (this->left_data.delta_time < (this->left_data.fadein_time + this->left_data.on_time + this->left_data.fadeout_time))) {
      new_color = this->adjustBrightness(new_color, 255 * (this->left_data.fadeout_time - (this->left_data.delta_time - left_data.fadein_time - this->left_data.on_time)) / this->left_data.fadeout_time);
    } else {
      new_color = CRGB::Black;
    }
    this->fillLeft(new_color);
  }
  if (0 == (this->light_packet.effect_modifier & MODIFIER_IGNORE_RIGHT)) {
    memcpy(new_color.raw, right_data.color_raw, 3);
    if ((this->right_data.fadein_time > 0) && (this->right_data.delta_time < this->right_data.fadein_time)) {
      new_color = this->adjustBrightness(new_color, 255 * this->right_data.delta_time / this->right_data.fadein_time);
    } else if (right_data.delta_time < (this->right_data.fadein_time + this->right_data.on_time)) {
    } else if ((this->right_data.fadeout_time > 0) && (this->right_data.delta_time < (this->right_data.fadein_time + this->right_data.on_time + this->right_data.fadeout_time))) {
      new_color = this->adjustBrightness(new_color, 255 * (this->right_data.fadeout_time - (this->right_data.delta_time - right_data.fadein_time - this->right_data.on_time)) / this->right_data.fadeout_time);
    } else {
      new_color = CRGB::Black;
    }
    this->fillRight(new_color);
  }
  FastLED.show();
}


void MlsLightEffects::effectFixed() {

  struct CRGB new_color;

  if (0 == (this->light_packet.effect_modifier & MODIFIER_IGNORE_LEFT)) {
    memcpy(new_color.raw, left_data.color_raw, 3);
    this->fillLeft(new_color);
  }
  if (0 == (this->light_packet.effect_modifier & MODIFIER_IGNORE_RIGHT)) {
    memcpy(new_color.raw, right_data.color_raw, 3);
    this->fillRight(new_color);
  }
  FastLED.show();
}


void MlsLightEffects::setLightData(uint16_t packetId, struct LIGHT_PACKET lightPacket) {

  if (this->received_packet != packetId) {
    this->received_packet = packetId;
    if (EFFECT_KEEP_ALIVE != lightPacket.effect) {
      memcpy(this->received_light_packet.raw, lightPacket.raw, LIGHT_PACKET_SIZE);
      this->lightPackedReceived = true;
    }
  }
}


void MlsLightEffects::updateLight() {
  bool effect_changed = false;
  bool repeat = false;
  uint16_t option = 0;

  if (this->lightPackedReceived) {
    effect_changed = (this->current_effect != this->received_light_packet.effect);
    this->current_effect = this->received_light_packet.effect;
    memcpy(this->light_packet.raw, this->received_light_packet.raw, LIGHT_PACKET_SIZE);
  }

  repeat = (0 != (this->light_packet.effect_modifier & MODIFIER_REPEAT));

  // Check/correct options for some specific effects
  switch (this->current_effect) {
    case EFFECT_STROBE:
      repeat = true;
      if (this->light_packet.option == 0) {
        this->light_packet.option = 100;
      }
      break;
    case EFFECT_PROGRESS:
      repeat = true;
      if (this->light_packet.option == 0) {
        this->light_packet.option = 300;
      }
      break;
    case EFFECT_PROGRESS_RAINBOW:
      repeat = true;
      if (this->light_packet.option == 0) {
        this->light_packet.option = 300;
      }
      break;
  }

  if (this->light_packet.option <= 0) {
    repeat = false;
  }

  if (this->current_effect == EFFECT_NONE) {
    effect_changed = false;
  }

  if (effect_changed) {
    this->play_counter = 0;
    this->current_play_counter = 0;
    this->effect_start_time = micros();
  } else if (repeat) {
    this->play_counter = ((micros() - this->effect_start_time) / (1000 * this->light_packet.option));
  }

  if (effect_changed || this->lightPackedReceived || (this->play_counter != this->current_play_counter)) {
    if ((0 != (this->light_packet.effect_modifier & MODIFIER_FLIP_FLOP)) && (0 != (this->play_counter % 2))) {
      // LEFT strip not disabled
      if (0 == (this->light_packet.effect_modifier & MODIFIER_IGNORE_LEFT)) {
        this->left_data.start_time = micros();
        this->left_data.fadein_time = this->light_packet.right_fadein_time * 10000; // From 10ms to microseconds
        this->left_data.on_time = this->light_packet.right_on_time * 10000; // From 10ms to microseconds
        this->left_data.fadeout_time = this->light_packet.right_fadeout_time * 10000; // From 10ms to microseconds
        memcpy(this->left_data.color_raw, this->light_packet.right_color_raw, 3);
      }
      // RIGHT strip not disabled
      if (0 == (this->light_packet.effect_modifier & MODIFIER_IGNORE_RIGHT)) {
        this->right_data.start_time = micros();
        this->right_data.fadein_time = this->light_packet.left_fadein_time * 10000; // From 10ms to microseconds
        this->right_data.on_time = this->light_packet.left_on_time * 10000; // From 10ms to microseconds
        this->right_data.fadeout_time = this->light_packet.left_fadeout_time * 10000; // From 10ms to microseconds
        memcpy(this->right_data.color_raw, this->light_packet.left_color_raw, 3);
      }
    } else { // Flip-flop mode
      // LEFT strip not disabled
      if (0 == (this->light_packet.effect_modifier & MODIFIER_IGNORE_LEFT)) {
        this->left_data.start_time = micros();
        this->left_data.fadein_time = this->light_packet.left_fadein_time * 10000; // From 10ms to microseconds
        this->left_data.on_time = this->light_packet.left_on_time * 10000; // From 10ms to microseconds
        this->left_data.fadeout_time = this->light_packet.left_fadeout_time * 10000; // From 10ms to microseconds
        memcpy(this->left_data.color_raw, this->light_packet.left_color_raw, 3);
      }
      // RIGHT strip not disabled
      if (0 == (this->light_packet.effect_modifier & MODIFIER_IGNORE_RIGHT)) {
        this->right_data.start_time = micros();
        this->right_data.fadein_time = this->light_packet.right_fadein_time * 10000; // From 10ms to microseconds
        this->right_data.on_time = this->light_packet.right_on_time * 10000; // From 10ms to microseconds
        this->right_data.fadeout_time = this->light_packet.right_fadeout_time * 10000; // From 10ms to microseconds
        memcpy(this->right_data.color_raw, this->light_packet.right_color_raw, 3);
      }
    }
  }

  this->current_play_counter = this->play_counter;
  if ((!effect_changed) && this->lightPackedReceived) {
    this->play_counter++;
  }
  this->lightPackedReceived = false;

  if (repeat) {
    left_data.delta_time = (micros() - this->left_data.start_time) % (1000 * this->light_packet.option);
    right_data.delta_time = (micros() - this->right_data.start_time) % (1000 * this->light_packet.option);
  } else {
    left_data.delta_time = (micros() - this->left_data.start_time);
    right_data.delta_time = (micros() - this->right_data.start_time);
  }

  switch (this->current_effect) {
    case EFFECT_BLANK:
      this->current_effect = EFFECT_NONE;
      FastLED.clear();
      FastLED.show();
      break;
    case EFFECT_STROBE:
      this->left_data.fadein_time = 0;
      this->left_data.on_time = 40000;
      this->left_data.fadeout_time = 0;
      this->left_data.color_r = 255; this->left_data.color_g = 255; this->left_data.color_b = 255;
      this->right_data.fadein_time = 0;
      this->right_data.on_time = 40000;
      this->right_data.fadeout_time = 0;
      this->right_data.color_r = 255; this->right_data.color_g = 255; this->right_data.color_b = 255;
      this->effectFlash();
      break;
    case EFFECT_FLASH:
      this->effectFlash();
      break;
    case EFFECT_BREATH:
      // this->effectBreath();
      break;
    case EFFECT_PROGRESS:
      this->effectProgress();
      break;
    case EFFECT_PROGRESS4:
      this->effectProgress(4);
      break;
    case EFFECT_PROGRESS_RAINBOW:
      this->effectProgressRainbowAll();
      break;
    case EFFECT_FIXED:
      this->effectFixed();
      break;
    default:
      break;
  }
}


void MlsLightEffects::progressRainbow(struct CRGB *strip) {
  int8_t light_wave = 255 - ((256 * left_data.delta_time / (1000 * this->light_packet.option)) % 256);
  for (uint16_t i=0; i < this->leds_per_strip; i++) {
    strip[i] = CHSV( (light_wave + map(i % this->leds_per_strip, 0, this->leds_per_strip, 0, 255) % 256), 255, 255);
  }
}


void MlsLightEffects::effectProgressRainbowAll() {
  this->progressRainbow(this->left_strip);
  this->progressRainbow(this->right_strip);
  FastLED.show();
}


void MlsLightEffects::rainbow(struct CRGB *strip) {
  for (uint16_t i=0; i < this->leds_per_strip; i++) {
    strip[i] = CHSV( map(i % this->leds_per_strip, 0, this->leds_per_strip, 0, 255), 255, 255);
  }
}


void MlsLightEffects::effectRainbowAll() {
  this->rainbow(this->left_strip);
  this->rainbow(this->right_strip);
  FastLED.show();
}


void MlsLightEffects::effectRainbowLeft() {
  this->rainbow(this->left_strip);
  FastLED.show();
}


void MlsLightEffects::effectRainbowRight() {
  this->rainbow(this->right_strip);
  FastLED.show();
}


void MlsLightEffects::fill(struct CRGB color, struct CRGB *strip) {
  for (uint16_t i=0; i < this->leds_per_strip; i++) {
    strip[i] = color;
  }
}


void MlsLightEffects::fillAll(struct CRGB color) {
  this->fill(color, this->left_strip);
  this->fill(color, this->right_strip);
}


void MlsLightEffects::fillLeft(struct CRGB color) {
  this->fill(color, this->left_strip);
}


void MlsLightEffects::fillRight(struct CRGB color) {
  this->fill(color, this->right_strip);
}


void MlsLightEffects::setValue(uint16_t value, uint16_t maxValue, struct CRGB colorOffMin, struct CRGB colorOffMax, struct CRGB colorOnMin, struct CRGB colorOnMax, struct CRGB *strip) {
  for (uint16_t i=0; i < value; i++) {
    if (i < maxValue/2) {
      strip[i] = colorOnMin;
    } else {
      strip[i] = colorOnMax;
    }
  } 
  for (uint16_t i=value; i < maxValue; i++) {
    if (i < maxValue/2) {
      strip[i] = colorOffMin;
    } else {
      strip[i] = colorOffMax;
    }
  } 
  for (uint16_t i=maxValue; i < this->leds_per_strip; i++) {
    strip[i] = CRGB::Black;
  }
}


void MlsLightEffects::setValueThree(uint16_t value, uint16_t maxValue, struct CRGB colorOff, struct CRGB colorOn1, struct CRGB colorOn2, struct CRGB colorOn3, struct CRGB *strip) {
  for (uint16_t i=0; i < value; i++) {
    switch (i % 3) {
      case 0:
        strip[i] = colorOn1;
        break;
      case 1:
        strip[i] = colorOn2;
        break;
      case 2:
        strip[i] = colorOn3;
        break;
    }
  } 
  for (uint16_t i=value; i < maxValue; i++) {
    strip[i] = colorOff;
  } 
  for (uint16_t i=maxValue; i < this->leds_per_strip; i++) {
    strip[i] = CRGB::Black;
  }
}


void MlsLightEffects::setValueThreeAll(uint16_t value, uint16_t maxValue, struct CRGB colorOff, struct CRGB colorOn1, struct CRGB colorOn2, struct CRGB colorOn3) {
  this->setValueThree(value, maxValue, colorOff, colorOn1, colorOn2, colorOn3, this->left_strip);
  this->setValueThree(value, maxValue, colorOff, colorOn1, colorOn2, colorOn3, this->right_strip);
}


void MlsLightEffects::setValueThreeLeft(uint16_t value, uint16_t maxValue, struct CRGB colorOff, struct CRGB colorOn1, struct CRGB colorOn2, struct CRGB colorOn3) {
  this->setValueThree(value, maxValue, colorOff, colorOn1, colorOn2, colorOn3, this->left_strip);
}


void MlsLightEffects::setValueThreeRight(uint16_t value, uint16_t maxValue, struct CRGB colorOff, struct CRGB colorOn1, struct CRGB colorOn2, struct CRGB colorOn3) {
  this->setValueThree(value, maxValue, colorOff, colorOn1, colorOn2, colorOn3, this->right_strip);
}


void MlsLightEffects::setValueAll(uint16_t value, uint16_t maxValue, struct CRGB colorOffMin, struct CRGB colorOffMax, struct CRGB colorOnMin, struct CRGB colorOnMax) {
  this->setValue(value, maxValue, colorOffMin, colorOffMax, colorOnMin, colorOnMax, this->left_strip);
  this->setValue(value, maxValue, colorOffMin, colorOffMax, colorOnMin, colorOnMax, this->right_strip);
}


void MlsLightEffects::setValueLeft(uint16_t value, uint16_t maxValue, struct CRGB colorOffMin, struct CRGB colorOffMax, struct CRGB colorOnMin, struct CRGB colorOnMax) {
  this->setValue(value, maxValue, colorOffMin, colorOffMax, colorOnMin, colorOnMax, this->left_strip);
}


void MlsLightEffects::setValueRight(uint16_t value, uint16_t maxValue, struct CRGB colorOffMin, struct CRGB colorOffMax, struct CRGB colorOnMin, struct CRGB colorOnMax) {
  this->setValue(value, maxValue, colorOffMin, colorOffMax, colorOnMin, colorOnMax, this->right_strip);
}


void MlsLightEffects::stopUpdate() {
  this->setLightData(0, (LIGHT_PACKET){EFFECT_BLANK, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
}
