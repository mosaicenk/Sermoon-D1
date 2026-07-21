/**
 * Marlin 3D Printer Firmware — Sermoon D1 fork
 *
 * M888 — Sermoon Z Lock manuel kontrol
 */

#include "../../../inc/MarlinConfig.h"

#if ENABLED(SERMOON_Z_LOCK)

#include "../../gcode.h"
#include "../../../feature/sermoon_zlock.h"

/**
 * M888: Sermoon Z Lock kontrol
 *
 *   M888         Mevcut durumu raporla
 *   M888 S0      Z lock release (pinleri LOW yap)
 *   M888 S1      Z lock engage (pinleri HIGH yap, default)
 *
 * Örnek:
 *   M888         -> "Z Lock: ENGAGED"
 *   M888 S0      -> "Z Lock: RELEASED"
 *
 * NOT: SERMOON_Z_LOCK_AUTO etkinse otomatik engage/release Z hareketleri
 * sırasında devreye girer. M888 ile manuel override mümkün; bir sonraki
 * Z hareketine kadar geçerlidir.
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
