# FlipBox — CLAUDE.md

Flipper Zero FAP for reading/writing NFC tags used by the QIDI Box filament management system.

## Build

```zsh
# direnv activates venv and sets UFBT_HOME automatically
direnv allow

# build
direnv exec /home/kuno/d/flipbox ufbt

# output
dist/flipbox.fap
```

Without active direnv session:
```zsh
UFBT_HOME=/home/kuno/d/flipbox/.ufbt UFBT_SDK_CHANNEL=release .venv/bin/ufbt
```

## Dev environment

- **direnv** + **uv** — `.envrc` creates `.venv/`, installs `ufbt` via `uv pip install`
- **UFBT_HOME** = `.ufbt/` inside project root (not `~/.ufbt`)
- **UFBT_SDK_CHANNEL** = `release`
- Target: f7, API 87.1
- Firmware: Official release (compatible with Momentum/Unleashed/RogueMaster)

## Project structure

```
flipbox.c / flipbox.h         — entry point, app struct, ViewDispatcher, SceneManager
qidi_tag.c / qidi_tag.h       — material/color tables, encode/decode helpers
nfc_worker.c / nfc_worker.h   — NFC read/write worker
scenes/
  scenes.h / scenes.c         — scene ID enum, three on_enter/on_event/on_exit arrays
  scene_menu.c                — main menu: Read Tag / Write Tag
  scene_read.c                — wait for tag, display result
  scene_write_cfg.c           — write config (VariableItemList: Material/Color/Maker/Write)
  scene_material_group.c      — material group picker (PLA/ABS/ASA/PA/PET/Support/TPU/Other)
  scene_material_select.c     — material picker filtered by group (Submenu)
  scene_color_select.c        — color picker (Submenu, 24 entries)
  scene_write.c               — write tag, display result
```

## NFC / QIDI tag format

- Chip: FM11RF08S (MIFARE Classic 1K compatible), 13.56 MHz, ISO 14443-A
- Data location: **Sector 1, Block 0** = absolute block **4**
- Payload: 3 bytes, hex-encoded
  - `byte[0]` — Material Code (1–50)
  - `byte[1]` — Color Code (1–24)
  - `byte[2]` — Manufacturer Code (default 1 = QIDI)
- Auth keys tried in order:
  1. `D3:F7:D3:F7:D3:F7` — primary (from community implementations, not in official wiki)
  2. `FF:FF:FF:FF:FF:FF` — factory default
- Spec source: `RFID.html` (offline copy of https://wiki.qidi3d.com/en/QIDIBOX/RFID)
- **Manufacturer byte (data[2]) is ignored by the box firmware.** Reverse-engineered from
  `box_rfid.py` (extracted from `.so`): after reading the tag, the box hardcodes `vendor = 1`
  regardless of what is written. The Maker field in the UI is cosmetic / future-proofing only.
- **No temperature data in the tag.** Print/drying temps are hardcoded in box firmware by
  material type, not stored on the chip. The wiki does not mention this.
- **Box firmware validation:** material `1–99` (wider than wiki's 1–50), color `1–24`.
- Firmware source: https://github.com/qidi-community/Plus4-Wiki/blob/main/content/qidibox-on-orcaslicer/original_source/box_rfid.py

## NFC worker

`nfc_worker.c` uses `FuriThread` + synchronous `mf_classic_poller_sync_*` API.
Thread is started in `nfc_worker_start()` and runs for app lifetime.
Scene calls `nfc_worker_read()` / `nfc_worker_write()` → sets worker state → thread picks it up →
calls `mf_classic_poller_sync_read_block` / `mf_classic_poller_sync_write_block` → fires user
callback → `view_dispatcher_send_custom_event`.

Auth keys tried in order: primary D3:F7:D3:F7:D3:F7, then factory default FF:FF:FF:FF:FF:FF.

**Why not event-based poller:** `mf_classic_poller_write_block` fails when called from
`MfClassicPollerEventTypeRequestMode` callback context. Read works but write silently fails.
Sync API (`mf_classic_poller_sync_write_block`) is the reliable path.

**Cancellation:** `nfc_worker_stop()` sets state=Stop and joins thread. Thread exits after current
sync operation completes (fast — sync ops timeout quickly). Stale ReadFail/WriteFail events
delivered after scene change are harmlessly ignored by the new scene.

**Retry:** on fail the NFC scene sets state back to Read/Write by calling `nfc_worker_read/write()`
again. Thread picks it up on the next loop iteration.

## Materials

35 named materials from the official wiki (codes 1–50, gaps at 9, 10, 15–17, 20–23, 28–29,
35–36, 46, 48 — unassigned). Organized into 8 groups in `scene_material_group.c`:
PLA (0-7), ABS (8-11), ASA (12-13), PA/Nylon (14-19), PET/PETG (23-29,31),
Support (20-21), TPU (33-34), Other (22,30,32).

## Navigation

- Write flow: Menu → WriteCfg → MaterialGroup → MaterialSelect → (back to WriteCfg via
  `scene_manager_search_and_switch_to_previous_scene`) → ColorSelect → Write
- `scene_manager_search_and_switch_to_previous_scene(sm, FlipBoxSceneWriteCfg)` skips the
  group scene when returning from material select — user lands directly on WriteCfg.

## Pitfalls

- `SceneManagerHandlers` uses **three separate function pointer arrays** (`on_enter_handlers`,
  `on_event_handlers`, `on_exit_handlers`) — not a per-scene struct.
- Correct callback name: `variable_item_list_set_enter_callback` (not `set_selected_item_callback`)
- `view_dispatcher_enable_queue` is deprecated — do not use
- `variable_item_set_current_value_text` stores a **pointer**, not a copy — two simultaneously
  visible items need two distinct buffers (`text_buf` / `text_buf2` in `FlipBoxApp`)
- Flipper display font is ASCII-only — no Unicode
- In write_cfg: `...` suffix = opens submenu, `>` = executes action (space before `...`: `"%s ..."`)
- NFC worker uses FuriThread with STACK_SIZE=2KB; `stack_size=4KB` in `application.fam` is for the app main thread
