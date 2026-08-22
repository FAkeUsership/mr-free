# AndroidRun — Self-Contained Android APK Player for Windows

**Drag & drop any `.apk` onto the window → it installs + launches it in a real Android system running on your PC.**
100% offline after setup, no ads, no tracking, no account. Free & open source (GPL-2.0).

> **v1.4 — the "it actually works now" release.** Earlier builds had three fatal bugs (invalid QEMU
> accelerator syntax so the VM never started, no ADB networking bridge so APKs could never install,
> and a log window that printed everything on one line). All fixed, plus a real hardware-acceleration
> system and the SWITCH ANDROID button that v1.3's README promised but didn't ship.

## Two Androids — one program (switch anytime)

|  | **CLASSIC** (Android 9) | **MODERN** (Android 12.1) |
| --- | --- | --- |
| File | android-x86_64-9.0-r2.iso | Bliss-OS-12-Android12.1-libndk.iso |
| x86 / x86_64 APKs | ✅ | ✅ |
| 32-bit ARM APKs | ✅ (ENABLE ARM once) | ✅ built-in |
| ARM64-only APKs | ❌ | ⚠️ partial (libndk) |
| Apps needing Android 10+ | ❌ | ✅ |
| Stability | HIGH | MEDIUM (alpha) |

## 📥 Download — latest release: [v1.4](https://github.com/FAkeUsership/mr-free/releases/latest)

| File | Size | What |
| --- | --- | --- |
| ⭐ **AndroidRun-SelfContained-v1.2.zip** | 1.06 GB | FULL package: launcher + QEMU + Android 9 (Classic) + ARM engine + ADB |
| **Bliss-OS-12-Android12.1-libndk.iso** | 1.23 GB | MODERN Android 12.1 (add to `android\` folder) |
| updated-setup-modern.bat | 10 KB | setup with Modern/Classic choice |
| AndroidRun.exe-switch | 95 KB | launcher with SWITCH ANDROID button |
| README-modern.txt | 5 KB | full dual-version guide |

## 🚀 Quick Start

1. Extract the full ZIP anywhere (e.g. `C:\AndroidRun`).
2. Double-click **setup.bat** — one time, ~10 minutes. It:
   - checks the bundled QEMU / ADB / Android image
   - creates the Android disk (`images\android9.img` or `android12.img`)
   - opens the Android **installer** in QEMU — follow its on-screen steps
   - then boots Android once and asks you to do the **one Alt+F1 step** (below)
3. Open **AndroidRun.exe** → **START** → drag & drop APKs. Done.

### 🔑 The one manual step that makes ADB work (one time, 30 seconds)

Raw QEMU VMs have no built-in adb channel, so Android must be told once to listen
on the network port the app forwards. setup.bat walks you through this; the short version:

1. At the Android home screen, press **Alt + F1** (Bliss OS: try **Ctrl + Alt + F1**).
2. Type exactly:
   ```
   echo service.adb.tcp.port=5555 >> /system/build.prop
   ```
3. Type `reboot -f` and press Enter. (Alt+F7 = back to graphics.)

After that, **START → "Android is READY" → drop APK** works every time.

## ⚡ Performance (the part proprietary emulators brag about — free here)

- **Hardware virtualization:** the launcher auto-probes **WHPX** (Windows Hypervisor
  Platform) at every start and tells you in the log whether it's ON:
  - **ON** → near-native speed. This is the same tech BlueStacks/LDPlayer use.
  - **OFF** → log prints exact fix: *Start → "Turn Windows features on or off" →
    tick "Windows Hypervisor Platform" → restart*, plus enable **VT-x/SVM** in BIOS.
    On AMD you also need Windows 10 2004 or newer.
- **NVIDIA / any GPU:** `gpu=1` in `settings.ini` (default) renders the emulator
  window through your GPU via OpenGL. Honest note: on Windows hosts the Android
  guest does 3D with optimized CPU code (SwiftShader/ANGLE) — no Windows-host
  emulator, paid or free, passes a real GPU into Android-x86. CPU speed (WHPX) is
  what matters, and that's enabled.
- Defaults: **4 GB RAM, 4 cores, cpu=max**. Tune in `settings.ini` (`ram_mb`, `cores`).

## 🎛 settings.ini

| key | default | meaning |
| --- | --- | --- |
| `ram_mb` | 4096 | VM memory (512–32768) |
| `cores` | 4 | CPU cores (1–32) |
| `accel` | whpx:tcg | accelerator + fallback (`tcg` = software only) |
| `cpu` | max | QEMU CPU model |
| `gpu` | 1 | 1 = OpenGL window on your GPU · 0 = safe mode |
| `audio` | dsound | set to `none` if QEMU complains about audio |

## ❓ Will it work on my PC? (checklist)

- ✅ Windows 10 (1803+) or 11, 64-bit
- ✅ Virtualization ON in BIOS (VT-x / AMD-V)
- ✅ "Windows Hypervisor Platform" feature ticked (log confirms after START)
- ✅ ~12 GB free disk space
- ❌ Not supported: ARM Windows devices, 32-bit Windows, Windows 7/8

## 🧩 Troubleshooting

| Symptom | Fix |
| --- | --- |
| Emulator exits instantly, log shows WhpxAvailable probe but QEMU window dies | Old builds had this — you are on v1.4 now. If it persists, run `engine\qemu\qemu-system-x86_64.exe -machine accel=whpx:tcg -display none -S` manually and read the error. |
| "Android is READY" never appears on FIRST boot | Do the Alt+F1 step above (one time). |
| QEMU audio error | `audio=none` in settings.ini |
| Broken/black emulator window | `gpu=0` in settings.ini |
| INSTALL_FAILED_NO_MATCHING_ABIS | ARM32 app → click ENABLE ARM once (Classic). ARM64-only → switch to MODERN build. |
| Everything slow | Enable WHPX (log tells you how). Software mode is 5–10× slower. |

## 🛠 Build from source (Linux cross-compile)

```bash
./build.sh   # needs x86_64-w64-mingw32 (gcc or llvm-mingw) + python3
```

## License

- This launcher & scripts: **GPL-2.0-or-later**
- QEMU: GPL-2.0 · Android-x86: Apache-2.0 · ADB: Apache-2.0
- ARM translation (houdini) is bundled for compatibility; ARM64-only apps are
  impossible on any Windows PC emulator — physics, not laziness.
