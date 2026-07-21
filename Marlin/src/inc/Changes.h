/**
 * Marlin 3D Printer Firmware — Sermoon D1 fork
 *
 * Changes.h — Compile-time deprecated configuration detection
 *
 * Backport from Marlin 2.1.x. Eski isim kullanan kullanıcı configurations'ı
 * uyarır ve yeni isimlere yönlendirir.
 *
 * Bu dosya Sermoon-relevant subset içerir; tüm Marlin tarihsel rename'leri
 * değil. Sermoon kullanıcısının likely yapacağı hatalara odaklı.
 */
#pragma once

//
// Configuration name renames (Marlin 2.0.x → 2.1.x)
//

#ifdef ABL_GRID_POINTS
  #error "ABL_GRID_POINTS is now GRID_MAX_POINTS_X and GRID_MAX_POINTS_Y."
#endif

#ifdef ABL_GRID_POINTS_X
  #error "ABL_GRID_POINTS_X is now GRID_MAX_POINTS_X."
#endif

#ifdef ABL_GRID_POINTS_Y
  #error "ABL_GRID_POINTS_Y is now GRID_MAX_POINTS_Y."
#endif

#ifdef BEEPER
  #error "BEEPER is now BEEPER_PIN."
#endif

#ifdef SDCARDDETECT
  #error "SDCARDDETECT is now SD_DETECT_PIN."
#endif

#ifdef SDCARDDETECTINVERTED
  #error "SDCARDDETECTINVERTED is now SD_DETECT_INVERTED (or SD_DETECT_STATE in modern Marlin)."
#endif

// NOT: Sermoon stock kodu hâlâ SD_DETECT_INVERTED kullanır (modern adı
// SD_DETECT_STATE). Code tabanı geçişine kadar bu rename forced değil.

#ifdef BTENABLED
  #error "BTENABLED is now BLUETOOTH."
#endif

#ifdef CUSTOM_MENDEL_NAME
  #error "CUSTOM_MENDEL_NAME is now CUSTOM_MACHINE_NAME."
#endif

#ifdef HAS_AUTOMATIC_VERSIONING
  #error "HAS_AUTOMATIC_VERSIONING is now CUSTOM_VERSION_FILE."
#endif

#ifdef USE_AUTOMATIC_VERSIONING
  #error "USE_AUTOMATIC_VERSIONING is now CUSTOM_VERSION_FILE."
#endif

#ifdef SDSLOW
  #error "SDSLOW deprecated. Set SD_SPI_SPEED to SPI_HALF_SPEED instead."
#endif

#ifdef SDEXTRASLOW
  #error "SDEXTRASLOW deprecated. Set SD_SPI_SPEED to SPI_QUARTER_SPEED instead."
#endif

#ifdef FILAMENT_SENSOR
  #error "FILAMENT_SENSOR is now FILAMENT_WIDTH_SENSOR."
#endif

#ifdef ENDSTOPPULLUP_FIL_RUNOUT
  #error "ENDSTOPPULLUP_FIL_RUNOUT is now FIL_RUNOUT_PULLUP."
#endif

#ifdef DISABLE_MAX_ENDSTOPS
  #error "DISABLE_MAX_ENDSTOPS deprecated. Use individual USE_*MAX_PLUG options instead."
#endif

#ifdef DISABLE_MIN_ENDSTOPS
  #error "DISABLE_MIN_ENDSTOPS deprecated. Use individual USE_*MIN_PLUG options instead."
#endif

#ifdef LANGUAGE_INCLUDE
  #error "LANGUAGE_INCLUDE has been replaced by LCD_LANGUAGE."
#endif

#ifdef EXTRUDER_OFFSET_X
  #error "EXTRUDER_OFFSET_X is deprecated. Use HOTEND_OFFSET_X instead."
#endif

#ifdef EXTRUDER_OFFSET_Y
  #error "EXTRUDER_OFFSET_Y is deprecated. Use HOTEND_OFFSET_Y instead."
#endif

#ifdef PID_PARAMS_PER_EXTRUDER
  #error "PID_PARAMS_PER_EXTRUDER is deprecated. Use PID_PARAMS_PER_HOTEND instead."
#endif

#ifdef EXTRUDER_WATTS
  #error "EXTRUDER_WATTS is deprecated and should be removed."
#endif

#ifdef BED_WATTS
  #error "BED_WATTS is deprecated and should be removed."
#endif

#ifdef SERVO_ENDSTOP_ANGLES
  #error "SERVO_ENDSTOP_ANGLES is deprecated. Use Z_SERVO_ANGLES instead."
#endif

#ifdef X_ENDSTOP_SERVO_NR
  #error "X_ENDSTOP_SERVO_NR is deprecated and should be removed."
#endif

#ifdef Y_ENDSTOP_SERVO_NR
  #error "Y_ENDSTOP_SERVO_NR is deprecated and should be removed."
#endif

#ifdef Z_ENDSTOP_SERVO_NR
  #error "Z_ENDSTOP_SERVO_NR is now Z_PROBE_SERVO_NR."
#endif

#ifdef DEFAULT_XYJERK
  #error "DEFAULT_XYJERK is deprecated. Use DEFAULT_XJERK and DEFAULT_YJERK instead."
#endif

#ifdef XY_TRAVEL_SPEED
  #error "XY_TRAVEL_SPEED is now XY_PROBE_FEEDRATE."
#endif

#ifdef Z_HOMING_HEIGHT_BEFORE_HOMING
  #error "Z_HOMING_HEIGHT_BEFORE_HOMING is now Z_HOMING_HEIGHT."
#endif

#ifdef Z_RAISE_BEFORE_HOMING
  #error "Z_RAISE_BEFORE_HOMING is now Z_HOMING_HEIGHT."
#endif

#ifdef PROBE_OFFSET_FROM_EXTRUDER
  #error "*_PROBE_OFFSET_FROM_EXTRUDER are deprecated. Use NOZZLE_TO_PROBE_OFFSET instead."
#endif

#ifdef X_PROBE_OFFSET_FROM_EXTRUDER
  #error "X_PROBE_OFFSET_FROM_EXTRUDER is deprecated. Use NOZZLE_TO_PROBE_OFFSET { X, Y, Z } instead."
#endif

#ifdef Y_PROBE_OFFSET_FROM_EXTRUDER
  #error "Y_PROBE_OFFSET_FROM_EXTRUDER is deprecated. Use NOZZLE_TO_PROBE_OFFSET { X, Y, Z } instead."
#endif

#ifdef Z_PROBE_OFFSET_FROM_EXTRUDER
  #error "Z_PROBE_OFFSET_FROM_EXTRUDER is deprecated. Use NOZZLE_TO_PROBE_OFFSET { X, Y, Z } instead."
#endif

//
// Sermoon-specific reminders
//

#if defined(BOARD_CREALITY) && !defined(STM32_XL_DENSITY)
  #warning "Sermoon D1 V4.3.1 is XL_DENSITY (STM32F103RET6). Build with creality env."
#endif
