/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2019 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

/**
 * servo.h - Interrupt driven Servo library for Arduino using 16 bit timers- Version 2
 * Copyright (c) 2009 Michael Margolis.  All right reserved.
 *
 * Sermoon D1: Only STM32F1 (Creality V4.3.1) is targeted.
 */

#if defined(__STM32F1__) || defined(TARGET_STM32F1)
  #include "../HAL_STM32F1/Servo.h"
#else
  #error "Sermoon D1 firmware targets STM32F1 only."
#endif
