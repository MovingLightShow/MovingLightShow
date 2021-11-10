/**********************************************************************

   MovingLightShow package - Synchronized LED strips for musicians
   https://MovingLightShow.art

   @file  mls_config.h
   @brief Config file and global variables

 **********************************************************************/
#ifndef MLS_CONFIG_H
#define MLS_CONFIG_H
  
  #define DEBUG_MLS // Should NOT be defined for production use

  // https://github.com/FastLED/FastLED
  #define FASTLED_ALLOW_INTERRUPTS 0
  
  #define INITIAL_IID           "MLS"
  #define OTA_URL               "http://movinglightshow.art/"

  #define DEFAULT_WIFI_SSID     "IOT_NETWORK"
  #define DEFAULT_WIFI_SECRET   "gzfh-dkse-6943-dfrt"

  #define LED_TYPE              WS2812B
  #define LED_COLOR_ORDER       GRB
  #define NUM_LEDS_PER_STRIP    18 // (18 LEDs, 30cm)
  #define LED_TEST_BRIGHTNESS   63
  #define LED_CONFIG_BRIGHTNESS 128
  #define LED_MAX_BRIGHTNESS    255
  #define LED_MIN_BRIGHTNESS    15

  #define BLE_PREFIX_NAME       "MovingLightShow" // BLE prefix name

  // See the following for generating UUIDs others than UART service
  // https://www.uuidgenerator.net/
  #define SERVICE_UUID           "fe150000-c76e-46b7-a964-3358a4efcf62" // MovingLightShow service UUID
  #define CHARACTERISTIC_UUID_RX "fe150001-c76e-46b7-a964-3358a4efcf62"
  #define CHARACTERISTIC_UUID_TX "fe150002-c76e-46b7-a964-3358a4efcf62"

  // #define ROTARY_CHANGE_BRIGHTNESS // Each individual rotary button change it's own brightness

  #define ESPNOW_CHANNEL        0 // Don't set anything else than 0 or 1

  // #define FORCE_SLAVE           // This device is a slave
  // #define FORCE_MASTER          // This device is a master
  // #define SKIP_CONFIG           // Config must be skipped on this device
  // #define MASTER_WITHOUT_EFFECT // The master must NOT play regular effect /(only special effects defined using modifier)
  // #define BEST_SYNC_DELAY       // Effects are fully synchronized by delaying by a multiple of 3ms per time slot up to the last repeater
  
  #define SSID_REPEAT_TRIAL_TIME 100000 // 0.1 seconds = 100'000 microseconds
  #define SSID_TRIAL_MAX_TIME    6000000 // 6 seconds

  #define CONFIG_MAX_TIME        30000000 // 30 seconds

  /**********************************************************************

                    !!! IMPORTANT PINOUT INFORMATION !!!
  
    PIN 12 must NEVER be at Vcc during boot sequence
    Strapping pin: GPIO0, GPIO2, GPIO5, GPIO12 (MTDI), and GPIO15 (MTDO) are strapping pins.
      For more infomation, please refer to ESP32 datasheet.
    SPI0/1: GPIO6-11 and GPIO16-17 are usually connected to the SPI flash and PSRAM integrated
      on the module and therefore should not be used for other purposes.
    JTAG: GPIO12-15 are usually used for inline debug.
    GPI: GPIO34-39 can only be set as input mode and do not have software-enabled pullup or pulldown functions.
    TXD & RXD are usually used for flashing and debugging.
      ADC2: ADC2 pins cannot be used when Wi-Fi is used. So, if you’re using Wi-Fi and you’re having trouble
      getting the value from an ADC2 GPIO, you may consider using an ADC1 GPIO instead, that should solve
      your problem. For more details, please refer to ADC limitations.

   Based on: https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
             https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html
      
   **********************************************************************/
    
  #ifdef ARDUINO_ESP32_DEV
    // #define BLE_SERVER                   // BLE server support
    #define ROTARY_ENCODER_BUTTON_PIN 13 // 13 Digital input, rotary encoder button make a contact with GND
    #define ROTARY_ENCODER_A_PIN      22 // 22 Digital input, rotary encoder center PIN is connected on Vcc
    #define ROTARY_ENCODER_B_PIN      23 // 23 Digital input, rotary encoder center PIN is connected on Vcc
    #define MASTER_PIN                21 // 21 Digital input, master make a contact with GND
    #define LEFT_LEDS_PIN             14 // 14 Digital output
    #define RIGHT_LEDS_PIN            27 // 27 Digital output
    #define I2S_WS_PIN                32 // 32 WS/LRCLK/FS IO pin for I2S MEMS microphone
    #define I2S_SCK_PIN               33 // 33 SCK/BCLK/BCK IO pin for I2S MEMS microphone
    #define I2S_SD_PIN                35 // 35 SD/SDATA/SDOUT/DACDAT/ADCDAT serial data input pin for I2S MEMS microphone
    #define PIEZO_PIN                 -1 // -1 Analog input, piezo signal (WARNING! to protect your input, add a big resistance in parallel with GND)
    #define USB_DP_P0_PIN             16 // 16
    #define USB_DM_P0_PIN             17 // 17
  #endif

  #ifdef ARDUINO_TTGO_LoRa32_v21new
    #define BLE_SERVER                   // BLE server support
    #define ROTARY_ENCODER_BUTTON_PIN 12 // Digital input, rotary encoder button make a contact with GND !!! The PIN12 must NEVER be at Vcc during boot !!!
    #define ROTARY_ENCODER_A_PIN      13 // Digital input, rotary encoder center PIN is connected on Vcc
    #define ROTARY_ENCODER_B_PIN      14 // Digital input, rotary encoder center PIN is connected on Vcc
    #define MASTER_PIN                4  // Digital input, master make a contact with GND
    #define LEFT_LEDS_PIN             15 // Digital output
    #define RIGHT_LEDS_PIN            2  // Digital output
    #define I2S_WS_PIN                -1 // -1/19 WS/LRCLK/FS IO pin for I2S MEMS microphone
    #define I2S_SCK_PIN               -1 // -1/23 SCK/BCLK/BCK IO pin for I2S MEMS microphone
    #define I2S_SD_PIN                -1 // -1/34 SD/SDATA/SDOUT/DACDAT/ADCDAT serial data input pin for I2S MEMS microphone
    #define PIEZO_PIN                 34 // 34/-1 Analog input, piezo signall (WARNING! to protect your input, add a big resistance in parallel with GND)
    #define USB_DP_P0_PIN             -1 // 22
    #define USB_DM_P0_PIN             -1 // 21

    #define ONBOARD_LED               LED_BUILTIN
    #define LED_BUILDIN_ON            HIGH
    #define LED_BUILDIN_OFF           LOW

    #define LORA_SCK_PIN              5     // GPIO5  -- SCK
    #define LORA_MISO_PIN             19    // GPIO19 -- MISO
    #define LORA_MOSI_PIN             27    // GPIO27 -- MOSI
    #define LORA_SS_PIN               18    // GPIO18 -- CS
    #define LORA_RST_PIN              23    // GPIO14 -- RESET (If Lora does not work, replace it with GPIO14)
    #define LORA_DI0_PIN              26    // GPIO26 -- IRQ(Interrupt Request)

    #define LORA_BAND                 869525000 // 869525000 European frequency (default is 868E6)
    #define LORA_TX_POWER             20        // 20
    #define LORA_SIGNAL_BANDWIDTH     500E3     // 250E3
    #define LORA_SPREADING_FACTOR     6         // 6 (SF6, not existing in LoRaWAN, is faster than SF7 or more)
    #define LORA_CODING_RATE_4        5         // 5, which means 4/5
    #define LORA_PREAMBLE_LENGTH      6         // 6
  #endif

  #define ROTARY_ENCODER_STEPS    8
  #define ROTARY_ENCODER_VCC_PIN -1

  #define I2S_SAMPLE_RATE          16000 // 16000
  #define I2S_BUFFERS              8     // 8
  #define I2S_BLOCK_SIZE           1024  // 1024
  #define i2S_EDGE_RATIO_LONG_TERM 1.2   // 5 (3 better)
  #define i2S_EDGE_RATIO_INSTANT   1.1   // 4 (2 better)
  #define i2S_EDGE_ROLLING_MAX     3     // 4 = first try (to sensitive)
  #define I2S_ROLLING_MEAN_SIZE    100   // 200

  #define JSON_SIZE                1024
  #define UNIQUEID_CHAR_SIZE       10

  #define URL_CHAR_SIZE            1000
  #define SSID_CHAR_SIZE           64
  #define SECRET_CHAR_SIZE         64
  #define MAC_ADDR_CHAR_SIZE       14

  #define MAX_UPDATE_FIRMWARE_TRIALS 5

#endif
