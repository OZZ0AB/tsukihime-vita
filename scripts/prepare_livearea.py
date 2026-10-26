#!/usr/bin/env python3
"""Resize user-selected artwork into Vita LiveArea resources without stretching."""

import argparse
from pathlib import Path

from PIL import Image, ImageOps


def save_indexed(image, path):
    indexed = image.convert("RGB").quantize(
        colors=256, method=Image.Quantize.MEDIANCUT,
        dither=Image.Dither.FLOYDSTEINBERG,
    )
    indexed.save(path, format="PNG", optimize=True)
    if path.stat().st_size > 420 * 1024:
        raise ValueError(f"LiveArea image exceeds 420 KiB: {path}")
    print(f"{path}: {image.width}x{image.height}, indexed PNG, {path.stat().st_size} bytes")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--icon", type=Path, required=True)
    parser.add_argument("--background", type=Path, required=True)
    parser.add_argument("--output", type=Path, default=Path("assets/livearea"))
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)

    with Image.open(args.icon) as source:
        icon = ImageOps.exif_transpose(source).convert("RGB")
        icon = ImageOps.fit(icon, (128, 128), method=Image.Resampling.LANCZOS)
        save_indexed(icon, args.output / "icon0.png")

    with Image.open(args.background) as source:
        background = ImageOps.exif_transpose(source).convert("RGB")
        for name, size in (("bg.png", (840, 500)), ("startup.png", (280, 158))):
            # A centered crop retains the central title and character, with the
            # original proportions preserved for each required Vita canvas.
            image = ImageOps.fit(background, size, method=Image.Resampling.LANCZOS,
                                 centering=(0.5, 0.5))
            save_indexed(image, args.output / name)


if __name__ == "__main__":
    main()
