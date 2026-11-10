//------------------------------------------------------------------
//
//	FILE	  : DDSLoad.cpp
//
//	PURPOSE	  : See DDSLoad.h
//
//------------------------------------------------------------------

#include "DDSLoad.h"

#include <string.h>
#include <stdio.h>

namespace DDS
{

namespace
{
	const uint32_t DDSD_MIPMAPCOUNT		= 0x00020000;

	const uint32_t DDPF_ALPHAPIXELS		= 0x00000001;
	const uint32_t DDPF_ALPHA			= 0x00000002;
	const uint32_t DDPF_FOURCC			= 0x00000004;
	const uint32_t DDPF_RGB				= 0x00000040;
	const uint32_t DDPF_LUMINANCE		= 0x00020000;

	const uint32_t DDSCAPS2_CUBEMAP		= 0x00000200;
	const uint32_t DDSCAPS2_VOLUME		= 0x00200000;

	inline uint32_t Rd32(const uint8_t *p)
	{
		return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
	}

	inline uint32_t Rd16(const uint8_t *p)
	{
		return (uint32_t)p[0] | ((uint32_t)p[1] << 8);
	}

	inline uint32_t MakeFourCC(char a, char b, char c, char d)
	{
		return (uint32_t)(uint8_t)a | ((uint32_t)(uint8_t)b << 8) | ((uint32_t)(uint8_t)c << 16) | ((uint32_t)(uint8_t)d << 24);
	}

	bool Fail(std::string *pError, const char *pMsg)
	{
		if(pError)
			*pError = pMsg;
		return false;
	}

	enum Format
	{
		F_DXT1, F_DXT3, F_DXT5,
		F_MASKS,		// RGB(A) described by bit masks
		F_LUMINANCE,
		F_ALPHA
	};

	struct Masks
	{
		uint32_t r, g, b, a;
		uint32_t rShift, gShift, bShift, aShift;
		uint32_t rBits, gBits, bBits, aBits;
	};

	void SetupChannel(uint32_t mask, uint32_t &shift, uint32_t &bits)
	{
		shift = 0; bits = 0;
		if(!mask)
			return;
		while(!((mask >> shift) & 1))
			shift++;
		while((mask >> (shift + bits)) & 1)
			bits++;
	}

	// Expands a channel of "bits" bits to 8 bits.
	inline uint8_t Expand(uint32_t v, uint32_t bits)
	{
		if(bits == 0)	return 0;
		if(bits >= 8)	return (uint8_t)(v >> (bits - 8));
		// replicate the top bits so 0 -> 0 and max -> 255
		uint32_t r = v << (8 - bits);
		for(uint32_t s = bits; s < 8; s <<= 1)
			r |= r >> s;
		return (uint8_t)r;
	}

	inline void Put565(uint16_t c, uint8_t *rgb)
	{
		uint32_t r = (c >> 11) & 31, g = (c >> 5) & 63, b = c & 31;
		rgb[0] = (uint8_t)((r << 3) | (r >> 2));
		rgb[1] = (uint8_t)((g << 2) | (g >> 4));
		rgb[2] = (uint8_t)((b << 3) | (b >> 2));
	}

	// Decodes one 4x4 colour block (the 8 byte DXT1 colour part) into out[16][4] (B,G,R,A).
	// allowAlpha: DXT1 style, c0 <= c1 selects the 3 colour + transparent mode.
	void DecodeColorBlock(const uint8_t *src, uint8_t out[16][4], bool allowAlpha)
	{
		uint16_t c0 = (uint16_t)Rd16(src), c1 = (uint16_t)Rd16(src + 2);
		uint8_t col[4][4];	// R, G, B, A
		Put565(c0, col[0]); col[0][3] = 255;
		Put565(c1, col[1]); col[1][3] = 255;

		if(c0 > c1 || !allowAlpha)
		{
			for(int i = 0; i < 3; i++)
			{
				col[2][i] = (uint8_t)((2 * col[0][i] + col[1][i]) / 3);
				col[3][i] = (uint8_t)((col[0][i] + 2 * col[1][i]) / 3);
			}
			col[2][3] = 255; col[3][3] = 255;
		}
		else
		{
			for(int i = 0; i < 3; i++)
			{
				col[2][i] = (uint8_t)((col[0][i] + col[1][i]) / 2);
				col[3][i] = 0;
			}
			col[2][3] = 255; col[3][3] = 0;
		}

		uint32_t idx = Rd32(src + 4);
		for(int i = 0; i < 16; i++)
		{
			const uint8_t *c = col[(idx >> (2 * i)) & 3];
			out[i][0] = c[2]; out[i][1] = c[1]; out[i][2] = c[0]; out[i][3] = c[3];
		}
	}

	void DecodeDXT(Format fmt, const uint8_t *src, uint32_t w, uint32_t h, uint8_t *dst)
	{
		uint32_t bw = (w + 3) / 4, bh = (h + 3) / 4;
		uint32_t blockSize = (fmt == F_DXT1) ? 8 : 16;

		for(uint32_t by = 0; by < bh; by++)
		{
			for(uint32_t bx = 0; bx < bw; bx++)
			{
				const uint8_t *blk = src + (by * bw + bx) * blockSize;
				uint8_t px[16][4];

				if(fmt == F_DXT1)
				{
					DecodeColorBlock(blk, px, true);
				}
				else
				{
					DecodeColorBlock(blk + 8, px, false);

					if(fmt == F_DXT3)
					{
						// 4 bits of explicit alpha per pixel
						for(int i = 0; i < 16; i++)
						{
							uint32_t a = (blk[i >> 1] >> ((i & 1) * 4)) & 15;
							px[i][3] = (uint8_t)((a << 4) | a);
						}
					}
					else
					{
						// DXT5: two end points and 3 bit interpolation indices
						uint32_t a0 = blk[0], a1 = blk[1];
						uint8_t al[8];
						al[0] = (uint8_t)a0; al[1] = (uint8_t)a1;
						if(a0 > a1)
						{
							for(uint32_t i = 1; i < 7; i++)
								al[1 + i] = (uint8_t)(((7 - i) * a0 + i * a1) / 7);
						}
						else
						{
							for(uint32_t i = 1; i < 5; i++)
								al[1 + i] = (uint8_t)(((5 - i) * a0 + i * a1) / 5);
							al[6] = 0; al[7] = 255;
						}

						uint64_t bits = 0;
						for(int i = 0; i < 6; i++)
							bits |= (uint64_t)blk[2 + i] << (8 * i);
						for(int i = 0; i < 16; i++)
							px[i][3] = al[(bits >> (3 * i)) & 7];
					}
				}

				for(uint32_t y = 0; y < 4; y++)
				{
					if(by * 4 + y >= h)
						break;
					for(uint32_t x = 0; x < 4; x++)
					{
						if(bx * 4 + x >= w)
							break;
						uint8_t *d = dst + ((by * 4 + y) * w + bx * 4 + x) * 4;
						memcpy(d, px[y * 4 + x], 4);
					}
				}
			}
		}
	}

	void DecodeMasks(const Masks &m, uint32_t bytesPerPixel, bool hasAlpha, const uint8_t *src, uint32_t w, uint32_t h, uint8_t *dst)
	{
		for(uint32_t i = 0; i < w * h; i++)
		{
			uint32_t v = 0;
			for(uint32_t k = 0; k < bytesPerPixel; k++)
				v |= (uint32_t)src[i * bytesPerPixel + k] << (8 * k);

			dst[i * 4 + 0] = Expand((v & m.b) >> m.bShift, m.bBits);
			dst[i * 4 + 1] = Expand((v & m.g) >> m.gShift, m.gBits);
			dst[i * 4 + 2] = Expand((v & m.r) >> m.rShift, m.rBits);
			dst[i * 4 + 3] = hasAlpha ? Expand((v & m.a) >> m.aShift, m.aBits) : 255;
		}
	}

	void DecodeLuminance(uint32_t bytesPerPixel, uint32_t lumMask, uint32_t alphaMask, const uint8_t *src, uint32_t w, uint32_t h, uint8_t *dst)
	{
		uint32_t ls, lb, as, ab;
		SetupChannel(lumMask, ls, lb);
		SetupChannel(alphaMask, as, ab);

		for(uint32_t i = 0; i < w * h; i++)
		{
			uint32_t v = 0;
			for(uint32_t k = 0; k < bytesPerPixel; k++)
				v |= (uint32_t)src[i * bytesPerPixel + k] << (8 * k);

			uint8_t l = Expand((v & lumMask) >> ls, lb);
			dst[i * 4 + 0] = l; dst[i * 4 + 1] = l; dst[i * 4 + 2] = l;
			dst[i * 4 + 3] = alphaMask ? Expand((v & alphaMask) >> as, ab) : 255;
		}
	}

	void DecodeAlphaOnly(const uint8_t *src, uint32_t w, uint32_t h, uint8_t *dst)
	{
		for(uint32_t i = 0; i < w * h; i++)
		{
			dst[i * 4 + 0] = 255; dst[i * 4 + 1] = 255; dst[i * 4 + 2] = 255;
			dst[i * 4 + 3] = src[i];
		}
	}
}

bool Load(const void *pData, size_t nSize, Image &out, std::string *pError, uint32_t nMaxMips)
{
	out = Image();

	const uint8_t *p = (const uint8_t*)pData;
	size_t size = nSize;

	// Condemned / F.E.A.R. wrapper: "TEXR", version, flags, then a normal .dds
	if(size >= 12 && memcmp(p, "TEXR", 4) == 0)
	{
		p += 12;
		size -= 12;
	}

	if(size < 128 || memcmp(p, "DDS ", 4) != 0)
		return Fail(pError, "not a DDS texture");

	const uint8_t *hdr = p + 4;
	if(Rd32(hdr) != 124)
		return Fail(pError, "bad DDS header size");

	uint32_t flags		= Rd32(hdr + 4);
	uint32_t height		= Rd32(hdr + 8);
	uint32_t width		= Rd32(hdr + 12);
	uint32_t mipCount	= (flags & DDSD_MIPMAPCOUNT) ? Rd32(hdr + 24) : 1;
	uint32_t pfFlags	= Rd32(hdr + 76);
	uint32_t fourCC		= Rd32(hdr + 80);
	uint32_t bitCount	= Rd32(hdr + 84);
	uint32_t caps2		= Rd32(hdr + 104);

	if(width == 0 || height == 0 || width > 16384 || height > 16384)
		return Fail(pError, "bad DDS dimensions");
	if(caps2 & DDSCAPS2_VOLUME)
		return Fail(pError, "volume textures are not supported");
	if(mipCount == 0)
		mipCount = 1;
	if(mipCount > 15)
		return Fail(pError, "bad DDS mip count");

	Format fmt;
	Masks m;
	memset(&m, 0, sizeof(m));
	bool hasAlpha = false;
	uint32_t bytesPerPixel = 0;
	uint32_t blockBytes = 0;

	if(pfFlags & DDPF_FOURCC)
	{
		if(fourCC == MakeFourCC('D','X','T','1'))		{ fmt = F_DXT1; blockBytes = 8;  hasAlpha = true; }
		else if(fourCC == MakeFourCC('D','X','T','3'))	{ fmt = F_DXT3; blockBytes = 16; hasAlpha = true; }
		else if(fourCC == MakeFourCC('D','X','T','5'))	{ fmt = F_DXT5; blockBytes = 16; hasAlpha = true; }
		else
			return Fail(pError, "unsupported DDS compression (only DXT1, DXT3 and DXT5)");
	}
	else if(pfFlags & DDPF_RGB)
	{
		if(bitCount != 16 && bitCount != 24 && bitCount != 32)
			return Fail(pError, "unsupported DDS bit depth");
		fmt = F_MASKS;
		bytesPerPixel = bitCount / 8;
		m.r = Rd32(hdr + 88); m.g = Rd32(hdr + 92); m.b = Rd32(hdr + 96); m.a = Rd32(hdr + 100);
		hasAlpha = (pfFlags & DDPF_ALPHAPIXELS) != 0 && m.a != 0;
		SetupChannel(m.r, m.rShift, m.rBits);
		SetupChannel(m.g, m.gShift, m.gBits);
		SetupChannel(m.b, m.bShift, m.bBits);
		SetupChannel(m.a, m.aShift, m.aBits);
	}
	else if(pfFlags & DDPF_LUMINANCE)
	{
		if(bitCount != 8 && bitCount != 16)
			return Fail(pError, "unsupported DDS bit depth");
		fmt = F_LUMINANCE;
		bytesPerPixel = bitCount / 8;
		m.r = Rd32(hdr + 88);	// luminance mask
		m.a = Rd32(hdr + 100);
		hasAlpha = (pfFlags & DDPF_ALPHAPIXELS) != 0 && m.a != 0;
		if(!hasAlpha)
			m.a = 0;
	}
	else if((pfFlags & DDPF_ALPHA) && bitCount == 8)
	{
		fmt = F_ALPHA;
		bytesPerPixel = 1;
		hasAlpha = true;
	}
	else
		return Fail(pError, "unsupported DDS pixel format");

	out.width = width;
	out.height = height;
	out.isCube = (caps2 & DDSCAPS2_CUBEMAP) != 0;
	out.hasAlpha = hasAlpha;

	// Faces are stored one after the other, each holding its whole mip chain. Only the
	// first face is decoded, which is the start of the pixel data.
	const uint8_t *data = p + 128;
	size_t remaining = size - 128;

	uint32_t levels = mipCount;
	if(nMaxMips && levels > nMaxMips)
		levels = nMaxMips;

	for(uint32_t lvl = 0; lvl < levels; lvl++)
	{
		uint32_t w = width >> lvl;  if(!w) w = 1;
		uint32_t h = height >> lvl; if(!h) h = 1;

		size_t srcBytes;
		if(blockBytes)
			srcBytes = (size_t)((w + 3) / 4) * ((h + 3) / 4) * blockBytes;
		else
			srcBytes = (size_t)w * h * bytesPerPixel;

		if(srcBytes > remaining)
		{
			if(lvl == 0)
				return Fail(pError, "DDS file is truncated");
			break;	// keep the levels we have
		}

		std::vector<uint8_t> px((size_t)w * h * 4);
		switch(fmt)
		{
		case F_DXT1: case F_DXT3: case F_DXT5:	DecodeDXT(fmt, data, w, h, &px[0]);	break;
		case F_MASKS:		DecodeMasks(m, bytesPerPixel, hasAlpha, data, w, h, &px[0]);	break;
		case F_LUMINANCE:	DecodeLuminance(bytesPerPixel, m.r, m.a, data, w, h, &px[0]);	break;
		case F_ALPHA:		DecodeAlphaOnly(data, w, h, &px[0]);	break;
		}

		out.mips.push_back(px);
		data += srcBytes;
		remaining -= srcBytes;
	}

	return true;
}

bool LoadFile(const char *pFilename, Image &out, std::string *pError, uint32_t nMaxMips)
{
	FILE *f = fopen(pFilename, "rb");
	if(!f)
		return Fail(pError, "unable to open the texture file");

	std::vector<uint8_t> buf;
	uint8_t tmp[16384];
	size_t got;
	while((got = fread(tmp, 1, sizeof(tmp), f)) > 0)
	{
		buf.insert(buf.end(), tmp, tmp + got);
		if(buf.size() > (256u << 20))
			break;
	}
	fclose(f);

	if(buf.empty())
		return Fail(pError, "the texture file is empty");
	return Load(&buf[0], buf.size(), out, pError, nMaxMips);
}

} // namespace DDS
