# MovingLightShow
Synchronized LED strips for musicians  
https://MovingLightShow.art  

MovingLightShow can be used by up to 100 musicians simultaneously. Some light
effects are synchronized with the bass drum, and some others are just visual effects
with specific timing. Effects are programmed to works in 4 and 6 columns layout.
6 columns layout position is calculated automagically from the 4 columns layout,
based on the Armourins' practice.

The MovingLightShow concept is based on the following elements:
* ESP32 boards with ESP-Now support
* ESP32 boards with LoRa support and OLED display (remote control from outside of the band)
* Rotary Digital Encoder Push Button for the setup
* WS2812 intelligent LEds strips
* I2S MEMS microphone
* MIDI launchpad with 64 PDAD to select the effects (experimental)
* 5V powerbank for each musician, with at least 25 Wh of energy
* MLSmesh proprietary interconnection protocol using ESP-Now,
  with dynamic repeaters selection based on RSSI analysis
* automatic OTA (over-the-air) firmware upgrade during boot,
  if a known access point is available

This package is the result of a *LOT* of work. If you are happy using this
package, contact us for a donation to support this project.