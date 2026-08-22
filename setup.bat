@echo off
setlocal EnableDelayedExpansion
title AndroidRun - Setup
mode con cols=100 lines=40
cd /d "%~dp0"

echo ================================================================
echo   AndroidRun v1.4 - Self-Contained Setup  (works 100%% OFFLINE)
echo ================================================================
echo.
echo   Everything you need is bundled in this package:
echo     - QEMU 11.1     (the machine engine, GPL open source)
echo     - Android-x86 9 (Android OS, x86_64 + ARM 32-bit apps)
echo     - ARM engine    (houdini - runs 32-bit ARM APKs)
echo     - ADB tools     (APK installer)
echo   No ads. No account. No internet needed after setup.
echo ================================================================
echo.

set "BASE=%~dp0"

rem ============ 1) QEMU engine ============
set "QSYS="
if exist "%BASE%engine\qemu\qemu-system-x86_64.exe" set "QSYS=%BASE%engine\qemu\qemu-system-x86_64.exe"
if not defined QSYS (
    echo [1/4] QEMU engine not found in this folder.
    echo        Downloading it from the official GitHub release ^(one time^)...
    powershell -NoProfile -Command "$ErrorActionPreference='Stop'; [Net.ServicePointManager]::SecurityProtocol=[Net.SecurityProtocolType]::Tls12; try { Start-BitsTransfer -Source 'https://github.com/FAkeUsership/mr-free/releases/latest/download/qemu-portable-w64-11.1.0.zip' -Destination '%BASE%qemu-portable.zip' } catch { Invoke-WebRequest -Uri 'https://github.com/FAkeUsership/mr-free/releases/latest/download/qemu-portable-w64-11.1.0.zip' -OutFile '%BASE%qemu-portable.zip' }"
    if not exist "%BASE%qemu-portable.zip" ( echo [FAIL] QEMU download failed. Check internet or copy the full package. & goto :end )
    powershell -NoProfile -Command "Expand-Archive -Path '%BASE%qemu-portable.zip' -DestinationPath '%BASE%' -Force"
    if exist "%BASE%engine\qemu\qemu-system-x86_64.exe" ( set "QSYS=%BASE%engine\qemu\qemu-system-x86_64.exe" ) else ( echo [FAIL] QEMU extraction failed. & goto :end )
    echo [ok] QEMU engine ready.
) else (
    echo [1/4] QEMU engine: present ^(bundled^)
)
set "QIMG=%BASE%engine\qemu\qemu-img.exe"
if not exist "%QIMG%" ( echo [FAIL] qemu-img.exe missing. & goto :end )

rem ============ 2) ADB ============
if exist "%BASE%adb\adb.exe" (
    echo [2/4] ADB tools: present ^(bundled^)
) else (
    echo [2/4] ADB tools not found. Downloading...
    powershell -NoProfile -Command "$ErrorActionPreference='Stop'; [Net.ServicePointManager]::SecurityProtocol=[Net.SecurityProtocolType]::Tls12; try { Start-BitsTransfer -Source 'https://github.com/FAkeUsership/mr-free/releases/latest/download/platform-tools-windows.zip' -Destination '%BASE%platform-tools.zip' } catch { Invoke-WebRequest -Uri 'https://github.com/FAkeUsership/mr-free/releases/latest/download/platform-tools-windows.zip' -OutFile '%BASE%platform-tools.zip' }"
    if not exist "%BASE%platform-tools.zip" ( echo [FAIL] ADB download failed. & goto :end )
    powershell -NoProfile -Command "Expand-Archive -Path '%BASE%platform-tools.zip' -DestinationPath '%BASE%' -Force"
    if not exist "%BASE%adb\adb.exe" ( echo [FAIL] ADB extraction failed. & goto :end )
    echo [ok] ADB tools ready.
)

rem ============ 3) Choose Android version ============
set "ISO="
for /f "delims=" %%I in ('dir /b /s "%BASE%android\*.iso" 2^>nul') do set "ISO=%%~fI"

if not defined ISO (
    echo [3/4] No Android image found in the android\ folder.
    echo        Download one from the GitHub release page and put it in the android\ folder:
    echo          - CLASSIC: android-x86_64-9.0-r2.iso  (Android 9 - most stable, biggest app library)
    echo          - MODERN : Bliss-OS-12-Android12.1-libndk.iso  (Android 12 - latest apps, ARM64)
    echo        Or download CLASSIC automatically now ^(966 MB, one time^)...
    powershell -NoProfile -Command "$ErrorActionPreference='Stop'; [Net.ServicePointManager]::SecurityProtocol=[Net.SecurityProtocolType]::Tls12; try { Start-BitsTransfer -Source 'https://github.com/FAkeUsership/mr-free/releases/latest/download/android-x86_64-9.0-r2.iso' -Destination '%BASE%android\android-x86_64-9.0-r2.iso' } catch { Invoke-WebRequest -Uri 'https://github.com/FAkeUsership/mr-free/releases/latest/download/android-x86_64-9.0-r2.iso' -OutFile '%BASE%android\android-x86_64-9.0-r2.iso' }"
    for /f "delims=" %%I in ('dir /b /s "%BASE%android\*.iso" 2^>nul') do set "ISO=%%~fI"
    if not defined ISO ( echo [FAIL] Android image download failed. & goto :end )
)

for %%I in ("%ISO%") do set "ISONAME=%%~nI"
echo [3/4] Android image found: %ISONAME%

set "DISK=android9.img"
echo "%ISONAME%" | findstr /I "bliss a12 12.1 android12" >nul 2>&1 && set "DISK=android12.img"
echo "%ISONAME%" | findstr /I "android-x86 9.0 r2" >nul 2>&1 && set "DISK=android9.img"

rem if user already installed the other version, keep their choice
if exist "%BASE%images\android12.img" if not exist "%BASE%images\android9.img" set "DISK=android12.img"
if exist "%BASE%images\android9.img"  if not exist "%BASE%images\android12.img" set "DISK=android9.img"

if exist "%BASE%images\%DISK%" (
    echo [4/4] Android disk %DISK% already installed - keeping it.
) else (
    echo [4/4] Creating 8 GB disk for %DISK% ^(only uses space as it fills^)...
    mkdir "%BASE%images" 2>nul
    "%QIMG%" create -f qcow2 "%BASE%images\%DISK%" 8G
    if errorlevel 1 ( echo [FAIL] Could not create disk image. & goto :end )
)

rem ============ settings.ini ============
set "DISPATH=%BASE%images\%DISK%"

>  "%BASE%settings.ini" echo [Engine]
>> "%BASE%settings.ini" echo qemu=%QSYS%
>> "%BASE%settings.ini" echo qemu_img=%QIMG%
>> "%BASE%settings.ini" echo disk=%DISPATH%
>> "%BASE%settings.ini" echo disk_format=qcow2
>> "%BASE%settings.ini" echo adb=%BASE%adb\adb.exe
>> "%BASE%settings.ini" echo.
>> "%BASE%settings.ini" echo [Emulator]
>> "%BASE%settings.ini" echo ram_mb=4096
>> "%BASE%settings.ini" echo cores=4
>> "%BASE%settings.ini" echo accel=whpx:tcg
>> "%BASE%settings.ini" echo cpu=max
>> "%BASE%settings.ini" echo gpu=1
>> "%BASE%settings.ini" echo audio=dsound
echo [ok] settings.ini written ^(disk = %DISK%^).
echo.
echo   To switch versions later: open AndroidRun.exe - click SWITCH ANDROID.

rem ============ One-time Android install into the disk ============
echo.
echo ================================================================
echo   LAST STEP - install Android into the disk ^(ONE TIME, ~10 min^)
echo ================================================================
echo   A QEMU window will open booting the Android installer.
echo   Do EXACTLY this:
echo.
echo   1. In the menu select:   "Installation - Install Android to harddisk"
echo   2. Partition: choose "Create/Modify partitions"
echo        - "Do you want to use GPT?"  - select  No
echo        - Select  New     - press Enter on the size - select  Primary
echo        - Select  Bootable
echo        - Select  Write   - type  yes  - press Enter
echo        - Select  Quit
echo   3. "Select a partition to install Android":  sda1
echo      Filesystem:  ext4     -  confirm  yes  to format
echo   4. "Install GRUB?"  -  Yes
echo      "Install /system as read-write?"  -  Yes
echo   5. When it finishes, it asks to REBOOT:  just CLOSE the QEMU
echo      window ^(press X^) - do not let it reboot into the installer.
echo.
echo   Then run  AndroidRun.exe, click START, and Android boots from
echo   the disk. Everything after this is 100%% OFFLINE.
echo.
echo   ARM APPS: after Android boots, click "ENABLE ARM (1-time)".
echo ================================================================
echo.
pause
"%QSYS%" -m 4096 -smp 4 -machine accel=whpx:tcg -cpu max -cdrom "%ISO%" -hda "%DISPATH%" -boot d -vga std -usb -device usb-tablet -rtc base=localtime

echo.
echo ================================================================
echo   *** IMPORTANT - DO THIS NOW (one time) ***
echo   Android's installer is done. We now boot the installed system
echo   once so we can give it a "phone line" (ADB over network).
echo   Without this the app can never talk to Android.
echo.
echo   Steps in the Android window that is about to open:
echo     1. Wait for the Android home screen (a few minutes, first boot
echo        is slower - this is normal).
echo     2. Press  Alt + F1   (a black text console appears; in Bliss OS
echo        try Ctrl + Alt + F1 if plain Alt+F1 does nothing).
echo     3. Type EXACTLY this line and press Enter:
echo.
echo          echo service.adb.tcp.port=5555 >> /system/build.prop
echo.
echo     4. Type:   reboot -f     and press Enter.
echo        (Or just close the window and use AndroidRun to start it.)
echo     5. Press Alt + F7 to go back to the graphics screen.
echo.
echo   That's it. After this, everything works forever from the app.
echo ================================================================
echo.
pause
start "" /b "%QSYS%" -m 4096 -smp 4 -machine accel=whpx:tcg -cpu max -hda "%DISPATH%" -boot c -vga std -usb -device usb-tablet -rtc base=localtime -netdev user,id=n1,hostfwd=tcp:127.0.0.1:5555-:5555 -device e1000,netdev=n1

echo [A] Starting adb...
"%BASE%adb\adb.exe" start-server >nul 2>&1
echo [A] Connecting to Android (you did the Alt+F1 step, right?)...
"%BASE%adb\adb.exe" connect 127.0.0.1:5555 >nul 2>&1
echo [A] Waiting for Android to boot ^(up to ~5 min, first boot is slow^)...

:waitboot
"%BASE%adb\adb.exe" connect 127.0.0.1:5555 >nul 2>&1
"%BASE%adb\adb.exe" -s 127.0.0.1:5555 shell getprop sys.boot_completed 2>nul | findstr /r "^1" >nul 2>&1
if errorlevel 1 (
    timeout /t 5 /nobreak >nul
    goto waitboot
)
echo [A] Android is up.

if not exist "%BASE%arm\houdini9_y.sfs" (
    echo [A] ARM engine file not found - you can click ENABLE ARM later in the program.
    goto armdone
)
echo [A] ARM engine found - installing it so 32-bit ARM apps work...
"%BASE%adb\adb.exe" -s 127.0.0.1:5555 shell mkdir -p /sdcard/arm
"%BASE%adb\adb.exe" -s 127.0.0.1:5555 push "%BASE%arm\houdini9_y.sfs" /sdcard/arm/houdini9_y.sfs
if errorlevel 1 ( echo [A] Push failed - you can click ENABLE ARM later. & goto armdone )
"%BASE%adb\adb.exe" -s 127.0.0.1:5555 root >nul 2>&1
"%BASE%adb\adb.exe" connect 127.0.0.1:5555 >nul 2>&1
"%BASE%adb\adb.exe" -s 127.0.0.1:5555 wait-for-device
"%BASE%adb\adb.exe" -s 127.0.0.1:5555 shell su -c "mkdir -p /data/arm && cp /sdcard/arm/houdini9_y.sfs /data/arm/houdini9_y.sfs" >nul 2>&1
"%BASE%adb\adb.exe" -s 127.0.0.1:5555 shell su -c "sh /system/bin/enable_nativebridge" >nul 2>&1
echo [A] ARM support is ON now. 32-bit ARM apps will just work.

:armdone
echo.
echo ================================================================
echo   ALL DONE. From now on:
echo     Run AndroidRun.exe - START - wait for "Android is READY" -
echo     drag ^& drop APKs. ARM apps work, no extra clicks.
echo.
echo   For FULL SPEED, enable "Windows Hypervisor Platform":
echo     Start menu - search "Turn Windows features on or off"
echo     tick "Windows Hypervisor Platform"  -  restart PC
echo   (Also enable VT-x / SVM in BIOS. NVIDIA GPU + gpu=1 in
echo    settings.ini = fast smooth window. No bloat. No errors.)
echo ================================================================
pause

:end
echo.
echo Setup finished. Check the messages above.
pause
