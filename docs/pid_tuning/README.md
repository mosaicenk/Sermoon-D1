# Sermoon D1 — PID Otomatik Tuning Rehberi

PID (Proportional-Integral-Derivative) tuning, hotend ve yatak sıcaklığını
hedef değerde **stabil** tutmak için kontrol katsayılarını kalibre eder.

Bu firmware'de:
- Hotend PID: ✅ aktif (`PIDTEMP`)
- Yatak PID: ✅ aktif (`PIDTEMPBED`)
- Otomatik tuning: ✅ aktif (`M303` + `PID_AUTOTUNE_MENU`)
- Manuel düzenleme: ✅ aktif (`PID_EDIT_MENU` + `M301`/`M304`)

## Neden Tuning Şart?

Configuration.h'da yer alan **default değerler genel-amaçlıdır**, Sermoon
donanımına özel değildir:

```c
// Hotend (Ultimaker referans değerleri)
DEFAULT_Kp = 21.73, Ki = 1.54, Kd = 76.55

// Yatak (250W silikon referans)
DEFAULT_bedKp = 327.11, bedKi = 19.20, bedKd = 1393.45
```

Sermoon D1'in 24V 40W hotend ve ~200W yatak'ı bu referanslarla aynı
**termal kütle/zaman sabiti** profiline sahip değil. Sonuçlar:
- **Aşırı salınım** (hedef ±5°C dalgalanma)
- **Overshoot** (set 200°C → ölçüm 207°C)
- **Yavaş sıkışma** (set 200°C → 30 sn 195°C'de takılı)
- **Bang-bang davranışı** (PID etkisiz, klasik on/off gibi)

Doğru tuning sonrası:
- Hedef sıcaklığa **±0.5°C** içinde tutunma
- Overshoot < 2°C
- Print kalitesinde sıcaklık-bağımlı artifaktların azalması

## Tuning Prosedürü

### 1. Yazıcıyı Hazırla

```
✓ Yazıcı tam soğuk (oda sıcaklığında, ~25°C)
✓ Print bittikten en az 30 dk geçmiş (uniform termal denge)
✓ Kabin kapağı kapalı (gerçek print koşullarıyla aynı)
✓ SD kart yerinde
```

### 2. Hotend Tuning (8-12 dakika)

**Yöntem A — SD karttan:**
1. `pid_hotend.gcode` dosyasını SD'nin köküne kopyala
2. Yazıcıyı boot et
3. Ekrandan "Print" → `pid_hotend.gcode` seç
4. Bekle (yazıcı 210°C'ye ısınır, ~8 cycle salınım yapar)
5. Sonuç ekranda + EEPROM'a otomatik kaydedilir

**Yöntem B — Host (OctoPrint/PrusaSlicer console)'dan:**
```gcode
M106 S128                ; Part fan %50 — gerçek print yükü simülasyonu
M303 E0 S210 C8 U1       ; Tune et + uygula
M500                     ; Kaydet
M107                     ; Fan off
```

**Parametre özelleştirme:**
- `S210` — kullandığın filament sıcaklığı (PLA: 195-210, PETG: 230, ABS: 240)
- `C8` — cycle sayısı (3 minimum, 5 yeterli, 8 yüksek kalite)
- `U1` — otomatik uygula. Olmazsa sadece raporlar.

### 3. Yatak Tuning (25-40 dakika)

```gcode
M303 E-1 S60 C5 U1       ; 60°C, 5 cycle (PLA için)
M500
```

ABS için `S100`, PETG için `S80`.

### 4. Doğrulama

Tuning bittikten sonra:

```gcode
M501                     ; EEPROM'dan yükle (emin olmak için)
M503                     ; Tüm ayarları göster
```

`M503` çıktısında `M301` (hotend) ve `M304` (yatak) satırlarını ara:

```
echo:; PID settings:
echo:  M301 P21.73 I1.54 D76.55      ← TUNE ÖNCESİ (default)
echo:  M304 P327.11 I19.20 D1393.45  ← TUNE ÖNCESİ
```

vs. tune sonrası (Sermoon-spesifik örnekler):
```
echo:  M301 P14.48 I0.92 D56.92      ← Sermoon hotend (gerçek print sonucu örnek)
echo:  M304 P145.83 I26.84 D659.13   ← Sermoon yatak
```

### 5. Print Testi

Bir kalibrasyon küpü veya termal-kalibrasyon basit shape print et.
Console/host'ta sıcaklık grafiğini gözle:
- Set 200°C → grafik 200 ± 0.5°C bandında düz çizgi olmalı
- Yatak set 60°C → grafik 60 ± 1°C

## Beklenen Değer Aralıkları (Sermoon D1)

| Parametre | Tipik aralık | Patolojik |
|---|---|---|
| Hotend Kp | 12 - 28 | <5 veya >50 → yanlış sensör/heater |
| Hotend Ki | 0.5 - 2.0 | >5 → çok agresif, salınım |
| Hotend Kd | 30 - 90 | <10 veya >200 → ısıtıcı yanıt sorunu |
| Yatak Kp | 50 - 250 | <20 → çok yavaş, >500 → osilasyon |
| Yatak Ki | 5 - 30 | — |
| Yatak Kd | 200 - 1500 | — |

Değerler bu aralıkların **dışında** çıkıyorsa:
1. Thermistor bağlantısını kontrol et
2. Heater wire integrity kontrol et
3. Tuning sıcaklığını değiştirip tekrar dene
4. Cycle sayısını artır (`C10`)

## Configuration.h'a Yazma (Opsiyonel)

EEPROM kayıt yeterli — fakat firmware'i tekrar derlersen `M502` (factory
reset) sonrası kaybolur. Kalıcı default için Configuration.h'da güncelle:

```c
// Sermoon D1 (kişisel kalibrasyon, YYYY-MM-DD)
#define DEFAULT_Kp    14.48
#define DEFAULT_Ki     0.92
#define DEFAULT_Kd    56.92

#define DEFAULT_bedKp  145.83
#define DEFAULT_bedKi   26.84
#define DEFAULT_bedKd  659.13
```

Sonra `pio run -e creality` ile rebuild + reflash. Bu adım opsiyonel,
çoğu kullanıcı için EEPROM yeterli.

## Troubleshooting

**"PID Autotune failed! Bad extruder number"**
→ E parametresi yanlış. Hotend için `E0`, yatak için `E-1` kullanın.

**"PID Autotune failed! temperature too high"**
→ Hedef sıcaklık çok yüksek (>HEATER_0_MAXTEMP). S değerini düşürün.

**"PID Autotune failed! Timeout"**
→ Heater bağlantı sorunu, heater veya thermistor arızalı, ya da
   thermal protection devreye girdi. Donanımı kontrol edin.

**Sıcaklık tuning sırasında düşmüyor (cooling phase)**
→ Part cooling fan kapalı; M106 S128 ile açın. Veya hotend silikon
   sock yokluğu, sıcaklık çok agresif tutunuyor.

**Sonuçlar EEPROM'a kaydedilmedi**
→ M500 unuttunuz veya EEPROM yazma başarısız. M503 ile kontrol edin.
   EEPROM hatası varsa M502 ile reset, sonra tekrar deneyin.

**Tuning sonrası ısınma yavaş**
→ Normal — PID artık daha kontrollü ısıtıyor. Bang-bang gibi tam güç
   atmıyor. Print için yine yeterli hızda ısınır.

## Tekrar Tuning Ne Zaman?

- Heater veya thermistor değiştirildiğinde
- Hotend tipi değiştirildiğinde (örn. all-metal hotend'e geçiş)
- Yeni materyal yelpazesi (örn. PLA only → PETG/ABS)
- Print kalitesinde sıcaklık-bağımlı sorunlar gözlemlendiğinde
- Yılda bir bakım kontrolü olarak (mekanik aşınmaya bağlı)

## İleri Düzey: Materyal-Bazlı PID

Tek değer set hem PLA hem ABS için ortalama olur. Materyal-spesifik
kontrol istiyorsan **slicer start gcode**'una koyabilirsin:

```gcode
; PLA start gcode örneği
M301 P14.48 I0.92 D56.92  ; PLA için optimize edilmiş PID

; ABS start gcode örneği
M301 P12.95 I0.85 D49.21  ; ABS daha düşük termal kütle gerektiriyor
```

Her materyalle ayrı tuning yap, sonuçları slicer'a kaydet.

---

## Hızlı Başvuru — Kalibrasyon Komutları

```gcode
M303 E0 S210 C8 U1   ; Hotend tune (8 cycle, uygula)
M303 E-1 S60 C5 U1   ; Yatak tune
M500                 ; EEPROM'a kaydet
M501                 ; EEPROM'dan yükle
M502                 ; Factory reset (default değerler)
M503                 ; Tüm ayarları göster
M301 P14 I0.92 D57   ; Manuel hotend PID set
M304 P150 I27 D660   ; Manuel yatak PID set
```
