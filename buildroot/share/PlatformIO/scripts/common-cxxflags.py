#
# common-cxxflags.py
# Convenience script to apply customizations to CPP flags
#
Import("env")

# ---------------------------------------------------------------------------
# LINK asamasi — newlib-nano
#
# Olcum: link komutunda --specs=nano.specs YOKTU (pio run -v ile dogrulandi),
# yani TAM newlib'e linkleniyorduk. Sonuc: _svfprintf_r + _dtoa_r + _strtod_l +
# _mprec ailesi 16.088 byte, newlib malloc 2.820 byte.
#
# Bu flag'ler Marlin/src/HAL/HAL_STM32F1/build_flags.py'nin SCons 'else:'
# dalinda zaten yaziliydi, ancak o dosya extra_scripts'te listelenmedigi icin
# o dal HIC calismiyordu — asagisi orijinal niyeti fiilen etkin kiliyor.
#
# -u_printf_float ZORUNLU: nano.specs'in printf'i varsayilan olarak %f
# desteklemez. Marlin dtostrf() kullaniyor (powerloss recovery G-code uretimi,
# M114 pozisyon raporu, LCD_RTS pause ekrani) ve dtostrf sprintf("%*.*f")
# uzerinden calisir. Bu flag olmadan bu degerler bozuk basilir.
# ---------------------------------------------------------------------------
# cxx_runtime_min.cpp VARLIK NOBETCISI
#
# O dosya kaybolursa derleme sessizce basarili olur ve firmware ~28,8 KB
# buyur (libstdc++'in zayif __verbose_terminate_handler'i geri gelir, o da
# __cxa_demangle uzerinden tum isim cozumleyicisini yeniden linkler).
# Asagidaki --require-defined sembolun TANIMLI olmasini sart kosar: dosya
# yoksa link "symbol ... required but not defined" ile DURUR.
# Sembol .set ile mutlak tanimli => 0 byte maliyet.
#
# --undefined DEGIL: o secenek sembolu yalnizca "undefined" olarak girer
# (amaci arsivden modul cektirmek) ve cozumlenmeden kalirsa HATA VERMEZ.
# Olculdu: --undefined ile dosya silindiginde link basariyla tamamlandi,
# yani nobetci sessizce ise yaramiyordu. --require-defined dogru secenek.
env.Append(LINKFLAGS=[
  "--specs=nano.specs",
  "-u_printf_float",
  "-Wl,--require-defined=sermoon_cxx_runtime_min_present"
])

env.Append(CXXFLAGS=[
  "-Wno-register",

  # Marlin hicbir yerde throw/catch veya dynamic_cast/typeid kullanmaz.
  # Bu flag'ler olmadan GCC her fonksiyon icin unwind tablosu uretir ve
  # libsupc++'in istisna makinesini (_Unwind_*, __gxx_personality_v0,
  # __cxa_*) binary'ye baglar.
  #
  # OLCUM (arm-none-eabi-nm): EH/unwind ailesi = 5.838 byte flash, 59 sembol.
  #
  # CXXFLAGS'a konuyor, build_flags'a DEGIL: build_flags C dosyalarina da
  # uygulanir ve GCC orada "valid for C++ but not for C" uyarisi verir.
  "-fno-exceptions",
  "-fno-rtti",
  "-fno-unwind-tables",
  "-fno-asynchronous-unwind-tables",

  # Fonksiyon-ici static'ler icin __cxa_guard_acquire/release uretilmesini
  # engeller. Marlin tek is-parcacikli (ISR'ler C++ static init yapmaz), bu
  # kilitler gereksiz. Ustelik guard fonksiyonlari libsupc++'tan cekildiginde
  # yanlarinda istisna personality rutinini de getiriyorlardi.
  "-fno-threadsafe-statics",

  # Global destructor kaydini __cxa_atexit yerine atexit'e cevirir. Gomulu
  # hedefte main() hicbir zaman donmez; destructor'lar zaten calismaz.
  #
  # NOT: Bu iki flag Marlin/src/HAL/HAL_STM32F1/build_flags.py icindeki SCons
  # 'else:' dalinda da tanimli, ANCAK o dal hic calismiyor — dosya sadece
  # "!python ..." ile stdout uretmek uzere cagriliyor, extra_scripts'te
  # listelenmedigi icin Import("env") dali olu kod. Etkin olmalari icin
  # buraya tasindilar.
  "-fno-use-cxa-atexit"
  #"-Wno-incompatible-pointer-types",
  #"-Wno-unused-const-variable",
  #"-Wno-maybe-uninitialized",
  #"-Wno-sign-compare"
])
