AndroidRun - Self-Contained Android APK Player for Windows 10/11 64-bit
========================================================================

WHAT THIS IS
------------
A standalone, minimal program: drag & drop an .apk file onto the
window and it installs + launches it in a real Android system running
on your PC. No ads, no account, no tracking. Fully self-contained,
works 100% OFFLINE after one-time setup.

TWO ANDROID VERSIONS (choose what fits your apps)
-------------------------------------------------
1) CLASSIC - Android 9 (android-x86 9.0-r2)
   - MOST STABLE, biggest app library, lowest requirements
   - Runs: x86/x86_64/universal APKs + 32-bit ARM APKs (after
     one-click ENABLE ARM)
   - File: android\android-x86_64-9.0-r2.iso (included in the zip)

2) MODERN - Android 12.1 (Bliss OS 12, libndk)
   - LATEST apps & games, ARM64 app support (libndk translation)
   - Newer apps that refuse Android 9 will install here
   - Alpha-level: can be less stable than Classic
   - File: download "Bliss-OS-12-Android12.1-libndk.iso" from the
     GitHub release and put it in the android\ folder

How to switch: in AndroidRun.exe click "SWITCH ANDROID 9/12"
(the version must be installed first via setup.bat).

APP SUPPORT (honest, by version)
--------------------------------
                     CLASSIC (A9)     MODERN (A12.1)
x86 / x86_64 APKs      YES              YES
32-bit ARM APKs        YES (ENABLE ARM) YES (built-in)
ARM64-only APKs        NO               PARTIAL (libndk - works
                                         for many, not all)
Apps needing Android 10+ NO             YES
Apps needing Android 12+ NO             YES (12.1)
Stability              HIGH             MEDIUM (alpha)

- ARM64-only APKs that even libndk cannot translate: cannot run on
  ANY Windows emulator (BlueStacks included). Hard technical limit.
- Some apps block "PC/emulator" devices by design; not fixable.

WHY IT IS ~1 GB
---------------
A real Android OS image is ~1 GB, plus the QEMU engine (~170 MB) and
the ARM engine (43 MB). Anyone selling a 10 MB "Android emulator exe"
is lying to you (usually malware). This is the honest, working
version of that idea: one package with everything inside.

FILES
-----
AndroidRun.exe          the program (drag & drop APK installs)
setup.bat               one-time setup (~10 min, OFFLINE) - asks
                        which Android version to use
arm_enable.bat          one-click ARM support (Classic only)
arm\houdini9_y.sfs      ARM 32-bit translation engine (Classic)
settings.ini            RAM / cores / paths / active disk
README.txt              this file
LICENSE.txt             licenses (GPL-2.0 + Apache)
adb\                    ADB installer tool (bundled)
engine\qemu\            QEMU engine (bundled)
android\                Android system images (bundled + optional)

FIRST TIME (~15 minutes, fully offline)
---------------------------------------
1. Run  setup.bat
   - it finds the bundled engine automatically, asks which Android
   - then a QEMU window opens with the Android installer
   - follow the on-screen instructions (install to disk, ~10 min)
2. Run  AndroidRun.exe  ->  click  START EMULATOR
   - first boot takes a few minutes; the log says when Android is ready
3. (Classic only, for ARM apps) click  ENABLE ARM (1-time)
4. Drag & drop an .apk onto the window -> installed + launched.

INSTALLING THE MODERN (ANDROID 12) OPTION
-----------------------------------------
1. Download  Bliss-OS-12-Android12.1-libndk.iso  from the GitHub
   release page
2. Put it in the android\ folder (next to the Android 9 iso)
3. Run setup.bat - it detects it and asks which version to install
4. Or if Android 9 is already installed: click SWITCH ANDROID in the
   program, stop the emulator, run setup.bat to install Android 12.

EVERYDAY USE
------------
1. Run AndroidRun.exe
2. Click START EMULATOR, wait for "Android is READY"
3. Drag & drop APK files anytime. 100% offline, forever.

REQUIREMENTS
------------
- Windows 10 / 11 - 64-bit
- 4 GB RAM recommended (2 GB works)
- ~3 GB free disk space (Android 12 image uses more)
- Virtualization (VT-x/AMD-V) NOT required, but strongly helps speed.
  Enable "Windows Hypervisor Platform" in Windows features for full
  speed. Without it, it still runs, just slower.

TROUBLESHOOTING
---------------
- "Windows protected your PC" -> More info -> Run anyway (unsigned)
- Slow -> enable "Windows Hypervisor Platform" + reboot
- INSTALL_FAILED_NO_MATCHING_ABIS:
     * on CLASSIC -> it may be ARM64-only; try MODERN (Android 12)
     * 32-bit ARM app on CLASSIC -> did you run ENABLE ARM first?
- App crashes on MODERN -> it is alpha; try CLASSIC instead
- Small screen -> F11 for fullscreen inside the Android window
- RAM/cores -> edit settings.ini (open via SETTINGS button)

BUILD FROM SOURCE
-----------------
The launcher is open source (GPL-2.0). See src\ and build.sh.
QEMU: https://www.qemu.org
Android-x86: https://www.android-x86.org
Bliss OS: https://blissos.org  (Android 12.1 libndk build archived at
https://archive.org/details/android_x86_64-a12.1_r1-03.16.22-01-mesa22-ksu-gapps-libndk-sd)

Downloads / updates: https://github.com/FAkeUsership/mr-free/releases
