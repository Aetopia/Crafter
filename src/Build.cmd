@echo off

cd "%~dp0"
rmdir /Q /S "bin">nul 2>&1
mkdir "bin">nul 2>&1

windres.exe -i "Resources\.rc" -o "%TEMP%\.o"

gcc.exe -s -Os -Wl,--gc-sections -mwindows -nostdlib "WinMain.c" "%TEMP%\.o" -lUser32 -lKernel32 -lAdvapi32 -lOle32 -lComdlg32 -lComctl32 -o "bin\Crafter.exe"

del "%TEMP%\.o" 
upx --best --ultra-brute "bin\*">nul 2>&1