/**
 * Marlin 3D Printer Firmware
 * Copyright (C) 2016 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * CREALITY (STM32F103) board pin assignments
 */

#ifndef __STM32F1__
  #error "Oops! Select an STM32F1 board in 'Tools > Board.'"
#endif

#if HOTENDS > 1 || E_STEPPERS > 1
  #error "CREALITY supports up to 1 hotends / E-steppers. Comment out this line to continue."
#endif

#define BOARD_NAME "CREALITY"

//
// EEPROM
//
/* I2C */
// #define I2C_EEPROM
// E2END = LAST VALID ADDRESS (not capacity). BL24C16 = 16Kbit = 2048 bytes
// => valid range 0x000..0x7FF. Marlin uses it as capacity() = E2END + 1;
// writing 0x800 would make the capacity look like 2049 and PLR_ADDR would
// spill off the chip (see Configuration.h PLR_ADDR).
#define E2END 0x7FF       // 16Kbit (24C16) — last valid address
#define MYI2C_EEPROM      // EEPROM on I2C-0
#define IIC_EEPROM_SDA       PA11
#define IIC_EEPROM_SCL       PA12

// NOTE: On this board the EEPROM is on I2C (see MYI2C_EEPROM, BL24C16
// above). SPI EEPROM and flash-emulation options do not exist on the
// V4.3.1; the unused alternative definitions were removed to avoid confusion.

//
// Limit Switches
//
#define X_MIN_PIN          PA5
// #define X_MAX_PIN          PA4
#define Y_MIN_PIN          PA6
#define Z_MIN_PIN          PA7   // Mechanical endstop — Z homing uses this

//
// Z Probe — NONE
//
// This printer has no Z-probe installed: no BLTouch, no inductive sensor.
// Z homing is done with the mechanical endstop (PA7).
//
// The PB0/PB1 pins of the on-board "BLTouch" connector are reserved for the
// Z LOCK module (see Z_KEEP_PIN_* below). That is why no probe pin is
// defined here — driving the same pin as both output (Z lock) and input
// (probe) would create a hardware conflict.
//
// If a probe is added later, SERMOON_Z_LOCK in Configuration_adv.h must be
// disabled FIRST; the two can never be enabled at the same time.
// SanityCheck.h catches this at compile time. Setup notes: MANUAL.md section 10.

//
// Steppers
//
// WARNING — SINGLE ENABLE LINE: all four drivers' EN inputs are tied to
// PC3. Marlin does not count this pin per axis; a disable_Z() call pulls
// PC3 inactive and releases X/Y/E0 as well. Not a problem in practice,
// because the axes are only disabled when all of them are idle at once
// (DEFAULT_STEPPER_DEACTIVE_TIME + all DISABLE_INACTIVE_* true). Two
// practical constraints:
//   1. Disabling a single axis independently because of heat is NOT
//      POSSIBLE. HR4988SQ (Z/E0) heat cannot be reduced in software
//      during a print; the solution is hardware cooling.
//   2. DISABLE_X/Y/Z/E in Configuration.h must all stay false;
//      setting one true brings the others down too.
#define X_ENABLE_PIN       PC3
#define X_STEP_PIN         PC2
#define X_DIR_PIN          PB9

#define Y_ENABLE_PIN       PC3
#define Y_STEP_PIN         PB8
#define Y_DIR_PIN          PB7

// Z: TWO motors are wired in PARALLEL to this SINGLE STEP/DIR/EN set (there
// is no second driver). That is why the Z2_* pin definitions and
// Z2_DRIVER_TYPE are deliberately left undefined.
#define Z_ENABLE_PIN       PC3
#define Z_STEP_PIN         PB6
#define Z_DIR_PIN          PB5

// E0 STEP/DIR = PB4/PB3 = JTAG lines. Without the DISABLE_DEBUG below these
// pins never become GPIO and the extruder never moves.
#define E0_ENABLE_PIN      PC3
#define E0_STEP_PIN        PB4
#define E0_DIR_PIN         PB3

//
// Driver types (Configuration.h *_DRIVER_TYPE) — MIXED:
//   X, Y  : TMC2208_STANDALONE
//   Z, E0 : A4988 — the physical chip is HR4988SQ. Marlin has no HR4988
//           type; A4988 is the hardware-compatible equivalent (same
//           STEP/DIR/EN, same timing). Rationale and global side effects
//           are documented in Configuration.h.
//
// Both run STANDALONE: STEP/DIR/EN only; the firmware does NOT talk to the
// drivers. No UART, so M906 (current), M569 (chop mode), M350 (microsteps)
// and sensorless homing are unavailable. Both current (Vref pot) and
// microstepping (MS1/MS2, hard-wired on the PCB — no jumpers) are set in
// HARDWARE.
//
// Z's Vref needs special attention: the two motors are wired in parallel,
// so the driver's output current is split in half.
//
// The former *_HARDWARE_SERIAL / *_SERIAL_*_PIN definitions here lived
// inside the HAS_TMC220x block; since that macro is false in standalone
// mode they never compiled. Worse, they pointed at MSerial2 (USART2) even
// though USART3 is reserved for the DWIN screen. Removed to avoid giving
// the wrong impression.
//

//
// Remap JTAG pins to GPIO. PB3 (JTDO) and PB4 (JNTRST) are used as
// E0_DIR/E0_STEP; without this definition the extruder does not work.
// NOTE: the old comment said "PB4 (Y_ENABLE_PIN)" — that was wrong,
// Y_ENABLE is PC3.
//
#define DISABLE_DEBUG

//
// Temperature Sensors
//
#define TEMP_0_PIN         PC5   // TH1
#define TEMP_BED_PIN       PC4   // TB1

//
// Heaters / Fans
//
#define HEATER_0_PIN       PA1   // HEATER1
#define HEATER_BED_PIN     PA2   // HOT BED

#define FAN_PIN            PA0   // FAN
#define FAN_SOFT_PWM

//
// Display: DWIN T5L, RTS protocol over USART3 (SERIAL_PORT_2 = 3).
// The display's own pins are NOT defined here — the driver is under
// src/lcd/dwin/.
//
// The pin maps of the 12864 character-LCD and DWIN-encoder variants used
// to sit here as comments. None of them are used on the Sermoon D1, and
// some conflicted with active pins (e.g. PA4=CHECKFILEMENT,
// PA5/PA6/PA7=endstops). Removed to eliminate the risk of accidental
// activation.
//

/* SD card detect */
#define SD_DETECT_PIN      PC7

//
// CHECKFILEMENT / Filament Runout Sensor
//
#define CHECKFILEMENT_PIN  PA4
#ifndef FIL_RUNOUT_PIN
  #define FIL_RUNOUT_PIN   PA4  // Same pin as CHECKFILEMENT_PIN (optical sensor)
#endif

/* Z轴锁定模块 — Z Lock module (SERMOON_Z_LOCK) */
// These two pins map to the on-board "BLTouch" connector. Since NO probe
// is installed, both are reserved for the Z lock and both are driven as
// OUTPUT. If a probe is added, SERMOON_Z_LOCK must be disabled first.
#define Z_KEEP_PIN_PB1   PB1  //对应板子上的IN
#define Z_KEEP_PIN_PB0   PB0  //对应板子上的OUT

