# Changelog

## 0.2.0 - 2026-10-28

First version of the bridge firmware, `n2k-ble-bridge`, with nmea2k-ble-libraries 0.2.0 in `libraries/`.

- NMEA 2000 → BLE for up to 4 clients; sending for accepted clients as its own N2K node.
- Pairing with passkey and pairing window; accept clients on the web page or in the console.
- Web page (http://n2k-ble.local): status, clients, pairing, settings, remove settings, restart, SD card files. Setup access point when no Wi-Fi is set.
- Serial Monitor console, settings in flash, log in RAM and on the SD card.
- Board configuration and first-start defaults in `config.h` (Waveshare ESP32-S3-Touch-LCD-4 V4).
