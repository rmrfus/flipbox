# FlipBox

Flipper Zero app for reading and writing NFC tags used by the [QIDI Box](https://wiki.qidi3d.com/en/QIDIBOX/RFID) filament management system.

## Screenshots

| Menu                             | Read result                                    | Write config                                  |
|----------------------------------|------------------------------------------------|-----------------------------------------------|
| ![Menu](screenshots/01-menu.png) | ![Read result](screenshots/02-read-result.png) | ![Write config](screenshots/03-write-cfg.png) |

| Material picker                                        | Color picker                                     | Write success                                      |
|--------------------------------------------------------|--------------------------------------------------|----------------------------------------------------|
| ![Material picker](screenshots/04-material-picker.png) | ![Color picker](screenshots/05-color-picker.png) | ![Write success](screenshots/06-write-success.png) |

## What it does

- **Read** — hold the Flipper near a QIDI Box spool tag and see material, color, and manufacturer
- **Write** — pick printer / material / color / maker, then write your own tag

The tag stores only numeric codes, and each printer model maps them to names with its own table,
so pick your printer first (`Write Tag → Printer`, left/right). The choice is remembered and also
used to name materials when reading.

Works with any MIFARE Classic 1K spool tag (FM11RF08S or compatible).

## Compatibility

A Flipper app only loads on firmware with the same API major version, so each release ships one
file per API:

| Firmware                       | API | File                |
|--------------------------------|-----|---------------------|
| Official 1.4.x                 | 87  | `flipbox-api87.fap` |
| Momentum mntm-012              | 87  | `flipbox-api87.fap` |
| Official 1.5.x                 | 88  | `flipbox-api88.fap` |
| Unleashed unlshd-090 and newer | 88  | `flipbox-api88.fap` |
| RogueMaster RM0713 and newer   | 88  | `flipbox-api88.fap` |

If the Flipper says "Update Application" or "Update Firmware", you have the file for the other API.

| Printer | Code table                                                                                  |
|---------|---------------------------------------------------------------------------------------------|
| Q2      | `officiall_filas_list.cfg` from [QIDI_Q2](https://github.com/QIDITECH/QIDI_Q2) V1.1.1       |
| Max4    | `officiall_filas_list.cfg` from [QIDI_MAX4](https://github.com/QIDITECH/QIDI_MAX4)          |
| Plus4   | `officiall_filas_list.cfg` from [QIDI_PLUS4](https://github.com/QIDITECH/QIDI_PLUS4) V1.7.1 |

**Plus5 and Q2C are not supported yet:** both work with the QIDI Box, but QIDI has not published
their filament table (`officiall_filas_list.cfg`), so the code-to-name mapping for them is unknown.
The Q2 or Max4 table may match, but this is unverified.

## Install

Download the `.fap` for your firmware (see [Compatibility](#compatibility)) from [Releases](https://github.com/rmrfus/flipbox/releases) and copy it to `SD:/apps/NFC/` on the Flipper's SD card.

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

The SDK defaults to the official release channel. To build for the next API (e.g. Unleashed), switch the channel first:

```zsh
direnv exec . ufbt update --channel=rc
```

## Tag format

Chip: FM11RF08S (MIFARE Classic 1K), 13.56 MHz, ISO 14443-A.

Data is in **Sector 1, Block 0** (absolute block 4):

| Byte     | Field             | Notes                     |
|----------|-------------------|---------------------------|
| `[0]`    | Material code     | 1–50, see table below     |
| `[1]`    | Color code        | 1–24                      |
| `[2]`    | Manufacturer code | 1 = QIDI, 0 = Generic     |
| `[3–15]` | —                 | Unused, written as `0x00` |

Auth keys tried in order:
1. `D3:F7:D3:F7:D3:F7` (QIDI primary)
2. `FF:FF:FF:FF:FF:FF` (factory default)

> **Note on manufacturer byte:** the stock QIDI Box firmware ignores `byte[2]` and hardcodes vendor = 1 internally. Community firmware replacements read it, using the printer's `[vendor_list]` (0 = Generic, 1 = QIDI). FlipBox 0.1.x wrote Generic as `2`; such tags read as `Mfr: #2`.

### Materials

Names as each printer shows them; `—` means the printer's table has no entry for that code.
Codes not listed are unused on all three.

| Code | Q2                 | Max4               | Plus4              |
|------|--------------------|--------------------|--------------------|
| 1    | PLA Rapido         | PLA Rapido         | PLA                |
| 2    | PLA Matte          | PLA Matte          | PLA Matte          |
| 3    | PLA Metal          | PLA Metal          | PLA Metal          |
| 4    | PLA Silk           | PLA Silk           | PLA Silk           |
| 5    | PLA-CF             | PLA-CF             | PLA-CF             |
| 6    | PLA-Wood           | PLA-Wood           | PLA-Wood           |
| 7    | PLA Basic          | PLA Basic          | PLA Basic          |
| 8    | PLA Matte Basic    | PLA Matte Basic    | PLA Matte Basic    |
| 10   | Support For PLA    | Support For PLA    | —                  |
| 11   | ABS Rapido         | ABS Rapido         | ABS                |
| 12   | ABS-GF             | ABS-GF             | ABS-GF             |
| 13   | ABS-Metal          | ABS-Metal          | ABS-Metal          |
| 14   | ABS-Odorless       | ABS-Odorless       | ABS-Odorless       |
| 15   | —                  | TPU-GF             | —                  |
| 18   | ASA                | ASA                | ASA                |
| 19   | ASA-Aero           | ASA-Aero           | ASA-Aero           |
| 20   | ASA-CF             | ASA-CF             | —                  |
| 23   | PC                 | PC                 | —                  |
| 24   | UltraPA            | UltraPA            | UltraPA            |
| 25   | PA-CF              | PA-CF              | PA12-CF            |
| 26   | UltraPA-CF25       | UltraPA-CF25       | UltraPA-CF25       |
| 27   | PA12-CF            | PA12-CF            | —                  |
| 30   | PAHT-CF            | PAHT-CF            | PAHT-CF            |
| 31   | PAHT-GF            | PAHT-GF            | PAHT-GF            |
| 32   | Support For PAHT   | Support For PAHT   | Support For PAHT   |
| 33   | Support For PET/PA | Support For PET/PA | Support For PET/PA |
| 34   | PC/ABS-FR          | PC/ABS-FR          | PC/ABS-FR          |
| 35   | TPEE               | TPEE               | —                  |
| 36   | PEBA               | PEBA               | —                  |
| 37   | PET-CF             | PET-CF             | PET-CF             |
| 38   | PET-GF             | PET-GF             | PET-GF             |
| 39   | PETG Basic         | PETG Basic         | PETG Basic         |
| 40   | PETG-Tough         | PETG-Tough         | PETG-Tough         |
| 41   | PETG Rapido        | PETG Rapido        | PETG               |
| 42   | PETG-CF            | PETG-CF            | —                  |
| 43   | PETG-GF            | PETG-GF            | —                  |
| 44   | PPS-CF             | PPS-CF             | PPS-CF             |
| 45   | PETG Translucent   | PETG Translucent   | PETG Translucent   |
| 46   | PPS-GF             | PPS-GF             | —                  |
| 47   | PVA                | PVA                | PVA                |
| 48   | TPU-AERO 64D       | TPU-AERO 64D       | —                  |
| 49   | TPU-Aero           | TPU-Aero           | TPU-Aero           |
| 50   | TPU 95A-HF         | TPU 95A-HF         | TPU                |

In the app the materials are grouped as PLA, ABS, ASA, PA / Nylon, PET / PETG, Support,
TPU / Flex and Other.

### Colors (24)

White, Black, Light Gray, Lime Green, Mint Green, Blue, Pink, Yellow, Dark Green,
Light Blue, Dark Blue, Lavender, Yellow-Green, Ocean Blue, Sky Blue, Purple,
Light Red, Red, Beige, Gray, Brown, Tan, Orange, Bronze.

## References

- [QIDI Box RFID spec](https://wiki.qidi3d.com/en/QIDIBOX/RFID)
- [QIDI Box firmware source (community reverse-engineered)](https://github.com/qidi-community/Plus4-Wiki/blob/main/content/qidibox-on-orcaslicer/original_source/box_rfid.py)
- [alexk42/qidi-filament-nfc-flipper](https://github.com/alexk42/qidi-filament-nfc-flipper) — original Flipper implementation this was inspired by
