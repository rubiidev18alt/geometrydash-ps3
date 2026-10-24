#!/usr/bin/env python3
"""Convert the prototype's simple binary PPM screenshots to PNG (stdlib only)."""

import struct
import sys
import zlib
from pathlib import Path


def chunk(kind: bytes, data: bytes) -> bytes:
    return (struct.pack(">I", len(data)) + kind + data +
            struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF))


def convert(source: Path, destination: Path) -> None:
    raw = source.read_bytes()
    marker = b"255\n"
    end = raw.find(marker)
    if not raw.startswith(b"P6\n") or end < 0:
        raise ValueError(f"not a supported binary PPM: {source}")
    header = raw[:end + len(marker)].decode("ascii").split()
    width, height = int(header[1]), int(header[2])
    pixels = raw[end + len(marker):]
    if len(pixels) != width * height * 3:
        raise ValueError(f"unexpected pixel payload size in {source}")

    rows = b"".join(b"\0" + pixels[y * width * 3:(y + 1) * width * 3]
                    for y in range(height))
    png = (b"\x89PNG\r\n\x1a\n" +
           chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)) +
           chunk(b"IDAT", zlib.compress(rows, 9)) +
           chunk(b"IEND", b""))
    destination.write_bytes(png)


if __name__ == "__main__":
    if len(sys.argv) != 3:
        raise SystemExit("usage: ppm_to_png.py INPUT.ppm OUTPUT.png")
    convert(Path(sys.argv[1]), Path(sys.argv[2]))
