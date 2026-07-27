# Sermoon D1 — PID Auto-Tuning Guide

PID (Proportional-Integral-Derivative) tuning calibrates the control coefficients to keep the hotend and bed temperatures **stable** at the target value.

In this firmware:
- Hotend PID: ✅ active (`PIDTEMP`)
- Bed PID: ✅ active (`PIDTEMPBED`)
- Auto tuning: ✅ active (`M303` + `PID_AUTOTUNE_MENU`)
- Manual editing: ✅ active (`PID_EDIT_MENU` + `M301`/`M304`)

## Why is Tuning Mandatory?

The **default values** located in Configuration.h are general-purpose; they are not specific to the Sermoon hardware:

```c
// Hotend (Ultimaker reference values)
DEFAULT_Kp = 21.73, Ki = 1.54, Kd = 76.55

// Bed (250W silicone reference)
DEFAULT_bedKp = 327.11, bedKi = 19.20, bedKd = 1393.45
```

The 24V 40W hotend and ~200W bed of the Sermoon D1 do not share the same **thermal mass/time constant** profile as these references. The results:
- **Excessive oscillation** (target ±5°C fluctuation)
- **Overshoot** (set 200°C → measuring 207°C)
- **Slow settling** (set 200°C → stuck at 195°C for 30 sec)
- **Bang-bang behavior** (PID ineffective, acting like classic on/off)

After proper tuning:
- Holding within **±0.5°C** of the target temperature
- Overshoot < 2°C
- Reduction of temperature-dependent artifacts in print quality

## Tuning Procedure

### 1. Prepare the Printer

```
✓ Printer completely cold (at room temperature, ~25°C)
✓ At least 30 minutes passed since the last print finished (uniform thermal equilibrium)
✓ Chamber enclosure closed (same as actual print conditions)
✓ SD card inserted
```

### 2. Hotend Tuning (8-12 minutes)

**Method A — From SD card:**
1. Copy the `pid_hotend.gcode` file to the root of the SD card
2. Boot the printer
3. From the screen: "Print" → select `pid_hotend.gcode`
4. Wait (the printer will heat to 210°C, perform ~8 cycles of oscillation)
5. Result appears on screen + automatically saved to EEPROM

**Method B — From Host (OctoPrint/PrusaSlicer console):**
```gcode
M106 S128                ; Part fan at 50% — simulates actual print load
M303 E0 S210 C8 U1       ; Tune + apply
M500                     ; Save
M107                     ; Fan off
```

**Customizing parameters:**
- `S210` — the filament temperature you use (PLA: 195-210, PETG: 230, ABS: 240)
- `C8` — cycle count (3 minimum, 5 sufficient, 8 high quality)
- `U1` — auto-apply. If omitted, it only reports.

### 3. Bed Tuning (25-40 minutes)

```gcode
M303 E-1 S60 C5 U1       ; 60°C, 5 cycles (for PLA)
M500
```

For ABS use `S100`, for PETG `S80`.

### 4. Verification

After tuning finishes:

```gcode
M501                     ; Load from EEPROM (to be sure)
M503                     ; Show all settings
```

Look for the `M301` (hotend) and `M304` (bed) lines in the `M503` output:

```
echo:; PID settings:
echo:  M301 P21.73 I1.54 D76.55      ← PRE-TUNE (default)
echo:  M304 P327.11 I19.20 D1393.45  ← PRE-TUNE
```

vs. post-tune (Sermoon-specific examples):
```
echo:  M301 P14.48 I0.92 D56.92      ← Sermoon hotend (actual print result example)
echo:  M304 P145.83 I26.84 D659.13   ← Sermoon bed
```

### 5. Print Test

Print a calibration cube or a thermal-calibration basic shape.
Observe the temperature graph in the console/host:
- Set 200°C → the graph should be a flat line within the 200 ± 0.5°C band
- Bed set 60°C → graph 60 ± 1°C

## Expected Value Ranges (Sermoon D1)

| Parameter | Typical range | Pathological |
|---|---|---|
| Hotend Kp | 12 - 28 | <5 or >50 → wrong sensor/heater |
| Hotend Ki | 0.5 - 2.0 | >5 → too aggressive, oscillation |
| Hotend Kd | 30 - 90 | <10 or >200 → heater response issue |
| Bed Kp | 50 - 250 | <20 → too slow, >500 → oscillation |
| Bed Ki | 5 - 30 | — |
| Bed Kd | 200 - 1500 | — |

If values fall **outside** these ranges:
1. Check thermistor connection
2. Check heater wire integrity
3. Change tuning temperature and retry
4. Increase cycle count (`C10`)

## Writing to Configuration.h (Optional)

EEPROM saving is sufficient — but if you recompile the firmware, it will be lost after `M502` (factory reset). To make it a permanent default, update it in Configuration.h:

```c
// Sermoon D1 (personal calibration, YYYY-MM-DD)
#define DEFAULT_Kp    14.48
#define DEFAULT_Ki     0.92
#define DEFAULT_Kd    56.92

#define DEFAULT_bedKp  145.83
#define DEFAULT_bedKi   26.84
#define DEFAULT_bedKd  659.13
```

Then rebuild + reflash with `pio run -e creality`. This step is optional, EEPROM is sufficient for most users.

## Troubleshooting

**"PID Autotune failed! Bad extruder number"**
→ E parameter is wrong. Use `E0` for hotend, `E-1` for bed.

**"PID Autotune failed! temperature too high"**
→ Target temperature is too high (>HEATER_0_MAXTEMP). Lower the S value.

**"PID Autotune failed! Timeout"**
→ Heater connection issue, faulty heater or thermistor, or thermal protection kicked in. Check the hardware.

**Temperature doesn't drop during tuning (cooling phase)**
→ Part cooling fan is off; turn it on with M106 S128. Or missing hotend silicone sock causes the temperature to hold too aggressively.

**Results were not saved to EEPROM**
→ You forgot M500 or the EEPROM write failed. Verify with M503. If there is an EEPROM error, reset with M502, then try again.

**Heating is slow after tuning**
→ Normal — PID now heats with more control. It doesn't output full power like bang-bang. It will still heat up sufficiently fast for printing.

## When to Tune Again?

- When a heater or thermistor is replaced
- When the hotend type is changed (e.g. switching to an all-metal hotend)
- New spectrum of materials (e.g. PLA only → PETG/ABS)
- When temperature-dependent issues are observed in print quality
- As an annual maintenance check (due to mechanical wear)

## Advanced: Material-Based PID

A single set of values serves as an average for both PLA and ABS. If you want material-specific control, you can put it in your **slicer start gcode**:

```gcode
; PLA start gcode example
M301 P14.48 I0.92 D56.92  ; Optimized PID for PLA

; ABS start gcode example
M301 P12.95 I0.85 D49.21  ; ABS requires a lower thermal mass
```

Tune separately with each material and save the results to the slicer.

---

## Quick Reference — Calibration Commands

```gcode
M303 E0 S210 C8 U1   ; Hotend tune (8 cycles, apply)
M303 E-1 S60 C5 U1   ; Bed tune
M500                 ; Save to EEPROM
M501                 ; Load from EEPROM
M502                 ; Factory reset (default values)
M503                 ; Show all settings
M301 P14 I0.92 D57   ; Manual hotend PID set
M304 P150 I27 D660   ; Manual bed PID set
```
