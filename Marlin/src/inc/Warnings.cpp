/**
 * Marlin 3D Printer Firmware — Sermoon D1 fork
 *
 * Warnings.cpp — Compile-time warnings for non-fatal config issues
 *
 * Backport from Marlin 2.1.x. Warning'ler config seçimleri ile ilgili
 * dikkat edilmesi gereken noktalar için (hata değil, bilgi/öneri).
 *
 * Compile-time mesajlar #pragma message ile yayınlanır — derleme devam eder.
 */

// MarlinConfig.h zaten Changes.h'ı çekti — burada sadece warning'ler.
#include "MarlinConfig.h"

//
// Sermoon-spesifik durumlar
//

#if !defined(STM32_XL_DENSITY)
  #pragma message ">>> Note: Sermoon D1 V4.3.1 uses STM32F103RET6 (XL_DENSITY)."
#endif

#if !ENABLED(EEPROM_SETTINGS)
  #pragma message ">>> Warning: EEPROM_SETTINGS disabled — M500/M501 won't persist values."
#endif

#if !ENABLED(EEPROM_PLR)
  #pragma message ">>> Warning: EEPROM_PLR disabled — power-loss recovery EEPROM cache off."
#endif

#if !ENABLED(POWER_LOSS_RECOVERY)
  #pragma message ">>> Note: POWER_LOSS_RECOVERY disabled."
#endif

#if !ENABLED(THERMAL_PROTECTION_HOTENDS) || !ENABLED(THERMAL_PROTECTION_BED)
  #pragma message ">>> WARNING: Thermal protection partially disabled — fire risk!"
#endif

#if ENABLED(BANG_MAX) && !ENABLED(PIDTEMP)
  #pragma message ">>> Note: Hotend uses bang-bang control. PIDTEMP recommended for stability."
#endif

#if !ENABLED(USE_WATCHDOG)
  #pragma message ">>> Warning: USE_WATCHDOG disabled — firmware lockup may not be detected."
#endif

// (Filament diameter check removed: preprocessor cannot compare floats)

#if ENABLED(JUNCTION_DEVIATION) && !ENABLED(CLASSIC_JERK)
  #pragma message ">>> Note: JUNCTION_DEVIATION active. Default 0.013 — calibrate per docs/junction_deviation/."
#endif

#if defined(SOFT_PWM_SCALE) && SOFT_PWM_SCALE < 4
  #pragma message ">>> Note: SOFT_PWM_SCALE < 4 may cause audible fan PWM whine on Sermoon (PA0)."
#endif

#if ENABLED(MARLIN_DEV_MODE)
  #pragma message ">>> WARNING: MARLIN_DEV_MODE enabled — debug build, not for production!"
#endif

#if ENABLED(POSTMORTEM_DEBUGGING)
  #pragma message ">>> WARNING: POSTMORTEM_DEBUGGING enabled — assertions will halt CPU!"
#endif

//
// LIN_ADVANCE K kalibrasyon hatırlatması
//
#if ENABLED(LIN_ADVANCE)
  #pragma message ">>> Note: LIN_ADVANCE active. Direct drive Sermoon için K kalibre et (typ 0.02-0.15). See docs/lin_advance/."
#endif

//
// JUNCTION_DEVIATION + CLASSIC_JERK uyarısı (sadece biri aktif olmalı)
//
#if ENABLED(CLASSIC_JERK) && defined(JUNCTION_DEVIATION_MM)
  #pragma message ">>> Warning: CLASSIC_JERK and JUNCTION_DEVIATION_MM both defined — only one should be active."
#endif

//
// PRINTCOUNTER stat
//
#if !ENABLED(PRINTCOUNTER) && ENABLED(PRINTJOB_TIMER_AUTOSTART)
  #pragma message ">>> Note: PRINTJOB_TIMER active but PRINTCOUNTER disabled — print stats won't be saved."
#endif
