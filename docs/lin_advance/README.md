# Sermoon D1 — LIN_ADVANCE K Calibration Guide

LIN_ADVANCE (Linear Advance) **pre-calculates filament pressure** during extruder speed changes, reducing corner blobs, stringing, and under-extrusion issues. The Sermoon D1 features a **direct drive** extruder (gearless MK8 type, drive gear directly above the hotend) — therefore, K values are small but not zero.

## Why is it Important for Sermoon?

In a direct drive, the path between the drive gear and the nozzle is short, yet there is still an elastic pressure line: the short PTFE throat + the molten plastic in the melt zone.

```
Drive gear ──[throat + melt zone]── Nozzle
              ↑
         pressure build-up is delayed here
```

The result (on a smaller scale compared to Bowden, but still visible):
- When the printer stops → line is still pressurized → **over-extrusion** (corner blob)
- When the printer accelerates → until pressure settles → **under-extrusion** (corner gap)
- After retraction → priming delay → **layer start defect**

LIN_ADVANCE models this: speed changes are not instantaneous, it calculates based on pressure dynamics. The K coefficient is this pressure-time constant. Because the path is short, the K value for Sermoon typically falls in the **0.02-0.15** range (compared to 0.4-0.9 for Bowden).

## Current Status

| Setting | Value | Location |
|---|---|---|
| `LIN_ADVANCE` | ✅ Active | `Configuration_adv.h:1466` |
| `LIN_ADVANCE_K` (default) | 0.06 | `Configuration_adv.h:1483` |
| `M900` (runtime K set) | ✅ Working | `gcode/feature/advance/M900.cpp` |
| `M500` (EEPROM save) | ✅ Active | — |

⚠️ **Default K=0.06 is a reasonable starting point for direct drive but unreliable without calibration** — the E0 driver changed to HR4988SQ in SD1-2.4, and **K is driver-specific**; if you have an old calibration value, it is also invalid.

## Sermoon Direct Drive K Ranges

| Filament | Typical K range | Starting point |
|---|---|---|
| PLA | 0.02 - 0.10 | Test 0.02, 0.06, 0.10 |
| PETG | 0.04 - 0.15 | Test 0.05, 0.10, 0.15 |
| ABS | 0.02 - 0.10 | Test 0.02, 0.06, 0.10 |
| TPU | 0.10 - 0.40 | Test 0.10, 0.25, 0.40 |
| Flexible/Flex | 0.20 - 0.60 | Test 0.20, 0.40, 0.60 |

> These ranges are a starting window, not a guarantee — the correct K is determined by the pattern. Different filament brands on the same printer may yield a different K (filament flexibility and temperature play a role). Separate calibration is recommended for each main filament.

## Calibration Methods

### Method A — Marlin Online K-Factor Tool (RECOMMENDED)

Marlin's official tool automatically generates the best test pattern.

**1.** Open the following link:
```
https://marlinfw.org/tools/lin_advance/k-factor.html
```

**2.** Enter the Sermoon D1 parameters:

| Field | Value |
|---|---|
| Filament Type | PLA (or what you are using) |
| Filament Diameter | 1.75 |
| Direct Drive | **Yes** (Sermoon D1 is direct drive) |
| Bed Size X | 290 |
| Bed Size Y | 270 |
| Origin Bed Center | No |
| Nozzle Temperature | 210 (PLA) / 240 (ABS) / 230 (PETG) |
| Bed Temperature | 60 (PLA) / 100 (ABS) / 80 (PETG) |
| Nozzle Diameter | 0.4 |
| Layer Height | 0.2 |
| Slow Speed | 20 mm/s |
| Fast Speed | 80 mm/s |
| K-Factor Start | **0.0** |
| K-Factor End | **0.3** |
| K-Factor Step | **0.02** |
| Number of Test Lines | 16 |
| Use TX in stock GCode | Yes (Marlin) |

**3.** Click the "Generate G-code" button and download the generated `.gcode` file.

**4.** Copy to the SD card and run it from the printer.

**5.** Look at the numbered lines in the finished print pattern — whichever K value yields the most uniform transition is your K.

### Method B — Manual Tower Test (time-consuming)

Use the `la_tower_test.gcode` file:
1. Open the file, find the `M900 K0.06` line at the top
2. Run once with a K value (e.g. 0.02) → record the result
3. Run again with K=0.06
4. Run again with K=0.10
5. Choose the K value that gives the most uniform pattern

It is fast but requires 5-6 prints. The online tool does this in a single print.

## Result Analysis (Visual Guide)

After the test pattern is finished, look at each K line from the side:

### ✅ Optimal K (e.g. 0.04 - 0.10)
**Uniform wall** across all speed transitions — constant thickness, smooth surface.

```
═══════════════════════════════════════
Slow │  Fast  │ Slow │  Fast  │ Slow
     ↑        ↑      ↑        ↑
  Transitions clear, no blob/gap
```

### ❌ K too LOW (e.g. 0.00 - 0.02)
**Thinning** on slow-to-fast transition, **swelling/blob** on fast-to-slow transition.

```
══════╗   ╔══════╗   ╔══════╗  ← corner BLOB
      ╚═══╝      ╚═══╝         ← THINNING in fast region
```

### ❌ K too HIGH (e.g. 0.25+)
**Gap** on slow-to-fast transition, **thinning** on fast-to-slow transition.

```
══╗   ╔════════╗   ╔══════════ ← GAP at transition
   ╚══╝         ╚══╝
```

## Setting and Saving K

### Temporary (for testing)
```gcode
M900 K0.06    ; LIN_ADVANCE K = 0.06
```

### Permanent (EEPROM)
```gcode
M900 K0.06    ; Set
M500          ; Write to EEPROM
M501          ; Verification: load
M503          ; Show all settings — M900 K0.06 line should appear
```

### Firmware permanent (unaffected even by factory reset)

In Configuration_adv.h:
```c
#define LIN_ADVANCE_K 0.06   // Sermoon D1 PLA — calibrated YYYY-MM-DD
```
Then recompile and flash with `pio run -e creality`.

## Slicer Integration

A single K value will not suffice for all filaments. Place material-specific M900 in the **slicer's start gcode**:

### Cura
Add above "Filament Settings" → "Start G-code":
```gcode
M900 K0.06  ; Optimized for PLA
```

### PrusaSlicer
"Filament Settings" → "Custom G-code" → "Start G-code":
```gcode
M900 K0.06
```

### OrcaSlicer / Bambu Studio
In filament profile under "Filament start G-code":
```gcode
M900 K0.06
```

## Recommended Material-Specific Profile

After your calibration is complete, document it for each material:

```
Sermoon D1 — My LIN_ADVANCE K profile:
─────────────────────────────────────
Filament              K
─────────────────────────────────────
Generic PLA           0.06
PETG                  0.10
ABS                   0.05
TPU 95A               0.25
Esun PLA+             0.07
Polymaker PLA         0.06
─────────────────────────────────────
Date: YYYY-MM-DD
```

Keep this list near the printer and mirror it to your slicer profiles.

## Verification Print

After calibration, print a real model — for example:
- 20×20×20 calibration cube (Cura's own model)
- "All In One Test" model (Thingiverse)
- If the surface has **clean edges** in the corners and uniform layer thickness → tuning is successful

## Known Interaction: S_CURVE_ACCELERATION

In this firmware, `S_CURVE_ACCELERATION` is active (`Configuration.h:859`) and has a structural tension with LA: LA's extruder compensation speed per block (`advance_speed`) is calculated assuming **constant (trapezoidal) acceleration**; S-curve, however, alters the instantaneous acceleration during the phase (~0 at the extremes, ~1.9 times the average at the peak).
As it appears in the code: `stepper.cpp:1582/1627` — `LA_isr_rate` is a single value throughout the block, it does not follow the Bézier speed curve. Upstream Marlin once blocked this pairing with a SanityCheck and placed it behind the `EXPERIMENTAL_SCURVE` flag; this codebase is older than that protection.

Practical result:
- Because direct drive K's are small (≤0.15), the compensation steps added by LA are few — the practical effect of the interaction is much lower compared to Bowden setups; it goes unnoticed in most prints.
- **Symptom**: If no K line is perfectly uniform in the calibration pattern and the blob at speed-transition corners persists at every K value → disable `S_CURVE_ACCELERATION` (`Configuration.h:859`), recompile, and print the pattern again. Corner-speed control is already provided by Junction Deviation; the loss of being without S-curve is minor. If you disable it, recalibrate K as well.

## Troubleshooting

**"Echo:Unknown command: M900"**
→ LIN_ADVANCE flag is not active. Check `Configuration_adv.h:1466`, recompile with `pio run -e creality`.

**I see no difference at all after the test finishes**
→ K differences in direct drive are subtle; a step of 0.1 skips the optimum.
   Try again between 0.0 - 0.2 with 0.01-0.02 steps. If there's still no difference between lines, increase the speed contrast in the pattern (Slow 20 / Fast 100 mm/s) — the LA effect becomes more pronounced as the speed difference grows.

**During print, M73 progress went to 100% but the print didn't finish**
→ The LA test pattern is long, the slicer estimate might be wrong. It will continue without issues.

**Extruder skips at high K (clicking sound)**
→ K is too high, extruder torque is insufficient. Decrease K or increase the `DEFAULT_EJERK` value in Configuration.h (default 5).

**M900 K is not working (old behavior returns after M900)**
→ You performed a reset without doing M500. The sequence is: M900 K... → M500 → reset.

## Quick Command Reference

```gcode
M900             ; Query the current K value
M900 K0.6        ; Set K = 0.6
M500             ; Write to EEPROM
M501             ; Read from EEPROM
M502             ; Factory defaults (returns to K=0.06)
M503             ; Show all settings
```

## Advanced: EXTRA_LIN_ADVANCE_K

If you enable the `EXTRA_LIN_ADVANCE_K` flag in Configuration_adv.h, you gain **two K value slots** (M900 T0 K... and M900 T1 K...). During a material change, you can switch slots with M900 for a quick transition. Default is OFF — if you want to activate it, say so, I can add it.

---

## Calibration Recipe Summary — TL;DR

```
1. Go to the online tool: marlinfw.org/tools/lin_advance/k-factor.html
2. Enter Sermoon parameters (table above)
3. K_start=0.0, K_end=0.3, K_step=0.02
4. Generate → copy to SD → run
5. Examine the printed pattern, take the K from the most uniform line
6. M900 K<value> + M500
7. Add M900 K<value> to your slicer start gcode
8. Celebrate 🎉
```
