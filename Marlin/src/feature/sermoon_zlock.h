/**
 * Marlin 3D Printer Firmware — Sermoon D1 fork
 *
 * Sermoon Z Lock Module
 *
 * PB0 (OUT) and PB1 (IN/OUT) pins on Sermoon D1 motherboard are Z axis
 * It depends on the "keep" circuit. Location of the Z axis in a closed cabinet printer
 * It prevents it from slipping under the effect of gravity.
 *
 * In the current firmware, both pins are always kept HIGH (engaged state).
 * This module streamlines control and allows manual control with M888.
 * verir. Otomatik (hareket-tetiklemeli) mod YOKTUR — bkz. Configuration_adv.h
 * SERMOON_Z_LOCK_AUTO note in it.
 *
 * Pin assignments: pins/stm32/pins_CREALITY.h
 *   #define Z_KEEP_PIN_PB0   PB0   // OUT — board IN
 *   #define Z_KEEP_PIN_PB1   PB1   // IN  — board OUT
 *
 * NOTE: The discrepancy between pin interpretations and actual behavior may be due to hardware
 * Unverified because the scheme is unknown. Default behavior current firmware
 * Same as: both pins are HIGH (engaged).
 */
#pragma once

#include "../inc/MarlinConfigPre.h"

#if ENABLED(SERMOON_Z_LOCK)

class SermoonZLock {
public:
  static void init();
  static void engage();          // Z lock aktive (her iki pin HIGH)
  static void release();         // Z lock disabled (both pins LOW)
  static bool is_engaged() { return engaged; }

private:
  static bool engaged;
};

extern SermoonZLock zlock;

#endif // SERMOON_Z_LOCK
