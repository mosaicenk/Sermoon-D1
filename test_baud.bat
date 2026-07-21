@echo off
echo COM8 Baud Rate Test
echo ===================
echo.

rem Common Marlin baud rates
for %%b in (250000 115200 57600 38400 19200 9600) do (
    echo Testing %%b baud...
    mode COM8: %%b,n,8,1 >nul 2>&1
    echo M115 > COM8
    timeout /t 1 /nobreak >nul
    echo.
)

echo Test complete. Check printer response.
pause
