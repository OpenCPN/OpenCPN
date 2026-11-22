#!/usr/bin/env python3
"""Create disposable MBTiles fixtures or check #5566 chart-open attempts."""

import argparse
from collections import Counter
from pathlib import Path
import sqlite3
import struct
import zlib


def create_charts(directory):
    directory.mkdir(parents=True, exist_ok=False)

    def chunk(kind, data):
        return (struct.pack("!I", len(data)) + kind + data
                + struct.pack("!I", zlib.crc32(kind + data)))

    image = (b"\x89PNG\r\n\x1a\n"
             + chunk(b"IHDR", struct.pack("!2I5B", 256, 256, 8, 2, 0, 0, 0))
             + chunk(b"IDAT", zlib.compress(
                 (b"\0" + bytes([100, 160, 210]) * 256) * 256))
             + chunk(b"IEND", b""))
    for name in ("a.mbtiles", "b.mbtiles"):
        with sqlite3.connect(directory / name) as db:
            db.executescript(
                "CREATE TABLE metadata (name TEXT, value TEXT);"
                "CREATE TABLE tiles (zoom_level INTEGER, tile_column INTEGER,"
                " tile_row INTEGER, tile_data BLOB);")
            db.executemany("INSERT INTO metadata VALUES (?, ?)", [
                ("name", name), ("type", "overlay"), ("version", "1"),
                ("format", "png"), ("bounds", "-2,50,0,52"),
                ("minzoom", "8"), ("maxzoom", "8")])
            db.execute("INSERT INTO tiles VALUES (8,127,170,?)", (image,))
    print(f"Created test charts in {directory}; not for navigation.")


def check_log(directory, log):
    paths = {str(directory / name) for name in ("a.mbtiles", "b.mbtiles")}
    counts = Counter()
    for line in log.read_text(errors="replace").splitlines():
        _, found, chart = line.partition("Initializing Chart ")
        if found and chart in paths:
            counts[chart] += 1
    for path in sorted(paths):
        print(f"{path}: {counts[path]} attempts")
    # Zero would be an invalid reproduction, not a successful regression test.
    return all(counts[path] == 1 for path in paths)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=("create", "check"))
    parser.add_argument("directory", type=Path)
    parser.add_argument("--log", type=Path)
    args = parser.parse_args()
    directory = args.directory.absolute()
    if args.action == "create":
        create_charts(directory)
    else:
        if args.log is None:
            parser.error("check requires --log")
        raise SystemExit(0 if check_log(directory, args.log) else 1)
