# AndroidRun — Self-Contained Android APK Player for Windows

**Drag & drop any `.apk` onto the window → it installs + launches it in a real Android system running on your PC.**

- ✅ **100% offline** after a one-time setup — engine bundled inside
- ✅ **No ads, no accounts, no tracking** — fully free & open source (GPL-2.0)
- ✅ **ARM app support** (32-bit ARM translation built in)
- ✅ Tiny launcher (~90 KB) + free open-source engine (QEMU + Android-x86 + ADB)

---

## 📥 Download

| File | Size | What it is |
|---|---|---|
| **[AndroidRun-SelfContained-v1.1.zip](https://github.com/FAkeUsership/mr-free/releases/latest/download/AndroidRun-SelfContained-v1.1.zip)** ⭐ | ~1 GB | **THE FULL PACKAGE** — everything inside (recommended) |
| [AndroidRun-SelfContained-v1.1-setup.exe](https://github.com/FAkeUsership/mr-free/releases/latest/download/AndroidRun-SelfContained-v1.1-setup.exe) | ~1 GB | Same content as a self-extracting EXE |
| [android-x86_64-9.0-r2.iso](https://github.com/FAkeUsership/mr-free/releases/latest/download/android-x86_64-9.0-r2.iso) | 966 MB | Android 9 OS image alone |
| [qemu-portable-w64-11.1.0.zip](https://github.com/FAkeUsership/mr-free/releases/latest/download/qemu-portable-w64-11.1.0.zip) | 61 MB | QEMU 11.1 engine alone |
| [platform-tools-windows.zip](https://github.com/FAkeUsership/mr-free/releases/latest/download/platform-tools-windows.zip) | 4 MB | ADB tools alone |

> Why is it ~1 GB? A real Android OS image is ~1 GB. Anyone selling a 10 MB "Android emulator exe" is lying to you (usually malware). This is the honest, working version of that idea — one package, everything inside.

## 🚀 Quick Start (15 minutes, fully offline)

1. Download the **ZIP** → right-click → **Extract All** (built into Windows 10/11)
2. Open the `AndroidRun` folder → run **`setup.bat`** (one time, ~10 min)
   - It finds the bundled engine automatically, creates the Android disk, and opens the Android installer — follow the on-screen steps
3. Run **`AndroidRun.exe`** → click **START EMULATOR** → wait until the log says *"Android is READY"*
4. **Drag & drop any APK** onto the window → it installs and launches

## ✅ Requirements

- Windows 10 / 11 — **64-bit**
- 4 GB RAM recommended (2 GB works)
- ~3 GB free disk space
- Virtualization (VT-x/AMD-V): not required, but enable **"Windows Hypervisor Platform"** in Windows features for full speed

## 🧩 What runs & what doesn't (honest limits)

| App type | Result |
|---|---|
| Normal APKs (x86 / x86_64 / universal) | ✅ Works well |
| 32-bit ARM APKs | ✅ Usually works (built-in ARM translation) |
| **ARM64-only APKs** (no x86 version) | ❌ Usually fails — hard limit of *every* x86 emulator (even BlueStacks) |
| Apps requiring **Android 10+** | ❌ Won't install (Android 9 is the latest stable open-source x86 Android) |

## 🛠 Tech (all free & open source)

| Component | Project | License |
|---|---|---|
| Machine engine | [QEMU](https://www.qemu.org) 11.1 | GPL-2.0 |
| Android OS | [Android-x86](https://www.android-x86.org) 9.0-r2 | Apache-2.0 / GPL |
| APK installer | [platform-tools (adb)](https://developer.android.com) | Apache-2.0 |
| Launcher (this repo) | `src/AndroidRun.c` | GPL-2.0 |

## 🔨 Build the launcher from source

```bash
# needs mingw-w64 cross compiler (Linux) 
cd src && ./build.sh   # or on Windows: gcc -mwindows AndroidRun.c app.rc -lshell32 -lcomdlg32 -lgdi32 -luser32 -lole32
```
