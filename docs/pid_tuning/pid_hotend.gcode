; ═══════════════════════════════════════════════════════════════════
; Sermoon D1 — Hotend PID Auto-Tuning
; ═══════════════════════════════════════════════════════════════════
;
; Bu dosyayı SD karta kopyalayıp ekrandan "pid_hotend.gcode" seçin.
; Yazıcının soğuk olması ÖNERİLİR (önce M104 S0 ile soğutun).
; Yaklaşık süre: 8-12 dakika
;
; PARAMETRELER:
;   M303 E0  → hotend (extruder 0)
;   S210     → 210°C tuning sıcaklığı (PLA/PETG için tipik)
;              ABS için S240, PETG için S230 kullanın
;   C8       → 8 cycle (yüksek kalite; min 3, default 5)
;   U1       → sonuçları otomatik aktive et (uygulama)
;
; SONUÇ:
;   M500     → EEPROM'a kaydet (script sonunda otomatik)
;
; Tuning sırasında nozül 210°C'ye ısınır, sonra ~%50 PWM ile sürekli
; salınım yapar. Ekranda Kp/Ki/Kd değerleri raporlanır.
;
; ═══════════════════════════════════════════════════════════════════

; Başlangıç: emniyetli soğuk durum
M107                           ; Fan kapalı
M104 S0                        ; Hotend soğut
M140 S0                        ; Yatak soğut

; Part cooling fan'ı sabit %50'de tut (gerçek print koşullarına yakın)
M106 S128

; PID tuning başlat
M303 E0 S210 C8 U1

; Sonuçları EEPROM'a kaydet
M500

; Soğuma + temizlik
M107
M104 S0

; Bitti — M501 ile değerleri tekrar yükleyebilir, M503 ile görebilirsiniz.
