; ═══════════════════════════════════════════════════════════════════
; Sermoon D1 — EEPROM Backup (pre-flash güvenlik)
; ═══════════════════════════════════════════════════════════════════
;
; YENİ FIRMWARE FLASH ETMEDEN ÖNCE çalıştır.
;
; Amaç: Mevcut firmware'in M500 ile EEPROM'a yazdığı tüm değerleri
; (PID, steps/mm, jerk, accel, Z-offset, mesh, lin_advance K, vb.)
; serial console'a dump eder. Bu çıktıyı bir text dosyasına KAYDET ET.
;
; Yeni firmware flash ettikten sonra problem çıkarsa:
;   - Eski değerleri elle re-input edebilirsin
;   - Veya eski firmware'e geri dönerken tekrar M500 yapabilirsin
;
; KULLANIM:
; 1. Yazıcıyı host'a bağla (PrusaSlicer console, OctoPrint, Pronterface, vb.)
; 2. Bu dosyayı SD karta atma; doğrudan host'tan göndermek daha kolay
; 3. Host'ta gelecek output'u "save log" ile yedekle
;
; Veya SD'den çalıştırıyorsan: yazıcıya bağlı bir log alma cihazı
; olmadığı için fayda azalır — host yöntemini tercih et.
;
; ═══════════════════════════════════════════════════════════════════

; Console banner
M118 ===== EEPROM BACKUP START =====
M118 Sermoon D1 — Firmware ayarları yedeği
M118 Tarih: kullanıcı not düşsün

; Firmware tanıtım
M115                  ; Marlin version + capabilities

; Tüm EEPROM ayarlarını dump et
M503                  ; Steps, feedrate, accel, jerk, PID, probe offset, vb.

; LIN_ADVANCE K (M503 içinde de görünür ama netlik için ayrıca)
M900

; Probe Z-offset (BLTouch varsa)
M851

; Skew correction (varsa)
M852

; Print counter (PRINTCOUNTER aktifse)
M78

; Console banner
M118 ===== EEPROM BACKUP END =====
M118 Bu cikti'yi bir text dosyasina KAYDET.
M118 Yeni firmware flash sonrasi gerekirse manuel re-input icin saklayin.
