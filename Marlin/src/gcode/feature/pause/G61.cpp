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
#include "../../../module/planner.h"

extern xyze_pos_t stored_position[SAVED_POSITIONS];
extern uint8_t saved_slots[(SAVED_POSITIONS + 7) >> 3];

/**
 * G61: Return to a saved position
 *   F<rate>   - Feedrate (optional) for the move back
 *   S<slot>   - Slot # (0-based) to restore from (default 0)
 *   X<offset> - Restore X axis with optional offset (default 0)
 *   Y<offset> - Restore Y axis with optional offset (default 0)
 *   Z<offset> - Restore Z axis with optional offset (default 0)
 *   E<offset> - Restore E axis with optional offset (default 0)
 *
 *   If no axes specified, all axes restored.
 */
void GcodeSuite::G61() {
  const uint8_t slot = parser.byteval('S');

  if (slot >= SAVED_POSITIONS) {
    SERIAL_ERROR_START();
    SERIAL_ECHOPAIR("Bad slot: max ", int(SAVED_POSITIONS - 1));
    SERIAL_EOL();
    return;
  }

  // No saved position?
  if (!TEST(saved_slots[slot >> 3], slot & 0x07)) {
    SERIAL_ERROR_MSG("Slot empty");
    return;
  }

  // Optional feedrate
  const float saved_fr = feedrate_mm_s;
  const float fr = parser.linearval('F');
  if (fr > 0.0f) feedrate_mm_s = MMM_TO_MMS(fr);

  // Determine which axes to restore
  const bool seen_xyz = parser.seen('X') || parser.seen('Y') || parser.seen('Z');
  const bool seen_e   = parser.seen('E');

  if (!seen_xyz && !seen_e) {
    // Default: restore all axes
    do_blocking_move_to(stored_position[slot], feedrate_mm_s);
    current_position.e = stored_position[slot].e;
    planner.set_e_position_mm(current_position.e);
  }
  else {
    if (seen_xyz) {
      destination = current_position;
      if (parser.seen('X')) destination.x = stored_position[slot].x + parser.value_linear_units();
      if (parser.seen('Y')) destination.y = stored_position[slot].y + parser.value_linear_units();
      if (parser.seen('Z')) destination.z = stored_position[slot].z + parser.value_linear_units();
      prepare_move_to_destination();
    }
    if (seen_e) {
      current_position.e = stored_position[slot].e + parser.value_linear_units();
      planner.set_e_position_mm(current_position.e);
    }
  }

  feedrate_mm_s = saved_fr;
}

#endif // SAVED_POSITIONS
