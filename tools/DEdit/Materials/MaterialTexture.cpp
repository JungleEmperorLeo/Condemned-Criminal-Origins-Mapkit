//------------------------------------------------------------------
//
//	FILE	  : MaterialTexture.cpp
//
//	PURPOSE	  : See MaterialTexture.h
//
//------------------------------------------------------------------

#include "bdefs.h"
#include "dtxmgr.h"
#include "streamsim.h"
#include "MaterialTexture.h"
#include "DDSLoad.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include <math.h>
#include <string>
#include <vector>

static std::string g_ResourceRoot;

void MaterialTexture_SetResourceRoot(const char *pRoot)
{
	g_ResourceRoot = pRoot ? pRoot : "";
	{
		char full[MAX_PATH];
		if(!g_ResourceRoot.empty() && _fullpath(full, g_ResourceRoot.c_str(), MAX_PATH))
			g_ResourceRoot = full;
	}
	while(!g_ResourceRoot.empty() && (g_ResourceRoot[g_ResourceRoot.size()-1] == '\\' || g_ResourceRoot[g_ResourceRoot.size()-1] == '/'))
		g_ResourceRoot.erase(g_ResourceRoot.size()-1);
}

bool MaterialTexture_IsMaterialName(const char *pFilename)
{
	if(!pFilename)
		return false;
	size_t n = strlen(pFilename);
	if(n < 6)
		return false;
	const char *e = pFilename + n - 6;
	return (e[0]=='.') && (e[1]=='M'||e[1]=='m') && (e[2]=='A'||e[2]=='a') && (e[3]=='T'||e[3]=='t') && e[4]=='0' && e[5]=='0';
}

static bool FileExists(const std::string &p)
{
	FILE *f = fopen(p.c_str(), "rb");
	if(!f)
		return false;
	fclose(f);
	return true;
}

static std::string Join(const std::string &dir, const char *rel)
{
	std::string r = rel;
	while(!r.empty() && (r[0] == '\\' || r[0] == '/'))
		r.erase(0, 1);
	if(dir.empty())
		return r;
	return dir + "\\" + r;
}

bool MaterialTexture_ResolveTexture(const char *pMaterialFile, const char *pTexturePath, std::string &fullPath)
{
	if(!pTexturePath || !pTexturePath[0])
		return false;

	if(!g_ResourceRoot.empty())
	{
		std::string p = Join(g_ResourceRoot, pTexturePath);
		if(FileExists(p)) { fullPath = p; return true; }
	}

	// Walk up from the material's folder.
	if(pMaterialFile)
	{
		std::string dir = pMaterialFile;
		for(;;)
		{
			size_t cut = dir.find_last_of("\\/");
			if(cut == std::string::npos)
				break;
			dir.erase(cut);
			if(dir.empty())
				break;
			std::string p = Join(dir, pTexturePath);
			if(FileExists(p)) { fullPath = p; return true; }
		}
	}
	return false;
}

std::string MaterialTexture_PreviewParam(const MatFile::Material &mat)
{
	static const char *kPreferred[] = {
		"tDiffuseMap", "tDEditMap", "tBaseMap", "tEmissiveMap", "tSkinMap", "tTerrainDiffuseMap", "tDiffuse", 0 };

	for(int i = 0; kPreferred[i]; i++)
	{
		const MatFile::Param *p = mat.Find(kPreferred[i]);
		if(p && p->type == MatFile::PT_TEXTURE && !p->str.empty())
			return p->name;
	}
	for(size_t i = 0; i < mat.params.size(); i++)
	{
		const MatFile::Param &p = mat.params[i];
		if(p.type == MatFile::PT_TEXTURE && !p.str.empty() && p.name.find("Normal") == std::string::npos
			&& p.name.find("Bump") == std::string::npos)
			return p.name;
	}
	for(size_t i = 0; i < mat.params.size(); i++)
		if(mat.params[i].type == MatFile::PT_TEXTURE && !mat.params[i].str.empty())
			return mat.params[i].name;
	return std::string();
}

//--------------------------------------------------------------------
// Image helpers
//--------------------------------------------------------------------

static uint32 NearestValidSize(uint32 v)
{
	uint32 best = 8;
	uint32 bestDiff = 0xFFFFFFFF;
	for(uint32 s = 8; s <= MAX_DTX_SIZE; s <<= 1)
	{
		uint32 d = v > s ? v - s : s - v;
		if(d < bestDiff) { bestDiff = d; best = s; }
	}
	return best;
}

static uint32 Log2Floor(uint32 v)
{
	uint32 r = 0;
	while(v > 1) { v >>= 1; r++; }
	return r;
}

// Nearest neighbour resample of BGRA pixels.
static void Resample(const uint8 *pSrc, uint32 sw, uint32 sh, uint8 *pDst, uint32 dw, uint32 dh)
{
	for(uint32 y = 0; y < dh; y++)
	{
		uint32 sy = (uint32)(((uint64_t)y * sh) / dh);
		for(uint32 x = 0; x < dw; x++)
		{
			uint32 sx = (uint32)(((uint64_t)x * sw) / dw);
			memcpy(pDst + ((size_t)y * dw + x) * 4, pSrc + ((size_t)sy * sw + sx) * 4, 4);
		}
	}
}

// Dull checkerboard used when a material has no usable texture, so it still shows up.
static void MakePlaceholder(std::vector<uint8> &px, uint32 &w, uint32 &h)
{
	w = h = 64;
	px.resize(w * h * 4);
	for(uint32 y = 0; y < h; y++)
		for(uint32 x = 0; x < w; x++)
		{
			bool on = (((x >> 3) + (y >> 3)) & 1) != 0;
			uint8 *p = &px[(y * w + x) * 4];
			p[0] = on ? 0xA0 : 0x60;	// B
			p[1] = on ? 0x70 : 0x40;	// G
			p[2] = on ? 0xA0 : 0x60;	// R
			p[3] = 0xFF;
		}
}

// Reads whole file into memory.
static bool ReadAll(const std::string &path, std::vector<uint8> &out)
{
	FILE *f = fopen(path.c_str(), "rb");
	if(!f)
		return false;
	uint8 tmp[8192];
	size_t got;
	out.clear();
	while((got = fread(tmp, 1, sizeof(tmp), f)) > 0)
		out.insert(out.end(), tmp, tmp + got);
	fclose(f);
	return true;
}

#include <map>

void MaterialTexture_GetTileSize(const char *pMaterialRelName, float &w, float &h)
{
	static std::map<std::string, std::pair<float, float> > cache;
	w = h = 100.0f;
	if(!pMaterialRelName || !pMaterialRelName[0])
		return;

	std::string key = pMaterialRelName;
	std::map<std::string, std::pair<float, float> >::iterator it = cache.find(key);
	if(it != cache.end())
	{
		w = it->second.first;
		h = it->second.second;
		return;
	}

	std::vector<uint8> bytes;
	MatFile::Material mat;
	if(ReadAll(Join(g_ResourceRoot, pMaterialRelName), bytes) && !bytes.empty() &&
		MatFile::Parse(&bytes[0], bytes.size(), mat))
	{
		float fw = mat.GetFloat("DefaultWidth", 0.0f);
		float fh = mat.GetFloat("DefaultHeight", 0.0f);
		if(fw >= 1.0f) w = fw;
		if(fh >= 1.0f) h = fh;
	}
	cache[key] = std::make_pair(w, h);
}

//--------------------------------------------------------------------
// The hook
//--------------------------------------------------------------------

// Opens a path as given, else relative to the project root (the hook may receive either).
static bool ReadAllResolved(const char *pPath, std::vector<uint8> &out, std::string *pUsed = 0)
{
	if(!pPath || !pPath[0])
		return false;
	std::string p = pPath;
	if(ReadAll(p, out)) { if(pUsed) *pUsed = p; return true; }
	if(!g_ResourceRoot.empty())
	{
		p = Join(g_ResourceRoot, pPath);
		if(ReadAll(p, out)) { if(pUsed) *pUsed = p; return true; }
	}
	return false;
}

static bool IsDdsName(const char *f)
{
	size_t n = f ? strlen(f) : 0;
	return n > 4 && _stricmp(f + n - 4, ".dds") == 0;
}

static ILTStream* MaterialOpenHook(const char *pFilename)
{
	bool isDds = IsDdsName(pFilename);
	MatFile::Material mat;
	std::string matUsed;
	if(!isDds)
	{
		if(!MaterialTexture_IsMaterialName(pFilename))
			return 0;
		std::vector<uint8> matBytes;
		if(!ReadAllResolved(pFilename, matBytes, &matUsed) || matBytes.empty())
			return 0;
		if(!MatFile::Parse(&matBytes[0], matBytes.size(), mat))
			return 0;
	}

	std::vector<uint8> pixels;
	uint32 pw = 0, ph = 0;
	uint32 uiOffset = 0;

	std::string param = isDds ? std::string() : MaterialTexture_PreviewParam(mat);
	bool gotImage = false;

	if(isDds || !param.empty())
	{
		std::string texFile;
		if(isDds ? (texFile = pFilename, true) : MaterialTexture_ResolveTexture(matUsed.c_str(), mat.GetTexture(param.c_str()).c_str(), texFile))
		{
			std::vector<uint8> ddsBytes;
			DDS::Image img;
			if(ReadAllResolved(texFile.c_str(), ddsBytes) && !ddsBytes.empty() && DDS::Load(&ddsBytes[0], ddsBytes.size(), img) && !img.mips.empty())
			{
				// Skip top mips until it fits.
				size_t level = 0;
				while(level + 1 < img.mips.size() && (img.MipWidth(level) > MAX_DTX_SIZE || img.MipHeight(level) > MAX_DTX_SIZE))
					level++;

				uint32 sw = img.MipWidth(level), sh = img.MipHeight(level);
				pw = NearestValidSize(sw);
				ph = NearestValidSize(sh);
				pixels.resize((size_t)pw * ph * 4);
				if(sw == pw && sh == ph)
					memcpy(&pixels[0], &img.mips[level][0], pixels.size());
				else
					Resample(&img.mips[level][0], sw, sh, &pixels[0], pw, ph);
				gotImage = true;
			}
		}
	}

	if(!gotImage)
		MakePlaceholder(pixels, pw, ph);

	// Pixel size vs. the size the material says it should map at.
	float defW = mat.GetFloat("DefaultWidth", 0.0f);
	if(!isDds && gotImage && defW >= 1.0f)
	{
		double ratio = (double)pw / (double)defW;
		int off = (int)floor(log(ratio) / log(2.0) + 0.5);
		if(off < 0) off = 0;
		if(off > 7) off = 7;
		uiOffset = (uint32)off;
	}

	uint32 minSide = pw < ph ? pw : ph;
	uint32 nMips = Log2Floor(minSide) + 1;
	if(nMips > MAX_DTX_MIPMAPS)
		nMips = MAX_DTX_MIPMAPS;

	uint32 allocSize = 0, dataSize = 0;
	TextureData *pTex = dtx_Alloc(BPP_32, pw, ph, nMips, &allocSize, &dataSize);
	if(!pTex)
		return 0;

	memcpy(pTex->m_Mips[0].m_Data, &pixels[0], (size_t)pw * ph * 4);
	pTex->m_Header.m_Extra[1] = (uint8)nMips;
	pTex->m_Header.SetUIMipmapOffset(uiOffset);
	if(!isDds)
	{
		float tw = mat.GetFloat("DefaultWidth", 0.0f);
		float th = mat.GetFloat("DefaultHeight", 0.0f);
		if(tw < 1.0f) tw = 100.0f;
		if(th < 1.0f) th = 100.0f;
		uint32 iw = (uint32)(tw + 0.5f), ih = (uint32)(th + 0.5f);
		if(iw > 65535) iw = 65535;
		if(ih > 65535) ih = 65535;
		pTex->m_Header.m_Extra[6] = (uint8)iw; pTex->m_Header.m_Extra[7] = (uint8)(iw >> 8);
		pTex->m_Header.m_Extra[8] = (uint8)ih; pTex->m_Header.m_Extra[9] = (uint8)(ih >> 8);
	}
	dtx_BuildMipmaps(pTex);

	ILTStream *pStream = streamsim_OpenMemStream();
	if(!pStream)
	{
		dtx_Destroy(pTex);
		return 0;
	}

	if(dtx_Save(pTex, pStream) != LT_OK)
	{
		dtx_Destroy(pTex);
		pStream->Release();
		return 0;
	}
	dtx_Destroy(pTex);

	pStream->SeekTo(0);
	return pStream;
}

void MaterialTexture_Install()
{
	streamsim_SetOpenHook(MaterialOpenHook);
}
