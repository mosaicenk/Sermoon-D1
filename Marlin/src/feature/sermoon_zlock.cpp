/**
 * Marlin 3D Printer Firmware — Sermoon D1 fork
 *
 * Sermoon Z Lock Module — implementation
 *
 * Bkz. sermoon_zlock.h
 */

#include "../inc/MarlinConfig.h"

#if ENABLED(SERMOON_Z_LOCK)

#include "sermoon_zlock.h"

SermoonZLock zlock;
bool SermoonZLock::engaged = false;

void SermoonZLock::init() {
  // Pinleri output moda al
  SET_OUTPUT(Z_KEEP_PIN_PB0);
  SET_OUTPUT(Z_KEEP_PIN_PB1);
  // Engage at boot (compatible with current Sermoon firmware behavior)
  engage();
}

void SermoonZLock::engage() {
  WRITE(Z_KEEP_PIN_PB0, HIGH);
  WRITE(Z_KEEP_PIN_PB1, HIGH);
  engaged = true;
}

void SermoonZLock::release() {
  WRITE(Z_KEEP_PIN_PB0, LOW);
  WRITE(Z_KEEP_PIN_PB1, LOW);
  engaged = false;
}

#endif // SERMOON_Z_LOCK
