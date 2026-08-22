# AndroidRun — Self-Contained Android APK Player for Windows

**Drag & drop any `.apk` onto the window → it installs + launches it in a real Android system running on your PC.** 100% offline, no ads, no tracking, free & open source (GPL-2.0).

## Two Androids — one program (switch anytime)
| | **CLASSIC** (Android 9) | **MODERN** (Android 12.1) |
|---|---|---|
| File | android-x86_64-9.0-r2.iso | Bliss-OS-12-Android12.1-libndk.iso |
| x86 / x86_64 APKs | ✅ | ✅ |
| 32-bit ARM APKs | ✅ (ENABLE ARM once) | ✅ built-in |
| ARM64-only APKs | ❌ | ⚠️ partial (libndk) |
| Apps needing Android 10+ | ❌ | ✅ |
| Stability | HIGH | MEDIUM (alpha) |

## 📥 Download — latest release: [v1.3](https://github.com/FAkeUsership/mr-free/releases/latest)
| File | Size | What |
|---|---|---|
| ⭐ **AndroidRun-SelfContained-v1.2.zip** | 1.06 GB | FULL package: launcher + QEMU + Android 9 (Classic) + ARM engine + ADB |
| **Bliss-OS-12-Android12.1-libndk.iso** | 1.23 GB | MODERN Android 12.1 (add to `android\` folder) |
| updated-setup-modern.bat | 10 KB | setup with Modern/Classic choice |
| AndroidRun.exe-switch | 95 KB | launcher with SWITCH ANDROID button |
| README-modern.txt | 5 KB | full dual-version guide |

## 🚀 Quick Start
1. Extract the full v1.2 zip → open `AndroidRun` folder
2. (optional, for latest apps) put the Bliss 12 ISO in `android\`
3. Run **setup.bat** → choose Modern or Classic → follow on-screen install (~10 min)
4. Run **AndroidRun.exe** → START EMULATOR → wait for "Android is READY" → drag & drop APKs

## ✅ Requirements
Windows 10/11 64-bit · 4 GB RAM · ~3 GB disk · (recommended) enable "Windows Hypervisor Platform"

## 🛠 Tech
QEMU 11.1 (GPL) · Android-x86 9 (Apache) · Bliss OS 12 (Apache) · adb (Apache) · houdini + libndk ARM translation

## 🔨 Build
```bash
cd src && ./build.sh   # mingw-w64
```
