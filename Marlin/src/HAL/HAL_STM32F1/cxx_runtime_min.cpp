/**
 * Marlin 3D Printer Firmware — Sermoon D1 fork
 *
 * Minimal C++ runtime — libstdc++ bloat preventer.
 *
 * PROBLEM
 * libstdc++'s default std::terminate handler,
 * __gnu_cxx::__verbose_terminate_handler(), calls __cxa_demangle() to make
 * the TYPE NAME of an uncaught exception human-readable. That single
 * reference links libiberty's ENTIRE C++ name demangler into the binary:
 * d_print_comp (11,448 B), d_type (2,020 B), cplus_demangle_operators,
 * d_encoding, d_exprlist, d_print_mod ... 44 symbols in total.
 *
 * MEASURED (arm-none-eabi-nm, SD1-2.2 base binary):
 *   demangler family = 28,824 bytes flash -> 15.6% of the firmware
 *
 * Marlin never throws/catches exceptions anywhere; all of that code is
 * unreachable.
 *
 * SOLUTION
 * The definition below overrides libstdc++'s weak version at link time.
 * With no reference to __cxa_demangle left, the linker never pulls the
 * archive members containing the demangler.
 *
 * BEHAVIOR
 * std::terminate is effectively uncallable (no exceptions). If it ever is
 * called, a Cortex-M3 system reset is triggered.
 *
 * WHY RESET, NOT "disable interrupts + halt": disabling interrupts and
 * spinning would leave a heater pin that is HIGH at that moment HIGH
 * forever — with the soft-PWM ISR no longer running, the heater stays on
 * continuously (thermal runaway). A reset instead returns all GPIO to
 * input mode in hardware => heaters guaranteed off.
 */

#ifdef __STM32F1__

#include <stdint.h>

/**
 * LINK-TIME PRESENCE GUARD
 *
 * PROBLEM: if this file goes missing (git clean, incomplete commit, fresh
 * clone) the build still SUCCEEDS SILENTLY. Only libstdc++'s weak
 * __verbose_terminate_handler comes back, the demangler chain is re-linked
 * and the firmware grows by ~28.8 KB. No error is raised — the regression
 * goes unnoticed.
 *
 * SOLUTION: an absolute symbol is defined here; the -Wl,--require-defined=
 * in common-cxxflags.py makes that symbol MANDATORY for the link. If the
 * file is absent, the link stops with "symbol required but not defined".
 *
 * COST: .set produces an absolute symbol, reserves no space in any
 * section => 0 bytes. (Measured: firmware size unchanged after the guard
 * was added.)
 */
__asm__(".globl sermoon_cxx_runtime_min_present\n"
        ".set   sermoon_cxx_runtime_min_present, 0\n");

// SCB->AIRCR = VECTKEY(0x5FA) | SYSRESETREQ — ARMv7-M architecture
// standard, independent of the libmaple/CMSIS headers. The reset returns
// all GPIO to input mode => heaters guaranteed off.
static inline void sermoon_system_reset() {
  *(volatile uint32_t *)0xE000ED0CUL = (0x5FAUL << 16) | (1UL << 2);
  __asm__ volatile("dsb");
  for (;;) {}   // until the reset kicks in
}

namespace __gnu_cxx {
  void __verbose_terminate_handler() { sermoon_system_reset(); }
}

/**
 * NOTE — __cxa_pure_virtual is NOT defined here.
 * The maple framework already provides it (cores/maple/cxxabi-compat.cpp),
 * so that reference does not pull in libsupc++. Defining it a second time
 * here produces a "multiple definition" link error (tried, verified).
 */

#endif // __STM32F1__
