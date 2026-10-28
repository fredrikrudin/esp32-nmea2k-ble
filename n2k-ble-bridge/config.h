// esp32-nmea2k-ble - n2k-ble-bridge
// Copyright (c) 2026 Fredrik Rudin. Licensed CC BY-NC 4.0, see LICENSE.md
// config.h - board pins and first-start defaults. Everything except the pins
// can be changed later on the web page or in the Serial Monitor, and is then
// kept in flash (saved values win over these).
#pragma once

#define FW_NAME    "n2k-ble-bridge"
#define FW_VERSION "0.2.0"

// ---- Board: Waveshare ESP32-S3-Touch-LCD-4 V4
#define CAN_TX_PIN GPIO_NUM_6
#define CAN_RX_PIN GPIO_NUM_0
#define SD_CLK_PIN 2  // SD card, SDMMC 1-bit
#define SD_CMD_PIN 1
#define SD_D0_PIN  4
#define PAIR_BUTTON_PIN -1  // a button to GND that opens pairing (hold 1 s); -1 = none. Not GPIO0 here: CAN RX.

// ---- NMEA 2000 node
#define N2K_PREFERRED_ADDRESS 42
#define N2K_MODEL_ID          "N2K-BLE bridge"
#define N2K_PRODUCT_CODE      100
#define N2K_MANUFACTURER_CODE 2046  // none registered
#define N2K_DEVICE_FUNCTION   130   // PC gateway
#define N2K_DEVICE_CLASS      25    // inter/intranetwork device

// ---- First-start defaults
#define DEF_BLE_NAME       "N2K-BLE"
#define DEF_MDNS_NAME      "n2k-ble"      // http://n2k-ble.local
#define DEF_AP_PASSWORD    "n2kbridge"    // setup access point, change it
#define DEF_PASSKEY        0              // 6 digits for passkey pairing; 0 = Just Works
#define DEF_PAIRING_S      120            // pairing open this long after boot
#define DEF_BIND           N2kBindTx      // only accepted clients may send
#define DEF_AUTO_BIND      false          // new clients wait for Accept on the web page
#define DEF_TX_RATE        20             // messages per second per client
// PGNs clients may send until changed on the web page
#define DEF_TX_PGNS {130316, 130312, 130313, 130314, 127502}
