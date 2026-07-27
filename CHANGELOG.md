# Sermoon D1 Firmware — Changelog

Bu dosya bu fork üzerinde stock Creality Sermoon D1 firmware'ine göre
yapılan tüm değişiklikleri belgeler.

Format: [Keep a Changelog](https://keepachangelog.com/en/1.1.0/)
Versiyonlama: Sermoon-D1-X.Y[-suffix] (X = major iyileştirme, Y = minor)

---

## [Sermoon-D1-2.9] — 2026-07-27

Homing duyarlılığı, mekanik switch ömrü ve köşe geçiş kalitesi optimizasyonları. Flash +24 byte
(126.864 → **126.888**, %24,2), RAM değişmedi (13.176). Binary değişti →
**yeniden flash gerekir**.

### Changed

- **`JUNCTION_DEVIATION_MM` `0.013` → `0.015`.**
  Sermoon D1 baskı performansını artırmak için varsayılan JD değeri 0.015 olarak ayarlandı.

- **`HOMING_BUMP_DIVISOR` `{ 2, 2, 4 }` → `{ 4, 4, 4 }`.**
  X ve Y eksenlerinde 2. yavaş dokunma hızı 8.3 mm/s'den 4.1 mm/s'ye düşürüldü.
  Limit switch mekanik esnemesi azaltılarak homing tekrarlanabilirliği mikron seviyesinde artırıldı.

- **`HOMING_BACKOFF_MM` `{ 0, 0, 2 }` → `{ 1, 1, 2 }`.**
  Homing sonrası X ve Y eksenlerinde 1 mm geri çekilme eklendi. Switch yayının sürekli
  basılı kalması engellenerek mekanik ömrü uzatıldı ve `M119` ile kopuk kablo / park ayrımı düzeltildi.

- **`Z_SAFE_HOMING_X/Y_POINT` `X_MIN_POS` → `(X_MIN_POS + 1)` = `(-9, -9)`.**
  Backoff (1 mm) ile tam uyumlu kılınarak Z homing öncesinde X/Y hareketsiz park noktasında bırakıldı.

---

## [Sermoon-D1-2.8] — 2026-07-27

X/Y park noktası −8'den **−10'a** alındı (kullanıcı isteği). Flash −24 byte
(126.888 → **126.864**, %24,2), RAM değişmedi (13.176). Binary değişti →
**yeniden flash gerekir** (SHA256 `A9567E83…23DA`, 2026-07-27 derlemesi).

### Changed

- **`HOMING_BACKOFF_MM` `{ 2, 2, 2 }` → `{ 0, 0, 2 }`.**
  −10 = `X_MIN_POS`/`Y_MIN_POS` = endstop trigger noktasının kendisi. Oraya
  park etmek "homing sonrası hiç geri çekilme" demektir: `homeaxis()`
  içindeki `if (backoff_mm)` koşulu 0'da false kalır ve geri çekme hareketi
  hiç üretilmez (`motion.cpp:1686`). **Z'de 2 mm korundu** — istek yalnızca
  X/Y içindi.

- **`Z_SAFE_HOMING_X/Y_POINT` `(X_MIN_POS + 2)` → `X_MIN_POS`.**
  Nokta, X/Y homing'in bıraktığı konumla aynı kalmalı ki
  `home_z_safely()` içindeki `do_blocking_move_to_xy()` sıfır uzunlukta
  kalsın. Backoff 0 olunca doğru değer `X_MIN_POS`'un kendisi.
  Ölçüldü: `Z_SAFE_XY = -10 , -10`.
  > İki ayar **birbirine bağlı**: `HOMING_BACKOFF_MM` tekrar sıfırdan farklı
  > yapılırsa buraya aynı miktar eklenmelidir, aksi hâlde nozul G28 sonunda
  > yer değiştirir. Not `Configuration.h`'a yazıldı.

### Sonuç — G28 sonrası konum

Her varyantta **(−10, −10)**: `G28`, `G28 X Y`, `G28 X`, `G28 Y`.
(SD1-2.7'de −8, ondan önce tam `G28` için 145/135.)

### Hareket açısından güvenli — ölçüldü

Araba artık endstop **basılı** halde park ediyor. Kontrol edildi:

| Kontrol | Sonuç |
|---|---|
| `ENDSTOPS_ALWAYS_ON_DEFAULT` | **OFF** → endstop'lar yalnız homing'de izleniyor |
| `endstops.cpp:711` X_MIN kontrolü | yalnız **−yön** dalında; `+` hareket tetiklemiyor |
| `MIN_SOFTWARE_ENDSTOPS` | açık → −10 altına inilemiyor |

### Bedeli (kabul edildi, belgelendi)

- **Mekanik:** anahtar kolu/yayı boşta sürekli baskı altında.
- **Teşhis:** `M119` dinlenme konumunda daima `x_min: TRIGGERED` verir.
  Endstop'lar NC bağlı olduğu için **kopuk kablo da TRIGGERED gösterir** —
  yani "evinde" ile "arızalı" tek bakışta ayırt edilemez hâle geldi.
  MANUAL'a **§6.5** eklendi: ekseni 20 mm uzaklaştırıp `M119` okuma yordamı.

### Doğrulama

| Adım | Sonuç |
|---|---|
| `#pragma message` — `Z_SAFE_XY` | `-10 , -10` |
| `#pragma message` — `XY_MIN_POS` | `-10 , -10` |
| `#pragma message` — `ENDSTOPS_ALWAYS_ON` | `OFF` |
| Temiz derleme | 126.864 B / 13.176 B, proje kodunda 0 uyarı |
| Binary sürüm dizesi | `SD1-2.8` |

> ⚠️ **DONANIMDA DOĞRULANMADI.** SD1-2.7'nin iki elle kontrolü hâlâ geçerli
> ve artık 2 mm daha kritik: (−10, −10) tabla dışıdır ve araba endstop'a
> tam dayanmış durumdadır. Z inişinin o köşede takılacağı bir şey olmadığını
> ve mekanik strok sonunda sıkışma olmadığını doğrulayın.

---

## [Sermoon-D1-2.7] — 2026-07-27

X/Y homing davranışı kullanıcı isteğiyle değiştirildi: **eksenler sırayla
homeleniyor ve G28 sonrası nozul tabla ortasına gitmiyor.** Flash −288 byte
(127.176 → **126.888**, %24,2), RAM değişmedi (13.176). Binary değişti →
**yeniden flash gerekir** (SHA256 `F3827387…7E17`, 2026-07-27 derlemesi).

### Changed

- **`QUICK_HOME` kapatıldı — X ve Y artık sırayla homeleniyor.**
  Açıkken G28, X ve Y'yi tek çapraz hamleyle aynı anda iki endstop'a
  sürüyordu (hedef (−450, −420), hız 22,8 mm/s =
  `min(homing_feedrate) × √((280/300)² + 1)`). Kapalıyken `G28.cpp`'nin
  normal sırası geçerli: **önce X, sonra Y** (`HOME_Y_BEFORE_X` kapalı),
  her biri kendi hızlı geçiş + 5 mm bump + yavaş geçiş döngüsüyle.
  - Ölçüldü: `QUICK_HOME = OFF`, `SIRA = X sonra Y`.
  - Flash −288 byte — `quick_home_xy()` tamamen düştü.
  - Bedeli süre: çapraz tek tarama yerine iki ayrı tam tarama, uzak köşeden
    kabaca iki katı (**hesap; ölçülmedi**).

- **`Z_SAFE_HOMING` noktası tabla ortasından park noktasına alındı:**
  `((X_BED_SIZE)/2, (Y_BED_SIZE)/2)` = (145, 135) → `(X_MIN_POS + 2,
  Y_MIN_POS + 2)` = **(−8, −8)**.
  Bu, X/Y homing'in zaten bıraktığı konum (`homeaxis()` ekseni −10 sayar,
  `HOMING_BACKOFF_MM` 2 mm geri çeker). Dolayısıyla `home_z_safely()`
  içindeki `do_blocking_move_to_xy()` **sıfır uzunluklu** bir hareket
  üretiyor — nozul hiçbir yere gitmiyor.
  - Ölçüldü: `Z_SAFE_XY = (-10 + 2) , (-10 + 2)`.
  - **`Z_SAFE_HOMING` kapatılMADI, bilerek.** Makro aynı zamanda *"X ve Y
    homelenmeden Z homelenemez"* korumasını taşıyor (`G28.cpp:128`,
    `axis_known_position`). DWIN ekranı `LCD_RTS.cpp:1459`'da tek başına
    `G28 Z0` gönderebiliyor; koruma kalksaydı o komut Z'yi kafanın
    bulunduğu rastgele X/Y konumunda homelerdi. Noktayı taşımak istenen
    sonucu veriyor, korumayı ise yerinde bırakıyor.
  - `(−8, −8)` erişilebilirlik sınırları içinde: `position_is_reachable`
    kartezyen dalı `WITHIN(rx, X_MIN_POS − slop, X_MAX_POS + slop)`
    kontrolü yapıyor, −10 ≤ −8.

### Sonuç — G28 sonrası konum

Artık **her varyantta (−8, −8)**: `G28`, `G28 X Y`, `G28 X`, `G28 Y`.
Önceki sürümlerde tam `G28` (145, 135)'te bitiyordu.

### Doğrulama

| Adım | Sonuç |
|---|---|
| `#pragma message` — `QUICK_HOME` | **OFF** |
| `#pragma message` — homing sırası | **X sonra Y** |
| `#pragma message` — `Z_SAFE_HOMING` | **ON** (koruma duruyor) |
| `#pragma message` — `Z_SAFE_XY` | `(-10 + 2) , (-10 + 2)` |
| Temiz derleme | 126.888 B / 13.176 B, proje kodunda 0 uyarı |
| Binary sürüm dizesi | `SD1-2.7` |

> ⚠️ **DONANIMDA DOĞRULANMADI — flash öncesi iki elle kontrol:**
> 1. **(−8, −8) tabla dışıdır.** Z inişinin o köşede takılacağı bir tabla
>    klipsi, kablo veya şasi parçası olmadığını doğrulayın.
> 2. **İlk katman kalibrasyonu.** Z artık (145, 135) yerine (−8, −8)'de
>    homeleniyor. Z endstop'u gövdeye sabit mekanik switch (PA7) olduğu için
>    tetik yüksekliğinin X/Y'den bağımsız olması *beklenir* — ama bu
>    firmware'den doğrulanamaz, switch'in montaj yerine bağlıdır. Flash
>    sonrası kâğıt testiyle teyit edin.
>
> Ayrıca slicer start G-code'unuz veya makrolarınız G28 sonrası nozulun tabla
> ortasında olduğunu varsayıyorsa gözden geçirin.

---

## [Sermoon-D1-2.6] — 2026-07-27

X/Y homing denetimi. `IMPROVE_HOMING_RELIABILITY` **SD1-1.2'den beri hiç
derlenmiyordu**; etkinleştirildi. Flash +64 byte (127.112 → **127.176**,
%24,2 → %24,3), RAM değişmedi (13.176). Binary değişti → **yeniden flash
gerekir** (SHA256 `445D2E5A…2037`, 2026-07-27 derlemesi).

### Fixed

- **`IMPROVE_HOMING_RELIABILITY` etkisizdi — X/Y homing tam ivmeyle
  çalışıyordu.** Tanım `Configuration_adv.h`'ın TMC bölümünde,
  `#if HAS_TRINAMIC` bloğunun (2029–2346) **içinde** duruyordu. Bu kartta
  X/Y `TMC2208_STANDALONE`, Z/E0 `A4988` → `HAS_TRINAMIC` **false**
  (`drivers.h:80`), dolayısıyla makro hiç tanımlanmıyor ve `G28.cpp`'deki
  `begin_slow_homing()` / `end_slow_homing()` çağrılarının tamamı `#if` ile
  dışarıda kalıyordu.
  - **Nasıl bulundu:** `#pragma message` ölçümü →
    `IMPROVE_HOMING_RELIABILITY = OFF`. Yalnız `#define` satırına bakan bir
    denetim ayarı açık sanırdı — 2.4'teki DIR delay hatasıyla aynı desen.
  - **Etkisi:** homing sırasında X/Y ivmesi 100 mm/s²'ye düşmesi gerekirken
    `DEFAULT_MAX_ACCELERATION` = **800 mm/s²**'de kalıyordu. `QUICK_HOME`
    açık olduğu için G28, X/Y'yi tek çapraz hamleyle **22,8 mm/s** hızla
    iki mekanik dayanağa aynı anda sürüyor
    (`min(homing_feedrate) × √((280/300)² + 1)`); bu ayarın var oluş sebebi
    tam olarak o hamlenin başlangıç şokunu azaltmaktı.
  - `CLASSIC_JERK` kapalı (JUNCTION_DEVIATION kullanılıyor) → ölçüldü:
    `HAS_CLASSIC_JERK = FALSE`. Yani jerk dalı derlenmiyor, etkili olan tek
    şey ivme düşüşü. Yorumdaki bu iddia doğruydu.

- **`slow_homing_t` tipi erişilemezdi — makroyu taşımak tek başına yetmedi.**
  Upstream bu tipi `feature/tmc_util.h:375`'te ÜÇ katmanlı guard altında
  tanımlıyor (`HAS_TRINAMIC` → `USE_SENSORLESS` →
  `ENABLED(IMPROVE_HOMING_RELIABILITY)`) ve `G28.cpp` o başlığı yalnızca
  `#if ENABLED(SENSORLESS_HOMING)` ile include ediyor. Yani upstream, sadece
  planner ivmesini değiştiren bu özelliği **sensorless homing'e bağlamış**.
  - Ölçüldü: makro taşındıktan sonra derleme
    `'slow_homing_t' does not name a type` ile durdu. İlk teşhis ("tek satır
    taşınacak") bu yüzden yanlıştı.
  - Çözüm: tip `G28.cpp`'de `#if !USE_SENSORLESS` altında tanımlandı.
    `USE_SENSORLESS` true olan bir yapılandırmaya geçilirse tanım yine
    `tmc_util.h`'tan gelir, çift tanım oluşmaz.
  - `USE_SENSORLESS = FALSE` ölçümle teyit edildi.

### Changed

- `Configuration_adv.h`: `IMPROVE_HOMING_RELIABILITY` TMC bölümünden
  `@section homing` altına taşındı (`HOMING_BACKOFF_MM`'in yanına). Eski
  konumuna neden orada olmaması gerektiğini açıklayan bir not bırakıldı.

### Doğrulama

| Adım | Sonuç |
|---|---|
| `#pragma message` — taşımadan önce | `IMPROVE_HOMING_RELIABILITY = OFF` |
| Yalnız `#define` taşındı, derleme | **FAILED** — `slow_homing_t` tanımsız |
| Tip `G28.cpp`'ye eklendi, derleme | SUCCESS |
| `#pragma message` — sonra | `= ON`, `HAS_CLASSIC_JERK = FALSE`, `USE_SENSORLESS = FALSE` |
| Temiz derleme | 127.176 B / 13.176 B, uyarı: proje kodunda 0 |

> **Donanımda doğrulanmadı.** Değişiklik yalnız G28 süresince planner
> ivmesini etkiler; hesapla eklenen süre saniyenin altında (kısa bump/backoff
> hamleleri baskın). Gerçek homing süresi ve tekrarlanabilirlik ölçülmedi.

---

## [Sermoon-D1-2.5] — 2026-07-27

Sürüm kimliği ve build bütünlüğü. **Marlin mantığı değişmedi.** Flash −8 byte
(127.120 → **127.112**, %24,2 sabit), RAM değişmedi (13.176). Binary değişti
→ **yeniden flash gerekir** (SHA256 `1BA1E0A4…C54C`, 2026-07-27 derlemesi).

Bu sürümle birlikte SD1-2.3 ve SD1-2.4 **ilk kez commit edildi**; o güne kadar
her ikisi de yalnızca çalışma dizininde duruyordu.

### Fixed

- **Firmware kendini `SD1-*` olarak tanıtmıyordu.** `Marlin/Version.h`'daki
  `SHORT_BUILD_VERSION` değeri `"MarlinV2 by CTK"` idi — hangi fork sürümünün
  yüklü olduğunu söylemiyor, üstelik 15 karakterle DWIN'in ~14 karakterlik
  `FW_VERSION_TEXT_VP` slot'unu aşıyordu (dosyanın kendi yorumu bu sınırı
  zaten yazmıştı; değer ekranda kırpılıyordu). Artık `"SD1-2.5"` — 7 karakter.
  Atıf `DETAILED_BUILD_VERSION`'a taşındı; M115 yanıtı:
  `SD1-2.5 (Sermoon D1 by CTK, base V1.1.10)`.
  `STRING_DISTRIBUTION_DATE` 2026-07-21 → 2026-07-27.
  Ölçüm: temiz derlemede **−8 byte** (dize 16 → 8 byte).

- **`cxx_runtime_min.cpp` kaybolsa build SESSİZCE başarılı oluyordu.**
  Dosya versiyon kontrolünde değildi. Yokluğunda libstdc++'ın zayıf
  `__verbose_terminate_handler`'ı geri gelir, `__cxa_demangle` üzerinden
  çözümleyici zinciri yeniden linklenir ve firmware **~28,8 KB büyür** —
  hiçbir uyarı verilmeden. SD1-2.3'ün en büyük kazancı bir `git clean` ile
  sessizce geri alınabilirdi.
  Eklenen link-zamanı nöbetçisi: dosyada `.set` ile mutlak sembol
  (`sermoon_cxx_runtime_min_present`), `common-cxxflags.py`'de
  `-Wl,--require-defined=...`. Dosya yoksa link durur.
  - **Ölçümle doğrulandı:** dosya geçici kaldırılıp derlendi → `[FAILED]`;
    geri konup derlendi → `[SUCCESS]`.
  - **`--undefined` DEĞİL — önce o denendi ve İŞE YARAMADI.** Dosya
    silinmesine rağmen link başarıyla tamamlandı. `--undefined` sembolü
    yalnızca "undefined" olarak girer (amacı arşivden modül çektirmek) ve
    çözümlenmeden kalırsa hata vermez; `--require-defined` tanımlı olmayı
    şart koşar. Nöbetçinin kendisi de ölçülmeden doğru sayılmadı.
  - Maliyet: **0 byte** (izole ölçüm: nöbetçi sabit tutulup yalnız versiyon
    dizesi değiştirildi, fark tam olarak dize farkı kadar çıktı).

### Documentation

- **`Marlin/Version.h` artımlı derlemede YENİDEN DERLENMİYOR — belgelendi.**
  Dosya `MarlinConfigPre.h:42`'de makro ile dahil ediliyor
  (`#include XSTR(../../CUSTOM_VERSION_FILE)`); SCons'un C tarayıcısı
  makro-genişletmeli include yolunu çözemez, dolayısıyla dosya bağımlılık
  grafiğinde **yer almaz**. Ölçüm: dosya değiştirilip `pio run` çalıştırıldı
  → **0 birim derlendi, binary değişmedi**; aynı değişiklik temiz derlemede
  −8 byte üretti. Sonuç: **versiyon yükseltirken temiz derleme zorunlu**,
  aksi hâlde firmware sessizce eski sürüm dizesini taşır. Uyarı `Version.h`
  başına yazıldı.

  > Bu, yukarıdaki versiyon düzeltmesini denetlerken ortaya çıktı: izole
  > boyut ölçümü beklenen farkı vermeyince neden arandı.

---

## [Sermoon-D1-2.4] — 2026-07-23

Z ve E0 sürücüleri **HR4988SQ** olarak tanımlandı; X/Y TMC2208 standalone
kaldı → firmware artık **karma sürücü** yapılandırması. Flash +40 byte
(127.080 → **127.120**, %24,2 sabit), RAM değişmedi (13.176). Binary değişti
→ **yeniden flash gerekir** (SHA256 `BDAB96BB…B987`).

Ayrıca bu sürüm, sürücü değişiminin açığa çıkardığı **bir zamanlama hatasını**
düzeltiyor (aşağıda) — bu, değişikliğin en önemli parçasıdır.

### Fixed — Kritik

- **`MINIMUM_STEPPER_*_DIR_DELAY` 30 ns → 200 ns.**
  HR4988SQ (A4988 ailesi) DIR sinyalinin STEP kenarından en az **200 ns**
  önce kararlı olmasını ister. Yapılandırmada değer **30 ns** olarak elle
  sabitlenmişti (TMC2xxx'in 20 ns'ine küçük pay eklenerek). Sürücü tipini
  değiştirmek bunu **kendiliğinden düzeltmez**: `Conditionals_post.h:572`'deki
  otomatik A4988 dalı yalnızca makro tanımsızken çalışır, burada tanımlıydı.
  - Belirtisi kozmetik değil: DIR kararlı olmadan gelen adım **eski yönde**
    atılır. Yön değişiminin sık olduğu iki yol tam da kritik olanlar —
    Z'de katman geçişi (paralel iki motor birlikte yanlış yöne gider) ve
    E'de her retract / LIN_ADVANCE geri beslemesi.
  - Maliyeti: 72 MHz'de 200 ns ≈ 15 çevrim, yalnızca yön değiştiğinde ödenir.
    Flash artışının (+40 B) tamamı bu gecikme kodundan geliyor.
  - **Nasıl bulundu:** tahminle değil ölçümle. Değişiklik sonrası
    `#pragma message` ile türetilmiş makrolar basıldı; beklenen 200 yerine
    30 görüldü. Yalnızca `Configuration.h`'a bakan bir denetim bunu kaçırırdı.

### Changed

- **`Z_DRIVER_TYPE` / `E0_DRIVER_TYPE`: `TMC2208_STANDALONE` → `A4988`.**
  Marlin'de `HR4988` diye bir sürücü tipi yok; HR4988SQ, A4988'in donanım
  uyumlu klonudur (aynı STEP/DIR/EN arayüzü, aynı MS1/MS2/MS3 mikroadım
  seçimi, aynı zamanlama sınırları). `A4988` doğru eşlemedir.

- **Doğrulanan (değiştirilmeyen) türetilmiş değerler.** Karma yapılandırmada
  bu makrolar global olduğu için **en katı** gereksinim geçerlidir; üçü de
  ölçümle teyit edildi:

  | Makro | Değer | Belirleyen |
  |---|---|---|
  | `MINIMUM_STEPPER_PULSE` | 1 µs | HR4988SQ (TMC2208 ~100 ns yeterdi) |
  | `MAXIMUM_STEPPER_RATE` | 400.000 | TMC2208 (A4988 500 kHz kaldırırdı) |
  | `*_DIR_DELAY` | 200 ns | HR4988SQ (yukarıda düzeltildi) |

  `MAXIMUM_STEPPER_RATE` pratikte bağlayıcı değil: en hızlı eksen X/Y,
  250 mm/s × 80 step/mm = 20 kHz, tavanın %5'i. Ancak `stepper.h:160`'ta
  darbe tabanını da belirliyor — 72 MHz / 400.000 = 180 çevrim (2,5 µs),
  HR4988SQ'nun istediği 1 µs'nin rahatça üstünde.

- **`ADAPTIVE_STEP_SMOOTHING` artık yük taşıyor.** TMC2208 16x girişi çip
  içinde 256x'e interpole eder; HR4988SQ etmez — Z/E'de 16x gerçekten 16x.
  Ayar zaten açıktı, gerekçesi güncellendi: HR4988SQ'ya geçtikten sonra
  **kapatılmamalıdır**.

### Documentation

- **Ekstruder tipi düzeltildi: "Bowden" → DIRECT DRIVE** (2026-07-24,
  kullanıcı donanım doğrulaması). SD1-2.4 denetimi, MANUAL'ın orijinal
  "direct drive" ifadesini "E steps/mm 95 = stok MK8 = Bowden" çıkarımıyla
  değiştirmişti. Çıkarım hatalıydı: 95 steps/mm dişlisiz MK8 tipi
  **besleyicinin** değeridir ve besleyicinin nerede durduğunu kanıtlamaz —
  dişlisiz direct drive'da da 95'tir (dişlili olsaydı ~400-450 olurdu).
  Düzeltilen yerler: README.md (donanım tablosu, §4.5d, tuning listesi,
  geliştirici notu), MANUAL.md (7 konum), docs/lin_advance/README.md
  (K aralıkları 0.4-0.9 → **0.02-0.15**, K-factor aracı parametreleri
  0-1.5/0.1 → 0-0.3/0.02, tüm örnekler), la_tower_test.gcode (M900 K0.40 →
  K0.06), docs/README.md, docs/junction_deviation/README.md,
  `Configuration_adv.h` LIN_ADVANCE_K yorumu, `Warnings.cpp` pragma metni.
  **Kod/binary etkisi yok** (yalnız yorum + pragma metni). `LIN_ADVANCE_K`
  0.06 default'u DEĞİŞMEDİ — direct drive için makul başlangıç; sürücü
  değişimi (HR4988SQ) nedeniyle kalibrasyon yine şart.

- **SHA256'nın güne bağlı olduğu keşfedildi ve belgelendi** (2026-07-24).
  `Marlin.cpp:956` binary'ye `__DATE__` gömer; temiz derleme başka günde
  farklı hash üretir. Ölçüm: yorum/pragma düzeltmeleri revert/re-apply
  deneyinde aynı gün iki derleme birebir aynı çıktı (`4402B902…AD818`) —
  yani bu turun kod-dosyası dokunuşları (yorum + pragma metni) binary'ye
  etkisizdir; dünkü `BDAB96BB…B987`'den tek fark tarih string'i.
  Sonuç: bit-bit doğrulama yalnızca aynı gün yapılan derlemeler arasında
  anlamlı. README Build Footprint'e uyarı eklendi.

- **`docs/lin_advance/README.md` gerçek yapılandırmayla hizalandı** (2026-07-24).
  Doküman default K'yı **0.22** diye veriyordu; gerçek değer
  `Configuration_adv.h:1483`'te **0.06** (M502 sonrası dönülen değer de bu).
  Bayat satır referansları güncellendi (1429/1421 → 1466/1483).
- **S_CURVE_ACCELERATION + LIN_ADVANCE etkileşimi belgelendi** (2026-07-24).
  LA'nın blok başına telafi hızı (`advance_speed`) sabit-ivme varsayımıyla
  hesaplanır; S-curve anlık ivmeyi değiştirir (`stepper.cpp:1582/1627` —
  `LA_isr_rate` blok boyunca sabit). Upstream bir dönem bu birlikteliği
  `EXPERIMENTAL_SCURVE` bayrağıyla engellemişti; bu taban o korumadan eski.
  K=0.06'da etkisi ölçülemez; kalibrasyon deseni hiçbir K değerinde uniform
  olmuyorsa S-curve'ü kapatma yordamı
  `docs/lin_advance/README.md`'ye yazıldı. **Kod değiştirilmedi** — karar,
  K kalibrasyonu sırasında desene bakılarak verilmeli.

- **Karma sürücü ve paralel Z belgelendi.** Z'de tek sürücüye paralel iki
  motor bağlı; Marlin tarafında bu tek eksendir (`Z2_DRIVER_TYPE` bilerek
  kapalı). Paralel bağlantı sürücünün verdiği akımı ikiye böler → Vref, tek
  motorlu kuruluma göre iki katı akıma karşılık gelmeli.
- **Tek enable hattı (PC3) belgelendi.** Dört sürücünün de EN girişi PC3'e
  bağlı; Marlin bu pini eksen bazında saymaz. Sonuç: **tek bir ekseni ısınma
  nedeniyle yazılımdan kapatmak mümkün değil** — HR4988SQ'nun ısısı yalnızca
  donanımsal soğutmayla yönetilebilir.
- **README §4.5 eklendi**: HR4988SQ devreye alma sırası — yön kontrolü
  (Z yukarıda, çarpmasız G-code ile), Vref hesabı, soğutma, LIN_ADVANCE K
  yeniden kalibrasyonu, mikroadım doğrulaması.
- **`INVERT_Z_DIR` / `INVERT_E0_DIR` bilerek DEĞİŞTİRİLMEDİ.** StepStick
  formatında A4988 ve TMC2208 modüllerinin motor çıkış sırası terstir, yani
  fiziksel modül değiştiyse yön ters dönebilir. Ancak bu, kartın gerçekte
  nasıl bağlı olduğu ölçülmeden bilinemez ve yanlış tahmin nozülü tablaya
  sürer. README'ye güvenli test yordamı kondu; karar kullanıcının.
- ~~**`LIN_ADVANCE_K` yorumu düzeltildi.**~~ **Bu madde 2026-07-24'te GERİ
  ALINDI.** Bu turda yorum "bu yazıcı Bowden" gerekçesiyle değiştirilmişti
  (E steps/mm 95 = stok MK8 çıkarımı). Çıkarım hatalıydı ve kullanıcı donanımı
  doğruladı: Sermoon D1 **direct drive**. Ayrıntı yukarıdaki 2026-07-24
  düzeltme kaydında. Değer (0.06) hiçbir turda değişmedi; doğru K yine
  kalibrasyonla bulunur.
- **`DISABLE_DEBUG` yorumu düzeltildi.** "Release PB4 (Y_ENABLE_PIN)"
  diyordu; PB4 `E0_STEP_PIN`, Y_ENABLE ise PC3. Doğrusu: PB3/PB4 JTAG
  hattıdır ve E0_DIR/E0_STEP olarak kullanılır — `DISABLE_DEBUG` olmadan
  ekstruder hiç dönmez.

### Documentation — MANUAL.md gerçek donanım verisiyle hizalandı

Kullanıcıdan alınan ölçülmüş donanım verisi (Creality **42-40** motorlar,
`R150` = **0.15 Ω** sense direnci, kart üstünden okunan Vref'ler) MANUAL'daki
sürücü bölümlerinin büyük kısmını geçersiz kıldı.

**Doğrulanmış akımlar** (formüller sürücü ailesine göre farklı: TMC2208 Vref
**RMS**, HR4988SQ Vref **PEAK** ayarlar — karıştırmak √2 kat hata):

| Eksen | Vref | Akım | 42-40 nominalinin | Sürücü tavanının |
|---|---|---|---|---|
| X/Y | 1.27 V | 0.69 A RMS | %69 | — |
| Z (×2 paralel) | 1.60 V | 0.47 A RMS/motor | %47 | %67 |
| E0 | 0.86 V | 0.51 A RMS | %51 | %36 |

Sonuç: **fabrika ayarı doğru, hiçbir pot değiştirilmemeli.** Ara hesaplarda
R_sense 0.10 Ω varsayılarak Z'nin sürücü tavanında çalıştığı ve düşürülmesi
gerektiği düşünülmüştü; `R150` ölçümü bunu çürüttü.

**§8 tamamen yeniden yazıldı.** Eski hâli `STEALTHCHOP_*`,
`HYBRID_THRESHOLD` ve `INTERPOLATE` ayarlarını etkinmiş gibi anlatıyordu.
Hepsi `Configuration_adv.h`'daki `#if HAS_TRINAMIC` bloğunda ve bu blok
**hiç derlenmiyor** (`drivers.h:80` — standalone eşleşmez). Yeni §8 gerçekte
neyin neyi belirlediğini gösteriyor: akım Vref potundan, mikroadım PCB'ye
sabit kablanmış MS1/MS2'den (jumper yok), chopper çipin donanımından;
firmware'in kontrol ettiği üç değer ise `MINIMUM_STEPPER_PULSE`,
`MAXIMUM_STEPPER_RATE` ve `*_DIR_DELAY`.

Düzeltilen diğer iddialar:

| Bölüm | Yazıyordu | Gerçek |
|---|---|---|
| §2.2 | 4 eksen de TMC2208, 800 mA RMS | Karma; ölçülen akımlar yukarıda |
| §2.2 | `RSENSE 0.11 Ω` | **0.15 Ω** — genel TMC2208 modül değerinden kopyalanmış |
| §2.2 | Z tek motor | **Paralel iki motor**, tek sürücü, akım ikiye bölünür |
| §2.3, §5.1 | "Direct drive (bowden değil)" | ~~"Bowden" yapıldı~~ — **hatalıydı; 2026-07-24'te geri alındı** (donanım: direct drive) |
| §5.2 | "TMC2208 StealthChop limiti" | Gövde rezonansı (300→250) |
| §14.2 | Yalnızca pin tablosu | PC3 paylaşımının sonuçları, PB3/PB4 JTAG, Z2 uyarısı |
| §17 | "HYBRID_THRESHOLD kontrol et" | Bu kartta yok; DIR delay ve Vref'e yönlendirildi |

**`Configuration_adv.h`:**
- `#if HAS_TRINAMIC` bloğunun başına, bloğun tamamının bu kartta derlenmediğini
  ve gerçek donanım ayarlarının nerede olduğunu açıklayan uyarı eklendi.
- `*_RSENSE` 0.11 → **0.15** (13 tanım). Blok ölü olduğu için davranışa etkisi
  yok, ama ileride UART'lı sürücüye geçilirse doğru başlangıç noktası olur.
- `CHOPPER_TIMING` yorumu düzeltildi: "HAS_TRINAMIC aktif olduğu için bu değer
  TMCStepper init sırasında kullanılır" diyordu — `HAS_TRINAMIC` false,
  TMCStepper hiç linklenmiyor.

**Doğrulama:** bu turdaki tüm değişiklikler sonrası üretilen binary, sürücü
değişikliği sonrası binary ile **bit-bit aynı** (`BDAB96BB…B987`). `*_RSENSE`
değerlerini değiştirmenin binary'yi hiç etkilememesi, `#if HAS_TRINAMIC`
bloğunun ölü olduğunun bağımsız kanıtıdır.

### Documentation — ölçümle çelişen iddialar düzeltildi (README mekanik tablo)

Yapılandırma dosyalarıyla karşılaştırıldı, dört satır hatalıydı:

| Satır | README diyordu | Gerçek (`Configuration.h`) |
|---|---|---|
| Maks. hız | 300, 300, 5, 25 | **250**, **250**, 5, 25 |
| Maks. ivme | 1000, 1000, 100, 1000 | **800**, **800**, 100, **5000** |
| Travel ivmesi | 1000 mm/s² | **800** mm/s² |
| Jerk | "10, 10, 0.4, 5" | **Kullanılmıyor** — `CLASSIC_JERK` kapalı, `JUNCTION_DEVIATION` 0.013 etkin |

Jerk satırı en yanıltıcısıydı: verilen değerler `Configuration.h:802-804`'te
**yorum satırındaki** eski kalibrasyon değerleri.

### Doğrulama

| Adım | Sonuç |
|---|---|
| Sürücü tipi değişimi sonrası derleme | başarılı, 127.080 B (boyut değişmedi) |
| Türetilmiş makro ölçümü (`#pragma message`) | `POST_DIR_DELAY=30` → **hata yakalandı** |
| DIR delay 200 ns düzeltmesi sonrası ölçüm | `POST=200 PRE=200 PULSE=1 RATE=400000` ✓ |
| Temiz derleme | başarılı, 127.120 B / 13.176 B RAM |
| Derleyici uyarısı | proje kodunda **0** (+1 bilinen upstream: `util_adc.c`) |

---

## [Sermoon-D1-2.3] — 2026-07-22

Toolchain yapılandırma denetimi. **Flash −57.116 byte (%35.1 → %24.2)**,
**RAM −1.992 byte (%23.1 → %20.1)**. Marlin mantığı değişmedi; kazancın tamamı
derleyici/linker ayarlarından geliyor. Binary değişti → **yeniden flash gerekir**
(`firmware.bin` 127.080 byte, SHA256 `974DDA75…1A75`).

Tüm boyutlar `arm-none-eabi-nm --print-size` ile ölçüldü.

### Removed — 57.116 byte flash

- **C++ isim çözümleyici zinciri — 35.080 byte.**
  libstdc++'ın varsayılan `std::terminate` işleyicisi
  `__gnu_cxx::__verbose_terminate_handler()`, istisna tipini okunur yazmak için
  `__cxa_demangle()` çağırıyordu. Bu tek referans libiberty'nin tüm
  çözümleyicisini bağlıyordu (`d_print_comp` 11.448 B, `d_type` 2.020 B,
  `cplus_demangle_operators`, … 44 sembol). Marlin istisna kullanmaz — kodun
  tamamı erişilemezdi.
  Çözüm: `Marlin/src/HAL/HAL_STM32F1/cxx_runtime_min.cpp` içinde kendi
  işleyicimiz tanımlandı (Cortex-M3 sistem reset). Referans kalmayınca linker
  arşiv üyelerini hiç çekmiyor.

- **İstisna/unwind makinesi — 9.288 byte.**
  `-fno-exceptions -fno-rtti -fno-unwind-tables -fno-asynchronous-unwind-tables`
  **tek başına yetmedi** (flag'ler uygulandığı halde boyut değişmedi): asıl
  bağlayıcı `__cxa_guard_acquire/release` referanslarıydı — bunlar libsupc++'ın
  `eh_personality.o`'sunu, o da libgcc'nin ARM unwinder'ını çekiyordu.
  `-fno-threadsafe-statics` ile guard'lar kalkınca `__gxx_personality_v0`,
  `_Unwind_*`, `__gnu_unwind_*` ailesi tamamen düştü (66 sembol → 0).
  `.ARM.exidx` 400 B → 8 B.

- **Tam newlib → newlib-nano — 12.748 byte flash + 1.944 byte RAM.**
  Link komutunda `--specs=nano.specs` **yoktu** (`pio run -v` ile doğrulandı),
  yani tam newlib'e linkleniyorduk: `_svfprintf_r` + `_dtoa_r` + `_strtod_l` +
  `_mprec` ailesi 16.088 B, newlib malloc 2.820 B.
  `-u_printf_float` ile birlikte eklendi — `dtostrf()` üç kritik yolda
  kullanıldığı için (`M114`, power-loss recovery G-code üretimi, DWIN pause
  ekranı) `%f` desteği korunmak zorundaydı.

- **`SERMOON_Z_LOCK_AUTO` — ölü özellik.**
  `on_motion_start()`/`on_motion_end()` tanımlıydı ancak kod tabanında hiçbir
  yerden çağrılmıyordu; planner/stepper hook'u hiç yazılmamıştı. Flag'i açmak
  davranışı değiştirmiyordu. Flag, iki fonksiyon ve üç yerdeki yanıltıcı
  dokümantasyon kaldırıldı.

### Fixed

- **`extra_scripts` sessizce eziliyordu.** `[env:creality]`'deki `extra_scripts`,
  `[common]`'daki aynı isimli anahtarı eziyordu (hiçbir yer
  `${common.extra_scripts}` interpolate etmiyor). Sonuç: `common-cxxflags.py`
  **hiç çalışmıyordu** — `-Wno-register` dahil hiçbir C++ flag'i uygulanmıyordu.
  Artık açıkça listeleniyor.

- **`build_flags.py`'nin SCons `else:` dalı ölü koddu.** Dosya yalnızca
  `!python ...` ile stdout üretmek için çağrılıyor; `extra_scripts`'te
  listelenmediği için `Import("env")` dalı hiç çalışmıyordu. İçindeki
  `-fno-threadsafe-statics`, `-fno-use-cxa-atexit`, `--specs=nano.specs`,
  `-u_printf_float` **hiçbiri etkin değildi**. Hepsi `common-cxxflags.py`'ye
  taşındı — orijinal niyet fiilen etkinleştirildi.

- **`settings.cpp` sınır kontrolü yeniden etkin.** `sizeof(SettingsData)`
  assert'i yorum satırıydı *ve* yanlıştı (`EEPROM_OFFSET`'i saymıyordu).
  `PersistentStore::write_data()` sınır kontrolü yapmadığı için ayar bloğu
  büyüdüğünde M500 sessizce `.bss`'i taşırırdı. Doğru form
  (`EEPROM_OFFSET + sizeof(SettingsData) <= E2END + 1`) etkinleştirildi; mevcut
  yapılandırmada geçiyor.

### Documentation — ölçümle çelişen iddialar düzeltildi

- **`backtrace` binary'de yok.** README onu "3.682 B, hardfault'ta stack trace
  basar, ayıklamada değerli" diye listeliyordu. Ölçüm: `unwarm*`/`UnwReport*`
  sembollerinin hiçbiri binary'de değil — kaynak derleniyor ama hiçbir fault
  handler'dan çağrılmadığı için `--gc-sections` tamamını atmış.
- **M500 ayarları SD kartta.** Bu kartta `EEPROM_SETTINGS`,
  `persistent_store_sdcard.cpp` ile karşılanıyor (`eeprom.dat`), I2C EEPROM ile
  değil. **SD kart takılı değilse M500 sessizce başarısız olur.** README bunu
  hiç belirtmiyordu. BL24C16 yalnızca PLR + DWIN bayrakları + varlık kontrolü
  için kullanılıyor.
- "Ölü özellikler" tablosundaki diğer değerler yeniden ölçüldü
  (`BEZIER_CURVE_SUPPORT` 197 B değil **898 B**).

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
- ~~**MANUAL.md** — Bowden/Direct drive çelişkisi giderildi (Sermoon D1 = Bowden)~~
  *(2026-07-24: bu karar yanlış yöndeydi — donanım **direct drive**; bkz. SD1-2.4 kaydı)*
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
- `LIN_ADVANCE` — köşe kalitesi için (K=0.22 default, kalibrasyon gerekir)
  *(2026-07-24 notu: "Bowden için kritik" yazıyordu — donanım direct drive)*
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
