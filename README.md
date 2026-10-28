# esp32-nmea2k-ble

A wireless NMEA 2000 bridge on an ESP32-S3: it puts the boat's N2K bus on Bluetooth Low Energy for displays around the boat, and lets accepted displays send data back onto the bus. You run it from a web page or the Serial Monitor.

Built on [nmea2k-ble-libraries](https://github.com/fredrikrudin/nmea2k-ble-libraries) (N2kBle, N2kBleTx, N2kBleWeb).

```
 NMEA 2000 bus ──CAN──▶  n2k-ble-bridge  ──BLE──▶ displays (up to 4), each gets only its PGNs
 depth, speed, engine,   own N2K node    ◀─TX──── e.g. cabin temperature, switch commands
 tanks, GPS ...          http://n2k-ble.local     status, accept displays, settings, SD log
```

## What it does

- Streams every NMEA 2000 message to up to 4 BLE clients, fast-packets reassembled, each client with its own PGN filter.
- Sends messages from accepted clients as its own node on the bus (it claims an address and shows up in the plotter's device list). Only PGNs on the allow-list, with a rate limit per client.
- Pairing with a passkey and a pairing window after boot. New displays wait for **Accept** on the web page.
- Web page: status, connected and bound clients, pairing, all settings, remove settings, restart, SD card files.
- Serial Monitor console with the same functions (`help`).
- Settings in flash. Event log in RAM, and on the SD card when one is in.

## Hardware

Waveshare **ESP32-S3-Touch-LCD-4 V4**: its on-board CAN transceiver (TWAI TX GPIO6, RX GPIO0) and SD slot (SDMMC 1-bit: CLK 2, CMD 1, D0 4) are used. The screen is not used by this firmware. Other ESP32/ESP32-S3 boards with a CAN transceiver work: change the pins in [`n2k-ble-bridge/config.h`](n2k-ble-bridge/config.h).

Connect CAN H/L to the N2K backbone through a drop cable. Bus power is 9–16 V: check what your board's supply input accepts before powering it from the bus.

## Build and flash

This repository is an Arduino sketchbook: the sketch is in `n2k-ble-bridge/` and the libraries it needs from nmea2k-ble-libraries are in `libraries/`.

1. Arduino IDE → *File → Preferences → Sketchbook location*: this folder. (Or copy the three folders from `libraries/` to your own `Documents/Arduino/libraries/`.)
2. Library Manager: install **NimBLE-Arduino** 2.x, **NMEA2000** and **NMEA2000_esp32_twai**.
3. Board: *ESP32S3 Dev Module*, with:
   - *Flash Size*: 16MB
   - *Partition Scheme*: **16M Flash (3MB APP/9.9MB FATFS)**. Wi-Fi + BLE together is about 1.35 MB, more than the default 1.2 MB app partition.
   - *PSRAM*: OPI PSRAM
4. Open `n2k-ble-bridge/n2k-ble-bridge.ino`, upload, open the Serial Monitor at 115200 (line ending: newline).

Tested to compile with arduino-esp32 3.3.12, NimBLE-Arduino 2.5.2, NMEA2000 4.25.1. Not yet tested on a boat.

## First start

1. With no Wi-Fi set, the bridge opens the access point **`n2k-ble-setup`**, password `n2kbridge` (change `DEF_AP_PASSWORD` in config.h before flashing, or under Settings).
2. Join it and open **http://192.168.4.1**.
3. Under **Settings**: your Wi-Fi, a **web login password** (user `admin`), a 6-digit **passkey**. Save, then Restart.
4. The bridge joins your network: **http://n2k-ble.local** (or the IP address in the Serial Monitor).

Or all of it in the Serial Monitor:

```
wifi MyBoat secret
webpass something
passkey 482915
save
restart
```

## Adding a display

1. On the web page: **Pairing → Open 2 min**. Pairing is also open for 2 minutes after every boot.
2. Start the display with the same passkey (`N2kBleClientConfig::passkey`).
3. It appears under **Connected clients** as *not bound*: check the address and press **Accept**.

From then on it reconnects by itself, also after the pairing window has closed. **Remove** under *Bound clients* takes it away. **Settings → Forget all clients** starts over.

Who must be accepted (Settings → *Who must be bound*):

| | |
|---|---|
| Nobody | any client gets data and can send what the allow-list permits (if paired) |
| Clients that send (default) | anyone in range may read; only accepted clients may send |
| Every client | only accepted clients get any data. Displays must pair (`secure = true`, or use TX) |

## Sending from a display

The PGNs clients may send are under **Settings → Sending (TX)**. The defaults are temperature (130312, 130316), humidity (130313), pressure (130314) and switch bank control (127502). Bus-management PGNs (address claim, transport protocol, heartbeat, product information) can never be sent for a client. On the display, `N2kBleClientTx::send(msg)` reports every message as sent or refused, with the reason. See the N2kBleTx examples.

## Serial Monitor

`help` lists everything. The most used commands:

| | |
|---|---|
| `status` | bus, BLE, clients, TX, pairing, log, memory |
| `clients` / `accept #n` / `kick #n` | connected clients, accept one, disconnect one |
| `pair [s]` / `pair close` | open or close pairing |
| `bind` / `bind add|del <mac>` / `bind mode off|tx|all` | bound clients |
| `tx` / `tx allow|deny <pgn>` / `tx rate <n>` | what clients may send |
| `log [n]` / `log level e|w|i|d` / `sd` / `sd eject` | log and SD card |
| `name` / `mdns` / `wifi` / `webpass` / `passkey` | settings |
| `save` / `defaults` / `restart` | |

The status is also printed every 10 minutes.

## Log

Connections, pairing, accepted and refused clients, refused TX messages, Wi-Fi and SD events. The last 80 lines are kept in RAM (web page, `log`), and written to `/n2kble.log` on the SD card when one is present (the file rolls over to `.old` at 1 MB). A card inserted later is picked up within 30 s. Use **Eject** (web page) or `sd eject` before taking it out.

## Security notes

- Set a passkey. Without one, anyone in range can pair while pairing is open.
- Set a web password. The page is plain HTTP (small and fast), so the password crosses the network unencrypted: keep the bridge on your boat's own network.
- An address alone can be copied. That's why sending requires a bonded, encrypted link, and with *Every client* the bridge sends no data until the link is encrypted with a stored bond.

## Configuration file

[`config.h`](n2k-ble-bridge/config.h): pins, NMEA 2000 device information, and the defaults used on first start (BLE name, mDNS name, AP password, passkey, pairing window, binding, TX PGNs). Once settings are saved from the web page or console, the saved values win. **Remove settings** returns to config.h.

## Libraries in this repository

`libraries/` is a copy of [nmea2k-ble-libraries](https://github.com/fredrikrudin/nmea2k-ble-libraries) 0.2.0, so the project builds on its own. Change the libraries there and copy them here, not the other way round.

## License

Copyright (c) 2026 Fredrik Rudin. Licensed under [Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0)](https://creativecommons.org/licenses/by-nc/4.0/), see [LICENSE.md](LICENSE.md).

NMEA 2000® is a registered trademark of the National Marine Electronics Association. This project is not affiliated with or certified by NMEA.
