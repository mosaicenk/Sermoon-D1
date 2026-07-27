# Sermoon D1 — Junction Deviation Calibration Guide

Switched from CLASSIC_JERK to **JUNCTION_DEVIATION**. This document covers:
1. What JD is and why we switched
2. Calibration procedure
3. Reverting if you are not satisfied with the results

## JD vs Classic Jerk — Quick Comparison

| Feature | Classic Jerk | Junction Deviation |
|---|---|---|
| Number of parameters | 4 (X/Y/Z/E separate) | 1 (`JUNCTION_DEVIATION_MM`) |
| Calculation model | Step velocity delta | Physics-based geometric cornering |
| Considers corner angle | No | Yes (sin θ formula) |
| Axis-to-axis consistency | Manual adjustment | Automatic |
| Interaction with acceleration | Independent | Combined (`v² = δ·a/...`) |
| Modern Marlin recommendation | ✗ Legacy | ✓ Recommended (Marlin 2.0+) |
| Calibration difficulty | Medium-high (4 values) | Easy (1 value) |

JD mathematics:
```
v_max_corner = √( JUNCTION_DEVIATION_MM × acceleration × (1/sin(θ/2) - 1)⁻¹ )
```

θ = change in corner angle. Sharp corner (large θ) = low v_max; straight continuation (θ ≈ 0) = unlimited speed.

## Current Setting

`Configuration.h:849`:
```c
#if DISABLED(CLASSIC_JERK)
  #define JUNCTION_DEVIATION_MM 0.015   // (mm) Default — Default for Cartesian Sermoon D1
#endif
```

## Calibration Range Table

Typical values for Sermoon D1 (direct drive, enclosed chamber):

| δ (mm) | Character | Print Time | Quality |
|---|---|---|---|
| 0.003 | Overly tight | +15% slower | Excellent but unnecessary |
| 0.005 | Very tight | +10% | High detail |
| 0.008 | Tight | +5% | Precise geometries, sharp corners |
| 0.013 | Production balance | Baseline | Good balance |
| **0.015** | **DEFAULT** | Fast | High speed and performance balance |
| 0.020 | Loose | -5% faster | Slight rounding |
| 0.025 | Very loose | -10% | Noticeable rounding |
| > 0.030 | Not recommended | — | Risk of corner overshoot |

## Calibration Procedure

### 1. Baseline Print
Do a reference print (calibration cube, all-in-one test) with the current 0.013. Inspect the corners **with a microscope / magnifying glass**.

### 2. Run Test Gcode

Copy `docs/junction_deviation/jd_test.gcode` to the SD card and run it. This file runs a test containing rapid direction changes — you can observe the corner behavior.

### 3. Adjusting the Value

Runtime change:
```gcode
M205 J0.008    ; Set JD = 0.008 (tight test)
M500           ; Save to EEPROM
```

Permanent code change (`Configuration.h:756`):
```c
#define JUNCTION_DEVIATION_MM 0.008
```

### 4. Sweep Test (best method)

Print the same model 3 times with different JD values:
- Print 1: `M205 J0.008` → label the model "0.008" when finished
- Print 2: `M205 J0.013` → "0.013"
- Print 3: `M205 J0.020` → "0.020"

Place the 3 models side by side, look at the **corners** and **curves**:
- Which one is the most stable?
- Which one has overshoot?
- Which one is too slow but lacks necessary detail?

Generally, the 0.013 baseline yields good results — unless there is a problem worth changing, do not touch it.

## M205 Command (Runtime Change)

```gcode
M205             ; Show all current speed settings
M205 J0.013      ; Set JD
M205 X<v> Y<v>   ; Classic jerk values (if CLASSIC_JERK is present)
M205 S<v>        ; Min print speed
M205 T<v>        ; Min travel speed
M500             ; Save to EEPROM
M501             ; Read from EEPROM
```

## Expected Behavioral Differences

### When it was CLASSIC_JERK you used to see:
- Distinct "clunking" sound in some corners (high jerk → sudden speed change)
- Smooth on large curves
- Overshoot on small details
- Separate control for extruder via E-jerk (DEFAULT_EJERK 5)

### With JUNCTION_DEVIATION you are likely to see:
- All corners behave **consistently with each other** (automatic angle-based)
- Faster for large angles (no unnecessary slowdown)
- Smarter slowdown for small angles
- DEFAULT_EJERK is still used for the extruder (LIN_ADVANCE)

## Reverting

If you do not like JD, revert to classic jerk:

`Configuration.h:744`:
```c
//#define CLASSIC_JERK    ← uncomment this

#define CLASSIC_JERK      ← make it look like this
#if ENABLED(CLASSIC_JERK)
  #define DEFAULT_XJERK 10.0
  ...
```

Then:
```powershell
pio run -e creality
# re-flash
```

Alternative: Manual jerk value setting via runtime over EEPROM (**does not work** because CLASSIC_JERK requires build-time — code change is mandatory).

## Known Tradeoffs

1. **DEFAULT_EJERK remains**: LIN_ADVANCE uses this. JD does not change it.
2. **Concept of Z jerk disappears**: JD works for Z (Sermoon Z movements are already slow, it won't be a problem).
3. **Print time difference**: At average speeds for Sermoon (50-80 mm/s), a +/-5% difference is expected.
4. **EEPROM compatibility**: Jerk values in existing M500 records are **ignored** in the new firmware. Reset with M502 + M500 if not needed.

## Quick Command Reference

```gcode
M205 J0.013      ; Set JD at runtime
M205             ; Show current JD/jerk values
M500 / M501      ; Save / load
M502 + M500      ; Factory + Write to EEPROM
M503             ; All settings — JD line appears in M205
```

## Verification

After flashing the new firmware:
```gcode
M115             ; Version: SD1-1.0
M503             ; "M205 J0.013" should appear in the output
                 ; (If it were CLASSIC_JERK, "M205 X10 Y10 Z0.4 E5" would appear)
```
