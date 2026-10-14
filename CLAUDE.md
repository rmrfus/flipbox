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
UFBT_HOME=/home/kuno/d/flipbox/.ufbt .venv/bin/ufbt
```

Switch SDK channel (stored in `$UFBT_HOME/current/ufbt_state.json`; ufbt has no env var for it):
```zsh
direnv exec /home/kuno/d/flipbox ufbt update --channel=rc   # or release / dev
```

## Dev environment

- **direnv** + **uv** — `.envrc` creates `.venv/`, installs `ufbt` via `uv pip install`
- **UFBT_HOME** = `.ufbt/` inside project root (not `~/.ufbt`)
- Target: f7. A FAP loads only on firmware with the **same API major** (`ApiTooOld`/`ApiTooNew` otherwise).
  As of 2026-10: OFW release 1.4.3 and Momentum mntm-012 are API 87; OFW 1.5.x-rc, Unleashed
  (since unlshd-090) and RogueMaster (since RM0713) are API 88. CI builds one pinned SDK per
  major and names the output `flipbox-api<major>.fap` (see Pitfalls).

## Project structure

```
flipbox.c / flipbox.h         — entry point, app struct, ViewDispatcher, SceneManager
qidi_tag.c / qidi_tag.h       — material/color tables, encode/decode helpers
nfc_worker.c / nfc_worker.h   — NFC read/write worker
scenes/
  scenes.h / scenes.c         — scene ID enum, three on_enter/on_event/on_exit arrays
  scene_menu.c                — main menu: Read Tag / Write Tag
  scene_read.c                — wait for tag, display result
  scene_write_cfg.c           — write config (VariableItemList: Printer/Material/Color/Maker/Write)
  scene_material_group.c      — material group picker (groups with materials on the selected printer)
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
  - `byte[2]` — Manufacturer Code (`[vendor_list]` in the cfg: 0 = Generic, 1 = QIDI)
- Auth keys tried in order:
  1. `D3:F7:D3:F7:D3:F7` — primary (from community implementations, not in official wiki)
  2. `FF:FF:FF:FF:FF:FF` — factory default
- Spec source: `RFID.md` (offline copy of https://wiki.qidi3d.com/en/QIDIBOX/RFID). The wiki's
  material table is stale; the source of truth for names is each printer's
  `config/officiall_filas_list.cfg` in QIDITECH/QIDI_Q2, QIDI_MAX4, QIDI_PLUS4 (see Materials).
- **Manufacturer byte (data[2]) is ignored by the box firmware.** Reverse-engineered from
  `box_rfid.py` (extracted from `.so`): after reading the tag, the box hardcodes `vendor = 1`
  regardless of what is written. The Maker field in the UI is cosmetic / future-proofing only.
- **No temperature data in the tag.** Print/drying temps are hardcoded in box firmware by
  material type, not stored on the chip. The wiki does not mention this.
- **Box firmware validation:** material `1–99` (wider than wiki's 1–50), color `1–24`.
  The Q2 V1.1.1 `box_rfid.so` (Cython) still carries the same constants (1, 24, 99) and symbol set
  as the Plus4 `box_rfid.py`; not decompiled. Community forks of the cfg add codes 51+ and vendors 2+.
- **QIDIStudio "gen-2 box"** (`identity: "box_v2"` in `multi_color_controller`) only hides the Unload
  button in Studio's UI; no different tag format was found.
- Firmware source: https://github.com/qidi-community/Plus4-Wiki/blob/main/content/qidibox-on-orcaslicer/original_source/box_rfid.py

## NFC worker

`nfc_worker.c` uses `FuriThread` + synchronous `mf_classic_poller_sync_*` API.
Thread is started in `nfc_worker_start()` and runs for app lifetime.
Scene calls `nfc_worker_read()` / `nfc_worker_write()` → posts a request → thread picks it up →
calls `mf_classic_poller_sync_read_block` / `mf_classic_poller_sync_write_block` → fires user
callback → `view_dispatcher_send_custom_event`.

Auth keys tried in order: primary D3:F7:D3:F7:D3:F7, then factory default FF:FF:FF:FF:FF:FF.

**Why not event-based poller:** `mf_classic_poller_write_block` fails when called from
`MfClassicPollerEventTypeRequestMode` callback context. Read works but write silently fails.
Sync API (`mf_classic_poller_sync_write_block`) is the reliable path.

**Synchronization:** request (`cmd`), `write_data`, `generation` and `stop` live under one
`FuriMutex`. The worker takes the request atomically (resetting it to Idle) and remembers its
generation; every request, `nfc_worker_cancel()` and `nfc_worker_stop()` bump the generation, and
a result is delivered only if the generation is unchanged. So a cancelled or superseded request
never fires the callback, and the worker never writes `stop` (no lost shutdown).

**Cancellation:** Read and Write scenes call `nfc_worker_cancel()` in `on_exit`. A sync call that
is already on the air can't be aborted (it returns fast: without a tag the first activation
attempt errors out), but its result is dropped and the fallback-key attempt is skipped.
`nfc_worker_stop()` sets `stop` and joins; the thread exits after the current sync call.

**Retry:** on fail the NFC scene posts the request again by calling `nfc_worker_read/write()`
again. Thread picks it up on the next loop iteration.

## Materials

Code → name mapping is **per printer**: each printer resolves codes through its own
`officiall_filas_list.cfg`, and the tables differ (Plus4 V1.7.1: 25 = PA12-CF, 27 unused; Q2/Max4:
25 = PA-CF, 27 = PA12-CF; Q2 lacks Max4's 15 = TPU-GF). Sources: QIDI_Q2 V1.1.1 (2026-01),
QIDI_MAX4 (2026-08), QIDI_PLUS4 V1.7.1 (2025-08).
**Plus5 / Q2C: no public cfg yet** — not offered in the printer list; README says so. Add them
once QIDI publishes `officiall_filas_list.cfg` for them.

`qidi_materials[]` has one row per (code, name) with a printer bitmask and a `QidiGroup`; sorted by
code, menus keep that order. Q2 = 42 rows, Max4 = 43, Plus4 = 32. Groups (`qidi_group_names[]`):
PLA, ABS, ASA, PA / Nylon, PET / PETG, Support, TPU / Flex, Other. The group picker hides groups
with no material on the selected printer.

The selected printer (`app->printer`) applies to both read and write, and is saved to
`/ext/apps_data/flipbox/settings.bin` (`saved_struct`) on exit as the raw `QidiPrinter` ordinal —
**append new printers at the end of the enum**, never insert or reorder. Invariant: `app->mat_idx` always
points at a row available on `app->printer`; switching printer keeps the row if shared, else
resets to the printer's first material.

## Navigation

- WriteCfg rows: Printer (left/right) / Material ... / Color ... / Maker (left/right) / Write Tag >.
  The Material/Color row is kept in scene state so returning from a picker lands on it. Write Tag
  is never remembered, so no entry into WriteCfg puts the cursor one OK away from writing.
- Write flow: Menu → WriteCfg → MaterialGroup → MaterialSelect → (back to WriteCfg via
  `scene_manager_search_and_switch_to_previous_scene`) → ColorSelect → Write
- Read result offers "Edit & Write" only when material and color are both in the selected
  printer's table; editing would otherwise silently replace the unknown code.
- `scene_manager_search_and_switch_to_previous_scene(sm, FlipBoxSceneWriteCfg)` skips the
  group scene when returning from material select — user lands directly on WriteCfg.

## Device tests

Driven by the `rmrf-code:flipper` skill (`flipper.py info` / `flipper.py run -f ...`), which
takes screenshots and presses keys over USB RPC. Scenarios live in `tests/device/`:

- `smoke.steps` — menu, printer switching, per-printer groups/materials, row restore,
  persistence of the printer, read popup. No tag needed and never enters Write, so a tag left on
  the Flipper is safe. Run after every deploy.
- `nfc-roundtrip.steps` — **writes the tag twice** (PLA Silk / Red, then PA12-CF read back with
  Plus4 to check `#27` and the missing Edit & Write). Ask the user first; needs a spare
  MIFARE Classic 1K tag on the Flipper.

Both stash `settings.bin` and restore it afterwards. Before deploying, `flipper.py info` gives the
device's API major; build with the matching SDK (see the CI matrix for the zip URLs).

## Pitfalls

- `SceneManagerHandlers` uses **three separate function pointer arrays** (`on_enter_handlers`,
  `on_event_handlers`, `on_exit_handlers`) — not a per-scene struct.
- Correct callback name: `variable_item_list_set_enter_callback` (not `set_selected_item_callback`)
- `view_dispatcher_enable_queue` is deprecated — do not use
- VariableItemList stores the **label as a pointer** (must outlive the item), while
  `variable_item_set_current_value_text` **copies** into a FuriString.
- VariableItemList column split is **fixed in pixels**: label x=6..73 (67px ≈11 chars), value
  centered at x=95 between 73..115 (42px ≈7 chars). Rendering of longer text differs by firmware:
  - Momentum (mntm-012, checked on device): label and value are clipped to their columns; the
    selected row marquee-scrolls both, unselected rows show the truncated start ("PLA Rapi").
  - OFW 1.4.3 / 1.5.1-rc (source only, not checked on device): plain `canvas_draw_str_aligned`,
    no clipping or scrolling, so a long value can spill past its column.
- Flipper display font is ASCII-only — no Unicode
- In write_cfg: `...` suffix = opens submenu, `>` = executes action (space before `...`: `"%s ..."`)
- CI pins one SDK per API major (`matrix.include` in `.github/workflows/build.yml`, fetched with
  `ufbt update --hw-target=f7 --url=...`; URL mode requires `--hw-target`). Bump `sdk` there when
  a newer firmware of that major ships (e.g. 88 → OFW 1.5.x release), add a row for a new major.
- NFC worker uses FuriThread with STACK_SIZE=2KB; `stack_size=4KB` in `application.fam` is for the app main thread
