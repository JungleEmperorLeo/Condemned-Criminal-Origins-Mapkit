//------------------------------------------------------------------
//
//	FILE	  : DDSLoad.h
//
//	PURPOSE	  : Decodes .dds textures to 32 bit BGRA so DEdit can show them.
//
//	            Handles DXT1 / DXT3 / DXT5 and uncompressed RGB(A), luminance
//	            and alpha-only formats, with mipmaps. Condemned and F.E.A.R.
//	            wrap their DDS data in a 12 byte header ("TEXR", u32 version,
//	            u32 flags); that wrapper is skipped, plain .dds files work too.
//	            For cube maps only the first face of each mip level is kept
//	            (this is for previews, not for rendering the cube).
//
//	            No dependencies beyond the C++ standard library.
//
//------------------------------------------------------------------

#ifndef __DDSLOAD_H__
#define __DDSLOAD_H__

#include <string>
#include <vector>
#include <stdint.h>

namespace DDS
{
	struct Image
	{
		Image() : width(0), height(0), isCube(false), hasAlpha(false) {}

		uint32_t	width, height;		// size of mip 0
		bool		isCube;
		bool		hasAlpha;			// the format carries an alpha channel
		// One entry per mip level. Level i is max(1, width >> i) by max(1, height >> i)
		// pixels, 4 bytes per pixel in B, G, R, A order, no row padding.
		std::vector< std::vector<uint8_t> >	mips;

		uint32_t MipWidth(size_t i) const	{ uint32_t v = width >> i;  return v ? v : 1; }
		uint32_t MipHeight(size_t i) const	{ uint32_t v = height >> i; return v ? v : 1; }
	};

	// pMaxMips: stop after this many levels (0 = all of them).
	bool Load(const void *pData, size_t nSize, Image &out, std::string *pError = 0, uint32_t nMaxMips = 0);
	bool LoadFile(const char *pFilename, Image &out, std::string *pError = 0, uint32_t nMaxMips = 0);
}

#endif // __DDSLOAD_H__
