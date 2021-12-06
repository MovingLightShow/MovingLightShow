/**********************************************************************

   MovingLightShow package - Synchronized LED strips for musicians
   https://MovingLightShow.art

   @file  mls_config.h
   @brief Config file and global variables

 **********************************************************************/
#ifndef MLS_CONFIG_H
#define MLS_CONFIG_H
  
  #include <stdint.h>
 
  // #define MLS_DEMO // Demo mode of various effects after reboot (stop automatically)

  #define DEBUG_MLS // Should NOT be defined for production use :-)

  #define INITIAL_IID           "MLS"
  #define OTA_URL               "http://movinglightshow.art/"

  #define INITIAL_COLUMNS       4
  #define INITIAL_RANKS         18

  // Default Wifi network for initial provisioning
  // This Wifi information will be desactived the first time a provisioned Wifi is connected
  #define DEFAULT_WIFI_SSID     "IOT_NETWORK"
  #define DEFAULT_WIFI_SECRET   "gzfh-dkse-6943-dfrt"

  #define LED_TYPE                  WS2812B
  #define LED_COLOR_ORDER           GRB
  #define NUM_LEDS_PER_STRIP        18 // (18 LEDs, 30cm)
  #define NUM_LEDS_PER_STRIP_MASTER 105 // (105 LEDs, 176 cm)
  #define LED_TEST_BRIGHTNESS       63
  #define LED_CONFIG_BRIGHTNESS     128
  #define LED_MAX_BRIGHTNESS        255
  #define LED_MIN_BRIGHTNESS        15

  #define MASTER_WITHOUT_EFFECT // The master must NOT play regular effect /(only special effects defined using modifier)

  // #define ROTARY_CHANGE_BRIGHTNESS // Each individual rotary button change it's own brightness

  #define MLSMESH_CHANNEL             0 // Don't set anything else than 0 or 1

  // #define FORCE_SLAVE              // This device is a slave
  // #define FORCE_MASTER             // This device is a master
  // #define SKIP_CONFIG              // Config must be skipped on this device
  // #define BEST_SYNC_DELAY          // Effects are fully synchronized by delaying by a multiple of 3ms per time slot up to the last repeater

  #define SSID_REPEAT_TRIAL_TIME      100000   // 0.1 seconds = 100'000 microseconds
  #define SSID_TRIAL_MAX_TIME         6000000  // 6 seconds

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
    // #define BLE_SERVER                   // BLE server support during configuration step
    #define ROTARY_ENCODER_BUTTON_PIN 13 // 13 Digital input, rotary encoder button make a contact with GND
    #define ROTARY_ENCODER_A_PIN      22 // 22 Digital input, rotary encoder center PIN is connected on Vcc
    #define ROTARY_ENCODER_B_PIN      23 // 23 Digital input, rotary encoder center PIN is connected on Vcc
    #define MASTER_PIN                21 // 21 Digital input, master make a contact with GND
    #define LEFT_LEDS_PIN             14 // 14 Digital output
    #define RIGHT_LEDS_PIN            27 // 27 Digital output
    #define PIEZO_PIN                 -1 // -1 Analog input, piezo signal (WARNING! to protect your input, add a big resistance in parallel with GND)
    #define I2S_WS_PIN                32 // 32 WS/LRCLK/FS IO pin for I2S MEMS microphone
    #define I2S_SCK_PIN               33 // 33 SCK/BCLK/BCK IO pin for I2S MEMS microphone
    #define I2S_SD_PIN                35 // 35 SD/SDATA/SDOUT/DACDAT/ADCDAT serial data input pin for I2S MEMS microphone
    #define USB_DP_P0_PIN             -1 // 16
    #define USB_DM_P0_PIN             -1 // 17
    #define FORCE_FIRMWARE_UPDATE_PIN 0  // 0/-1 Force firmware update pin
    #define RANDOM_INIT_PIN           36 // 36/-1 Random PIN analog value for initialization

    #define CONFIG_TIMEOUT_TIME       30000000 // 30 seconds
#endif

  #ifdef ARDUINO_TTGO_LoRa32_v21new
    #define BLE_SERVER                   // BLE server support during configuration step
    #define ROTARY_ENCODER_BUTTON_PIN 12 // Digital input, rotary encoder button make a contact with GND !!! The PIN12 must NEVER be at Vcc during boot !!!
    #define ROTARY_ENCODER_A_PIN      13 // Digital input, rotary encoder center PIN is connected on Vcc
    #define ROTARY_ENCODER_B_PIN      14 // Digital input, rotary encoder center PIN is connected on Vcc
    #define MASTER_PIN                4  // Digital input, master make a contact with GND
    #define LEFT_LEDS_PIN             15 // Digital output
    #define RIGHT_LEDS_PIN            2  // Digital output
    #define PIEZO_PIN                 34 // 34 Analog input, piezo signall (WARNING! to protect your input, add a big resistance in parallel with GND)
    #define I2S_WS_PIN                -1 // -1/19 WS/LRCLK/FS IO pin for I2S MEMS microphone
    #define I2S_SCK_PIN               -1 // -1/23 SCK/BCLK/BCK IO pin for I2S MEMS microphone
    #define I2S_SD_PIN                -1 // -1/34 SD/SDATA/SDOUT/DACDAT/ADCDAT serial data input pin for I2S MEMS microphone
    #define USB_DP_P0_PIN             -1 // 22
    #define USB_DM_P0_PIN             -1 // 21
    #define RANDOM_INIT_PIN           36 // 36/-1 Random PIN analog value for initialization

    #define CONFIG_TIMEOUT_TIME       10000000 // 10 seconds (because LoRa32 devices are mostly used for Remote Control

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
    #define LORA_SIGNAL_BANDWIDTH     62.5E3    // 250E3 (125E3 is slower, but quality is better)
    #define LORA_SPREADING_FACTOR     10        // 6 (SF6, not existing in LoRaWAN, is faster than SF7 or more)
    #define LORA_CODING_RATE_4        8         // 5, which means 4/5 (8 is slower, but with better error correction)
    #define LORA_PREAMBLE_LENGTH      8         // 6
    #define LORA_IMPLICIT_HEADER      false     // is false (explicit) by default, must be true for SF6
    #define LORA_ENABLE_CRC                     // Comment to disable (default) the CRC
    #define LORA_SYNC_WORD            0xAE      // Specific private LoRa sync word
  #endif

  #define CONFIG_TIMEOUT_TIME_DEBUG   30000000 // 30 seconds also during debug time

  #define ROTARY_ENCODER_STEPS        8
  #define ROTARY_ENCODER_VCC_PIN      -1 // We don't define a Vcc pin for the rotary encoder

  #define I2S_SAMPLE_RATE             16000 // 16000
  #define I2S_BUFFERS                 8     // 8
  #define I2S_BLOCK_SIZE              1024  // 1024
  #define I2S_EDGE_RATIO_LONG_TERM    1.2   // 5 (3 better)
  #define I2S_EDGE_RATIO_INSTANT      1.1   // 4 (2 better)
  #define I2S_EDGE_ROLLING_MAX        3     // 4 = first try (to sensitive)
  #define I2S_ROLLING_MEAN_SIZE       100   // 200

  #define JSON_SIZE                   1024
  #define UNIQUEID_CHAR_SIZE          10

  #define URL_CHAR_SIZE               1000
  #define SSID_CHAR_SIZE              64
  #define SECRET_CHAR_SIZE            64
  #define MAC_ADDR_CHAR_SIZE          14

  #define MAX_FORCE_FIRMWARE_TRIALS   5
  #define MIN_FORCE_FIRMWARE_WAIT_MS  5000
  #define MAX_FORCE_FIRMWARE_WAIT_MS  30000

  #define MLSMESH_MAX_MS_FIRST_PACKET 60000 // How long to wait in ms before receiving the first ESPNOW packet (otherwise we will reboot)
  #define MLSMESH_MASTER_TIMEOUT_MS   10000 // How long to wait in ms before a new ESPNOW packet is sent (otherwise we will send a keep alive packet)

  #define BLE_NOTIF_HEARTBEAT_MS      1000 // BLE notification heartbeat in ms

  #define CHECK_RESEND_TIME_MS        500   // Check resend time in ms (to repeat the command during CHECk effect)
  #define CHECK_RANK_TIME_MS          1500

  #define OLED_INFO_REFRESH_TIME      1000

  #define SUBSCRIBE_RETRY_TIME_MS     1000

  // FastLED global options
  // -----------------------
  // https://github.com/FastLED/FastLED
  #define FASTLED_ALLOW_INTERRUPTS 0
  
  // Bluetooth LE definition
  // -----------------------
  // See https://www.uuidgenerator.net/ for generating UUIDs others than UART service
  #define BLE_PREFIX_NAME             "MovingLightShow" // BLE prefix name
  #define SERVICE_UUID                "fe150000-c76e-46b7-a964-3358a4efcf62" // MovingLightShow service UUID
  #define CHARACTERISTIC_UUID_RX      "fe150001-c76e-46b7-a964-3358a4efcf62"
  #define CHARACTERISTIC_UUID_TX      "fe150002-c76e-46b7-a964-3358a4efcf62"

#endif
