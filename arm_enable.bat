@echo off
setlocal EnableDelayedExpansion
title AndroidRun - Enable ARM app support
mode con cols=100 lines=35
cd /d "%~dp0"

echo ================================================================
echo   AndroidRun - Enable ARM (32-bit) app support
echo   This installs the ARM translation engine into Android so that
echo   apps made for ARM phones can run on your PC.
echo.
echo   IMPORTANT: Start the emulator first, wait until the log says
echo   "Android is READY", THEN run this. (One time only.)
echo ================================================================
echo.

if not exist "arm\houdini9_y.sfs" (
    echo [FAIL] arm\houdini9_y.sfs not found next to this script.
    echo        Re-download the full package from the GitHub release.
    goto :end
)
if not exist "adb\adb.exe" (
    echo [FAIL] adb not found. Run setup.bat first.
    goto :end
)

echo [1] Starting adb...
"adb\adb.exe" start-server >nul 2>&1

echo [2] Waiting for the Android emulator to be ready...
"adb\adb.exe" wait-for-device
if errorlevel 1 ( echo [FAIL] No Android device found. Is the emulator running? & goto :end )

echo [3] Copying ARM engine to Android...
"adb\adb.exe" shell mkdir -p /sdcard/arm
"adb\adb.exe" push "arm\houdini9_y.sfs" /sdcard/arm/houdini9_y.sfs
if errorlevel 1 ( echo [FAIL] Could not copy the ARM engine. & goto :end )

echo [4] Restarting adb as root (Android-x86 includes root)...
"adb\adb.exe" root >nul 2>&1
"adb\adb.exe" wait-for-device

echo [5] Activating ARM translation...
"adb\adb.exe" shell su -c "mkdir -p /data/arm && cp /sdcard/arm/houdini9_y.sfs /data/arm/houdini9_y.sfs && enable_nativebridge"
"adb\adb.exe" shell su -c "sh /system/bin/enable_nativebridge" >nul 2>&1

echo [6] Verifying...
"adb\adb.exe" shell su -c "ls /system/lib/arm/ | head -5"
echo.
echo ================================================================
echo   Done. ARM (32-bit) apps can now be installed.
echo   Note: ARM64-only apps still cannot run - that is impossible
echo   on ANY Windows emulator, including BlueStacks.
echo ================================================================
pause

:end
echo.
pause
