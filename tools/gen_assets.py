#!/usr/bin/env python3

import math
import os
import random

from PIL import Image, ImageDraw, ImageFilter, ImageFont, ImageChops

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
MODELS = os.path.join(ROOT, "models")
TEXTURES = os.path.join(ROOT, "textures")

FONT_LATIN = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf"
FONT_CJK = "/usr/share/fonts/opentype/noto/NotoSansCJK-Bold.ttc"

class ObjWriter:

    def __init__(self):
        self.v, self.vt, self.vn, self.f = [], [], [], []

    def vertex(self, p, uv, n):
        self.v.append(p)
        self.vt.append(uv)
        self.vn.append(n)
        return len(self.v)

    def tri(self, a, b, c):
        self.f.append((a, b, c))

    def quad(self, a, b, c, d):

        self.tri(a, b, c)
        self.tri(a, c, d)

    def save(self, name, comment):
        path = os.path.join(MODELS, name)
        with open(path, "w") as out:
            for p in self.v:
                out.write("v %.5f %.5f %.5f\n" % p)
            for uv in self.vt:
                out.write("vt %.5f %.5f\n" % uv)
            for n in self.vn:
                out.write("vn %.5f %.5f %.5f\n" % n)
            for (a, b, c) in self.f:
                out.write("f %d/%d/%d %d/%d/%d %d/%d/%d\n" % (a, a, a, b, b, b, c, c, c))
        print("  %-22s %4d triangles" % (name, len(self.f)))

def make_quad():
    o = ObjWriter()
    n = (0.0, 0.0, 1.0)
    a = o.vertex((-0.5, -0.5, 0.0), (0, 0), n)
    b = o.vertex((0.5, -0.5, 0.0), (1, 0), n)
    c = o.vertex((0.5, 0.5, 0.0), (1, 1), n)
    d = o.vertex((-0.5, 0.5, 0.0), (0, 1), n)
    o.quad(a, b, c, d)
    o.save("nn_quad.obj", "Unit quad, XY plane, facing +Z")

def make_box():
    o = ObjWriter()
    faces = [

        ((0, 0, 1), (-0.5, 0, 0.5), (1, 0, 0), (0, 1, 0)),
        ((0, 0, -1), (0.5, 0, -0.5), (-1, 0, 0), (0, 1, 0)),
        ((1, 0, 0), (0.5, 0, 0.5), (0, 0, -1), (0, 1, 0)),
        ((-1, 0, 0), (-0.5, 0, -0.5), (0, 0, 1), (0, 1, 0)),
        ((0, 1, 0), (-0.5, 1, 0.5), (1, 0, 0), (0, 0, -1)),
        ((0, -1, 0), (-0.5, 0, -0.5), (1, 0, 0), (0, 0, 1)),
    ]
    for n, org, u, v in faces:
        idx = []
        for (s, t) in ((0, 0), (1, 0), (1, 1), (0, 1)):
            p = tuple(org[i] + s * u[i] + t * v[i] for i in range(3))
            idx.append(o.vertex(p, (s, t), n))
        o.quad(*idx)
    o.save("nn_box.obj", "Unit box, base at y=0")

def make_tube(segments=12):
    o = ObjWriter()
    ring = []
    for i in range(segments + 1):
        a = 2.0 * math.pi * i / segments
        x, z = 0.5 * math.cos(a), 0.5 * math.sin(a)
        n = (math.cos(a), 0.0, math.sin(a))
        u = float(i) / segments
        ring.append((o.vertex((x, 0.0, z), (u, 0), n), o.vertex((x, 1.0, z), (u, 1), n)))
    for i in range(segments):
        b0, t0 = ring[i]
        b1, t1 = ring[i + 1]
        o.quad(b0, t0, t1, b1)
    for y, ny in ((1.0, 1.0), (0.0, -1.0)):
        c = o.vertex((0.0, y, 0.0), (0.5, 0.5), (0, ny, 0))
        pts = []
        for i in range(segments):
            a = 2.0 * math.pi * i / segments
            pts.append(o.vertex((0.5 * math.cos(a), y, 0.5 * math.sin(a)),
                                (0.5 + 0.5 * math.cos(a), 0.5 + 0.5 * math.sin(a)), (0, ny, 0)))
        for i in range(segments):
            if ny > 0:
                o.tri(c, pts[(i + 1) % segments], pts[i])
            else:
                o.tri(c, pts[i], pts[(i + 1) % segments])
    o.save("nn_tube.obj", "Cylinder r=0.5 along +Y, y in [0,1]")

def make_cone(segments=24):
    o = ObjWriter()
    slant = 1.0 / math.sqrt(2.0)
    rings = []
    for i in range(segments + 1):
        a = 2.0 * math.pi * i / segments
        x, y = math.cos(a), math.sin(a)
        n = (x * slant, y * slant, slant)
        u = float(i) / segments
        rings.append((o.vertex((0.0, 0.0, 0.0), (u, 0.0), n), o.vertex((x, y, -1.0), (u, 1.0), n)))
    for i in range(segments):
        a0, b0 = rings[i]
        a1, b1 = rings[i + 1]
        o.tri(a0, b0, b1)
    o.save("nn_cone.obj", "Open cone (light beam), apex at origin, opening along -Z")

def save(img, name):
    img.save(os.path.join(TEXTURES, name))
    print("  %-22s %dx%d" % (name, img.size[0], img.size[1]))

def tile3(img):
    w, h = img.size
    big = Image.new(img.mode, (w * 3, h * 3))
    for i in range(3):
        for j in range(3):
            big.paste(img, (i * w, j * h))
    return big

def blur_tileable(img, radius):
    w, h = img.size
    return tile3(img).filter(ImageFilter.GaussianBlur(radius)).crop((w, h, 2 * w, 2 * h))

def noise(size, sigma, blur=0.0, mean=128):
    img = Image.effect_noise(size, sigma)
    if mean != 128:
        img = img.point(lambda p: max(0, min(255, p - 128 + mean)))
    if blur > 0:
        img = blur_tileable(img, blur)
    return img

def tinted(gray, color):
    return Image.merge("RGB", [gray.point(lambda p, c=c: p * c // 255) for c in color])

WINDOW_COLORS = [(255, 214, 150), (255, 190, 120), (255, 160, 90), (170, 220, 255),
                 (220, 235, 255), (150, 255, 225)]

def make_facade(name, cols, rows, lit_ratio, style, seed):
    random.seed(seed)
    size = 512
    cw, ch = size // cols, size // rows

    base = tinted(noise((size, size), 18, 0.6, 140), (165, 160, 178))
    rgb = base.copy()
    alpha = Image.new("L", (size, size), 0)
    d = ImageDraw.Draw(rgb)
    da = ImageDraw.Draw(alpha)

    for r in range(rows + 1):
        d.line([(0, r * ch), (size, r * ch)], fill=(45, 44, 52), width=3)

    for r in range(rows):
        if style == "strips":

            y0, y1 = r * ch + int(ch * 0.30), r * ch + int(ch * 0.78)
            d.rectangle([0, y0, size, y1], fill=(40, 48, 66))
            x = 0
            while x < size:
                w = random.randint(cw // 2, cw * 2)
                if random.random() < lit_ratio:
                    col = random.choice(WINDOW_COLORS)
                    k = random.uniform(0.3, 1.0)
                    col = tuple(int(c * k) for c in col)
                    d.rectangle([x + 2, y0 + 3, min(size, x + w) - 2, y1 - 3], fill=col)
                    da.rectangle([x + 2, y0 + 3, min(size, x + w) - 2, y1 - 3], fill=255)
                x += w
            for xm in range(0, size, cw // 2):
                d.line([(xm, y0), (xm, y1)], fill=(15, 15, 20), width=2)
        else:
            for c in range(cols):
                x0, y0 = c * cw + int(cw * 0.22), r * ch + int(ch * 0.25)
                x1, y1 = (c + 1) * cw - int(cw * 0.22), (r + 1) * ch - int(ch * 0.20)
                if style == "industrial" and random.random() < 0.35:

                    d.rectangle([x0, y0, x1, y1], fill=(45, 45, 50))
                    for yy in range(y0 + 4, y1, 6):
                        d.line([(x0 + 3, yy), (x1 - 3, yy)], fill=(20, 20, 24), width=2)
                    continue
                if random.random() < lit_ratio:
                    col = random.choice(WINDOW_COLORS)
                    k = random.uniform(0.3, 1.0)
                    col = tuple(int(v * k) for v in col)
                    d.rectangle([x0, y0, x1, y1], fill=col)
                    da.rectangle([x0, y0, x1, y1], fill=255)

                    if random.random() < 0.4:
                        yb = random.randint(y0, (y0 + y1) // 2)
                        d.rectangle([x0, y0, x1, yb], fill=(30, 28, 34))
                        da.rectangle([x0, y0, x1, yb], fill=0)
                else:
                    d.rectangle([x0, y0, x1, y1], fill=(40, 48, 66))
                    d.line([(x0, y1), (x1, y0)], fill=(70, 82, 105), width=3)
                d.rectangle([x0, y0, x1, y1], outline=(30, 30, 36), width=4)

    grime = blur_tileable(noise((size, size // 8), 40, 0, 128).resize((size, size)), 1)
    rgb = ImageChops.multiply(rgb, Image.merge("RGB", [grime.point(lambda p: 170 + p // 3)] * 3))
    rgb.putalpha(alpha)
    save(rgb, name)

def make_asphalt():
    size = 512
    random.seed(7)
    gray = noise((size, size), 30, 0.8, 95)
    rgb = tinted(gray, (150, 148, 162))
    d = ImageDraw.Draw(rgb)

    for _ in range(14):
        x, y = random.randint(0, size), random.randint(0, size)
        pts = [(x, y)]
        for _ in range(random.randint(4, 9)):
            x += random.randint(-30, 30)
            y += random.randint(-30, 30)
            pts.append((x, y))
        d.line(pts, fill=(14, 14, 16), width=2)
    save(rgb, "nn_asphalt.png")

    small = noise((size // 16, size // 16), 90, 0, 128)
    blobs = tile3(small).resize((size * 3, size * 3), Image.BICUBIC).crop((size, size, 2 * size, 2 * size))
    blobs = blur_tileable(blobs, 10)
    spec = blobs.point(lambda p: 255 if p > 165 else (35 + (p - 140) * 8 if p > 140 else 35))
    spec = blur_tileable(spec, 3)
    save(spec.convert("RGB"), "nn_asphalt_spec.png")

def glow_text_layer(size, lines, border):
    w, h = size
    core = Image.new("L", size, 0)
    d = ImageDraw.Draw(core)
    if border:
        d.rounded_rectangle([12, 12, w - 13, h - 13], radius=26, outline=255, width=7)
    for (text, font_path, font_size, y, vertical) in lines:
        font = ImageFont.truetype(font_path, font_size)
        if vertical:
            yy = y
            for ch in text:
                bb = d.textbbox((0, 0), ch, font=font)
                d.text(((w - (bb[2] - bb[0])) / 2 - bb[0], yy - bb[1]), ch, font=font, fill=255)
                yy += int(font_size * 1.08)
        else:
            bb = d.textbbox((0, 0), text, font=font)
            d.text(((w - (bb[2] - bb[0])) / 2 - bb[0], y - bb[1]), text, font=font, fill=255)

    glow = core.filter(ImageFilter.GaussianBlur(9))
    glow = glow.point(lambda p: min(255, p * 2))
    alpha = ImageChops.lighter(core, glow)

    panel = tinted(noise(size, 10, 0.5, 40), (90, 90, 100))
    tube = Image.merge("RGB", [core.point(lambda p: p * 230 // 255)] * 3)
    rgb = ImageChops.lighter(panel, tube)
    rgb.putalpha(alpha)
    return rgb

def make_signs():
    save(glow_text_layer((1024, 384), [("NEON NOIR", FONT_LATIN, 150, 40, False),
                                       ("— 2087 —", FONT_LATIN, 96, 220, False)], True), "nn_sign_title.png")
    save(glow_text_layer((256, 1024), [("ネオン街", FONT_CJK, 190, 40, True)], True), "nn_sign_kanji.png")
    save(glow_text_layer((1024, 320), [("ラーメン", FONT_CJK, 170, 20, False),
                                       ("RAMEN  24H", FONT_LATIN, 62, 228, False)], True), "nn_sign_ramen.png")
    save(glow_text_layer((512, 256), [("BAR", FONT_LATIN, 130, 30, False),
                                      ("OPEN", FONT_LATIN, 48, 180, False)], True), "nn_sign_bar.png")
    save(glow_text_layer((256, 768), [("電脳", FONT_CJK, 180, 60, True),
                                      ("2087", FONT_LATIN, 58, 520, False)], True), "nn_sign_cyber.png")

def make_misc():
    save(Image.new("RGB", (8, 8), (255, 255, 255)), "nn_white.png")

    size = 256
    halo = Image.new("L", (size, size), 0)
    px = halo.load()
    for y in range(size):
        for x in range(size):
            r = math.hypot(x - size / 2 + 0.5, y - size / 2 + 0.5) / (size / 2)
            v = max(0.0, 1.0 - r)
            px[x, y] = int(255 * (v ** 2.2))
    save(Image.merge("RGB", [halo] * 3), "nn_halo.png")

    size = 256
    rgb = tinted(noise((size, size), 14, 0.5, 60), (120, 125, 135))
    d = ImageDraw.Draw(rgb)
    for k in range(0, size, 64):
        d.line([(k, 0), (k, size)], fill=(25, 25, 30), width=2)
        d.line([(0, k), (size, k)], fill=(25, 25, 30), width=2)
    for k in range(-size, size, 32):
        d.polygon([(k, 200), (k + 16, 200), (k + 40, 224), (k + 24, 224)], fill=(200, 160, 20))
    d.rectangle([0, 224, size, 232], fill=(20, 20, 24))
    save(rgb, "nn_drone.png")

if __name__ == "__main__":
    os.makedirs(MODELS, exist_ok=True)
    os.makedirs(TEXTURES, exist_ok=True)
    print("Meshes:")
    make_quad()
    make_box()
    make_tube()
    make_cone()
    print("Textures:")
    make_facade("nn_facade_a.png", 4, 4, 0.35, "windows", 11)
    make_facade("nn_facade_b.png", 4, 6, 0.45, "strips", 23)
    make_facade("nn_facade_c.png", 3, 4, 0.20, "industrial", 37)
    make_asphalt()
    make_signs()
    make_misc()
