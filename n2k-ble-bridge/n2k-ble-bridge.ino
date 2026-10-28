// esp32-nmea2k-ble - n2k-ble-bridge
// Copyright (c) 2026 Fredrik Rudin. Licensed CC BY-NC 4.0, see LICENSE.md
//
// An ESP32 on the NMEA 2000 bus that
//   - streams every message over BLE to up to 4 displays (each with its own PGN filter)
//   - sends PGNs for accepted displays as its own N2K node (allow-list, rate limit)
//   - is run from a web page (http://n2k-ble.local) or the Serial Monitor
//   - keeps its settings in flash and a log in RAM and on the SD card
//
// Libraries (in ../libraries when this repo is the Arduino sketchbook):
//   N2kBle, N2kBleTx, N2kBleWeb (nmea2k-ble-libraries), NimBLE-Arduino 2.x,
//   NMEA2000, NMEA2000_esp32_twai
//
// First start: join the Wi-Fi "n2k-ble-setup" (password in config.h), open
// http://192.168.4.1, set Wi-Fi, web password and passkey under Settings,
// Save, Restart. Then: Pairing "Open 2 min", start the display, Accept it.

#include "config.h"
#define ESP32_CAN_TX_PIN CAN_TX_PIN
#define ESP32_CAN_RX_PIN CAN_RX_PIN
#include <NMEA2000_esp32_twai.h>
#include <N2kBleTx.h>
#include <N2kBleWeb.h>

NMEA2000_esp32_twai n2k(ESP32_CAN_TX_PIN, ESP32_CAN_RX_PIN);
N2kBleBridge bridge;
N2kBleBridgeTx tx(n2k);
N2kBleBridgeSettings settings;
N2kBleBridgeConsole console(Serial, bridge, settings);
N2kBleBridgeWeb web(bridge, settings);

static void loadSettings() {
  strncpy(settings.deviceName, DEF_BLE_NAME, sizeof(settings.deviceName) - 1);
  strncpy(settings.mdnsName, DEF_MDNS_NAME, sizeof(settings.mdnsName) - 1);
  strncpy(settings.apPass, DEF_AP_PASSWORD, sizeof(settings.apPass) - 1);
  settings.passkey = DEF_PASSKEY;
  settings.pairingWindowMs = DEF_PAIRING_S * 1000UL;
  settings.bind = DEF_BIND;
  settings.autoBind = DEF_AUTO_BIND;
  settings.txRate = DEF_TX_RATE;
  static const uint32_t pgns[] = DEF_TX_PGNS;
  for (uint32_t p : pgns) settings.addTxPgn(p);
  if (!settings.load()) N2KBLE_LOGI("main", "first start: defaults from config.h");
}

static void startN2k() {
  n2k.SetProductInformation("00000001", N2K_PRODUCT_CODE, N2K_MODEL_ID, FW_VERSION " / lib " N2KBLE_VERSION, "1");
  n2k.SetDeviceInformation((uint32_t)(ESP.getEfuseMac() & 0x1FFFFF),  // unique number from the MAC
                           N2K_DEVICE_FUNCTION, N2K_DEVICE_CLASS, N2K_MANUFACTURER_CODE);
  n2k.SetMode(tNMEA2000::N2km_ListenAndNode, N2K_PREFERRED_ADDRESS);
  n2k.SetN2kCANMsgBufSize(32);
  n2k.SetN2kCANReceiveFrameBufSize(150);
  n2k.SetN2kCANSendFrameBufSize(64);
  bridge.attachTo(n2k);
  n2k.Open();
}

static void pairButton() {
#if PAIR_BUTTON_PIN >= 0
  static uint32_t downSince = 0;
  static bool done = false;
  if (digitalRead(PAIR_BUTTON_PIN) == LOW) {
    if (!downSince) downSince = millis();
    if (!done && millis() - downSince > 1000) {
      bridge.openPairing(DEF_PAIRING_S * 1000UL);
      done = true;
    }
  } else {
    downSince = 0;
    done = false;
  }
#endif
}

void setup() {
  Serial.begin(115200);
  N2kBleLog::get().begin(80);
#if PAIR_BUTTON_PIN >= 0
  pinMode(PAIR_BUTTON_PIN, INPUT_PULLUP);
#endif
  N2KBLE_LOGI("main", FW_NAME " " FW_VERSION " (libraries " N2KBLE_VERSION ")");

  loadSettings();
  if (settings.logToSd) N2kBleLog::get().attachSdMmc("/n2kble.log", SD_CLK_PIN, SD_CMD_PIN, SD_D0_PIN);

  tx.begin();
  bridge.addModule(tx);
  settings.applyTo(bridge);  // bound clients + TX allow-list (before begin)
  startN2k();

  N2kBleBridgeConfig cfg;
  settings.applyTo(cfg);
  bridge.begin(cfg);

  web.begin();
  console.statusEveryMs = 10 * 60 * 1000;
  console.begin();
  Serial.printf("Web page: %s\n", web.address().c_str());
}

void loop() {
  n2k.ParseMessages();
  bridge.loop();   // BLE, TX, log
  web.loop();
  console.loop();  // also saves when a client was bound by itself
  pairButton();
}
