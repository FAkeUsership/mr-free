#!/bin/bash
# Build AndroidRun.exe for Windows 10/11 x64 (cross-compile from Linux)
# Works with gcc-mingw-w64 OR llvm-mingw. Icon is only regenerated if
# python3 can run make_icon.py; otherwise the committed icon.ico is used.
set -e
cd "$(dirname "$0")/src"

CC="${CC:-x86_64-w64-mingw32-gcc}"
WINDRES="${WINDRES:-x86_64-w64-mingw32-windres}"
command -v "$CC" >/dev/null || { echo "ERROR: $CC not found (install gcc-mingw-w64-x86-64 or llvm-mingw)"; exit 1; }
command -v "$WINDRES" >/dev/null || { echo "ERROR: $WINDRES not found"; exit 1; }

if [ ! -f icon.ico ] || python3 make_icon.py 2>/dev/null; then
    echo "icon: ok"
else
    echo "icon: make_icon.py failed (Pillow missing?) - using existing icon.ico"
    [ -f icon.ico ] || { echo "ERROR: no icon.ico"; exit 1; }
fi

"$WINDRES" app.rc -o app.o
"$CC" -Wall -Wextra -O2 -s -mwindows \
    -o ../AndroidRun.exe AndroidRun.c app.o \
    -lshell32 -lcomdlg32 -lgdi32 -luser32 -lole32
rm -f app.o
echo "BUILD OK:"
ls -la ../AndroidRun.exe
