/**
 * Marlin 3D Printer Firmware — Sermoon D1 fork
 *
 * Backport from Marlin 2.1.x (bug_on.h) — runtime assertion macros.
 *
 * Usage:
 *   BUG_ON("Some condition broke", value);
 *
 * Behavior:
 *   POSTMORTEM_DEBUGGING enabled → dumps + halts CPU
 *   MARLIN_DEV_MODE enabled      → dumps to serial, continues
 *   release mode                 → NOOP (zero overhead)
 */
#pragma once

#include "serial.h"

// __FILE__'in sadece dosya adını al (full path değil)
// AVR/ARM uyumlu basit makro
#ifndef ONLY_FILENAME
  #define ONLY_FILENAME __FILE__
#endif

#if ENABLED(POSTMORTEM_DEBUGGING)
  // Halt CPU on bug for debugger to inspect
  #define BUG_ON(V) do { \
    SERIAL_ECHO(ONLY_FILENAME); \
    SERIAL_ECHO(__LINE__); \
    SERIAL_ECHOLNPGM(": "); \
    SERIAL_ECHOLNPGM(V); \
    SERIAL_FLUSH(); \
    *(char*)0 = 42; \
  } while(0)
#elif ENABLED(MARLIN_DEV_MODE)
  // Dump to serial, continue execution
  #define BUG_ON(V) do { \
    SERIAL_ECHO(ONLY_FILENAME); \
    SERIAL_ECHO(__LINE__); \
    SERIAL_ECHOLNPGM(": BUG! "); \
    SERIAL_ECHOLNPGM(V); \
    SERIAL_FLUSH(); \
  } while(0)
#else
  // Release: zero overhead
  #define BUG_ON(V) NOOP
#endif
