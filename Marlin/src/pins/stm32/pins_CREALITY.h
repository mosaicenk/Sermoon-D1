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
// E2END = SON GECERLI ADRES (kapasite degil). BL24C16 = 16Kbit = 2048 byte
// => gecerli aralik 0x000..0x7FF. Marlin bunu capacity() = E2END + 1 olarak
// kullanir; 0x800 yazilirsa kapasite 2049 sanilir ve PLR_ADDR cipin disina
// tasar (bkz. Configuration.h PLR_ADDR).
#define E2END 0x7FF       // 16Kbit (24C16) — son gecerli adres
#define MYI2C_EEPROM      // EEPROM on I2C-0
#define IIC_EEPROM_SDA       PA11
#define IIC_EEPROM_SCL       PA12

// NOT: Bu kartta EEPROM I2C uzerindedir (yukaridaki MYI2C_EEPROM, BL24C16).
// SPI EEPROM ve flash-emulasyon secenekleri V4.3.1'de yoktur; kullanilmayan
// alternatif tanimlar kafa karistirmamasi icin kaldirildi.

//
// Limit Switches
//
#define X_MIN_PIN          PA5
// #define X_MAX_PIN          PA4
#define Y_MIN_PIN          PA6
#define Z_MIN_PIN          PA7   // Mekanik endstop — Z homing bunu kullanir

//
// Z Probe — YOK
//
// Bu yazicida hicbir Z-probe takili degil: BLTouch yok, enduktif sensor yok.
// Z homing mekanik endstop (PA7) ile yapilir.
//
// Board uzerindeki "BLTouch" konnektorunun PB0/PB1 pinleri Z LOCK modulune
// ayrilmistir (bkz. asagidaki Z_KEEP_PIN_*). Bu yuzden buraya probe pini
// TANIMLANMAZ — ayni pini hem output (Z lock) hem input (probe) yapmak
// donanim catismasi yaratir.
//
// Ileride probe eklenirse ONCE Configuration_adv.h'daki SERMOON_Z_LOCK
// kapatilmalidir; ikisi ayni anda etkin OLAMAZ. SanityCheck.h bunu
// derleme zamaninda yakalar. Kurulum notlari: MANUAL.md bolum 10.

//
// Steppers
//
// DIKKAT — TEK ENABLE HATTI: dort surucunun de EN girisi PC3'e baglidir.
// Marlin bu pini eksen bazinda saymaz; disable_Z() cagrisi PC3'u pasife
// cekerek X/Y/E0'i de birakir. Pratikte sorun cikarmaz cunku eksenler yalnizca
// hepsi birden bosta kalinca (DEFAULT_STEPPER_DEACTIVE_TIME + tum
// DISABLE_INACTIVE_* true) kapatilir. Sonucu olan iki kisit:
//   1. Tek bir ekseni isinma nedeniyle bagimsiz kapatmak MUMKUN DEGIL.
//      HR4988SQ'nun (Z/E0) isisi baski sirasinda yazilimla azaltilamaz;
//      cozum donanimsal sogutma.
//   2. Configuration.h'daki DISABLE_X/Y/Z/E hepsi false kalmalidir;
//      birini true yapmak digerlerini de dusurur.
#define X_ENABLE_PIN       PC3
#define X_STEP_PIN         PC2
#define X_DIR_PIN          PB9

#define Y_ENABLE_PIN       PC3
#define Y_STEP_PIN         PB8
#define Y_DIR_PIN          PB7

// Z: bu TEK STEP/DIR/EN setine PARALEL iki motor baglidir (ikinci surucu YOK).
// Bu yuzden Z2_* pin tanimlari ve Z2_DRIVER_TYPE bilerek tanimsiz birakildi.
#define Z_ENABLE_PIN       PC3
#define Z_STEP_PIN         PB6
#define Z_DIR_PIN          PB5

// E0 STEP/DIR = PB4/PB3 = JTAG hatti. Asagidaki DISABLE_DEBUG olmadan bu
// pinler GPIO'ya donmez ve ekstruder hic hareket etmez.
#define E0_ENABLE_PIN      PC3
#define E0_STEP_PIN        PB4
#define E0_DIR_PIN         PB3

//
// Surucu tipleri (Configuration.h *_DRIVER_TYPE) — KARMA:
//   X, Y  : TMC2208_STANDALONE
//   Z, E0 : A4988 — fiziksel cip HR4988SQ. Marlin'de HR4988 tipi yok; A4988
//           donanim uyumlu esdegeridir (ayni STEP/DIR/EN, ayni zamanlama).
//           Gerekce ve global yan etkileri Configuration.h'da yazili.
//
// Ikisi de STANDALONE calisir: sadece STEP/DIR/EN; firmware surucuyle
// KONUSMAZ. UART yok, bu yuzden M906 (akim), M569 (chop modu), M350
// (mikroadim) ve sensorless homing kullanilamaz. Hem akim (Vref potu) hem
// mikroadim (MS1/MS2, PCB'de sabit kablanmis — jumper yok) DONANIMDAN ayarlanir.
//
// Z'nin Vref'i ozel dikkat ister: iki motor paralel oldugu icin surucunun
// verdigi akim ikiye bolunur.
//
// Buradaki eski *_HARDWARE_SERIAL / *_SERIAL_*_PIN tanimlari HAS_TMC220x
// bloğunun icindeydi; standalone'da o makro false oldugu icin hicbir zaman
// derlenmiyorlardi. Ustelik MSerial2 (USART2) gosteriyorlardi, oysa USART3
// DWIN ekranina ayrilmis durumda. Yanlis izlenim vermemesi icin kaldirildi.
//

//
// JTAG pinlerini GPIO'ya cevir. PB3 (JTDO) ve PB4 (JNTRST) E0_DIR/E0_STEP
// olarak kullaniliyor; bu tanim olmadan ekstruder calismaz.
// NOT: eski yorum "PB4 (Y_ENABLE_PIN)" diyordu — yanlisti, Y_ENABLE PC3'tur.
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
// Ekran: DWIN T5L, USART3 uzerinden RTS protokolu (SERIAL_PORT_2 = 3).
// Ekranin kendi pinleri burada TANIMLANMAZ — surucu src/lcd/dwin/ altindadir.
//
// Bu noktada 12864 karakter-LCD ve DWIN-encoder varyantlarinin pin haritalari
// yorum olarak duruyordu. Hicbiri Sermoon D1'de kullanilmiyor ve bazilari
// aktif pinlerle celisiyordu (orn. PA4=CHECKFILEMENT, PA5/PA6/PA7=endstop).
// Yanlislikla acilma riskini ortadan kaldirmak icin kaldirildi.
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

/* Z轴锁定模块 — Z Lock modulu (SERMOON_Z_LOCK) */
// Bu iki pin board'daki "BLTouch" konnektorune denk gelir. Probe TAKILI
// OLMADIGI icin ikisi de Z lock'a ayrilmistir; her ikisi de OUTPUT olarak
// surulur. Probe eklenecekse once SERMOON_Z_LOCK kapatilmalidir.
#define Z_KEEP_PIN_PB1   PB1  //对应板子上的IN
#define Z_KEEP_PIN_PB0   PB0  //对应板子上的OUT

