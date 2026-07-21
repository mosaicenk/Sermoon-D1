# Sermoon D1 — Custom Marlin Firmware

Creality **Sermoon D1** kapalı kabin 3D yazıcısı için Marlin 2.0.x bugfix
tabanlı, modülerleştirilmiş ve genişletilmiş firmware.

**Versiyon**: `SD1-2.1` (base: stock Creality V1.1.10)
**Build hedefi**: Creality V4.3.1 anakart (STM32F103RET6) — **tek hedef, başka kart desteklenmez**
**Son güncelleme**: 2026-07-21

> Bu fork stock Sermoon D1 firmware'inden temizlenmiş, optimize edilmiş ve
> Marlin 2.1.x'ten cherry-pick'lenmiş özelliklerle güçlendirilmiştir.
> Tüm değişiklikler [CHANGELOG.md](CHANGELOG.md)'de listelidir.

## Donanım

| Bileşen | Değer |
|---|---|
| Anakart | Creality V4.3.1 (STM32F103RET6, 64KB RAM, 512KB Flash) |
| Stepper sürücüler | TMC2208 standalone (UART yok, 16x microstep + 256x interpolation) |
| Ekran | DWIN T5L yatay (RTS protokolü — özel) |
| Yazdırma alanı | 280 × 260 × 310 mm |
| Mekanik bed alanı | 290 × 270 × 320 mm |
| Extruder | Bowden, tek nozül, 1.75 mm filament |
| EEPROM | BL24C16 (16Kbit, I2C bit-bang) — geçerli aralık 0x000–0x7FF |
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

Çıktı: `.pio\build\creality\firmware.bin` (~184 KB).

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

Yeni firmware'i ilk kez flash ettikten sonra:

```gcode
M502     ; Factory defaults yükle
M500     ; EEPROM'a yaz
```

Sonra:
1. **PID kalibrasyonu** → [docs/pid_tuning/](docs/pid_tuning/README.md)
2. **LIN_ADVANCE K** → [docs/lin_advance/](docs/lin_advance/README.md)

## Mekanik Parametreler

`Configuration.h`'da (kullanıcı kalibrasyonu, **dokunma**):

| Parametre | Değer |
|---|---|
| Steps/mm (X, Y, Z, E) | 80, 79.60, 400, 95 (Y SD1-1.4'te 80 → 79.60 cube ölçüm kalibrasyonu) |
| Maks. hız (mm/s) | 300, 300, 5, 25 |
| Maks. ivme (mm/s²) | 1000, 1000, 100, 1000 |
| Yazdırma ivmesi | 500 mm/s² |
| Travel ivmesi | 1000 mm/s² |
| Jerk (X, Y, Z, E) | 10, 10, 0.4, 5 |
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
| `SERMOON_Z_LOCK_AUTO` | OFF | Deneysel: Z hareketinde lock'ı auto-release |

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
- `HOMING_BACKOFF_MM { 2, 2, 2 }` — endstop koruması
- `NO_MOTION_BEFORE_HOMING` — home edilmeden hareket engelleme
- `Z_HOMING_HEIGHT 4` — home öncesi Z+4mm yukarı

### Print Kalitesi
- `LIN_ADVANCE` (K=0.06, **kalibrasyon şart** — Bowden için 0.4-0.9 tipik)
- `FWRETRACT` (G10/G11 — slicer-bağımsız retraction)
- `S_CURVE_ACCELERATION` — sigmoid hız profili
- `JUNCTION_DEVIATION` (0.013) — CLASSIC_JERK yerine modern köşe-hız kontrolü
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
- `QUICK_HOME` — diagonal X+Y home

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
| Hardware microstep değişimi (M350) | TMC2208 standalone, MS pinleri sabit jumper'lı |
| `FAST_PWM_FAN` | PA0 → TIM2/TIM5 timer çakışması (TEMP/STEP ile) |
| `EMERGENCY_PARSER` | STM32F1 HAL'da implementasyon yok |
| MPC, Input Shaping | Marlin 2.1.x'e tam migration gerekirdi (~50+ saat) |

## Build Footprint

| Metrik | Değer |
|---|---|
| Flash | **184.196 byte** (%35.1 / 524288 byte) |
| RAM | **15.168 byte** (%23.1 / 65536 byte) |
| Compile warning | **0** (proje kodu) + 1 upstream (`util_adc.c`, framework) |
| Build süresi | ~22 sn (clean) |

> **2026-07-21 (SD1-2.1)**: Z-probe kodu devre dışı bırakıldığı için Flash
> −2.832 byte, RAM −16 byte. Kalan 3 DWIN uyarısı da giderildi → proje
> kodunda sıfır uyarı.

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
- Z Lock AUTO mode (default OFF, donanım davranışı belirsizliği)
- LIN_ADVANCE K=0.06 default değeri Bowden için **başlangıç** olarak yetersiz — kullanıcı kalibrasyonu şart (0.4-0.9 arası tipik)
- **SD1-2.1 PLR düzeltmesi donanımda doğrulanmalı** — adres matematiği ve blok
  sınırları derleme zamanında `static_assert` ile garantilendi, ancak gerçek bir
  elektrik kesintisi senaryosu (baskı sırasında fişi çek → aç → ekran "devam et"
  önermeli) fiziksel test gerektirir.
- MINTEMP 0 → 5 değişikliği: yazıcı 5 °C altı bir ortamda çalıştırılırsa soğuk
  boot'ta MINTEMP hatası verir. Isıtılmayan atölyede kullanılacaksa değer
  düşürülebilir (0 yapılmamalı).

> **Versiyon kontrolü yok.** Bu dizin bir git deposu değil (`.git` yok) —
> `.gitignore` mevcut ama işlevsiz. Firmware config'i elle kalibre edilmiş
> değerler içerdiği için `git init` + ilk commit şiddetle önerilir; aksi halde
> hatalı bir düzenlemeden geri dönüş yolu yok.

Sorular veya iyileştirme önerileri için CHANGELOG'a bak veya kodu doğrudan
incele — mimari modüler ve dokümante edilmiştir.
