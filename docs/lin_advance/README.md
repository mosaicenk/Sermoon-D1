# Sermoon D1 — LIN_ADVANCE K Kalibrasyon Rehberi

LIN_ADVANCE (Linear Advance), extruder hızı değişimlerinde **filament basıncını
önceden hesaplayıp** köşe blob'ları, ipliklenme (stringing) ve under-extrusion
sorunlarını azaltır. Sermoon D1 **direct drive** ekstrudere sahiptir
(dişlisiz MK8 tipi, drive gear hotend'in hemen üstünde) — K değerleri bu
yüzden küçüktür ama sıfır değildir.

## Neden Sermoon İçin Önemli?

Direct drive'da drive gear ile nozül arası yol kısadır, yine de elastik
davranan bir basınç hattı vardır: kısa PTFE boğaz + melt bölgesindeki
sıvı plastik.

```
Drive gear ──[boğaz + melt bölgesi]── Nozzle
              ↑
        basınç birikmesi burada gecikir
```

Sonuç (Bowden'a göre küçük ölçekte, ama görünür):
- Yazıcı durduğunda → hat hâlâ basınçlı → **fazla extrude** (köşe blob'u)
- Yazıcı hızlandığında → basınç oturana kadar → **az extrude** (köşe boşluğu)
- Retraction sonrası → priming gecikmesi → **layer start defect**

LIN_ADVANCE bunu modeller: hız değişimi anlık değil, basınç dinamiği üzerinden
hesaplar. K katsayısı bu basınç-zaman sabitidir. Yol kısa olduğu için
Sermoon'da K tipik olarak **0.02-0.15** bandındadır (Bowden'ın 0.4-0.9'una
karşılık).

## Mevcut Durum

| Ayar | Değer | Konum |
|---|---|---|
| `LIN_ADVANCE` | ✅ Aktif | `Configuration_adv.h:1466` |
| `LIN_ADVANCE_K` (default) | 0.06 | `Configuration_adv.h:1483` |
| `M900` (runtime K set) | ✅ Çalışıyor | `gcode/feature/advance/M900.cpp` |
| `M500` (EEPROM kayıt) | ✅ Aktif | — |

⚠️ **Default K=0.06 direct drive için makul bir başlangıçtır ama kalibre
edilmeden güvenilmez** — E0 sürücüsü SD1-2.4 ile HR4988SQ'ya değişti ve
**K sürücüye özgüdür**; eski kalibrasyon değeri varsa o da geçersizdir.

## Sermoon Direct Drive K Aralıkları

| Filament | Tipik K aralığı | Başlangıç noktası |
|---|---|---|
| PLA | 0.02 - 0.10 | Test 0.02, 0.06, 0.10 |
| PETG | 0.04 - 0.15 | Test 0.05, 0.10, 0.15 |
| ABS | 0.02 - 0.10 | Test 0.02, 0.06, 0.10 |
| TPU | 0.10 - 0.40 | Test 0.10, 0.25, 0.40 |
| Esnek/Flex | 0.20 - 0.60 | Test 0.20, 0.40, 0.60 |

> Bu aralıklar başlangıç penceresidir, garanti değil — doğru K'yı desen
> belirler. Aynı yazıcıda farklı filament markası farklı K verebilir
> (filament esnekliği ve sıcaklık etkili). Her ana filament için ayrı
> kalibrasyon önerilir.

## Kalibrasyon Yöntemleri

### Yöntem A — Marlin Online K-Factor Tool (ÖNERİLEN)

Marlin'in resmi aracı en iyi test pattern'i otomatik üretir.

**1.** Aşağıdaki link'i aç:
```
https://marlinfw.org/tools/lin_advance/k-factor.html
```

**2.** Sermoon D1 parametrelerini gir:

| Alan | Değer |
|---|---|
| Filament Type | PLA (veya kullandığın) |
| Filament Diameter | 1.75 |
| Direct Drive | **Evet** (Sermoon D1 direct drive) |
| Bed Size X | 290 |
| Bed Size Y | 270 |
| Origin Bed Center | Hayır |
| Nozzle Temperature | 210 (PLA) / 240 (ABS) / 230 (PETG) |
| Bed Temperature | 60 (PLA) / 100 (ABS) / 80 (PETG) |
| Nozzle Diameter | 0.4 |
| Layer Height | 0.2 |
| Slow Speed | 20 mm/s |
| Fast Speed | 80 mm/s |
| K-Factor Start | **0.0** |
| K-Factor End | **0.3** |
| K-Factor Step | **0.02** |
| Number of Test Lines | 16 |
| Use TX in stock GCode | Yes (Marlin) |

**3.** "Generate G-code" butonuna bas, üretilen `.gcode` dosyasını indir.

**4.** SD karta kopyala, yazıcıdan çalıştır.

**5.** Print biten patterndeki numaralı satırlara bak — hangi K değeri
en uniform geçişi veriyorsa o senin K'in.

### Yöntem B — Manuel Tower Test (zaman alır)

`la_tower_test.gcode` dosyasını kullan:
1. Dosyayı aç, üstte `M900 K0.06` satırını bul
2. Bir K değeri (örn. 0.02) ile bir kez çalıştır → sonucu kaydet
3. K=0.06 ile tekrar çalıştır
4. K=0.10 ile tekrar
5. En uniform desenin K değerini seç

Hızlı ama 5-6 print gerektirir. Online araç bunu tek printte yapar.

## Sonuç Analizi (Görsel Rehber)

Test pattern bittikten sonra her K satırına yandan bak:

### ✅ Optimal K (örn. 0.04 - 0.10)
Tüm hız geçişlerinde **uniform duvar** — kalınlık sabit, yüzey pürüzsüz.

```
═══════════════════════════════════════
Slow │  Fast  │ Slow │  Fast  │ Slow
     ↑        ↑      ↑        ↑
   Geçişler net, blob/delik yok
```

### ❌ K çok DÜŞÜK (örn. 0.00 - 0.02)
Yavaştan hıza geçişte **incelme**, hızdan yavaşa geçişte **şişme/blob**.

```
══════╗   ╔══════╗   ╔══════╗  ← köşe BLOB
      ╚═══╝      ╚═══╝          ← hızlı bölgede İNCELME
```

### ❌ K çok YÜKSEK (örn. 0.25+)
Yavaştan hıza geçişte **delik**, hızdan yavaşa geçişte **incelme**.

```
══╗   ╔════════╗   ╔══════════ ← geçişte DELİK
   ╚══╝         ╚══╝
```

## K'yı Set Etme ve Kaydetme

### Geçici (test için)
```gcode
M900 K0.06    ; LIN_ADVANCE K = 0.06
```

### Kalıcı (EEPROM)
```gcode
M900 K0.06    ; Set
M500          ; EEPROM'a yaz
M501          ; Doğrulama: yükle
M503          ; Tüm ayarları göster — M900 K0.06 satırı görünmeli
```

### Firmware kalıcı (factory reset bile etkilemez)

Configuration_adv.h'da:
```c
#define LIN_ADVANCE_K 0.06   // Sermoon D1 PLA — kalibre edildi YYYY-MM-DD
```
Sonra `pio run -e creality` ile yeniden derle ve flash et.

## Slicer Entegrasyonu

Tek K değeri tüm filamentler için yeterli olmaz. **Slicer'ın start gcode**'una
materyal-spesifik M900 koy:

### Cura
"Filament Settings" → "Start G-code" üstüne ekle:
```gcode
M900 K0.06  ; PLA için optimize
```

### PrusaSlicer
"Filament Settings" → "Custom G-code" → "Start G-code":
```gcode
M900 K0.06
```

### OrcaSlicer / Bambu Studio
Filament profile'da "Filament start G-code":
```gcode
M900 K0.06
```

## Materyal-Spesifik Önerilen Profil

Kalibrasyonun bittikten sonra her materyal için belge:

```
Sermoon D1 — LIN_ADVANCE K profilim:
─────────────────────────────────────
Filament              K
─────────────────────────────────────
Generic PLA           0.06
PETG                  0.10
ABS                   0.05
TPU 95A               0.25
Esun PLA+             0.07
Polymaker PLA         0.06
─────────────────────────────────────
Tarih: YYYY-MM-DD
```

Bu listeyi yazıcının yanında tut, slicer profilelarına yansıt.

## Doğrulama Print

Kalibrasyon sonrası gerçek bir model print et — örneğin:
- 20×20×20 kalibrasyon küpü (Cura'nın kendi modeli)
- "All In One Test" model (Thingiverse)
- Yüzeyde köşelerde **net edge**, layer kalınlıkları uniform → tuning başarılı

## Bilinen Etkileşim: S_CURVE_ACCELERATION

Bu firmware'de `S_CURVE_ACCELERATION` açık (`Configuration.h:859`) ve LA ile
yapısal bir gerilimi var: LA'nın blok başına ekstruder telafi hızı
(`advance_speed`) **sabit (trapez) ivme** varsayımıyla hesaplanır; S-curve ise
anlık ivmeyi faz içinde değiştirir (uçlarda ~0, zirvede ortalamanın ~1.9 katı).
Kodda görünür hali: `stepper.cpp:1582/1627` — `LA_isr_rate` blok boyunca tek
değerdir, Bézier hız eğrisini takip etmez. Upstream Marlin bir dönem bu
birlikteliği SanityCheck ile engelleyip `EXPERIMENTAL_SCURVE` bayrağı arkasına
almıştı; bu taban o korumadan eskidir.

Pratik sonuç:
- Direct drive K'ları küçük olduğu için (≤0.15) LA'nın eklediği telafi
  adımları azdır — etkileşimin pratik etkisi Bowden kurulumlarına göre çok
  daha düşüktür; çoğu baskıda fark edilmez.
- **Belirti**: kalibrasyon deseninde hiçbir K satırı tam uniform olmuyor ve
  hız-geçiş köşelerindeki blob her K değerinde sürüyorsa →
  `S_CURVE_ACCELERATION`'ı kapat (`Configuration.h:859`), yeniden derle ve
  deseni tekrar bas. Köşe-hız kontrolünü Junction Deviation zaten sağlıyor;
  S-curve'süz kalmanın kaybı küçüktür. Kapatırsan K'yı da yeniden kalibre et.

## Troubleshooting

**"Echo:Unknown command: M900"**
→ LIN_ADVANCE flag aktif değil. `Configuration_adv.h:1466`'yı kontrol et,
   `pio run -e creality` ile yeniden derle.

**Test bittikten sonra hiç fark görmüyorum**
→ Direct drive'da K farkları incedir; 0.1'lik adım optimumu atlar.
   0.0 - 0.2 arası 0.01-0.02 adımla tekrar dene. Satırlar arasında hâlâ fark
   yoksa desendeki hız kontrastını artır (Slow 20 / Fast 100 mm/s) — hız
   farkı büyüdükçe LA etkisi belirginleşir.

**Print sırasında M73 progress %100'e gitti ama print bitmedi**
→ LA test pattern uzun, slicer estimate yanlış olabilir. Sorunsuz devam eder.

**Yüksek K'da extruder skip yapıyor (klick sesi)**
→ K çok yüksek, extruder torku yetmiyor. K azalt veya
   `DEFAULT_EJERK` değerini Configuration.h'da artır (default 5).

**M900 K çalışmıyor (M900 sonrası yine eski davranış)**
→ M500 yapmadan reset yapmışsın. Sırayı: M900 K... → M500 → reset.

## Hızlı Komut Referansı

```gcode
M900             ; Mevcut K değerini sorgula
M900 K0.6        ; K = 0.6 set
M500             ; EEPROM'a yaz
M501             ; EEPROM'dan oku
M502             ; Factory defaults (K=0.06'ya döner)
M503             ; Tüm ayarları göster
```

## İleri Düzey: EXTRA_LIN_ADVANCE_K

Configuration_adv.h'da `EXTRA_LIN_ADVANCE_K` flag'ini açarsan **iki K değeri
slot'u** elde edersin (M900 T0 K... ve M900 T1 K...). Materyal değişiminde
M900 ile slot değiştirip hızlı geçiş yapabilirsin. Default OFF — aktive
etmek istersen söyle, ekleyebilirim.

---

## Kalibrasyon Tarif Özeti — TL;DR

```
1. Online araca git: marlinfw.org/tools/lin_advance/k-factor.html
2. Sermoon parametreleri gir (yukarıdaki tablo)
3. K_start=0.0, K_end=0.3, K_step=0.02
4. Generate → SD'ye kopyala → çalıştır
5. Yazılı pattern'i incele, en uniform olan satırın K'sini al
6. M900 K<value> + M500
7. Slicer start gcode'una M900 K<value> ekle
8. Bayram et 🎉
```
