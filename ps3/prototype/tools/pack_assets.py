#!/usr/bin/env python3
"""Decode the small set of source PNGs into the PS3 prototype's RGBA bundle.

Uses Python's standard library only, so host preview/test and PS3 builds do not
need Pillow or an image toolkit. The target receives a compact, embedded raw
texture bundle and needs no runtime PNG decoder.
"""

from __future__ import annotations

import struct
import sys
import zlib
from pathlib import Path

ASSET_PATHS = (
    Path("gfx/icons/player_01_001.png"),
    Path("gfx/icons/player_01_2_001.png"),
    Path("gfx/block001_01_001.png"),
    Path("gfx/spike_01_001.png"),
    Path("gfx/back_and_grounds/bg.png"),
    Path("gfx/back_and_grounds/ground.png"),
)
PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"


def paeth(a: int, b: int, c: int) -> int:
    p = a + b - c
    pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
    if pa <= pb and pa <= pc:
        return a
    if pb <= pc:
        return b
    return c


def decode_png(path: Path) -> tuple[int, int, bytes]:
    data = path.read_bytes()
    if not data.startswith(PNG_SIGNATURE):
        raise ValueError(f"not a PNG: {path}")

    width = height = bit_depth = color_type = interlace = None
    palette = transparency = None
    compressed = bytearray()
    pos = len(PNG_SIGNATURE)
    while pos + 12 <= len(data):
        length = struct.unpack_from(">I", data, pos)[0]
        kind = data[pos + 4:pos + 8]
        payload = data[pos + 8:pos + 8 + length]
        if pos + 12 + length > len(data):
            raise ValueError(f"truncated PNG chunk in {path}")
        if kind == b"IHDR":
            width, height, bit_depth, color_type, comp, filt, interlace = struct.unpack(">IIBBBBB", payload)
            if comp != 0 or filt != 0:
                raise ValueError(f"unsupported PNG compression/filter in {path}")
        elif kind == b"PLTE":
            palette = payload
        elif kind == b"tRNS":
            transparency = payload
        elif kind == b"IDAT":
            compressed.extend(payload)
        elif kind == b"IEND":
            break
        pos += 12 + length

    if not width or not height or bit_depth != 8 or interlace != 0:
        raise ValueError(f"unsupported PNG format in {path}: {width}x{height}, depth={bit_depth}, interlace={interlace}")

    channels = {2: 3, 3: 1, 6: 4}.get(color_type)
    if channels is None or (color_type == 3 and palette is None):
        raise ValueError(f"unsupported PNG color type {color_type} in {path}")
    stride = width * channels
    raw = zlib.decompress(compressed)
    if len(raw) != height * (stride + 1):
        raise ValueError(f"unexpected decompressed PNG size in {path}")

    rows: list[bytearray] = []
    prev = bytearray(stride)
    cursor = 0
    for _ in range(height):
        filter_type = raw[cursor]
        scan = bytearray(raw[cursor + 1:cursor + 1 + stride])
        cursor += stride + 1
        for i in range(stride):
            left = scan[i - channels] if i >= channels else 0
            above = prev[i]
            upper_left = prev[i - channels] if i >= channels else 0
            if filter_type == 1:
                scan[i] = (scan[i] + left) & 255
            elif filter_type == 2:
                scan[i] = (scan[i] + above) & 255
            elif filter_type == 3:
                scan[i] = (scan[i] + ((left + above) >> 1)) & 255
            elif filter_type == 4:
                scan[i] = (scan[i] + paeth(left, above, upper_left)) & 255
            elif filter_type != 0:
                raise ValueError(f"unknown PNG filter {filter_type} in {path}")
        rows.append(scan)
        prev = scan

    rgba = bytearray(width * height * 4)
    out = 0
    if color_type == 6:
        for row in rows:
            rgba[out:out + len(row)] = row
            out += len(row)
    elif color_type == 2:
        for row in rows:
            for i in range(0, len(row), 3):
                rgba[out:out + 4] = bytes((row[i], row[i + 1], row[i + 2], 255))
                out += 4
    else:  # indexed color
        colors = [tuple(palette[i:i + 3]) for i in range(0, len(palette), 3)]
        for row in rows:
            for index in row:
                if index >= len(colors):
                    raise ValueError(f"palette index out of range in {path}")
                r, g, b = colors[index]
                a = transparency[index] if transparency and index < len(transparency) else 255
                rgba[out:out + 4] = bytes((r, g, b, a))
                out += 4

    return width, height, bytes(rgba)


def pack(output: Path) -> None:
    bundle = bytearray(b"P3TX" + struct.pack("<HH", 1, len(ASSET_PATHS)))
    for path in ASSET_PATHS:
        width, height, rgba = decode_png(path)
        if width > 65535 or height > 65535:
            raise ValueError(f"asset exceeds bundle dimensions: {path}")
        bundle.extend(struct.pack("<HHI", width, height, len(rgba)))
        bundle.extend(rgba)
        print(f"{path}: {width}x{height}")
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_bytes(bundle)
    print(f"wrote {output} ({len(bundle)} bytes)")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        raise SystemExit("usage: pack_assets.py OUTPUT_BUNDLE")
    pack(Path(sys.argv[1]))
