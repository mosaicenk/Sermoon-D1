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

#include "../inc/MarlinConfig.h"

#if ENABLED(GCODE_REPEAT_MARKERS)

#include "repeat.h"

#include "../gcode/gcode.h"
#include "../sd/cardreader.h"

repeat_marker_t Repeat::marker[MAX_REPEAT_NESTING];
uint8_t Repeat::index;

void Repeat::add_marker(const uint32_t sdpos, const uint16_t count) {
  if (index >= MAX_REPEAT_NESTING)
    SERIAL_ECHO_MSG("!Too many markers.");
  else {
    marker[index].sdpos = sdpos;
    marker[index].counter = count ? count - 1 : -1;
    index++;
  }
}

void Repeat::loop() {
  if (!index)
    SERIAL_ECHO_MSG("!No marker set.");
  else {
    const uint8_t ind = index - 1;
    if (!marker[ind].counter) {
      index--;
    }
    else {
      card.setIndex(marker[ind].sdpos);
      if (marker[ind].counter > 0)
        --marker[ind].counter;
    }
  }
}

void Repeat::cancel() { for (uint8_t i = 0; i < index; ++i) marker[i].counter = 0; }

void Repeat::early_parse_M808(char * const cmd) {
  if (is_command_M808(cmd)) {
    parser.parse(cmd);
    if (parser.seen('L'))
      add_marker(card.getIndex(), parser.value_ushort());
    else
      Repeat::loop();
  }
}

#endif // GCODE_REPEAT_MARKERS
