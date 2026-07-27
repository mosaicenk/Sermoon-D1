# MarlinV2 by CTK — Sermoon D1 Firmware Manual

**Firmware:** MarlinV2 by CTK (Marlin 2.0.x bugfix branch)  
**Yazıcı:** Creality Sermoon D1  
**Board:** Creality V4.3.1 (STM32F103RET6, 72 MHz, 512KB Flash, 64KB RAM)  
**Ekran:** DWIN T5L (LCD_RTS protokolü)  
**Tabla:** 290 × 270 mm, Alüminyum plaka, 320 mm Z travel  
**Sürücüler:** Karma — X/Y TMC2208 standalone, Z/E0 HR4988SQ  
**Tarih:** 2026-07-23 (SD1-2.4)  

> **2026-07-23 denetimi.** Bu kılavuzun sürücü/akım bölümleri ölçülmüş
> donanım verisiyle yeniden yazıldı. Özellikle **§8 tamamen değişti**: eski
> hâli `#if HAS_TRINAMIC` bloğundaki ayarları (StealthChop, HYBRID_THRESHOLD,
> INTERPOLATE) etkinmiş gibi anlatıyordu — bu blok bu kartta **hiç
> derlenmiyor**. Eski §8'e dayanarak yapılmış hiçbir ayar davranışı
> değiştirmemiştir.

---

## İçindekiler

1. [Genel Bakış](#1-genel-bakış)
2. [Donanım Bilgileri](#2-donanım-bilgileri)
3. [Firmware Derleme](#3-firmware-derleme)
4. [Firmware Yükleme](#4-firmware-yükleme)
5. [Step ve Hareket Ayarları](#5-step-ve-hareket-ayarları)
6. [Homing Yapılandırması](#6-homing-yapılandırması)
7. [Hareket Algoritmaları](#7-hareket-algoritmaları)
8. [Sürücü Ayarları (karma: TMC2208 + HR4988SQ)](#8-sürücü-ayarları-karma-tmc2208--hr4988sq)
9. [Linear Advance (LIN_ADVANCE)](#9-linear-advance-lin_advance)
10. [Z-Probe ve Auto Bed Leveling (ABL)](#10-z-probe-ve-auto-bed-leveling-abl)
11. [Sıcaklık ve Güvenlik](#11-sıcaklık-ve-güvenlik)
12. [EEPROM ve SD Kart](#12-eeprom-ve-sd-kart)
13. [DWIN Ekran Entegrasyonu](#13-dwin-ekran-entegrasyonu)
14. [Pin Haritası](#14-pin-haritası)
15. [Kalibrasyon Kılavuzu](#15-kalibrasyon-kılavuzu)
16. [G-Code Referansı](#16-g-code-referansı)
17. [Sorun Giderme](#17-sorun-giderme)
18. [Değişiklik Günlüğü](#18-değişiklik-günlüğü)

---

## 1. Genel Bakış

Bu firmware, Creality'nin orijinal Sermoon D1 V1.1.10 yazılımı üzerine Marlin 2.0.x bugfix branch'ten özel fork olarak derlenmiştir. Orijinale göre yapılan tüm değişiklikler:

| Özellik | Orijinal | Güncel | Açıklama |
|---------|----------|--------|----------|
| Versiyon string | Creality V1.1.10 | MarlinV2 by CTK | Tanımlayıcı |
| DWIN databuf | 26 byte | 40 byte | Taşma düzeltmesi |
| HOMING_FEEDRATE_XY | 3000 mm/m | 1000 mm/m | Daha yumuşak homing |
| LIN_ADVANCE_K | 0.22 | 0.06 | Direct drive — **kalibrasyon zorunlu**, SD1-2.4'te E0 sürücüsü değişti |
| ~~HYBRID_THRESHOLD~~ | Kapalı | **Hiç etkin olmadı** | `#if HAS_TRINAMIC` içinde → bu kartta derlenmiyor (§8) |
| Z/E0 sürücü tipi | TMC2208 sanılıyordu | **HR4988SQ** (`A4988`) | SD1-2.4 — donanımla hizalandı |
| `MINIMUM_STEPPER_*_DIR_DELAY` | 30 ns | **200 ns** | HR4988SQ gereği; 30 ns'de ters yönde adım riski |
| Z_SAFE_HOMING | Kapalı | Açık | G28'de Z, tabla ortasında (145, 135) home edilir |
| HEATER_0_MINTEMP / BED_MINTEMP | 0 | 5 | Kopuk termistör koruması geri kazanıldı |
| E2END | 0x800 (hatalı) | 0x7FF | 24C16 son geçerli adres — PLR taşmasını giderdi |
| SERMOON_Z_LOCK | (kod dışı) | Açık, PB0+PB1 | M888 ile kontrol edilebilir Z lock modülü |

**Z-probe / ABL**: Stock'ta olduğu gibi **kapalı**. Yazıcıda probe donanımı yok;
PB0/PB1 Z lock modülüne ayrılmıştır. Ayrıntı için [bölüm 10](#10-z-probe-ve-auto-bed-leveling-abl).

---

## 2. Donanım Bilgileri

### 2.1 Board: Creality V4.3.1

| Özellik | Değer |
|---------|-------|
| MCU | STM32F103RET6 |
| Saat hızı | 72 MHz |
| Flash | 512 KB |
| RAM | 64 KB |
| Besleme | 24V DC |
| Logic seviye | 3.3V (MCU tarafı) |
| Bootloader | 0x08000000 (SD kart yükleme) |
| Firmware yük adresi | 0x08007000 (28KB offset) |

### 2.2 Step Motor Sürücüler

Kart **karma** sürücü taşır. Hepsi kartta lehimlidir ve her birinin kendi
Vref trim potu vardır.

| Eksen | Sürücü | Marlin tipi | Ölçülen Vref | Gerçek akım | 42-40 nominalinin |
|-------|--------|-------------|--------------|-------------|-------------------|
| X | TMC2208 | `TMC2208_STANDALONE` | 1.27 V | 0.69 A RMS | %69 |
| Y | TMC2208 | `TMC2208_STANDALONE` | 1.27 V | 0.69 A RMS | %69 |
| Z | **HR4988SQ** | `A4988` | 1.60 V | **0.47 A RMS/motor** | %47 |
| E0 | **HR4988SQ** | `A4988` | 0.86 V | 0.51 A RMS | %51 |

- **Motorlar:** dört eksende de Creality **42-40** (~1.0 A/faz, ~0.40 N·m,
  ~2.8 Ω, ~5.5 mH)
- **R_SENSE: 0.15 Ω (`R150`)** — tüm sürücülerde, sarım başına 2 adet.
  *(Bu kılavuzun eski hâlindeki `0.11 Ω` yanlıştı; genel TMC2208 modül
  değerinden kopyalanmıştı.)*
- **Mikroadım: 16×**, sürücüler karta lehimli (ayrı jumper'lı modül yok —
  board fotoğrafıyla doğrulandı), MS1/MS2 PCB üzerinde sabit kablanmış. MCU'ya
  bağlı değil → `M350` çalışmaz.
- **256× interpolasyon yalnızca X/Y'de** — TMC2208'in kendi donanım
  özelliği. HR4988SQ interpolasyon yapmaz, Z/E0'da 16× gerçekten 16×'tir.

> **Z'de tek sürücüye paralel bağlı İKİ motor vardır.** Sürücünün verdiği
> akım ikiye bölünür; tablodaki 0.47 A değeri motor başınadır (sürücü toplamı
> 0.94 A RMS / 1.33 A peak, HR4988SQ tavanının %67'si).

> **Dört sürücünün de enable girişi PC3'tür.** Marlin bu pini eksen bazında
> saymaz — tek bir ekseni bağımsız olarak kapatmak mümkün değildir. HR4988SQ'da
> duruş akımı azaltma da yoktur (TMC2208'de vardır), yani Z sürücüsü katmanlar
> arasında tam akımda bekler. Soğutucu zorunludur.

### Vref hesabı — iki formül aynı şeyi ölçmez

| Sürücü | Vref neyi ayarlar | Formül (Rs = 0.15 Ω) |
|---|---|---|
| TMC2208 | **RMS** | `I_RMS = Vref × 0.541` |
| HR4988SQ | **PEAK** | `I_peak = Vref / 1.2` , `I_RMS = I_peak / √2` |

TMC2208'in Vref'ini HR4988SQ'ya kopyalamak **√2 kat** hata demektir.

> **Fabrika ayarı doğrulanmıştır — potlara dokunmayın.** Üç değer de hem motor
> (%47–69) hem sürücü (%36–67) açısından güvenli bölgede. Tek düşük marj E0
> (%51); ekstruder tıklaması görülürse **önce LIN_ADVANCE K kalibre edilmeli**
> (§9), akım ancak o zaman hâlâ sorun varsa 1.05 V'a çıkarılmalı.

### 2.3 Ekstruder

- **Tip:** **Direct drive** (dişlisiz MK8 tipi; drive gear hotend üzerinde) —
  kullanıcı donanım doğrulaması, 2026-07-24.
  *(Tarihçe: dokümanın ilk hâli "direct drive" diyordu; SD1-2.4 denetimi bunu
  "E steps/mm = 95 → MK8 → Bowden" çıkarımıyla "Bowden" olarak değiştirdi.
  Çıkarım hatalıydı: 95 steps/mm dişlisiz MK8 tipi **besleyicinin** değeridir
  ve besleyicinin nerede durduğunu söylemez — dişlisiz direct drive'da da
  95'tir; ancak dişlili bir direct drive ~400–450 olurdu. Doğrusu donanıma
  bakılarak kesinleşti.)*
- **Filament:** 1.75 mm
- **Nozul:** V-hotend (Creality özel)
- **E steps/mm:** 95

---

## 3. Firmware Derleme

### 3.1 Gereksinimler

- PlatformIO CLI (`pio` komutu erişilebilir olmalı)
- Python 3.x
- Git

### 3.2 Derleme Komutu

```bash
cd C:\Users\CNK\Desktop\Sermoon-D1
pio run
```

### 3.3 Derleme Sonucu (SD1-2.4, ölçüm 2026-07-24)

```
RAM:   [==        ]  20.1% (13,176 / 65,536 bytes)
Flash: [==        ]  24.2% (127,120 / 524,288 bytes)
SUCCESS — ~11 saniye
```

### 3.4 Binary Konumu

```
.pio\build\creality\firmware.bin
```

### 3.5 Bilinen Uyarılar (Hata değil)

Proje kodunda **uyarı yok** (SD1-2.1). Geriye yalnızca framework kaynaklı bir
uyarı kalır, bu bizim kodumuz değildir ve düzeltilemez:

- `util_adc.c:10` — `'adc_result' initialized and declared 'extern'`
  (maple framework, `framework-arduinoststm32-maple` paketi içinde)

Ayrıca `Warnings.cpp` bir bilgilendirme `#pragma message` basar (hata değil):
LIN_ADVANCE aktif, K kalibrasyonu hatırlatması.

> SD1-2.1 öncesinde `LCD_RTS.cpp`'de 3 adet "uninitialized variable" uyarısı
> vardı. Bunlar zararsız sanılıyordu ancak `axis` başlatılmamışken
> `current_position[axis]` **sınır dışı dizi yazması** anlamına geliyordu;
> değişkenler güvenli varsayılanlarla başlatılarak giderildi.

### 3.6 Build Sistemi Detayları

- Platform: `ststm32@<6.2.0`
- Board: `genericSTM32F103RE`
- Framework: Arduino (STM32duino maple)
- Linker script: `buildroot/share/PlatformIO/ldscripts/creality.ld`
- Vektör tablosu: `0x08007000` (creality.py ile relocate edilir)

---

## 4. Firmware Yükleme

### 4.1 SD Kart ile Yükleme

1. `firmware.bin` dosyasını FAT32 formatlı SD kartın kök dizinine kopyala
2. Yazıcı kapalıyken SD kartı tak
3. Yazıcıyı aç — bootloader otomatik firmware yükler
4. Yükleme ~10-20 saniye sürer
5. Ekran açıldığında "MarlinV2 by CTK" versiyonunu kontrol et

### 4.2 Önemli Notlar

- SD kartta başka `.bin` dosyası olmamalı
- Bootloader 0x08007000 adresine yazar — creality.py ve creality.ld bu offset'i sağlar
- Yükleme sonrası EEPROM ayarları sıfırlanabilir → `M502` + `M500` ile fabrika ayarlarını geri yükle

---

## 5. Step ve Hareket Ayarları

### 5.1 Steps Per Unit

```cpp
#define DEFAULT_AXIS_STEPS_PER_UNIT { 80, 79.60, 400, 95 }
```

| Eksen | Steps/mm | Açıklama |
|-------|----------|----------|
| X | 80.00 | GT2 kayış, 20 diş çark, 200 step rev, ×16 microstep |
| Y | 79.60 | **Farklı** — muhtemelen fabrika kalibrasyonu |
| Z | 400.00 | T8 × 8mm kurşun mil (lead screw) |
| E | 95.00 | **Direct drive** ekstruder (dişlisiz MK8 tipi besleyici) |

> ⚠️ **Y steps = 79.60** — Teorik olarak X ile aynı mekanik yapıda 80 olmalı. Boyutsal test baskısı ile doğrula. Sapma varsa M92 ile ayarla.

### 5.2 Maksimum Hızlar

```cpp
#define DEFAULT_MAX_FEEDRATE { 250, 250, 5, 25 }  // mm/s
```

| Eksen | Max hız | Not |
|-------|---------|-----|
| X | 250 mm/s | 300→250: yüksek hızda gövde rezonansı (StealthChop limiti *değil*) |
| Y | 250 mm/s | Aynı gerekçe |
| Z | 5 mm/s | Kurşun mil, yüksek tork gerektirir |
| E | 25 mm/s | Ekstruder filaman besleme |

### 5.3 İvmeleme

```cpp
#define DEFAULT_MAX_ACCELERATION      { 800, 800, 100, 5000 }  // mm/s²
#define DEFAULT_ACCELERATION          500    // mm/s² (print)
#define DEFAULT_RETRACT_ACCELERATION  5000   // mm/s² (retract)
#define DEFAULT_TRAVEL_ACCELERATION   800    // mm/s² (travel)
```

| Parametre | Değer | Açıklama |
|-----------|-------|----------|
| X/Y max ivme | 800 mm/s² | Ringing olmayan güvenli değer |
| Z max ivme | 100 mm/s² | Kurşun mil |
| E max ivme | 5000 mm/s² | Hızlı retract |
| Print ivme | 500 mm/s² | Normal baskı hareketleri |
| Travel ivme | 800 mm/s² | Baskısız hareket |

---

## 6. Homing Yapılandırması

### 6.1 Homing Parametreleri

```cpp
#define HOMING_FEEDRATE_XY    1000     // mm/m (X/Y homing hızı)
#define HOMING_FEEDRATE_Z     (4*60)   // mm/m = 240 mm/m (Z homing hızı)
#define X_HOME_DIR            -1       // Sol (MIN yönüne)
#define Y_HOME_DIR            -1       // Arka (MIN yönüne)
#define Z_HOME_DIR            -1       // Aşağı (MIN yönüne)
#define X_HOME_BUMP_MM        5        // İkinci dokunma mesafesi
#define Y_HOME_BUMP_MM        5
#define Z_HOME_BUMP_MM        2
#define HOMING_BUMP_DIVISOR   { 2, 2, 4 }  // İkinci geçiş hız böleni
#define HOMING_BACKOFF_MM     { 2, 2, 2 }  // Homing sonrası geri çekilme
#define QUICK_HOME                       // X+Y aynı anda home
#define IMPROVE_HOMING_RELIABILITY       // Homing boyunca X/Y ivmesi → 100 mm/s²
```

> **`IMPROVE_HOMING_RELIABILITY` SD1-2.6'ya kadar etkisizdi.** Tanım
> `Configuration_adv.h`'ın TMC bölümünde, `#if HAS_TRINAMIC` bloğunun içindeydi;
> bu kartta `HAS_TRINAMIC` false olduğu için makro hiç tanımlanmıyordu ve
> homing tam ivmeyle (800 mm/s²) çalışıyordu. SD1-2.6'da `@section homing`
> altına taşındı. `CLASSIC_JERK` kapalı olduğundan yalnızca ivme dalı
> derlenir; jerk'e dokunulmaz.

### 6.2 Homing Akışı

Aşağıdaki değerler ölçülmüş yapılandırmadan türetilmiştir
(`max_length` X = 290−(−10) = 300 mm, Y = 270−(−10) = 280 mm;
`HOMING_FEEDRATE_XY` 1000 mm/dk = 16,67 mm/s).

```
0. X/Y ivmesi geçici 100 mm/s²'ye çekilir     (IMPROVE_HOMING_RELIABILITY)
1. Z, Z_HOMING_HEIGHT = 4 mm kaldırılır       (X/Y'den ÖNCE, çarpma payı)
2. QUICK_HOME — tek çapraz hamle:
      hedef (−450, −420) = 1.5 × max_length × yön
      hız   22,8 mm/s    = 16,67 × √((280/300)² + 1)
      → X ve Y endstop'larına aynı anda dayanır
3. X ekseni (HOME_Y_BEFORE_X kapalı → X önce):
      hızlı −450 mm @ 16,67 mm/s   (endstop zaten basılı, anında biter)
      geri  +5 mm                   (X_HOME_BUMP_MM)
      yavaş −10 mm @ 8,33 mm/s      (bölen 2) ← gerçek sıfır burada
      X = X_MIN_POS = −10, sonra backoff +2 → X = −8
4. Y ekseni: aynı şablon (hızlı hamle −420 mm) → Y = −8
5. Z: (145, 135)'e git, 240 mm/dk ile dokun, 2 mm bump, bölen 4 → 1 mm/s
6. İvme orijinal değerine geri yüklenir
```

> **G28 sonrası nozul (−8, −8) konumundadır** — tabla dışında, her iki eksende
> 8 mm. `X_MIN_POS`/`Y_MIN_POS` = −10 olduğu için bu tasarım gereğidir.

### 6.3 Z Home Offset

```cpp
#define MANUAL_Z_HOME_POS    -1       // Z home = -1mm koordinatında
#define Z_MIN_POS            -1       // Minimum Z pozisyonu
#define Z_AFTER_HOMING       0        // Homing sonrası Z pozisyonu
```

> Tabla ayar vidalarının range sonu için Z=0 referansı 1mm yukarı kaydırılmış. Mekanik trigger Z=-1'de, Z=0 trigger'dan 1mm yukarıda.

### 6.4 Z Safe Homing

```cpp
#define Z_SAFE_HOMING
#define Z_SAFE_HOMING_X_POINT  ((X_BED_SIZE) / 2)  // 145 mm
#define Z_SAFE_HOMING_Y_POINT  ((Y_BED_SIZE) / 2)  // 135 mm
```

- Z homing sadece X/Y homing sonrası yapılabilir
- Z home öncesi tabla ortasına (145, 135) gider

> **Probe olmadan da anlamlı.** Z_SAFE_HOMING başlangıçta probe güvenliği için
> eklenmişti, ancak probe kaldırıldıktan sonra da faydası sürüyor: nozul köşe
> yerine tablanın ortasında iniyor (köşe klipsi/yüzey bozukluğu riski yok) ve
> Z=0 referansı her G28'de aynı X/Y noktasında ölçülüyor — tabla tam düz
> olmadığında bu tekrarlanabilirlik demektir.
>
> **Kapatmayın** — kapatmak Z=0 referansını değiştirir ve ilk katman
> yüksekliğinizin yeniden kalibrasyonunu gerektirir.

---

## 7. Hareket Algoritmaları

### 7.1 Algoritma Özeti

| Algoritma | Durum | Kalite Etkisi | Açıklama |
|-----------|-------|---------------|----------|
| S-Curve Acceleration (Bézier) | ✅ Aktif | 9/10 | Düz ivmeleme yerine eğrisel geçiş |
| Junction Deviation | ✅ Aktif | 9/10 | Adaptif köşe hızı (Classic Jerk yerine) |
| Linear Advance (K=0.06) | ⚠️ Aktif, **K kalibre edilmemiş** | 8/10 | Direct drive — K kalibre edilmeli (§9) |
| Adaptive Step Smoothing | ✅ Aktif | 7/10 | Düşük hızlarda step kalitesi |
| Arc Support + Bézier | ✅ Aktif | 7/10 | G2/G3 yay hareketleri |
| ~~HYBRID_THRESHOLD~~ | ❌ **Hiç derlenmiyor** | — | `#if HAS_TRINAMIC` içinde (§8.1) |
| `ADAPTIVE_STEP_SMOOTHING` | ✅ Aktif | 9/10 | HR4988SQ'da interpolasyon olmadığı için kritik (§8.4) |
| Classic Jerk | ❌ Kapalı | — | Junction Deviation kullanılıyor |
| Backlash Compensation | ❌ Kapalı | — | Gerekirse açılabilir |

### 7.2 S-Curve Acceleration

```cpp
#define S_CURVE_ACCELERATION
```

- Düz (trapez) ivmeleme yerine Bézier eğrisi kullanır
- Hareket başlangıç ve bitişinde daha yumuşak geçiş
- Ringing (çizgi izi) azalır
- `BEZIER_CURVE_SUPPORT` da aktif (G5 komutu desteği)

### 7.3 Junction Deviation

```cpp
#define JUNCTION_DEVIATION_MM 0.013
```

- Classic Jerk yerine modern adaptif köşe hızı algoritması
- Köşe açısına göre otomatik hız azaltma
- `DEFAULT_EJERK 5.0` — Ekstruder jerk (her iki modda da aktif)
- Değer 0.013 — Sermoon D1 boyutları için optimize

### 7.4 Planner Parametreleri

```cpp
#define MINIMUM_PLANNER_SPEED    0.05  // mm/s (minimum planlanan hız)
#define MIN_STEPS_PER_SEGMENT    6     // Segment başına minimum step
```

---

## 8. Sürücü Ayarları (karma: TMC2208 + HR4988SQ)

> ⚠️ **Bu bölüm 2026-07-23'te tamamen yeniden yazıldı.** Eski hâli
> `STEALTHCHOP_*`, `HYBRID_THRESHOLD` ve `INTERPOLATE` ayarlarını etkinmiş gibi
> anlatıyordu. Hiçbiri etkin değil — hepsi `Configuration_adv.h`'daki
> `#if HAS_TRINAMIC` bloğunun içinde ve **bu blok bu kartta derlenmiyor**.
> Bu ayarları değiştirmiş olsanız bile yazıcının davranışı değişmemiştir.

### 8.1 Neden hiçbir TMC ayarı çalışmıyor

`Marlin/src/core/drivers.h:80` bunu açıkça söylüyor:

> *"Test for supported TMC drivers that require advanced configuration —
> **Does not match standalone configurations**"*

`HAS_TRINAMIC` yalnızca **UART/SPI ile yapılandırılabilir** TMC sürücüler için
true olur. Bu kartta:

| Eksen | `*_DRIVER_TYPE` | `HAS_TRINAMIC`'e girer mi? |
|---|---|---|
| X, Y | `TMC2208_STANDALONE` | Hayır — `_STANDALONE` eşleşmez |
| Z, E0 | `A4988` (HR4988SQ) | Hayır — TMC değil |

Sonuç `HAS_TRINAMIC = false`. Blok içindeki **`*_CURRENT`, `*_MICROSTEPS`,
`*_RSENSE`, `INTERPOLATE`, `STEALTHCHOP_*`, `HYBRID_THRESHOLD`,
`CHOPPER_TIMING`, `MONITOR_DRIVER_STATUS` hiç derlenmez.**

Dolayısıyla şu G-code'lar da **yoktur**: `M906` (akım), `M569` (chop modu),
`M350` (mikroadım), `M122` (sürücü durumu). Sensorless homing da yok.

### 8.2 Gerçekte neyin neyi belirlediği

| Ayar | Nerede belirlenir | Yazılımdan değişir mi? |
|---|---|---|
| Akım | Sürücü üzerindeki **Vref potu** | ❌ Hayır |
| Mikroadım | Sürücüde **PCB'ye sabit kablanmış MS1/MS2** (16×, jumper yok) | ❌ Hayır |
| Chopper modu | Çipin donanım varsayılanı | ❌ Hayır |
| 256× interpolasyon | TMC2208'in kendi donanımı (**yalnızca X/Y**) | ❌ Hayır |
| Adım darbe genişliği | `MINIMUM_STEPPER_PULSE` | ✅ Evet |
| DIR kurulum süresi | `MINIMUM_STEPPER_*_DIR_DELAY` | ✅ Evet |
| Adım yumuşatma | `ADAPTIVE_STEP_SMOOTHING` | ✅ Evet |

### 8.3 Firmware'in gerçekten kontrol ettiği üç değer

Karma yapılandırmada bu makrolar **global**'dir (eksen başına ayarlanamaz), bu
yüzden her zaman **en katı** gereksinim geçerlidir:

```cpp
#define MINIMUM_STEPPER_PULSE          1       // µs  — HR4988SQ belirliyor
#define MAXIMUM_STEPPER_RATE           400000  // Hz  — TMC2208 belirliyor
#define MINIMUM_STEPPER_POST_DIR_DELAY 200     // ns  — HR4988SQ belirliyor
#define MINIMUM_STEPPER_PRE_DIR_DELAY  200     // ns
```

| Makro | Değer | Belirleyen | Diğerinin isteği |
|---|---|---|---|
| `MINIMUM_STEPPER_PULSE` | 1 µs | HR4988SQ | TMC2208 ~100 ns yeterdi |
| `MAXIMUM_STEPPER_RATE` | 400 kHz | TMC2208 | HR4988SQ 500 kHz kaldırırdı |
| `*_DIR_DELAY` | 200 ns | HR4988SQ | TMC2208 20 ns yeterdi |

> **SD1-2.4'teki kritik düzeltme:** `*_DIR_DELAY` daha önce **30 ns** idi
> (TMC2208'e göre ayarlanmıştı) ve HR4988SQ için **yetersizdi**. DIR pini STEP
> kenarından yeterince önce kararlı değilse sürücü adımı **eski yönde** atar.
> En çok etkilenen yollar yön değişiminin sık olduğu yerlerdir: Z'de katman
> geçişi (paralel iki motor birlikte yanlış yöne gider) ve E'de her retract.
> Belirtisi katman kayması ve retract sonrası tutarsız akıştır.

`MAXIMUM_STEPPER_RATE` pratikte bağlayıcı değil — en hızlı eksen X/Y,
250 mm/s × 80 step/mm = 20 kHz, tavanın %5'i. Ancak `stepper.h:160`'ta darbe
tabanını da belirler: 72 MHz / 400000 = 180 çevrim = **2.5 µs**, HR4988SQ'nun
istediği 1 µs'nin rahatça üstünde.

### 8.4 ADAPTIVE_STEP_SMOOTHING — HR4988SQ'da kritik

```cpp
#define ADAPTIVE_STEP_SMOOTHING
```

X/Y'de TMC2208 16× girişi çip içinde 256×'e interpole ettiği için hareket zaten
pürüzsüz. **Z/E0'da böyle bir şey yok** — 16× gerçekten 16×. Düşük ve orta step
frekanslarında adım basamakları duyulur hâle gelir; Z'de paralel iki motor
olduğu için titreşim daha da belirgindir.

`ADAPTIVE_STEP_SMOOTHING` bu aralıkta efektif step rate'i ikiye katlayarak
aradaki farkı kapatır. **HR4988SQ ile kapatılmamalıdır.**

### 8.5 Akım ayarı → §2.2

Vref değerleri, ölçülen akımlar, R_sense (0.15 Ω) ve iki sürücü ailesinin
farklı Vref formülleri **[bölüm 2.2](#22-step-motor-sürücüler)**'de.

Özet: **fabrika ayarı doğrulanmıştır, potlara dokunmayın.**

---

## 9. Linear Advance (LIN_ADVANCE)

### 9.1 Yapılandırma

```cpp
#define LIN_ADVANCE
#define LIN_ADVANCE_K 0.06   // mm compression per 1mm/s extruder speed
```

### 9.2 K Faktörü Açıklaması

- **0.06** — direct drive için makul başlangıç değeri (Sermoon D1 = direct drive)
- Direct drive tipik aralık: 0.02 – 0.15 (Sermoon D1 için geçerli aralık)
- Bowden tipik aralık: 0.4 – 0.9 (bu yazıcı için geçerli DEĞİL)
- **Kullanıcı kalibrasyonu şart** — E0 sürücüsü SD1-2.4'te değişti, başlangıç değeri sadece referans

### 9.3 K Faktörü Kalibrasyonu

1. Test deseni indir: https://marlinfw.org/tools/lin_advance/k-factor.html
2. Başlangıç K=0 ile test baskısı yap
3. Her satırda K değerini artır (0.02 artışla)
4. En düzgün extrusion çizgisine karşılık gelen K değerini seç
5. `M900 K0.06` → test et → `M500` ile kaydet

> ⚠️ Her filaman markası/değişimi için K farklı olabilir. PLA ile kalibre et, PETG/ABS için tekrar ölç.

---

## 10. Z-Probe ve Auto Bed Leveling (ABL)

> ## ⛔ BU BÖLÜM ŞU AN GEÇERLİ DEĞİL
>
> **Bu yazıcıda Z-probe YOK.** Ne BLTouch ne de endüktif sensör takılı.
> Aşağıdakiler **ileride probe eklenirse** izlenecek kurulum notlarıdır;
> mevcut firmware'in davranışını **anlatmaz**.
>
> Firmware'deki gerçek durum:
> - `FIX_MOUNTED_PROBE` → **kapalı** (`Configuration.h`)
> - `BLTOUCH` → **kapalı**
> - `AUTO_BED_LEVELING_*`, `MESH_BED_LEVELING` → **hepsi kapalı** → `HAS_LEVELING` false
> - Z homing: **mekanik endstop PA7**. G29 anlamlı iş yapmaz.
> - Tabla tesviyesi: manuel, 4 köşe vidası + DWIN ekranındaki "Assistant Level"
>   (ekran nozulu köşelere götürür) + Z offset ince ayarı (babystep).
>
> **Probe eklemeden önce zorunlu adım:** PB0/PB1 pinleri Z lock modülüne
> ayrılmıştır. Önce `Configuration_adv.h`'da `SERMOON_Z_LOCK` kapatılmalıdır.
> Kapatmadan probe açarsanız derleme `SanityCheck.h` hatası ile durur.
>
> ### ⚠️ Kaynak dosyaları artık ağaçta yok
>
> Sermoon-D1 **2.2** ölü kod temizliğinde probe ve tabla-tesviye kaynak
> dosyaları silindi (binary'ye zaten 0 byte katkıları vardı). Bu bölümü
> uygulamadan önce onları geri getirmen gerekir:
>
> ```bash
> # Temizlik öncesi taban commit'inden geri al
> git checkout 1d2ba27 -- \
>   Marlin/src/module/probe.cpp \
>   Marlin/src/feature/bedlevel \
>   Marlin/src/gcode/probe \
>   Marlin/src/gcode/bedlevel \
>   Marlin/src/gcode/calibrate/M48.cpp \
>   Marlin/src/gcode/calibrate/G425.cpp \
>   Marlin/src/libs/vector_3.cpp \
>   Marlin/src/libs/least_squares_fit.cpp \
>   Marlin/src/libs/least_squares_fit.h
> ```
>
> Başlık dosyaları (`probe.h`, `bedlevel.h` vb.) **silinmedi** — hâlâ ağaçta.
> Sadece `.cpp` gövdeleri ve yetim kalan iki başlık geri alınmalı.
> Tam liste: `git show --stat 1d2ba27..HEAD`.

### 10.1 Sensör Bilgileri (probe eklenirse — referans)

| Özellik | Değer |
|---------|-------|
| Sensör | BES M18MG-PSC15F-S04K |
| Tip | Endüktif yakınlık sensörü |
| Çıkış | PNP Normally Open (NO) |
| Besleme | 10–30V DC (24V kullanılıyor) |
| Algılama mesafesi | 15 mm (çelik) / **~5–7 mm (alüminyum)** |
| Boyut | M18 × ~65 mm |
| Algılama yüzeyi | Alüminyum tabla |

### 10.2 Donanım Kurulumu

#### 10.2.1 Sensör Montajı

- Sensörü print kafasına bracket ile monte et
- Sensör yüzü **nozul ucundan 2–3 mm yukarıda** olmalı
- Sensör merkezi mümkün olduğunca nozula yakın olmalı (X/Y offset küçük = doğru probing)
- M18 (18mm çap) — kafa tasarımında yer açılması gerekebilir

#### 10.2.2 Voltaj Bölücü Devresi (KRİTİK!)

```
⚠️ UYARI: Sensör çıkışı 24V, MCU 3.3V!
Doğrudan bağlarsan STM32 MCU yanar — board kullanılamaz hale gelir!

Voltaj bölücü ZORUNlu:
```

```
Sensör siyah kablo (sinyal, 24V)
         │
    [R1: 10kΩ]
         │
         ├──────── MCU PB1 pini (3.13V)
         │
    [R2: 1.5kΩ]
         │
        GND

Vout = 24V × R2/(R1+R2) = 24 × 1.5/11.5 = 3.13V ✓
```

**Kablo renkleri (Balluff standardı):**

| Renk | Fonksiyon | Bağlantı |
|------|-----------|----------|
| Kahverengi | VCC (10-30V) | Güç kaynağı 24V terminaline |
| Mavi | GND | Güç kaynağı GND + Board GND (ortak) |
| Siyah | Sinyal çıkış | R1 girişine → R1/R2 bölücü → PB1 |

#### 10.2.3 Board Bağlantı Noktası

- **PB1 pini:** Board üzerindeki BLTouch konnektörünün sinyal pinine karşılık gelir
- Sensör 24V beslemesi: Güç kaynağı terminalinden alınacak
- Board 5V çıkışı kullanılmayacak — sensör 10-30V gerektirir
- GND: Sensör ve board ortak toprak

### 10.3 Firmware Yapılandırması

#### 10.3.1 Pin Tanımı (`pins_CREALITY.h`)

> ⚠️ **PB1 boşta değil.** PB0 ve PB1 `SERMOON_Z_LOCK` tarafından OUTPUT olarak
> sürülüyor. Probe'u PB1'e almak için önce Z lock kapatılmalı — aynı pini hem
> output hem input yapmak, `endstops.init()` `zlock.init()`'ten sonra çalıştığı
> için Z lock'ı sessizce devre dışı bırakır ve `M888`'i probe pininin bias'ını
> oynatan bir komuta dönüştürür.

```cpp
// MEVCUT DURUM:
#define Z_MIN_PIN        PA7   // Mekanik endstop — homing
// Z_MIN_PROBE_PIN tanımlı DEĞİL (probe yok)
#define Z_KEEP_PIN_PB0   PB0   // Z lock
#define Z_KEEP_PIN_PB1   PB1   // Z lock

// PROBE EKLENİRSE: önce SERMOON_Z_LOCK kapat, sonra
// #define Z_MIN_PROBE_PIN PB1
```

#### 10.3.2 Probe Ayarları (`Configuration.h`) — probe eklenirse

```cpp
#define FIX_MOUNTED_PROBE                              // Sabit monte sensör (ŞU AN KAPALI)
//#define Z_MIN_PROBE_USES_Z_MIN_ENDSTOP_PIN           // Ayrı pin kullanılıyor
#define Z_MIN_PROBE_ENDSTOP_INVERTING false            // PNP NO: HIGH = trigger
#define NOZZLE_TO_PROBE_OFFSET { 0, 0, 2 }            // Başlangıç — ölç ve kalibre et!
#define MIN_PROBE_EDGE            30                   // Tabla kenarından minimum mesafe
#define XY_PROBE_SPEED            3000                 // Probe noktaları arası hız (mm/m)
#define Z_PROBE_SPEED_FAST        HOMING_FEEDRATE_Z    // 240 mm/m
#define Z_PROBE_SPEED_SLOW        (Z_PROBE_SPEED_FAST / 2)  // 120 mm/m
#define MULTIPLE_PROBING          2                    // Her noktada 2× prob
#define Z_CLEARANCE_DEPLOY_PROBE  10                   // Probe öncesi Z yüksekliği
#define Z_CLEARANCE_BETWEEN_PROBES 5                   // Probe noktaları arası Z yüksekliği
#define Z_PROBE_LOW_POINT         -2                   // Trigger altı minimum mesafe
#define Z_PROBE_OFFSET_RANGE_MIN  -10                  // M851 minimum
#define Z_PROBE_OFFSET_RANGE_MAX  10                   // M851 maksimum
```

#### 10.3.3 NOZZLE_TO_PROBE_OFFSET Detayı

```
{ X, Y, Z }

X → Sensör nozulun SAĞINDAYSa pozitif (+), solundaysa negatif (-)
Y → Sensör nozulun ARKASINDAYSA pozitif (+), önündeyse negatif (-)
Z → Sensör yüzü nozul ucundan YUKARIDA pozitif (+), aşağıda negatif (-)
```

**Örnek:** Sensör nozulun 10mm sağına, 5mm arkasına ve 3mm yukarısına monte edilmişse:
```cpp
#define NOZZLE_TO_PROBE_OFFSET { 10, 5, 3 }
```

> ⚠️ Bu değer sensör monte edildikten sonra kumpas/mikrometre ile ölçülmelidir. {0, 0, 2} başlangıç değeridir.

#### 10.3.4 ABL Ayarları

> ⛔ **Bunların hiçbiri şu an açık değil.** `Configuration.h`'da tüm
> `AUTO_BED_LEVELING_*` ve `MESH_BED_LEVELING` satırları yorumda; `HAS_LEVELING`
> false. Aşağıdaki blok probe eklendikten **sonra** açılacak ayarların
> referansıdır — mevcut firmware'i tarif etmez.

```cpp
// ŞU AN HEPSİ KAPALI — probe eklenirse açılacak:
//#define AUTO_BED_LEVELING_BILINEAR   // Grid bazlı seviyeleme
//#define GRID_MAX_POINTS_X            4   // 4×4 = 16 probe noktası
//#define ENABLE_LEVELING_FADE_HEIGHT  // Yükseklikte düzeltme azalması
//#define SEGMENT_LEVELED_MOVES        // Mesh üzerinde segmentli hareket
```

**Grid görselleştirme (4×4):**

```
   ○──○──○──○   ← Y = 270 - 30 = 240 (MIN_PROBE_EDGE)
   │           │
   ○──○──○──○
   │           │
   ○──○──○──○
   │           │
   ○──○──○──○   ← Y = 30
   ↑           ↑
  X=30       X=260

Tablo boyutu: 290 × 270 mm
Probe alanı: 230 × 210 mm (30mm kenar boşluğu)
```

### 10.4 Kalibrasyon Prosedürü

#### Adım 1: Sensör Testi

```
M119              ← Endstop durumlarını kontrol et
```

Beklenen çıktı:
- Sensör bed'den uzakta: `z_min: TRIGGERED` (mechanical) + `z_probe: OPEN`
- Sensör bed'e yakın: `z_probe: TRIGGERED`
- Durum ters ise → `Z_MIN_PROBE_ENDSTOP_INVERTING` değerini değiştir

#### Adım 2: İlk Probe

```
M851 Z0           ← Probe offset sıfırla
G28               ← Home all (Z_SAFE_HOMING → tabla ortasına gider)
G29               ← 4×4 grid prob yap
M420 V            ← Mesh verisini göster
```

#### Adım 3: Z Offset Kalibrasyonu

```
G28               ← Home all
G29               ← Probe
G1 X145 Y135 Z0   ← Nozulu tabla ortasına getir
```

Kağıt testi yap:
- Kağıt nozul altında sıkışıyor → Z offset çok düşük → `M851 Z-0.1` (negatife doğru)
- Kağıt nozul altında gevşek → Z offset çok yüksek → `M851 Z0.1` (pozitife doğru)
- Kağıt hafif sürtünmeyle geçiyor → TAMAM

```
M500              ← EEPROM'a kaydet
```

#### Adım 4: Slicer Start G-Code

Slicer'ın (Cura, PrusaSlicer, vs.) start G-code bölümüne ekle:

```gcode
G28                ; Home all axes
G29                ; Auto bed level (4×4 grid)
; M420 S1          ; G29 zaten mesh'i aktif eder, yedek olarak
```

### 10.5 Probe ile İlgili G-Code Komutları

| Komut | Açıklama |
|-------|----------|
| `G28` | Home all (Z_SAFE_HOMING aktif) |
| `G29` | ABL prob yap (4×4 grid) |
| `G30` | Tek nokta prob |
| `M420 S1` | Mesh'i aktif et |
| `M420 S0` | Mesh'i devre dışı bırak |
| `M420 V` | Mesh verisini yazdır |
| `M420 Z10` | Fade height 10mm ayarla |
| `M851 Z0.5` | Probe Z offset ayarla |
| `M851` | Mevcut probe offset göster |
| `M48 P10` | 10× tekrarlanabilirlik testi |
| `M500` | EEPROM'a kaydet |
| `M503` | Tüm ayarları göster |

---

## 11. Sıcaklık ve Güvenlik

### 11.1 Sıcaklık Limitleri

| Bileşen | Min | Max | Açıklama |
|---------|-----|-----|----------|
| Hotend (E0) | 0°C | 275°C | V-hotend |
| Bed | 0°C | 110°C | Alüminyum + ısıtıcı |
| Extrude min | 180°C | — | Altında extrusion engellenir |

### 11.2 Ön Isıtma Profilleri

| Profil | Hotend | Bed | Kullanım |
|--------|--------|-----|----------|
| PLA | 195°C | 45°C | Standart |
| ABS/PETG | 240°C | 100°C | Yüksek sıcaklık |

### 11.3 Termal Korumalar

```cpp
#define THERMAL_PROTECTION_HOTENDS   // Hotend termal runaway koruması
#define THERMAL_PROTECTION_BED       // Bed termal runaway koruması
```

Her iki koruma da aktif. Sensör arızası durumunda ısıtıcı otomatik kapanır.

---

## 12. EEPROM ve SD Kart

### 12.1 EEPROM

```cpp
#define EEPROM_SETTINGS              // Ayarları EEPROM'a kaydet (M500)
#define EEPROM_AUTO_INIT             // İlk açılışta varsayılanları yaz
```

- Tip: I2C EEPROM (24C16, 16Kb)
- Pinler: SDA=PA11, SCL=PA12
- `M500` → kaydet, `M501` → yükle, `M502` → fabrika ayarları, `M503` → göster

### 12.2 SD Kart

```cpp
#define SDSUPPORT                    // SD kart desteği
#define SD_DETECT_PIN      PC7       // SD kart algılama pini
```

- Firmware yükleme: SD kart ile (FAT32 format)
- Baskı dosyaları: SD karttan okunur
- Maksimum dosya adı: uzun dosya adı desteği aktif

### 12.3 Baskı İstatistikleri

```cpp
#define PRINTCOUNTER                 // Baskı sayacı aktif
```

---

## 13. DWIN Ekran Entegrasyonu

### 13.1 Ekran Yapılandırması

```cpp
#define SizeofDatabuf      40        // Veri tampon boyutu (26 → 40 düzeltme)
#define FIRMWARE_VERSION   "MarlinV2 by CTK"
#define MACHINE_TYPE       "Sermoon D1"
#define HARDWARE_VERSION   "HW 4.3.1"
#define SCREEN_VERSION     "DWIN 1.1.14"
#define PRINT_SIZE         "280*260*310"
```

### 13.2 DWIN Protokolü

- RTS (Real-Time Serial) protokolü ile MCU-DWIN iletişimi
- `SizeofDatabuf = 40` — orijinal Creality kodunda 26 byte idi, taşma düzeltmesi için 40'a çıkarıldı
- Versiyon string ~14 karakterle sınırlı (ekran alan kısıtlaması)

### 13.3 Ekran Üzerinden Ayarlar

- Babystepping aktif (`BABYSTEPPING`)
- `BABYSTEP_ZPROBE_OFFSET` — M851 Z ile babystep senkronize
- Z offset ayarı ekrandan yapılabilir

---

## 14. Pin Haritası

### 14.1 Endstop Pinleri

| Fonksiyon | Pin | MCU Pin | Not |
|-----------|-----|---------|-----|
| X_MIN | PA5 | PA5 | Mekanik endstop |
| Y_MIN | PA6 | PA6 | Mekanik endstop |
| Z_MIN | PA7 | PA7 | Mekanik endstop (homing) |
| Z_MIN_PROBE | — | — | **Tanımsız** — probe yok, PB1 Z lock'a ayrılmış |

### 14.2 Step Motor Pinleri

| Fonksiyon | Enable | Step | Dir | Sürücü |
|-----------|--------|------|-----|--------|
| X | PC3 | PC2 | PB9 | TMC2208 standalone |
| Y | PC3 | PB8 | PB7 | TMC2208 standalone |
| Z | PC3 | PB6 | PB5 | **HR4988SQ** — bu tek sete **paralel 2 motor** |
| E0 | PC3 | PB4 | PB3 | **HR4988SQ** |

> **Ortak enable pini (PC3), aktif low.** Marlin bu pini eksen bazında saymaz:
> `disable_Z()` çağrısı PC3'ü pasife çekerek X/Y/E0'ı da bırakır. Pratikte
> sorun çıkarmaz çünkü eksenler yalnızca hepsi birden boşta kalınca kapatılır
> (`DEFAULT_STEPPER_DEACTIVE_TIME` + tüm `DISABLE_INACTIVE_*` true).
> **Sonucu:** tek bir ekseni ısınma nedeniyle bağımsız kapatmak mümkün değil,
> ve `Configuration.h`'daki `DISABLE_X/Y/Z/E` hepsi `false` kalmalıdır.

> **PB3/PB4 JTAG hattıdır** (JTDO / JNTRST) ve E0_DIR/E0_STEP olarak
> kullanılır. `pins_CREALITY.h`'daki `DISABLE_DEBUG` tanımı kaldırılırsa bu
> pinler GPIO'ya dönmez ve **ekstruder hiç hareket etmez**.

> **Z için Z2_* pin tanımı YOKTUR ve olmamalıdır.** İki motor tek sürücüye
> paralel bağlı; `Z2_DRIVER_TYPE` ikinci bir bağımsız sürücü demektir, öyle
> bir donanım yok.

### 14.3 Sıcaklık ve Isıtıcı Pinleri

| Fonksiyon | Pin |
|-----------|-----|
| Hotend termistör | PC5 |
| Bed termistör | PC4 |
| Hotend ısıtıcı | PA1 |
| Bed ısıtıcı | PA2 |
| Fan | PA0 |

### 14.4 Diğer Pinler

| Fonksiyon | Pin | Not |
|-----------|-----|-----|
| SD Detect | PC7 | SD kart algılama |
| Filament runout | PA4 | Optik sensör |
| I2C EEPROM SDA | PA11 | |
| I2C EEPROM SCL | PA12 | |
| Z Lock IN | PB1 | Board'un "BLTouch" konnektörü — tamamen Z lock'a ayrılmış |
| Z Lock OUT | PB0 | Aynı konnektör |

---

## 15. Kalibrasyon Kılavuzu

### 15.1 Boyutsal Doğruluk Testi

1. 20×20×20 mm küp yazdır
2. X, Y, Z boyutlarını dijital kumpas ile ölç
3. Sapma varsa:

```
M92 X80.00       ← X steps/mm ayarla
M92 Y79.60       ← Y steps/mm ayarla (gerekirse düzelt)
M92 Z400.00      ← Z steps/mm
M92 E95.00       ← E steps/mm
M500              ← EEPROM'a kaydet
```

### 15.2 LIN_ADVANCE_K Kalibrasyonu

1. https://marlinfw.org/tools/lin_advance/k-factor.html adresinden test deseni oluştur
2. K=0 ile başla, her satırda K'yı 0.02 artır
3. En tutarlı extrusion çizgisini seç
4. `M900 K<yeni_değer>` → `M500`

### 15.3 Z Offset (İlk Katman) Kalibrasyonu

Probe olmadığı için `M851` kullanılmaz. Z offset **babystep** ile ayarlanır:

1. `G28` — home yap
2. Bir test baskısı başlat (tek katmanlı kare ideal)
3. İlk katman yazılırken DWIN ekranında **Adjust → Z offset ± butonları**
   - Her basış 0.1 mm (`LCD_RTS.cpp`, `babystep.add_mm`)
   - Nozul çok yakınsa (+), çok uzaksa (−)
4. Sonuç iyiyken `M500` — EEPROM'a kaydet

> Ekrandaki Z offset değeri DWIN sürücüsünün kendi `zprobe_zoffset` değişkenidir
> ve doğrudan babystep'e uygulanır; Marlin'in probe offset'i ile ilgisi yoktur.
> Bu yüzden probe olmadan da tam çalışır.

### 15.4 E Steps Kalibrasyonu

1. Filamentı ekstruder girişine kadar it
2. Filament üzerine 120mm işaretle
3. `G1 E100 F100` — 100mm extrude et
4. İşaretten nozzle'a kalan mesafeyi ölç
5. Kalan = 20mm ise doğru (120-100=20)
6. Sapma varsa:

```
M92 E<yeni_steps>   ← mevcut_steps × (100 / gerçek_extrude_mm)
M500
```

---

## 16. G-Code Referansı

### 16.1 Temel Hareket

| Komut | Açıklama |
|-------|----------|
| `G0 X Y Z F` | Hızlı hareket (travel) |
| `G1 X Y Z E F` | Doğrusal hareket (print) |
| `G28` | Home all axes |
| `G29` | Auto bed level |
| `G92 X Y Z E` | Pozisyon sıfırla |

### 16.2 Kalibrasyon

| Komut | Açıklama |
|-------|----------|
| `M851 Z<değer>` | Z probe offset ayarla |
| `M92 X Y Z E` | Steps/mm ayarla |
| `M900 K<değer>` | Linear Advance K faktörü |
| `M201 X Y Z E` | Maksimum ivmeleme |
| `M203 X Y Z E` | Maksimum hız |
| `M204 P R T` | İvmeleme (Print/Retract/Travel) |
| `M205 J<değer>` | Junction deviation |
| `M206 X Y Z` | Home offset |

### 16.3 Probe ve Leveling — **bu firmware'de yok**

`G29`, `G30`, `M420`, `M421`, `M48`, `M851` komutlarının **hiçbiri derlenmedi**.
Probe donanımı olmadığı için `HAS_BED_PROBE` ve `HAS_LEVELING` false; bu
komutlar gönderilirse yazıcı `unknown command` döner.

Tabla tesviyesi bu yazıcıda **manueldir**:

| Yöntem | Açıklama |
|--------|----------|
| 4 köşe vidası | Klasik kağıt yöntemi |
| DWIN "Assistant Level" | Ekran nozulu sırayla 5 noktaya götürür (köşeler + orta) |
| Z offset babystep | İlk katman ince ayarı — [bölüm 15.3](#153-z-offset-ilk-katman-kalibrasyonu) |

### 16.3b Sermoon'a Özgü

| Komut | Açıklama |
|-------|----------|
| `M888` | Z lock durumunu sorgula |
| `M888 S0` | Z lock serbest bırak |
| `M888 S1` | Z lock kilitle (varsayılan) |

### 16.4 EEPROM

| Komut | Açıklama |
|-------|----------|
| `M500` | EEPROM'a kaydet |
| `M501` | EEPROM'dan yükle |
| `M502` | Fabrika ayarlarına dön |
| `M503` | Tüm ayarları göster |

### 16.5 Tanı ve Test

| Komut | Açıklama |
|-------|----------|
| `M119` | Endstop durumlarını göster |
| `M115` | Firmware versiyonunu göster |
| `M48 P10` | Probe tekrarlanabilirlik testi |
| `M111 S32` | Leveling debug log aç |

---

## 17. Sorun Giderme

### 17.1 Derleme Hataları

| Sorun | Çözüm |
|-------|-------|
| `pio: command not found` | PlatformIO CLI kurulu değil — `pip install platformio` |
| `No module named platformio` | Python 3.14 uyumsuzluğu — `pio` CLI komutunu kullan |
| HAL dosya hatası | STM32 platform sürümü kontrol et — `ststm32@<6.2.0` gerekli |

### 17.2 Firmware Yükleme Sorunları

| Sorun | Çözüm |
|-------|-------|
| Ekran açılmıyor | SD kartı çıkar, yeniden başlat. Bootloader beklemiyor olabilir |
| Versiyon güncellenmedi | SD kartta eski .bin dosyası kalmış olabilir — sil ve tekrar yükle |
| EEPROM hatası | `M502` → `M500` ile fabrika ayarlarını geri yükle |

### 17.3 Homing ve Z Lock Sorunları

Probe olmadığı için probe sorun giderme bölümü kaldırıldı. Bunun yerine:

| Sorun | Çözüm |
|-------|-------|
| `M119` z_min her zaman TRIGGERED | Mekanik endstop (PA7) sıkışmış veya kablo kopuk; `Z_MIN_ENDSTOP_INVERTING` kontrol et |
| Z home etmiyor | `M119` ile endstop tetikleniyor mu bak; `Z_HOMING_HEIGHT 4` yeterli boşluk bırakıyor mu |
| G28 sonrası nozul tablaya çok yakın/uzak | Z offset babystep ile ayarla ([bölüm 15.3](#153-z-offset-ilk-katman-kalibrasyonu)), `M500` |
| Z ekseni kendiliğinden düşüyor | `M888` ile lock durumunu sorgula; `ENGAGED` değilse `M888 S1` |
| `M888` cevap vermiyor | `SERMOON_Z_LOCK` derlenmiş mi kontrol et (`Configuration_adv.h`) |

> **Not:** PB0/PB1 Z lock'a ayrıldığı için `M42 P` ile bu pinlere elle yazmayın —
> lock durumunu bozarsınız.

### 17.4 Baskı Kalite Sorunları

| Sorun | Çözüm |
|-------|-------|
| Ringing (çizgi izi) | `DEFAULT_ACCELERATION` düşür (500→300), `JUNCTION_DEVIATION_MM` düşür |
| Layer shift (X/Y) | Vref 1.27 V mi ölç (0.69 A RMS). Kayış gerginliği ve kasnak vidası. `HYBRID_THRESHOLD` **aramayın — bu kartta yok** |
| Layer shift (Z) veya retract sonrası bozuk akış | `MINIMUM_STEPPER_*_DIR_DELAY` **200** mü kontrol et. 30 ns'de HR4988SQ ters yönde adım atar (§8.3) |
| İlk katman yapışmıyor | Z offset'i babystep ile kalibre et ([15.3](#153-z-offset-ilk-katman-kalibrasyonu)), tablayı manuel tesviye et |
| Extrusion tutarsız | `LIN_ADVANCE_K` kalibre et, E steps doğrula |
| Yüzey kalitesiz / Z'de duyulur basamak | `ADAPTIVE_STEP_SMOOTHING` açık mı kontrol et. `STEALTHCHOP`/`INTERPOLATE` **bu kartta yok** (§8.1) — Z/E0'da HR4988SQ interpolasyon yapmaz |

### 17.5 Sensör Voltaj Bölücü Doğrulama — *yalnızca probe eklenirse*

> Bu adımlar mevcut yazıcı için geçerli değildir (probe yok). İleride endüktif
> sensör takılırsa, **`SERMOON_Z_LOCK` kapatıldıktan sonra** kullanılacaktır.

Multimetre ile ölç:
1. Sensör güçsüz (metal yakınında değil) → siyah kablo ≈ 0V
2. Sensör tetiklenmiş (metal yakınında) → siyah kablo ≈ 24V
3. Voltaj bölücü çıkışı (R1-R2 junction) → tetikte ≈ 3.1V, tetiksiz ≈ 0V

---

## 18. Değişiklik Günlüğü

### Versiyon: MarlinV2 by CTK — SD1-2.1 (2026-07-21)

#### Oturum 4 — Donanım Gerçeğiyle Hizalama + Kritik Düzeltmeler

- **Z-probe kaldırıldı.** Yazıcıda probe donanımı yok. `FIX_MOUNTED_PROBE`
  kapatıldı; PB0/PB1 tamamen Z lock modülüne ayrıldı. Oturum 3'te eklenen probe
  entegrasyonu, Z lock ile aynı pinleri kullandığı için **çakışıyordu**:
  `endstops.init()` `zlock.init()`'ten sonra çalıştığı için PB1'deki Z lock
  boot'ta sessizce devre dışı kalıyordu.
- **PLR (elektrik kesintisi kurtarma) düzeltildi.** `E2END` 0x800 → 0x7FF.
  Yanlış değer PLR bölgesini çipin dışına taşırıyordu (1853..2048, oysa 24C16
  sadece 0..2047). `valid_foot` byte'ı hiç yazılamıyor, yanlış adresten
  okunuyordu → `recovery.valid()` asla true olmuyordu.
- **EEPROM okuma protokolü düzeltildi.** Restart sonrası okuma kontrol byte'ı
  sabit `0xA1` idi; blok-seçim bitlerini taşımadığı için 255 üstü tüm adresler
  blok 0'dan okunuyordu. Ayrıca sequential read artık 256-byte blok sınırında
  bölünüyor.
- **Termistör koruması geri kazanıldı.** `HEATER_0_MINTEMP` ve `BED_MINTEMP`
  0 → 5.
- **Sınır dışı dizi yazması giderildi.** `RTS_HandleData()` içinde `axis`,
  `min`, `max` başlatılmamış kullanılabiliyordu.
- **Derleme zamanı korumaları eklendi.** `SanityCheck.h`: Z lock + probe aynı
  anda açılamaz. `powerloss.cpp`: PLR bölgesi EEPROM ve blok sınırları içinde
  olmalı (`static_assert`).
- `monitor_speed` 250000 → 115200 (`BAUDRATE` ile eşitlendi).
- Flash: 187.028 → **184.196** byte. Proje kodunda **0 uyarı**.

### SD1-2.4 (2026-07-23) — Karma sürücü + doküman denetimi

**Firmware**
- `Z_DRIVER_TYPE` / `E0_DRIVER_TYPE`: `TMC2208_STANDALONE` → `A4988`
  (fiziksel çip **HR4988SQ**; Marlin'de HR4988 tipi yok, A4988 donanım
  uyumlu eşdeğeridir). X/Y TMC2208 standalone kaldı.
- **KRİTİK:** `MINIMUM_STEPPER_POST_DIR_DELAY` / `PRE_DIR_DELAY`
  **30 ns → 200 ns**. 30 ns HR4988SQ için yetersizdi → yön değişimlerinde
  ters yönde adım riski (Z katman geçişi, E retract). Bkz. §8.3.
- Flash +40 byte (127.080 → 127.120), RAM değişmedi. **Yeniden flash gerekir.**

**Doküman denetimi — ölçümle çelişen iddialar düzeltildi**

| Nerede | Yazıyordu | Gerçek |
|---|---|---|
| §2.2 | 4 eksen de TMC2208, 800 mA RMS | Karma; X/Y 0.69 A, Z 0.47 A/motor, E0 0.51 A |
| §2.2 | `RSENSE 0.11 Ω` | **0.15 Ω** (`R150`), ölçüldü |
| §2.2 | Z tek motor | **Paralel iki motor**, tek sürücü |
| §2.3, §5.1 | "Direct drive" | ~~"Bowden" yapıldı~~ — **geri alındı 2026-07-24**: donanım direct drive (aşağı bak) |
| §5.2 | "TMC2208 StealthChop limiti" | Gövde rezonansı (300→250) |
| **§8 tamamı** | StealthChop/HYBRID/INTERPOLATE etkin | **Hiç derlenmiyor** — `#if HAS_TRINAMIC` false |
| §17 | "HYBRID_THRESHOLD kontrol et" | Bu kartta yok; DIR delay ve Vref'e bak |

- `Configuration_adv.h`: `#if HAS_TRINAMIC` bloğunun başına ölü olduğunu
  açıklayan uyarı eklendi; `*_RSENSE` 0.11 → 0.15; `CHOPPER_TIMING`'in
  "HAS_TRINAMIC aktif olduğu için kullanılır" yorumu (yanlıştı) düzeltildi.

### Versiyon: MarlinV2 by CTK (2026-05-23)

#### Oturum 1 — Temel Düzeltmeler
- Eksik HAL kaynak dosyaları derlemeye eklendi
- `SizeofDatabuf` 26 → 40 byte (DWIN ekran taşma düzeltmesi)
- Versiyon string: `"MarlinV2 by CTK"`

#### Oturum 2 — Hareket Optimizasyonu
- `HOMING_FEEDRATE_XY`: 3000 → 1000 mm/m (daha yumuşak homing)
- `LIN_ADVANCE_K`: 0.22 → 0.06 (direct drive için başlangıç değeri; kullanıcı kalibrasyonu zorunlu)
- ~~`HYBRID_THRESHOLD`: aktif edildi~~ — **hiçbir zaman etkin olmadı**;
  `#if HAS_TRINAMIC` bloğunda olduğu için derlenmiyordu (2026-07-23 denetimi)

#### Oturum 3 — Z-Probe / ABL Entegrasyonu *(SD1-2.1'de geri alındı — bkz. Oturum 4)*
- `pins_CREALITY.h`: PB1 → `Z_MIN_PROBE_PIN` (FIX_MOUNTED_PROBE koşullu)
- `Z_MIN_PROBE_ENDSTOP_INVERTING`: true → false (PNP NO sensör)
- `Z_MIN_PROBE_USES_Z_MIN_ENDSTOP_PIN`: yorumda (ayrı probe pini)
- `FIX_MOUNTED_PROBE`: aktif (endüktif sensör)
- `NOZZLE_TO_PROBE_OFFSET`: {0,0,0} → {0,0,2} (başlangıç)
- `AUTO_BED_LEVELING_BILINEAR`: aktif (4×4 grid)
- `GRID_MAX_POINTS_X`: 3 → 4
- `Z_SAFE_HOMING`: aktif
- `MULTIPLE_PROBING`: 2 (çift probing)
- Flash boyutu: 184,300 → 194,524 bytes (+10,224 bytes ABL kodu)

---

## Ek A: Dosya Konumları

```
sermoon-d1-backup/
├── Marlin/
│   ├── Configuration.h          ← Ana yapılandırma
│   ├── Configuration_adv.h      ← Gelişmiş yapılandırma
│   ├── Version.h                ← Versiyon string'leri
│   └── src/
│       ├── pins/stm32/
│       │   └── pins_CREALITY.h  ← Board pin tanımları
│       ├── lcd/dwin/
│       │   ├── LCD_RTS.h        ← DWIN ekran header
│       │   └── LCD_RTS.cpp      ← DWIN ekran implementasyon
│       └── HAL/HAL_STM32F1/     ← STM32F1 HAL katmanı
├── buildroot/
│   └── share/PlatformIO/
│       ├── scripts/creality.py  ← Firmware relocate scripti (0x08007000)
│       └── ldscripts/creality.ld ← Linker scripti
├── .pio/build/creality/
│   └── firmware.bin             ← Derlenmiş firmware binary
├── platformio.ini               ← PlatformIO proje yapılandırması
└── MANUAL.md                    ← Bu dosya
```

## Ek B: Faydalı Bağlantılar

- Marlin Probe Konfigürasyonu: https://marlinfw.org/docs/configuration/probes.html
- LIN_ADVANCE K-Faktörü Kalibrasyonu: https://marlinfw.org/tools/lin_advance/k-factor.html
- Marlin G-Code Referansı: https://marlinfw.org/meta/gcode/
- Balluff BES M18 Serisi: https://www.balluff.com/en-de/products/areas/A0001/groups/G0101

---

*Bu manual, MarlinV2 by CTK firmware'inin Sermoon D1 yazıcısındaki tüm yapılandırmasını belgeler. Her değişiklikte güncellenmelidir.*
