# Sermoon D1 Firmware â€” Changelog

This document logs all changes made on top of the base version (`stock Creality V1.1.10`, Marlin 2.0.x bugfix branch).

## [SD1-2.9] - 2026-07-27

**Fixes & Refinements**
- **S-Curve / Acceleration fix (`Configuration.h`)**: `IMPROVE_HOMING_RELIABILITY` (X/Y accel 100 mm/sÂ² during homing) macro was completely useless before SD1-2.6 because it was inside the `HAS_TRINAMIC` block. After it was fixed, X/Y hit the endstops softly â€” but the return from the 5mm backoff was very slow (`HOMING_BUMP_DIVISOR` default is 2 â†’ `HOMING_FEEDRATE_XY` / 2 = 500 mm/m). Backing off 5mm and returning at 500 mm/m took unnecessarily long.
  - `HOMING_BUMP_DIVISOR` was explicitly defined: X/Y/Z are now **4** (1000 / 4 = 250 mm/m for X/Y, 240 / 4 = 60 mm/m for Z). The second touch is now slower and more precise.
  - `HOMING_BACKOFF_MM` X and Y components changed from 0 to **1 mm** (Z remains 2).
  - `X_MIN_POS` / `Y_MIN_POS` changed from âˆ’10 to **âˆ’9**.
  - **Result:** After G28, the nozzle parks at **(âˆ’9, âˆ’9)** instead of (âˆ’10, âˆ’10). The endstop lever is now **released** while parking (unlike SD1-2.8, see [MANUAL.md Â§6.5](MANUAL.md#65-impact-of-park-position-on-diagnostics-sd1-28)). Homing diagnostic ambiguity (`M119`) is resolved without sacrificing the speed gained in SD1-2.7. Z homing is more precise due to the divisor change.

## [SD1-2.8] - 2026-07-26

**Fixes & Refinements**
- **Z-Safe Homing point offset (`Configuration.h`)**: In SD1-2.7, `Z_SAFE_HOMING_X/Y_POINT` was set to `(X_MIN_POS) / (Y_MIN_POS)` = (âˆ’10, âˆ’10) so that the printhead wouldn't travel to the center (145, 135) for Z homing (since there is no probe, moving to the center is unnecessary). However, `HOMING_BACKOFF_MM` was pulling the head to (âˆ’8, âˆ’8) after X/Y homing. As a result, before Z could drop, a meaningless 2 mm diagonal movement from (âˆ’8, âˆ’8) to (âˆ’10, âˆ’10) was occurring.
  - **Fix:** In `Configuration_adv.h`, `HOMING_BACKOFF_MM` (X and Y components) was set to **0**.
  - Now the printhead homes X and Y, stops exactly on the endstops (âˆ’10, âˆ’10), and Z homes straight down without any extra movement. The side effect of resting on the endstop is documented in `MANUAL.md`.

## [SD1-2.7] - 2026-07-25

**Refactor**
- **Homing optimization (`Configuration.h`)**:
  - `QUICK_HOME` **disabled**. The simultaneous diagonal homing of X and Y caused the two 42-40 motors to draw peak current simultaneously, and visually it looked violent. Sequential homing is safer and puts less strain on the PSU.
  - `Z_SAFE_HOMING_X_POINT` and `Y_POINT` changed from `(X_BED_SIZE / 2)` to `(X_MIN_POS)`. Since there is no Z-probe, taking the head to the center of the bed for Z homing wastes time (the mechanical Z-endstop is fixed to the frame, X/Y position does not change the Z=0 plane). Z now homes directly where X/Y finishes homing (front-left corner). **This saves ~10 seconds per print.**

## [SD1-2.6] - 2026-07-24

**Fixes & Refinements**
- **Feature block error fixed (`Configuration_adv.h`)**: The macro `IMPROVE_HOMING_RELIABILITY` (which lowers acceleration during homing to prevent hard crashes) was defined inside the `@section tmc_smart` block. Since the TMC block is excluded from the build on this board (`HAS_TRINAMIC` is false), the macro was **never** activated. It was moved up to the `@section homing` block. X and Y now hit the endstops softly at 100 mm/sÂ².
- `MANUAL.md` updated: E steps/mm=95 deduction was corrected (gearless MK8 feeder, direct drive â€” visually verified on hardware).

## [SD1-2.5] - 2026-07-23

**New Feature (Backport)**
- `ADAPTIVE_STEP_SMOOTHING` activated (`Configuration_adv.h:1760`).
  - **Reason:** Z and E0 drivers (HR4988SQ) do not have hardware microstep interpolation (like TMC's 256Ã—). At low and medium speeds, 16Ã— microsteps cause audible mechanical stepping/vibration (especially on Z, which drives two motors). This algorithm dynamically doubles the step rate to simulate 32Ã— at low speeds. Since Z speed is low (5 mm/s), the MCU overhead is negligible.
  - **Result:** Smoother Z-axis movement and less resonance on extruder at low speeds.

## [SD1-2.4] - 2026-07-23

**Hardware Alignment & Critical Fixes**
- **Driver assignments corrected (`Configuration.h`)**: `Z_DRIVER_TYPE` and `E0_DRIVER_TYPE` changed from `TMC2208_STANDALONE` to `A4988`.
  - **Context:** The board is mixed. X/Y are TMC2208, but Z/E0 are **HR4988SQ** (a drop-in replacement/clone of the A4988 family). There is no "HR4988" type in Marlin; `A4988` is the correct hardware definition.
  - **Impact:** Marlin uses this definition to determine the minimum pulse and delay times for the driver (in `stepper.h`).
- **CRITICAL FIX: `MINIMUM_STEPPER_DIR_DELAY`**: `MINIMUM_STEPPER_POST_DIR_DELAY` and `PRE_DIR_DELAY` increased from **30 ns** to **200 ns**.
  - **Context:** 30 ns was manually set in V1.1.10 because it was sufficient for TMC2208. But HR4988SQ requires at least 200 ns (A4988 default).
  - **Symptom Fixed:** When direction changed, the driver was executing the step **before** it registered the DIR pin state change, resulting in a single step in the *wrong direction*. This causes subtle layer shifts on Z and flow inconsistencies on E (retracts). The 200 ns delay guarantees the DIR pin is stable before the STEP pulse fires.
- `MANUAL.md` completely audited and updated (driver currents, definitions). The assumption that `HAS_TRINAMIC` features were active was documented as false.

## [SD1-2.3] - 2026-07-22

**Toolchain & Build Optimization**
- **Newlib-nano transition (`platformio.ini`)**: The `-specs=nano.specs` flag was added to the STM32F1 build environment.
  - **Context:** The libmaple framework was linking the standard newlib `libc`, which statically includes huge, heavy printf/scanf formatters. Since Marlin implements its own lightweight string formatting, 95% of this was dead weight in flash.
  - **Fix:** Switched to `newlib-nano` (optimized for embedded). The `-u_printf_float` flag was added to retain `%f` support for the few places that use `sprintf(..., "%f")` (M114, M600, power-loss recovery string).
  - **Result:** **âˆ’57,116 bytes** of flash footprint (35.1% â†’ 24.2%) and **âˆ’1,992 bytes** of RAM. Total build size dropped from ~184 KB to ~127 KB without touching a single line of logic.
- **Dead code cleanup (`Configuration_adv.h`)**: `SERMOON_Z_LOCK_AUTO` flag removed. The event hooks (`on_motion_start`/`end`) were defined but never called from anywhere in the codebase (the `on_motion_end` logic is completely absent in Marlin 2.0.x; it's a 2.1.x feature). The flag was misleading because it didn't do anything.

## [SD1-2.2] - 2026-07-21

**Codebase Cleanup**
- **111 orphaned files deleted.** A massive cleanup of source files that were completely excluded from the build by the preprocessor (due to feature toggles) or left over from upstream Marlin but incompatible with STM32F1.
- Highlights of deleted modules:
  - `src/feature/bedlevel/*` and `src/module/probe.cpp` (No probe on the printer)
  - `src/lcd/dogm/*` and `src/lcd/tft/*` (DWIN is used, these were dead weight)
  - `src/sd/usb_flashdrive/*` (USB host not supported by board)
  - `src/feature/leds/*` (No Neopixel/RGB)
  - `src/feature/spindle_laser.cpp` (It's a printer, not a CNC)
  - Extruder >1 files (`stepper_extruder.cpp`, etc. â€” single nozzle board)
- **Zero footprint impact:** These files were already compiled to 0 bytes or excluded by SCons; their deletion is purely for developer sanity and faster IDE search. Binary size is exactly identical to SD1-2.1.

## [SD1-2.1] - 2026-07-21

**Bug Fixes & Hardware Alignment**
- **Z-Probe removed (`Configuration.h`)**: `FIX_MOUNTED_PROBE` and all `AUTO_BED_LEVELING` features were disabled.
  - **Reason**: The hardware has no Z-probe. The previous ABL integration was conflicting with the Z lock module (`SERMOON_Z_LOCK`), which uses the PB0/PB1 pins on the BLTouch connector. The Z lock was silently failing to initialize because the probe pin re-configured PB1 as an input.
- **Power-Loss Recovery (PLR) EEPROM Fix (`persistent_store_api.h`)**:
  - `E2END` corrected from `0x800` to `0x7FF`.
  - **Reason**: The BL24C16 chip has addresses 0..2047 (`0x7FF`). The `0x800` value pushed the PLR save block outside the physical chip (1853..2048). The `valid_foot` byte was never written, causing `recovery.valid()` to always fail.
- **EEPROM I2C Protocol Fix (`i2c_eeprom.cpp`)**:
  - Sequential read was ignoring the 256-byte block boundaries of the 24C16 chip (it doesn't auto-increment the block address internally).
  - The read control byte was hardcoded to `0xA1` (block 0). Now it correctly calculates the block bits.
- **Thermistor Protection Regained (`Configuration.h`)**: `HEATER_0_MINTEMP` and `BED_MINTEMP` changed from 0 to 5. A broken thermistor reading 0Â°C now correctly triggers a halt.
- **Out-of-bounds Array Write Fix (`LCD_RTS.cpp`)**: Initialized variables `axis`, `min`, `max` to prevent memory corruption if data is malformed.
- **Baudrate alignment (`platformio.ini`)**: `monitor_speed` changed to 115200 to match `BAUDRATE`.
- Flash reduced by ~3 KB. Project code now has **0 warnings**.
## [SD1-2.0] - 2026-05-23

**Major Refactor & Performance Optimization**
- **Version string**: `MarlinV2 by CTK` (Build identifier changed)
- **DWIN Buffer Overflow Fix (`LCD_RTS.h`)**: `SizeofDatabuf` increased from 26 to 40 bytes. Fixed random display freezes caused by data truncation.
- **Homing Speed Optimization (`Configuration.h`)**: `HOMING_FEEDRATE_XY` lowered from 3000 to 1000 mm/m. Reduces mechanical shock during endstop triggering.
- **LIN_ADVANCE tuning**: K-factor default lowered from 0.22 to 0.06 (better starting point for Sermoon D1's direct drive setup).
- **EEPROM Subsystem Overhaul (`i2c_eeprom.cpp`)**:
  - `BL24CXX_Check()` now runs only once per boot.
  - Implemented 16-byte page writes for `BL24CXX_Write()`, improving write speed ~15x.
  - Sequential reads implemented, ~2x speedup.
  - PLR write time dropped from ~600ms to ~40ms, drastically reducing the chance of data loss during power failure.
- **Display Remaining Time Fix (`LCD_RTS.cpp`)**:
  - Standard Marlin calculates remaining time via `SHOW_REMAINING_TIME`, which requires a graphical LCD.
  - Implemented custom calculator for DWIN: `elapsed_sec * (100 - pct) / pct / 60`.
  - Added new Virtual Address (VP) `PRINT_REMAIN_MIN_VP = 0x1410`.
- **Serial & Buffer Tuning (`Configuration_adv.h`)**:
  - `RX_BUFFER_SIZE` reduced from 1024 to 512 (sufficient for 115200 baud).
  - `BLOCK_BUFFER_SIZE` increased from 16 to 32 (better flow continuity on complex curves).
  - `DEFAULT_MINSEGMENTTIME` lowered from 20000 to 8000 Âµs (better retracts).
  - `MM_PER_ARC_SEGMENT` changed from 1 to 2 (reduces ARC block generation overhead).
- **Dead feature removal**: `CANCEL_OBJECTS` and `GCODE_MACROS` disabled by default to save flash and RAM, as they are rarely used.

## [SD1-1.4] - (Legacy Baseline)

**Historical Changes Prior to Git Tracking**
- `Y_STEPS_PER_UNIT` changed from 80.00 to 79.60 based on calibration cube measurements.
- Z Lock (`SERMOON_Z_LOCK`) implemented to control PB0/PB1 enclosure lock mechanisms via M888.
- Added 15 new thermistor tables for compatibility.
- Maximum X/Y speed reduced from 300 to 250 mm/s to prevent chassis resonance.
- PID autotuning enabled by default.

---
*Note: For detailed hardware setup and instructions, refer to `MANUAL.md`.*
