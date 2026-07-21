; ═══════════════════════════════════════════════════════════════════
; Sermoon D1 — Junction Deviation Karakter Testi
; ═══════════════════════════════════════════════════════════════════
;
; Bu gcode köşe davranışını gözleyebileceğin hızlı bir test çalıştırır.
; Hiçbir extrusion yok — sadece motor sesleri ve hareket karakteri.
;
; Aynı pattern'i 3 farklı JD ile koşturmak için yorum satırlarını değiştir.
; Köşe ses farklarını duyabilirsin (insan kulağı korner overshoot'unu hisseder).
;
; KULLANIM:
;   1. SD karta kopyala
;   2. Yazıcı SOĞUK olabilir (extrusion yok)
;   3. Çalıştır, yazıcı 90° / 45° / kavis denemeleri yapar
;   4. Aşağıdaki M205 J satırını değiştirip 3 kez koş — farkı duy/gör
;
; ═══════════════════════════════════════════════════════════════════

; ── BURAYI DEĞİŞTİR ──
M205 J0.013        ; JD test değeri (önerilen: 0.008, 0.013, 0.020)
; ─────────────────────

;----- SETUP -----
G21                ; mm
G90                ; absolute
G92 E0
M83                ; E relative (extrusion olmayacak ama yine)

; Home (gerekli)
G28
G1 Z5 F600         ; Z biraz kaldır

; Test alanı: Yatağın merkezi
G1 X50 Y50 F6000

;----- TEST PATTERN A: 90° köşeler -----
M117 90deg corners

G1 X150 Y50 F8000   ; Sağa
G1 X150 Y150 F8000  ; Yukarı (90° turn)
G1 X50 Y150 F8000   ; Sol
G1 X50 Y50 F8000    ; Aşağı
G4 P500             ; 0.5 sn dur

; Tekrarla — 3 lap
G1 X150 Y50 F8000
G1 X150 Y150 F8000
G1 X50 Y150 F8000
G1 X50 Y50 F8000
G4 P500

;----- TEST PATTERN B: Zigzag / 60° köşeler -----
M117 zigzag 60deg

G1 X70 Y70 F12000
G1 X120 Y100 F12000
G1 X70 Y130 F12000
G1 X120 Y160 F12000
G1 X70 Y190 F12000
G1 X120 Y100 F12000
G1 X70 Y70 F12000
G4 P500

;----- TEST PATTERN C: Star — sharp angles -----
M117 sharp star

G1 X100 Y50 F10000
G1 X130 Y130 F10000   ; sharp angle
G1 X50 Y100 F10000    ; sharp
G1 X150 Y100 F10000   ; sharp
G1 X70 Y130 F10000    ; sharp
G1 X100 Y50 F10000    ; geri
G4 P500

;----- TEST PATTERN D: Curve — smooth path -----
M117 curve

G1 X100 Y100 F8000
G2 X100 Y100 I30 J0 F10000   ; tam tur
G3 X100 Y100 I-30 J0 F10000  ; ters yön

;----- CLEANUP -----
G91
G1 Z10 F600
G90
G28 X Y
M84                  ; Motors off
M117 JD test done — duy/gor

; ═══════════════════════════════════════════════════════════════════
; ANALİZ:
;   Pattern A (90°): köşelerde "tık" sesi/durma → JD çok düşük olabilir
;   Pattern B (zigzag): yumuşak akış olmalı; titriyorsa JD çok yüksek
;   Pattern C (sharp): köşelerde overshoot/wobble varsa JD çok yüksek
;   Pattern D (curve): pürüzsüz olmalı; tıklıyorsa düz çizgi segmentation
;
; Optimal JD = pattern A,B,C smooth + pattern D pürüzsüz
; ═══════════════════════════════════════════════════════════════════
