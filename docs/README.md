# Sermoon D1 Firmware — Documentation Index

This folder contains all application notes, calibration guides, and technical reference documents related to the Sermoon D1 firmware.

## Contents

### Calibration Guides

| Folder | Content |
|---|---|
| **[`pid_tuning/`](pid_tuning/README.md)** | PID auto-tuning (M303) — accurate temperature control for hotend and bed |
| **[`lin_advance/`](lin_advance/README.md)** | LIN_ADVANCE K calibration (M900) — corner quality (direct drive, low K) |
| **[`junction_deviation/`](junction_deviation/README.md)** | Junction Deviation calibration (M205 J) — modern corner-speed control |

In each folder:
- `README.md` — comprehensive procedure, troubleshooting, expected values
- `*.gcode` — test files executable from SD

### Reference

| File | Content |
|---|---|
| [`Bresenham.md`](Bresenham.md) | Explanation of the Bresenham algorithm used in Marlin step generation (original Marlin documentation) |

### Project Level

| File | Location | Content |
|---|---|---|
| [`README.md`](../README.md) | Project root | Quick start, build, hardware summary |
| [`CHANGELOG.md`](../CHANGELOG.md) | Project root | History of changes made on this fork |

## Quick Start Index

Initial setup:
1. Check hardware → [project README](../README.md#hardware)
2. Build firmware → [project README → Build](../README.md#1-build-platformio-required)
3. Flash to printer → [pre-flash backup](pid_tuning/README.md#flash-the-firmware)

Initial calibration (recommended order):
1. **PID calibration** → [pid_tuning/](pid_tuning/README.md)
2. **LIN_ADVANCE K** → [lin_advance/](lin_advance/README.md)
3. **Z offset (first layer)** → via babystep, [MANUAL.md §15.3](../MANUAL.md#153-z-offset-first-layer-calibration)
4. **Verification print** → calibration cube, all-in-one test model

> This printer **does not have a Z-probe** (neither BLTouch nor an inductive sensor is installed).
> Bed leveling is manual; `G29`/`M851` commands are not compiled.

Advanced:
- Z Lock manual control → [Marlin/Configuration_adv.h](../Marlin/Configuration_adv.h) → search for `SERMOON_Z_LOCK`
- Backported new features → [CHANGELOG.md](../CHANGELOG.md)
- Sermoon-specific feature flag list → [project README](../README.md#sermoon-specific-flags)
