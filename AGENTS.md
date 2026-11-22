# ETLTest — agent instructions

`ETLTest` is the test bench for the [ETL](https://github.com/JimorMarlow/ETL) library
(`etl::vector`, `array`, `lookup`, `queue`, `optional`, `filter`, `led`, `settings`, `little_fs`,
`wifi`, `webui`, `mqtt`, `espnow`, ...). The library code is **not** in this repository:
`platformio.ini` pulls ETL from GitHub through `lib_deps`, and `src/main.cpp` runs
`etl::unittest::test_all(Serial)` on the board. `src/light_*.{h,cpp}` are an optional
WEB-UI/MQTT experiment, compiled only under `USE_WIFI_UI_SERVER` (currently commented out in
`main.cpp`).

The local ETL checkout lives next to this repository: `C:\Projects\Arduino\ETL`.

## Canary

Owner of this project: the value of `git config user.name` (currently `JimorMarlow`). Take it
from git yourself, do not ask. Start **every** answer with the owner's name and a comma, then
continue the answer on the same line — so it is visible that this file was read and the rule is
followed:

`JimorMarlow, <the answer>`

Even a one-line answer, an error report or a question starts with it.

## No STL — extend ETL instead

- Do not use STL (`std::vector`, `std::string`, ...) here: only `etl::`, no exceptions, no RTTI,
  no unnecessary dynamic allocations.
- If ETL lacks a common tool that the task needs, do not substitute `std::` and do not keep a
  local workaround: implement the missing functionality in this project in an `etl_*` file,
  debug it on the bench (switch `lib_deps` to `../ETL` for a local build — see "Local ETL
  debugging"), and when it works propose adding it to the ETL library to the developer.

## Build and verification

`pio` is not in PATH — call it by full path. Run it from the project root (`-d` is not needed):

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run                      # all environments
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -e d1_mini_lite      # ESP8266
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -e nodemcuv3         # ESP8266
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -e esp32c3           # ESP32-C3
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -e esp32-wroom-32u   # ESP32
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -e esp32-wroom-30u   # ESP32
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -e esp32-wroom-38u   # ESP32
```

- **All six** environments must build. Before saying "done", show the result for each env
  (RAM/Flash, errors).
- Build commands need no approval. Uploading, monitoring and erasing a board touch hardware — do
  them **only on the user's explicit request** and only for the board named in it (see
  "Hardware check on request" below):

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -e esp32c3 -t upload
& "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" device monitor -b 115200
```

- Read compilation errors from the whole output: the first error is the cause, the rest are
  usually its consequences.
- Filesystem image (`partitions/*.csv` partition schemes, `data/` contents):
  `... pio run -e <env> -t buildfs`, then `-t uploadfs`. Do not remove
  `platform_packages = platformio/tool-mkspiffs@^1` from the envs — the image tasks need it.
- There are no host-side tests: `test/` is empty, the tests live in ETL (`src/etl/etl_test.cpp`)
  and run on the device through `src/main.cpp`. The correctness criterion is a build of every env
  plus the `test_all()` output on hardware (when a board is connected).
- Full clean: `... pio run -t clean`. Do not wipe `.pio` without a reason — rebuilds get slower.

### Hardware check on request

Three test boards cover all six environments:

| Board | Platform | Envs checked on it |
|---|---|---|
| Wemos D1 mini | ESP8266 | `d1_mini_lite`, `nodemcuv3` |
| ESP32-C3 mini | ESP32-C3 | `esp32c3` |
| ESP32 WROOM-32 | ESP32 | `esp32-wroom-32u`, `esp32-wroom-30u`, `esp32-wroom-38u` |

- Run a check on the device **only when the user asks for it**. The user says which board is
  connected; never upload, monitor or erase on your own initiative.
- Do not assume a board from an earlier request: ask which one is plugged in, and if the named
  board does not match the env, say so before touching the port.
- One board stands for a whole platform, so the env comes from the task, not from the board: the
  D1 mini runs both ESP8266 envs, the WROOM-32 runs all three ESP32-WROOM envs.
- Report what the board actually printed (`test_all()` output). All six builds plus that output are
  the correctness criterion; a green build alone says nothing about device behaviour.

## Local ETL debugging

ETL is normally taken from GitHub (`https://github.com/JimorMarlow/ETL@^1.1.4`). To test a local
copy, replace that line with `../ETL` in `[env] lib_deps` — and restore the GitHub line before
handing the work back, otherwise the project no longer pins what it tests.

Each env keeps its own copy in `.pio/libdeps/<env>/ETL`; an env that was not rebuilt since the
change may still hold an older version. Check `.pio/libdeps/<env>/ETL/src/etl/etl_version.h` or run
`... pio pkg update` before reporting a version mismatch.

## Adding a new board

One env is not enough — a board is "supported" only when all four steps are done:

1. `[env:<name>]` in `platformio.ini` with its own `-D BOARD_XXX` in `build_flags`;
2. a branch for `BOARD_XXX` in `src/pinout.h` (`LED_BUILTIN`, `INVERSE_BUILTING_LED`,
   `SERIAL_INIT_DELAY`);
3. a row in the board table in `readme.md`;
4. the env builds.

An unknown board falls into the `#else` branch of `pinout.h` and prints a `#pragma message` — that
is a configuration error, not a warning to live with.

## Platform and dependencies

- ESP32 environments use `platform = espressif32 @ ^8.0.0` (Arduino core 3.x, currently 3.3.12).
  The pin must be present in **every** ESP32 env (`esp32c3`, `esp32-wroom-32u/30u/38u`) — that is
  what keeps the test bench on core 3.x instead of silently building against an older core.
- The commented-out `-D ESP32` lines in `platformio.ini` stay commented out: under core 3.x `ESP32`
  is not defined, and defining it by hand only hides which branch is really compiled.
- ESP32 envs need `-std=gnu++17` plus `build_unflags = -std=gnu++11, -Wregister` — core 3.x does
  not build this code as gnu++11.
- `lib_ldf_mode = deep` (not `deep+`): otherwise the system FS and Network libraries are not pulled
  into the build, and the ESP32 build fails with `Network.h: No such file or directory`.
- Dependencies are pinned (GTimer, FileData, ArduinoJson, PubSubClient, ETL); `async-mqtt-client` is
  taken from a git URL. Add a new library only on request; do not bump versions "while at it".

## Platform-specific code: ETL_ESP32 / ETL_ESP8266 and BOARD_xxx

Two different things, two different tools:

- **Platform** (ESP8266 vs ESP32) — the macros from ETL, `src/etl/etl_defines.h`:
  `ETL_ESP32` / `ETL_ESP8266`.
- **Board** (which ESP32 to flash) — the `BOARD_xxx` defines from `platformio.ini` through
  `src/pinout.h`.

```cpp
#include "etl/etl_defines.h"   // idempotent; start the file with it, not with <Arduino.h>

#if defined(ETL_ESP32)
    ...
#elif defined(ETL_ESP8266)
    ...
#else
    #pragma message("ERROR: <file>: board not defined")
#endif
```

- **Never `#ifdef ESP32`.** Since `espressif32 @ ^8.0.0` (core 3.x) `ESP32` is not defined, so the
  guard silently compiles the file to nothing and the failure only shows up on the device. That is
  exactly how ETL's `etl_led` broke when moving to 8.x.
- `ESP8266` is still defined, but use `ETL_ESP8266` anyway, so both branches look the same.
- `src/light_*.{h,cpp}` and `src/etl_amqtt.*` are already converted to the ETL macros (v1.2.1) —
  keep them that way. Before v1.2.1 they used `#if defined(ESP8266) || defined(ESP32)`, so on core
  3.x the ESP32 build lost `light_web_server_t` and the whole implementations.
- `#warning "ETL: Platform not detected. ETL_ESP8266 / ETL_ESP32 are undefined."` in the log is an
  environment configuration error (or a forgotten include), not a warning to deal with later.

## What must not break

- ESP8266 and ESP32 building at the same time — a change to one platform must not break the other.
- No STL: only `etl::` (`vector`, `array`, `lookup`, ...), no exceptions and no RTTI, no unnecessary
  dynamic allocations.
- Version: `src/version.h` is the source of truth; `sync_version.py` (a build pre-script) copies it
  into `package.json`. PATCH for regular changes, MINOR for new features. After changing the version
  by hand run `python sync_version.py`.
- `readme.md` is the version history: on release add a section in Russian (`### v1.2.0` + bullets),
  in the existing style, and keep the board table in sync with `platformio.ini`.
- `partitions/*.csv` are partition schemes, `data/` is the filesystem content. Do not touch them
  without a reason.
- `src/secret.h` is not in the repository: `src/secret.h.example` is the template, the real file
  comes from the private `ETLSecrets` repo and is git-ignored. Never commit it, and never put real
  credentials into `platformio.ini` — the `mywifissid` / `mypass` placeholders stay placeholders.
- `docs/` holds task descriptions (`tasks/`), HTML prototypes and the older
  `rules/qwen_make_no_mistakes.md`. `AGENTS.md` is the only instruction file now: the former
  top-level `rules/README.md` and `QWEN.md` are merged into it and deleted — do not recreate them,
  update this file instead.

## Style

- Minimal diff: same style, same names; never reformat someone else's file wholesale.
- Comments — to the point and in Russian; do not comment the obvious.
- Answer the user in Russian.
- Never commit or push to git yourself. On request, prepare a commit message (short, in Russian, in
  the style of the history: `v1.2.0 - migration to ETLv1.2.x with API core ESP32 3.x`) and show it —
  the user pushes it after review.

## Accuracy

- Never invent APIs, members or build flags: read the header or the tool's own documentation that
  declares them first. If a call cannot be found in the source, say so and ask — a plausible-looking
  line that does not compile costs more than a question.
- When unsure about platform behaviour, state the doubt in the answer (and `// FIXME:` in the code)
  instead of presenting a guess as a fact.
- For the reason behind a decision, read the commit history (`git log`) — the messages carry it, not
  just the change.

## Workflow

1. Read the affected files (and `readme.md`) first, only then edit.
2. Propose the solution and wait for approval first — never change code without permission.
3. Change one logical thing at a time, and build all six envs after each change.
4. Use subagents for code search and for reviewing others' changes, but make the edits yourself.
5. If a build fails for an environment reason (no toolchain, no board, no network), say so
   directly; do not present it as a code bug.
