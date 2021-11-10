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
  struct CRGB new_color;

  int8_t light_step = (this->light_packet.option * left_data.delta_time_micros / (1000 * this->light_packet.duration)) % this->light_packet.option;
  if (last_light_step != light_step) {
    last_light_step = light_step;
  }
 
  if (MODIFIER_IGNORE_LEFT != (this->light_packet.effect_modifier & MODIFIER_IGNORE_LEFT)) {
    for (uint16_t i=0; i < this->leds_per_strip; i++) {
      if (light_step == (i % this->light_packet.option)) {
        new_color = CRGB(left_data.color_r, left_data.color_g, left_data.color_b);
      } else {
        new_color = CRGB::Black;
      }
      this->left_strip[i] = CRGB(pgm_read_byte(&gamma8[new_color.r]),  pgm_read_byte(&gamma8[new_color.g]),  pgm_read_byte(&gamma8[new_color.b]));
    }
  }
  if (MODIFIER_IGNORE_RIGHT != (this->light_packet.effect_modifier & MODIFIER_IGNORE_RIGHT)) {
    for (uint16_t i=0; i < this->leds_per_strip; i++) {
      if (light_step == (i  %this->light_packet.option)) {
        new_color = CRGB(right_data.color_r, right_data.color_g, right_data.color_b);
      } else {
        new_color = CRGB::Black;
      }
      this->right_strip[i] = CRGB(pgm_read_byte(&gamma8[new_color.r]),  pgm_read_byte(&gamma8[new_color.g]),  pgm_read_byte(&gamma8[new_color.b]));
    }
  }
  FastLED.show();
  this->last_light_step = light_step;
}


void MlsLightEffects::effectFlash() {
  struct CRGB new_color;

  if (MODIFIER_IGNORE_LEFT != (this->light_packet.effect_modifier & MODIFIER_IGNORE_LEFT)) {
    memcpy(new_color.raw, left_data.color_raw, 3);
    if ((this->left_data.delta_time_micros >= 0) && (this->left_data.fadein_time_micros > 0) && (this->left_data.delta_time_micros < this->left_data.fadein_time_micros)) {
      new_color = this->adjustBrightness(new_color, 255 * this->left_data.delta_time_micros / this->left_data.fadein_time_micros);
    } else if ((this->left_data.delta_time_micros >= 0) && (this->left_data.delta_time_micros < (this->left_data.fadein_time_micros + this->left_data.on_time_micros))) {
    } else if ((this->left_data.delta_time_micros >= 0) && (this->left_data.fadeout_time_micros > 0) && (this->left_data.delta_time_micros < (this->left_data.fadein_time_micros + this->left_data.on_time_micros + this->left_data.fadeout_time_micros))) {
      new_color = this->adjustBrightness(new_color, 255 * (this->left_data.fadeout_time_micros - (this->left_data.delta_time_micros - left_data.fadein_time_micros - this->left_data.on_time_micros)) / this->left_data.fadeout_time_micros);
    } else {
      new_color = CRGB::Black;
    }
    this->fillLeft(new_color);
  }
  if (MODIFIER_IGNORE_RIGHT != (this->light_packet.effect_modifier & MODIFIER_IGNORE_RIGHT)) {
    memcpy(new_color.raw, right_data.color_raw, 3);
    if ((this->left_data.delta_time_micros >= 0) && (this->right_data.fadein_time_micros > 0) && (this->right_data.delta_time_micros < this->right_data.fadein_time_micros)) {
      new_color = this->adjustBrightness(new_color, 255 * this->right_data.delta_time_micros / this->right_data.fadein_time_micros);
    } else if ((this->left_data.delta_time_micros >= 0) && (right_data.delta_time_micros < (this->right_data.fadein_time_micros + this->right_data.on_time_micros))) {
    } else if ((this->left_data.delta_time_micros >= 0) && (this->right_data.fadeout_time_micros > 0) && (this->right_data.delta_time_micros < (this->right_data.fadein_time_micros + this->right_data.on_time_micros + this->right_data.fadeout_time_micros))) {
      new_color = this->adjustBrightness(new_color, 255 * (this->right_data.fadeout_time_micros - (this->right_data.delta_time_micros - right_data.fadein_time_micros - this->right_data.on_time_micros)) / this->right_data.fadeout_time_micros);
    } else {
      new_color = CRGB::Black;
    }
    this->fillRight(new_color);
  }
  FastLED.show();
}


void MlsLightEffects::effectFixed() {
  struct CRGB new_color;

  if (MODIFIER_IGNORE_LEFT != (this->light_packet.effect_modifier & MODIFIER_IGNORE_LEFT)) {
    memcpy(new_color.raw, left_data.color_raw, 3);
    if ((this->left_data.delta_time_micros >= 0) && (this->left_data.fadein_time_micros > 0) && (this->left_data.delta_time_micros < this->left_data.fadein_time_micros)) {
      new_color = this->adjustBrightness(new_color, 255 * this->left_data.delta_time_micros / this->left_data.fadein_time_micros);
    } else if (this->left_data.delta_time_micros < 0) {
      new_color = CRGB::Black;
    }
    this->fillLeft(new_color);
  }
  if (MODIFIER_IGNORE_RIGHT != (this->light_packet.effect_modifier & MODIFIER_IGNORE_RIGHT)) {
    memcpy(new_color.raw, right_data.color_raw, 3);
    if ((this->right_data.delta_time_micros >= 0) && (this->right_data.fadein_time_micros > 0) && (this->right_data.delta_time_micros < this->right_data.fadein_time_micros)) {
      new_color = this->adjustBrightness(new_color, 255 * this->right_data.delta_time_micros / this->right_data.fadein_time_micros);
    } else if (this->right_data.delta_time_micros < 0) {
      new_color = CRGB::Black;
    }
    this->fillRight(new_color);
  }
  FastLED.show();
}


// setLightData with latency informations
void MlsLightEffects::setLightData(uint16_t packetId, struct LIGHT_PACKET lightPacket, uint32_t receivedLatencyTimeMicros) {
  if (this->received_packet != packetId) {
    this->received_packet = packetId;
    this->receivedLatencyTimeMicros = receivedLatencyTimeMicros;
    if (EFFECT_KEEP_ALIVE != lightPacket.effect) {
      memcpy(this->received_light_packet.raw, lightPacket.raw, LIGHT_PACKET_SIZE);
      this->currentPackedReceivedTimeMicros = micros();
      this->lightPacketReceived = true;
    }
  }
}


// setLightData without latency informations
void MlsLightEffects::setLightData(uint16_t packetId, struct LIGHT_PACKET lightPacket) {
  if (this->received_packet != packetId) {
    this->received_packet = packetId;
    this->receivedLatencyTimeMicros = 0;
    if (EFFECT_KEEP_ALIVE != lightPacket.effect) {
      memcpy(this->received_light_packet.raw, lightPacket.raw, LIGHT_PACKET_SIZE);
      this->currentPackedReceivedTimeMicros = micros();
      this->lightPacketReceived = true;
    }
  }
}


void MlsLightEffects::updateLight() {
  bool packetReleased = false;
  bool effect_changed = false;
  bool repeat = false;
  uint16_t option = 0;


  if (this->lightPacketReceived) {
    this->currentlightPacketReceived = true;
    this->lightPacketReceived = false;

    DEBUG_PRINTLN("*** Light packet received ***");
    DEBUG_PRINT("Effect: ");
    DEBUG_PRINTLN(this->received_light_packet.effect);
    DEBUG_PRINT("Effect modifier: ");
    DEBUG_PRINTLN(this->received_light_packet.effect_modifier);
    DEBUG_PRINT("Repeat counter: ");
    DEBUG_PRINTLN(this->received_light_packet.repeat_counter);
    DEBUG_PRINT("Duration (ms): ");
    DEBUG_PRINTLN(this->received_light_packet.duration);
    DEBUG_PRINT("Option: ");
    DEBUG_PRINTLN(this->received_light_packet.option);
    DEBUG_PRINT("Left color: ");
    DEBUG_PRINTHEX(this->received_light_packet.left_color_r);
    DEBUG_PRINT(" ");
    DEBUG_PRINTHEX(this->received_light_packet.left_color_g);
    DEBUG_PRINT(" ");
    DEBUG_PRINTHEX(this->received_light_packet.left_color_b);
    DEBUG_PRINTLN();
    DEBUG_PRINT("Left Fade in (x 10ms): ");
    DEBUG_PRINTLN(this->received_light_packet.left_fadein_time);
    DEBUG_PRINT("Left On time (x 10ms): ");
    DEBUG_PRINTLN(this->received_light_packet.left_on_time);
    DEBUG_PRINT("Left Fade out (x 10ms): ");
    DEBUG_PRINTLN(this->received_light_packet.left_fadeout_time);
    DEBUG_PRINT("Right color: ");
    DEBUG_PRINTHEX(this->received_light_packet.right_color_r);
    DEBUG_PRINT(" ");
    DEBUG_PRINTHEX(this->received_light_packet.right_color_g);
    DEBUG_PRINT(" ");
    DEBUG_PRINTHEX(this->received_light_packet.right_color_b);
    DEBUG_PRINTLN();
    DEBUG_PRINT("Right Fade in (x 10ms): ");
    DEBUG_PRINTLN(this->received_light_packet.right_fadein_time);
    DEBUG_PRINT("Right On time (x 10ms): ");
    DEBUG_PRINTLN(this->received_light_packet.right_on_time);
    DEBUG_PRINT("Right Fade out (x 10ms): ");
    DEBUG_PRINTLN(this->received_light_packet.right_fadeout_time);
  }

  if (this->currentlightPacketReceived && ((micros() - this->currentPackedReceivedTimeMicros) >= this->receivedLatencyTimeMicros)) {
    packetReleased = true;
    DEBUG_PRINTLN("*** Light packet released ***");
    DEBUG_PRINT("Latency to respect: ");
    DEBUG_PRINTLN(this->receivedLatencyTimeMicros);
    this->currentLatencyTimeMicros = this->receivedLatencyTimeMicros;
    this->currentlightPacketReceived = false;
  }
  
  if (packetReleased) {
    effect_changed = (this->current_effect != this->received_light_packet.effect);
    this->current_effect = this->received_light_packet.effect;
    memcpy(this->light_packet.raw, this->received_light_packet.raw, LIGHT_PACKET_SIZE);
  }

  repeat = (MODIFIER_REPEAT == (this->light_packet.effect_modifier & MODIFIER_REPEAT));

  // Check/correct options for some specific effects
  switch (this->current_effect) {
    case EFFECT_STROBE:
      repeat = true;
      if (this->light_packet.duration == 0) {
        this->light_packet.duration = 100;
      }
      break;

    case EFFECT_PROGRESS4:
	    this->light_packet.option = 4;
    case EFFECT_PROGRESS:
      repeat = true;
      if (this->light_packet.duration == 0) {
        this->light_packet.duration = 300;
      }
  	  if (this->light_packet.option == 0) {
  		  this->light_packet.option = 3;
  	  }
      break;

    case EFFECT_PROGRESS_RAINBOW:
      repeat = true;
      if (this->light_packet.duration == 0) {
        this->light_packet.duration = 300;
      }
      break;

    case EFFECT_CHECK:
      repeat = true;
      if (this->light_packet.duration == 0) {
        this->light_packet.duration = 1000;
      }
      break;
  }

  if (this->light_packet.duration <= 0) {
    repeat = false;
  }

  if (this->current_effect == EFFECT_NONE) {
    effect_changed = false;
  }

  if (effect_changed) {
    this->play_counter = this->light_packet.repeat_counter;
    this->current_play_counter = this->light_packet.repeat_counter;
    this->effect_start_time_micros = micros();
  } else if (repeat) {
    this->play_counter = ((micros() - this->effect_start_time_micros) / (1000 * this->light_packet.duration));
  }

  if (effect_changed || packetReleased || (this->play_counter != this->current_play_counter)) {
    if ((MODIFIER_FLIP_FLOP == (this->light_packet.effect_modifier & MODIFIER_FLIP_FLOP)) && (0 != (this->play_counter % 2))) {
      // LEFT strip not disabled
      if (MODIFIER_IGNORE_LEFT != (this->light_packet.effect_modifier & MODIFIER_IGNORE_LEFT)) {
        this->left_data.start_time_micros = micros();
        this->left_data.fadein_time_micros = this->light_packet.right_fadein_time * 10000; // From 10ms to microseconds
        this->left_data.on_time_micros = this->light_packet.right_on_time * 10000; // From 10ms to microseconds
        this->left_data.fadeout_time_micros = this->light_packet.right_fadeout_time * 10000; // From 10ms to microseconds
        memcpy(this->left_data.color_raw, this->light_packet.right_color_raw, 3);
      }
      // RIGHT strip not disabled
      if (MODIFIER_IGNORE_RIGHT != (this->light_packet.effect_modifier & MODIFIER_IGNORE_RIGHT)) {
        this->right_data.start_time_micros = micros();
        this->right_data.fadein_time_micros = this->light_packet.left_fadein_time * 10000; // From 10ms to microseconds
        this->right_data.on_time_micros = this->light_packet.left_on_time * 10000; // From 10ms to microseconds
        this->right_data.fadeout_time_micros = this->light_packet.left_fadeout_time * 10000; // From 10ms to microseconds
        memcpy(this->right_data.color_raw, this->light_packet.left_color_raw, 3);
      }
    } else { // Flip-flop mode
      // LEFT strip not disabled
      if (MODIFIER_IGNORE_LEFT != (this->light_packet.effect_modifier & MODIFIER_IGNORE_LEFT)) {
        this->left_data.start_time_micros = micros();
        this->left_data.fadein_time_micros = this->light_packet.left_fadein_time * 10000; // From 10ms to microseconds
        this->left_data.on_time_micros = this->light_packet.left_on_time * 10000; // From 10ms to microseconds
        this->left_data.fadeout_time_micros = this->light_packet.left_fadeout_time * 10000; // From 10ms to microseconds
        memcpy(this->left_data.color_raw, this->light_packet.left_color_raw, 3);
      }
      // RIGHT strip not disabled
      if (MODIFIER_IGNORE_RIGHT != (this->light_packet.effect_modifier & MODIFIER_IGNORE_RIGHT)) {
        this->right_data.start_time_micros = micros();
        this->right_data.fadein_time_micros = this->light_packet.right_fadein_time * 10000; // From 10ms to microseconds
        this->right_data.on_time_micros = this->light_packet.right_on_time * 10000; // From 10ms to microseconds
        this->right_data.fadeout_time_micros = this->light_packet.right_fadeout_time * 10000; // From 10ms to microseconds
        memcpy(this->right_data.color_raw, this->light_packet.right_color_raw, 3);
      }
    }
  }

  this->current_play_counter = this->play_counter;
  if ((!effect_changed) && packetReleased) {
    this->play_counter++;
  }
  if (repeat) {
  	if (MODIFIER_IGNORE_LATENCY == (this->light_packet.effect_modifier & MODIFIER_IGNORE_LATENCY)) {
      left_data.delta_time_micros = (micros() - this->left_data.start_time_micros) % (1000 * this->light_packet.duration);
  	  right_data.delta_time_micros = (micros() - this->right_data.start_time_micros) % (1000 * this->light_packet.duration);
  	} else {
      left_data.delta_time_micros = (micros() - this->left_data.start_time_micros - currentLatencyTimeMicros) % (1000 * this->light_packet.duration);
  	  right_data.delta_time_micros = (micros() - this->right_data.start_time_micros - currentLatencyTimeMicros) % (1000 * this->light_packet.duration);
  	}
  } else {
  	if (MODIFIER_IGNORE_LATENCY == (this->light_packet.effect_modifier & MODIFIER_IGNORE_LATENCY)) {
      left_data.delta_time_micros = (micros() - this->left_data.start_time_micros);
      right_data.delta_time_micros = (micros() - this->right_data.start_time_micros);
  	} else {
      left_data.delta_time_micros = (micros() - this->left_data.start_time_micros - currentLatencyTimeMicros);
      right_data.delta_time_micros = (micros() - this->right_data.start_time_micros - currentLatencyTimeMicros);
  	}
  }

  switch (this->current_effect) {
    case EFFECT_BLANK:
      this->current_effect = EFFECT_NONE;
      FastLED.clear();
      FastLED.show();
      break;
    case EFFECT_STROBE:
      this->left_data.fadein_time_micros = 0;
      this->left_data.on_time_micros = 40000;
      this->left_data.fadeout_time_micros = 0;
      this->left_data.color_r = 255; this->left_data.color_g = 255; this->left_data.color_b = 255;
      this->right_data.fadein_time_micros = 0;
      this->right_data.on_time_micros = 40000;
      this->right_data.fadeout_time_micros = 0;
      this->right_data.color_r = 255; this->right_data.color_g = 255; this->right_data.color_b = 255;
      this->effectFlash();
      break;
    case EFFECT_FLASH:
      this->effectFlash();
      break;
    case EFFECT_BREATH:
      // this->effectBreath();
      break;
    case EFFECT_PROGRESS4:
    case EFFECT_PROGRESS:
      this->effectProgress();
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
  int8_t light_wave = 255 - ((256 * left_data.delta_time_micros / (1000 * this->light_packet.duration)) % 256);
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
    strip[i] = CRGB(pgm_read_byte(&gamma8[color.r]),  pgm_read_byte(&gamma8[color.g]),  pgm_read_byte(&gamma8[color.b]));
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
      strip[i] = CRGB(pgm_read_byte(&gamma8[colorOnMin.r]),  pgm_read_byte(&gamma8[colorOnMin.g]),  pgm_read_byte(&gamma8[colorOnMin.b]));
    } else {
      strip[i] = CRGB(pgm_read_byte(&gamma8[colorOnMax.r]),  pgm_read_byte(&gamma8[colorOnMax.g]),  pgm_read_byte(&gamma8[colorOnMax.b]));
    }
  } 
  for (uint16_t i=value; i < maxValue; i++) {
    if (i < maxValue/2) {
      strip[i] = CRGB(pgm_read_byte(&gamma8[colorOffMin.r]),  pgm_read_byte(&gamma8[colorOffMin.g]),  pgm_read_byte(&gamma8[colorOffMin.b]));
    } else {
      strip[i] = CRGB(pgm_read_byte(&gamma8[colorOffMax.r]),  pgm_read_byte(&gamma8[colorOffMax.g]),  pgm_read_byte(&gamma8[colorOffMax.b]));
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
  for (uint16_t i=value; i < maxValue; i++) {
    strip[i] = CRGB(pgm_read_byte(&gamma8[colorOff.r]),  pgm_read_byte(&gamma8[colorOff.g]),  pgm_read_byte(&gamma8[colorOff.b]));
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
  this->setLightData(0, (LIGHT_PACKET){EFFECT_BLANK, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0});
}
