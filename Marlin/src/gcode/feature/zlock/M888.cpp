/**
 * Marlin 3D Printer Firmware — Sermoon D1 fork
 *
 * M888 — Sermoon Z Lock manual control
 */

#include "../../../inc/MarlinConfig.h"

#if ENABLED(SERMOON_Z_LOCK)

#include "../../gcode.h"
#include "../../../feature/sermoon_zlock.h"

/**
 * M888: Sermoon Z Lock control
 *
 *   M888         Report current state
 *   M888 S0      Release Z lock (drive pins LOW)
 *   M888 S1      Engage Z lock (drive pins HIGH, default)
 *
 * Example:
 *   M888         -> "Z Lock: ENGAGED"
 *   M888 S0      -> "Z Lock: RELEASED"
 *
 * The set state is permanent — no automatic mechanism will undo it.
 * zlock.init() engages at boot; Afterwards it is completely manual.
 */
void GcodeSuite::M888() {
  if (parser.seenval('S')) {
    if (parser.value_bool())
      zlock.engage();
    else
      zlock.release();
  }

  SERIAL_ECHO_START();
  SERIAL_ECHOPGM("Z Lock: ");
  serialprintln_onoff(zlock.is_engaged());
}

#endif // SERMOON_Z_LOCK
