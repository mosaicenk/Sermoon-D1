/**
 * Marlin 3D Printer Firmware — Sermoon D1 fork
 *
 * autoreport.h — Generic auto-reporter template
 *
 * Backport from Marlin 2.1.x (libs/autoreport.h)
 *
 * Usage:
 *   struct MyReporter { static void report() { ... } };
 *   AutoReporter<MyReporter> reporter;
 *   reporter.set_interval(seconds);   // M154/M155/etc.
 *   reporter.tick();                  // periodic call from the main loop
 *
 * Sermoon-specific notes:
 *   - PORT_REDIRECT support is optional (Marlin 2.0.x has a single port)
 *   - depends on the millis() and ELAPSED() macros
 */
#pragma once

#include "../inc/MarlinConfig.h"

template <typename Helper>
struct AutoReporter {
  millis_t next_report_ms;
  uint8_t report_interval;

  AutoReporter() : next_report_ms(0), report_interval(0) {}

  inline void set_interval(uint8_t seconds, const uint8_t limit = 60) {
    report_interval = _MIN(seconds, limit);
    next_report_ms = millis() + (millis_t(seconds) * 1000UL);
  }

  inline void tick() {
    if (!report_interval) return;
    const millis_t ms = millis();
    if (ELAPSED(ms, next_report_ms)) {
      next_report_ms = ms + (millis_t(report_interval) * 1000UL);
      Helper::report();
    }
  }
};
