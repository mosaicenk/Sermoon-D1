/**
 * Marlin 3D Printer Firmware — Sermoon D1 fork
 *
 * Minimal C++ runtime — libstdc++ sisme onleyici.
 *
 * SORUN
 * libstdc++'in varsayilan std::terminate isleyicisi
 * __gnu_cxx::__verbose_terminate_handler(), yakalanmamis bir istisnanin TIP
 * ADINI insan-okunur hale getirmek icin __cxa_demangle() cagirir. Bu tek
 * referans, libiberty'nin C++ isim cozumleyicisinin TAMAMINI binary'ye baglar:
 * d_print_comp (11.448 B), d_type (2.020 B), cplus_demangle_operators,
 * d_encoding, d_exprlist, d_print_mod ... toplam 44 sembol.
 *
 * OLCUM (arm-none-eabi-nm, SD1-2.2 taban binary):
 *   cozumleyici ailesi = 28.824 byte flash  -> firmware'in %15,6'si
 *
 * Marlin hicbir yerde istisna atmaz/yakalamaz; bu kodun tamami erisilemez.
 *
 * COZUM
 * Asagidaki tanim libstdc++'in zayif surumunu link zamaninda gecersiz kilar.
 * Artik __cxa_demangle'a referans kalmadigi icin linker cozumleyiciyi
 * iceren arsiv uyelerini hic cekmez.
 *
 * DAVRANIS
 * std::terminate pratikte cagrilamaz (istisna yok). Yine de cagrilirsa Cortex-M3
 * sistem reset'i tetiklenir.
 *
 * NEDEN RESET, "kesmeleri kapat + dur" DEGIL: kesmeleri kapatip donmak, o anda
 * HIGH olan bir isitici pinini kalici HIGH birakir — soft-PWM ISR'i artik
 * calismadigi icin isitici surekli acik kalir (termal kacak). Reset ise tum
 * GPIO'yu donanimsal olarak giris moduna dondurur => isiticilar garantili kapali.
 */

#ifdef __STM32F1__

#include <stdint.h>

/**
 * LINK-ZAMANI VARLIK NOBETCISI
 *
 * SORUN: bu dosya kaybolursa (git clean, eksik commit, taze klon) derleme
 * SESSIZCE BASARILI olur. Sadece libstdc++'in zayif __verbose_terminate_handler
 * geri gelir, cozumleyici zinciri yeniden linklenir ve firmware ~28,8 KB
 * buyur. Hicbir hata verilmez — regresyon fark edilmez.
 *
 * COZUM: burada mutlak (absolute) bir sembol tanimlanir; common-cxxflags.py
 * icindeki -Wl,--undefined=... bu sembolu link icin ZORUNLU kilar. Dosya
 * yoksa link "undefined reference" ile durur.
 *
 * MALIYET: .set mutlak sembol uretir, hicbir section'a yer ayirmaz => 0 byte.
 * (Olculdu: nobetci eklendikten sonra firmware boyutu degismedi.)
 */
__asm__(".globl sermoon_cxx_runtime_min_present\n"
        ".set   sermoon_cxx_runtime_min_present, 0\n");

// SCB->AIRCR = VECTKEY(0x5FA) | SYSRESETREQ — ARMv7-M mimari standardi,
// libmaple/CMSIS basligina bagimli degil. Reset tum GPIO'yu giris moduna
// dondurur => isiticilar garantili kapali.
static inline void sermoon_system_reset() {
  *(volatile uint32_t *)0xE000ED0CUL = (0x5FAUL << 16) | (1UL << 2);
  __asm__ volatile("dsb");
  for (;;) {}   // reset devreye girene kadar
}

namespace __gnu_cxx {
  void __verbose_terminate_handler() { sermoon_system_reset(); }
}

/**
 * NOT — __cxa_pure_virtual burada TANIMLANMAZ.
 * maple framework'u zaten saglar (cores/maple/cxxabi-compat.cpp), dolayisiyla
 * o referans libsupc++'i cekmiyor. Burada ikinci bir tanim vermek
 * "multiple definition" link hatasi uretir (denendi, dogrulandi).
 */

#endif // __STM32F1__
