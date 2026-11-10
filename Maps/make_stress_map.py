"""Stress map: a floor, 36 sectors and N small separate boxes (8 physics vertices each) so the physics mesh exceeds 65535
vertices (Condemned uses 32-bit mesh indices there). usage: make_stress_map.py <out.World00a> <N>"""
import sys, pathlib, importlib.util, random
random.seed(7)
here = pathlib.Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location('mrm', here / 'make_room_map.py'); m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)
n = int(sys.argv[2]); side = int(n ** 0.5) + 1
solid = [('Floor', (-100, side * 40 + 100, -32, 0, -100, side * 40 + 100), m.FLOOR)]
for i in range(n):
    x = (i % side) * 40 + random.randint(0, 11); z = (i // side) * 40 + random.randint(0, 11)
    w = random.randint(12, 24); d = random.randint(12, 24); h = random.randint(10, 60)
    solid.append(('Box%05d' % i, (x, x + w, 0, h, z, z + d), m.WALL))
m.solid[:] = solid
tiles = 6; span = (side * 40 + 200) / tiles
m.SECTOR = ('SectorA', (-100, -100 + span - 2, 0, 300, -100, -100 + span - 2), '')
m.EXTRA_SECTORS[:] = [('Sector%d_%d' % (a, b), (-100 + a * span, -100 + (a + 1) * span - 2, 0, 300, -100 + b * span, -100 + (b + 1) * span - 2))
                      for a in range(tiles) for b in range(tiles) if (a, b) != (0, 0)]
p = pathlib.Path(sys.argv[1]); p.write_text(m.build(p.stem), encoding='ascii', newline='\n'); print(p, p.stat().st_size, 'boxes', n)
