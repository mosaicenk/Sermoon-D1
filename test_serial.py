import serial
import time

try:
    # COM8'i aç
    ser = serial.Serial('COM8', 115200, timeout=1)
    print("✅ COM8 açık")

    # Buffer'ı temizle
    ser.reset_input_buffer()
    ser.reset_output_buffer()

    # M115 komutu gönder (firmware info)
    ser.write(b'M115\n')
    time.sleep(0.5)

    # Cevabı oku
    if ser.in_waiting > 0:
        response = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
        print(f"📩 Cevap: {response}")
    else:
        print("❌ Cevap yok")

    ser.close()
    print("✅ COM8 kapandı")

except Exception as e:
    print(f"❌ Hata: {e}")
