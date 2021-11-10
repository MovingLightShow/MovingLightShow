# MovingLightShow
Synchronized LED strips for musicians  
https://MovingLightShow.art - contact@movinglightshow.art
(c) 2020-2021 Showband Les Armourins, Neuchatel, Switzerland

MovingLightShow can be used by up to 200 musicians simultaneously. Some light
effects are synchronized with the bass drum, and some others are just visual effects
with specific timing. Effects are programmed to works in 4 and 6 columns layout.
6 columns layout position is calculated automagically from the 4 columns layout,
based on the Armourins' practice.
 
The MovingLightShow concept is based on the following elements:
- ESP32 boards with ESP-Now support
- ESP32 boards with LoRa support and OLED display (for remote control)
- Rotary Digital Encoder Push Button (like EC11) for the setup
- Intelligent LED strips (like WS2812B)
- I2S MEMS microphone (like INMP441)
- piezo vibration sensor module (like 52Pi)
- custom PCB for easier mass production
- Bluetooth LE connection to select the effects on the master or from the remote
- Remote controler through LoRa
- PWA with web Bluetooth API for effects selection (compatible with Chrome on Android or Bluefy on iOS)
- 5V powerbank for each musician, with at least 25 Wh of energy (10'000 mAh)
- MLSmesh proprietary interconnection protocol using ESP-Now,
  with dynamic repeaters selection based on RSSI analysis (12 repeaters)
- automatic OTA (over-the-air) firmware upgrade during boot,
  if a known access point is available

The Drum major is always the sender ID 1 and must be set-up on rank 0.
The Bass Drum instrument is always the master, with the sender ID 0.
The device with ID 0 is for the instrument only, with dedicated LED strips.
The bass drum player must have a separate device like any other players.
  
This package is the result of a *LOT* of work. If you are happy using this
package, contact us for a donation to support this project.

---

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
