; ═══════════════════════════════════════════════════════════════════
; Sermoon D1 — LIN_ADVANCE Quick K Test
; ═══════════════════════════════════════════════════════════════════
;
; Bu dosya tek bir K değerini kısa sürede test eder. Aynı dosyayı
; M900 satırını farklı K değerleri ile birkaç kez çalıştırarak en
; iyiyi seçebilirsin. (Tower test için sonraki dosyaya bak.)
;
; PATTERN:
;   - 5 paralel zigzag çizgisi
;   - Her çizgide: yavaş (20mm/s) → hızlı (80mm/s) → yavaş geçişler
;   - Hız değişimleri = LA'nın etkili olduğu yer
;
; KULLANIM:
;   1. Aşağıdaki M900 K satırını test edilmek istenen değer ile değiştir
;   2. SD'ye kopyala, çalıştır
;   3. Yatağa yandan bak: yavaş↔hızlı geçişlerde duvar uniform mu?
;        - Geçişte BLOB (tıknaz şişme) → K çok düşük
;        - Geçişte DELİK (incelme) → K çok yüksek
;        - Uniform → bingo, K doğru
;   4. K'yı 0.1 artırarak/azaltarak tekrar dene (5-6 deneme yeterli)
;
; ═══════════════════════════════════════════════════════════════════

; ── BURAYI DEĞİŞTİR ──
M900 K0.40                ; Sermoon Bowden tipik aralık: 0.4 - 1.2
; ─────────────────────

;----- SETUP -----
G21                       ; mm
G90                       ; absolute
M82                       ; E absolute

M140 S60                  ; Yatak
M104 S210                 ; Hotend
M190 S60
M109 S210

G28                       ; Home

;----- PURGE -----
G1 Z2 F600
G1 X20 Y20 F6000
G1 Z0.3 F600
G92 E0
G1 X120 Y20 E10 F1500     ; Purge line
G1 X120 Y20.5 F6000
G1 X20 Y20.5 E20 F1500
G92 E0
G1 Z2 F600

;----- TEST PATTERN: 5 line, slow-fast-slow-fast-slow -----
; Her line 100mm uzunluğunda, ortasında hız değişimleri
; 5mm separation between lines

G1 X20 Y50 Z0.2 F6000
G92 E0

; Line 1
G1 X40  Y50 E1.5 F1200       ; 20mm slow start
G1 X80  Y50 E4.5 F4800       ; 40mm fast    ← LA etkisi burada
G1 X100 Y50 E6.0 F1200       ; 20mm slow
G1 X120 Y50 E7.5 F4800       ; 20mm fast
G1 X140 Y50 E9.0 F1200       ; 20mm slow end

; Travel
G1 Z0.5 F600
G1 X20 Y55 F6000
G1 Z0.2 F600
G92 E0

; Line 2 (same pattern, parallel)
G1 X40  Y55 E1.5 F1200
G1 X80  Y55 E4.5 F4800
G1 X100 Y55 E6.0 F1200
G1 X120 Y55 E7.5 F4800
G1 X140 Y55 E9.0 F1200

; Travel
G1 Z0.5 F600
G1 X20 Y60 F6000
G1 Z0.2 F600
G92 E0

; Line 3
G1 X40  Y60 E1.5 F1200
G1 X80  Y60 E4.5 F4800
G1 X100 Y60 E6.0 F1200
G1 X120 Y60 E7.5 F4800
G1 X140 Y60 E9.0 F1200

; Travel
G1 Z0.5 F600
G1 X20 Y65 F6000
G1 Z0.2 F600
G92 E0

; Line 4
G1 X40  Y65 E1.5 F1200
G1 X80  Y65 E4.5 F4800
G1 X100 Y65 E6.0 F1200
G1 X120 Y65 E7.5 F4800
G1 X140 Y65 E9.0 F1200

; Travel
G1 Z0.5 F600
G1 X20 Y70 F6000
G1 Z0.2 F600
G92 E0

; Line 5
G1 X40  Y70 E1.5 F1200
G1 X80  Y70 E4.5 F4800
G1 X100 Y70 E6.0 F1200
G1 X120 Y70 E7.5 F4800
G1 X140 Y70 E9.0 F1200

;----- CLEANUP -----
G91
G1 Z10 F600
G90
G28 X Y
M104 S0
M140 S0
M84
M117 K=0.40 done — check & retry
