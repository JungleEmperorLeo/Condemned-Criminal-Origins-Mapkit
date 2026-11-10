// CONDEMNED FORK: FEAR whole world -> Condemned v113 trio (WLDP/WLDC/WLDS).
// C++ port of condemned-worldpacker/src/condemned_container.py fear_to_condemned
// (round-trip verified on 10 retail maps, see FORMAT.md). No CRT; Arena allocation.
#pragma once
namespace CondemnedContainer {
struct Cursor {
  const unsigned char* d; unsigned long long size, at; bool bad;
  Cursor(const unsigned char* data, unsigned long long n, unsigned long long start) : d(data), size(n), at(start), bad(false) {}
  void Take(unsigned long long n) { if (bad || n > size - at) { bad = true; return; } at += n; }
  unsigned U() { if (bad || 4 > size - at) { bad = true; return 0; } unsigned v = d[at] | (d[at+1] << 8) | (d[at+2] << 16) | ((unsigned)d[at+3] << 24); at += 4; return v; }
};
inline void PhysicsBody(Cursor& r, unsigned kind, unsigned depth) {
  if (r.bad || depth > 64) { r.bad = true; return; }
  if (kind == 0) return;
  if (kind == 2) r.Take(24); else if (kind == 3) r.Take(12); else if (kind == 7) r.Take(36);
  else if (kind == 1 || kind == 4) {
    r.Take(64); unsigned vertices = r.U(), other = r.U(); r.Take((unsigned long long)vertices * 12);
    if (kind == 1) { r.Take((unsigned long long)other * (vertices > 65535 ? 4 : 2)); r.Take(16); r.Take(r.U()); }
    else r.Take((unsigned long long)other * 16);
  } else if (kind == 6) {
    unsigned n = r.U(); if (n > 100000) { r.bad = true; return; }
    for (unsigned i = 0; i < n && !r.bad; i++) { unsigned m = r.U(); if (m == 5) { r.Take(28); m = r.U(); } PhysicsBody(r, m, depth + 1); }
  } else r.bad = true;
}
// Offsets (absolute, in the FEAR file) of the WorldModels count words and the Streaming header.
inline bool Locate(const unsigned char* d, unsigned size, unsigned& wm, unsigned& streaming) {
  Cursor r(d, size, 56);
  r.Take(24); unsigned nodes = r.U(); r.U(); r.Take((nodes + 7ull) / 8);
  wm = (unsigned)r.at;
  unsigned names = r.U() ^ 399, sb = r.U() ^ 399, nor = r.U() ^ 399, models = r.U() ^ 399; r.Take(16);
  if (r.bad || models > 4096) return false;
  r.Take(sb); r.Take((unsigned long long)names * 8); r.Take((unsigned long long)nor * 12);
  for (unsigned m = 0; m < models && !r.bad; m++) {
    r.U(); unsigned pt = r.U(), po = r.U(); r.U(); unsigned nd = r.U(); r.Take(24); r.U();
    unsigned long long s = r.at; r.Take(po); if (r.bad) return false;
    for (unsigned long long i = 0; i < po; i++) r.Take(12 + 4ull * d[s + i]);
    r.Take((unsigned long long)nd * 12); r.Take((unsigned long long)pt * 12);
  }
  unsigned polygons = r.U(); r.U();
  for (unsigned i = 0; i < polygons && !r.bad; i++) { r.Take(16); r.Take((unsigned long long)r.U() * 12); }
  unsigned shapes = r.U(); if (shapes > 4096) return false;
  for (unsigned i = 0; i < shapes && !r.bad; i++) { r.Take(28); PhysicsBody(r, r.U(), 0); }
  if (r.bad) return false;
  streaming = (unsigned)r.at; return true;
}
struct Output { unsigned char* world; unsigned worldSize; unsigned char* client; unsigned clientSize; unsigned char* server; unsigned serverSize; };
inline void Put(unsigned char* p, unsigned v) { p[0] = (unsigned char)v; p[1] = (unsigned char)(v >> 8); p[2] = (unsigned char)(v >> 16); p[3] = (unsigned char)(v >> 24); }
inline unsigned Get(const unsigned char* p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((unsigned)p[3] << 24); }
template <class Arena> bool Split(const unsigned char* fear, unsigned size, Arena& arena, Output& out) {
  if (size < 56 || Get(fear) != 113) return false;
  unsigned r = Get(fear + 4), v = Get(fear + 8), o = Get(fear + 12), b = Get(fear + 16);
  if (!(56 <= r && r <= v && v <= o && o <= b && b <= size)) return false;
  unsigned wm, st; if (!Locate(fear, size, wm, st) || wm + 32 > r || st + 44 > r) return false;
  unsigned bodySize = r - 56, blindSize = size - b;
  out.worldSize = 44 + bodySize + 4 + blindSize; out.clientSize = 8 + (o - r); out.serverSize = 8 + (b - o);
  out.world = (unsigned char*)arena.Allocate(out.worldSize); out.client = (unsigned char*)arena.Allocate(out.clientSize); out.server = (unsigned char*)arena.Allocate(out.serverSize);
  if (!out.world || !out.client || !out.server) return false;
  unsigned char* w = out.world; w[0] = 'W'; w[1] = 'L'; w[2] = 'D'; w[3] = 'P'; Put(w + 4, 113);
  for (unsigned i = 0; i < 36; i++) w[8 + i] = fear[20 + i];
  unsigned char* body = w + 44; for (unsigned i = 0; i < bodySize; i++) body[i] = 0;
  unsigned split = st - 56 + 44;                       // streaming header end (11 words) inside the body
  for (unsigned i = 0; i < split; i++) body[i] = fear[56 + i];
  Put(body + split, 0);                                  // Condemned 12th streaming header word
  for (unsigned i = split; i < bodySize; i++) body[i + 4] = fear[56 + i];
  for (unsigned i = 0; i < 8; i++) Put(body + (wm - 56) + 4 * i, Get(fear + wm + 4 * i) ^ 399);
  unsigned char* tail = body + bodySize + 4; for (unsigned i = 0; i < blindSize; i++) tail[i] = fear[b + i];
  unsigned char* c = out.client; c[0] = 'W'; c[1] = 'L'; c[2] = 'D'; c[3] = 'C'; Put(c + 4, 113); for (unsigned i = 0; i < o - r; i++) c[8 + i] = fear[r + i];
  unsigned char* s = out.server; s[0] = 'W'; s[1] = 'L'; s[2] = 'D'; s[3] = 'S'; Put(s + 4, 113); for (unsigned i = 0; i < b - o; i++) s[8 + i] = fear[o + i];
  return true;
}
}
