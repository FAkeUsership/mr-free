/*
 * AndroidRun - Offline Android APK player (launcher) for Windows 10/11 x64
 * Drag & drop an .apk -> installs it into the running Android VM -> auto-launches it.
 *
 * Free & open-source stack underneath:
 *   - QEMU (GPL)          : the machine emulator/virtualizer
 *   - Android-x86 (APL)   : the Android OS image (x86_64 with ARM32 translation)
 *   - platform-tools adb  : APK installer transport
 *
 * This file: GPL-2.0-or-later
 *
 * v1.4 - real fix release:
 *   - QEMU accelerator syntax FIXED ("-machine accel=whpx:tcg"; "-accel whpx:tcg"
 *     is invalid and QEMU would exit instantly - the emulator could never start).
 *   - ADB actually works now: user-mode networking forwards host 127.0.0.1:5555
 *     to the guest and the launcher runs "adb connect". Previously the device
 *     could never be found, so installing APKs was impossible.
 *   - Hardware acceleration is PROBED at start (hidden 2s QEMU test launch) and
 *     the log tells you exactly how to enable "Windows Hypervisor Platform".
 *   - GPU option (uses your NVIDIA/AMD/Intel GPU to render the window via OpenGL).
 *   - Audio device added (dsound; set audio=none in settings.ini if it complains).
 *   - SWITCH ANDROID button implemented (android9.img <-> android12.img).
 *   - Log window newline handling fixed (everything used to print on one line).
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define IDC_DROP        1001
#define IDC_BTN_START   1002
#define IDC_BTN_STOP    1003
#define IDC_BTN_APK     1004
#define IDC_BTN_SETUP   1005
#define IDC_BTN_INI     1006
#define IDC_BTN_FOLDER  1007
#define IDC_LOG         1008
#define IDC_STATUS      1009
#define IDC_BTN_ARM     1010
#define IDC_BTN_SWITCH  1011
#define IDC_NOTE        1012

#define WM_APP_LOG      (WM_APP + 1)
#define WM_APP_DONE     (WM_APP + 2)

#define ADB_HOST_PORT   "127.0.0.1:5555"

static HINSTANCE g_hInst;
static HWND g_hwnd, g_hLog, g_hStatus, g_hDrop, g_hBtnStart, g_hBtnStop;
static HBRUSH g_hDropBrush;

static char g_base[MAX_PATH];   /* folder that contains AndroidRun.exe */
static char g_qemu[MAX_PATH];   /* qemu-system-x86_64.exe */
static char g_qemuImg[MAX_PATH];/* qemu-img.exe  (same folder as qemu) */
static char g_disk[MAX_PATH];   /* android disk image */
static char g_adb[MAX_PATH];    /* adb.exe */
static char g_diskFmt[16];      /* qcow2 | raw */
static char g_accel[64];        /* whpx:tcg (default) */
static char g_cpu[64];          /* max (default) */
static char g_audio[32];        /* dsound | none */
static int  g_gpu    = 1;       /* 1 = render window with host GPU (OpenGL) */
static int  g_ramMB  = 4096;
static int  g_cores  = 4;

static HANDLE   g_qemuProc = NULL;
static volatile LONG g_busy = 0;
static int      g_keepRunning = 0; /* close window but keep emulator */

/* ---------------- tiny helpers ---------------- */

static char *mystrdup(const char *s){
    size_t n = strlen(s) + 1;
    char *p = (char*)malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

/* if rel looks absolute (X:\ or \\), use it as-is; else join base\rel */
static void PathJoin(char *dst, size_t n, const char *base, const char *rel){
    if (rel[0] && (rel[1] == ':' || (rel[0]=='\\' && rel[1]=='\\'))){
        snprintf(dst, n, "%s", rel);
        return;
    }
    if (base && base[0]) snprintf(dst, n, "%s\\%s", base, rel);
    else                 snprintf(dst, n, "%s", rel);
}

static void ExeDir(char *out, size_t n){
    GetModuleFileNameA(NULL, out, (DWORD)n);
    char *slash = strrchr(out, '\\');
    if (slash) *slash = 0;
}

static int ProcAlive(HANDLE h){
    if (!h) return 0;
    DWORD c = 0;
    if (GetExitCodeProcess(h, &c)) return (c == STILL_ACTIVE);
    return 0;
}

/* thread-safe logging */
static void LogF(const char *fmt, ...){
    char buf[3072];
    va_list ap; va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    PostMessage(g_hwnd, WM_APP_LOG, 0, (LPARAM)mystrdup(buf));
}

/* ---------------- settings ---------------- */

static void LoadSettings(void){
    char ini[MAX_PATH], def[MAX_PATH], buf[1024]; int v;
    PathJoin(ini, sizeof(ini), g_base, "settings.ini");

    PathJoin(def, sizeof(def), g_base, "engine\\qemu\\qemu-system-x86_64.exe");
    GetPrivateProfileStringA("Engine", "qemu", def, buf, sizeof(buf), ini);
    PathJoin(g_qemu, sizeof(g_qemu), g_base, buf);

    PathJoin(def, sizeof(def), g_base, "engine\\qemu\\qemu-img.exe");
    GetPrivateProfileStringA("Engine", "qemu_img", def, buf, sizeof(buf), ini);
    PathJoin(g_qemuImg, sizeof(g_qemuImg), g_base, buf);

    PathJoin(def, sizeof(def), g_base, "images\\android9.img");
    GetPrivateProfileStringA("Engine", "disk", def, buf, sizeof(buf), ini);
    PathJoin(g_disk, sizeof(g_disk), g_base, buf);

    PathJoin(def, sizeof(def), g_base, "adb\\adb.exe");
    GetPrivateProfileStringA("Engine", "adb", def, buf, sizeof(buf), ini);
    PathJoin(g_adb, sizeof(g_adb), g_base, buf);

    GetPrivateProfileStringA("Engine", "disk_format", "qcow2", g_diskFmt, sizeof(g_diskFmt), ini);

    GetPrivateProfileStringA("Emulator", "accel", "whpx:tcg", g_accel, sizeof(g_accel), ini);
    GetPrivateProfileStringA("Emulator", "cpu",   "max",      g_cpu,   sizeof(g_cpu),   ini);
    GetPrivateProfileStringA("Emulator", "audio", "dsound",   g_audio, sizeof(g_audio), ini);

    v = GetPrivateProfileIntA("Emulator", "ram_mb", 4096, ini);
    g_ramMB = (v >= 512 && v <= 32768) ? v : 4096;
    v = GetPrivateProfileIntA("Emulator", "cores", 4, ini);
    g_cores = (v >= 1 && v <= 32) ? v : 4;
    v = GetPrivateProfileIntA("Emulator", "gpu", 1, ini);
    g_gpu = (v == 0) ? 0 : 1;
}

/* ---------------- run a program, capture stdout+stderr to a file ---------------- */

static int RunCapture(const char *exe, const char *args, const char *outfile, DWORD timeoutMs){
    char cmd[4200];
    snprintf(cmd, sizeof(cmd), "\"%s\" %s", exe, args ? args : "");

    SECURITY_ATTRIBUTES sa; sa.nLength = sizeof(sa); sa.bInheritHandle = TRUE; sa.lpSecurityDescriptor = NULL;
    HANDLE hOut = CreateFileA(outfile, GENERIC_WRITE, FILE_SHARE_READ, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    STARTUPINFOA si; memset(&si, 0, sizeof(si)); si.cb = sizeof(si);
    PROCESS_INFORMATION pi; memset(&pi, 0, sizeof(pi));
    if (hOut != INVALID_HANDLE_VALUE){
        si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
        si.hStdOutput = hOut; si.hStdError = hOut;
        si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
        si.wShowWindow = SW_HIDE;
    }
    BOOL ok = CreateProcessA(NULL, cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    if (hOut != INVALID_HANDLE_VALUE) CloseHandle(hOut);
    if (!ok){ LogF("[err] Failed to run: %s (error %lu)", exe, (unsigned long)GetLastError()); return -1; }
    WaitForSingleObject(pi.hProcess, timeoutMs);
    DWORD code = 0; GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
    return (int)code;
}

static void LogFile(const char *path){
    HANDLE h = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;
    DWORD sz = GetFileSize(h, NULL);
    if (sz > 200000) sz = 200000;
    char *raw = (char*)malloc(sz + 1);
    if (raw){
        DWORD rd = 0;
        ReadFile(h, raw, sz, &rd, NULL);
        raw[rd] = 0;
        /* normalize CRLF/CR -> LF so the log window shows real lines */
        char *cln = (char*)malloc(rd + 1);
        if (cln){
            DWORD i, j = 0;
            for (i = 0; i < rd; i++){
                if (raw[i] == '\r'){
                    if (i + 1 < rd && raw[i+1] == '\n') i++; /* eat CRLF */
                    cln[j++] = '\n';
                } else cln[j++] = raw[i];
            }
            cln[j] = 0;
            while (j && cln[j-1] == '\n') cln[--j] = 0; /* trim trailing newlines */
            if (j) LogF("%s", cln);
            free(cln);
        }
        free(raw);
    }
    CloseHandle(h);
}

/* ---------------- adb helpers ---------------- */

/* Connect (harmless if already connected), then return first live serial. */
static int GetDeviceSerial(char *out, size_t n){
    char tmp[MAX_PATH];
    PathJoin(tmp, sizeof(tmp), g_base, "adb_tmp.txt");
    RunCapture(g_adb, "connect " ADB_HOST_PORT, tmp, 10000);
    RunCapture(g_adb, "devices", tmp, 15000);
    FILE *f = fopen(tmp, "rb");
    if (!f) return 0;
    char line[512]; int found = 0; out[0] = 0;
    while (fgets(line, sizeof(line), f)){
        char a[160], b[160];
        if (sscanf(line, "%159s %159s", a, b) == 2 && strcmp(b, "device") == 0 &&
            (strncmp(a, "127.0.0.1:", 10) == 0 || strncmp(a, "localhost:", 10) == 0 ||
             strncmp(a, "emulator-", 9) == 0)){
            snprintf(out, n, "%s", a); found = 1; break;
        }
    }
    fclose(f);
    return found;
}

/* ---------------- install worker ---------------- */

static DWORD WINAPI InstallThread(LPVOID lp){
    char *apk = (char*)lp;
    char serial[128];

    if (InterlockedExchange(&g_busy, 1)){
        LogF("[!] Another task is running - wait for it to finish.");
        free(apk); return 0;
    }
    if (!ProcAlive(g_qemuProc)){
        LogF("[!] Emulator is not running. Click START EMULATOR first.");
        InterlockedExchange(&g_busy, 0); free(apk); return 0;
    }

    /* wait until adb sees the device (Android booted) */
    if (!GetDeviceSerial(serial, sizeof(serial))){
        LogF("[>] Android still booting, waiting (up to 5 minutes)...");
        int ok = 0;
        for (int i = 0; i < 100; i++){
            if (!ProcAlive(g_qemuProc)){ LogF("[!] Emulator exited during boot."); InterlockedExchange(&g_busy, 0); free(apk); return 0; }
            if (GetDeviceSerial(serial, sizeof(serial))){ ok = 1; break; }
            if ((i % 10) == 9) LogF("[>] ...still waiting for Android...");
            Sleep(3000);
        }
        if (!ok){
            LogF("[!] Android did not become ready.");
            LogF("[!] If this is the FIRST boot ever: open the Android window, press Alt+F1 and run:");
            LogF("     echo service.adb.tcp.port=5555 >> /system/build.prop");
            LogF("     reboot -f");
            LogF("    (Then Alt+F7 back to graphics. One time only - setup.bat explains this too.)");
            InterlockedExchange(&g_busy, 0); free(apk); return 0;
        }
    }

    /* snapshot of installed packages BEFORE */
    char bf[MAX_PATH], af[MAX_PATH], tmp[MAX_PATH];
    PathJoin(bf,  sizeof(bf),  g_base, "pkgs_before.txt");
    PathJoin(af,  sizeof(af),  g_base, "pkgs_after.txt");
    PathJoin(tmp, sizeof(tmp), g_base, "adb_install.txt");

    char args[4200];
    snprintf(args, sizeof(args), "-s %s shell pm list packages", serial);
    RunCapture(g_adb, args, bf, 60000);

    LogF("[>] Installing: %s", apk);
    snprintf(args, sizeof(args), "-s %s install -r \"%s\"", serial, apk);
    int code = RunCapture(g_adb, args, tmp, 600000); /* up to 10 min */
    LogFile(tmp);

    if (code == 0){
        LogF("[ok] APK installed successfully!");

        /* find the newly added package and try to launch it */
        snprintf(args, sizeof(args), "-s %s shell pm list packages", serial);
        RunCapture(g_adb, args, af, 60000);
        char newpkg[512]; newpkg[0] = 0;
        {
            FILE *fa = fopen(af, "rb");
            if (fa){
                char line[1024];
                while (fgets(line, sizeof(line), fa)){
                    char pkg[512];
                    if (sscanf(line, "package:%511s", pkg) == 1){
                        int present = 0;
                        FILE *fb = fopen(bf, "rb");
                        if (fb){
                            char l2[1024];
                            while (fgets(l2, sizeof(l2), fb)){
                                char p2[512];
                                if (sscanf(l2, "package:%511s", p2) == 1 && strcmp(p2, pkg) == 0){ present = 1; break; }
                            }
                            fclose(fb);
                        }
                        if (!present){ snprintf(newpkg, sizeof(newpkg), "%s", pkg); break; }
                    }
                }
                fclose(fa);
            }
        }
        /* strip any trailing whitespace/CR from package name */
        { size_t L = strlen(newpkg); while (L && (newpkg[L-1]=='\r'||newpkg[L-1]=='\n'||newpkg[L-1]==' ')) newpkg[--L]=0; }

        if (newpkg[0]){
            LogF("[ok] Launching %s ...", newpkg);
            char m[4200];
            snprintf(m, sizeof(m), "-s %s shell monkey -p %s -c android.intent.category.LAUNCHER 1", serial, newpkg);
            char mtmp[MAX_PATH];
            PathJoin(mtmp, sizeof(mtmp), g_base, "adb_monkey.txt");
            int mc = RunCapture(g_adb, m, mtmp, 60000);
            if (mc != 0){
                LogF("[ok] Auto-launch failed - open the app in the Android app drawer instead.");
            }
        } else {
            LogF("[ok] Open the app in the Android app drawer to run it.");
        }
    } else {
        LogF("[!] Install failed (adb code %d).", code);
        LogF("[!] INSTALL_FAILED_NO_MATCHING_ABIS  = app is ARM (32-bit) but ARM support is off -> click ENABLE ARM once. ARM64-only apps cannot run on Classic.");
        LogF("[!] INSTALL_FAILED_UPDATE_INCOMPATIBLE = an older version is installed - uninstall it in Android first.");
        LogF("[!] INSTALL_FAILED_INSUFFICIENT_STORAGE = enlarge the disk image / free space in Android.");
    }

    InterlockedExchange(&g_busy, 0);
    free(apk);
    PostMessage(g_hwnd, WM_APP_DONE, 0, 0);
    return 0;
}

/* ---------------- emulator control ---------------- */

/* Probe whether WHPX (Windows Hypervisor Platform) actually works:
 * launch QEMU hidden for 2 seconds with accel=whpx. If it stays alive,
 * WHPX initialized fine. If it exits, WHPX is disabled/unavailable. */
static int WhpxAvailable(void){
    char cmd[MAX_PATH + 96];
    snprintf(cmd, sizeof(cmd), "\"%s\" -machine accel=whpx -display none -S -nodefaults", g_qemu);

    STARTUPINFOA si; memset(&si, 0, sizeof(si)); si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW; si.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION pi; memset(&pi, 0, sizeof(pi));

    if (!CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
        return -1;
    DWORD r = WaitForSingleObject(pi.hProcess, 2000);
    int ok = 0;
    if (r == WAIT_TIMEOUT){ ok = 1; TerminateProcess(pi.hProcess, 0); WaitForSingleObject(pi.hProcess, 1000); }
    CloseHandle(pi.hThread); CloseHandle(pi.hProcess);
    return ok;
}

static DWORD WINAPI BootWaitThread(LPVOID lp){
    (void)lp;
    char serial[128];
    int i;
    for (i = 0; i < 100; i++){ /* 5 minutes */
        if (!ProcAlive(g_qemuProc)){
            LogF("[!] Emulator exited before Android finished booting.");
            break;
        }
        if (GetDeviceSerial(serial, sizeof(serial))){
            LogF("[ok] Android is READY (%s).  Drag & drop an APK here to install it.", serial);
            break;
        }
        if ((i % 10) == 9) LogF("[>] ...still booting (%d s)...", (i + 1) * 3);
        Sleep(3000);
    }
    if (!GetDeviceSerial(serial, sizeof(serial)) && ProcAlive(g_qemuProc)){
        LogF("[>] Android still booting in the background... you can keep using this window.");
        LogF("[>] FIRST boot ever? In the Android window press Alt+F1, run:");
        LogF("     echo service.adb.tcp.port=5555 >> /system/build.prop");
        LogF("     reboot -f");
    }
    InterlockedExchange(&g_busy, 0);
    PostMessage(g_hwnd, WM_APP_DONE, 0, 0);
    return 0;
}

static void StartEmulator(void){
    if (InterlockedCompareExchange(&g_busy, 1, 0) != 0){ LogF("[!] Busy - wait for the current task."); return; }
    if (ProcAlive(g_qemuProc)){
        LogF("[!] Emulator is already running.");
        InterlockedExchange(&g_busy, 0); return;
    }
    if (GetFileAttributesA(g_qemu) == INVALID_FILE_ATTRIBUTES){
        LogF("[!] Engine not found: %s", g_qemu);
        LogF("[!] Run SETUP.bat once (needs internet) to download the free engine. Then it is 100%% offline.");
        InterlockedExchange(&g_busy, 0); return;
    }
    if (GetFileAttributesA(g_disk) == INVALID_FILE_ATTRIBUTES){
        LogF("[!] Android disk image not found: %s", g_disk);
        LogF("[!] Run SETUP.bat once to create + install it (~10 minutes, one time).");
        InterlockedExchange(&g_busy, 0); return;
    }

    /* --- hardware acceleration check (what makes it FAST) --- */
    int w = WhpxAvailable();
    if (w == 1){
        LogF("[ok] Hardware acceleration (WHPX): ON - running at near-native speed.");
    } else if (w == 0){
        LogF("[!] Hardware acceleration (WHPX): OFF - will run in SLOW software mode.");
        LogF("[!] Fix (one time): Start -> 'Turn Windows features on or off' ->");
        LogF("    tick 'Windows Hypervisor Platform' -> restart PC.");
        LogF("[!] Also make sure VT-x (Intel) or SVM (AMD) is enabled in your BIOS.");
        LogF("[!] AMD needs Windows 10 2004+. After that it is full speed forever.");
    } else {
        LogF("[?] Could not probe WHPX - trying to start anyway (whpx:tcg fallback).");
    }

    char args[3200];
    snprintf(args, sizeof(args),
             "-machine accel=%s -cpu %s -m %d -smp %d "
             "-drive file=\"%s\",format=%s,if=ide "
             "-vga std -display sdl,gl=%s "
             "-audiodev %s,id=snd0 -device intel-hda -device hda-duplex,audiodev=snd0 "
             "-netdev user,id=n1,hostfwd=tcp:%s-:5555 "
             "-device e1000,netdev=n1 "
             "-usb -device usb-tablet "
             "-rtc base=localtime",
             g_accel, g_cpu, g_ramMB, g_cores,
             g_disk, g_diskFmt,
             g_gpu ? "on" : "off",
             g_audio, ADB_HOST_PORT);

    char cmd[3600];
    snprintf(cmd, sizeof(cmd), "\"%s\" %s", g_qemu, args);

    STARTUPINFOA si; memset(&si, 0, sizeof(si)); si.cb = sizeof(si);
    PROCESS_INFORMATION pi; memset(&pi, 0, sizeof(pi));
    if (!CreateProcessA(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)){
        LogF("[err] Failed to start emulator (error %lu). Check settings.ini paths.", (unsigned long)GetLastError());
        InterlockedExchange(&g_busy, 0); return;
    }
    if (g_qemuProc) CloseHandle(g_qemuProc);
    g_qemuProc = pi.hProcess;
    CloseHandle(pi.hThread);

    LogF("[>] Emulator started: %d MB RAM, %d core(s), disk: %s", g_ramMB, g_cores, g_disk);
    LogF("[>] Display: %s | Audio: %s | CPU model: %s",
         g_gpu ? "SDL + OpenGL (uses your GPU, e.g. NVIDIA)" : "SDL (safe mode)",
         g_audio, g_cpu);
    LogF("[>] If QEMU closes instantly with an audio error: set audio=none in settings.ini.");
    LogF("[>] If the window looks wrong: set gpu=0 in settings.ini.");
    LogF("[>] Waiting for Android to boot (first boot can take a few minutes)...");
    SetWindowTextA(g_hStatus, "State: starting...");

    char tmp[MAX_PATH];
    PathJoin(tmp, sizeof(tmp), g_base, "adb_server.txt");
    RunCapture(g_adb, "start-server", tmp, 15000);

    CreateThread(NULL, 0, BootWaitThread, NULL, 0, NULL);
}

static void StopEmulator(void){
    if (!ProcAlive(g_qemuProc)){
        LogF("[!] Emulator is not running.");
        return;
    }
    LogF("[x] Stopping emulator...");
    TerminateProcess(g_qemuProc, 0);
    WaitForSingleObject(g_qemuProc, 5000);
    CloseHandle(g_qemuProc);
    g_qemuProc = NULL;

    /* clean adb so the next boot reconnects instantly */
    char tmp[MAX_PATH];
    PathJoin(tmp, sizeof(tmp), g_base, "adb_tmp.txt");
    if (GetFileAttributesA(g_adb) != INVALID_FILE_ATTRIBUTES)
        RunCapture(g_adb, "disconnect " ADB_HOST_PORT, tmp, 8000);

    LogF("[x] Emulator stopped.");
    SetWindowTextA(g_hStatus, "State: stopped");
}

/* ---------------- SWITCH ANDROID (9 <-> 12) ---------------- */

static void SwitchAndroid(void){
    char d9[MAX_PATH], d12[MAX_PATH], ini[MAX_PATH];
    int has9, has12;

    if (ProcAlive(g_qemuProc)){
        LogF("[!] Stop the emulator first, then switch Android versions.");
        return;
    }
    PathJoin(d9,  sizeof(d9),  g_base, "images\\android9.img");
    PathJoin(d12, sizeof(d12), g_base, "images\\android12.img");
    PathJoin(ini, sizeof(ini), g_base, "settings.ini");
    has9  = GetFileAttributesA(d9)  != INVALID_FILE_ATTRIBUTES;
    has12 = GetFileAttributesA(d12) != INVALID_FILE_ATTRIBUTES;

    if (!has9 && !has12){
        LogF("[!] No Android disks installed yet. Run SETUP.bat first.");
        return;
    }
    /* decide target: if currently on 9 try 12, if on 12 try 9, else pick what exists */
    const char *target = NULL;
    if (has9 && has12){
        target = (_stricmp(g_disk, d9) == 0) ? d12 : d9;
    } else if (has12 && _stricmp(g_disk, d12) != 0){
        target = d12;
    } else if (has9 && _stricmp(g_disk, d9) != 0){
        target = d9;
    } else {
        LogF("[i] Only one Android version is installed (%s).", has12 ? "Android 12 MODERN" : "Android 9 CLASSIC");
        LogF("[i] To get the other one, re-run setup.bat with the other ISO in the android\\ folder.");
        return;
    }

    if (!WritePrivateProfileStringA("Engine", "disk", target, ini)){
        LogF("[err] Could not write settings.ini (error %lu).", (unsigned long)GetLastError());
        return;
    }
    WritePrivateProfileStringA("Engine", "disk_format", "qcow2", ini);
    LoadSettings();
    LogF("[ok] SWITCHED Android. Now using: %s", g_disk);
    LogF("[>] Click START EMULATOR to boot it.");
    SetWindowTextA(g_hStatus, "State: stopped (switched Android)");
}

/* ---------------- UI ---------------- */

static void AppendLog(const char *s){
    int n = GetWindowTextLengthA(g_hLog);
    SendMessageA(g_hLog, EM_SETSEL, n, n);
    /* convert \n (lone) into \r\n which EDIT controls require */
    const char *p = s;
    char chunk[1025]; size_t c = 0;
    while (*p){
        char ch = *p++;
        if (ch == '\n'){
            if (c + 3 >= sizeof(chunk)){ chunk[c] = 0; SendMessageA(g_hLog, EM_REPLACESEL, FALSE, (LPARAM)chunk); c = 0; }
            chunk[c++] = '\r'; chunk[c++] = '\n';
        } else {
            if (c + 2 >= sizeof(chunk)){ chunk[c] = 0; SendMessageA(g_hLog, EM_REPLACESEL, FALSE, (LPARAM)chunk); c = 0; }
            chunk[c++] = ch;
        }
    }
    if (c){ chunk[c] = 0; SendMessageA(g_hLog, EM_REPLACESEL, FALSE, (LPARAM)chunk); }
    SendMessageA(g_hLog, EM_REPLACESEL, FALSE, (LPARAM)"\r\n");
    SendMessageA(g_hLog, EM_SCROLLCARET, 0, 0);
}

static void HandleDrop(HDROP hd){
    char path[MAX_PATH];
    UINT got = DragQueryFileA(hd, 0, path, sizeof(path));
    DragFinish(hd);
    if (!got) return;
    size_t len = strlen(path);
    if (len < 5 || _stricmp(path + len - 4, ".apk") != 0){
        LogF("[!] Not an APK file (need .apk): %s", path);
        return;
    }
    LogF("[>] APK dropped: %s", path);
    CreateThread(NULL, 0, InstallThread, mystrdup(path), 0, NULL);
}

static void PickApk(void){
    OPENFILENAMEA ofn; memset(&ofn, 0, sizeof(ofn));
    char file[MAX_PATH] = "";
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g_hwnd;
    ofn.lpstrFilter = "Android apps (*.apk)\0*.apk\0All files\0*.*\0";
    ofn.lpstrFile = file;
    ofn.nMaxFile = sizeof(file);
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrTitle = "Choose an APK to install";
    if (GetOpenFileNameA(&ofn)){
        LogF("[>] APK selected: %s", file);
        CreateThread(NULL, 0, InstallThread, mystrdup(file), 0, NULL);
    }
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
    switch (msg){
    case WM_CREATE:{
        g_hwnd = hwnd;
        LoadSettings();
        DragAcceptFiles(hwnd, TRUE);

        HFONT fBig = CreateFontA(-22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                 CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
        HFONT fSmall = CreateFontA(-15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                   DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                   CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
        HFONT fMono = CreateFontA(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                  DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                  CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
        g_hDropBrush = CreateSolidBrush(RGB(255, 249, 210));

        g_hDrop = CreateWindowExA(0, "STATIC",
            "DRAG & DROP AN .apk FILE HERE\n(install + run in one step)",
            WS_CHILD | WS_VISIBLE | SS_CENTER | SS_SUNKEN, 10, 10, 540, 64,
            hwnd, (HMENU)IDC_DROP, g_hInst, NULL);
        SendMessageA(g_hDrop, WM_SETFONT, (WPARAM)fBig, TRUE);

        g_hBtnStart  = CreateWindowExA(0, "BUTTON", "START EMULATOR", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 10, 84, 120, 34, hwnd, (HMENU)IDC_BTN_START, g_hInst, NULL);
        g_hBtnStop   = CreateWindowExA(0, "BUTTON", "STOP", WS_CHILD | WS_VISIBLE, 136, 84, 80, 34, hwnd, (HMENU)IDC_BTN_STOP, g_hInst, NULL);
        CreateWindowExA(0, "BUTTON", "INSTALL APK...", WS_CHILD | WS_VISIBLE, 222, 84, 130, 34, hwnd, (HMENU)IDC_BTN_APK, g_hInst, NULL);
        CreateWindowExA(0, "BUTTON", "SETUP (1-time)", WS_CHILD | WS_VISIBLE, 358, 84, 120, 34, hwnd, (HMENU)IDC_BTN_SETUP, g_hInst, NULL);
        CreateWindowExA(0, "BUTTON", "SETTINGS", WS_CHILD | WS_VISIBLE, 484, 84, 66, 34, hwnd, (HMENU)IDC_BTN_INI, g_hInst, NULL);
        SendMessageA(g_hBtnStart, WM_SETFONT, (WPARAM)fSmall, TRUE);
        SendMessageA(g_hBtnStop,  WM_SETFONT, (WPARAM)fSmall, TRUE);
        SendMessageA(GetDlgItem(hwnd, IDC_BTN_APK),  WM_SETFONT, (WPARAM)fSmall, TRUE);
        SendMessageA(GetDlgItem(hwnd, IDC_BTN_SETUP),WM_SETFONT, (WPARAM)fSmall, TRUE);
        SendMessageA(GetDlgItem(hwnd, IDC_BTN_INI),  WM_SETFONT, (WPARAM)fSmall, TRUE);

        /* second row: ARM support + version switch */
        CreateWindowExA(0, "BUTTON", "ENABLE ARM (1-time)", WS_CHILD | WS_VISIBLE, 10, 124, 170, 32, hwnd, (HMENU)IDC_BTN_ARM, g_hInst, NULL);
        CreateWindowExA(0, "BUTTON", "SWITCH ANDROID", WS_CHILD | WS_VISIBLE, 190, 124, 120, 32, hwnd, (HMENU)IDC_BTN_SWITCH, g_hInst, NULL);
        CreateWindowExA(0, "STATIC", "9 <-> 12 (both must be installed)",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 320, 131, 230, 20, hwnd, (HMENU)IDC_NOTE, g_hInst, NULL);
        SendMessageA(GetDlgItem(hwnd, IDC_BTN_ARM),    WM_SETFONT, (WPARAM)fSmall, TRUE);
        SendMessageA(GetDlgItem(hwnd, IDC_BTN_SWITCH), WM_SETFONT, (WPARAM)fSmall, TRUE);
        SendMessageA(GetDlgItem(hwnd, IDC_NOTE),       WM_SETFONT, (WPARAM)fSmall, TRUE);

        CreateWindowExA(0, "STATIC", "Log:", WS_CHILD | WS_VISIBLE, 10, 166, 60, 18, hwnd, NULL, g_hInst, NULL);

        g_hLog = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
            WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
            10, 186, 540, 292, hwnd, (HMENU)IDC_LOG, g_hInst, NULL);
        SendMessageA(g_hLog, WM_SETFONT, (WPARAM)fMono, TRUE);

        g_hStatus = CreateWindowExA(0, "STATIC", "State: stopped   (engine: see Log)",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 488, 540, 24, hwnd, (HMENU)IDC_STATUS, g_hInst, NULL);
        SendMessageA(g_hStatus, WM_SETFONT, (WPARAM)fSmall, TRUE);

        LogF("AndroidRun v1.4 - offline Android APK player (free & open source)");
        LogF("Engine (QEMU): %s", g_qemu);
        LogF("Disk image   : %s", g_disk);
        LogF("RAM %d MB, %d core(s), accel=%s, cpu=%s", g_ramMB, g_cores, g_accel, g_cpu);
        LogF("Drag & drop an APK, or click INSTALL APK...");
        LogF("First time? Click SETUP (1-time) - needs internet ONCE, then offline forever.");
        LogF("For ARM (32-bit) apps: Android must be running, then click ENABLE ARM once.");
        LogF("ARM64-only apps need the MODERN (Android 12) build - swap with SWITCH ANDROID.");
        return 0;
    }

    case WM_CTLCOLORSTATIC:{
        HDC hdc = (HDC)wParam;
        if ((HWND)lParam == g_hDrop){
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(90, 70, 0));
            return (LRESULT)g_hDropBrush;
        }
        break;
    }

    case WM_DROPFILES:
        HandleDrop((HDROP)wParam);
        return 0;

    case WM_COMMAND:{
        int id = LOWORD(wParam);
        switch (id){
        case IDC_BTN_START:  StartEmulator(); return 0;
        case IDC_BTN_STOP:   StopEmulator();  return 0;
        case IDC_BTN_APK:    PickApk();       return 0;
        case IDC_BTN_SWITCH: SwitchAndroid(); return 0;
        case IDC_BTN_SETUP:
            ShellExecuteA(NULL, "open", "setup.bat", NULL, g_base, SW_SHOWNORMAL);
            LogF("[>] Opening SETUP.bat... follow the on-screen instructions (one time).");
            return 0;
        case IDC_BTN_ARM:
            ShellExecuteA(NULL, "open", "arm_enable.bat", NULL, g_base, SW_SHOWNORMAL);
            LogF("[>] Opening arm_enable.bat - Android must be running for this.");
            return 0;
        case IDC_BTN_INI:{
            char ini[MAX_PATH];
            PathJoin(ini, sizeof(ini), g_base, "settings.ini");
            ShellExecuteA(NULL, "open", "notepad.exe", ini, g_base, SW_SHOWNORMAL);
            return 0;
        }
        case IDC_BTN_FOLDER:
            ShellExecuteA(NULL, "open", g_base, NULL, NULL, SW_SHOW);
            return 0;
        }
        break;
    }

    case WM_APP_LOG:
        AppendLog((const char*)lParam);
        free((void*)lParam);
        return 0;

    case WM_APP_DONE:
        SetWindowTextA(g_hStatus, ProcAlive(g_qemuProc) ? "State: running" : "State: stopped");
        return 0;

    case WM_CLOSE:
        if (ProcAlive(g_qemuProc)){
            int r = MessageBoxA(hwnd,
                "The Android emulator is still running.\n\n"
                "Yes  = stop it and exit\n"
                "No   = keep it running in the background and close this window\n"
                "Cancel = go back",
                "AndroidRun", MB_YESNOCANCEL | MB_ICONQUESTION);
            if (r == IDCANCEL) return 0;
            if (r == IDNO){ g_keepRunning = 1; }
            else StopEmulator();
        }
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        if (!g_keepRunning && ProcAlive(g_qemuProc)){
            TerminateProcess(g_qemuProc, 0);
            CloseHandle(g_qemuProc);
            g_qemuProc = NULL;
        }
        if (g_hDropBrush) DeleteObject(g_hDropBrush);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpCmd, int nShow){
    (void)hPrev; (void)lpCmd; (void)nShow;
    g_hInst = hInst;
    ExeDir(g_base, sizeof(g_base));

    WNDCLASSEXA wc; memset(&wc, 0, sizeof(wc));
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hIcon         = LoadIconA(hInst, MAKEINTRESOURCEA(1));
    wc.hIconSm       = LoadIconA(hInst, MAKEINTRESOURCEA(1));
    wc.hCursor       = LoadCursorA(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = "AndroidRunWnd";
    RegisterClassExA(&wc);

    HWND hwnd = CreateWindowExA(0, "AndroidRunWnd", "AndroidRun v1.4 - offline Android APK player (Win10/11 x64)",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 576, 562,
        NULL, NULL, hInst, NULL);
    if (!hwnd) return 1;

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)){
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return (int)msg.wParam;
}
