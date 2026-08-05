# MarlinV2 by CTK â€” Sermoon D1 Firmware Manual

**Firmware:** MarlinV2 by CTK (Marlin 2.0.x bugfix branch)  
**Printer:** Creality Sermoon D1  
**Board:** Creality V4.3.1 (STM32F103RET6, 72 MHz, 512KB Flash, 64KB RAM)  
**Display:** DWIN T5L (LCD_RTS protocol)  
**Bed:** 290 Ã— 270 mm, Aluminum plate, 320 mm Z travel  
**Drivers:** Mixed â€” X/Y TMC2208 standalone, Z/E0 HR4988SQ  
**Date:** 2026-07-23 (SD1-2.4)  

> **2026-07-23 audit.** The driver/current sections of this guide have been rewritten with measured hardware data. Specifically **Â§8 has completely changed**: the old version described the settings in the `#if HAS_TRINAMIC` block (StealthChop, HYBRID_THRESHOLD, INTERPOLATE) as if they were active â€” this block **is not compiled at all** on this board. No settings made based on the old Â§8 have changed behavior.

---

## Table of Contents

1. [Overview](#1-overview)
2. [Hardware Information](#2-hardware-information)
3. [Firmware Build](#3-firmware-build)
4. [Firmware Flashing](#4-firmware-flashing)
5. [Step and Movement Settings](#5-step-and-movement-settings)
6. [Homing Configuration](#6-homing-configuration)
7. [Movement Algorithms](#7-movement-algorithms)
8. [Driver Settings (mixed: TMC2208 + HR4988SQ)](#8-driver-settings-mixed-tmc2208--hr4988sq)
9. [Linear Advance (LIN_ADVANCE)](#9-linear-advance-lin_advance)
10. [Z-Probe and Auto Bed Leveling (ABL)](#10-z-probe-and-auto-bed-leveling-abl)
11. [Temperature and Safety](#11-temperature-and-safety)
12. [EEPROM and SD Card](#12-eeprom-and-sd-card)
13. [DWIN Display Integration](#13-dwin-display-integration)
14. [Pin Map](#14-pin-map)
15. [Calibration Guide](#15-calibration-guide)
16. [G-Code Reference](#16-g-code-reference)
17. [Troubleshooting](#17-troubleshooting)
18. [Changelog](#18-changelog)

---

## 1. Overview

This firmware is a custom fork compiled on the Marlin 2.0.x bugfix branch over Creality's original Sermoon D1 V1.1.10 software. All changes made compared to the original:

| Feature | Original | Current | Description |
|---------|----------|--------|----------|
| Version string | Creality V1.1.10 | MarlinV2 by CTK | Identifier |
| DWIN databuf | 26 bytes | 40 bytes | Overflow fix |
| HOMING_FEEDRATE_XY | 3000 mm/m | 1000 mm/m | Smoother homing |
| LIN_ADVANCE_K | 0.22 | 0.06 | Direct drive â€” **calibration mandatory**, E0 driver changed in SD1-2.4 |
| ~~HYBRID_THRESHOLD~~ | Off | **Never active** | Inside `#if HAS_TRINAMIC` â†’ not compiled on this board (Â§8) |
| Z/E0 driver type | Assumed TMC2208 | **HR4988SQ** (`A4988`) | SD1-2.4 â€” aligned with hardware |
| `MINIMUM_STEPPER_*_DIR_DELAY` | 30 ns | **200 ns** | HR4988SQ requirement; risk of reverse step at 30 ns |
| Z_SAFE_HOMING | Off | On | Z homing mandatory after X/Y. Point: (145, 135) â†’ SD1-2.7 (âˆ’8, âˆ’8) â†’ SD1-2.8 **(âˆ’10, âˆ’10)** |
| HEATER_0_MINTEMP / BED_MINTEMP | 0 | 5 | Broken thermistor protection regained |
| E2END | 0x800 (wrong) | 0x7FF | 24C16 last valid address â€” fixed PLR overflow |
| SERMOON_Z_LOCK | (out of code) | On, PB0+PB1 | Z lock module controllable with M888 |

**Z-probe / ABL**: **Off** as in stock. The printer has no probe hardware;
PB0/PB1 are dedicated to the Z lock module. For details see [section 10](#10-z-probe-and-auto-bed-leveling-abl).

---

## 2. Hardware Information

### 2.1 Board: Creality V4.3.1

| Feature | Value |
|---------|-------|
| MCU | STM32F103RET6 |
| Clock speed | 72 MHz |
| Flash | 512 KB |
| RAM | 64 KB |
| Supply | 24V DC |
| Logic level | 3.3V (MCU side) |
| Bootloader | 0x08000000 (SD card loading) |
| Firmware load address | 0x08007000 (28KB offset) |

### 2.2 Stepper Motor Drivers

The board carries **mixed** drivers. All are soldered to the board and each has its own
Vref trim pot.

| Axis | Driver | Marlin type | Measured Vref | Actual current | % of 42-40 nominal |
|-------|--------|-------------|--------------|-------------|-------------------|
| X | TMC2208 | `TMC2208_STANDALONE` | 1.27 V | 0.69 A RMS | 69% |
| Y | TMC2208 | `TMC2208_STANDALONE` | 1.27 V | 0.69 A RMS | 69% |
| Z | **HR4988SQ** | `A4988` | 1.60 V | **0.47 A RMS/motor** | 47% |
| E0 | **HR4988SQ** | `A4988` | 0.86 V | 0.51 A RMS | 51% |

- **Motors:** Creality **42-40** on all four axes (~1.0 A/phase, ~0.40 NÂ·m,
  ~2.8 Î©, ~5.5 mH)
- **R_SENSE: 0.15 Î© (`R150`)** â€” on all drivers, 2 per winding.
  *(The `0.11 Î©` in the older version of this guide was wrong; copied from a generic TMC2208 module value.)*
- **Microstepping: 16Ã—**, drivers soldered to the board (no module with separate jumpers â€”
  verified via board photo), MS1/MS2 hard-wired on the PCB. Not connected to MCU
  â†’ `M350` does not work.
- **256Ã— interpolation only on X/Y** â€” TMC2208's own hardware
  feature. HR4988SQ does not interpolate, 16Ã— on Z/E0 is truly 16Ã—.

> **There are TWO motors connected in parallel to a single driver on Z.** The current provided by the driver is divided by two; the 0.47 A value in the table is per motor (driver total 0.94 A RMS / 1.33 A peak, 67% of HR4988SQ ceiling).

> **The enable input for all four drivers is PC3.** Marlin does not count this pin on a per-axis basis â€” it is impossible to disable a single axis independently. HR4988SQ also has no standstill current reduction (TMC2208 does), meaning the Z driver waits at full current between layers. A heatsink is mandatory.

### Vref calculation â€” two formulas do not measure the same thing

| Driver | What Vref sets | Formula (Rs = 0.15 Î©) |
|---|---|---|
| TMC2208 | **RMS** | `I_RMS = Vref Ã— 0.541` |
| HR4988SQ | **PEAK** | `I_peak = Vref / 1.2` , `I_RMS = I_peak / âˆš2` |

Copying the Vref of the TMC2208 to the HR4988SQ means a **âˆš2 factor** error.

> **Factory setting verified â€” do not touch the pots.** All three values are in the safe zone for both the motor (47â€“69%) and the driver (36â€“67%). The only low margin is E0 (51%); if extruder clicking is observed, **first LIN_ADVANCE K must be calibrated** (Â§9), current should only be raised to 1.05 V if the issue persists.

### 2.3 Extruder

- **Type:** **Direct drive** (gearless MK8 type; drive gear is on the hotend) â€”
  user hardware verification, 2026-07-24.
  *(History: the first version of the document said "direct drive"; the SD1-2.4 audit changed this to "Bowden" with the deduction "E steps/mm = 95 â†’ MK8 â†’ Bowden".
  The deduction was wrong: 95 steps/mm is the value for a gearless MK8 type **feeder** and does not say where the feeder is located â€” it is also 95 in a gearless direct drive; however, a geared direct drive would be ~400â€“450. The correct type was confirmed by looking at the hardware.)*
- **Filament:** 1.75 mm
- **Nozzle:** V-hotend (Creality custom)
- **E steps/mm:** 95

---

## 3. Firmware Build

### 3.1 Requirements

- PlatformIO CLI (`pio` command must be accessible)
- Python 3.x
- Git

### 3.2 Build Command

```bash
cd C:\Users\CNK\Desktop\Sermoon-D1
pio run
```

### 3.3 Build Result (SD1-2.4, measured 2026-07-24)

```
RAM:   [==        ]  20.1% (13,176 / 65,536 bytes)
Flash: [==        ]  24.2% (127,120 / 524,288 bytes)
SUCCESS â€” ~11 seconds
```

### 3.4 Binary Location

```
.pio\build\creality\firmware.bin
```

### 3.5 Known Warnings (Not Errors)

There are **no warnings** in the project code (SD1-2.1). Only a framework-based warning remains, which is not our code and cannot be fixed:

- `util_adc.c:10` â€” `'adc_result' initialized and declared 'extern'`
  (maple framework, in `framework-arduinoststm32-maple` package)

Additionally, `Warnings.cpp` prints an informational `#pragma message` (not an error):
LIN_ADVANCE active, K calibration reminder.

> Prior to SD1-2.1, there were 3 "uninitialized variable" warnings in `LCD_RTS.cpp`. These were thought to be harmless but when `axis` is uninitialized, `current_position[axis]` meant an **out-of-bounds array write**; this was fixed by initializing the variables with safe defaults.

---

## 4. Firmware Flashing

### 4.1 Flashing via SD Card

1. Copy the `firmware.bin` file to the root directory of a FAT32 formatted SD card
2. Insert the SD card while the printer is off
3. Turn on the printer â€” the bootloader automatically flashes the firmware
4. Flashing takes ~10-20 seconds
5. When the screen turns on, check the "MarlinV2 by CTK" version

### 4.2 Important Notes

- There must be no other `.bin` file on the SD card
- The bootloader writes to address 0x08007000 â€” creality.py and creality.ld provide this offset
- EEPROM settings might be reset after flashing â†’ restore factory settings with `M502` + `M500`

---

## 5. Step and Movement Settings

### 5.1 Steps Per Unit

```cpp
#define DEFAULT_AXIS_STEPS_PER_UNIT { 80, 79.60, 400, 95 }
```

| Axis | Steps/mm | Description |
|-------|----------|----------|
| X | 80.00 | GT2 belt, 20 tooth pulley, 200 step rev, Ã—16 microstep |
| Y | 79.60 | **Different** â€” probably factory calibration |
| Z | 400.00 | T8 Ã— 8mm lead screw |
| E | 95.00 | **Direct drive** extruder (gearless MK8 type feeder) |

> âš ï¸ **Y steps = 79.60** â€” Theoretically it should be 80 with the same mechanical structure as X. Verify with a dimensional test print. Adjust with M92 if there is a deviation.

### 5.2 Maximum Speeds

```cpp
#define DEFAULT_MAX_FEEDRATE { 250, 250, 5, 25 }  // mm/s
```

| Axis | Max speed | Note |
|-------|---------|-----|
| X | 250 mm/s | 300â†’250: chassis resonance at high speed (not a StealthChop limit) |
| Y | 250 mm/s | Same reason |
| Z | 5 mm/s | Lead screw, requires high torque |
| E | 25 mm/s | Extruder filament feeding |

### 5.3 Acceleration

```cpp
#define DEFAULT_MAX_ACCELERATION      { 800, 800, 100, 5000 }  // mm/sÂ²
#define DEFAULT_ACCELERATION          500    // mm/sÂ² (print)
#define DEFAULT_RETRACT_ACCELERATION  5000   // mm/sÂ² (retract)
#define DEFAULT_TRAVEL_ACCELERATION   800    // mm/sÂ² (travel)
```

| Parameter | Value | Description |
|-----------|-------|----------|
| X/Y max accel | 800 mm/sÂ² | Safe value without ringing |
| Z max accel | 100 mm/sÂ² | Lead screw |
| E max accel | 5000 mm/sÂ² | Fast retract |
| Print accel | 500 mm/sÂ² | Normal print movements |
| Travel accel | 800 mm/sÂ² | Non-print movement |

---
## 6. Homing Configuration

### 6.1 Homing Parameters

```cpp
#define HOMING_FEEDRATE_XY    1000     // mm/m (X/Y homing speed)
#define HOMING_FEEDRATE_Z     (4*60)   // mm/m = 240 mm/m (Z homing speed)
#define X_HOME_DIR            -1       // Left (towards MIN)
#define Y_HOME_DIR            -1       // Back (towards MIN)
#define Z_HOME_DIR            -1       // Down (towards MIN)
#define X_HOME_BUMP_MM        5        // Second touch distance
#define Y_HOME_BUMP_MM        5
#define Z_HOME_BUMP_MM        2
#define HOMING_BUMP_DIVISOR   { 4, 4, 4 }  // SD1-2.9: Second pass speed divisor (X/Y/Z)
#define HOMING_BACKOFF_MM     { 1, 1, 2 }  // SD1-2.9: X/Y retracts 1mm
//#define QUICK_HOME                     // SD1-2.7: OFF â€” X and Y sequentially
#define IMPROVE_HOMING_RELIABILITY       // X/Y accel â†’ 100 mm/sÂ² during homing
```

> **`IMPROVE_HOMING_RELIABILITY` was ineffective until SD1-2.6.** The definition was inside the `#if HAS_TRINAMIC` block in the TMC section of `Configuration_adv.h`; since `HAS_TRINAMIC` is false on this board, the macro was never defined and homing ran at full acceleration (800 mm/sÂ²). In SD1-2.6 it was moved under `@section homing`. Since `CLASSIC_JERK` is disabled, only the acceleration branch is compiled; jerk is untouched.

### 6.2 Homing Flow

The following values are derived from the measured configuration
(`max_length` X = 290âˆ’(âˆ’10) = 300 mm, Y = 270âˆ’(âˆ’10) = 280 mm;
`HOMING_FEEDRATE_XY` 1000 mm/min = 16.67 mm/s).

```
0. X/Y accel temporarily reduced to 100 mm/sÂ²  (IMPROVE_HOMING_RELIABILITY)
1. Z is raised Z_HOMING_HEIGHT = 4 mm          (BEFORE X/Y, clearance)
2. X axis (HOME_Y_BEFORE_X off â†’ X first):
      fast âˆ’450 mm @ 16.67 mm/s    (1.5 Ã— max_length Ã— dir)
      bump back  +5 mm               (X_HOME_BUMP_MM)
      slow âˆ’10 mm @ 8.33 mm/s        (divisor 2) â† true zero is here
      X = X_MIN_POS = âˆ’10, backoff 0 â†’ NO retraction â†’ X = âˆ’10
3. Y axis: same pattern, fast move âˆ’420 mm â†’ Y = âˆ’10
4. Z: go to (âˆ’10, âˆ’10) â€” X/Y IS ALREADY THERE, zero-length move
      touch at 240 mm/min, 2 mm bump, divisor 4 â†’ 1 mm/s
      then rise to Z_AFTER_HOMING = 0
5. Accel is restored to original value
```

> **Changed in SD1-2.7.** In previous versions, `QUICK_HOME` was at step 2: X and Y were driven simultaneously to both endstops in a single diagonal move (target (âˆ’450, âˆ’420), speed 22.8 mm/s). Now off â€” the two axes home sequentially. Step 4 also used to go to the bed center (145, 135).

### 6.2b Where does the nozzle stay after homing?

**Starting from SD1-2.8, it is the same in all cases: (âˆ’10, âˆ’10)** â€” which is the endstop trigger point itself.

| Command | Final X | Final Y |
|---|---|---|
| `G28` (full) | **âˆ’10** | **âˆ’10** |
| `G28 X Y` | **âˆ’10** | **âˆ’10** |
| `G28 X` | **âˆ’10** | unchanged |
| `G28 Y` | unchanged | **âˆ’10** |

`-10` comes from here: at the end of `homeaxis()`, the axis is considered to be at `X_MIN_POS`/`Y_MIN_POS` = **âˆ’10**. Since the X/Y inputs of `HOMING_BACKOFF_MM` are **0** in SD1-2.8, the `if (backoff_mm)` condition remains false and the retraction move is never generated
(`motion.cpp:1686`) â€” the axis stops at the trigger point. Being off the bed is by design (in SD1-2.7 this was âˆ’8, backoff was 2 mm).

Full `G28` also now ends at the same point because `Z_SAFE_HOMING_X/Y_POINT` is defined as `(X_MIN_POS + 2)` / `(Y_MIN_POS + 2)` â€” which is where it already is. The `do_blocking_move_to_xy()` call inside `home_z_safely()` generates a zero-length move.

> **In previous versions, a full `G28` ended at (145, 135).** If your slicer start G-code or macros assume the nozzle is at the bed center after G28, review them.

> **Do not confuse:** `NOZZLE_PARK_POINT` = `{ X_MIN_POS+10, Y_MAX_POSâˆ’10, 20 }` = **(0, 260, 20)** is unrelated to homing. It is the pause/filament change park point for `NOZZLE_PARK_FEATURE` (M125, M600), G28 does not use it.

### 6.3 Z Home Offset

```cpp
//#define MANUAL_Z_HOME_POS 0         // undefined (stock) → Z_HOME_POS = Z_MIN_POS = 0
#define Z_MIN_POS           0         // soft floor AND homing coordinate = bed surface
//#define Z_AFTER_HOMING 1            // undefined (stock) → no move after homing Z
```

> **SD1-3.2 returns the Z axis to stock Marlin behaviour.** Up to SD1-3.1 the reference was shifted 1 mm up (`MANUAL_Z_HOME_POS -1`), a leftover from SD1-1.3 when the bed screws had run out of travel: the mechanical trigger read `Z=-1` and a slicer first layer at `Z=0.2` physically ran **1.2 mm** above the plate. That had to be cancelled by hand with 1 mm of negative Z-offset on the screen after every homing.
>
> With both overrides deleted, the endstop trigger point **is** `Z=0`. A slicer first layer at `Z=0.2` really is 0.2 mm off the plate, automatically, on every `G28`, surviving `M502`.
>
> **After `G28` the screen reads `2.00`, not `0.00`.** With `Z_AFTER_HOMING` gone, the position left by `homeaxis()` is what stands, and that is not the trigger point: `set_axis_is_at_home()` sets `Z=0`, then `HOMING_BACKOFF_MM { 1, 1, 2 }` retracts the Z axis 2 mm off the endstop (`motion.cpp:1672-1696`). So homing ends with the bed 2 mm below the nozzle.
>
> Previous versions hid this — `Z_AFTER_HOMING 0` ran *after* the backoff and pulled the bed back up, which is why the screen used to read `0.00`. Nothing about the zero reference changes: `Z=0` is still the trigger point, and a slicer first layer at `Z=0.2` is still 0.2 mm off the plate. Only the resting position after `G28` is different, and the 2 mm gap is the safer one — it also leaves the Z endstop released, so `M119` reads honestly.
>
> **Do not dial −1 into the DWIN Z-offset any more.** It stacks on top of the new zero and puts the nozzle 1 mm into the plate. The screen should read `0.00`.
>
> **Mechanics:** on the Sermoon D1 the **bed** is what moves in Z (two lead screws driven by two motors wired in parallel on a single driver); the hotend is fixed in Z. Homing raises the bed until it trips the endstop, and `Z_AFTER_HOMING` then drops it 1 mm. So wherever this manual says the nozzle "goes down", what physically happens is the bed coming up. Marlin's Z coordinate is the nozzle-to-bed distance, so every number above is correct either way.

### 6.4 Z Safe Homing

```cpp
#define Z_SAFE_HOMING
#define Z_SAFE_HOMING_X_POINT  X_MIN_POS           // âˆ’10 mm  (SD1-2.8)
#define Z_SAFE_HOMING_Y_POINT  Y_MIN_POS           // âˆ’10 mm  (SD1-2.8)
```

- Z homing can only be done after X/Y homing
- Z home point = where X/Y homing leaves it â†’ **no extra movement**

> **Macro must remain ON, the point changed.** In SD1-2.7, the desired "don't go to bed center" behavior was achieved not by turning off `Z_SAFE_HOMING`, but by **setting the point to (âˆ’10, âˆ’10)**. Reason: this macro also carries the protection *"Z cannot be homed without homing X and Y"* (`G28.cpp:128`, `axis_known_position` check). The DWIN screen can send a standalone `G28 Z0` in `LCD_RTS.cpp:1459`; if the protection is lifted, that command will home Z at whatever **random** X/Y position the head is at. Moving the point gives the desired result, while leaving the protection in place.
>
> âš ï¸ **Verify your first layer calibration.** Z is now homed at (âˆ’10, âˆ’10) instead of (145, 135). Because the Z endstop is a mechanical switch (PA7) fixed to the frame, its trigger height is *expected* to be independent of X/Y â€” meaning the Z=0 reference should not change. **This cannot be verified from firmware**, it depends on where the switch is mounted. Confirm with a paper test after flashing.
>
> âš ï¸ **(âˆ’10, âˆ’10) is off the bed.** Manually check that there is no bed clip, cable, or chassis part where the Z descent could get caught in that corner.

### 6.5 Impact of park position on diagnostics (SD1-2.8)

Because the X/Y entries of `HOMING_BACKOFF_MM` are **0**, the carriage parks with the endstop **pressed** after homing. Safe for movement â€” verified:

- `ENDSTOPS_ALWAYS_ON_DEFAULT` is **off** â†’ endstops are only monitored during homing; them being pressed while idle does not affect normal movements.
- The X_MIN/Y_MIN check in `endstops.cpp:711` is only for the **âˆ’direction** branch; a `+` movement away from the endstop won't trigger it anyway.
- `MIN_SOFTWARE_ENDSTOPS` is on â†’ cannot go below âˆ’10.

**But it comes with two costs:**

| Effect | Detail |
|---|---|
| Mechanical fatigue | Switch lever/spring is under constant stress while idle |
| **Diagnostic ambiguity** | `M119` always returns `x_min: TRIGGERED` at the rest position |

The second one is important: endstops are connected **normally-closed (NC)** (`ENDSTOPPULLUPS` + `*_ENDSTOP_INVERTING false` â†’ trigger = pin HIGH = switch open). **A broken endstop cable also shows TRIGGERED.** Since the park position is also TRIGGERED, "resting at home" and "broken cable" are no longer distinguishable at a glance.

**Correct diagnostic procedure:** first move the axis away from the endstop, then read.

```gcode
G28 X            ; Home X
G91              ; Relative mode
G1 X20 F1000     ; Move 20 mm away from endstop
G90
M119             ; Read NOW â†’ x_min: open EXPECTED
```

If `open` is not returned, the endstop circuit is faulty (broken cable, stuck lever). In this case, homing silently accepts the wrong origin, and the print comes out shifted.

---

## 7. Movement Algorithms

### 7.1 Algorithm Summary

| Algorithm | Status | Quality Impact | Description |
|-----------|-------|---------------|----------|
| S-Curve Acceleration (BÃ©zier) | âœ… Active | 9/10 | Curvy transition instead of flat acceleration |
| Junction Deviation | âœ… Active | 9/10 | Adaptive corner speed (instead of Classic Jerk) |
| Linear Advance (K=0.06) | âš ï¸ Active, **K uncalibrated** | 8/10 | Direct drive â€” K must be calibrated (Â§9) |
| Adaptive Step Smoothing | âœ… Active | 7/10 | Step quality at low speeds |
| Arc Support + BÃ©zier | âœ… Active | 7/10 | G2/G3 arc movements |
| ~~HYBRID_THRESHOLD~~ | âŒ **Never compiled** | â€” | Inside `#if HAS_TRINAMIC` (Â§8.1) |
| `ADAPTIVE_STEP_SMOOTHING` | âœ… Active | 9/10 | Critical because HR4988SQ lacks interpolation (Â§8.4) |
| Classic Jerk | âŒ Disabled | â€” | Junction Deviation is used |
| Backlash Compensation | âŒ Disabled | â€” | Can be enabled if needed |

### 7.2 S-Curve Acceleration

```cpp
#define S_CURVE_ACCELERATION
```

- Uses a BÃ©zier curve instead of flat (trapezoidal) acceleration
- Smoother transitions at the start and end of movement
- Ringing (ghosting) is reduced
- `BEZIER_CURVE_SUPPORT` is also active (G5 command support)

### 7.3 Junction Deviation

```cpp
#define JUNCTION_DEVIATION_MM 0.015
```

- Modern adaptive corner speed algorithm instead of Classic Jerk
- Automatic speed reduction based on corner angle
- `DEFAULT_EJERK 5.0` â€” Extruder jerk (active in both modes)
- Value 0.015 â€” Optimized for Sermoon D1 dimensions

### 7.4 Planner Parameters

```cpp
#define MINIMUM_PLANNER_SPEED    0.05  // mm/s (minimum planned speed)
#define MIN_STEPS_PER_SEGMENT    6     // Minimum steps per segment
```

---

## 8. Driver Settings (mixed: TMC2208 + HR4988SQ)

> âš ï¸ **This section was completely rewritten on 2026-07-23.** The old version described settings like `STEALTHCHOP_*`, `HYBRID_THRESHOLD`, and `INTERPOLATE` as if they were active. None of them are active â€” they are all inside the `#if HAS_TRINAMIC` block in `Configuration_adv.h`, and **that block is not compiled on this board**. Even if you changed these settings, the printer's behavior did not change.

### 8.1 Why none of the TMC settings work

`Marlin/src/core/drivers.h:80` states clearly:

> *"Test for supported TMC drivers that require advanced configuration â€”
> **Does not match standalone configurations**"*

`HAS_TRINAMIC` is only true for **UART/SPI configurable** TMC drivers. On this board:

| Axis | `*_DRIVER_TYPE` | Enters `HAS_TRINAMIC`? |
|---|---|---|
| X, Y | `TMC2208_STANDALONE` | No â€” `_STANDALONE` does not match |
| Z, E0 | `A4988` (HR4988SQ) | No â€” Not TMC |

Result: `HAS_TRINAMIC = false`. Inside the block, **`*_CURRENT`, `*_MICROSTEPS`, `*_RSENSE`, `INTERPOLATE`, `STEALTHCHOP_*`, `HYBRID_THRESHOLD`, `CHOPPER_TIMING`, `MONITOR_DRIVER_STATUS` are never compiled.**

Thus, the following G-codes are also **missing**: `M906` (current), `M569` (chop mode), `M350` (microsteps), `M122` (driver status). There is no sensorless homing either.

### 8.2 What actually determines what

| Setting | Where it is determined | Can it be changed via software? |
|---|---|---|
| Current | **Vref pot** on the driver | âŒ No |
| Microsteps | **MS1/MS2 hard-wired to PCB** (16Ã—, no jumpers) | âŒ No |
| Chopper mode | Hardware default of the chip | âŒ No |
| 256Ã— interpolation | TMC2208's own hardware (**only on X/Y**) | âŒ No |
| Step pulse width | `MINIMUM_STEPPER_PULSE` | âœ… Yes |
| DIR setup time | `MINIMUM_STEPPER_*_DIR_DELAY` | âœ… Yes |
| Step smoothing | `ADAPTIVE_STEP_SMOOTHING` | âœ… Yes |

### 8.3 The three values the firmware actually controls

In a mixed configuration, these macros are **global** (cannot be set per axis), so the **strictest** requirement always applies:

```cpp
#define MINIMUM_STEPPER_PULSE          1       // Âµs  â€” Determines by HR4988SQ
#define MAXIMUM_STEPPER_RATE           400000  // Hz  â€” Determines by TMC2208
#define MINIMUM_STEPPER_POST_DIR_DELAY 200     // ns  â€” Determines by HR4988SQ
#define MINIMUM_STEPPER_PRE_DIR_DELAY  200     // ns
```

| Macro | Value | Determined by | What the other needs |
|---|---|---|---|
| `MINIMUM_STEPPER_PULSE` | 1 Âµs | HR4988SQ | TMC2208 ~100 ns would suffice |
| `MAXIMUM_STEPPER_RATE` | 400 kHz | TMC2208 | HR4988SQ could handle 500 kHz |
| `*_DIR_DELAY` | 200 ns | HR4988SQ | TMC2208 20 ns would suffice |

> **Critical fix in SD1-2.4:** `*_DIR_DELAY` used to be **30 ns** (set for TMC2208) and was **insufficient** for the HR4988SQ. If the DIR pin is not stable early enough from the STEP edge, the driver will take a step in the **old direction**. The most affected paths are where direction changes are frequent: Z during layer changes (parallel dual motors go the wrong way together) and E during every retract. The symptom is layer shifting and inconsistent flow after retraction.

`MAXIMUM_STEPPER_RATE` is not practically binding â€” the fastest axis is X/Y, 250 mm/s Ã— 80 step/mm = 20 kHz, 5% of the ceiling. However, it also dictates the pulse floor in `stepper.h:160`: 72 MHz / 400000 = 180 cycles = **2.5 Âµs**, comfortably above the 1 Âµs required by HR4988SQ.

### 8.4 ADAPTIVE_STEP_SMOOTHING â€” Critical for HR4988SQ

```cpp
#define ADAPTIVE_STEP_SMOOTHING
```

On X/Y, the TMC2208 interpolates the 16Ã— input to 256Ã— internally, so movement is already smooth. **This does not happen on Z/E0** â€” 16Ã— is truly 16Ã—. At low and medium step frequencies, step stepping becomes audible; the vibration is more pronounced because there are two parallel motors on Z.

`ADAPTIVE_STEP_SMOOTHING` closes the gap by effectively doubling the step rate in this range. **It must not be disabled with HR4988SQ.**

### 8.5 Current adjustment â†’ Â§2.2

Vref values, measured currents, R_sense (0.15 Î©), and the different Vref formulas of the two driver families are in **[section 2.2](#22-stepper-motor-drivers)**.

Summary: **factory setting is verified, do not touch the pots.**

---

## 9. Linear Advance (LIN_ADVANCE)

### 9.1 Configuration

```cpp
#define LIN_ADVANCE
#define LIN_ADVANCE_K 0.06   // mm compression per 1mm/s extruder speed
```

### 9.2 K Factor Explanation

- **0.06** â€” reasonable starting value for direct drive (Sermoon D1 = direct drive)
- Direct drive typical range: 0.02 â€“ 0.15 (Valid range for Sermoon D1)
- Bowden typical range: 0.4 â€“ 0.9 (NOT valid for this printer)
- **User calibration mandatory** â€” E0 driver changed in SD1-2.4, starting value is only a reference

### 9.3 K Factor Calibration

1. Download test pattern: https://marlinfw.org/tools/lin_advance/k-factor.html
2. Do a test print with starting K=0
3. Increase K value on each line (by 0.02 increments)
4. Choose the K value that corresponds to the smoothest extrusion line
5. `M900 K0.06` â†’ test â†’ save with `M500`

> âš ï¸ K may differ for each filament brand/change. Calibrate with PLA, re-measure for PETG/ABS.

---

## 10. Z-Probe and Auto Bed Leveling (ABL)

> ## â›” THIS SECTION IS CURRENTLY INVALID
>
> **There is NO Z-probe on this printer.** Neither BLTouch nor an inductive sensor is installed.
> The following are installation notes to follow **if a probe is added in the future**; it does **not** describe the behavior of the current firmware.
>
> Actual state in the firmware:
> - `FIX_MOUNTED_PROBE` â†’ **disabled** (`Configuration.h`)
> - `BLTOUCH` â†’ **disabled**
> - `AUTO_BED_LEVELING_*`, `MESH_BED_LEVELING` â†’ **all disabled** â†’ `HAS_LEVELING` is false
> - Z homing: **mechanical endstop PA7**. G29 does nothing meaningful.
> - Bed leveling: manual, 4 corner screws + "Assistant Level" on DWIN screen (screen takes nozzle to corners) + Z offset fine-tuning (babystep).
>
> **Mandatory step before adding a probe:** PB0/PB1 pins are dedicated to the Z lock module. First, `SERMOON_Z_LOCK` must be disabled in `Configuration_adv.h`. If you enable the probe without disabling it, compilation will stop with a `SanityCheck.h` error.
>
> ### âš ï¸ Source files are no longer in the tree
>
> During the SD1-2.2 dead code cleanup, the probe and bed-leveling source files were deleted (their contribution to the binary was already 0 bytes). Before following this section, you must restore them:
>
> ```bash
> # Recover from the pre-cleanup base commit
> git checkout 1d2ba27 -- \
>   Marlin/src/module/probe.cpp \
>   Marlin/src/feature/bedlevel \
>   Marlin/src/gcode/probe \
>   Marlin/src/gcode/bedlevel \
>   Marlin/src/gcode/calibrate/M48.cpp \
>   Marlin/src/gcode/calibrate/G425.cpp \
>   Marlin/src/libs/vector_3.cpp \
>   Marlin/src/libs/least_squares_fit.cpp \
>   Marlin/src/libs/least_squares_fit.h
> ```
>
> Header files (`probe.h`, `bedlevel.h`, etc.) **were not deleted** â€” they are still in the tree. Only the `.cpp` bodies and two orphaned headers must be recovered. Full list: `git show --stat 1d2ba27..HEAD`.

### 10.1 Sensor Information (if probe is added â€” reference)

| Feature | Value |
|---------|-------|
| Sensor | BES M18MG-PSC15F-S04K |
| Type | Inductive proximity sensor |
| Output | PNP Normally Open (NO) |
| Supply | 10â€“30V DC (24V used) |
| Sensing distance | 15 mm (steel) / **~5â€“7 mm (aluminum)** |
| Size | M18 Ã— ~65 mm |
| Sensing surface | Aluminum bed |

### 10.2 Hardware Installation

#### 10.2.1 Sensor Mounting

- Mount the sensor to the print head with a bracket
- Sensor face should be **2â€“3 mm above the nozzle tip**
- Sensor center should be as close to the nozzle as possible (small X/Y offset = accurate probing)
- M18 (18mm diameter) â€” space might need to be cleared in the head design

#### 10.2.2 Voltage Divider Circuit (CRITICAL!)

```
âš ï¸ WARNING: Sensor output is 24V, MCU is 3.3V!
If you connect it directly, the STM32 MCU will burn out â€” the board will become unusable!

A voltage divider is MANDATORY:
```

```
Sensor black wire (signal, 24V)
         â”‚
    [R1: 10kÎ©]
         â”‚
         â”œâ”€â”€â”€â”€â”€â”€â”€â”€ MCU PB1 pin (3.13V)
         â”‚
    [R2: 1.5kÎ©]
         â”‚
        GND

Vout = 24V Ã— R2/(R1+R2) = 24 Ã— 1.5/11.5 = 3.13V âœ“
```

**Cable colors (Balluff standard):**

| Color | Function | Connection |
|------|-----------|----------|
| Brown | VCC (10-30V) | To power supply 24V terminal |
| Blue | GND | Power supply GND + Board GND (common) |
| Black | Signal output | To R1 input â†’ R1/R2 divider â†’ PB1 |

#### 10.2.3 Board Connection Point

- **PB1 pin:** Corresponds to the signal pin on the board's BLTouch connector
- Sensor 24V supply: Will be taken from the power supply terminal
- Board 5V output will not be used â€” sensor requires 10-30V
- GND: Common ground for sensor and board

### 10.3 Firmware Configuration

#### 10.3.1 Pin Definition (`pins_CREALITY.h`)

> âš ï¸ **PB1 is not free.** PB0 and PB1 are driven as OUTPUT by `SERMOON_Z_LOCK`. To assign the probe to PB1, the Z lock must first be disabled â€” making the same pin both output and input silently disables the Z lock on boot because `endstops.init()` runs after `zlock.init()`, and turns `M888` into a command that plays with the probe pin's bias.

```cpp
// CURRENT STATE:
#define Z_MIN_PIN        PA7   // Mechanical endstop â€” homing
// Z_MIN_PROBE_PIN is NOT defined (no probe)
#define Z_KEEP_PIN_PB0   PB0   // Z lock
#define Z_KEEP_PIN_PB1   PB1   // Z lock

// IF PROBE IS ADDED: first disable SERMOON_Z_LOCK, then
// #define Z_MIN_PROBE_PIN PB1
```

#### 10.3.2 Probe Settings (`Configuration.h`) â€” if probe is added

```cpp
#define FIX_MOUNTED_PROBE                              // Fixed mounted sensor (CURRENTLY OFF)
//#define Z_MIN_PROBE_USES_Z_MIN_ENDSTOP_PIN           // A separate pin is used
#define Z_MIN_PROBE_ENDSTOP_INVERTING false            // PNP NO: HIGH = trigger
#define NOZZLE_TO_PROBE_OFFSET { 0, 0, 2 }            // Starting â€” measure and calibrate!
#define MIN_PROBE_EDGE            30                   // Minimum distance from bed edge
#define XY_PROBE_SPEED            3000                 // Speed between probe points (mm/m)
#define Z_PROBE_SPEED_FAST        HOMING_FEEDRATE_Z    // 240 mm/m
#define Z_PROBE_SPEED_SLOW        (Z_PROBE_SPEED_FAST / 2)  // 120 mm/m
#define MULTIPLE_PROBING          2                    // Probe 2Ã— at each point
#define Z_CLEARANCE_DEPLOY_PROBE  10                   // Z height before probing
#define Z_CLEARANCE_BETWEEN_PROBES 5                   // Z height between probe points
#define Z_PROBE_LOW_POINT         -2                   // Minimum distance below trigger
#define Z_PROBE_OFFSET_RANGE_MIN  -10                  // M851 minimum
#define Z_PROBE_OFFSET_RANGE_MAX  10                   // M851 maximum
```

#### 10.3.3 NOZZLE_TO_PROBE_OFFSET Detail

```
{ X, Y, Z }

X â†’ If sensor is to the RIGHT of the nozzle, positive (+), if left, negative (-)
Y â†’ If sensor is BEHIND the nozzle, positive (+), if in front, negative (-)
Z â†’ If sensor face is ABOVE the nozzle tip, positive (+), if below, negative (-)
```

**Example:** If the sensor is mounted 10mm to the right, 5mm behind, and 3mm above the nozzle:
```cpp
#define NOZZLE_TO_PROBE_OFFSET { 10, 5, 3 }
```

> âš ï¸ This value must be measured with calipers/micrometer after the sensor is mounted. {0, 0, 2} is a starting value.

#### 10.3.4 ABL Settings

> â›” **None of these are currently active.** All `AUTO_BED_LEVELING_*` and `MESH_BED_LEVELING` lines are commented out in `Configuration.h`; `HAS_LEVELING` is false. The block below is a reference for settings to be enabled **after** a probe is added â€” it does not describe the current firmware.

```cpp
// ALL ARE OFF RIGHT NOW â€” enable if probe is added:
//#define AUTO_BED_LEVELING_BILINEAR   // Grid based leveling
//#define GRID_MAX_POINTS_X            4   // 4Ã—4 = 16 probe points
//#define ENABLE_LEVELING_FADE_HEIGHT  // Correction fade out in height
//#define SEGMENT_LEVELED_MOVES        // Segmented movement over mesh
```

**Grid visualization (4Ã—4):**

```
   â—‹â”€â”€â—‹â”€â”€â—‹â”€â”€â—‹   â† Y = 270 - 30 = 240 (MIN_PROBE_EDGE)
   â”‚           â”‚
   â—‹â”€â”€â—‹â”€â”€â—‹â”€â”€â—‹
   â”‚           â”‚
   â—‹â”€â”€â—‹â”€â”€â—‹â”€â”€â—‹
   â”‚           â”‚
   â—‹â”€â”€â—‹â”€â”€â—‹â”€â”€â—‹   â† Y = 30
   â†‘           â†‘
  X=30       X=260

Bed size: 290 Ã— 270 mm
Probe area: 230 Ã— 210 mm (30mm margin)
```

### 10.4 Calibration Procedure

#### Step 1: Sensor Test

```
M119              â† Check endstop states
```

Expected output:
- Sensor away from bed: `z_min: TRIGGERED` (mechanical) + `z_probe: OPEN`
- Sensor close to bed: `z_probe: TRIGGERED`
- If state is reversed â†’ change `Z_MIN_PROBE_ENDSTOP_INVERTING` value
#### Step 2: First Probe

```
M851 Z0           â† Reset probe offset
G28               â† Home all (SD1-2.8: ends at (âˆ’10, âˆ’10), DOES NOT go to bed center)
G29               â† Do 4Ã—4 grid probe
M420 V            â† Show mesh data
```

#### Step 3: Z Offset Calibration

```
G28               â† Home all
G29               â† Probe
G1 X145 Y135 Z0   â† Bring nozzle to bed center
```

Do a paper test:
- Paper gets stuck under the nozzle â†’ Z offset is too low â†’ `M851 Z-0.1` (towards negative)
- Paper is loose under the nozzle â†’ Z offset is too high â†’ `M851 Z0.1` (towards positive)
- Paper passes with slight friction â†’ OK

```
M500              â† Save to EEPROM
```

#### Step 4: Slicer Start G-Code

Add to the start G-code section of the slicer (Cura, PrusaSlicer, etc.):

```gcode
G28                ; Home all axes
G29                ; Auto bed level (4Ã—4 grid)
; M420 S1          ; G29 already activates the mesh, just as a backup
```

### 10.5 G-Code Commands Related to Probe

| Command | Description |
|-------|----------|
| `G28` | Home all (Z_SAFE_HOMING active) |
| `G29` | Do ABL probe (4Ã—4 grid) |
| `G30` | Single point probe |
| `M420 S1` | Activate mesh |
| `M420 S0` | Disable mesh |
| `M420 V` | Print mesh data |
| `M420 Z10` | Set fade height to 10mm |
| `M851 Z0.5` | Set probe Z offset |
| `M851` | Show current probe offset |
| `M48 P10` | 10Ã— repeatability test |
| `M500` | Save to EEPROM |
| `M503` | Show all settings |

---

## 11. Temperature and Safety

### 11.1 Temperature Limits

| Component | Min | Max | Description |
|---------|-----|-----|----------|
| Hotend (E0) | 0Â°C | 275Â°C | V-hotend |
| Bed | 0Â°C | 110Â°C | Aluminum + heater |
| Extrude min | 180Â°C | â€” | Extrusion is prevented below this |

### 11.2 Preheating Profiles

| Profile | Hotend | Bed | Usage |
|--------|--------|-----|----------|
| PLA | 195Â°C | 45Â°C | Standard |
| ABS/PETG | 240Â°C | 100Â°C | High temperature |

### 11.3 Thermal Protections

```cpp
#define THERMAL_PROTECTION_HOTENDS   // Hotend thermal runaway protection
#define THERMAL_PROTECTION_BED       // Bed thermal runaway protection
```

Both protections are active. The heater automatically shuts down in case of a sensor failure.

---

## 12. EEPROM and SD Card

### 12.1 EEPROM

```cpp
#define EEPROM_SETTINGS              // Save settings to EEPROM (M500)
#define EEPROM_AUTO_INIT             // Write defaults on first boot
```

- Type: I2C EEPROM (24C16, 16Kb)
- Pins: SDA=PA11, SCL=PA12
- `M500` â†’ save, `M501` â†’ load, `M502` â†’ factory settings, `M503` â†’ show

### 12.2 SD Card

```cpp
#define SDSUPPORT                    // SD card support
#define SD_DETECT_PIN      PC7       // SD card detection pin
```

- Firmware flashing: via SD card (FAT32 format)
- Print files: Read from SD card
- Max filename: long filename support is active

### 12.3 Print Statistics

```cpp
#define PRINTCOUNTER                 // Print counter active
```

---

## 13. DWIN Display Integration

### 13.1 Display Configuration

```cpp
#define SizeofDatabuf      40        // Data buffer size (26 â†’ 40 fix)
#define FIRMWARE_VERSION   "MarlinV2 by CTK"
#define MACHINE_TYPE       "Sermoon D1"
#define HARDWARE_VERSION   "HW 4.3.1"
#define SCREEN_VERSION     "DWIN 1.1.14"
#define PRINT_SIZE         "280*260*310"
```

### 13.2 DWIN Protocol

- MCU-DWIN communication via RTS (Real-Time Serial) protocol
- `SizeofDatabuf = 40` â€” was 26 bytes in original Creality code, increased to 40 for overflow fix
- Version string limited to ~14 characters (screen space constraint)

### 13.3 Settings via Display

- Babystepping is active (`BABYSTEPPING`)
- `BABYSTEP_ZPROBE_OFFSET` â€” Babystep syncs with M851 Z
- Z offset can be adjusted from the screen

---

## 14. Pin Map

### 14.1 Endstop Pins

| Function | Pin | MCU Pin | Note |
|-----------|-----|---------|-----|
| X_MIN | PA5 | PA5 | Mechanical endstop |
| Y_MIN | PA6 | PA6 | Mechanical endstop |
| Z_MIN | PA7 | PA7 | Mechanical endstop (homing) |
| Z_MIN_PROBE | â€” | â€” | **Undefined** â€” no probe, PB1 dedicated to Z lock |

### 14.2 Stepper Motor Pins

| Function | Enable | Step | Dir | Driver |
|-----------|--------|------|-----|--------|
| X | PC3 | PC2 | PB9 | TMC2208 standalone |
| Y | PC3 | PB8 | PB7 | TMC2208 standalone |
| Z | PC3 | PB6 | PB5 | **HR4988SQ** â€” **2 parallel motors** to this single set |
| E0 | PC3 | PB4 | PB3 | **HR4988SQ** |

> **Common enable pin (PC3), active low.** Marlin does not count this pin per-axis: a `disable_Z()` call pulling PC3 inactive will also release X/Y/E0. In practice this does not cause problems because the axes are only disabled when they are all idle (`DEFAULT_STEPPER_DEACTIVE_TIME` + all `DISABLE_INACTIVE_*` true).
> **Result:** it is not possible to independently disable a single axis due to heating, and `DISABLE_X/Y/Z/E` in `Configuration.h` must all remain `false`.

> **PB3/PB4 are the JTAG line** (JTDO / JNTRST) and are used as E0_DIR/E0_STEP. If the `DISABLE_DEBUG` definition in `pins_CREALITY.h` is removed, these pins won't turn into GPIO and **the extruder will not move at all**.

> **There is NO Z2_* pin definition for Z, and there shouldn't be.** Two motors are connected in parallel to a single driver; `Z2_DRIVER_TYPE` means a second independent driver, which does not exist in the hardware.

### 14.3 Temperature and Heater Pins

| Function | Pin |
|-----------|-----|
| Hotend thermistor | PC5 |
| Bed thermistor | PC4 |
| Hotend heater | PA1 |
| Bed heater | PA2 |
| Fan | PA0 |

### 14.4 Other Pins

| Function | Pin | Note |
|-----------|-----|-----|
| SD Detect | PC7 | SD card detection |
| Filament runout | PA4 | Optical sensor |
| I2C EEPROM SDA | PA11 | |
| I2C EEPROM SCL | PA12 | |
| Z Lock IN | PB1 | Board's "BLTouch" connector â€” completely dedicated to Z lock |
| Z Lock OUT | PB0 | Same connector |

---

## 15. Calibration Guide

### 15.1 Dimensional Accuracy Test

1. Print a 20Ã—20Ã—20 mm cube
2. Measure X, Y, Z dimensions with digital calipers
3. If there is a deviation:

```
M92 X80.00       â† Set X steps/mm
M92 Y79.60       â† Set Y steps/mm (correct if necessary)
M92 Z400.00      â† Z steps/mm
M92 E95.00       â† E steps/mm
M500              â† Save to EEPROM
```

### 15.2 LIN_ADVANCE_K Calibration

1. Generate a test pattern from https://marlinfw.org/tools/lin_advance/k-factor.html
2. Start with K=0, increase K by 0.02 on each line
3. Choose the most consistent extrusion line
4. `M900 K<new_value>` â†’ `M500`

### 15.3 Z Offset (First Layer) Calibration

Because there is no probe, `M851` is not used. Z offset is adjusted via **babystep**:

1. `G28` â€” do a home
2. Start a test print (a single-layer square is ideal)
3. While the first layer is printing, on the DWIN screen use **Adjust â†’ Z offset Â± buttons**
   - Each press is 0.1 mm (`LCD_RTS.cpp`, `babystep.add_mm`)
   - If the nozzle is too close (+), if too far (âˆ’)
4. When the result is good, `M500` â€” save to EEPROM

> The Z offset value on the screen is the DWIN driver's own `zprobe_zoffset` variable and is applied directly to the babystep; it has nothing to do with Marlin's probe offset. Thus, it works perfectly even without a probe.

### 15.4 E Steps Calibration

1. Push filament up to the extruder inlet
2. Mark 120mm on the filament
3. `G1 E100 F100` â€” extrude 100mm
4. Measure the remaining distance from the mark to the nozzle
5. If Remaining = 20mm, it's correct (120-100=20)
6. If there is a deviation:

```
M92 E<new_steps>   â† current_steps Ã— (100 / actual_extrude_mm)
M500
```

---

## 16. G-Code Reference

### 16.1 Basic Movement

| Command | Description |
|-------|----------|
| `G0 X Y Z F` | Rapid movement (travel) |
| `G1 X Y Z E F` | Linear movement (print) |
| `G28` | Home all axes |
| `G29` | Auto bed level |
| `G92 X Y Z E` | Reset position |

### 16.2 Calibration

| Command | Description |
|-------|----------|
| `M851 Z<value>` | Set Z probe offset |
| `M92 X Y Z E` | Set Steps/mm |
| `M900 K<value>` | Linear Advance K factor |
| `M201 X Y Z E` | Maximum acceleration |
| `M203 X Y Z E` | Maximum speed |
| `M204 P R T` | Acceleration (Print/Retract/Travel) |
| `M205 J<value>` | Junction deviation |
| `M206 X Y Z` | Home offset |

### 16.3 Probe and Leveling â€” **not in this firmware**

**None** of the `G29`, `G30`, `M420`, `M421`, `M48`, `M851` commands were compiled. Because there is no probe hardware, `HAS_BED_PROBE` and `HAS_LEVELING` are false; if these commands are sent, the printer returns `unknown command`.

Bed leveling on this printer is **manual**:

| Method | Description |
|--------|----------|
| 4 corner screws | Classic paper method |
| DWIN "Assistant Level" | Screen takes the nozzle to 5 points in sequence (corners + center) |
| Z offset babystep | First layer fine-tuning â€” [section 15.3](#153-z-offset-first-layer-calibration) |

### 16.3b Sermoon-Specific

| Command | Description |
|-------|----------|
| `M888` | Query Z lock status |
| `M888 S0` | Release Z lock |
| `M888 S1` | Engage Z lock (default) |

### 16.4 EEPROM

| Command | Description |
|-------|----------|
| `M500` | Save to EEPROM |
| `M501` | Load from EEPROM |
| `M502` | Reset to factory settings |
| `M503` | Show all settings |

### 16.5 Diagnostics and Testing

| Command | Description |
|-------|----------|
| `M119` | Show endstop states |
| `M115` | Show firmware version |
| `M48 P10` | Probe repeatability test |
| `M111 S32` | Turn on leveling debug log |

---

## 17. Troubleshooting

### 17.1 Compilation Errors

| Problem | Solution |
|-------|-------|
| `pio: command not found` | PlatformIO CLI not installed â€” `pip install platformio` |
| `No module named platformio` | Python 3.14 incompatibility â€” use `pio` CLI command |
| HAL file error | Check STM32 platform version â€” `ststm32@<6.2.0` required |

### 17.2 Firmware Flashing Issues

| Problem | Solution |
|-------|-------|
| Screen doesn't turn on | Remove SD card, restart. Bootloader might not be waiting |
| Version did not update | Old .bin file might be left on SD card â€” delete it and re-flash |
| EEPROM error | Restore factory settings with `M502` â†’ `M500` |

### 17.3 Homing and Z Lock Issues

Because there is no probe, the probe troubleshooting section was removed. Instead:

| Problem | Solution |
|-------|-------|
| `M119` z_min is always TRIGGERED | Mechanical endstop (PA7) is stuck or cable broken; check `Z_MIN_ENDSTOP_INVERTING` |
| Z does not home | Check if endstop is triggered with `M119`; does `Z_HOMING_HEIGHT 4` leave enough clearance |
| Nozzle too close/far to bed after G28 | Adjust with Z offset babystep ([section 15.3](#153-z-offset-first-layer-calibration)), `M500` |
| Z axis drops on its own | Query lock state with `M888`; if not `ENGAGED`, do `M888 S1` |
| `M888` unresponsive | Check if `SERMOON_Z_LOCK` is compiled (`Configuration_adv.h`) |

> **Note:** Because PB0/PB1 are dedicated to the Z lock, do not write to these pins manually with `M42 P` â€” you will break the lock state.

### 17.4 Print Quality Issues

| Problem | Solution |
|-------|-------|
| Ringing (ghosting) | Lower `DEFAULT_ACCELERATION` (500â†’300), lower `JUNCTION_DEVIATION_MM` |
| Layer shift (X/Y) | Measure if Vref is 1.27 V (0.69 A RMS). Check belt tension and pulley screw. **Do not look for `HYBRID_THRESHOLD` â€” it is not on this board** |
| Layer shift (Z) or bad flow after retract | Check if `MINIMUM_STEPPER_*_DIR_DELAY` is **200**. At 30 ns, HR4988SQ takes a reverse step (Â§8.3) |
| First layer doesn't stick | Calibrate Z offset with babystep ([15.3](#153-z-offset-first-layer-calibration)), level the bed manually |
| Inconsistent extrusion | Calibrate `LIN_ADVANCE_K`, verify E steps |
| Poor surface / audible steps on Z | Check if `ADAPTIVE_STEP_SMOOTHING` is on. `STEALTHCHOP`/`INTERPOLATE` **are not on this board** (Â§8.1) â€” HR4988SQ does not interpolate on Z/E0 |

### 17.5 Sensor Voltage Divider Verification â€” *only if probe is added*

> These steps do not apply to the current printer (no probe). If an inductive sensor is added in the future, they will be used **after `SERMOON_Z_LOCK` is disabled**.

Measure with a multimeter:
1. Sensor unpowered (no metal nearby) â†’ black wire â‰ˆ 0V
2. Sensor triggered (metal nearby) â†’ black wire â‰ˆ 24V
3. Voltage divider output (R1-R2 junction) â†’ triggered â‰ˆ 3.1V, untriggered â‰ˆ 0V

---

## 18. Changelog

### Version: MarlinV2 by CTK â€” SD1-2.1 (2026-07-21)

#### Session 4 â€” Alignment with Hardware Reality + Critical Fixes

- **Z-probe removed.** The printer has no probe hardware. `FIX_MOUNTED_PROBE` was disabled; PB0/PB1 were fully dedicated to the Z lock module. The probe integration added in Session 3 **conflicted** because it used the same pins as the Z lock: since `endstops.init()` ran after `zlock.init()`, the Z lock on PB1 was silently disabled on boot.
- **PLR (power-loss recovery) fixed.** `E2END` 0x800 â†’ 0x7FF. The wrong value pushed the PLR region outside the chip (1853..2048, whereas 24C16 is only 0..2047). The `valid_foot` byte was never written and read from the wrong address â†’ `recovery.valid()` was never true.
- **EEPROM read protocol fixed.** The read control byte after a restart was fixed at `0xA1`; because it did not carry the block-select bits, all addresses above 255 were read from block 0. Also, sequential read is now split at the 256-byte block boundary.
- **Thermistor protection regained.** `HEATER_0_MINTEMP` and `BED_MINTEMP` 0 â†’ 5.
- **Out-of-bounds array write fixed.** Inside `RTS_HandleData()`, `axis`, `min`, `max` could be used uninitialized.
- **Compile-time protections added.** `SanityCheck.h`: Z lock + probe cannot be enabled simultaneously. `powerloss.cpp`: PLR region must be within EEPROM and block boundaries (`static_assert`).
- `monitor_speed` 250000 â†’ 115200 (matched with `BAUDRATE`).
- Flash: 187,028 â†’ **184,196** bytes. **0 warnings** in project code.

### SD1-2.4 (2026-07-23) â€” Mixed driver + document audit

**Firmware**
- `Z_DRIVER_TYPE` / `E0_DRIVER_TYPE`: `TMC2208_STANDALONE` â†’ `A4988`
  (Physical chip is **HR4988SQ**; there is no HR4988 type in Marlin, A4988 is its hardware equivalent). X/Y remained TMC2208 standalone.
- **CRITICAL:** `MINIMUM_STEPPER_POST_DIR_DELAY` / `PRE_DIR_DELAY`
  **30 ns â†’ 200 ns**. 30 ns was insufficient for HR4988SQ â†’ risk of reverse steps during direction changes (Z layer transition, E retract). See Â§8.3.
- Flash +40 bytes (127,080 â†’ 127,120), RAM unchanged. **Requires re-flash.**

**Document audit â€” claims contradicting measurement were fixed**

| Where | It said | Reality |
|---|---|---|
| Â§2.2 | All 4 axes TMC2208, 800 mA RMS | Mixed; X/Y 0.69 A, Z 0.47 A/motor, E0 0.51 A |
| Â§2.2 | `RSENSE 0.11 Î©` | **0.15 Î©** (`R150`), measured |
| Â§2.2 | Z single motor | **Two parallel motors**, single driver |
| Â§2.3, Â§5.1 | "Direct drive" | ~~Made "Bowden"~~ â€” **reverted 2026-07-24**: hardware is direct drive (see below) |
| Â§5.2 | "TMC2208 StealthChop limit" | Chassis resonance (300â†’250) |
| **All of Â§8** | StealthChop/HYBRID/INTERPOLATE active | **Never compiled** â€” `#if HAS_TRINAMIC` false |
| Â§17 | "Check HYBRID_THRESHOLD" | Not on this board; check DIR delay and Vref |

- `Configuration_adv.h`: A warning explaining that the `#if HAS_TRINAMIC` block is dead was added to its top; `*_RSENSE` 0.11 â†’ 0.15; `CHOPPER_TIMING`'s "used because HAS_TRINAMIC is active" comment (which was wrong) was fixed.

### Version: MarlinV2 by CTK (2026-05-23)

#### Session 1 â€” Basic Fixes
- Missing HAL source files were added to the build
- `SizeofDatabuf` 26 â†’ 40 bytes (DWIN screen overflow fix)
- Version string: `"MarlinV2 by CTK"`

#### Session 2 â€” Movement Optimization
- `HOMING_FEEDRATE_XY`: 3000 â†’ 1000 mm/m (smoother homing)
- `LIN_ADVANCE_K`: 0.22 â†’ 0.06 (starting value for direct drive; user calibration mandatory)
- ~~`HYBRID_THRESHOLD`: activated~~ â€” **never active**; was not compiling because it was inside the `#if HAS_TRINAMIC` block (2026-07-23 audit)

#### Session 3 â€” Z-Probe / ABL Integration *(Reverted in SD1-2.1 â€” see Session 4)*
- `pins_CREALITY.h`: PB1 â†’ `Z_MIN_PROBE_PIN` (conditional on FIX_MOUNTED_PROBE)
- `Z_MIN_PROBE_ENDSTOP_INVERTING`: true â†’ false (PNP NO sensor)
- `Z_MIN_PROBE_USES_Z_MIN_ENDSTOP_PIN`: commented out (separate probe pin)
- `FIX_MOUNTED_PROBE`: active (inductive sensor)
- `NOZZLE_TO_PROBE_OFFSET`: {0,0,0} â†’ {0,0,2} (starting)
- `AUTO_BED_LEVELING_BILINEAR`: active (4Ã—4 grid)
- `GRID_MAX_POINTS_X`: 3 â†’ 4
- `Z_SAFE_HOMING`: active
- `MULTIPLE_PROBING`: 2 (dual probing)
- Flash size: 184,300 â†’ 194,524 bytes (+10,224 bytes ABL code)

---

## Appendix A: File Locations

```
sermoon-d1-backup/
â”œâ”€â”€ Marlin/
â”‚   â”œâ”€â”€ Configuration.h          â† Main configuration
â”‚   â”œâ”€â”€ Configuration_adv.h      â† Advanced configuration
â”‚   â”œâ”€â”€ Version.h                â† Version strings
â”‚   â””â”€â”€ src/
â”‚       â”œâ”€â”€ pins/stm32/
â”‚       â”‚   â””â”€â”€ pins_CREALITY.h  â† Board pin definitions
â”‚       â”œâ”€â”€ lcd/dwin/
â”‚       â”‚   â”œâ”€â”€ LCD_RTS.h        â† DWIN display header
â”‚       â”‚   â””â”€â”€ LCD_RTS.cpp      â† DWIN display implementation
â”‚       â””â”€â”€ HAL/HAL_STM32F1/     â† STM32F1 HAL layer
â”œâ”€â”€ buildroot/
â”‚   â””â”€â”€ share/PlatformIO/
â”‚       â”œâ”€â”€ scripts/creality.py  â† Firmware relocate script (0x08007000)
â”‚       â””â”€â”€ ldscripts/creality.ld â† Linker script
â”œâ”€â”€ .pio/build/creality/
â”‚   â””â”€â”€ firmware.bin             â† Compiled firmware binary
â”œâ”€â”€ platformio.ini               â† PlatformIO project configuration
â””â”€â”€ MANUAL.md                    â† This file
```

## Appendix B: Useful Links

- Marlin Probe Configuration: https://marlinfw.org/docs/configuration/probes.html
- LIN_ADVANCE K-Factor Calibration: https://marlinfw.org/tools/lin_advance/k-factor.html
- Marlin G-Code Reference: https://marlinfw.org/meta/gcode/
- Balluff BES M18 Series: https://www.balluff.com/en-de/products/areas/A0001/groups/G0101

---

*This manual documents the entire configuration of the MarlinV2 by CTK firmware on the Sermoon D1 printer. It should be updated with every change.*
