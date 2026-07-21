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
 * Backport from Marlin 2.1.x
 */

#include "../../inc/MarlinConfig.h"

#if ENABLED(GCODE_REPEAT_MARKERS)

#include "../gcode.h"
#include "../../feature/repeat.h"

/**
 * M808: Set / Goto a repeat marker
 *
 *  L<count> - Set a repeat marker with 'count' repetitions. If omitted, infinity.
 *  K        - Cancel all loops
 *
 * Examples:
 *    M808 L     ; Set a loop marker with a count of infinity
 *    M808 L2    ; Set a loop marker with a count of 2
 *    M808       ; Decrement and loop if not zero
 *    M808 K     ; Cancel loops
 */
void GcodeSuite::M808() {
  // Handled early in queue.cpp; this is just for the dispatch logging.
  // M808 K cancels all loops.
  if (parser.seen('K')) repeat.cancel();
}

#endif // GCODE_REPEAT_MARKERS
