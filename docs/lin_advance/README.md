# Sermoon D1 — LIN_ADVANCE K Kalibrasyon Rehberi

LIN_ADVANCE (Linear Advance), extruder hızı değişimlerinde **filament basıncını
önceden hesaplayıp** köşe blob'ları, ipliklenme (stringing) ve under-extrusion
sorunlarını yok eder. Özellikle **Bowden setup** olan Sermoon D1 için kritiktir.

## Neden Sermoon İçin Önemli?

Bowden setup'ta filament ile drive gear arasındaki **uzun PTFE tüpü** filament
üzerinde elastik bir "yay" gibi davranır:

```
Drive gear ───[PTFE tüp ~50cm]─── Hotend nozzle
              ↑
         basıncı buradan iletmesi gecikir
```

Sonuç:
- Yazıcı durduğunda → tüp hâlâ basınçlı → **fazla extrude** (köşe blob'u)
- Yazıcı hızlandığında → tüp yetişene kadar → **az extrude** (köşe boşluğu)
- Retraction sonrası → priming gecikmesi → **layer start defect**

LIN_ADVANCE bunu modeller: hız değişimi anlık değil, basınç dinamiği üzerinden
hesaplar. K katsayısı bu basınç-zaman sabitidir.

## Mevcut Durum

| Ayar | Değer | Konum |
|---|---|---|
| `LIN_ADVANCE` | ✅ Aktif | `Configuration_adv.h:1429` |
| `LIN_ADVANCE_K` (default) | 0.22 | `Configuration_adv.h:1421` |
| `M900` (runtime K set) | ✅ Çalışıyor | `gcode/feature/advance/M900.cpp` |
| `M500` (EEPROM kayıt) | ✅ Aktif | — |

⚠️ **Default K=0.22 Sermoon Bowden için çok DÜŞÜK** — referans değer 1.75mm
PLA *Direct Drive* için. Bowden'da 2-5× daha yüksek değerler tipiktir.

## Sermoon Bowden K Aralıkları

| Filament | Tipik K aralığı | Başlangıç noktası |
|---|---|---|
| PLA | 0.4 - 0.9 | Test 0.4, 0.6, 0.8 |
| PETG | 0.5 - 1.2 | Test 0.6, 0.9, 1.2 |
| ABS | 0.4 - 0.8 | Test 0.4, 0.6, 0.8 |
| TPU | 0.6 - 1.5 | Test 0.6, 1.0, 1.4 |
| Esnek/Flex | 1.0 - 2.0 | Test 1.0, 1.5, 2.0 |

> Aynı yazıcıda farklı filament markası farklı K verebilir (Bowden uzunluğu
> ve filament esnekliği etkili). Her ana filament için ayrı kalibrasyon önerilir.

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
| Direct Drive | **Hayır** (Sermoon Bowden) |
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
| K-Factor End | **1.5** |
| K-Factor Step | **0.1** |
| Number of Test Lines | 16 |
| Use TX in stock GCode | Yes (Marlin) |

**3.** "Generate G-code" butonuna bas, üretilen `.gcode` dosyasını indir.

**4.** SD karta kopyala, yazıcıdan çalıştır.

**5.** Print biten patterndeki numaralı satırlara bak — hangi K değeri
en uniform geçişi veriyorsa o senin K'in.

### Yöntem B — Manuel Tower Test (zaman alır)

`la_tower_test.gcode` dosyasını kullan:
1. Dosyayı aç, üstte `M900 K0.40` satırını bul
2. Bir K değeri (örn. 0.4) ile bir kez çalıştır → sonucu kaydet
3. K=0.6 ile tekrar çalıştır
4. K=0.8 ile tekrar
5. En uniform desenin K değerini seç

Hızlı ama 5-6 print gerektirir. Online araç bunu tek printte yapar.

## Sonuç Analizi (Görsel Rehber)

Test pattern bittikten sonra her K satırına yandan bak:

### ✅ Optimal K (örn. 0.6 - 0.8)
Tüm hız geçişlerinde **uniform duvar** — kalınlık sabit, yüzey pürüzsüz.

```
═══════════════════════════════════════
Slow │  Fast  │ Slow │  Fast  │ Slow
     ↑        ↑      ↑        ↑
   Geçişler net, blob/delik yok
```

### ❌ K çok DÜŞÜK (örn. 0.0 - 0.3)
Yavaştan hıza geçişte **incelme**, hızdan yavaşa geçişte **şişme/blob**.

```
══════╗   ╔══════╗   ╔══════╗  ← köşe BLOB
      ╚═══╝      ╚═══╝          ← hızlı bölgede İNCELME
```

### ❌ K çok YÜKSEK (örn. 1.5+)
Yavaştan hıza geçişte **delik**, hızdan yavaşa geçişte **incelme**.

```
══╗   ╔════════╗   ╔══════════ ← geçişte DELİK
   ╚══╝         ╚══╝
```

## K'yı Set Etme ve Kaydetme

### Geçici (test için)
```gcode
M900 K0.6     ; LIN_ADVANCE K = 0.6
```

### Kalıcı (EEPROM)
```gcode
M900 K0.6     ; Set
M500          ; EEPROM'a yaz
M501          ; Doğrulama: yükle
M503          ; Tüm ayarları göster — M900 K0.6 satırı görünmeli
```

### Firmware kalıcı (factory reset bile etkilemez)

Configuration_adv.h'da:
```c
#define LIN_ADVANCE_K 0.60   // Sermoon D1 PLA — kalibre edildi YYYY-MM-DD
```
Sonra `pio run -e creality` ile yeniden derle ve flash et.

## Slicer Entegrasyonu

Tek K değeri tüm filamentler için yeterli olmaz. **Slicer'ın start gcode**'una
materyal-spesifik M900 koy:

### Cura
"Filament Settings" → "Start G-code" üstüne ekle:
```gcode
M900 K0.6  ; PLA için optimize
```

### PrusaSlicer
"Filament Settings" → "Custom G-code" → "Start G-code":
```gcode
M900 K0.6
```

### OrcaSlicer / Bambu Studio
Filament profile'da "Filament start G-code":
```gcode
M900 K0.6
```

## Materyal-Spesifik Önerilen Profil

Kalibrasyonun bittikten sonra her materyal için belge:

```
Sermoon D1 — LIN_ADVANCE K profilim:
─────────────────────────────────────
Filament              K
─────────────────────────────────────
Generic PLA           0.6
PETG                  0.9
ABS                   0.5
TPU 95A               1.2
Esun PLA+             0.7
Polymaker PLA         0.65
─────────────────────────────────────
Tarih: YYYY-MM-DD
```

Bu listeyi yazıcının yanında tut, slicer profilelarına yansıt.

## Doğrulama Print

Kalibrasyon sonrası gerçek bir model print et — örneğin:
- 20×20×20 kalibrasyon küpü (Cura'nın kendi modeli)
- "All In One Test" model (Thingiverse)
- Yüzeyde köşelerde **net edge**, layer kalınlıkları uniform → tuning başarılı

## Troubleshooting

**"Echo:Unknown command: M900"**
→ LIN_ADVANCE flag aktif değil. `Configuration_adv.h:1429`'u kontrol et,
   `pio run -e creality` ile yeniden derle.

**Test bittikten sonra hiç fark görmüyorum**
→ K çok küçük adımlarla deniyor olabilirsin. 0.0 - 1.5 arası 0.1 adım dene.
   Veya filament Bowden tipinde değil (örn. direct drive konversiyon yapılmış)
   olabilir; o zaman K=0.05-0.2 dene.

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
M502             ; Factory defaults (K=0.22'ye döner)
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
3. K_start=0.0, K_end=1.5, K_step=0.1
4. Generate → SD'ye kopyala → çalıştır
5. Yazılı pattern'i incele, en uniform olan satırın K'sini al
6. M900 K<value> + M500
7. Slicer start gcode'una M900 K<value> ekle
8. Bayram et 🎉
```
