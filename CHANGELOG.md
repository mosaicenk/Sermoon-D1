# Sermoon D1 Firmware â€” Changelog

This document logs all changes made on top of the base version (`stock Creality V1.1.10`, Marlin 2.0.x bugfix branch).

## [SD1-3.2] - 2026-08-05

**The Z axis is returned to stock Marlin behaviour.** Every fork-specific Z override
accumulated in SD1-1.2/1.3 is deleted, not re-tuned. After the bed was re-levelled by hand
(0.07 mm paper, four corners) and **verified flat**, a test cube printed cleanly — but only
while 1 mm of negative Z-offset was dialled into the DWIN screen after *every* homing. The
overrides were the sole reason that was necessary.

**Fixes**
- **Z=0 sat 1 mm too high (`Configuration.h`)**: `MANUAL_Z_HOME_POS` was `-1`, an SD1-1.3 workaround from when the bed adjustment screws had run out of travel — it lifted the Z zero by 1 mm, so a slicer first layer at `Z=0.2` physically ran **1.2 mm** above the plate (the original comment said so explicitly). Once the bed was levelled properly that bias became pure error.
  - `MANUAL_Z_HOME_POS` is now **undefined**, as upstream ships it. `Z_HOME_POS` falls through to `(Z_HOME_DIR < 0 ? Z_MIN_POS : Z_MAX_POS)` (`Conditionals_post.h:181`) `= Z_MIN_POS = 0`, so the endstop trigger point **is** `Z=0`.
  - The nozzle-to-bed gap is `gap_at_trigger + (Z_commanded - Z_HOME_POS)`, so dropping the `-1` closes the gap by exactly 1 mm at every commanded Z — identical to the manual −1 mm offset, now applied automatically on **every `G28`**, surviving `M502`, with no screen entry.
  - Path verified end to end: `Conditionals_post.h:178/181` → `base_home_pos()` (`motion.h:123`) → `set_axis_is_at_home()` (`motion.cpp:1375`).
  - `Z_MIN_POS` **-1 → 0**, also the stock value, and now doing double duty: with `MANUAL_Z_HOME_POS` gone it *is* the homing coordinate as well as the soft floor. Floor and bed coincide, so neither jogging nor the DWIN move menu (`LCD_RTS.cpp:1605` clamps to `Z_MIN_POS`) can drive the nozzle into the plate. `MIN_SOFTWARE_ENDSTOP_Z` is enabled, so the clamp is live.
  - Live micro-adjustment is unaffected: the DWIN Z-offset applies via babystepping (`LCD_RTS.cpp:1182`), which bypasses soft endstops.
- **`Z_AFTER_HOMING` removed (`Configuration_adv.h`)** — undefined, as upstream ships it. The `#if defined(Z_AFTER_HOMING)` block in `G28.cpp:469-470` drops out and the position left by `homeaxis()` now stands.
  - **That position is `Z=2`, not `Z=0`.** `homeaxis()` calls `set_axis_is_at_home()` (`Z=0`, the trigger point) and *then* applies `HOMING_BACKOFF_MM { 1, 1, 2 }`, retracting Z 2 mm off the endstop: `current_position[Z] -= ABS(2) * axis_home_dir` with `axis_home_dir = -1` (`motion.cpp:1672-1696`). `G28` therefore ends with the bed 2 mm below the nozzle and the DWIN screen reading **`2.00`**.
  - This was previously invisible: `Z_AFTER_HOMING 0` ran *after* the backoff and pulled the bed back up to the zero coordinate, which is why the screen used to read `0.00`. The backoff itself is unchanged and stays — it is the SD1-2.9 endstop-release behaviour that keeps `M119` unambiguous, and 2 mm of clearance after homing is the safe resting state.
  - The zero reference is unaffected either way: `Z=0` is the trigger point and a slicer first layer at `Z=0.2` is 0.2 mm off the plate.
  - SD1-1.2 had defined it as `0`, justified as "a known safe altitude that clarifies the reference for 2 parallel Z motors". That rationale does not hold: park height has no bearing on how two motors sharing one driver align, and under the SD1-1.3 zero the value `0` was not a safe altitude either — it was the 1 mm offset in disguise.
  - An interim SD1-3.2 draft set this to `1` to keep a parking gap, on the assumption that the bed centre was crowned. The user has since measured the bed flat, removing the only thing a park height protected against, so stock won.
  - `Z_HOMING_HEIGHT` (4 mm, `Configuration.h:1117`) was checked and left alone: it still lifts 4 mm before X/Y homing, now measured from the true bed surface.

**Mechanics note**: this printer moves the **bed** in Z (two lead screws, two motors in parallel on one driver); the hotend is fixed in Z. Homing raises the bed into the endstop. None of the arithmetic cares — Marlin's Z coordinate is the nozzle-to-bed distance, not a toolhead height — but the comments now say so, since "the nozzle moves down" is never literally what happens here.

**Why the offset never stuck before**: `zprobe_zoffset` is a plain global in `LCD_RTS.cpp:39`, held only there and in power-loss recovery — it is **not** part of the EEPROM settings, so it resets to 0 on every power-up. No amount of `M500` would have saved it; the value had to live in the firmware.

> **After flashing, stop entering −1 on the screen.** The screen offset would stack on top of the new zero and push the nozzle 1 mm into the bed. It should read **0.00**. Also check `M503` for a stored `M206 Z` home offset — if one is set it stacks the same way; clear it with `M206 Z0` + `M500`.

**Measured** (clean build, 2026-08-05): Flash 126,864 bytes (**−24** vs SD1-3.1), RAM 13,176 bytes (unchanged), 0 project warnings. The saving is real code, not constants: dropping `Z_AFTER_HOMING` removes the `do_blocking_move_to_z()` call from `G28` entirely. `firmware.bin` SHA256 `85F40505…81D6`; `__DATE__` is embedded, so the hash only reproduces on a same-day build.

Version string confirmed inside the image after the clean build:
```
$ grep -a -o "SD1-[0-9.]*" .pio/build/creality/firmware.bin
SD1-3.2
```

**Re-flashing required** — this is the whole point of the release.

## [SD1-3.1] - 2026-08-04

Follow-up to a full audit of the motion path (planner, stepper ISR, homing, arc/Bezier
generation). No motion algorithm was changed; these are mostly consistency fixes where the
configuration or comments claimed something the code does not do. The one behavioural change
is the idle stepper release time, listed below.

**Fixes**
- **Version string was stuck at `SD1-2.8` (`Marlin/Version.h`)**: `SHORT_BUILD_VERSION` was never bumped after 2026-07-26, so the binaries released as SD1-2.9, SD1-3.0 and SD1-3.1 all reported **`SD1-2.8`** in `M115` and on the DWIN "About" screen. Verified directly in the compiled image: the 126,888-byte `firmware.bin` contained the string `SD1-2.8 (Sermoon D1 by CTK, base V1.1.10)`. Bumped to `SD1-3.1`, and `STRING_DISTRIBUTION_DATE` from the equally stale `2026-07-27` to `2026-08-04`. This was a plain missed edit, not the incremental-build trap documented in the README — but the two look identical from the outside, so **always confirm the version inside the binary after a bump**: `grep -a -o "SD1-[0-9.]*" .pio/build/creality/firmware.bin`.
- **Idle stepper release shortened, 300 s → 60 s (`Configuration_adv.h`)**: the 300 s value was justified in-comment by "manual pauses were being interrupted". That justification was wrong. `manage_inactivity()` resets the stepper timeout on every pass while the print is paused (`Marlin.cpp:475-478`, `printingIsPaused()` = `did_pause_print || print_job_timer.isPaused() || IS_SD_PAUSED()`), and `PAUSE_PARK_NO_STEPPER_TIMEOUT` excludes `M600` a second time via `MOVE_AWAY_TEST`. Steppers **cannot** time out during a filament change or a paused print at any value of this setting, so nothing was ever protecting pauses.
  - The timeout therefore only governs a genuinely idle machine — not printing, not paused. Releasing the motors after 60 s instead of 300 s cuts four minutes of pointless idle heat inside the closed cabinet and standing thermal load on the TMC2208 standalone drivers. Z is held mechanically by the Z lock module, so `DISABLE_INACTIVE_Z true` stays safe.
  - **User-visible effect**: after a print ends, the motors go slack about four minutes sooner; the head can be pushed by hand at that point. Homing is required before the next print either way. Override at runtime with `M84 S<seconds>` if you want the old behaviour without rebuilding.
- **Retract feedrates stated values that could never take effect (`Configuration_adv.h`)**: `DEFAULT_MAX_FEEDRATE` caps the E axis at 25 mm/s, and the planner scales every block down to that ceiling (`planner.cpp:2032-2038`). `RETRACT_FEEDRATE 45` and `PAUSE_PARK_RETRACT_FEEDRATE 60` were therefore executed as 25 mm/s, silently and without warning. Both set to **25** so the configuration matches the motion actually produced. **Physical behaviour is unchanged** — retracts already ran at 25 mm/s. To retract faster, raise the E entry of `DEFAULT_MAX_FEEDRATE` first; the retract values alone do nothing.
  - Note: `M207`/`M208` values already stored in `eeprom.dat` are unaffected (they were being clamped to the same 25 mm/s). The new defaults apply after `M502` + `M500`.
- **`DEFAULT_EJERK` comment was wrong (`Configuration.h`)**: it claimed the value is "used by Linear Advance even with JUNCTION_DEVIATION". `HAS_CLASSIC_E_JERK = (CLASSIC_JERK || !LIN_ADVANCE)` (`Conditionals_post.h:47`) is false in this build, so LA derives its E jerk from `JUNCTION_DEVIATION_MM` instead (`planner.h:877-885`, ~13.5 mm/s at JD 0.015 with E accel 5000). Comment corrected; the value is untouched and remains inert outside EEPROM compatibility.
- **Dead pulse-timing loop removed (`src/module/stepper.cpp`)**: a stock-Creality `for(char i = 0; i < 2; i++);  // ns delay` sat between the Bresenham pulse start and pulse stop. Empty body, non-volatile counter — it produces no delay and the optimizer discards it. Removed; pulse width is guaranteed by the `MIN_PULSE_TICKS` busy-wait. **Verified: firmware.bin is byte-identical before and after removal** (SHA256 `8E9D0584…7627`), proving the compiler had already eliminated it.

**Documentation**
- `MM_PER_ARC_SEGMENT 2` rationale was stale: it cited `BLOCK_BUFFER_SIZE 16`, which has since become 32, so the buffer-starvation argument for 1 → 2 mm no longer holds. The comment now records this; **the value was not changed** — reverting to 1 mm doubles arc resolution (chord error at R=20 mm: 0.025 → 0.006 mm) but needs verification on an arc-heavy print first.
- The leveling block in `Configuration.h` is now marked as inert. Its `#define`s (`ENABLE_LEVELING_FADE_HEIGHT`, `SEGMENT_LEVELED_MOVES`, …) sit inside a guard that is false on this printer (no probe, no leveling mode), so they never reach the build despite appearing uncommented in a grep.

**Measured** (clean build, 2026-08-04): Flash 126,888 bytes (−8 vs SD1-3.0), RAM 13,176 bytes (unchanged), 0 project warnings. The version bump costs nothing — `SD1-2.8` and `SD1-3.1` are the same length. `firmware.bin` SHA256 `F3361492…6A00`; `__DATE__` is embedded, so this hash only reproduces on a build made the same day.

Version string verified inside the image after the clean build:
```
$ grep -a -o "SD1-[0-9.]*" .pio/build/creality/firmware.bin
SD1-3.1
```

**Re-flashing**: recommended. Without it the printer keeps reporting `SD1-2.8`, and the corrected retract defaults only land on a fresh `M502` + `M500`. The 60 s idle stepper release takes effect immediately on flashing (it is a compile-time default, not an EEPROM value) — `M84 S300` restores the old timing for the current session if you prefer it.

## [SD1-3.0] - 2026-08-03

**Fixes**
- **Filament runout sensor polarity (`src/lcd/dwin/LCD_RTS.cpp`)**: The DWIN/RTS screen had its own filament check, independent of Marlin's `feature/runout.cpp`, reading the same pin (PA4). The two disagreed on polarity: `Configuration.h` sets `FIL_RUNOUT_INVERTING false` (HIGH = filament present), while the RTS code treated HIGH as "no filament".
  - Symptom: with filament loaded, starting a print raised the "is filament loaded?" dialog, and pressing "Yes" did nothing. The Yes handler (`NoFilamentContinue`) re-runs `RTS_CheckFilement()` as its first statement and bails out via `break` when it reports empty, so `M23`/`M24` were never queued — the dialog could not be dismissed.
  - Both RTS pin reads inverted to `0 == READ(CHECKFILEMENT_PIN)` (LOW = no filament): the polling loop in `RTS_CheckFilement()` and the in-print check in `RTSUpdate()`. These are the only two reads of that pin.
  - Both detection paths now agree with the hardware. Flash 126,896 bytes (+8) -> **needs re-flashing.**

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
