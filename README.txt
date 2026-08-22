================================================================
 AndroidRun v1.4 - Offline Android APK Player for Windows 10/11
================================================================

DRAG & DROP any .apk onto the window -> it installs AND launches
it inside a real Android running on your PC. Offline. No ads.

----------------------------------------------------------------
FIRST TIME (one setup, ~10 minutes)
----------------------------------------------------------------
 1. Extract the full ZIP somewhere (e.g. C:\AndroidRun)
 2. Double-click setup.bat
 3. Follow the on-screen Android installer steps
 4. Do the ONE MANUAL STEP when setup tells you:

      At the Android home screen press Alt+F1, type:

          echo service.adb.tcp.port=5555 >> /system/build.prop

      then type:  reboot -f   and press Enter.

    (Bliss OS: try Ctrl+Alt+F1 instead. Alt+F7 = back to graphics.)
    This gives the app its "phone line" to Android. One time only.

 5. Open AndroidRun.exe -> START -> wait for "Android is READY"
    -> drag & drop APKs. Works offline forever after this.

----------------------------------------------------------------
SPEED (hardware acceleration)
----------------------------------------------------------------
The start log shows whether WHPX hardware acceleration is ON.
If it says OFF, do this once:
  Start menu -> "Turn Windows features on or off"
  -> tick "Windows Hypervisor Platform" -> restart PC.
Also make sure VT-x (Intel) / SVM (AMD) is enabled in BIOS.
NVIDIA GPU owners: leave gpu=1 in settings.ini (default) - the
window renders with OpenGL on your GPU.

----------------------------------------------------------------
BUTTONS
----------------------------------------------------------------
 START EMULATOR - boots Android (uses settings.ini)
 STOP           - hard stop + clean adb disconnect
 INSTALL APK... - file picker alternative to drag & drop
 SETUP (1-time) - runs setup.bat
 SETTINGS       - opens settings.ini in notepad
 ENABLE ARM     - one-time 32-bit ARM app support (Classic only)
 SWITCH ANDROID - toggle Android 9 <-> Android 12 (both installed)

----------------------------------------------------------------
ADVANCED: settings.ini
----------------------------------------------------------------
 ram_mb=4096   memory
 cores=4       cpu cores
 accel=whpx:tcg  hardware accel with software fallback (keep this)
 cpu=max       fastest cpu model
 gpu=1         OpenGL window (0 if window looks broken)
 audio=dsound  ("none" if QEMU errors on audio)

----------------------------------------------------------------
PROBLEMS?
----------------------------------------------------------------
 - "READY" never shows on very first boot -> do the Alt+F1 step.
 - Install error NO_MATCHING_ABIS -> ARM64-only app; use the
   MODERN (Android 12) build, or enable ARM for 32-bit ARM apps.
 - Slow -> WHPX is off; the log tells you exactly how to enable it.
 - Everything else -> read the Log panel; it explains every error.

GPL-2.0-or-later. Source: ./src (build.sh builds on Linux w/ mingw).
================================================================
