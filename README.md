# FlipBox

Flipper Zero app for reading and writing NFC tags used by the [QIDI Box](https://wiki.qidi3d.com/en/QIDIBOX/RFID) filament management system.

## Screenshots

| Menu | Read result | Write config |
|------|-------------|--------------|
| ![Menu](screenshots/01-menu.jpg) | ![Read result](screenshots/02-read-result.jpg) | ![Write config](screenshots/03-write-cfg.jpg) |

| Material picker | Color picker | Write success |
|-----------------|--------------|---------------|
| ![Material picker](screenshots/04-material-picker.jpg) | ![Color picker](screenshots/05-color-picker.jpg) | ![Write success](screenshots/06-write-success.jpg) |

## What it does

- **Read** — hold the Flipper near a QIDI Box spool tag and see material, color, and manufacturer
- **Write** — pick material / color / maker, then write your own tag

Works with any MIFARE Classic 1K spool tag (FM11RF08S or compatible).

## Compatibility

| Firmware | Status |
|---|---|
| Official Flipper firmware | ✅ |
| Momentum | ✅ |
| Unleashed | ✅ |
| RogueMaster | ✅ |

## Install

Download `flipbox.fap` from [Releases](https://github.com/rmrfus/flipbox/releases) and copy it to `SD:/apps/NFC/` on the Flipper's SD card.

Two ways to copy:

- **qFlipper** — connect Flipper via USB, open [qFlipper](https://flipperzero.one/update), go to SD card browser → `apps/NFC/`, drag the file in
- **USB mass storage** — on the Flipper: `Settings → USB → Mass Storage`, then mount as a flash drive and copy manually

The app will appear in `Apps → NFC → FlipBox`.

## Build from source

Requires [ufbt](https://github.com/flipperdevices/flipperzero-ufbt) and [direnv](https://direnv.net/).

```zsh
git clone https://github.com/rmrfus/flipbox
cd flipbox
direnv allow

# just build → dist/flipbox.fap
direnv exec . ufbt

# or build + deploy + launch over USB serial
direnv exec . ufbt launch
```

`ufbt launch` copies the app to `SD:/apps/NFC/` over USB serial and starts it immediately. The Flipper must be connected, unlocked, and on the main screen.

## Tag format

Chip: FM11RF08S (MIFARE Classic 1K), 13.56 MHz, ISO 14443-A.

Data is in **Sector 1, Block 0** (absolute block 4):

| Byte | Field | Notes |
|---|---|---|
| `[0]` | Material code | 1–50, see table below |
| `[1]` | Color code | 1–24 |
| `[2]` | Manufacturer code | 1 = QIDI, 2 = Generic |
| `[3–15]` | — | Unused, written as `0x00` |

Auth keys tried in order:
1. `D3:F7:D3:F7:D3:F7` (QIDI primary)
2. `FF:FF:FF:FF:FF:FF` (factory default)

> **Note on manufacturer byte:** the current QIDI Box firmware ignores `byte[2]` entirely and hardcodes vendor = 1 internally. The field is written for future firmware compatibility.

### Materials (35 types)

| Group | Materials |
|---|---|
| PLA | PLA Rapido, PLA Matte, PLA Metal, PLA Silk, PLA-CF, PLA-Wood, PLA Basic, PLA Matte Basic |
| ABS | ABS, ABS-GF, ABS-Metal, ABS-Odorless |
| ASA | ASA, ASA-AERO |
| PA / Nylon | UltraPA, PA-CF, UltraPA-CF25, PA12-CF, PAHT-CF, PAHT-GF |
| PET / PETG | PET-CF, PET-GF, PETG Basic, PETG Tough, PETG Rapido, PETG-CF, PETG-GF, PETG Translucent |
| Support | Support For PAHT, Support For PET/PA |
| TPU | TPU-Aero, TPU |
| Other | PC/ABS-FR, PPS-CF, PVA |

### Colors (24)

White, Black, Light Gray, Lime Green, Mint Green, Blue, Pink, Yellow, Dark Green,
Light Blue, Dark Blue, Lavender, Yellow-Green, Ocean Blue, Sky Blue, Purple,
Light Red, Red, Beige, Gray, Brown, Tan, Orange, Bronze.

## References

- [QIDI Box RFID spec](https://wiki.qidi3d.com/en/QIDIBOX/RFID)
- [QIDI Box firmware source (community reverse-engineered)](https://github.com/qidi-community/Plus4-Wiki/blob/main/content/qidibox-on-orcaslicer/original_source/box_rfid.py)
- [alexk42/qidi-filament-nfc-flipper](https://github.com/alexk42/qidi-filament-nfc-flipper) — original Flipper implementation this was inspired by
