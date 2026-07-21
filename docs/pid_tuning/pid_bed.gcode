; ═══════════════════════════════════════════════════════════════════
; Sermoon D1 — Heated Bed PID Auto-Tuning
; ═══════════════════════════════════════════════════════════════════
;
; Bu dosyayı SD karta kopyalayıp ekrandan "pid_bed.gcode" seçin.
; Yazıcının soğuk olması ÖNERİLİR.
; Yaklaşık süre: 25-40 dakika (yatak hotend'e göre çok daha yavaş ısınır)
;
; PARAMETRELER:
;   M303 E-1 → heated bed
;   S60      → 60°C tuning sıcaklığı (PLA için tipik)
;              ABS için S100, PETG için S80, ısı yoğun materyal için yüksek
;   C5       → 5 cycle (yatak için yeterli; daha fazla zaman alır)
;   U1       → sonuçları otomatik aktive et
;
; NOT:
;   Yatak silikon ısıtıcılarında PID osilasyonu yavaş olduğundan
;   tuning uzun sürer. Sermoon kapalı kabinde kabin sıcaklığı
;   ortam sıcaklığına etki eder; tuning sırasında kabin kapalı tutun
;   ki sonuç gerçek print koşullarına uysun.
;
; ═══════════════════════════════════════════════════════════════════

; Başlangıç: emniyetli soğuk durum
M107                           ; Fan kapalı
M104 S0                        ; Hotend soğut
M140 S0                        ; Yatak soğut

; PID tuning başlat (yatak)
M303 E-1 S60 C5 U1

; Sonuçları EEPROM'a kaydet
M500

; Soğuma
M140 S0

; Bitti — M501 / M503 ile değerleri kontrol edebilirsiniz.
