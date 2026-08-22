# AndroidRun — Self-Contained Android APK Player for Windows

**Drag & drop any `.apk` onto the window → it installs + launches it in a real Android system running on your PC.**

- ✅ **100% offline** after a one-time setup — engine bundled inside
- ✅ **ARM app support** — one-click ARM (32-bit) translation engine included
- ✅ **No ads, no accounts, no tracking** — fully free & open source (GPL-2.0)
- ✅ Tiny launcher (~95 KB) + free open-source engine (QEMU + Android-x86 + ADB)

---

## 📥 Download (latest: v1.2 — ARM App Support)

| File | Size | What it is |
|---|---|---|
| **[AndroidRun-SelfContained-v1.2.zip](https://github.com/FAkeUsership/mr-free/releases/latest/download/AndroidRun-SelfContained-v1.2.zip)** ⭐ | ~1.06 GB | **THE FULL PACKAGE** — everything inside, incl. ARM engine |
| [houdini9_y.sfs](https://github.com/FAkeUsership/mr-free/releases/latest/download/houdini9_y.sfs) | 43 MB | The ARM translation engine alone (re-add it if deleted) |
| [android-x86_64-9.0-r2.iso](https://github.com/FAkeUsership/mr-free/releases/latest/download/android-x86_64-9.0-r2.iso) | 966 MB | Android 9 OS image alone |
| [qemu-portable-w64-11.1.0.zip](https://github.com/FAkeUsership/mr-free/releases/latest/download/qemu-portable-w64-11.1.0.zip) | 61 MB | QEMU 11.1 engine alone |
| [platform-tools-windows.zip](https://github.com/FAkeUsership/mr-free/releases/latest/download/platform-tools-windows.zip) | 4 MB | ADB tools alone |

> Why ~1 GB? A real Android OS image is ~1 GB. Anyone selling a 10 MB "Android emulator exe" is lying to you (usually malware).

## 🚀 Quick Start (15 minutes, fully offline)

1. Download the **ZIP** → right-click → **Extract All** (built into Windows 10/11)
2. Open the `AndroidRun` folder → run **`setup.bat`** (one time, ~10 min, follow on-screen steps)
3. Run **`AndroidRun.exe`** → **START EMULATOR** → wait for *"Android is READY"*
4. **For ARM apps:** click **"ENABLE ARM (1-time)"** once → then drag & drop APKs

## 🧩 Compatibility (honest)

| App type | Result |
|---|---|
| x86 / x86_64 / universal APKs | ✅ Works well (native) |
| **32-bit ARM APKs** (armeabi-v7a — most older apps, many newer games) | ✅ Works after one-click **ENABLE ARM** |
| **ARM64-only APKs** (only a 64-bit ARM version exists) | ❌ Impossible on ANY Windows emulator — even BlueStacks |
| Apps requiring **Android 10+** | ❌ Won't install (Android 9 = latest stable open-source x86 Android) |

## ✅ Requirements
- Windows 10 / 11 — **64-bit** · 4 GB RAM (2 GB works) · ~3 GB disk
- Virtualization (VT-x/AMD-V): not required; enable **"Windows Hypervisor Platform"** in Windows features for full speed

## 🛠 Tech (all free & open source)
QEMU 11.1 (GPL-2.0) · Android-x86 9.0-r2 (Apache/GPL) · platform-tools adb (Apache-2.0) · ARM engine = official android-x86 houdini

## 🔨 Build the launcher
```bash
cd src && ./build.sh   # needs mingw-w64 cross compiler
```
