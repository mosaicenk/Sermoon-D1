# Sermoon D1 — Custom Marlin Firmware

Creality **Sermoon D1** kapalı kabin 3D yazıcısı için Marlin 2.0.x bugfix
tabanlı, modülerleştirilmiş ve genişletilmiş firmware.

**Versiyon**: `SD1-2.9` (base: stock Creality V1.1.10)
**Build hedefi**: Creality V4.3.1 anakart (STM32F103RET6) — **tek hedef, başka kart desteklenmez**
**Son güncelleme**: 2026-07-27

> Bu fork stock Sermoon D1 firmware'inden temizlenmiş, optimize edilmiş ve
> Marlin 2.1.x'ten cherry-pick'lenmiş özelliklerle güçlendirilmiştir.
> Tüm değişiklikler [CHANGELOG.md](CHANGELOG.md)'de listelidir.

## Donanım

| Bileşen | Değer |
|---|---|
| Anakart | Creality V4.3.1 (STM32F103RET6, 64KB RAM, 512KB Flash) |
| Stepper sürücüler | **Karma** — X/Y: TMC2208 standalone (16x + çip içi 256x interpolasyon)<br>Z/E0: **HR4988SQ** (16x, interpolasyon **yok**). İkisi de standalone: UART yok |
| Motorlar | Creality **42-40** (dört eksende de) — ~1.0 A/faz, ~0.40 N·m |
| Z motorları | **Tek sürücüye paralel bağlı iki motor** (ikinci sürücü yok, `Z2_DRIVER_TYPE` kapalı) |
| Stepper enable | **Dört sürücü de tek hat: PC3.** Tek ekseni bağımsız kapatmak mümkün değil |
| Sürücü akımı | Vref potuyla (M906 yok). R_sense **0.15 Ω** (`R150`). Ölçülen: X/Y 1.27 V → 0.69 A RMS, Z 1.60 V → 0.47 A/motor, E0 0.86 V → 0.51 A RMS. **Fabrika ayarı doğrulandı — dokunmayın** |
| Ekran | DWIN T5L yatay (RTS protokolü — özel) |
| Yazdırma alanı | 280 × 260 × 310 mm |
| Mekanik bed alanı | 290 × 270 × 320 mm |
| Extruder | **Direct drive** (dişlisiz MK8 tipi), tek nozül, 1.75 mm filament |
| EEPROM | BL24C16 (16Kbit, I2C bit-bang) — geçerli aralık 0x000–0x7FF |
| M500 ayarları | **SD kartta** `eeprom.dat` — I2C EEPROM'da *değil* (aşağıya bak) |
| Sıcaklık sensörü | 100k thermistor (tip 1) hotend + bed |
| Probe | **YOK** — BLTouch de endüktif sensör de takılı değil. Z homing mekanik endstop (PA7) |
| Filament runout | Mekanik switch (PA4) |
| Kapalı kabin | Z lock modülü (PB0 + PB1, ikisi de) |

## Hızlı Başlangıç

### 1. Build (PlatformIO gerekli)

```powershell
cd C:\sermoon-d1
pio run -e creality
```

Çıktı: `.pio\build\creality\firmware.bin` (126.864 byte ≈ 124 KB).

> **Versiyon değiştirdiyseniz temiz derleyin.** `Marlin/Version.h` makro ile
> include edildiği için SCons bağımlılık grafiğinde yok; artımlı derleme onu
> görmez ve firmware sessizce eski sürüm dizesini taşır:
> `rm -rf .pio/build/creality && pio run -e creality`

### 2. Flash

**Yol A — SD kart**: `firmware.bin` → SD root → yazıcıyı reset.

**Yol B — J-Link**:
```powershell
pio run -e creality -t upload
```

### 3. Pre-flash güvenlik (önerilir)

Yeni firmware'i flash etmeden önce mevcut ayarları yedekle:
```
docs/eeprom_backup.gcode
```
Host (OctoPrint/PrusaSlicer console) üzerinden çalıştır, çıktıyı text
dosyasına kaydet.

### 4. Post-flash kalibrasyon

> ⚠️ **M500 için SD kart TAKILI olmalı.** Bu kartta `EEPROM_SETTINGS`,
> I2C EEPROM ile değil `persistent_store_sdcard.cpp` ile karşılanır: ayarlar
> `HAL_eeprom_data[2048]` RAM tamponuna yazılır, sonra SD kartın köküne
> **`eeprom.dat`** olarak flush edilir. Kart takılı değilse
> `PersistentStore::access_start()` `false` döner ve **M500 sessizce başarısız
> olur** — hiçbir ayar kalıcı olmaz.
>
> BL24C16 I2C EEPROM yalnızca şunlar için kullanılır: power-loss recovery
> bloğu (`PLR_ADDR`, adres 2048−sizeof), DWIN dil/level bayrakları
> (`FONT_EEPROM`, adres 0–2) ve varlık kontrolü (adres 255 = `0x55`).

Yeni firmware'i ilk kez flash ettikten sonra (SD kart takılıyken):

```gcode
M502     ; Factory defaults yükle
M500     ; SD karttaki eeprom.dat'a yaz
```

Sonra:
1. **PID kalibrasyonu** → [docs/pid_tuning/](docs/pid_tuning/README.md)
2. **LIN_ADVANCE K** → [docs/lin_advance/](docs/lin_advance/README.md)

### 5. HR4988SQ (Z/E0) devreye alma — SD1-2.4 ile zorunlu

Firmware artık Z ve E0'ı HR4988SQ olarak zamanlıyor. Aşağıdakiler **yazılımdan
doğrulanamaz**, elle yapılmalıdır. Sırayı bozma.

**a) Yön kontrolü — ilk iş bu, motor bağlanmışken**

TMC2208 ve A4988 ailesi StepStick modüllerinin motor çıkış pin sırası
terstir. Sürücü fiziksel olarak değiştiyse Z ve E ters dönebilir.
Nozülü tablaya çarptırmamak için **Z'yi yukarıda test et**:

```gcode
G91          ; göreli mod
G1 Z5 F300   ; Z YUKARI gitmeli. Aşağı gidiyorsa → INVERT_Z_DIR'i çevir
G90
M302 P1      ; soğuk ekstrüzyona izin ver (yalnızca yön testi için)
G91
G1 E5 F100   ; filament İÇERİ girmeli. Dışarı çıkıyorsa → INVERT_E0_DIR
G90
M302 P0      ; soğuk ekstrüzyonu tekrar kapat
```

Yön yanlışsa `Configuration.h` içindeki `INVERT_Z_DIR` / `INVERT_E0_DIR`
değerini ters çevirip yeniden derle. **Bu değerler bilerek değiştirilmedi** —
mevcut donanımın gerçekte nasıl bağlı olduğu ölçülmeden tahmin edilemez ve
yanlış tahmin Z'yi tablaya sürer.

**b) Vref (akım) — ölçüldü, DEĞİŞTİRMEYİN**

Akım potansiyometreyle ayarlanır; `M906` **çalışmaz**. 2026-07-23'te dört
sürücünün de Vref'i ölçüldü ve fabrika ayarının doğru olduğu doğrulandı:

| Eksen | Vref | Akım | 42-40 nominalinin | Sürücü tavanının |
|---|---|---|---|---|
| X/Y | 1.27 V | 0.69 A RMS | %69 | — |
| Z (×2 paralel) | 1.60 V | 0.47 A RMS/motor | %47 | %67 |
| E0 | 0.86 V | 0.51 A RMS | %51 | %36 |

R_sense = **0.15 Ω** (`R150`, tüm sürücüler). Formüller aynı şeyi ölçmez:

- TMC2208 → Vref **RMS** ayarlar: `I_RMS = Vref × 0.541`
- HR4988SQ → Vref **PEAK** ayarlar: `I_peak = Vref / 1.2`, sonra `÷ √2`

TMC2208'in Vref'ini HR4988SQ'ya kopyalamak **√2 kat** hata demektir.

Tek düşük marj E0 (%51). Ekstruder tıklarsa 0.86 → 1.05 V güvenli bir adımdır
(%62, sürücü hâlâ %44'te) — **ama önce (d)'deki K kalibrasyonunu yapın**,
eksik ekstrüzyonun daha olası sebebi odur.

**c) Soğutma — pazarlıksız**

HR4988SQ aynı akımda TMC2208'den belirgin daha çok ısınır, Z'de çift motor
yükü bunu artırır. Üstelik dört sürücü tek enable hattını (PC3) paylaştığı
için **tek bir ekseni ısınma nedeniyle yazılımdan kapatmak mümkün değildir**.
Soğutucu + hava akışı şart. Aşırı ısınmada sürücü termal koruma ile adım
atlar; belirtisi baskı ortasında ani katman kayması.

**d) LIN_ADVANCE K yeniden kalibrasyonu**

K sürücüye özgüdür ve E0'daki sürücü değişti — mevcut `0.06` kalibre edilmemiş
bir başlangıçtır. Direct drive için 0.02-0.15 tipiktir. `M900 K<değer>` ile
canlı dene, bulunca `M500`. Yöntem: [docs/lin_advance/](docs/lin_advance/README.md)

**e) Mikroadım doğrulaması**

`DEFAULT_AXIS_STEPS_PER_UNIT` Z=400 ve E=95, sürücülerin **16x** mikroadımda
olduğunu varsayar (MS1/MS2 karta lehimli sürücülerde sabit kablanmış, jumper
yok). Bu varsayım yanlışsa steps/mm aynı oranda değişir (örn. 8x → Z=200).
100 mm komut edip cetvelle ölçerek doğrula.

## Mekanik Parametreler

`Configuration.h`'da (kullanıcı kalibrasyonu, **dokunma**):

| Parametre | Değer |
|---|---|
| Steps/mm (X, Y, Z, E) | 80, 79.60, 400, 95 (Y SD1-1.4'te 80 → 79.60 cube ölçüm kalibrasyonu) |
| Maks. hız (mm/s) | 250, 250, 5, 25 (X/Y 300→250: yüksek hızda gövde rezonansı) |
| Maks. ivme (mm/s²) | 800, 800, 100, 5000 |
| Yazdırma ivmesi | 500 mm/s² |
| Travel ivmesi | 800 mm/s² |
| Köşe-hız kontrolü | `JUNCTION_DEVIATION` 0.015 — **`CLASSIC_JERK` kapalı**, jerk değerleri kullanılmıyor |
| Yön invert | X=true, Y/Z/E=false |
| Homing yönü | X=MIN, Y=MIN, Z=MIN |

EEPROM'da kayıtlı (M500) değerler bu defaults'ları ezer. Reset için `M502` + `M500`.

## Sermoon'a Özgü Flag'ler

`Configuration_adv.h` içinde:

| Flag | Default | Açıklama |
|---|---|---|
| `RTS_AVAILABLE` | ON | DWIN ekran sürücüsü (`Configuration.h`) |
| `EEPROM_PLR` | ON | Power-loss recovery EEPROM kaydı (PLR_ADDR 1852, 196 byte) |
| `SERMOON_Z_LOCK` | ON | Z eksen lock modülü — PB0 **ve** PB1, ikisi de |

> `SERMOON_Z_LOCK_AUTO` **kaldırıldı** (SD1-2.3). Hiçbir zaman çalışmıyordu:
> `on_motion_start()`/`on_motion_end()` tanımlıydı ama kod tabanında hiçbir
> yerden çağrılmıyordu, dolayısıyla flag'i açmak davranışı değiştirmiyordu.

> **PB0/PB1 münhasırdır.** Bu pinler board'un "BLTouch" konnektörüdür ve
> tamamen Z lock'a ayrılmıştır. Bir gün probe eklenirse **önce**
> `SERMOON_Z_LOCK` kapatılmalı — aksi halde `SanityCheck.h` derlemeyi hata
> ile durdurur (bilerek: sessiz pin çakışması yerine gürültülü hata).

### M-Code'lar (Sermoon-spesifik)

```gcode
M888              ; Z lock status sorgula
M888 S0           ; Z lock release
M888 S1           ; Z lock engage (default)
```

## Backport Edilen Yenilikler (Marlin 2.1.x'ten)

Hepsi default OFF — kullanmak istediğini `Configuration_adv.h`'da aç:

```c
//#define AUTO_REPORT_POSITION   // M154 — host pozisyon raporlama
#define SAVED_POSITIONS 0        // 1+ → G60/G61 etkin
//#define GCODE_REPEAT_MARKERS   // M808 — gcode loop
//#define HOTEND_IDLE_TIMEOUT    // Kapalı kabin filament-charring koruması
```

15 yeni thermistor tipi de eklendi (kullanılmadıkça flash'a yük olmaz).

## Aktif Tuning Ayarları

Configuration aktivasyonları:

### Güvenlik
- `NO_TIMEOUTS 1000` — host bağlantı kararlılığı
- `HOMING_BACKOFF_MM { 1, 1, 2 }` — SD1-2.9: X/Y 1 mm geri çekilir, −9'da park
- `NO_MOTION_BEFORE_HOMING` — home edilmeden hareket engelleme
- `Z_HOMING_HEIGHT 4` — home öncesi Z+4mm yukarı

### Print Kalitesi
- `LIN_ADVANCE` (K=0.06, **kalibrasyon şart** — direct drive için 0.02-0.15 tipik)
- `FWRETRACT` (G10/G11 — slicer-bağımsız retraction)
- `S_CURVE_ACCELERATION` — sigmoid hız profili
- `JUNCTION_DEVIATION` (0.015) — CLASSIC_JERK yerine modern köşe-hız kontrolü
- `ADVANCED_PAUSE_FEATURE` (M600 gelişmiş)
- `ADAPTIVE_STEP_SMOOTHING` — düşük hızda effective microstep doublelama
- `MINIMUM_STEPPER_PULSE 1` (TMC2208 için)
- `MAXIMUM_STEPPER_RATE 400000` (TMC2208 max)

### Kullanım Kolaylığı
- `HOST_ACTION_COMMANDS` — OctoPrint/PrusaSlicer entegrasyonu
- `GCODE_MOTION_MODES` — G1 motion mode hatırlama
- `PAREN_COMMENTS` — `(comment)` syntax
- ~~`CANCEL_OBJECTS`~~ — default OFF (2026-05-23 optimizasyon); M486 gerekiyorsa aç
- ~~`GCODE_MACROS`~~ — default OFF (2026-05-23 optimizasyon); makro gerekiyorsa aç
- ~~`QUICK_HOME`~~ — **SD1-2.7'de kapatıldı**; X ve Y artık sırayla homeleniyor

### Performans
- `BUFSIZE 8` — host streaming akıcılığı
- `RX_BUFFER_SIZE 512` — 115200 baud için fazlasıyla yeterli (2026-05-23: 1024→512)
- `BLOCK_BUFFER_SIZE 32` — flow continuity (2026-05-23: 16→32)
- `DEFAULT_STEPPER_DEACTIVE_TIME 300` — 5 dk inactive timeout (Z lock var)
- `SOFT_PWM_SCALE 7` + `SOFT_PWM_DITHER` — fan PWM ~7.8 Hz → ~1 kHz (whine ↓)
- `FAN_KICKSTART_TIME 100` — fan stall önleme
- `FAN_MIN_PWM 50` — düşük PWM ölü-bölge atlama
- `MINIMUM_STEPPER_PULSE 1` — TMC2208 + LIN_ADVANCE uyumlu (2026-05-23: 0→1)
- `DEFAULT_MINSEGMENTTIME 8000` µs — retract/fine hareket (2026-05-23: 20000→8000)
- `MM_PER_ARC_SEGMENT 2` — ARC block azaltma (2026-05-23: 1→2)

### EEPROM (BL24C16 I2C bit-bang) optimizasyonları (2026-05-23)
- `BL24CXX_Check()` — boot başına 1 kez (önce: 2-3 I2C transaction)
- `BL24CXX_Read()` — sequential read (~2x hızlanma)
- `BL24CXX_Write()` — 16-byte page write (~15x hızlanma)
- PLR 120-byte okuma ~30ms→~15ms; PLR yazma ~600ms→~40ms

### SHOW_REMAINING_TIME — DWIN'e özel (2026-05-23)
- Marlin'in standart `SHOW_REMAINING_TIME`'ı HAS_GRAPHICAL_LCD veya
  EXTENSIBLE_UI istiyor; Sermoon DWIN bunlardan biri değil.
- **DWIN'e özel hesaplayıcı**: `LCD_RTS.cpp::EachMomentUpdate` içinde
  `elapsed_sec * (100 - pct) / pct / 60` formülüyle kalan süre dakika
  cinsinden hesaplanır.
- **Yeni VP**: `PRINT_REMAIN_MIN_VP = 0x1410`. Kalan süre dakika olarak
  yazılır. DWIN ekran tasarımında bu VP'ye text alanı bağlamak kullanıcının
  işidir (DWIN ekran tasarım aracında).
- Linear extrapolation: başlangıçta (~%1) tahmin yüksek olur, %50+ sonra
  oldukça tutarlı. %0 veya %100'de 0 yazılır.

### Stat
- `PRINTCOUNTER` — toplam print süresi/sayısı (M78)
- `NOZZLE_PARK_FEATURE` — M600/M125 park

## Donanım Kısıtları (Şeffaflık)

Bazı modern özellikler donanım/HAL nedeniyle aktive edilemez:

| Özellik | Neden |
|---|---|
| Hardware microstep değişimi (M350) | Her iki sürücü ailesi de standalone; MS1/MS2 PCB'ye sabit kablanmış (jumper yok), MCU'ya bağlı değil |
| Sürücü akımı yazılımdan (M906) | UART/SPI yok — hem TMC2208 hem HR4988SQ Vref potuyla ayarlanır |
| Tek ekseni bağımsız kapatma | Dört sürücü tek enable hattında (PC3); biri kapanınca hepsi kapanır |
| `FAST_PWM_FAN` | PA0 → TIM2/TIM5 timer çakışması (TEMP/STEP ile) |
| `EMERGENCY_PARSER` | STM32F1 HAL'da implementasyon yok |
| MPC, Input Shaping | Marlin 2.1.x'e tam migration gerekirdi (~50+ saat) |

## Build Footprint

| Metrik | Değer |
|---|---|
| Flash | **126.864 byte** (%24.2 / 524288 byte) |
| RAM | **13.176 byte** (%20.1 / 65536 byte) |
| Compile warning | **0** (proje kodu) + 1 upstream (`util_adc.c`, framework) |
| Build süresi | ~12 sn (clean) |
| `firmware.bin` SHA256 | `A9567E83…23DA` (2026-07-27 derlemesi — güne bağlı, nota bak) |

> ⚠️ **SHA256 güne bağlıdır.** `Marlin.cpp:956` binary'ye `__DATE__` gömer
> (`Compiled: Jul 23 2026` gibi). Temiz derleme başka bir günde farklı hash
> üretir; boyut ve davranış değişmez. Bit-bit karşılaştırma yalnızca **aynı
> gün** yapılan iki derleme arasında anlamlıdır. Ölçüm (2026-07-24): yorum
> değişikliği revert/re-apply deneyi aynı gün içinde birebir aynı hash'i
> verdi (`4402B902…AD818`); dünkü `BDAB96BB…B987`'den tek fark tarih string'i.
>
> **2026-07-21 (SD1-2.1)**: Z-probe kodu devre dışı bırakıldığı için Flash
> −2.832 byte, RAM −16 byte. Kalan 3 DWIN uyarısı da giderildi → proje
> kodunda sıfır uyarı.
>
> **2026-07-21 (SD1-2.2)**: 111 ölü dosya silindi (99 `.cpp` + 12 `.h`).
> Flash/RAM **değişmedi** — silinen dosyaların binary'ye katkısı zaten 0 byte
> ölçülmüştü. Üretilen binary 2.1 ile bit-bit aynıdır
> (SHA256 `E0CDBDE9…547E`) → **yeniden flash gerekmez**.

> **2026-07-22 (SD1-2.3)**: Toolchain seviyesinde **−57.116 byte flash**
> (%35.1 → %24.2) ve **−1.992 byte RAM**. Kaynak mantığı değişmedi; tamamı
> derleyici/linker yapılandırması. Detay: [CHANGELOG.md](CHANGELOG.md).
> Binary değişti → **yeniden flash gerekir.**

> **2026-07-23 (SD1-2.4)**: Z/E0 sürücüleri HR4988SQ olarak tanımlandı
> (karma yapılandırma). Flash **+40 byte** (127.080 → 127.120) — artışın
> tamamı `MINIMUM_STEPPER_*_DIR_DELAY` 30 ns → **200 ns** düzeltmesinden
> geliyor; 30 ns HR4988SQ için yetersizdi ve yön değişimlerinde ters adım
> riski taşıyordu. Binary değişti → **yeniden flash gerekir.**
> Flash öncesi/sonrası yapılacaklar: [§4.5](#5-hr4988sq-ze0-devreye-alma--sd1-24-ile-zorunlu).

### Binary'de kalan ölü özellikler

Aşağıdaki değerler `arm-none-eabi-nm --print-size` ile **ölçülmüştür**
(tahmin değil). Toplam ~3,8 KB (%3,0). Temizlemenin kazancı regresyon
riskine değmiyor:

| Özellik | Ölçülen | Neden bırakıldı |
|---|---|---|
| `BEZIER_CURVE_SUPPORT` (G5) | 898 B | Tamamen ölü; slicer'lar G5 üretmiyor |
| `ARC_SUPPORT` (G2/G3) | 1.358 B | Bazı slicer'lar arc fitting ile üretir |
| `PRINTCOUNTER` | 760 B | Gerçek özellik, ekranda gösteriliyor |
| `SPIClass` ctor | 500 B | SD kart SDIO kullanıyor; linker gövdeyi zaten atmış |
| `FWRETRACT` (G10/G11) | 304 B | Küçük, zararsız |

> ⚠️ **`backtrace` binary'de YOK.** Önceki sürümlerde bu tablo onu "3.682 B,
> hardfault'ta stack trace basar" diye listeliyordu; ölçüm bunu doğrulamıyor —
> `unwarm*`/`UnwReport*` sembollerinin **hiçbiri** binary'de değil, tamamı
> `--gc-sections` tarafından atılmış (kaynak dosyalar derleniyor ama hiçbir
> yerden çağrılmıyor). Hardfault ayıklaması için stack trace isteniyorsa
> özelliğin fiilen bir fault handler'a bağlanması gerekir.

## Dokümantasyon

| İçerik | Konum |
|---|---|
| **Tüm dokümantasyon TOC** | [`docs/README.md`](docs/README.md) |
| Değişiklik tarihçesi | [`CHANGELOG.md`](CHANGELOG.md) |
| PID kalibrasyon | [`docs/pid_tuning/`](docs/pid_tuning/README.md) |
| LIN_ADVANCE kalibrasyon | [`docs/lin_advance/`](docs/lin_advance/README.md) |
| Junction Deviation kalibrasyonu | [`docs/junction_deviation/`](docs/junction_deviation/README.md) |
| EEPROM yedekleme gcode | [`docs/eeprom_backup.gcode`](docs/eeprom_backup.gcode) |

## Önemli Dosyalar (kod tabanı)

| Yol | İçerik |
|---|---|
| `Marlin/Configuration.h` | Yazıcı parametreleri (mekanik, sıcaklık, endstop) |
| `Marlin/Configuration_adv.h` | Gelişmiş ayarlar (TMC, babystep, runout, fan, vb.) |
| `Marlin/Version.h` | Sürüm string'leri |
| `Marlin/src/lcd/dwin/LCD_RTS.cpp` | DWIN ekran sürücüsü (~2535 satır, RTS protokolü) |
| `Marlin/src/lcd/dwin/i2c_eeprom.cpp` | BL24C16 bit-bang I2C EEPROM |
| `Marlin/src/feature/sermoon_zlock.cpp` | Z lock modülü |
| `Marlin/src/pins/stm32/pins_CREALITY.h` | Anakart pin atamaları |
| `Marlin/src/HAL/HAL_STM32F1/` | Donanım soyutlama (libmaple tabanlı) |

## Lisans

Marlin GPL-3.0 lisansı altında dağıtılır. Detay için [`LICENSE`](LICENSE).

---

## Geliştirici Notu

Yapılan tüm iş **konservatif** prensiple yapıldı:
- Mekanik parametreler değiştirilmedi (kullanıcı kalibrasyonu olarak korundu)
- Yeni özellikler default OFF olarak eklendi (Tier 1 backportlar)
- API kıran refactor'ler atlandı (HostUI class, ExtUI mimari değişimi vb.)
- STM32F1 HAL'a derin müdahale yapılmadı (FAST_PWM_FAN, EMERGENCY_PARSER)
- Build her aşamada doğrulandı, son durum 0 warning ile temiz

Test edilmemiş alanlar:
- Yeni Configuration aktivasyonlarının fiziksel davranışı yazıcıda doğrulanmalı
- **SD1-2.3 newlib-nano geçişi**: `dtostrf()` kullanan üç yol donanımda
  doğrulanmalı — M114 pozisyon raporu, M600/pause ekranındaki Z/E değerleri ve
  power-loss recovery'nin ürettiği `G92.9 E<değer>` komutu. Ondalık basamaklar
  doğru basılıyorsa `-u_printf_float` doğru çalışıyor demektir.
- LIN_ADVANCE K=0.06 direct drive için makul bir **başlangıç**, ama E0 sürücüsü değiştiği (SD1-2.4) için kullanıcı kalibrasyonu şart (0.02-0.15 arası tipik)
- **SD1-2.1 PLR düzeltmesi donanımda doğrulanmalı** — adres matematiği ve blok
  sınırları derleme zamanında `static_assert` ile garantilendi, ancak gerçek bir
  elektrik kesintisi senaryosu (baskı sırasında fişi çek → aç → ekran "devam et"
  önermeli) fiziksel test gerektirir.
- MINTEMP 0 → 5 değişikliği: yazıcı 5 °C altı bir ortamda çalıştırılırsa soğuk
  boot'ta MINTEMP hatası verir. Isıtılmayan atölyede kullanılacaksa değer
  düşürülebilir (0 yapılmamalı).

> **Versiyon kontrolü**: SD1-2.2 ile git deposu kuruldu. `1d2ba27` commit'i
> temizlik öncesi tam durumu (433 dosya) dondurur — silinen her dosya oradan
> geri alınabilir: `git checkout 1d2ba27 -- <yol>`.

Sorular veya iyileştirme önerileri için CHANGELOG'a bak veya kodu doğrudan
incele — mimari modüler ve dokümante edilmiştir.
