# Sermoon D1 Firmware — Changelog

Bu dosya bu fork üzerinde stock Creality Sermoon D1 firmware'ine göre
yapılan tüm değişiklikleri belgeler.

Format: [Keep a Changelog](https://keepachangelog.com/en/1.1.0/)
Versiyonlama: Sermoon-D1-X.Y[-suffix] (X = major iyileştirme, Y = minor)

---

## [Sermoon-D1-2.2] — 2026-07-21

Ölü kod temizliği. **Firmware davranışı değişmedi** — üretilen binary 2.1 ile
bit-bit aynıdır (SHA256 `E0CDBDE9…547E`, 184.196 byte). Bu, kaynak düzeyinde
bir sürümdür; **yeniden flash gerekmez.**

### Added

- **Git deposu.** Proje bugüne kadar versiyon kontrolsüzdü. Temizlikten önce
  `1d2ba27` taban commit'i oluşturuldu; silinen her dosya oradan geri alınabilir.

### Removed — 111 dosya

Hepsinin binary'ye katkısı `arm-none-eabi-size` ile **0 byte** ölçüldü;
silinmeleri ölçülebilir şekilde davranışsızdır.

- **99 `.cpp`** — config tarafından tamamen `#if`'lenmiş kaynaklar:
  - Tabla tesviye / probe: `probe.cpp`, `bedlevel/**` (ABL, UBL, MBL),
    `gcode/probe/*` (G30, G31, G38, M401, M851, M951),
    `gcode/bedlevel/*` (G29, G26, G42, M420, M421), `M48`, `G425`
  - Diğer sürücüler: `L6470.cpp`, `TMC26X.cpp`, `trinamic.cpp`, `tmc_util.cpp`,
    `M122`, `M569`, `M906`, `M911-M914` — bizde TMC2208 **standalone** (UART yok)
  - Delta/SCARA kinematiği: `G33`, `M665`, `M666`
  - Servo: `Servo.cpp`, `servo.cpp`, `M280`, `M281`
  - Spindle/lazer: `M3-M5`, `M7-M9`
  - Çoklu ekstruder: `M217`, `M218`, `M605`
  - SPI SD kart: `Sd2Card.cpp` — bu kartta **SDIO** kullanılıyor
  - Alternatif EEPROM yolları: `eeprom_i2c.cpp`, `eeprom_spi.cpp`,
    `persistent_store_eeprom.cpp`, `persistent_store_flash.cpp`
  - Karakter LCD: `ultralcd.cpp`, `lcdprint.cpp`, `buzzer.cpp`, `M250`, `M300`
  - Çeşitli: `filwidth`, `twibus`, `power`, `binary_stream`, `backlash`,
    `cancel_object`, `e_parser`, `hotend_idle`, `repeat`, `M43`, `M100`
- **12 `.h`** — hiçbir yerden include edilmeyen yetim başlıklar:
  `language_tr.h` (aktif dil `en`), `pinsDebug.h`, `pinsDebug_list.h`,
  `onboard_sd.h`, `HAL_ST7920.h`, `MarlinSerial.h`, `servo_private.h`,
  `bug_on.h`, `thermistornames.h`, `bresenham.h`, `least_squares_fit.h`,
  `HAL_STM32F1/pinsDebug.h`

`inc/Warnings.cpp` **korundu** — derleme zamanı uyarılarını üreten altyapı.

### Changed — `pins_CREALITY.h`

Yanıltıcı ölü bloklar kaldırıldı (hiçbiri derlenmiyordu):

- `#if HAS_TMC220x` bloğu. Standalone sürücülerde bu makro **false**; blok hiç
  derlenmiyordu ama okuyana "UART bağlı" izlenimi veriyordu. Üstelik
  `MSerial2` (USART2) gösteriyordu, oysa USART3 DWIN ekranına ayrılmış.
  Yerine standalone'un ne anlama geldiğini açıklayan not kondu.
- Alternatif ekran pin haritaları (RET6/VET6 12864 LCD, DWIN encoder).
  Bazıları **aktif pinlerle çelişiyordu** — örn. `BTN_EN2 PA4` ile
  `CHECKFILEMENT_PIN PA4`, `LCD_PINS_ENABLE PA7` ile `Z_MIN_PIN PA7`.
  Yanlışlıkla açılma riski ortadan kaldırıldı.
- Kullanılmayan SPI/flash EEPROM stub'ları. Bu kartta I2C BL24C16 var.

### Kararlar

- **`Version.h` değiştirilmedi.** `STRING_DISTRIBUTION_DATE` binary'ye gömülü;
  bumplamak bit-bit aynılığı bozar ve işlevsel bir sebep olmadan yeniden flash
  gerektirirdi.
- **Binary'deki ölü özellikler bırakıldı** (`ARC_SUPPORT` 1.359 B,
  `BEZIER_CURVE_SUPPORT` 197 B, `FWRETRACT` 346 B, `backtrace` 3.682 B,
  `SPIClass` ctor 500 B — toplam ~6,9 KB / %3,7). Flash %35 dolu; bu kazanç
  için config kurcalamak regresyon riskine değmez. `backtrace` ayrıca hardfault
  ayıklamada işe yarar.

### Doğrulama

Her fazdan sonra derleme yapıldı ve SHA256 karşılaştırıldı:

| Faz | Sonuç |
|---|---|
| 99 `.cpp` silindi | binary bit-bit aynı |
| 14 yetim başlık silindi | **build kırıldı** → `spi_pins.h` ve `endstop_interrupts.h` geri alındı (makroyla include ediliyorlar, `HAL_PATH(...)`) |
| 12 başlık ile tekrar | binary bit-bit aynı |
| `pins_CREALITY.h` temizliği | binary bit-bit aynı |

### Docs

- `MANUAL.md §10` — probe ekleme rehberine "kaynak dosyaları artık ağaçta yok"
  uyarısı ve `git checkout 1d2ba27 -- …` geri alma komutu eklendi.

---

## [Sermoon-D1-2.1] — 2026-07-21

Donanım gerçeğiyle hizalama + üç kritik hata düzeltmesi. Kapsam: **yalnızca
Creality V4.3.1 anakartı, Sermoon D1, Z-probe yok.**

### Fixed — Kritik

- **PLR (elektrik kesintisi kurtarma) çalışmıyordu.**
  `pins_CREALITY.h`: `E2END` `0x800` → `0x7FF`.
  E2END "son geçerli adres" demektir, kapasite değil. 24C16 = 2048 byte, geçerli
  aralık 0..2047. Yanlış değer `PLR_ADDR`'ı (`E2END + 1 - sizeof(info)`) 1853'e
  itiyordu → kapsanan aralık 1853..**2048**, yani çipin bir byte dışına.
  - `sizeof(job_recovery_info_t)` = **196 byte** (ELF sembolünden ölçüldü;
    kodda "~120 byte" yazıyordu — yanlıştı).
  - Yazma: son byte için kontrol byte'ı `0xA0 + (8<<1)` = **0xB0** oluyordu;
    24C16 sadece 0xA0–0xAE'ye cevap verir → NAK, byte kayboluyordu.
  - Okuma: sequential read blok sınırında sardığı için o byte 1792'den
    okunuyordu.
  - O byte `valid_foot`. `recovery.valid()` = `valid_head == valid_foot` ve
    `valid_head` her kayıtta artıyor → **eşleşme asla olmuyordu**, ekran hiçbir
    zaman "devam et" önermiyordu.
  - Şimdi: `PLR_ADDR` = 1852, aralık 1852..2047, tamamı blok 7 içinde.
  - `E2END` ayrıca `persistentStore.capacity()` = `E2END + 1` olarak da
    kullanılıyordu; kapasite de 2049'dan 2048'e düzeldi.

- **EEPROM okuma protokolü hatalıydı (255 üstü tüm adresler).**
  `i2c_eeprom.cpp`: 24C16'da blok-seçim bitleri (B2..B0) I2C kontrol byte'ının
  parçasıdır ve random read'de restart'tan **sonraki** okuma kontrol byte'ında
  da tekrarlanmalıdır. Kod düz `0xA1` gönderiyordu → her zaman blok 0.
  `BL24CXX_ReadOneByte()` ve `BL24CXX_Read()` düzeltildi.
  - Ek olarak `BL24CXX_Read()` artık transaction'ı 256-byte blok sınırında
    bölüyor (adres sayacı blok içinde sarar, sonraki bloğa geçmez).
  - `BL24CXX_Write()` zaten doğruydu (kontrol byte'ı her 16-byte sayfada
    yeniden hesaplanıyor, 16 sayısı 256'yı tam böldüğü için sınır aşılamaz) —
    yalnızca yorumları düzeltildi.

- **PB0/PB1 pin çakışması: Z lock vs Z probe.**
  `FIX_MOUNTED_PROBE` (PB1) ile `SERMOON_Z_LOCK` (PB0+PB1) aynı anda etkindi.
  `setup()` sırasında `zlock.init()` (Marlin.cpp:856) PB1'i OUTPUT+HIGH yapıyor,
  ardından `endstops.init()` (Marlin.cpp:1001) aynı pini INPUT_PULLUP'a
  çeviriyordu → **PB1'deki Z lock boot'ta sessizce ölüyordu**, yalnızca PB0
  kalıyordu.
  - Ayrıca STM32F1'de `WRITE()` pin modunu değiştirmez (bare BSRR yazımı), bu
    yüzden boot sonrası `M888 S0/S1` Z lock yerine probe pininin
    pull-up/pull-down bias'ını oynatıyordu.
  - Çözüm: yazıcıda probe donanımı olmadığı için `FIX_MOUNTED_PROBE` kapatıldı;
    PB0 ve PB1 tamamen Z lock'a ayrıldı. Z homing mekanik endstop (PA7) ile.

- **Sınır dışı dizi yazması** — `LCD_RTS.cpp` `RTS_HandleData()`:
  `axis`, `min`, `max` başlatılmadan kullanılabiliyordu. `current_position[axis]`
  garbage bir indeksle yazılırsa STM32'de (MMU yok) bellek bozulması demektir.
  Güvenli varsayılanlarla başlatıldı, if-zinciri `else` ile kapatıldı.
  Derleyicinin 3 "maybe-uninitialized" uyarısı da böylece giderildi.

### Fixed — Güvenlik

- **`HEATER_0_MINTEMP` 0 → 5**, **`BED_MINTEMP` 0 → 5**.
  MINTEMP, kopuk/kısa devreli termistör korumasıdır; 0 değeri bu eşiği pratikte
  ulaşılamaz kılıyordu. (`HEATER_1..5_MINTEMP` zaten 5'ti — tutarsızlık da
  giderildi.) 5 °C altı ortamda çalıştırılacaksa düşürülebilir, 0 yapılmamalı.

### Added — Regresyon Koruması

- `SanityCheck.h`: `SERMOON_Z_LOCK` + `HAS_BED_PROBE` birlikte etkinse derleme
  hata ile durur. Pin çakışmasının sessizce geri gelmesini engeller.
- `powerloss.cpp`: iki `static_assert` — PLR bölgesi (a) EEPROM sınırı içinde,
  (b) tek bir 256-byte blok içinde olmalı. `job_recovery_info_t` büyürse
  derleme zamanında yakalanır.

### Changed

- `platformio.ini`: `monitor_speed` 250000 → **115200** (`BAUDRATE` ile eşitlendi).
  Not: host portu (SERIAL_PORT 1) ve DWIN portu (SERIAL_PORT_2 3) aynı BAUDRATE'i
  paylaşır; DWIN T5L 115200 istediği için host'u yükseltmek ayrı bir `BAUDRATE_2`
  eklemeyi gerektirir.
- `Configuration_adv.h`: `BLOCK_BUFFER_SIZE` için ölü `#if ENABLED(SDSUPPORT)`
  dallanması kaldırıldı (her iki dal da 32 idi).
- `Version.h`: `STRING_DISTRIBUTION_DATE` → 2026-07-21.

### Docs

- **README.md**: probe satırı "BLTouch (opsiyonel)" → "YOK"; LIN_ADVANCE K
  0.22 → 0.06 (2.0.2'de düzeltildiği iddia edilmişti, düzeltilmemişti);
  baud 250000 → 115200; footprint güncellendi; versiyon kontrolü uyarısı eklendi.
- **MANUAL.md**: §10 tamamen "geçerli değil" uyarısıyla işaretlendi —
  §10.3.4 `AUTO_BED_LEVELING_BILINEAR`'ı **aktif** gösteriyordu, oysa
  `Configuration.h`'da yorumda. §14 pin haritası, §15.3 (Z offset artık
  babystep), §16.3 (probe G-code'ları derlenmedi), §17.3 (probe → homing/Z lock
  sorun giderme), §17.5, §18 güncellendi.
- **docs/README.md**: BLTouch referansı kaldırıldı.

### Build Footprint

| Metrik | SD1-2.0.2 | SD1-2.1 | Δ |
|---|---|---|---|
| Flash | 187.028 byte | **184.196 byte** | **−2.832** |
| RAM | 15.184 byte | **15.168 byte** | **−16** |
| Proje kodu uyarısı | 3 | **0** | **−3** |

### Test Edilmemiş (Donanım gerekli)

- **PLR fiziksel doğrulama**: baskı sırasında fişi çek → aç → ekran "devam et"
  önermeli. Adres matematiği `static_assert` ile garantili ama gerçek senaryo
  test edilmedi.
- Z lock'ın artık PB0 **ve** PB1'de çalışması (önceden yalnızca PB0 idi) —
  `M888 S0` / `M888 S1` ile davranış farkı gözlenmeli.
- MINTEMP 5: soğuk ortamda boot davranışı.

---

## [Sermoon-D1-2.0.2] — 2026-05-23

Kapsamlı optimizasyon paketi — Senior developer review sonucu uygulandı.

### Compile/Correctness Fixes

- **`MINIMUM_STEPPER_PULSE 0 → 1`** — TMC2208 standalone E + LIN_ADVANCE
  SanityCheck hatası giderildi (önceki değer compile-time hata veriyordu).
  TMC2208 internal synchronizer 1µs pulse'u rahat kaldırır.
- **CHANGELOG 2.0.1 senkronizasyonu** — JD değerlendirildi, fiziksel testlerde
  ringing kabul edilemez → `0.013`'te kalındı (kod ile uyumlu).

### Performance Optimizations

- **`MINIMUM_STEPPER_PULSE 1`** — STM32F1 + TMC2208 + LIN_ADVANCE uyumlu
- **`DEFAULT_MINSEGMENTTIME 20000 → 8000 µs`** — retract/fine hareketlerde
  gereksiz yavaşlama kaldırıldı
- **`BLOCK_BUFFER_SIZE 16 → 32`** — flow continuity (RAM +512 byte)
- **`RX_BUFFER_SIZE 1024 → 512`** — 250000 baud yeterli (RAM -512 byte)
- **`MM_PER_ARC_SEGMENT 1 → 2`** — ARC block darboğazı azaltma
- **`DEFAULT_STEPPER_DEACTIVE_TIME 300`** korundu (Z lock uyumlu)

### EEPROM (BL24C16 I2C bit-bang) optimizasyonları

- **`BL24CXX_Check()`** — boot başına 1 kez (önce: 2-3 I2C transaction).
  `static bool checked` flag ile EEPROM wear azaltma.
- **`BL24CXX_Read()`** — sequential read (~2x hızlanma).
  120-byte PLR okuma ~30ms → ~15ms.
- **`BL24CXX_Write()`** — 16-byte page write (~15x hızlanma).
  120-byte PLR yazma ~600ms → ~40ms.

### DWIN Ekran optimizasyonu

- **`RTSUpdate()` SD throttle** — `RTS_SDCardUpate()` her main loop'tan
  RTS_UPDATE_INTERVAL (1000ms) ile sınırlandı. CPU yükü azaltma.

### RAM/Flash Tasarrufu (default OFF'a çekilen Tier 1 backportlar)

- **`GCODE_MACROS`** default ON → OFF (~255 byte RAM tasarrufu)
- **`CANCEL_OBJECTS`** default ON → OFF (parser 'O' case kaldırıldı)
- Her ikisi de `Configuration_adv.h`'da yorum olarak korundu — kullanıcı isterse açabilir

### Dokümantasyon

- **README.md** — kullanım kolaylığı / performans tabloları güncellendi,
  Build Footprint gerçek değerlerle (Flash 186964 / RAM 15184)
- **MANUAL.md** — Bowden/Direct drive çelişkisi giderildi (Sermoon D1 = Bowden)
- **README.md** — LIN_ADVANCE K=0.22 yorumu → K=0.06 ile düzeltildi

### Build Footprint (Ölçüldü, `pio run -e creality`)

| Metrik | SD1-2.0.1 (önce) | SD1-2.0.2 (sonra) | Δ |
|---|---|---|---|
| Flash | 187.292 byte | **186.964 byte** | **−328 byte** |
| RAM | 13.712 byte | **15.184 byte** | **+1.472 byte** |
| Warnings | 4 (RTS uninit) | **0** | **−4** |

### Test Edilmemiş (Donanım gerekli)

- EEPROM page-write davranışı (PLR save sırasında)
- RTS SD update throttle (1s yeterli mi baskı sırasında?)
- BLOCK_BUFFER_SIZE 32 (RAM +512 byte, flow continuity iyileşmesi)
- MM_PER_ARC_SEGMENT 2 (kalite etkisi)

---

## [Sermoon-D1-2.0.1] — 2026-05-23

Junction Deviation optimizasyonu denemesi — fiziksel test sonucu korundu.

### Configuration.h

- **`JUNCTION_DEVIATION_MM` değerlendirildi (0.013 ↔ 0.020)**
  - 0.020 denendi: %5-15 print süresi kazancı, ringing riski
  - **Sonuç**: Fiziksel testlerde ringing kabul edilemez seviyede; default
    `0.013`'te kalındı (Configuration.h:796). Kullanıcı isterse runtime'da
    `M205 J0.020` ile deneyebilir.
  - Default değişmedi → davranış Sermoon-D1-2.0 ile aynı.

### Test Planı

1. Normal print ile kalite kontrolü (özellikle köşeler)
2. Ringing tespiti → varsa `0.018` ara deneme
3. İyi sonuç → 0.02 koru, kötü sonuç → 0.013 geri dön

---

## [Sermoon-D1-2.0] — 2026-05-09

DWIN ekran sürücüsünde **Tier A — Symbolic Cleanup**. Davranış sıfır değişiklik;
hedef: kod okunabilirliği, tip güvenliği, ölü kod temizliği. Refactor tier'ları
arasından sadece bu uygulandı; Tier B (file split) ve Tier C-D (state enum,
protocol robustness) ileride değerlendirilecek.

### Header Refactor (`Marlin/src/lcd/dwin/LCD_RTS.h`)

- **`#define <addr>` → `constexpr uint16_t`** (numeric VP'ler için tip güvenli)
  - Frame protokolü sabitleri (`FHONE`, `FHTWO`, `RegAddr_W/R`, `VarAddr_W/R`,
    `ExchangePageBase`, vb.) `constexpr uint8_t/uint16_t/uint32_t` olarak
  - Tüm `*_VP`, `*_KEY`, `*_DATA_VP` adresleri `constexpr uint16_t`
  - String makroları (`MACHINE_TYPE`, `FIRMWARE_VERSION`, vb.) `#define` olarak
    bırakıldı (text concat compatibility)
- **Yeni `enum DwinStatusSlot : uint8_t`** — DWIN font tablosundaki status
  string slot offset'leri için sembolik enum:
  ```cpp
  DWIN_STATUS_CARD_OUT = 53, DWIN_STATUS_READY = 62,
  DWIN_STATUS_HEATING  = 71, DWIN_STATUS_PRINTING = 89,
  DWIN_STATUS_COOLING  = 107, DWIN_STATUS_PAUSED  = 125,
  ```
- **Yeni `Z_OFFSET_DISPLAY_VP = 0x1026`** — Adjust ekranı Z offset göstergesi
  (önceden 3 yerde magic literal idi, artık sembolik)

### Ölü VP/Sembol Temizliği (Header)

Aktif kod tarafından hiç kullanılmayan veya yenisi tarafından değiştirilmiş
40+ legacy define silindi:

| Tür | Silinen |
|---|---|
| Dead VP'ler | `IconPrintstatus`, `FeedrateDisplay`, `Stopprint`, `Pauseprint`, `Resumeprint`, `PrintscheduleIcon`, `Timehour`, `Timemin`, `Bedtemp`, `NozzleTemp`, `MacVersion`, `SoftVersion`, `PrinterSize`, `CorpWebsite`, `FilementUnit2`, `LCDKeyIcon`, `FanKeyIcon`, `HeatPercentIcon`, `AutoZeroIcon`, `Choosefilename`, `FilenameNature`, `FilenameIcon`, `FilenameIcon1`, `FilenameCount`, `FilenamePlay`, `FilenameChs`, `Printfilename`, `SDFILE_ADDR`, `VolumeIcon`, `SoundIcon`, `StartSoundSet`, `SoundAddr`, `StartIcon`, `Percentage`, `NzBdSet`, `NozzlePreheat`, `BedPreheat`, `AutoLevelMode`, `DisplayXaxis`, `DisplayYaxis`, `DisplayZaxis`, `Exchfilement`, `Root_Key`, `Erase_Status`, `ENSURE_KEY`, `CEIconGrap` |
| Dead enum | `enum PROC_COM` (Printfile, Ajust, Feedrate, vb. — `RTS_HandleData` switch'i artık doğrudan VP adresi kullanıyor, bu enum dispatch'a geçilmedi) |
| Dead helpers | `Addvalue`, `*_Value` defines, `const Addrbuf[]` array |
| `Version.h` | `MAC_LENGTH`, `MAC_WIDTH`, `MAC_HEIGHT`, `MACVERSION`, yorumlu `CORP_WEBSITE_*` defines |

Aktif kullanımı olan legacy VP'ler (`AutoZero`, `AutolevelVal`,
`AutolevelIcon`, `ExchFlmntIcon`, `FilementUnit1`) korundu; sembolik
gruplama altına alındı. (`AutoZero` build sırasında compile error ile
yakalandı, `enum ReturnKeyAddr` dışında `case` label'ı olarak kullanılıyor.)

### Magic Offset Eliminasyonu

DWIN status string'i için `language_change + N - 1` literal'leri `enum
DwinStatusSlot` ile değiştirildi. Toplam **22 yerde** (5 farklı dosya):

| Eski | Yeni |
|---|---|
| `language_change +63 -1, STATUS_DP_CHAR_VP` | `language_change + DWIN_STATUS_READY, STATUS_DP_CHAR_VP` |
| `language_change +54 -1, ...` | `... + DWIN_STATUS_CARD_OUT, ...` |
| `language_change +72 -1, ...` | `... + DWIN_STATUS_HEATING, ...` |
| `language_change +90 -1, ...` | `... + DWIN_STATUS_PRINTING, ...` |
| `language_change +108 -1, ...` | `... + DWIN_STATUS_COOLING, ...` |
| `language_change +126 -1, ...` | `... + DWIN_STATUS_PAUSED, ...` |

Etkilenen dosyalar: `LCD_RTS.cpp` (×17), `G28.cpp` (×2), `G0_G1.cpp` (×1),
`queue.cpp` (×1).

### Ölü Kod Bloğu Temizliği (`LCD_RTS.cpp`)

Yorum içinde kalmış 2535 → 2340 satır (**−195 satır, %7.7 azalma**):

- 13 `//case Xxx:` yorum satırı (silinen `PROC_COM` enum üyelerine atıf)
- 4 büyük `/* ... */` ölü blok (`RTS_SDCardUpate` filename clear, `RTS_SDcard_Stop`,
  `EachMomentUpdate` IconTemp/HeatPercentIcon hesaplama)
- 6 büyük çoklu `//` blok (legacy VP yazma denemeleri)
- ~30 dağınık tek-satır `//RTS_SndData(..., DEAD_VP)` referansı
- Duplicate file-clear loop (`RTS_SDCardUpate`'de aynı işi yapan 2 loop) tek loop'a
  birleştirildi

### Dil ve Yorum Çevirileri

Block comment header'ları ve davranış-açıklayıcı yorumlar Çince → Türkçe:
- `/*** transmit Printer information ***/` → `/*** Yazıcı bilgilerini ekrana yaz ***/`
- `/*** clean screen ***/` → `/*** Ekran temizliği ***/`
- VP map section başlıkları (`图标变量 → 标题` → `Başlık (Title) char VP'leri`, vb.)
- Inline trivial Çince yorumlar (`//点击"打印"按钮` vb.) iç tutarlılık için
  korundu; toplu çeviri Tier B kapsamında

### Footprint (Ölçüldü, `pio run -e creality`)

| Metrik | SD1-1.4 | SD1-2.0 | Δ |
|---|---|---|---|
| Flash | 183.644 byte | **183.868 byte** | **+224 byte (+0.12%)** |
| RAM | 14.224 byte | **14.232 byte** | **+8 byte** |
| Compile warning | 0 | **0** | 0 |
| LCD_RTS.h | 499 satır | 415 satır | −84 |
| LCD_RTS.cpp | 2535 satır | 2316 satır | −219 |

Flash artışı (+224 byte) beklenenden farklı; muhtemel sebep: `constexpr`
sembolleri compiler'ın bazı durumlarda farklı şekilde optimize etmesi
(magic literal `0x1026` → `Z_OFFSET_DISPLAY_VP` gibi referanslar yeniden
relocation gerektirebilir). RAM artışı (+8 byte) ihmal edilebilir. Refactor
gerçek davranış değişikliği getirmediği için footprint nötr kabul edilir.

Build kompozisyonu: 261 sn build süresi, 0 warning, 0 error.

### Test Planı (Manuel — Yazıcıda)

Davranış değişikliği yok, fakat refactor sırasında syntax/regex hatası
olmadığından emin olmak için minimum smoke test:
1. Flash, boot — DWIN logo ve dil seçim ekranı normal
2. Ana ekran → tüm sayfalar (Print/Temp/Settings) → tek tek açılış
3. Status string'leri her durumda doğru gösterilmeli:
   - Card removed (`DWIN_STATUS_CARD_OUT`)
   - Card inserted (`DWIN_STATUS_READY`)
   - Heating up (`DWIN_STATUS_HEATING`)
   - Printing (`DWIN_STATUS_PRINTING`)
   - Cooling down (`DWIN_STATUS_COOLING`)
   - Paused (G28 sonu pause path, `DWIN_STATUS_PAUSED`)
4. Z offset adjust ekranı (`Z_OFFSET_DISPLAY_VP=0x1026`) — değer gösterimi
5. SD eject mid-print → Card-out status doğru
6. M115 → `SD1-2.0` doğrulama

### Bilinçli Olarak Yapılmadı

- **Tier B (file split + dispatch table)**: Davranış riski + büyük diff,
  ayrı sürüm. Bkz. modernize-refactor planı (önceki tartışma).
- **Tier C-D (state enum, protocol robustness)**: Davranış değişikliği riski,
  somut tetik yokken yatırım yapılmadı.
- **DWIN UI string çevirileri** (Çinceden Türkçe'ye ekran metinleri): DWIN
  ekran flash'ında, firmware kapsamı dışı.
- **`AutolevelVal`, `AutolevelIcon`, `ExchFlmntIcon`, `FilementUnit1`**:
  Aktif kullanım kanıtı bulundu, korundu.
- **Inline Çince yorumlar (sayfa numaraları, akış açıklamaları)**: Toplu
  çeviri Tier B'de tek-pass yapılacak.

---

## [Sermoon-D1-1.4] — 2026-05-08

Y ekseni dimensional kalibrasyonu — kullanıcı baskı ölçüm sonucuna dayalı.

### Configuration.h

- **`DEFAULT_AXIS_STEPS_PER_UNIT {80, 80, 400, 95}` → `{80, 79.60, 400, 95}`**

### Kalibrasyon Verisi

20×20×20 mm test cube ölçüm sonucu:

| Eksen | Beklenen | Ölçülen | Sapma |
|---|---|---|---|
| X | 20.00 | 20.00 | 0.00% ✅ |
| **Y** | **20.00** | **20.10** | **+0.50%** ❌ |
| Z | 20.00 | 20.00 | 0.00% ✅ |

Düzeltme matematiği:
```
new_Y_steps = old_Y_steps × (expected / actual)
new_Y_steps = 80 × (20.00 / 20.10) = 79.60
```

### Olası Mekanik Sebep

Y ekseni X'ten farklı çünkü:
- Y belt path (270 mm bed) X belt path'ten (290 mm bed) farklı gerilim altında
- Y pulley nominal 16T ama gerçek diş çapı tolerans dahilinde sapabilir
- Belt esnemesi yüksek hızda Y'de daha belirgin

Stock Sermoon kullanıcılarının çoğunda 80 değeri yeterli. Bu fork'un yazıcısı için kalibrasyon sonucu farklı çıktı.

### Memory Notu

`feedback_mechanical_params.md`'deki "mekanik params değişmez" kuralı hâlâ
geçerli. Bu değişiklik **kullanıcı ölçüm sonucu** kalibrasyon, kuralın
ruhuna aykırı değil — kuralın amacı "rastgele tweak yapma", burada amaç
"ölçüm temelli düzeltme".

### Geri Alma

Eğer farklı bir Sermoon'a flash edilecekse veya Y belt değişirse:
```c
#define DEFAULT_AXIS_STEPS_PER_UNIT   {80, 80, 400, 95}  // stock
```

### Footprint

Sabit değer değişikliği. Flash ~0 byte fark beklenir.

---

## [Sermoon-D1-1.3] — 2026-05-08

Tabla mekanik kalibrasyon sınırı için Z=0 referans noktası kaydırması ve
post-homing rest pozisyonu paper-test gap konumuna senkronize edildi.

**Kullanıcı amacı:** "G28 sonu nozzle yataktan 1 mm yukarıda durmalı, bu
pozisyon Z=0 olarak tanımlanmalı." — bed leveling vidaları sıkışıklığı için
mekanik fix yerine yazılım workaround.

### Configuration.h

- **`Z_MIN_POS 0` → `-1`**: Software endstop minimum sınırı negatif Z'ye izin verir.
  Mekanik Z trigger artık koordinat olarak Z=-1 (yataktaki nozzle teması) konumunda.
- **`MANUAL_Z_HOME_POS -1`** (yeni, eskiden yorumlu): G28 sonrası
  set_axis_is_at_home(Z) `current_position.z = -1` yapar. Mekanik trigger
  noktası firmware'de Z=-1 olarak görünür, Z=0 trigger'dan **1 mm yukarıdaki**
  paper-test gap pozisyonudur.

### Configuration_adv.h

- **`Z_AFTER_HOMING 5` → `0`**: G28 sonu nozzle'ı Z=0 pozisyonuna götürür.
  MANUAL_Z_HOME_POS=-1 ile birlikte: G28 bittiğinde nozzle fiziksel olarak
  yataktan 1 mm yukarıda durur — yani **paper-test pozisyonu**. Bu konum
  kullanıcının manuel doğruladığı Z=0 referansıdır.

### Davranış Değişikliği — Tam Haritası

| Slicer/Gcode | Coord Z | Fiziksel Pozisyon |
|---|---|---|
| Mekanik trigger (homing slow pass son nokta) | -1 | nozzle yatakta temas |
| Slicer Z=0 | 0 | **yataktan 1 mm yukarı (paper-test)** |
| Slicer Z=0.2 (1. katman) | 0.2 | yataktan 1.2 mm yukarı (gap ✓) |
| Travel/park hareketleri Z=2-5 | 2-5 | yataktan 3-6 mm yukarı |

**G28 akış:**
```
1. Z fast pass → trigger (coord Z=-1, fiziksel bed contact)
2. Bump back 2 mm → coord Z=1 (fiziksel 2 mm yukarı)
3. Z slow pass → trigger tekrar (coord Z=-1)
4. HOMING_BACKOFF 2 mm → coord Z=1 (fiziksel 2 mm yukarı)
5. Z_AFTER_HOMING 0 → move to coord Z=0 (1 mm aşağı = fiziksel 1 mm yukarı)
6. Final: coord Z=0, fiziksel yataktan 1 mm yukarı
```

`M114` çıktısı G28 sonu: `X:0 Y:0 Z:0.00` — paper-test pozisyonu.

### Slicer Uyumluluğu

- **Cura, Prusa Slicer, Simplify3D**: Otomatik adapte. İlk katman Z=0.2 dediğinde
  fiziksel 1.2 mm gap. Slicer'da değişiklik gerekmez.
- **Custom gcode**: `MIN_SOFTWARE_ENDSTOP_Z = Z_MIN_POS = -1` koruması var, slicer
  Z=-2 gibi imkansız değer komut edemez.

### Paper Test Prosedürü (Yazıcıda)

```gcode
M502    ; factory defaults
M500    ; EEPROM
M115    ; "SD1-1.3" doğrula
G28     ; home
M114    ; Z:0.00 (paper-test pozisyonu)
; A4 kâğıt al, yatak ile nozzle arasına yerleştir, hafif sürtünme olmalı
; Eğer kâğıt sıkışıksa: gap çok az → MANUAL_Z_HOME_POS -1.5 dene
; Eğer kâğıt rahat: gap çok fazla → MANUAL_Z_HOME_POS -0.5 dene
```

### Geri Alma (Mekanik Fix Yapılırsa)

Z endstop bracket'ı yukarı kaydırırsan (ASIL DOĞRU FIX):
```c
// Configuration.h
#define Z_MIN_POS 0                    // -1 → 0
//#define MANUAL_Z_HOME_POS -1         // yorum yap
// Configuration_adv.h
#define Z_AFTER_HOMING 5               // 0 → 5 (eski güvenli rest yüksekliği)
```

### Slicer Uyumluluğu

- **Cura, Prusa Slicer, Simplify3D**: Otomatik adapte olur. İlk katman Z=0.2 dediği
  zaman fiziksel 1.2 mm gap. Slicer'da değişiklik gerekmez.
- **Custom gcode**: Eğer Z=-0.5 gibi negatif değer kullanıyorsan trigger'a
  yaklaşır, dikkat. `MIN_SOFTWARE_ENDSTOP_Z = Z_MIN_POS = -1` koruma sağlar.

### Geri Alma (Mekanik Fix Yapılırsa)

Z endstop bracket'ı yukarı kaydırırsan (ASIL DOĞRU FIX):
```c
// Configuration.h
#define Z_MIN_POS 0                    // -1 → 0
//#define MANUAL_Z_HOME_POS -1         // yorum yap
```

### Bilinçli Olarak Yapılmadı

- **Z endstop bracket fiziksel kaydırma**: Yazılım fix'i tercih edildi (kullanıcı
  isteği). Mekanik fix daha doğrudur ama kullanıcı firmware yolunu seçti.
- **M206 + M500 runtime offset**: Aynı etkiyi verirdi ama M502 sonrası kaybolurdu.
  Compile-time fix kalıcılığı sağlar.

### Footprint Beklentisi

MANUAL_Z_HOME_POS sadece Z_HOME_POS macro'sunu değiştirir, ek kod üretmez.
Z_MIN_POS değeri sabit, runtime overhead yok. Flash artışı **0 byte**.

---

## [Sermoon-D1-1.2] — 2026-05-08

Paralel bağlı 2 Z motoru altyapısı için homing iyileştirmeleri. Hareket
mekaniği değişmedi; sadece homing fazı tutarlılığı arttırıldı.

### Configuration_adv.h

- **`HOMING_BUMP_DIVISOR { 2, 2, 1 }` → `{ 2, 2, 4 }`**
  - Z slow pass 4 mm/s → 1 mm/s (X/Y aynı: 25 mm/s)
  - Paralel iki Z motorunun endstop'a daha tutarlı eş-konumlu trigger'ı
  - +0.8 sn homing süresi maliyeti (kabul edilebilir)

- **`Z_AFTER_HOMING 5` (yeni)**
  - G28 sonu Z otomatik 5 mm güvenli yüksekliğe gider
  - Paralel motorlar referans konumdan başlar — operatör belirsizliği azalır
  - G28.cpp'de tetikleme kodu eklendi

- **`IMPROVE_HOMING_RELIABILITY` aktive**
  - Stock konum: SENSORLESS_HOMING bloğu içinde gömülü
  - SD1-1.2: bağımsız global tanım, sensorless gerektirmez
  - Homing sırasında geçici X/Y accel düşürme (1000 → 100 mm/s²)
  - JD aktif olduğu için CLASSIC_JERK reduction etkisiz, sadece accel etkili
  - Start-of-motion shock azalır → her iki Z motoru daha sync başlar

### G28.cpp

- Z homing sonu `Z_AFTER_HOMING` kontrolü eklendi:
  ```c
  #if defined(Z_AFTER_HOMING)
    do_blocking_move_to_z(Z_AFTER_HOMING);
  #endif
  ```
- Mevcut `Z_AFTER_PROBING` (BLTOUCH'lı sistemler için) ile çakışmaz —
  Sermoon'da BLTOUCH yok, sadece Z_AFTER_HOMING aktif.

### Bilinçli Olarak Yapılmadı

- **`HOMING_FEEDRATE_Z` 4 mm/s → 3 mm/s düşürme**: Tartışıldı,
  reddedildi. Gerekçe: somut skip-step kanıtı yok, +25% homing süresi
  maliyeti, stock değer milyonlarca yazıcıda doğrulandı. Konservatif
  prensiple (kullanıcı tercihi) stock değer korundu.
- **Z_DUAL_STEPPER_DRIVERS** ve sensorless homing: Donanım kısıtı
  (tek driver/iki motor paralel + TMC2208 standalone). Ek modifikasyon
  gerekir, kapsam dışı.

### Footprint Beklentisi

Z_AFTER_HOMING 1 satır kod, IMPROVE_HOMING_RELIABILITY zaten Marlin'de
mevcut path. Flash artışı <%0.05 beklenir.

---

## [Sermoon-D1-1.1] — 2026-05-08

Tier A — Yapısal 2.1.x parity adımları. Davranış değişmedi, sadece
mimari/naming modernizasyonu.

### Module Renames (2.1.x convention)

- `module/configuration_store.cpp/h` → `module/settings.cpp/h`
- `feature/power_loss_recovery.cpp/h` → `feature/powerloss.cpp/h`
- `feature/emergency_parser.cpp/h` → `feature/e_parser.cpp/h`
- `feature/binary_protocol.cpp/h` → `feature/binary_stream.cpp/h`
- `feature/Max7219_Debug_LEDs.cpp/h` → `feature/max7219.cpp/h`

Bu rename'ler mevcut kodun davranışını değiştirmez; future cherry-pick'lerin
2.1.x naming convention'ı ile uyumlu olmasını sağlar.

### Yeni Infrastructure

- **`core/bug_on.h`** — Runtime assertion macros (BUG_ON). POSTMORTEM_DEBUGGING
  + MARLIN_DEV_MODE flag bağımlı; release'de zero overhead.
- **`inc/Changes.h`** — Compile-time deprecated config detection. Eski
  config name kullanan kullanıcı build hatası alır + new name'e yönlendirilir.
- **`inc/Warnings.cpp`** — Compile-time #pragma message warnings. Sermoon-spesifik
  config seçim önerileri (LIN_ADVANCE not kalibre, JD active, vb.).
- **`libs/autoreport.h`** — Generic AutoReporter template. M154 (auto-report
  position) bunu kullanacak şekilde refactor edildi. Future AUTO_REPORT_FANS,
  AUTO_REPORT_TEMPERATURES için temel.

### Changed

- **M154 implementation**: `auto_report_position_interval` global +
  `auto_report_position()` function pattern → `AutoReporter<PositionReport>
  position_auto_reporter` template instance
  - Kullanıcı API: `M154 S<seconds>` aynı çalışır
  - İçeride: 2.1.x AutoReporter template kullanır

### Footprint

| | SD1-1.0 | SD1-1.1 | Δ |
|---|---|---|---|
| Flash | 183.924 | 183.924 | 0 |
| RAM | 14.224 | 14.232 | +8 (template instantiation) |
| Compile warning | 0 | 0 | 0 |

Strüktürel parity ~2.1.x'te ~%75'e çıktı (modül naming + infrastructure
helpers). Davranışsal değişim yok.

---

## [Sermoon-D1-1.0] — 2026-05-08

İlk konsolide fork sürümü. Stock Creality Sermoon D1 V1.1.10 base'inden
modülerleştirme + yenilik backport + güvenlik iyileştirmeleri.

### Cleanup (Faz 0)

- Sermoon D1 dışındaki tüm donanım desteği temizlendi
- HAL: 10 platform → yalnız `HAL_STM32F1` + `HAL_shared`
- Pin dosyaları: 11 board klasörü + 36 STM32 pin dosyası → yalnız `pins_CREALITY.h`
- LCD altsistemleri: HD44780, dogm, extensible_ui, menu, extui_* → yalnız `dwin/`
- 50+ printer config örneği silindi (`/config/`)
- `platformio.ini`: 30+ env → yalnız `[env:creality]`
- `buildroot/`: 13 build script + 7 board JSON → yalnız Sermoon ile ilgili olanlar
- `data/www/` (ESP32 web UI) silindi
- `process-palette.json` (Atom IDE) silindi
- `boards.h`, `pins.h`, `platforms.h`, `shared/servo.h` Sermoon-only sadeleştirildi

### Build Sistemi Düzeltmeleri

- `creality.py` — Python 3.14 deque mutation fix
- `platformio.ini` lib_deps temizliği — TMC2208 standalone Sermoon hiçbir dış
  kütüphane gerektirmiyor (TMCStepper, MAX31865, LiquidCrystal, Sailfish vb.
  hepsi kaldırıldı)
- `src_filter` → `build_src_filter` (deprecated warning fix)

### Eklenen Özellikler (Marlin 2.1.x'ten Backport)

- **15 yeni thermistor tipi** — module/thermistor/thermistor_{14,17,21,22,23,30,68,202,332,502-505,1022,2000}.h
- **M154** — auto-report position (`AUTO_REPORT_POSITION` flag)
- **G60/G61** — save/restore current position (`SAVED_POSITIONS` flag)
- **M808** — gcode repeat marker (`GCODE_REPEAT_MARKERS` flag)
- **HOTEND_IDLE_TIMEOUT** — kapalı kabin için filament-charring koruması

### Atlandı (Donanım/API uyumsuzluğu)

- **M255** sleep mode — MarlinUI bağımlı, RTS uyumsuz
- **Fan Check** — Flags<N>/AutoReporter altyapısı yok, tach pin yok
- **Meatpack** — SerialBase template altyapısı 800+ satır gerekirdi
- **EMERGENCY_PARSER** — STM32F1 HAL'da implementasyon yok (sanity check blokluyor)

### Sermoon-Spesifik Yeni Modüller

- **Z Lock Module** (`feature/sermoon_zlock.cpp/h`)
  - Mevcut hack-style Marlin.cpp implementasyonu refactor edildi
  - `SERMOON_Z_LOCK` flag, `SERMOON_Z_LOCK_AUTO` opsiyonel auto mode
  - **M888** gcode — manuel kontrol (`M888 [S<0|1>]`)
  - Default: boot'ta engage, manuel override mümkün

### Configuration_adv.h Aktivasyonları

#### Güvenlik
- `NO_TIMEOUTS 1000` — host bağlantı kararlılığı
- `HOMING_BACKOFF_MM { 2, 2, 2 }` — endstop koruması

#### Print Kalitesi
- `LIN_ADVANCE` — Bowden için kritik (K=0.22 default, kalibrasyon gerekir)
- `FWRETRACT` — firmware-side retraction (G10/G11)
- `ADVANCED_PAUSE_FEATURE` — gelişmiş M600 (pause, park, unload, prompt)
- `ADAPTIVE_STEP_SMOOTHING` — düşük hızlarda effective microstep doublelama
- `MINIMUM_STEPPER_PULSE 1` (TMC2208 için)
- `MAXIMUM_STEPPER_RATE 400000` (TMC2208 max)

#### Kullanım Kolaylığı
- `HOST_ACTION_COMMANDS` — OctoPrint/PrusaSlicer pause/resume entegrasyonu
- `GCODE_MOTION_MODES` — G1 motion mode hatırlama
- `PAREN_COMMENTS` — `(comment)` syntax (CAM çıktıları)
- `CANCEL_OBJECTS` — multi-object print mid-print cancel
- `GCODE_MACROS` — M810-M819 makro
- `QUICK_HOME` — diagonal X+Y home

#### Performans
- `BUFSIZE`: 4 → 8 (host streaming akıcılığı)
- `RX_BUFFER_SIZE 1024` (USB throughput)
- `DEFAULT_STEPPER_DEACTIVE_TIME`: 120 → 600 sn (10dk timeout)
- `SOFT_PWM_SCALE`: 0 → 7 (PWM ~7.8 Hz → ~1 kHz, fan whine azalır)
- `SOFT_PWM_DITHER` — soft PWM çözünürlük telafisi
- `FAN_KICKSTART_TIME 100` — fan stall önleme
- `FAN_MIN_PWM 50` — düşük PWM ölü-bölgesi atlama

### Configuration.h Aktivasyonları

#### Güvenlik
- `NO_MOTION_BEFORE_HOMING` — home edilmeden hareket engelleme
- `Z_HOMING_HEIGHT 4` — home öncesi Z+4mm yukarı kaldır

#### Print Kalitesi
- `S_CURVE_ACCELERATION` — sigmoid hız profili (smoother motion)
- **`CLASSIC_JERK` → `JUNCTION_DEVIATION`** — modern köşe-hız algoritması
  - Eski: 4 jerk parametresi (X/Y/Z/E)
  - Yeni: tek `JUNCTION_DEVIATION_MM 0.013` parametresi
  - Geri dönüş: Configuration.h:744'te yorum açıp kapatılır
  - Kalibrasyon: `docs/junction_deviation/`

#### Stat
- `PRINTCOUNTER` — toplam print süresi/sayısı/filament istatistiği (M78)

#### Pause/Park
- `NOZZLE_PARK_FEATURE` — M600/M125 için park pozisyonu

### Dokümantasyon

Yeni `docs/` yapısı:
- `docs/README.md` — Master TOC
- `docs/pid_tuning/` — PID kalibrasyon (README + 2 gcode)
- `docs/lin_advance/` — LIN_ADVANCE kalibrasyon (README + gcode)
- `docs/Bresenham.md` — Marlin step algoritması (orijinal)

### Build Footprint Özeti

| Aşama | Flash | RAM |
|---|---|---|
| Stock baseline (cleanup öncesi) | 174.300 byte | 12.952 byte |
| **Konsolide fork (bu sürüm)** | **183.620 byte (35.0%)** | **14.224 byte (21.7%)** |
| Δ | +9.320 byte (~%5.3) | +1.272 byte (~%2) |

### Notlar

- Mekanik parametreler (steps/mm, feedrate, accel, jerk, bed/print size)
  **kullanıcı kalibrasyonu olarak korunduğu için DEĞİŞTİRİLMEDİ**
- Tüm yeni feature flag'ler **default OFF** olarak eklendi (Tier 1 backportları);
  `Configuration_adv.h` aktivasyonları kullanıcı için sıfır risk
- TMC2208 standalone donanım kısıtı: hardware microstep değişikliği
  firmware'den mümkün değil
- FAST_PWM_FAN: PA0 → TIM2/TIM5 timer çakışması nedeniyle aktive edilemez

---

## [Stock Creality Sermoon D1 V1.1.10] — Base

Marlin 2.0.x bugfix branch tabanlı, Creality tarafından Sermoon D1 için
özelleştirilmiş. Bu fork'un başlangıç noktası.
