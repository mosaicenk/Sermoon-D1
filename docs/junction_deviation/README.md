# Sermoon D1 — Junction Deviation Kalibrasyon Rehberi

CLASSIC_JERK'ten **JUNCTION_DEVIATION**'a geçiş yapıldı. Bu doküman:
1. JD nedir, neden geçtik
2. Kalibrasyon prosedürü
3. Sonuç beğenmezsen geri dönüş

## JD vs Klasik Jerk — Hızlı Karşılaştırma

| Özellik | Classic Jerk | Junction Deviation |
|---|---|---|
| Parametre sayısı | 4 (X/Y/Z/E ayrı) | 1 (`JUNCTION_DEVIATION_MM`) |
| Hesap modeli | Step velocity delta | Fizik bazlı geometric cornering |
| Köşe açısı dikkate alır | Hayır | Evet (sin θ formülü) |
| Eksen-eksene tutarlılık | Manuel ayar | Otomatik |
| İvme ile etkileşim | Bağımsız | Birleşik (`v² = δ·a/...`) |
| Modern Marlin önerisi | ✗ Legacy | ✓ Önerilen (Marlin 2.0+) |
| Kalibrasyon zorluğu | Orta-yüksek (4 değer) | Kolay (1 değer) |

JD'nin matematiği:
```
v_max_corner = √( JUNCTION_DEVIATION_MM × acceleration × (1/sin(θ/2) - 1)⁻¹ )
```

θ = köşe açısı değişimi. Keskin köşe (θ büyük) = düşük v_max; düz devam (θ ≈ 0) = sınırsız hız.

## Mevcut Ayar

`Configuration.h:756`:
```c
#if DISABLED(CLASSIC_JERK)
  #define JUNCTION_DEVIATION_MM 0.013   // (mm) Default — Cartesian Sermoon için iyi
#endif
```

## Kalibrasyon Aralık Tablosu

Sermoon D1 (Bowden, kapalı kabin) için tipik değerler:

| δ (mm) | Karakter | Print Süresi | Kalite |
|---|---|---|---|
| 0.003 | Aşırı sıkı | +%15 yavaş | Mükemmel ama gereksiz |
| 0.005 | Çok sıkı | +%10 | Yüksek detay |
| 0.008 | Sıkı | +%5 | Hassas geometriler |
| **0.013** | **DEFAULT** | Baseline | İyi denge |
| 0.020 | Gevşek | -%5 hızlı | Hafif yumuşama |
| 0.025 | Çok gevşek | -%10 | Belirgin yuvarlanma |
| > 0.030 | Önerilmez | — | Köşe overshoot riski |

## Kalibrasyon Prosedürü

### 1. Baseline Print
Mevcut 0.013 ile referans bir print (kalibrasyon küpü, all-in-one test) yap. Köşelere **mikroskopla / büyüteçle** bak.

### 2. Test Gcode'u Çalıştır

`docs/junction_deviation/jd_test.gcode` SD'ye kopyala, çalıştır. Bu dosya hızlı yön değişimleri içeren bir test çalıştırır — köşe davranışını gözleyebilirsin.

### 3. Değer Ayarlama

Runtime değişiklik:
```gcode
M205 J0.008    ; JD = 0.008 set (sıkı test)
M500           ; EEPROM'a kaydet
```

Kalıcı kod değişikliği (`Configuration.h:756`):
```c
#define JUNCTION_DEVIATION_MM 0.008
```

### 4. Sweep Test (en iyi yöntem)

Aynı modeli farklı JD değerleriyle 3 kez print et:
- Print 1: `M205 J0.008` → bitince modele etiket koy "0.008"
- Print 2: `M205 J0.013` → "0.013"
- Print 3: `M205 J0.020` → "0.020"

3 modeli yan yana koy, **köşelere** ve **kavislere** bak:
- Hangisi en kararlı?
- Hangisinde overshoot var?
- Hangisi çok yavaş ama gerekli detay yok?

Genelde 0.013 baseline iyi sonuç verir — değiştirmeye değecek bir sorun olmadıkça dokunma.

## M205 Komutu (Runtime Değişiklik)

```gcode
M205             ; Mevcut tüm hız ayarlarını göster
M205 J0.013      ; JD set
M205 X<v> Y<v>   ; Klasik jerk değerleri (CLASSIC_JERK varsa)
M205 S<v>        ; Min print speed
M205 T<v>        ; Min travel speed
M500             ; EEPROM'a kaydet
M501             ; EEPROM'dan oku
```

## Beklenen Davranış Farkları

### CLASSIC_JERK iken görüyordun:
- Bazı köşelerde belirgin "tıkanma" sesi (yüksek jerk → ani hız değişimi)
- Pürüzsüz büyük kavislerde sorunsuz
- Küçük detaylarda overshoot
- E-jerk (DEFAULT_EJERK 5) extruder için ayrı kontrol

### JUNCTION_DEVIATION ile görmen muhtemel:
- Tüm köşeler **birbiriyle tutarlı** davranır (otomatik açı bazlı)
- Büyük açılar için daha hızlı (gereksiz yavaşlama yok)
- Küçük açılar için daha akıllı yavaşlama
- DEFAULT_EJERK hâlâ extruder için kullanılır (LIN_ADVANCE)

## Geri Dönüş (Revert)

Eğer JD beğenmezsen, klasik jerk'e dön:

`Configuration.h:744`:
```c
//#define CLASSIC_JERK    ← yorumu kaldır

#define CLASSIC_JERK      ← böyle yap
#if ENABLED(CLASSIC_JERK)
  #define DEFAULT_XJERK 10.0
  ...
```

Sonra:
```powershell
pio run -e creality
# yeniden flash
```

Alternatif: runtime ile EEPROM üstünden manuel jerk değer set'i (CLASSIC_JERK build-time gerektirdiği için **işe yaramaz** — kod değişikliği şart).

## Bilinen Tradeoffs

1. **DEFAULT_EJERK kalır**: LIN_ADVANCE bunu kullanır. JD değiştirmez.
2. **Z jerk kavramı kaybolur**: JD Z için çalışır (Sermoon Z hareketleri zaten yavaş, sorun olmaz).
3. **Print süresi farkı**: Sermoon ortalama hızlarda (50-80 mm/s) +/-%5 fark beklenir.
4. **EEPROM uyumluluğu**: Mevcut M500 kayıtlarındaki jerk değerleri yeni firmware'de **görmezden gelinir**. Gerek yoksa M502 + M500 ile reset.

## Hızlı Komut Referansı

```gcode
M205 J0.013      ; JD set runtime
M205             ; Mevcut JD/jerk değerlerini göster
M500 / M501      ; Kaydet / yükle
M502 + M500      ; Factory + EEPROM yaz
M503             ; Tüm ayarlar — JD satırı M205'te görünür
```

## Doğrulama

Yeni firmware flash sonrası:
```gcode
M115             ; Versiyon: SD1-1.0
M503             ; Çıktıda "M205 J0.013" görünmeli
                 ; (CLASSIC_JERK olsa "M205 X10 Y10 Z0.4 E5" görünürdü)
```
