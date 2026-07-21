import os
import time

try:
    import win32file
    import win32con
except:
    print("pywin32 yukleniyor...")
    os.system('pip install pywin32 -q')
    import win32file
    import win32con

print("=== STM32 Bootloader Scanner ===\n")

bauds = [1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200]

for baud in bauds:
    try:
        hCom = win32file.CreateFile(
            r'\\.\COM8',
            win32con.GENERIC_READ | win32con.GENERIC_WRITE,
            0,
            None,
            win32con.OPEN_EXISTING,
            0,
            None
        )

        # Simple DCB setup
        dcb = win32file.GetCommState(hCom)
        dcb.BaudRate = baud
        dcb.ByteSize = 8
        dcb.Parity = win32con.NOPARITY
        dcb.StopBits = win32con.ONESTOPBIT
        win32file.SetCommState(hCom, dcb)

        win32file.SetCommTimeouts(hCom, 0, 100, 0, 0, 100)
        win32file.PurgeComm(hCom, win32file.PURGE_RXCLEAR | win32file.PURGE_TXCLEAR)

        # Send 0x7F
        win32file.WriteFile(hCom, bytes([0x7F]))
        time.sleep(0.1)

        try:
            data = win32file.ReadFile(hCom, 1)[1]
            if data and data[0] == 0x79:
                print(f"BOOTLOADER BULUNDU! Baud: {baud}")
                win32file.CloseHandle(hCom)
                break
            elif data:
                print(f"{baud} baud: {hex(data[0])}")
        except:
            pass

        win32file.CloseHandle(hCom)

    except Exception as e:
        if "access" in str(e).lower():
            print("COM8 kullanimda! Tum programlari kapat.")
            break
        print(f"{baud}: Hata")
