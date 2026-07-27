/**
 * Marlin 3D Printer Firmware — Sermoon D1 fork
 *
 * Sermoon Z Lock Module
 *
 * Sermoon D1 anakartında PB0 (OUT) ve PB1 (IN/OUT) pinleri Z ekseni
 * "keep" devresine bağlıdır. Kapalı kabin yazıcısında Z ekseninin yer
 * çekimi etkisiyle kayma yapmasını önler.
 *
 * Mevcut firmware'de her iki pin sürekli HIGH tutulur (engaged state).
 * Bu modül kontrolü düzenli hale getirir ve M888 ile manuel kontrole olanak
 * verir. Otomatik (hareket-tetiklemeli) mod YOKTUR — bkz. Configuration_adv.h
 * içindeki SERMOON_Z_LOCK_AUTO notu.
 *
 * Pin atamaları: pins/stm32/pins_CREALITY.h
 *   #define Z_KEEP_PIN_PB0   PB0   // OUT — board IN
 *   #define Z_KEEP_PIN_PB1   PB1   // IN  — board OUT
 *
 * NOT: Pin yorumları ve gerçek davranış arasındaki tutarsızlık donanım
 * şeması bilinmediği için doğrulanmadı. Default davranış mevcut firmware
 * ile aynı: her iki pin HIGH (engaged).
 */
#pragma once

#include "../inc/MarlinConfigPre.h"

#if ENABLED(SERMOON_Z_LOCK)

class SermoonZLock {
public:
  static void init();
  static void engage();          // Z lock aktive (her iki pin HIGH)
  static void release();         // Z lock devre dışı (her iki pin LOW)
  static bool is_engaged() { return engaged; }

private:
  static bool engaged;
};

extern SermoonZLock zlock;

#endif // SERMOON_Z_LOCK
