# Sermoon D1 — Custom Marlin Firmware

Marlin 2.0.x bugfix-based, modularized, and extended firmware for the Creality **Sermoon D1** enclosed 3D printer.

**Version**: `SD1-2.9` (base: stock Creality V1.1.10)
**Build target**: Creality V4.3.1 motherboard (STM32F103RET6) — **single target, no other boards supported**
**Last updated**: 2026-07-27

> This fork is cleaned from the stock Sermoon D1 firmware, optimized, and enhanced with features cherry-picked from Marlin 2.1.x.
> All changes are listed in [CHANGELOG.md](CHANGELOG.md).

## Hardware

| Component | Value |
|---|---|
| Motherboard | Creality V4.3.1 (STM32F103RET6, 64KB RAM, 512KB Flash) |
| Stepper drivers | **Mixed** — X/Y: TMC2208 standalone (16x + on-chip 256x interpolation)<br>Z/E0: **HR4988SQ** (16x, **no** interpolation). Both standalone: no UART |
| Motors | Creality **42-40** (on all four axes) — ~1.0 A/phase, ~0.40 N·m |
| Z motors | **Two motors connected in parallel to a single driver** (no second driver, `Z2_DRIVER_TYPE` off) |
| Stepper enable | **All four drivers on a single line: PC3.** It is impossible to disable a single axis independently |
| Driver current | Via Vref pot (no M906). R_sense **0.15 Ω** (`R150`). Measured: X/Y 1.27 V → 0.69 A RMS, Z 1.60 V → 0.47 A/motor, E0 0.86 V → 0.51 A RMS. **Factory setting verified — do not touch** |
| Display | DWIN T5L horizontal (RTS protocol — custom) |
| Print area | 280 × 260 × 310 mm |
| Mechanical bed area | 290 × 270 × 320 mm |
| Extruder | **Direct drive** (gearless MK8 type), single nozzle, 1.75 mm filament |
| EEPROM | BL24C16 (16Kbit, I2C bit-bang) — valid range 0x000–0x7FF |
| M500 settings | **On SD card** `eeprom.dat` — *not* in I2C EEPROM (see below) |
| Temp sensor | 100k thermistor (type 1) hotend + bed |
| Probe | **NONE** — Neither BLTouch nor inductive sensor is installed. Z homing via mechanical endstop (PA7) |
| Filament runout | Mechanical switch (PA4) |
| Enclosed chamber| Z lock module (PB0 + PB1, both) |

## Quick Start

### 1. Build (PlatformIO required)

```powershell
cd C:\sermoon-d1
pio run -e creality
```

Output: `.pio\build\creality\firmware.bin` (126,864 bytes ≈ 124 KB).

> **If you changed the version, do a clean build.** Because `Marlin/Version.h` is included via macro, it is not in the SCons dependency graph; an incremental build won't see it and the firmware will silently carry the old version string:
> `rm -rf .pio/build/creality && pio run -e creality`

### 2. Flash

**Option A — Direct Download (Pre-compiled Binary)**:
Download the latest compiled `firmware.bin` from [GitHub Releases](https://github.com/mosaicenk/Sermoon-D1/releases/tag/SD1-2.9) → Copy to SD root → Reset printer.

**Option B — Build from Source**:
`firmware.bin` → SD root → Reset printer.

**Option C — J-Link**:
```powershell
pio run -e creality -t upload
```

### 3. Pre-flash safety (recommended)

Backup existing settings before flashing the new firmware:
```
docs/eeprom_backup.gcode
```
Run it via Host (OctoPrint/PrusaSlicer console), save the output to a text file.

### 4. Post-flash calibration

> ⚠️ **The SD card MUST BE INSERTED for M500.** On this board, `EEPROM_SETTINGS` is handled by `persistent_store_sdcard.cpp`, not by the I2C EEPROM: settings are written to the `HAL_eeprom_data[2048]` RAM buffer, then flushed to the root of the SD card as **`eeprom.dat`**. If the card is not inserted, `PersistentStore::access_start()` returns `false` and **M500 silently fails** — no settings are persisted.
>
> The BL24C16 I2C EEPROM is only used for: the power-loss recovery block (`PLR_ADDR`, address 2048−sizeof), DWIN language/level flags (`FONT_EEPROM`, address 0–2), and presence check (address 255 = `0x55`).

After flashing the new firmware for the first time (with SD card inserted):

```gcode
M502     ; Load Factory defaults
M500     ; Write to eeprom.dat on SD card
```

Then:
1. **PID calibration** → [docs/pid_tuning/](docs/pid_tuning/README.md)
2. **LIN_ADVANCE K** → [docs/lin_advance/](docs/lin_advance/README.md)

### 5. HR4988SQ (Z/E0) commissioning — mandatory with SD1-2.4

The firmware now times Z and E0 as HR4988SQ. The following **cannot be verified from software**, it must be done manually. Do not change the order.

**a) Direction check — do this first, while the motor is connected**

The motor output pin order of the TMC2208 and A4988 family StepStick modules is inverted. If the driver physically changed, Z and E might be reversed.
To avoid crashing the nozzle into the bed, **test Z while it is high**:

```gcode
G91          ; relative mode
G1 Z5 F300   ; Z should move UP. If it moves down → invert INVERT_Z_DIR
G90
M302 P1      ; allow cold extrusion (only for direction test)
G91
G1 E5 F100   ; filament should go IN. If it comes out → INVERT_E0_DIR
G90
M302 P0      ; disable cold extrusion again
```

If the direction is wrong, invert the value of `INVERT_Z_DIR` / `INVERT_E0_DIR` in `Configuration.h` and recompile. **These values were intentionally not changed** — it cannot be guessed without measuring how the existing hardware is actually wired, and a wrong guess will drive Z into the bed.

**b) Vref (current) — measured, DO NOT CHANGE**

Current is adjusted via the potentiometer; `M906` **does not work**. On 2026-07-23, the Vref of all four drivers was measured and the factory setting was verified to be correct:

| Axis | Vref | Current | % of 42-40 nominal | % of Driver ceiling |
|---|---|---|---|---|
| X/Y | 1.27 V | 0.69 A RMS | 69% | — |
| Z (×2 parallel) | 1.60 V | 0.47 A RMS/motor | 47% | 67% |
| E0 | 0.86 V | 0.51 A RMS | 51% | 36% |

R_sense = **0.15 Ω** (`R150`, all drivers). The formulas do not measure the same thing:

- TMC2208 → Vref sets **RMS**: `I_RMS = Vref × 0.541`
- HR4988SQ → Vref sets **PEAK**: `I_peak = Vref / 1.2`, then `÷ √2`

Copying the Vref of the TMC2208 to the HR4988SQ means a **√2 factor** error.

The only low margin is E0 (51%). If the extruder clicks, stepping up 0.86 → 1.05 V is a safe move (62%, the driver is still at 44%) — **but first perform the K calibration in (d)**, under-extrusion is a more likely cause.

**c) Cooling — non-negotiable**

HR4988SQ runs noticeably hotter than TMC2208 at the same current, and the dual motor load on Z increases this. Moreover, because all four drivers share a single enable line (PC3), **it is not possible to disable a single axis from software due to heating**.
A heatsink + airflow is mandatory. Overheating causes the driver to skip steps due to thermal protection; the symptom is sudden layer shifting mid-print.

**d) LIN_ADVANCE K recalibration**

K is driver-specific, and the driver on E0 changed — the current `0.06` is an uncalibrated starting point. 0.02-0.15 is typical for direct drive. Test live with `M900 K<value>`, when found, `M500`. Method: [docs/lin_advance/](docs/lin_advance/README.md)

**e) Microstep verification**

`DEFAULT_AXIS_STEPS_PER_UNIT` Z=400 and E=95 assumes the drivers are at **16x** microstepping (MS1/MS2 are hard-wired on drivers soldered to the board, no jumpers). If this assumption is wrong, steps/mm will change proportionally (e.g. 8x → Z=200).
Verify by commanding 100 mm and measuring with a ruler.

## Mechanical Parameters

In `Configuration.h` (user calibration, **do not touch**):

| Parameter | Value |
|---|---|
| Steps/mm (X, Y, Z, E) | 80, 79.60, 400, 95 (Y 80 → 79.60 in SD1-1.4 cube measurement calibration) |
| Max speed (mm/s) | 250, 250, 5, 25 (X/Y 300→250: chassis resonance at high speed) |
| Max accel (mm/s²) | 800, 800, 100, 5000 |
| Print accel | 500 mm/s² |
| Travel accel | 800 mm/s² |
| Corner-speed control | `JUNCTION_DEVIATION` 0.015 — **`CLASSIC_JERK` is off**, jerk values are not used |
| Direction invert | X=true, Y/Z/E=false |
| Homing direction | X=MIN, Y=MIN, Z=MIN |

Values saved in EEPROM (M500) override these defaults. For reset use `M502` + `M500`.

## Sermoon-Specific Flags

In `Configuration_adv.h`:

| Flag | Default | Description |
|---|---|---|
| `RTS_AVAILABLE` | ON | DWIN display driver (`Configuration.h`) |
| `EEPROM_PLR` | ON | Power-loss recovery EEPROM save (PLR_ADDR 1852, 196 bytes) |
| `SERMOON_Z_LOCK` | ON | Z axis lock module — PB0 **and** PB1, both |

> `SERMOON_Z_LOCK_AUTO` **was removed** (SD1-2.3). It never worked: `on_motion_start()`/`on_motion_end()` were defined but never called from anywhere in the codebase, so enabling the flag did not change behavior.

> **PB0/PB1 are exclusive.** These pins are the "BLTouch" connector on the board and are entirely dedicated to the Z lock. If a probe is added one day, `SERMOON_Z_LOCK` must be turned off **first** — otherwise `SanityCheck.h` will stop compilation with an error (intentionally: loud error instead of a silent pin conflict).

### M-Codes (Sermoon-specific)

```gcode
M888              ; Query Z lock status
M888 S0           ; Z lock release
M888 S1           ; Z lock engage (default)
```

## Backported Features (from Marlin 2.1.x)

All are default OFF — enable what you want to use in `Configuration_adv.h`:

```c
//#define AUTO_REPORT_POSITION   // M154 — host position reporting
#define SAVED_POSITIONS 0        // 1+ → G60/G61 enabled
//#define GCODE_REPEAT_MARKERS   // M808 — gcode loop
//#define HOTEND_IDLE_TIMEOUT    // Enclosed chamber filament-charring protection
```

15 new thermistor types were also added (no burden on flash unless used).

## Active Tuning Settings

Configuration activations:

### Safety
- `NO_TIMEOUTS 1000` — host connection stability
- `HOMING_BACKOFF_MM { 1, 1, 2 }` — SD1-2.9: X/Y retracts 1 mm, parks at −9
- `NO_MOTION_BEFORE_HOMING` — blocks movement before homing
- `Z_HOMING_HEIGHT 4` — Z+4mm up before home

### Print Quality
- `LIN_ADVANCE` (K=0.06, **calibration mandatory** — 0.02-0.15 typical for direct drive)
- `FWRETRACT` (G10/G11 — slicer-independent retraction)
- `S_CURVE_ACCELERATION` — sigmoid speed profile
- `JUNCTION_DEVIATION` (0.015) — modern corner-speed control instead of CLASSIC_JERK
- `ADVANCED_PAUSE_FEATURE` (Advanced M600)
- `ADAPTIVE_STEP_SMOOTHING` — effective microstep doubling at low speeds
- `MINIMUM_STEPPER_PULSE 1` (for TMC2208)
- `MAXIMUM_STEPPER_RATE 400000` (TMC2208 max)

### Ease of Use
- `HOST_ACTION_COMMANDS` — OctoPrint/PrusaSlicer integration
- `GCODE_MOTION_MODES` — G1 motion mode memory
- `PAREN_COMMENTS` — `(comment)` syntax
- ~~`CANCEL_OBJECTS`~~ — default OFF (2026-05-23 optimization); enable if M486 is needed
- ~~`GCODE_MACROS`~~ — default OFF (2026-05-23 optimization); enable if macros are needed
- ~~`QUICK_HOME`~~ — **Disabled in SD1-2.7**; X and Y now home sequentially

### Performance
- `BUFSIZE 8` — host streaming smoothness
- `RX_BUFFER_SIZE 512` — more than enough for 115200 baud (2026-05-23: 1024→512)
- `BLOCK_BUFFER_SIZE 32` — flow continuity (2026-05-23: 16→32)
- `DEFAULT_STEPPER_DEACTIVE_TIME 300` — 5 min inactive timeout (Z lock present)
- `SOFT_PWM_SCALE 7` + `SOFT_PWM_DITHER` — fan PWM ~7.8 Hz → ~1 kHz (whine ↓)
- `FAN_KICKSTART_TIME 100` — fan stall prevention
- `FAN_MIN_PWM 50` — skips low PWM dead-zone
- `MINIMUM_STEPPER_PULSE 1` — TMC2208 + LIN_ADVANCE compatible (2026-05-23: 0→1)
- `DEFAULT_MINSEGMENTTIME 8000` µs — retract/fine moves (2026-05-23: 20000→8000)
- `MM_PER_ARC_SEGMENT 2` — ARC block reduction (2026-05-23: 1→2)

### EEPROM (BL24C16 I2C bit-bang) optimizations (2026-05-23)
- `BL24CXX_Check()` — 1 time per boot (previously: 2-3 I2C transactions)
- `BL24CXX_Read()` — sequential read (~2x speedup)
- `BL24CXX_Write()` — 16-byte page write (~15x speedup)
- PLR 120-byte read ~30ms→~15ms; PLR write ~600ms→~40ms

### SHOW_REMAINING_TIME — DWIN specific (2026-05-23)
- Marlin's standard `SHOW_REMAINING_TIME` requires HAS_GRAPHICAL_LCD or EXTENSIBLE_UI; Sermoon DWIN is neither.
- **DWIN specific calculator**: In `LCD_RTS.cpp::EachMomentUpdate`, the remaining time is calculated in minutes with the formula `elapsed_sec * (100 - pct) / pct / 60`.
- **New VP**: `PRINT_REMAIN_MIN_VP = 0x1410`. The remaining time is written as minutes. It is the user's job to link a text field to this VP in the DWIN screen design (in the DWIN screen design tool).
- Linear extrapolation: initially (~1%) the estimate will be high, quite consistent after 50%+. Writes 0 at 0% or 100%.

### Stat
- `PRINTCOUNTER` — total print time/count (M78)
- `NOZZLE_PARK_FEATURE` — M600/M125 park

## Hardware Limitations (Transparency)

Some modern features cannot be activated due to hardware/HAL constraints:

| Feature | Reason |
|---|---|
| Hardware microstep change (M350) | Both driver families are standalone; MS1/MS2 are hard-wired to the PCB (no jumpers), not connected to MCU |
| Driver current from software (M906) | No UART/SPI — both TMC2208 and HR4988SQ are adjusted via Vref pot |
| Disable a single axis independently | Four drivers are on a single enable line (PC3); when one closes, they all close |
| `FAST_PWM_FAN` | PA0 → TIM2/TIM5 timer conflict (with TEMP/STEP) |
| `EMERGENCY_PARSER` | No implementation in STM32F1 HAL |
| MPC, Input Shaping | Would require full migration to Marlin 2.1.x (~50+ hours) |

## Build Footprint

| Metric | Value |
|---|---|
| Flash | **126,864 bytes** (24.2% / 524288 bytes) |
| RAM | **13,176 bytes** (20.1% / 65536 bytes) |
| Compile warning | **0** (project code) + 1 upstream (`util_adc.c`, framework) |
| Build time | ~12 sec (clean) |
| `firmware.bin` SHA256 | `A9567E83…23DA` (2026-07-27 build — depends on the day, see note) |

> ⚠️ **SHA256 depends on the day.** `Marlin.cpp:956` embeds `__DATE__` into the binary (like `Compiled: Jul 23 2026`). A clean build on another day produces a different hash; size and behavior do not change. Bit-by-bit comparison is only meaningful between two builds done on the **same day**. Measurement (2026-07-24): comment change revert/re-apply experiment yielded the exact same hash on the same day (`4402B902…AD818`); the only difference from yesterday's `BDAB96BB…B987` is the date string.
>
> **2026-07-21 (SD1-2.1)**: Since Z-probe code was disabled, Flash −2,832 bytes, RAM −16 bytes. The remaining 3 DWIN warnings were also fixed → zero warnings in project code.
>
> **2026-07-21 (SD1-2.2)**: 111 dead files deleted (99 `.cpp` + 12 `.h`). Flash/RAM **unchanged** — the deleted files' contribution to the binary was already measured at 0 bytes. The generated binary is bit-for-bit identical to 2.1 (SHA256 `E0CDBDE9…547E`) → **no need to re-flash**.

> **2026-07-22 (SD1-2.3)**: At toolchain level **−57,116 bytes of flash** (35.1% → 24.2%) and **−1,992 bytes of RAM**. Source logic didn't change; all due to compiler/linker configuration. Details: [CHANGELOG.md](CHANGELOG.md).
> Binary changed → **needs re-flashing.**

> **2026-07-23 (SD1-2.4)**: Z/E0 drivers defined as HR4988SQ (mixed config). Flash **+40 bytes** (127,080 → 127,120) — all of the increase comes from the `MINIMUM_STEPPER_*_DIR_DELAY` 30 ns → **200 ns** fix; 30 ns was insufficient for HR4988SQ and carried the risk of reversed steps during direction changes. Binary changed → **needs re-flashing.**
> Pre/post-flash tasks: [§4.5](#5-hr4988sq-ze0-commissioning--mandatory-with-sd1-24).

### Dead features remaining in the binary

The following values were **measured** with `arm-none-eabi-nm --print-size` (not an estimate). Total ~3.8 KB (3.0%). The gain of cleaning is not worth the regression risk:

| Feature | Measured | Reason for leaving |
|---|---|---|
| `BEZIER_CURVE_SUPPORT` (G5) | 898 B | Completely dead; slicers don't output G5 |
| `ARC_SUPPORT` (G2/G3) | 1,358 B | Some slicers output with arc fitting |
| `PRINTCOUNTER` | 760 B | Real feature, displayed on screen |
| `SPIClass` ctor | 500 B | SD card uses SDIO; linker already dropped the body |
| `FWRETRACT` (G10/G11) | 304 B | Small, harmless |

> ⚠️ **`backtrace` is NOT in the binary.** In previous versions, this table listed it as "3,682 B, prints stack trace on hardfault"; measurement does not confirm this — **none** of the `unwarm*`/`UnwReport*` symbols are in the binary, they were all dropped by `--gc-sections` (source files are compiled but called from nowhere). If a stack trace is desired for hardfault debugging, the feature must practically be linked to a fault handler.

## Documentation

| Content | Location |
|---|---|
| **All documentation TOC** | [`docs/README.md`](docs/README.md) |
| Change history | [`CHANGELOG.md`](CHANGELOG.md) |
| PID calibration | [`docs/pid_tuning/`](docs/pid_tuning/README.md) |
| LIN_ADVANCE calibration | [`docs/lin_advance/`](docs/lin_advance/README.md) |
| Junction Deviation calibration | [`docs/junction_deviation/`](docs/junction_deviation/README.md) |
| EEPROM backup gcode | [`docs/eeprom_backup.gcode`](docs/eeprom_backup.gcode) |

## Important Files (codebase)

| Path | Content |
|---|---|
| `Marlin/Configuration.h` | Printer parameters (mechanical, temperature, endstops) |
| `Marlin/Configuration_adv.h` | Advanced settings (TMC, babystep, runout, fan, etc.) |
| `Marlin/Version.h` | Version strings |
| `Marlin/src/lcd/dwin/LCD_RTS.cpp` | DWIN display driver (~2535 lines, RTS protocol) |
| `Marlin/src/lcd/dwin/i2c_eeprom.cpp` | BL24C16 bit-bang I2C EEPROM |
| `Marlin/src/feature/sermoon_zlock.cpp` | Z lock module |
| `Marlin/src/pins/stm32/pins_CREALITY.h` | Motherboard pin assignments |
| `Marlin/src/HAL/HAL_STM32F1/` | Hardware abstraction (libmaple-based) |

## License

Marlin is distributed under the GPL-3.0 license. For details see [`LICENSE`](LICENSE).

---

## Developer Note

All work done was based on a **conservative** principle:
- Mechanical parameters were not changed (preserved as user calibration)
- New features were added as default OFF (Tier 1 backports)
- API breaking refactors were skipped (HostUI class, ExtUI architecture change, etc.)
- No deep intervention was made in the STM32F1 HAL (FAST_PWM_FAN, EMERGENCY_PARSER)
- Build was verified at each stage, final status is clean with 0 warnings

Untested areas:
- The physical behavior of the new Configuration activations should be verified on the printer
- **SD1-2.3 newlib-nano transition**: The three paths using `dtostrf()` must be verified on hardware — M114 position report, Z/E values on the M600/pause screen, and the `G92.9 E<value>` command generated by power-loss recovery. If decimal places are printed correctly, `-u_printf_float` is working properly.
- LIN_ADVANCE K=0.06 is a reasonable **starting point** for direct drive, but user calibration is mandatory since the E0 driver changed (SD1-2.4) (typically between 0.02-0.15)
- **SD1-2.1 PLR fix must be verified on hardware** — address math and block boundaries were guaranteed at compile-time via `static_assert`, but a real power outage scenario (pull the plug during print → turn on → screen should prompt "resume") requires physical testing.
- MINTEMP 0 → 5 change: if the printer is run in an environment below 5 °C, it will throw a MINTEMP error on cold boot. If it's to be used in an unheated workshop, the value can be lowered (should not be 0).

> **Version control**: A git repository was set up with SD1-2.2. The `1d2ba27` commit freezes the full state (433 files) prior to cleanup — any deleted file can be recovered from there: `git checkout 1d2ba27 -- <path>`.

For questions or improvement suggestions, look at CHANGELOG or examine the code directly — the architecture is modular and documented.
