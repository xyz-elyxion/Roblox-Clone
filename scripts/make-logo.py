#!/usr/bin/env python3
"""Draws the Linear mark into the icon files the executables and the site use.

The mark has three sources of truth and this is the only one that produces
binaries: web/public/assets/linear-mark.svg is the same shapes written as
vector for the browser, and include/LogoMark.h draws them in the windows. The
numbers below are the SVG's numbers, in units of 512, so all three agree.

    python3 scripts/make-logo.py

writes src/app.ico (the icon the built executables carry, which is also the
window icon: linearwin loads the resource named LINEARWIN_ICON) and
web/public/favicon.ico (what a browser asks the site for by itself).
"""

import os

from PIL import Image, ImageDraw

# The palette, matching the logoSkyTop/logoSkyBottom/logoSun/logoInk roles in
# include/UiTheme.h.
SKY_TOP = (0x0B, 0x1E, 0x39, 0xFF)
SKY_BOTTOM = (0x2E, 0x7B, 0xD6, 0xFF)
SUN = (0xFF, 0xD1, 0x66, 0xFF)
INK = (0xFF, 0xFF, 0xFF, 0xFF)

SIZES = (256, 128, 64, 48, 32, 24, 16)

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def unit(value):
    """A coordinate in the SVG's 512 grid, as a share of the tile."""
    return value / 512.0


def draw_master(size=1024):
    """The mark at one size, drawn with the SVG's geometry."""
    image = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)

    def box(left, top, right, bottom):
        return (
            unit(left) * size,
            unit(top) * size,
            unit(right) * size,
            unit(bottom) * size,
        )

    # The tile is clipped by a rounded mask rather than drawn as one rounded
    # rectangle, because the two bands meet in a hard edge the way the horizon
    # does in the installer's art.
    tile = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    tile_draw = ImageDraw.Draw(tile)
    tile_draw.rectangle(box(0, 0, 512, 282), fill=SKY_TOP)
    tile_draw.rectangle(box(0, 282, 512, 512), fill=SKY_BOTTOM)

    mask = Image.new("L", (size, size), 0)
    ImageDraw.Draw(mask).rounded_rectangle(box(0, 0, 512, 512), radius=unit(113) * size, fill=255)
    image.paste(tile, (0, 0), mask)

    draw.ellipse(box(348 - 59, 169 - 59, 348 + 59, 169 + 59), fill=SUN)
    draw.rounded_rectangle(box(138, 118, 210, 374), radius=unit(16) * size, fill=INK)
    draw.rounded_rectangle(box(138, 302, 363, 374), radius=unit(16) * size, fill=INK)
    return image


def main():
    master = draw_master()
    frames = [master.resize((size, size), Image.LANCZOS) for size in SIZES]

    for relative in ("src/app.ico", "web/public/favicon.ico"):
        path = os.path.join(ROOT, relative)
        frames[0].save(path, format="ICO", sizes=[(size, size) for size in SIZES])
        print("wrote", relative)


if __name__ == "__main__":
    main()
