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

////////////////////////////
// VENDOR VERSION EXAMPLE //
////////////////////////////

#define	SOFTVERSION		SHORT_BUILD_VERSION

/**
 * Marlin release version identifier
 *
 * Base: Stock Creality Sermoon D1 V1.1.10 (Marlin 2.0.x bugfix branch)
 * Custom fork build identifier: Sermoon-D1-X.Y
 *
 * NOTE: The DWIN screen fills the FW_VERSION_TEXT_VP slot with SOFTVERSION
 * (this value); on-screen space is limited to ~14 characters, longer strings
 * get clipped.
 */
// ###########################################################################
// # NOTE: This file is included via macro expansion (MarlinConfigPre.h:42   #
// #   #include XSTR(../../CUSTOM_VERSION_FILE)), so the SCons C scanner     #
// # cannot resolve the include path and Version.h is NOT in the dependency  #
// # graph. buildroot/share/PlatformIO/scripts/version-stamp.py injects this #
// # file's content hash as a build flag, forcing a full rebuild on change   #
// # (the old manual `rm -rf .pio/build/creality` step, automated).          #
// # Measured before the fix (2026-07-27): an incremental `pio run` compiled #
// # 0 units and the binary kept the OLD version string silently.            #
// ###########################################################################
//
// Must stay in sync with the fork version: the CHANGELOG.md header and the
// README "Version" row are updated together with this value. Since M115 and
// the DWIN screen print this string, it is the only way to tell which fork
// build is installed on a printer in the field.
//
// The old value "MarlinV2 by CTK" was 15 characters — over the ~14 character
// DWIN slot, so it clipped on screen, and it said nothing about which fork
// version was installed. Attribution moved to DETAILED_BUILD_VERSION.
#define SHORT_BUILD_VERSION "SD1-3.2"

/**
 * Verbose version identifier — Shown in M115 response.
 */
#define DETAILED_BUILD_VERSION SHORT_BUILD_VERSION " (Sermoon D1 by CTK, base V1.1.10)"

/**
 * The STRING_DISTRIBUTION_DATE represents when the binary file was built.
 */
#define STRING_DISTRIBUTION_DATE "2026-08-05"

/**
 * Defines a generic printer name to be output to the LCD after booting Marlin.
 */
#define MACHINE_NAME "Sermoon D1"

/**
 * The SOURCE_CODE_URL is the location where users will find the Marlin Source
 * Code which is installed on the device.
 *
 * Sermoon D1 specific documentation for this fork is in the README in the project root and
 * It is under docs/. If there is no public repo URL, keep the Marlin upstream URL.
 */
#define SOURCE_CODE_URL "https://github.com/MarlinFirmware/Marlin"

/**
 * Default generic printer UUID.
 */
#define DEFAULT_MACHINE_UUID "cede2a2f-41a2-4748-9b12-c55c62f367ff"

/**
 * The WEBSITE_URL is the location where users can get more information such as
 * documentation about a specific Marlin release.
 */
#define WEBSITE_URL "www.creality.com"

/**
 * Set the vendor info the serial USB interface, if changable
 * Currently only supported by DUE platform
 */
//#define  USB_DEVICE_VENDOR_ID           0x0000
//#define  USB_DEVICE_PRODUCT_ID          0x0000
//#define  USB_DEVICE_MANUFACTURE_NAME    WEBSITE_URL
