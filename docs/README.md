# Sermoon D1 Firmware — Dokümantasyon İndeksi

Bu klasör Sermoon D1 firmware'i ile ilgili tüm uygulama notları,
kalibrasyon rehberleri ve teknik referans dokümanlarını içerir.

## İçerik

### Kalibrasyon Rehberleri

| Klasör | İçerik |
|---|---|
| **[`pid_tuning/`](pid_tuning/README.md)** | PID otomatik kalibrasyon (M303) — hotend ve yatak için doğru sıcaklık kontrolü |
| **[`lin_advance/`](lin_advance/README.md)** | LIN_ADVANCE K kalibrasyonu (M900) — Bowden setup için köşe kalitesi |
| **[`junction_deviation/`](junction_deviation/README.md)** | Junction Deviation kalibrasyonu (M205 J) — modern köşe-hız kontrolü |

Her klasörde:
- `README.md` — kapsamlı prosedür, troubleshooting, beklenen değerler
- `*.gcode` — SD'den çalıştırılabilir test dosyaları

### Referans

| Dosya | İçerik |
|---|---|
| [`Bresenham.md`](Bresenham.md) | Marlin step üretiminde kullanılan Bresenham algoritmasının açıklaması (orijinal Marlin dokümanı) |

### Proje Düzeyi

| Dosya | Konum | İçerik |
|---|---|---|
| [`README.md`](../README.md) | Proje kökü | Hızlı başlangıç, build, donanım özet |
| [`CHANGELOG.md`](../CHANGELOG.md) | Proje kökü | Bu fork üzerinde yapılan değişikliklerin tarihçesi |

## Hızlı Başlangıç İndeksi

İlk kurulum:
1. Donanımı kontrol et → [proje README](../README.md#donanım)
2. Firmware'i derle → [proje README → Build](../README.md#build)
3. Yazıcıya flash et → [pre-flash backup](pid_tuning/README.md#firmwarei-flash-et)

İlk kalibrasyon (önerilen sıra):
1. **PID kalibrasyonu** → [pid_tuning/](pid_tuning/README.md)
2. **LIN_ADVANCE K** → [lin_advance/](lin_advance/README.md)
3. **Z offset (ilk katman)** → babystep ile, [MANUAL.md §15.3](../MANUAL.md#153-z-offset-ilk-katman-kalibrasyonu)
4. **Doğrulama print** → kalibrasyon küpü, all-in-one test model

> Bu yazıcıda **Z-probe yok** (BLTouch de endüktif sensör de takılı değil).
> Tabla tesviyesi manueldir; `G29`/`M851` komutları derlenmemiştir.

İleri:
- Z Lock manuel kontrol → [Marlin/Configuration_adv.h](../Marlin/Configuration_adv.h) → `SERMOON_Z_LOCK` araması
- Backport edilen yeni özellikler → [CHANGELOG.md](../CHANGELOG.md)
- Sermoon-spesifik feature flag listesi → [proje README](../README.md#sermoona-özgü-flag'ler)
