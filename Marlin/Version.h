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
 * NOT: DWIN ekran SOFTVERSION (= bu) ile FW_VERSION_TEXT_VP slot'unu doldurur,
 * ekran karakter alanı ~14 karakterle sınırlı; uzun string'ler kırpılabilir.
 */
#define SHORT_BUILD_VERSION "MarlinV2 by CTK"

/**
 * Verbose version identifier — M115 yanıtında gösterilir.
 */
#define DETAILED_BUILD_VERSION SHORT_BUILD_VERSION " (Sermoon D1, base V1.1.10)"

/**
 * The STRING_DISTRIBUTION_DATE represents when the binary file was built.
 */
#define STRING_DISTRIBUTION_DATE "2026-07-21"

/**
 * Defines a generic printer name to be output to the LCD after booting Marlin.
 */
#define MACHINE_NAME "Sermoon D1"

/**
 * The SOURCE_CODE_URL is the location where users will find the Marlin Source
 * Code which is installed on the device.
 *
 * Bu fork için Sermoon D1'e özel dokümantasyon proje root'undaki README ve
 * docs/ altındadır. Public repo URL'si yoksa Marlin upstream URL'sini koru.
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
