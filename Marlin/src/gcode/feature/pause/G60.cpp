/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Backport from Marlin 2.1.x (adapted to 2.0 API)
 */

#include "../../../inc/MarlinConfig.h"

#if SAVED_POSITIONS

#include "../../gcode.h"
#include "../../../module/motion.h"

xyze_pos_t stored_position[SAVED_POSITIONS];
uint8_t saved_slots[(SAVED_POSITIONS + 7) >> 3];

/**
 * G60: Save current position
 *   S<slot> - Memory slot # (0-based) to save into (default 0)
 */
void GcodeSuite::G60() {
  const uint8_t slot = parser.byteval('S');

  if (slot >= SAVED_POSITIONS) {
    SERIAL_ERROR_START();
    SERIAL_ECHOPAIR("Bad slot: max ", int(SAVED_POSITIONS - 1));
    SERIAL_EOL();
    return;
  }

  stored_position[slot] = current_position;
  SBI(saved_slots[slot >> 3], slot & 0x07);

  SERIAL_ECHOPAIR("Saved #", int(slot),
    " X:", stored_position[slot].x,
    " Y:", stored_position[slot].y,
    " Z:", stored_position[slot].z,
    " E:", stored_position[slot].e);
  SERIAL_EOL();
}

#endif // SAVED_POSITIONS
