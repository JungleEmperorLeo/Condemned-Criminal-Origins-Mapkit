"""Condemned v113 <-> FEAR v113 container conversion (prototype, pure bytes).

FEAR whole world: 56-byte header (113, render/visibility/objects/blind offsets,
36 bytes of vectors) + sections tree..particles | render | visibility | objects | blind.
Condemned: WLDP (magic, 113, vectors, tree..particles, blind), WLDC (render,
visibility), WLDS (objects). Differences applied (FORMAT.md):
  * WorldModels 8 count words: FEAR stores value^399, Condemned plain values.
  * Streaming header: Condemned 12 words, FEAR 11 (the extra word is last).
The walker locates boundaries identically for both layouts.
"""
import struct

XOR = 399


class _R:
    def __init__(s, d, at):
        s.d, s.at = d, at

    def take(s, n):
        if n < 0 or s.at + n > len(s.d):
            raise ValueError(('truncated', s.at, n))
        s.at += n

    def u(s, n=1):
        if s.at + 4 * n > len(s.d):
            raise ValueError(('truncated', s.at))
        v = struct.unpack_from('<%dI' % n, s.d, s.at)
        s.at += 4 * n
        return v if n > 1 else v[0]


def _physics_body(r, kind, depth=0):
    if kind == 0:
        return
    if kind in (2, 3, 7):
        r.take({2: 24, 3: 12, 7: 36}[kind])
    elif kind in (1, 4):
        r.take(64)
        vertices, other = r.u(), r.u()
        r.take(vertices * 12)
        if kind == 1:
            r.take(other * (4 if vertices > 65535 else 2))
            r.take(16)
            r.take(r.u())
        else:
            r.take(other * 16)
    elif kind == 6:
        for _ in range(r.u()):
            m = r.u()
            if m == 5:
                r.take(28)
                m = r.u()
            _physics_body(r, m, depth + 1)
    else:
        raise ValueError(('physics kind', kind, r.at))


def _polygons(r, ordinary):
    count = r.u()
    if ordinary:
        r.u()
    for _ in range(count):
        r.take(16)
        r.take(r.u() * 12)
    if not ordinary and r.u() != 0:
        raise ValueError('particle terminator')


def walk_body(d, start, xor, stream_words):
    """Offsets inside tree..particles that begins at `start`."""
    r = _R(d, start)
    o = {'tree': r.at}
    r.take(24)
    nodes = r.u()
    r.u()
    r.take((nodes + 7) // 8)
    o['wm'] = r.at
    names, sb, nor, models = [v ^ xor for v in r.u(8)][:4]
    r.take(sb)
    r.take(names * 8)
    r.take(nor * 12)
    for _ in range(models):
        _, pt, po, _, nd = r.u(5)
        r.take(24)
        r.u()
        s = r.at
        r.take(po)
        for c in d[s:s + po]:
            r.take(12 + 4 * c)
        r.take(nd * 12)
        r.take(pt * 12)
    o['ordinary'] = r.at
    _polygons(r, True)
    o['physics'] = r.at
    for _ in range(r.u()):
        r.take(28)
        _physics_body(r, r.u())
    o['streaming'] = r.at
    h = r.u(stream_words)
    sb, regions, convex, tcount = h[0], h[3], h[5], h[8]
    r.take(sb)
    r.take(r.u() * 8)
    for _ in range(regions):
        r.take(8)
        r.take(r.u() * 4)
        r.take(r.u() * 8)
    for _ in range(convex):
        r.take(8)
        c = r.u()
        r.take(c * 16)
        for _ in range(c):
            r.take(r.u() * 12)
    for _ in range(tcount):
        r.take(r.u() * 4)
        r.take(16)
    o['particles'] = r.at
    _polygons(r, False)
    o['end'] = r.at
    return o


def condemned_to_fear(wldp, wldc, wlds):
    """Rebuild the FEAR single-file layout from a Condemned trio."""
    if wldp[:4] != b'WLDP' or wldc[:4] != b'WLDC' or wlds[:4] != b'WLDS':
        raise ValueError('magic')
    vectors = wldp[8:44]
    o = walk_body(wldp, 44, 0, 12)
    body = bytearray(wldp[44:o['end']])
    base = 44
    # re-apply the FEAR count obfuscation
    w = o['wm'] - base
    for i in range(8):
        struct.pack_into('<I', body, w + 4 * i, struct.unpack_from('<I', body, w + 4 * i)[0] ^ XOR)
    # drop the 12th streaming-header word
    s = o['streaming'] - base
    if struct.unpack_from('<I', body, s + 44)[0] != 0:
        raise ValueError('unexpected non-zero 12th streaming word')
    del body[s + 44:s + 48]
    blind = wldp[o['end']:]
    # render | visibility split inside WLDC: render ends at the visibility header
    vis = _vis_start(wldc)
    render, visibility = wldc[8:vis], wldc[vis:]
    objects = wlds[8:]
    sections = [bytes(body), render, visibility, objects, blind]
    off = 56 + len(sections[0])
    offs = []
    for sct in sections[1:]:
        offs.append(off)
        off += len(sct)
    return struct.pack('<5I', 113, *offs) + vectors + b''.join(sections)


def fear_to_condemned(fear):
    """Split a FEAR whole world into the Condemned trio (WLDP, WLDC, WLDS)."""
    ver, r_off, v_off, o_off, b_off = struct.unpack_from('<5I', fear)
    vectors = fear[20:56]
    body = bytearray(fear[56:r_off])
    o = walk_body(fear, 56, XOR, 11)
    w = o['wm'] - 56
    for i in range(8):
        struct.pack_into('<I', body, w + 4 * i, struct.unpack_from('<I', body, w + 4 * i)[0] ^ XOR)
    s = o['streaming'] - 56
    body[s + 44:s + 44] = b'\0\0\0\0'
    wldp = b'WLDP' + struct.pack('<I', 113) + vectors + bytes(body) + fear[b_off:]
    wldc = b'WLDC' + struct.pack('<I', 113) + fear[r_off:o_off]
    wlds = b'WLDS' + struct.pack('<I', 113) + fear[o_off:b_off]
    return wldp, wldc, wlds


def _vis_start(wldc):
    """Find where render ends in WLDC by walking the render grammar."""
    d = wldc
    at = 8

    def words(n):
        nonlocal at
        v = struct.unpack_from('<%dI' % n, d, at)
        at += 4 * n
        return v

    def take(n):
        nonlocal at
        if n < 0 or at + n > len(d):
            raise ValueError(('truncated', at))
        at += n

    def string():
        n, = struct.unpack_from('<H', d, at)
        take(2 + n)

    hdr = words(10)
    for _ in range(hdr[3]):
        kind, = words(1)
        if kind == 0:
            surfaces, materials, vb, ib = words(4)
            take(vb + ib)
            defs, = words(1)
            for _ in range(defs):
                size, = words(1)
                take(size)
            words(1)
            take(36 * surfaces)
            for _ in range(materials):
                string()
        else:
            mats, = words(1)
            string()
            words(1)
            for _ in range(mats):
                string()
    for _ in range(hdr[0]):
        c, = words(1)
        for _ in range(c):
            n, s = words(2)
            take(36 * n)
            for _ in range(s):
                take(1)
                v = d[at - 1]
                take(v * 12)
    return at
