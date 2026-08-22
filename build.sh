#!/bin/bash
# Build AndroidRun.exe for Windows 10 x64 (cross-compile from Linux)
set -e
cd "$(dirname "$0")/src"
python3 make_icon.py
x86_64-w64-mingw32-windres app.rc -o app.o
x86_64-w64-mingw32-gcc -Wall -Wextra -O2 -s -mwindows \
    -o ../AndroidRun.exe AndroidRun.c app.o \
    -lshell32 -lcomdlg32 -lgdi32 -luser32 -lole32
echo "BUILD OK:"
ls -la ../AndroidRun.exe
