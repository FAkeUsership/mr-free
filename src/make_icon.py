#!/usr/bin/env python3
"""Generate a small green-android icon as icon.ico (16x16 + 32x32, 32bpp, opaque)."""
import struct, os

def make_icon(size):
    # 32bpp BGRA pixels, bottom-up rows
    px = [[(0,0,0,0)]*size for _ in range(size)]

    def put(x, y, c):
        if 0 <= x < size and 0 <= y < size:
            px[y][x] = c

    green = (0x5B, 0xA8, 0x54, 255)   # android green
    dark  = (0x20, 0x20, 0x20, 255)

    # scale coordinates from a 32x32 design grid
    def g(x, y, c):
        if size == 32:
            put(x, y, c)
        else:  # 16 -> each 2x2 block
            for dx in (0,1):
                for dy in (0,1):
                    put(x*2+dx, y*2+dy, c)

    # antennae
    g(10, 4, green); g(21, 4, green)
    g(10, 5, green); g(21, 5, green)
    # head outline (rect 6..25 x 8..21)
    for x in range(6, 26):
        for y in range(8, 22):
            g(x, y, green)
    # eyes (dark)
    for x in range(11, 14):
        for y in range(12, 15):
            g(x, y, dark)
    for x in range(18, 21):
        for y in range(12, 15):
            g(x, y, dark)
    # mouth line
    for x in range(13, 19):
        g(x, 17, dark)
    # arms
    for y in range(13, 21):
        g(4, y, green); g(5, y, green)
        g(26, y, green); g(27, y, green)
    # legs
    for x in range(10, 14):
        for y in range(22, 28):
            g(x, y, green)
    for x in range(18, 22):
        for y in range(22, 28):
            g(x, y, green)

    # BMP DIB (BITMAPINFOHEADER 40 bytes + BGRA data), bottom-up
    header = struct.pack('<IiiHHIIiiII', 40, size, size, 1, 32, 0, 0, 0, 0, 0, 0)
    data = b''
    for row in reversed(px):           # bottom-up
        for (b, g_, r, a) in row:
            data += struct.pack('<BBBB', b, g_, r, a)
    # AND mask (opaque): size/8 bytes per row, padded to 4 bytes
    and_row = size // 8
    and_pad = (4 - (and_row % 4)) % 4
    mask = b'\x00' * (and_row + and_pad)
    and_mask = mask * size
    return header + data + and_mask

def make_ico(path):
    entries = []
    blobs = []
    for size in (16, 32):
        blob = make_icon(size)
        entries.append((size, size, 0, 0, 1, 32, len(blob), 6 + 16*2 + sum(len(b) for b in blobs)))
        blobs.append(blob)
    out = struct.pack('<HHH', 0, 1, len(entries))
    for (w, h, c, r, planes, bpp, blen, boff) in entries:
        out += struct.pack('<BBBBHHII', w & 0xFF, h & 0xFF, c, r, planes, bpp, blen, boff)
    for b in blobs:
        out += b
    with open(path, 'wb') as f:
        f.write(out)
    print(f"wrote {path} ({os.path.getsize(path)} bytes)")

if __name__ == '__main__':
    here = os.path.dirname(os.path.abspath(__file__))
    make_ico(os.path.join(here, 'icon.ico'))
