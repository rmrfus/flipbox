# QIDI BOX — RFID Tag Spec

Offline copy of https://wiki.qidi3d.com/en/QIDIBOX/RFID (retrieved 2025-12).

> **Note:** the official spec is incomplete in places. See `CLAUDE.md` for corrections from
> reverse-engineered firmware (`box_rfid.py`).

---

## Chip: FM11RF08S

- Protocol: ISO/IEC 14443-A
- Frequency: 13.56 MHz
- Baud rate: 106 Kbit/s
- Encryption: MIFARE Classic (M1 compatible)

**Memory:** 16 sectors × 4 blocks × 16 bytes = 1024 bytes total.

| Block type | Content | Access |
|---|---|---|
| Manufacturer block | Factory UID | Read-only |
| Sector trailer | Key A / Access bits / Key B | R/W |
| Data block | User data | R/W |

Default factory key: `FF FF FF FF FF FF`

---

## Data layout

Data is stored in **Sector 1, Block 0** (absolute block 4).

| Byte | Field | Range |
|---|---|---|
| `[0]` | Material code | 1–50 |
| `[1]` | Color code | 1–24 |
| `[2]` | Manufacturer code | default 1 |
| `[3–15]` | Unused | `0x00` |

Encoding: raw hex bytes (not ASCII).

**Auth key used by QIDI Box:** `D3 F7 D3 F7 D3 F7` (Key A, Sector 1).
Fallback: `FF FF FF FF FF FF`.

---

## Material codes

| Code | Material | Code | Material |
|---|---|---|---|
| 1 | PLA | 30 | PAHT-CF |
| 2 | PLA Matte | 31 | PAHT-GF |
| 3 | PLA Metal | 32 | Support For PAHT |
| 4 | PLA Silk | 33 | Support For PET/PA |
| 5 | PLA-CF | 34 | PC/ABS-FR |
| 6 | PLA-Wood | 35 | — |
| 7 | PLA Basic | 36 | — |
| 8 | PLA Matte Basic | 37 | PET-CF |
| 9 | — | 38 | PET-GF |
| 10 | — | 39 | PETG Basic |
| 11 | ABS | 40 | PETG Tough |
| 12 | ABS-GF | 41 | PETG Rapido |
| 13 | ABS-Metal | 42 | PETG-CF |
| 14 | ABS-Odorless | 43 | PETG-GF |
| 15 | — | 44 | PPS-CF |
| 16 | — | 45 | PETG Translucent |
| 17 | — | 46 | — |
| 18 | ASA | 47 | PVA |
| 19 | ASA-AERO | 48 | — |
| 20 | — | 49 | TPU-Aero |
| 21 | — | 50 | TPU |
| 22 | — | | |
| 23 | — | | |
| 24 | UltraPA | | |
| 25 | PA-CF | | |
| 26 | UltraPA-CF25 | | |
| 27 | PA12-CF | | |
| 28 | — | | |
| 29 | — | | |

---

## Color codes

| Code | RGB | Code | RGB |
|---|---|---|---|
| 1 | `#FAFAFA` (White) | 13 | `#CADE4B` |
| 2 | `#060606` (Black) | 14 | `#1353AB` |
| 3 | `#D9E3ED` | 15 | `#5EA9FD` |
| 4 | `#5CF30F` | 16 | `#A878FF` |
| 5 | `#63E492` | 17 | `#FE717A` |
| 6 | `#2850FF` | 18 | `#FF362D` (Red) |
| 7 | `#FE98FE` | 19 | `#E2DFCD` |
| 8 | `#DFD628` (Yellow) | 20 | `#898F9B` (Grey) |
| 9 | `#228332` (Green) | 21 | `#6E3812` (Brown) |
| 10 | `#99DEFF` | 22 | `#CAC59F` (Beige) |
| 11 | `#1714B0` (Blue) | 23 | `#F28636` (Orange) |
| 12 | `#CEC0FE` | 24 | `#B87F2B` (Gold) |

---

## Community tools

- [MyFilaments Android app](https://play.google.com/store/apps/details?id=com.fab3dmaker.myfilaments)
- [BoxRFID — Electron desktop app](https://github.com/TinkerBarn/BoxRFID)
- [qidi-filament-nfc-flipper by alexk42](https://github.com/alexk42/qidi-filament-nfc-flipper)
- Compatible blank tags: https://amzn.to/4lu9v7c
