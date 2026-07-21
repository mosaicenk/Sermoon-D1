"""
Sermoon D1 Marlin Test Script
COM8 (CH340) üzerinden Marlin firmware test
"""

import sys
import time

# PySerial yoksa basit approximation
try:
    import serial
    HAS_SERIAL = True
except ImportError:
    HAS_SERIAL = False
    print("⚠️ PySerial modülü yok. Alternatif yöntem:")
    print("   Arduino IDE → Tools → Serial Monitor → COM8 → 115200 baud")
    print("   Oradan 'M115' yazıp gönder")
    sys.exit(1)

def test_marlin_connection(port='COM8', baud=115200):
    """Marlin'e bağlanıp test et"""

    print(f"🔌 {port} bağlanılıyor... ({baud} baud)")

    try:
        ser = serial.Serial(
            port=port,
            baudrate=baud,
            timeout=2,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE
        )

        if not ser.is_open:
            print("❌ Port açılamadı")
            return False

        print(f"✅ {port} açık")

        # Buffer temizle
        ser.reset_input_buffer()
        ser.reset_output_buffer()

        # M115 - Firmware info
        print("\n📤 M115 gönderiliyor...")
        ser.write(b'M115\n')

        # Cevabı bekle
        time.sleep(1)

        if ser.in_waiting > 0:
            response = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
            print(f"📩 Cevap:\n{response}")
            ser.close()
            return True
        else:
            print("❌ Cevap yok")

        ser.close()
        return False

    except serial.SerialException as e:
        print(f"❌ Serial hatası: {e}")
        return False
    except Exception as e:
        print(f"❌ Hata: {e}")
        return False

if __name__ == '__main__':
    # Farklı baud rate'leri dene
    bauds = [115200, 250000, 57600, 38400, 19200, 9600]

    print("=== Sermoon D1 Marlin Test ===\n")

    for baud in bauds:
        print(f"\n{baud} baud deneniyor...")
        if test_marlin_connection('COM8', baud):
            print(f"\n✅ BAŞARILI - {baud} baud ile bağlantı sağlandı!")
            break
    else:
        print("\n❌ Tüm baud rate'ler başarısız")
        print("\nÖneriler:")
        print("1. Printer açık mı kontrol et")
        print("2. USB kablosu düzgün takılı mı")
        print("3. Device Manager'da COM8 görünüyor mu")
