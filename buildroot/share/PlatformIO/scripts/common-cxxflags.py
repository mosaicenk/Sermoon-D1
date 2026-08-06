#
# common-cxxflags.py
# Convenience script to apply customizations to CPP flags
#
Import("env")

# ---------------------------------------------------------------------------
# LINK stage — newlib-nano
#
# Measurement: the link command did NOT contain --specs=nano.specs (verified
# with pio run -v), so we were linking against FULL newlib. Result:
# _svfprintf_r + _dtoa_r + _strtod_l + _mprec family 16,088 bytes, newlib
# malloc 2,820 bytes.
#
# These flags were already written in the SCons 'else:' branch of
# Marlin/src/HAL/HAL_STM32F1/build_flags.py, but that file is not listed in
# extra_scripts, so the branch NEVER ran — the below makes the original
# intent actually effective.
#
# -u_printf_float is MANDATORY: nano.specs printf has no %f support by
# default. Marlin uses dtostrf() (power-loss recovery G-code generation,
# M114 position report, LCD_RTS pause screen) and dtostrf works via
# sprintf("%*.*f"). Without this flag those values print garbage.
# ---------------------------------------------------------------------------
# cxx_runtime_min.cpp PRESENCE GUARD
#
# If that file goes missing, the build still succeeds silently and the
# firmware grows by ~28.8 KB (libstdc++'s weak __verbose_terminate_handler
# comes back, which re-links the whole name demangler via __cxa_demangle).
# The --require-defined below mandates that the symbol is DEFINED: if the
# file is absent the link STOPS with "symbol ... required but not defined".
# The symbol is defined absolute via .set => 0 byte cost.
#
# NOT --undefined: that option only registers the symbol as "undefined"
# (its purpose is pulling a module from an archive) and does NOT error if
# it stays unresolved. Measured: with --undefined the link completed
# successfully after the file was deleted — the guard silently did nothing.
# --require-defined is the correct option.
env.Append(LINKFLAGS=[
  "--specs=nano.specs",
  "-u_printf_float",
  "-Wl,--require-defined=sermoon_cxx_runtime_min_present"
])

env.Append(CXXFLAGS=[
  "-Wno-register",

  # Marlin never uses throw/catch or dynamic_cast/typeid anywhere.
  # Without these flags GCC emits an unwind table for every function and
  # links libsupc++'s exception machinery (_Unwind_*, __gxx_personality_v0,
  # __cxa_*) into the binary.
  #
  # MEASURED (arm-none-eabi-nm): EH/unwind family = 5,838 bytes flash, 59 symbols.
  #
  # Placed in CXXFLAGS, NOT build_flags: build_flags also applies to C files
  # and GCC warns "valid for C++ but not for C" there.
  "-fno-exceptions",
  "-fno-rtti",
  "-fno-unwind-tables",
  "-fno-asynchronous-unwind-tables",

  # Prevents __cxa_guard_acquire/release emission for function-local statics.
  # Marlin is single-threaded (ISRs do no C++ static init), so these locks
  # are unnecessary. Worse, pulling the guard functions from libsupc++ also
  # dragged in the exception personality routine.
  "-fno-threadsafe-statics",

  # Registers global destructors with atexit instead of __cxa_atexit. On the
  # embedded target main() never returns; destructors never run anyway.
  #
  # NOTE: These two flags are also defined in the SCons 'else:' branch of
  # Marlin/src/HAL/HAL_STM32F1/build_flags.py, BUT that branch never runs —
  # the file is only invoked via "!python ..." to produce stdout, and since
  # it is not listed in extra_scripts its Import("env") branch is dead code.
  # They were moved here to take effect.
  "-fno-use-cxa-atexit"
  #"-Wno-incompatible-pointer-types",
  #"-Wno-unused-const-variable",
  #"-Wno-maybe-uninitialized",
  #"-Wno-sign-compare"
])
