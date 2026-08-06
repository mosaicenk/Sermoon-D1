/**
 * Marlin 3D Printer Firmware — Sermoon D1 fork
 *
 * Sermoon Z Lock Module
 *
 * The PB0 (OUT) and PB1 (IN/OUT) pins on the Sermoon D1 motherboard are
 * wired to the Z axis "keep" circuit. In an enclosed-cabinet printer they
 * prevent the Z axis from slipping under gravity.
 *
 * In the stock firmware both pins are held HIGH at all times (engaged
 * state). This module makes the control explicit and adds manual control
 * via M888. There is NO automatic (motion-triggered) mode.
 *
 * Pin assignments: pins/stm32/pins_CREALITY.h
 *   #define Z_KEEP_PIN_PB0   PB0   // OUT — board IN
 *   #define Z_KEEP_PIN_PB1   PB1   // IN  — board OUT
 *
 * NOTE: The naming mismatch between the pin comments and actual behavior
 * is unverified — the hardware schematic is unavailable. Default behavior
 * matches the stock firmware: both pins HIGH (engaged).
 */
#pragma once

#include "../inc/MarlinConfigPre.h"

#if ENABLED(SERMOON_Z_LOCK)

class SermoonZLock {
public:
  static void init();
  static void engage();          // Engage Z lock (both pins HIGH)
  static void release();         // Z lock disabled (both pins LOW)
  static bool is_engaged() { return engaged; }

private:
  static bool engaged;
};

extern SermoonZLock zlock;

#endif // SERMOON_Z_LOCK
