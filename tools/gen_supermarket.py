#!/usr/bin/env python3

import math
import os
import random

from PIL import Image, ImageDraw, ImageFilter, ImageFont

from gen_assets import ObjWriter, save, noise, MODELS, TEXTURES, FONT_LATIN

def add_box(o, c, size):
    sx, sy, sz = size
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
            p = [org[i] + s * u[i] + t * v[i] for i in range(3)]
            idx.append(o.vertex((c[0] + p[0] * sx, c[1] + p[1] * sy, c[2] + p[2] * sz), (s, t), n))
        o.quad(*idx)

def add_tube(o, c, r, y0, y1, v0=0.0, v1=1.0, segments=8, top_cap=True):
    ring = []
    for i in range(segments + 1):
        a = 2.0 * math.pi * i / segments
        n = (math.cos(a), 0.0, math.sin(a))
        x, z = c[0] + r * n[0], c[2] + r * n[2]
        u = float(i) / segments
        ring.append((o.vertex((x, y0, z), (u, v0), n), o.vertex((x, y1, z), (u, v1), n)))
    for i in range(segments):
        o.quad(ring[i][0], ring[i][1], ring[i + 1][1], ring[i + 1][0])
    if top_cap:
        center = o.vertex((c[0], y1, c[2]), (0.5, v1), (0, 1, 0))
        pts = [o.vertex((c[0] + r * math.cos(2 * math.pi * i / segments), y1, c[2] + r * math.sin(2 * math.pi * i / segments)),
                        (0.5, v1), (0, 1, 0)) for i in range(segments)]
        for i in range(segments):
            o.tri(center, pts[(i + 1) % segments], pts[i])

def add_sphere(o, c, radii, slices=8, stacks=5):
    grid = []
    for j in range(stacks + 1):
        phi = math.pi * j / stacks
        row = []
        for i in range(slices + 1):
            theta = 2.0 * math.pi * i / slices
            n = (math.sin(phi) * math.cos(theta), math.cos(phi), math.sin(phi) * math.sin(theta))
            p = (c[0] + radii[0] * n[0], c[1] + radii[1] * n[1], c[2] + radii[2] * n[2])
            row.append(o.vertex(p, (float(i) / slices, 1.0 - float(j) / stacks), n))
        grid.append(row)
    for j in range(stacks):
        for i in range(slices):
            o.quad(grid[j][i], grid[j][i + 1], grid[j + 1][i + 1], grid[j + 1][i])

def make_rows():
    for count in (4, 6):
        for shape in ("box", "can", "bottle"):
            o = ObjWriter()
            for i in range(count):
                c = ((i - (count - 1) / 2.0) * 1.1, 0.0, 0.0)
                if shape == "box":
                    add_box(o, c, (1.0, 1.0, 1.0))
                elif shape == "can":
                    add_tube(o, c, 0.5, 0.0, 1.0)
                else:
                    add_tube(o, c, 0.5, 0.0, 0.72, 0.0, 0.9)
                    add_tube(o, c, 0.2, 0.72, 0.94, 0.9, 1.0, 6, False)
                    add_tube(o, c, 0.25, 0.94, 1.0, 1.0, 1.0, 6)
            o.save("sm_row%d_%s.obj" % (count, shape), "Row of %d products (%s), unit size" % (count, shape))

def make_fruit_piles():
    for (name, nx, nz) in (("sm_fruits_small.obj", 6, 4), ("sm_fruits_big.obj", 3, 2)):
        rnd = random.Random(nx)
        o = ObjWriter()
        for layer in range(2):
            for i in range(nx - layer):
                for k in range(nz - layer):
                    s = 0.9 + 0.2 * rnd.random()
                    r = (0.5 / nx * s, 0.5 * s, 0.5 / nz * s)
                    c = ((i + 0.5 + layer * 0.5) / nx - 0.5 + rnd.uniform(-0.03, 0.03) / nx * 3,
                         r[1] + layer * 0.45,
                         (k + 0.5 + layer * 0.5) / nz - 0.5 + rnd.uniform(-0.03, 0.03) / nz * 3)
                    add_sphere(o, c, r)
        o.save(name, "Pile of fruits, unit size, base at y=0")

def make_floor():
    size, n = 512, 4
    rnd = random.Random(3)
    img = Image.new("RGB", (size, size))
    d = ImageDraw.Draw(img)
    step = size // n
    for ty in range(n):
        for tx in range(n):
            v = rnd.randint(-6, 6)
            d.rectangle([tx * step, ty * step, (tx + 1) * step - 1, (ty + 1) * step - 1],
                        fill=(236 + v, 232 + v, 222 + v))
    grain = noise((size, size), 6, 0.6, 128)
    img = Image.blend(img, Image.merge("RGB", [grain] * 3), 0.06)
    d = ImageDraw.Draw(img)
    for k in range(0, size, step):
        d.line([(k, 0), (k, size)], fill=(170, 168, 160), width=3)
        d.line([(0, k), (size, k)], fill=(170, 168, 160), width=3)
    save(img, "sm_floor.png")

def make_ceiling():
    size = 256
    img = Image.new("RGB", (size, size), (238, 238, 234))
    grain = noise((size, size), 10, 0.8, 128)
    img = Image.blend(img, Image.merge("RGB", [grain] * 3), 0.05)
    d = ImageDraw.Draw(img)
    for k in (0, size // 2):
        d.line([(k, 0), (k, size)], fill=(180, 180, 176), width=4)
        d.line([(0, k), (size, k)], fill=(180, 180, 176), width=4)
    save(img, "sm_ceiling.png")

def make_labels():

    w, h = 128, 128
    img = Image.new("L", (w, h), 255)
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, w, 22], fill=150)
    d.ellipse([34, 36, 94, 86], fill=255, outline=120, width=4)
    d.rectangle([0, 98, w, 112], fill=200)
    d.rectangle([14, 118, 60, 124], fill=120)
    save(Image.merge("RGB", [img] * 3), "sm_label_box.png")

    img = Image.new("L", (128, 128), 210)
    d = ImageDraw.Draw(img)
    d.rectangle([0, 34, 128, 90], fill=255)
    d.rectangle([0, 50, 128, 58], fill=150)
    d.rectangle([0, 0, 128, 8], fill=170)
    save(Image.merge("RGB", [img] * 3), "sm_label_can.png")

def make_price_strip():
    img = Image.new("RGB", (128, 16), (245, 245, 245))
    d = ImageDraw.Draw(img)
    d.rectangle([4, 0, 30, 16], fill=(255, 215, 30))
    save(img, "sm_price_strip.png")

def make_shadow():
    size = 128
    img = Image.new("L", (size, size), 255)
    d = ImageDraw.Draw(img)
    d.rectangle([26, 26, size - 27, size - 27], fill=95)
    img = img.filter(ImageFilter.GaussianBlur(11))
    save(Image.merge("RGB", [img] * 3), "sm_shadow.png")

def sign(name, size, background, lines, accent=None, border=True):
    w, h = size
    img = Image.new("RGB", size, background)
    d = ImageDraw.Draw(img)
    if accent:
        d.rectangle([0, h - h // 7, w, h], fill=accent)
    if border:
        d.rectangle([4, 4, w - 5, h - 5], outline=(255, 255, 255), width=5)
    for (text, font_size, y, color) in lines:
        draw_text(d, w, text, font_size, y, color)
    save(img, name)

def draw_text(d, w, text, font_size, y, color):
    while True:
        font = ImageFont.truetype(FONT_LATIN, font_size)
        bb = d.textbbox((0, 0), text, font=font)
        if bb[2] - bb[0] <= w * 0.86 or font_size < 12:
            break
        font_size -= 2
    h = bb[3] - bb[1]
    d.text(((w - (bb[2] - bb[0])) / 2 - bb[0], y - h / 2 - bb[1]), text, font=font, fill=color)

WHITE = (255, 255, 255)
GREEN = (26, 120, 72)
ORANGE = (245, 140, 30)

def make_signs():
    sign("sm_sign_title.png", (1024, 192), GREEN,
         [("SUPERMARCHÉ", 120, 82, WHITE)], ORANGE, border=False)
    aisles = [("sm_sign_epicerie.png", "ÉPICERIE", (200, 50, 45)),
              ("sm_sign_petitdej.png", "PETIT-DÉJ", (225, 160, 20)),
              ("sm_sign_snacks.png", "SNACKS", (235, 105, 25)),
              ("sm_sign_boissons.png", "BOISSONS", (30, 100, 200)),
              ("sm_sign_hygiene.png", "HYGIÈNE", (120, 75, 180)),
              ("sm_sign_frais.png", "PRODUITS FRAIS", (20, 150, 190)),
              ("sm_sign_fruits.png", "FRUITS & LÉGUMES", (60, 150, 50)),
              ("sm_sign_caisses.png", "CAISSES", GREEN)]
    for (name, text, color) in aisles:
        sign(name, (512, 128), color, [(text, 72, 64, WHITE)])

    w, h = 256, 384
    img = Image.new("RGB", (w, h), (250, 210, 30))
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, w, 90], fill=(210, 30, 35))
    for (text, size, y, color) in [("PROMO", 64, 45, WHITE), ("-30%", 104, 180, (210, 30, 35)),
                                   ("sur tout", 34, 285, (40, 40, 40)), ("le rayon", 34, 328, (40, 40, 40))]:
        draw_text(d, w, text, size, y, color)
    save(img, "sm_poster.png")

def make_beam(s_max=3.2, sheets=6, steps=8):
    o = ObjWriter()
    for vertical in (True, False):
        for i in range(sheets):
            c = (i + 0.5) / sheets
            n = (1.0, 0.0, -c) if vertical else (0.0, 1.0, -c)
            grid = []
            for j in range(steps + 1):
                s = 1.0 + (s_max - 1.0) * j / steps
                row = []
                for k in range(5):
                    t = k / 4.0
                    u, v = (c, t) if vertical else (t, c)
                    row.append(o.vertex((s * u, s * v, s), (t, float(j) / steps), n))
                grid.append(row)
            for j in range(steps):
                for k in range(4):
                    o.quad(grid[j][k], grid[j][k + 1], grid[j + 1][k + 1], grid[j + 1][k])
    o.save("sm_beam.obj", "Light beam: sheets in a pyramid cut between z=1 and z=%.1f" % s_max)

def make_dust(count=140):
    rnd = random.Random(7)
    o = ObjWriter()
    for _ in range(count):
        p = (rnd.random(), rnd.random(), 1.05 + rnd.random() * 1.6)
        phase = rnd.random()
        idx = [o.vertex(p, (0.0, 0.0), (cx, cy, phase)) for (cx, cy) in ((-1, -1), (1, -1), (1, 1), (-1, 1))]
        o.quad(*idx)
    o.save("sm_dust.obj", "Dust grains of the light beam (positions in beam coordinates)")

def make_cartons():
    for (name, layers, seed) in (("sm_cartons_a.obj", 3, 4), ("sm_cartons_b.obj", 2, 9)):
        rnd = random.Random(seed)
        o = ObjWriter()
        h = 1.0 / 3.0
        for layer in range(layers):
            for i in range(2):
                for k in range(2):
                    if layer == layers - 1 and rnd.random() < 0.3:
                        continue
                    c = (-0.25 + 0.5 * i + rnd.uniform(-0.02, 0.02), layer * h, -0.25 + 0.5 * k + rnd.uniform(-0.02, 0.02))
                    add_box(o, c, (0.47, h * 0.97, 0.47))
        o.save(name, "Pallet load of cardboard boxes, unit size")

def add_cone(o, c, r, h, segments=6):
    apex = (c[0], c[1] + h, c[2])
    for i in range(segments):
        a0 = 2.0 * math.pi * i / segments
        a1 = 2.0 * math.pi * (i + 1) / segments
        am = 0.5 * (a0 + a1)
        n = (math.cos(am), r / h, math.sin(am))
        p0 = o.vertex((c[0] + r * math.cos(a0), c[1], c[2] + r * math.sin(a0)), (0.0, 0.0), n)
        p1 = o.vertex((c[0] + r * math.cos(a1), c[1], c[2] + r * math.sin(a1)), (1.0, 0.0), n)
        o.tri(p0, o.vertex(apex, (0.5, 1.0), n), p1)

def add_path_tube(o, points, r0, r1, segments=6):
    rings = []
    for j, p in enumerate(points):
        a = points[max(j - 1, 0)]
        b = points[min(j + 1, len(points) - 1)]
        t = (b[1] - a[1], b[2] - a[2])
        length = math.hypot(t[0], t[1])
        side = (-t[1] / length, t[0] / length)
        r = r0 + (r1 - r0) * j / (len(points) - 1)
        ring = []
        for i in range(segments + 1):
            a_ = 2.0 * math.pi * i / segments
            n = (math.cos(a_), math.sin(a_) * side[0], math.sin(a_) * side[1])
            ring.append(o.vertex((p[0] + r * n[0], p[1] + r * n[1], p[2] + r * n[2]),
                                 (float(i) / segments, float(j) / (len(points) - 1)), n))
        rings.append(ring)
    for j in range(len(rings) - 1):
        for i in range(segments):
            o.quad(rings[j][i], rings[j + 1][i], rings[j + 1][i + 1], rings[j][i + 1])

def make_cat():
    o = ObjWriter()
    add_sphere(o, (0.0, 0.23, -0.02), (0.10, 0.10, 0.21), 10, 6)
    add_sphere(o, (0.0, 0.26, 0.13), (0.085, 0.095, 0.09))
    add_sphere(o, (0.0, 0.21, -0.15), (0.095, 0.09, 0.09))
    o.save("sm_cat_body.obj", "Cat: body")

    o = ObjWriter()
    add_sphere(o, (0.0, 0.03, 0.05), (0.075, 0.07, 0.07), 10, 6)
    add_sphere(o, (0.0, 0.0, 0.105), (0.042, 0.03, 0.032))
    for side in (-1, 1):
        add_cone(o, (side * 0.042, 0.075, 0.035), 0.03, 0.065)
    o.save("sm_cat_head.obj", "Cat: head (origin at the neck)")

    o = ObjWriter()
    for side in (-1, 1):
        add_sphere(o, (side * 0.03, 0.04, 0.108), (0.016, 0.016, 0.016), 8, 5)
    o.save("sm_cat_eyes.obj", "Cat: eyes (origin at the neck)")

    o = ObjWriter()
    path = [(0.0, 0.0, 0.0), (0.0, 0.02, -0.08), (0.0, 0.07, -0.15), (0.0, 0.15, -0.20),
            (0.0, 0.24, -0.21), (0.0, 0.31, -0.18)]
    add_path_tube(o, path, 0.022, 0.015)
    o.save("sm_cat_tail.obj", "Cat: tail (origin at its root)")

def make_reserve_textures():

    size = 512
    base = noise((size, size), 30, 2.0, 120)
    stains = noise((size, size), 60, 18.0, 128)
    img = Image.blend(base, stains, 0.45).point(lambda p: int(p * 0.55 + 70))
    img = Image.merge("RGB", [img, img, img.point(lambda p: p + 4)])
    d = ImageDraw.Draw(img)
    for k in (0, size // 2):
        d.line([(k, 0), (k, size)], fill=(80, 80, 82), width=2)
        d.line([(0, k), (size, k)], fill=(80, 80, 82), width=2)
    save(img, "sm_concrete.png")

    def cardboard(w, h):
        im = Image.new("RGB", (w, h), (186, 140, 92))
        grain = noise((w, h), 14, 0.8, 128)
        im = Image.blend(im, Image.merge("RGB", [grain] * 3), 0.08)
        dd = ImageDraw.Draw(im)
        dd.rectangle([w * 0.42, 0, w * 0.58, h], fill=(204, 168, 118))
        return im, dd
    img, d = cardboard(128, 128)
    font = ImageFont.truetype(FONT_LATIN, 22)
    d.text((10, 92), "↑↑", font=font, fill=(60, 40, 25))
    save(img, "sm_cardboard.png")

    img, d = cardboard(256, 256)
    draw_text(d, 256, "FRAGILE", 58, 118, (200, 25, 25))
    d.rectangle([22, 84, 234, 152], outline=(200, 25, 25), width=5)
    draw_text(d, 256, "Ne pas poser près du chat", 16, 182, (60, 40, 25))
    d.line([(116, 196), (128, 222), (140, 196)], fill=(60, 40, 25), width=4)
    d.line([(128, 222), (128, 240)], fill=(60, 40, 25), width=4)
    d.line([(116, 240), (140, 240)], fill=(60, 40, 25), width=4)
    save(img, "sm_fragile.png")

    rnd = random.Random(5)
    img = Image.new("L", (128, 64), 235)
    d = ImageDraw.Draw(img)
    x = 0
    while x < 128:
        w = rnd.randint(4, 8)
        d.polygon([(x, 0), (x + w, 0), (x + w + rnd.randint(-4, 4), 64), (x + rnd.randint(-4, 4), 64)], fill=150)
        x += w + rnd.randint(6, 12)
    img = img.filter(ImageFilter.GaussianBlur(1.5))
    save(Image.merge("RGB", [img] * 3), "sm_cat_fur.png")

    w, h = 256, 320
    img = Image.new("RGB", (w, h), (250, 248, 240))
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, w, 56], fill=GREEN)
    draw_text(d, w, "EMPLOYÉ DU MOIS", 30, 28, WHITE)
    d.rectangle([48, 70, 208, 218], fill=(120, 170, 210))
    cx, cy = 128, 152
    d.polygon([(cx - 52, cy - 20), (cx - 44, cy - 70), (cx - 14, cy - 40)], fill=(225, 135, 50))
    d.polygon([(cx + 52, cy - 20), (cx + 44, cy - 70), (cx + 14, cy - 40)], fill=(225, 135, 50))
    d.ellipse([cx - 54, cy - 46, cx + 54, cy + 50], fill=(230, 140, 55))
    for s in (-1, 1):
        d.ellipse([cx + s * 22 - 10, cy - 14, cx + s * 22 + 10, cy + 8], fill=(150, 210, 60))
        d.ellipse([cx + s * 22 - 3, cy - 12, cx + s * 22 + 3, cy + 6], fill=(20, 20, 20))
        for k in (-1, 0, 1):
            d.line([(cx + s * 16, cy + 26), (cx + s * 62, cy + 20 + k * 10)], fill=(60, 40, 30), width=2)
    d.polygon([(cx - 7, cy + 16), (cx + 7, cy + 16), (cx, cy + 24)], fill=(230, 110, 130))
    draw_text(d, w, "MINOU", 34, 244, (30, 30, 30))
    draw_text(d, w, "Chef de la sécurité (nuit)", 16, 274, (90, 90, 90))
    draw_text(d, w, "Prime : 1 sac de croquettes", 15, 298, (200, 60, 30))
    save(img, "sm_employee.png")

    img = Image.new("RGB", (128, 128), (110, 50, 150))
    d = ImageDraw.Draw(img)
    d.rectangle([0, 0, 128, 30], fill=(250, 200, 40))
    draw_text(d, 128, "CROQUETTES", 18, 15, (110, 50, 150))
    d.ellipse([34, 46, 94, 100], fill=(240, 150, 60))
    d.polygon([(38, 62), (42, 36), (58, 50)], fill=(240, 150, 60))
    d.polygon([(90, 62), (86, 36), (70, 50)], fill=(240, 150, 60))
    draw_text(d, 128, "MIAOU !", 16, 114, WHITE)
    save(img, "sm_croquettes.png")

    sign("sm_sign_reserve.png", (512, 128), (60, 64, 70),
         [("RÉSERVE", 64, 50, WHITE), ("PERSONNEL AUTORISÉ", 24, 102, (250, 200, 40))])
    sign("sm_sign_exit.png", (512, 128), (20, 160, 70), [("SORTIE  →", 72, 64, WHITE)])

if __name__ == "__main__":
    print("Meshes:")
    make_rows()
    make_fruit_piles()
    make_beam()
    make_dust()
    make_cartons()
    make_cat()
    print("Textures:")
    make_floor()
    make_ceiling()
    make_labels()
    make_price_strip()
    make_shadow()
    make_signs()
    make_reserve_textures()
