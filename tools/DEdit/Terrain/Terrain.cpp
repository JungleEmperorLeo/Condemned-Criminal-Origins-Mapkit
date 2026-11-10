//------------------------------------------------------------------
//
//	FILE	  : Terrain.cpp
//
//	PURPOSE	  : Paintable terrain (see Terrain.h).
//
//	Layout of a terrain with N cells per side over the footprint
//	(x0, z0) - (x0 + sx, z0 + sz):
//	  - grid vertex (i, j), 0 <= i, j <= N, sits at
//	        x = x0 + sx * i / N,  z = z0 + sz * j / N
//	  - cell (i, j) holds two prisms, (i,j)(i+1,j)(i+1,j+1) and
//	    (i,j)(i+1,j+1)(i,j+1). Each prism brush has 6 points: 0-2 are
//	    the top (painted heights), 3-5 the flat base at baseY.
//	  - the brushes are found again by their top points, so undo and
//	    reordering in the node tree don't matter.
//
//------------------------------------------------------------------

#include "bdefs.h"
#include "dedit.h"
#include "resource.h"
#include "Terrain.h"
#include "regiondoc.h"
#include "regionview.h"
#include "projectbar.h"
#include "edithelpers.h"
#include "editregion.h"
#include "editbrush.h"
#include "editpoly.h"
#include "worldnode.h"
#include "node_ops.h"
#include "edit_actions.h"
#include "nodeview.h"
#include "texture.h"
#include "dtxmgr.h"
#include "sysstreamsim.h"
#include "pixelformat.h"
#include "d_filemgr.h"
#include "editprojectmgr.h"
#include "drawbase.h"

#include "../Materials/MaterialFile.h"
#include "../Materials/MaterialTexture.h"
#include <vector>
#include <list>
#include <map>
#include <string>
#include <algorithm>
#include <math.h>
#include <ctype.h>
#include <stdio.h>

extern FormatMgr g_FormatMgr;

#define TERRAIN_TEX_SIZE	1024
#define TERRAIN_NUM_LAYERS	4
#define TERRAIN_MIN_THICK	1.0f


//------------------------------------------------------------------
// Settings shared by the tool window and the painting code.
//------------------------------------------------------------------

static int		g_Mode		= Terrain::MODE_RAISE;
static float	g_Radius	= 128.0f;
static int		g_Strength	= 25;		// 1 - 100
static bool		g_bPaint	= true;
static CString	g_Layers[TERRAIN_NUM_LAYERS];

// Reticle position under the mouse.
static bool			g_bHover = false;
static CWorldNode	*g_pHoverNode = NULL;
static LTVector		g_vHover(0.0f, 0.0f, 0.0f);



//------------------------------------------------------------------
// Terrain data
//------------------------------------------------------------------

struct TerrainInfo
{
	int					m_N;
	float				m_x0, m_z0, m_sx, m_sz, m_BaseY;
	std::string			m_Layers[TERRAIN_NUM_LAYERS];
	std::vector<float>	m_Weights;	// (N+1)^2 * 4
	// Paint map: layer weights at a fine resolution (independent of the height grid),
	// m_PW * m_PH texels * 4 layers, 0..255. Saved as NNNN.wgt next to the .trn.
	int					m_PW, m_PH;
	std::vector<uint8>	m_Paint;

	TerrainInfo() : m_N(1), m_x0(0), m_z0(0), m_sx(1), m_sz(1), m_BaseY(0), m_PW(0), m_PH(0) {}

	int		NumVerts() const	{ return (m_N + 1) * (m_N + 1); }
	float	GX(int i) const		{ return m_x0 + m_sx * (float)i / (float)m_N; }
	float	GZ(int j) const		{ return m_z0 + m_sz * (float)j / (float)m_N; }

	// Grid index of a world position (rounded), -1 if it's off the grid.
	int GridIndex(float x, float z) const
	{
		int i = (int)floor((x - m_x0) / m_sx * m_N + 0.5f);
		int j = (int)floor((z - m_z0) / m_sz * m_N + 0.5f);
		if(i < 0 || j < 0 || i > m_N || j > m_N)
			return -1;
		return j * (m_N + 1) + i;
	}
};

struct TerrainRef
{
	CWorldNode					*m_pNode;
	int							m_ID;
	TerrainInfo					m_Info;
	std::vector<CEditBrush*>	m_Brushes;
	std::vector<float>			m_Heights;
};


static CString Terrain_ProjectDir()
{
	CString dir = GetProject()->m_BaseProjectDir;
	if(dir.GetLength() > 0 && dir[dir.GetLength()-1] != '\\' && dir[dir.GetLength()-1] != '/')
		dir += "\\";
	return dir;
}

static CString Terrain_TextureName(int id)
{
	CString s;
	s.Format("Textures\\Terrainmesh\\%04d.Mat00", id);
	return s;
}

// The baked colour texture (Condemned .dds) the terrain material points at.
static CString Terrain_DdsName(int id)
{
	CString s;
	s.Format("Textures\\Terrainmesh\\%04d.dds", id);
	return s;
}

bool ConvertDtxFileToCondemnedDds(const char *pFullPath, bool *pWasDtx);
bool WriteCondemnedDdsBGRA(const char *pPath, const unsigned char *pBGRA, int w, int h);

static CString Terrain_SidecarPath(int id)
{
	CString s;
	s.Format("%sTextures\\Terrainmesh\\%04d.trn", (LPCTSTR)Terrain_ProjectDir(), id);
	return s;
}

static void Terrain_MakeDirs()
{
	CString dir = Terrain_ProjectDir() + "Textures";
	CreateDirectory(dir, NULL);
	dir += "\\Terrainmesh";
	CreateDirectory(dir, NULL);
}

#define TERRAIN_PAINT_UNITS	4.0f	// world units per paint texel
#define TERRAIN_PAINT_MAX	1024

// Layer weights from the height-grid vertices (old terrains, before the paint map).
static void Terrain_VertexWeightsAt(const TerrainInfo &info, float x, float z, float *out)
{
	const int stride = info.m_N + 1;
	float gz = (z - info.m_z0) / info.m_sz * info.m_N;
	int j = (int)floor(gz);
	if(j < 0) j = 0;
	if(j > info.m_N - 1) j = info.m_N - 1;
	float fz = gz - (float)j;
	if(fz < 0) fz = 0; if(fz > 1) fz = 1;
	float gx = (x - info.m_x0) / info.m_sx * info.m_N;
	int i = (int)floor(gx);
	if(i < 0) i = 0;
	if(i > info.m_N - 1) i = info.m_N - 1;
	float fx = gx - (float)i;
	if(fx < 0) fx = 0; if(fx > 1) fx = 1;
	if(info.m_Weights.size() < (size_t)info.NumVerts() * TERRAIN_NUM_LAYERS)
	{
		out[0] = 1.0f; out[1] = out[2] = out[3] = 0.0f;
		return;
	}
	const float *w00 = &info.m_Weights[(j * stride + i) * TERRAIN_NUM_LAYERS];
	const float *w10 = &info.m_Weights[(j * stride + i + 1) * TERRAIN_NUM_LAYERS];
	const float *w01 = &info.m_Weights[((j+1) * stride + i) * TERRAIN_NUM_LAYERS];
	const float *w11 = &info.m_Weights[((j+1) * stride + i + 1) * TERRAIN_NUM_LAYERS];
	for(int l=0; l < TERRAIN_NUM_LAYERS; l++)
		out[l] = (w00[l] * (1 - fx) + w10[l] * fx) * (1 - fz) + (w01[l] * (1 - fx) + w11[l] * fx) * fz;
}

// Builds the paint map (from the vertex weights) if the terrain doesn't have one yet.
static void Terrain_EnsurePaint(TerrainInfo &info)
{
	int W = 16, H = 16;
	while(W < TERRAIN_PAINT_MAX && (float)W * TERRAIN_PAINT_UNITS < info.m_sx) W <<= 1;
	while(H < TERRAIN_PAINT_MAX && (float)H * TERRAIN_PAINT_UNITS < info.m_sz) H <<= 1;
	if(info.m_PW == W && info.m_PH == H && info.m_Paint.size() == (size_t)W * H * TERRAIN_NUM_LAYERS)
		return;
	info.m_PW = W;
	info.m_PH = H;
	info.m_Paint.assign((size_t)W * H * TERRAIN_NUM_LAYERS, 0);
	for(int v=0; v < H; v++)
		for(int u=0; u < W; u++)
		{
			float w[TERRAIN_NUM_LAYERS];
			Terrain_VertexWeightsAt(info, info.m_x0 + ((float)u + 0.5f) * info.m_sx / W, info.m_z0 + ((float)v + 0.5f) * info.m_sz / H, w);
			uint8 *p = &info.m_Paint[((size_t)v * W + u) * TERRAIN_NUM_LAYERS];
			for(int l=0; l < TERRAIN_NUM_LAYERS; l++)
			{
				float f = w[l] * 255.0f + 0.5f;
				p[l] = (uint8)(f < 0 ? 0 : (f > 255 ? 255 : f));
			}
		}
}

// Layer weights at a world position (bilinear over the paint map).
static void Terrain_WeightsAtRaw(const TerrainInfo &info, float x, float z, float *out)
{
	const int W = info.m_PW, H = info.m_PH;
	if(W <= 0 || H <= 0 || info.m_Paint.size() != (size_t)W * H * TERRAIN_NUM_LAYERS)
	{
		Terrain_VertexWeightsAt(info, x, z, out);
		return;
	}
	float fu = (x - info.m_x0) / info.m_sx * W - 0.5f;
	float fv = (z - info.m_z0) / info.m_sz * H - 0.5f;
	int u0 = (int)floor(fu), v0 = (int)floor(fv);
	float ax = fu - (float)u0, az = fv - (float)v0;
	int u1 = u0 + 1, v1 = v0 + 1;
	if(u0 < 0) u0 = 0; if(u0 > W - 1) u0 = W - 1;
	if(u1 < 0) u1 = 0; if(u1 > W - 1) u1 = W - 1;
	if(v0 < 0) v0 = 0; if(v0 > H - 1) v0 = H - 1;
	if(v1 < 0) v1 = 0; if(v1 > H - 1) v1 = H - 1;
	const uint8 *p00 = &info.m_Paint[((size_t)v0 * W + u0) * TERRAIN_NUM_LAYERS];
	const uint8 *p10 = &info.m_Paint[((size_t)v0 * W + u1) * TERRAIN_NUM_LAYERS];
	const uint8 *p01 = &info.m_Paint[((size_t)v1 * W + u0) * TERRAIN_NUM_LAYERS];
	const uint8 *p11 = &info.m_Paint[((size_t)v1 * W + u1) * TERRAIN_NUM_LAYERS];
	for(int l=0; l < TERRAIN_NUM_LAYERS; l++)
		out[l] = ((p00[l] * (1 - ax) + p10[l] * ax) * (1 - az) + (p01[l] * (1 - ax) + p11[l] * ax) * az) * (1.0f / 255.0f);
}

// The game shader blends two layers: layer 2 stands in for layers 2-4.
static void Terrain_WeightsAt(const TerrainInfo &info, float x, float z, float *out)
{
	Terrain_WeightsAtRaw(info, x, z, out);
	out[1] += out[2] + out[3];
	out[2] = out[3] = 0.0f;
}

static CString Terrain_PaintPath(int id)
{
	CString s = Terrain_SidecarPath(id);
	s = s.Left(s.GetLength() - 3) + "wgt";
	return s;
}

static void Terrain_SavePaint(int id, const TerrainInfo &info)
{
	if(info.m_PW <= 0 || info.m_Paint.empty())
		return;
	FILE *fp = fopen(Terrain_PaintPath(id), "wb");
	if(!fp)
		return;
	int hdr[3] = { 0x54475754 /* TWGT */, info.m_PW, info.m_PH };
	fwrite(hdr, sizeof(hdr), 1, fp);
	fwrite(&info.m_Paint[0], 1, info.m_Paint.size(), fp);
	fclose(fp);
}

static void Terrain_LoadPaint(int id, TerrainInfo &info)
{
	info.m_PW = info.m_PH = 0;
	info.m_Paint.clear();
	FILE *fp = fopen(Terrain_PaintPath(id), "rb");
	if(fp)
	{
		int hdr[3];
		if(fread(hdr, sizeof(hdr), 1, fp) == 1 && hdr[0] == 0x54475754 && hdr[1] > 0 && hdr[2] > 0 &&
			hdr[1] <= 8192 && hdr[2] <= 8192)
		{
			std::vector<uint8> buf((size_t)hdr[1] * hdr[2] * TERRAIN_NUM_LAYERS);
			if(fread(&buf[0], 1, buf.size(), fp) == buf.size())
			{
				info.m_PW = hdr[1];
				info.m_PH = hdr[2];
				info.m_Paint.swap(buf);
			}
		}
		fclose(fp);
	}
	Terrain_EnsurePaint(info);
}

static bool Terrain_SaveInfo(int id, const TerrainInfo &info)
{
	Terrain_SavePaint(id, info);
	Terrain_MakeDirs();
	FILE *fp = fopen(Terrain_SidecarPath(id), "wt");
	if(!fp)
		return false;

	fprintf(fp, "LTTERRAIN 1\n");
	fprintf(fp, "%d %f %f %f %f %f\n", info.m_N, info.m_x0, info.m_z0, info.m_sx, info.m_sz, info.m_BaseY);
	for(int l=0; l < TERRAIN_NUM_LAYERS; l++)
		fprintf(fp, "%s\n", info.m_Layers[l].empty() ? "-" : info.m_Layers[l].c_str());
	for(int v=0; v < info.NumVerts(); v++)
	{
		const float *w = &info.m_Weights[v * TERRAIN_NUM_LAYERS];
		fprintf(fp, "%.3f %.3f %.3f %.3f\n", w[0], w[1], w[2], w[3]);
	}
	fclose(fp);
	return true;
}

static bool Terrain_LoadInfo(int id, TerrainInfo &info)
{
	FILE *fp = fopen(Terrain_SidecarPath(id), "rt");
	if(!fp)
		return false;

	char line[512];
	bool bOK = false;
	if(fgets(line, sizeof(line), fp) && (strncmp(line, "LTTERRAIN", 9) == 0 || strncmp(line, "LT1TERRAIN", 10) == 0) &&
		fgets(line, sizeof(line), fp) &&
		sscanf(line, "%d %f %f %f %f %f", &info.m_N, &info.m_x0, &info.m_z0, &info.m_sx, &info.m_sz, &info.m_BaseY) == 6 &&
		info.m_N >= 1 && info.m_N <= 64 && info.m_sx > 0.0f && info.m_sz > 0.0f)
	{
		bOK = true;
		for(int l=0; l < TERRAIN_NUM_LAYERS && bOK; l++)
		{
			if(!fgets(line, sizeof(line), fp))
			{
				bOK = false;
				break;
			}
			char *p = line + strlen(line);
			while(p > line && (p[-1] == '\n' || p[-1] == '\r' || p[-1] == ' '))
				*--p = 0;
			info.m_Layers[l] = strcmp(line, "-") == 0 ? "" : line;
		}

		info.m_Weights.assign(info.NumVerts() * TERRAIN_NUM_LAYERS, 0.0f);
		for(int v=0; v < info.NumVerts() && bOK; v++)
		{
			float *w = &info.m_Weights[v * TERRAIN_NUM_LAYERS];
			if(!fgets(line, sizeof(line), fp) || sscanf(line, "%f %f %f %f", &w[0], &w[1], &w[2], &w[3]) != 4)
			{
				// Missing weights: fall back to layer 1.
				for(; v < info.NumVerts(); v++)
					info.m_Weights[v * TERRAIN_NUM_LAYERS] = 1.0f;
				break;
			}
		}
	}

	fclose(fp);
	if(bOK)
		Terrain_LoadPaint(id, info);
	return bOK;
}


// Terrain nodes are labelled "TerrainNNNN".
static int Terrain_NodeID(CWorldNode *pNode)
{
	const char *pLabel = pNode->GetNodeLabel();
	if(!pLabel || _strnicmp(pLabel, "Terrain", 7) != 0)
		return -1;
	const char *p = pLabel + 7;
	if(strlen(p) != 4)
		return -1;
	for(int i=0; i < 4; i++)
		if(p[i] < '0' || p[i] > '9')
			return -1;
	return atoi(p);
}

static void Terrain_FindNodes(CWorldNode *pNode, std::vector<CWorldNode*> &out)
{
	if(Terrain_NodeID(pNode) >= 0)
	{
		out.push_back(pNode);
		return;
	}
	for(GPOS pos = pNode->m_Children.GetHeadPosition(); pos; )
		Terrain_FindNodes(pNode->m_Children.GetNext(pos), out);
}


// Fills in the brushes and heights of a terrain from its node.
// Brush layouts: a solid prism has 6 points (the first three are the top) and 5 faces;
// anything else is a one-sided surface whose points are all on the terrain surface.
static bool Terrain_IsPrism(CEditBrush *pBrush)
{
	return pBrush->m_Points.GetSize() == 6 && pBrush->m_Polies.GetSize() == 5;
}

static uint32 Terrain_NumSurfacePoints(CEditBrush *pBrush)
{
	return Terrain_IsPrism(pBrush) ? 3 : pBrush->m_Points.GetSize();
}

static bool Terrain_IsTerrainBrush(CEditBrush *pBrush)
{
	return Terrain_IsPrism(pBrush) || (pBrush->m_Points.GetSize() >= 3 && pBrush->m_Polies.GetSize() >= 1);
}

static bool Terrain_RayTri(const LTVector &orig, const LTVector &dir, const LTVector &a, const LTVector &b, const LTVector &c, float &t);

// Nearest hit of a ray on a terrain brush's surface.
static bool Terrain_RayBrush(CEditBrush *pBrush, const LTVector &orig, const LTVector &dir, float &tBest)
{
	bool bHit = false;
	float t;
	if(Terrain_IsPrism(pBrush))
	{
		if(Terrain_RayTri(orig, dir, pBrush->m_Points[0], pBrush->m_Points[1], pBrush->m_Points[2], t) && t < tBest)
		{
			tBest = t;
			bHit = true;
		}
		return bHit;
	}

	for(uint32 p=0; p < pBrush->m_Polies.GetSize(); p++)
	{
		CEditPoly *pPoly = pBrush->m_Polies[p];
		if(!pPoly || pPoly->NumVerts() < 3)
			continue;
		for(uint32 v=1; v + 1 < pPoly->NumVerts(); v++)
		{
			if(Terrain_RayTri(orig, dir, pPoly->Pt(0), pPoly->Pt(v), pPoly->Pt(v+1), t) && t < tBest)
			{
				tBest = t;
				bHit = true;
			}
		}
	}
	return bHit;
}

static bool Terrain_Gather(CWorldNode *pNode, TerrainRef &ref)
{
	ref.m_pNode = pNode;
	ref.m_ID = Terrain_NodeID(pNode);
	if(ref.m_ID < 0 || !Terrain_LoadInfo(ref.m_ID, ref.m_Info))
		return false;

	const TerrainInfo &info = ref.m_Info;
	ref.m_Brushes.clear();
	ref.m_Heights.assign(info.NumVerts(), info.m_BaseY + TERRAIN_MIN_THICK);

	for(GPOS pos = pNode->m_Children.GetHeadPosition(); pos; )
	{
		CWorldNode *pChild = pNode->m_Children.GetNext(pos);
		if(pChild->GetType() != Node_Brush)
			continue;
		CEditBrush *pBrush = pChild->AsBrush();
		if(!Terrain_IsTerrainBrush(pBrush))
			continue;

		ref.m_Brushes.push_back(pBrush);
		for(uint32 k=0; k < Terrain_NumSurfacePoints(pBrush); k++)
		{
			const CEditVert &pt = pBrush->m_Points[k];
			int g = info.GridIndex(pt.x, pt.z);
			if(g >= 0)
				ref.m_Heights[g] = pt.y;
		}
	}
	return ref.m_Brushes.size() > 0;
}


// Writes the heights back into the prisms.
static void Terrain_Apply(TerrainRef &ref)
{
	for(size_t b=0; b < ref.m_Brushes.size(); b++)
	{
		CEditBrush *pBrush = ref.m_Brushes[b];
		for(uint32 k=0; k < Terrain_NumSurfacePoints(pBrush); k++)
		{
			CEditVert &pt = pBrush->m_Points[k];
			int g = ref.m_Info.GridIndex(pt.x, pt.z);
			if(g >= 0)
				pt.y = ref.m_Heights[g];
		}
		pBrush->UpdatePlanes();
		pBrush->UpdateBoundingInfo();
	}
}


//------------------------------------------------------------------
// Texture baking
//------------------------------------------------------------------

struct LayerImage
{
	int						m_W, m_H;
	std::vector<uint32>		m_Pixels;		// 0x00RRGGBB
	uint32					m_UserFlags;	// the texture's surface flags
};

// Loads mip 0 of a DTX (any pixel format DEdit can display) as 32 bit pixels.
static bool Terrain_LoadDtx(const char *pName, LayerImage &img)
{
	if(!pName || !pName[0])
		return false;

	CString path = Terrain_ProjectDir() + pName;
	DStream *pStream = streamsim_Open((LPCTSTR)path, "rb");
	if(!pStream)
		return false;

	TextureData *pData = NULL;
	DRESULT dResult = dtx_Create(pStream, &pData, TRUE);
	pStream->Release();
	if(dResult != DE_OK || !pData)
		return false;

	bool bOK = false;
	TextureMipData *pMip = &pData->m_Mips[0];
	int w = (int)pMip->m_Width, h = (int)pMip->m_Height;
	if(pData->m_Header.m_nMipmaps >= 1 && w >= 1 && h >= 1 && w <= 4096 && h <= 4096)
	{
		img.m_W = w;
		img.m_H = h;
		img.m_UserFlags = (uint32)pData->m_Header.m_UserFlags;
		img.m_Pixels.assign((size_t)w * h, 0);

		FMConvertRequest cRequest;
		cRequest.m_pDest = (BYTE*)&img.m_Pixels[0];
		cRequest.m_pDestFormat->InitPValueFormat();
		cRequest.m_DestPitch = w * 4;

		cRequest.m_pSrc = pMip->m_Data;
		dtx_SetupDTXFormat(pData, cRequest.m_pSrcFormat);
		cRequest.m_SrcPitch = pMip->m_Pitch;

		cRequest.m_Width = pMip->m_Width;
		cRequest.m_Height = pMip->m_Height;
		cRequest.m_Flags = 0;

		// True colour palettized textures keep their palette in a section.
		if(pData->m_Header.GetBPPIdent() == BPP_32P)
		{
			DtxSection *pSection = dtx_FindSection(pData, "PALLETE32");
			if(pSection)
				cRequest.m_pSrcPalette = (RPaletteColor*)pSection->m_Data;
		}

		if(g_FormatMgr.ConvertPixels(&cRequest) == LT_OK)
		{
			for(size_t i=0; i < img.m_Pixels.size(); i++)
				img.m_Pixels[i] &= 0x00FFFFFF;
			bOK = true;
		}
	}

	dtx_Destroy(pData);
	return bOK;
}

// Writes a size x size 32 bit DTX with a full mipmap chain.
static bool Terrain_WriteDtx(const char *pPath, const std::vector<uint32> &pixels, int size, uint32 userFlags)
{
	TextureData *pData = dtx_Alloc(BPP_32, size, size, 4, NULL, NULL);
	if(!pData)
		return false;

	pData->m_Header.m_UserFlags = (int32)userFlags;

	TextureMipData *pMip = &pData->m_Mips[0];
	for(int y=0; y < size; y++)
	{
		uint32 *pRow = (uint32*)(pMip->m_Data + (size_t)y * pMip->m_Pitch);
		for(int x=0; x < size; x++)
			pRow[x] = 0xFF000000 | pixels[(size_t)y * size + x];
	}

	dtx_BuildMipmaps(pData);

	DStream *pStream = streamsim_Open(pPath, "wb");
	if(!pStream)
	{
		dtx_Destroy(pData);
		return false;
	}

	DRESULT dResult = dtx_Save(pData, pStream);
	pStream->Release();
	dtx_Destroy(pData);
	return dResult == DE_OK;
}

// Fallback colours for layers without a loadable texture.
static const uint32 g_LayerFallback[TERRAIN_NUM_LAYERS] = { 0x5A7A3A, 0x7A6648, 0x8A8A88, 0xC8C090 };

// Layer 1 must be a terrain_blend material for the game to get tiled layers + blend map.
static bool Terrain_LoadLayerMat(const TerrainInfo &info, int l, MatFile::Material &mat)
{
	const std::string &lay = info.m_Layers[l];
	if(lay.empty() || !MaterialTexture_IsMaterialName(lay.c_str()))
		return false;
	// called per polygon on conversion: keep the parsed material for a moment
	struct Ent { DWORD t; bool ok; MatFile::Material m; };
	static std::map<std::string, Ent> s_Mats;
	DWORD now = GetTickCount();
	std::map<std::string, Ent>::iterator it = s_Mats.find(lay);
	if(it == s_Mats.end() || now - it->second.t > 1500)
	{
		Ent e;
		e.t = now;
		e.ok = MatFile::LoadFile(Terrain_ProjectDir() + lay.c_str(), e.m);
		s_Mats[lay] = e;
		it = s_Mats.find(lay);
	}
	if(!it->second.ok)
		return false;
	mat = it->second.m;
	return true;
}

static bool Terrain_CanSplat(const TerrainInfo &info, MatFile::Material *pMat = NULL)
{
	MatFile::Material m;
	if(!Terrain_LoadLayerMat(info, 0, m))
		return false;
	std::string sh = m.shader;
	for(size_t i=0; i < sh.size(); i++) sh[i] = (char)tolower((unsigned char)sh[i]);
	if(sh.find("terrain_blend") == std::string::npos)
		return false;
	if(pMat) *pMat = m;
	return true;
}

// World units one tile of the layer textures covers (0 = unknown).
static float Terrain_TileSize(const TerrainInfo &info)
{
	MatFile::Material m;
	if(!Terrain_LoadLayerMat(info, 0, m))
		return 0.0f;
	float w = m.GetFloat("DefaultWidth", 0.0f);
	return (w >= 1.0f) ? w : 0.0f;
}

// The game shader blends two layers, so every pair of layers gets its own material
// (NNNN_gK.Mat00 + NNNN_bK.dds); each poly uses the pair that covers it best.
#define TERRAIN_NUM_PAIRS	6
#define TERRAIN_PAIR_SCALE	100000.0f
static const int g_PairA[TERRAIN_NUM_PAIRS] = { 0, 0, 0, 1, 1, 2 };
static const int g_PairB[TERRAIN_NUM_PAIRS] = { 1, 2, 3, 2, 3, 3 };

static CString Terrain_BlendName(int id, int k)
{
	CString s;
	s.Format("Textures\\Terrainmesh\\%04d_b%d.dds", id, k);
	return s;
}

static CString Terrain_GameMatName(int id, int k)
{
	CString s;
	s.Format("Textures\\Terrainmesh\\%04d_g%d.Mat00", id, k);
	return s;
}

// Texture size follows the terrain's footprint: painting bakes 2 world units per
// texel (fast), World > Process / Rebake bakes 1 unit per texel. The faces map the
// texture by fraction of the footprint, so the pixel size can differ freely.
static void Terrain_TexDims(const TerrainInfo &info, bool bFinal, int &W, int &H, bool bLive = false)
{
	const float upt = bFinal ? 1.0f : (bLive ? 4.0f : 2.0f);
	const int maxSz = bFinal ? 4096 : (bLive ? 1024 : 2048);
	int w = 64, h = 64;
	while(w < maxSz && (float)w * upt < info.m_sx) w <<= 1;
	while(h < maxSz && (float)h * upt < info.m_sz) h <<= 1;
	W = w;
	H = h;
}

struct BakeCache
{
	BakeCache() : m_ID(-1), m_W(0), m_H(0) {}
	int					m_ID, m_W, m_H;
	std::string			m_Sig;
	std::vector<uint32>	m_Px;
};
static BakeCache g_BakeCache;

// Bounds (world x/z) of what the current stroke has painted.
static bool		g_bDirty = false;
static float	g_Dirty[4];	// minx, maxx, minz, maxz
// everything the stroke painted (for the file bake when it ends)
static bool		g_bStrokeDirty = false;
static float	g_StrokeDirty[4];
static void Terrain_MergeStrokeDirty()
{
	if(!g_bDirty)
		return;
	if(!g_bStrokeDirty)
	{
		for(int i=0; i < 4; i++) g_StrokeDirty[i] = g_Dirty[i];
		g_bStrokeDirty = true;
		return;
	}
	if(g_Dirty[0] < g_StrokeDirty[0]) g_StrokeDirty[0] = g_Dirty[0];
	if(g_Dirty[1] > g_StrokeDirty[1]) g_StrokeDirty[1] = g_Dirty[1];
	if(g_Dirty[2] < g_StrokeDirty[2]) g_StrokeDirty[2] = g_Dirty[2];
	if(g_Dirty[3] > g_StrokeDirty[3]) g_StrokeDirty[3] = g_Dirty[3];
}

static void Terrain_BakePixels(const TerrainInfo &info, const LayerImage *layers, const bool *bHave, float tile,
	int W, int H, int u0, int u1, int v0, int v1, std::vector<uint32> &pixels)
{
	for(int v=v0; v < v1; v++)
	{
		// v runs from the far (max z) edge towards min z.
		float z = info.m_z0 + info.m_sz - ((float)v + 0.5f) * info.m_sz / (float)H;

		for(int u=u0; u < u1; u++)
		{
			float x = info.m_x0 + ((float)u + 0.5f) * info.m_sx / (float)W;
			float wts[TERRAIN_NUM_LAYERS];
			Terrain_WeightsAt(info, x, z, wts);

			float rgb[3] = { 0, 0, 0 }, total = 0.0f;
			for(int l=0; l < TERRAIN_NUM_LAYERS; l++)
			{
				float w = wts[l];
				if(w <= 0.001f)
					continue;

				uint32 c;
				if(bHave[l])
				{
					// One tile of the material covers 'tile' world units (default: one texel per unit).
					int tx, ty;
					if(tile > 0.0f)
					{
						tx = ((int)floor(x / tile * (float)layers[l].m_W)) % layers[l].m_W;
						ty = ((int)floor(-z / tile * (float)layers[l].m_H)) % layers[l].m_H;
					}
					else
					{
						tx = ((int)floor(x)) % layers[l].m_W;
						ty = ((int)floor(-z)) % layers[l].m_H;
					}
					if(tx < 0) tx += layers[l].m_W;
					if(ty < 0) ty += layers[l].m_H;
					c = layers[l].m_Pixels[ty * layers[l].m_W + tx];
				}
				else
				{
					c = g_LayerFallback[l];
				}
				rgb[0] += w * ((c >> 16) & 255);
				rgb[1] += w * ((c >> 8) & 255);
				rgb[2] += w * (c & 255);
				total += w;
			}
			if(total <= 0.0f)
			{
				total = 1.0f;
				rgb[0] = rgb[1] = rgb[2] = 128.0f;
			}

			uint32 r = (uint32)(rgb[0] / total + 0.5f), g = (uint32)(rgb[1] / total + 0.5f), b = (uint32)(rgb[2] / total + 0.5f);
			if(r > 255) r = 255;
			if(g > 255) g = 255;
			if(b > 255) b = 255;
			pixels[(size_t)v * W + u] = 0xFF000000 | (r << 16) | (g << 8) | b;
		}
	}
}

// pDirty (minx, maxx, minz, maxz): only that part of a cached preview is recomputed.
static bool Terrain_WriteGame(const TerrainRef &ref);

static bool Terrain_Bake(const TerrainRef &ref, bool bFinal = true, const float *pDirty = NULL, bool bGame = true)
{
	const TerrainInfo &info = ref.m_Info;
	LayerImage layers[TERRAIN_NUM_LAYERS];
	bool bHave[TERRAIN_NUM_LAYERS];
	int l;
	// Layer textures are cached between bakes (reloaded only when the file changes).
	struct CachedLayer { LayerImage img; bool ok; FILETIME t; };
	static std::map<std::string, CachedLayer> s_Cache;
	std::string sig;
	for(l=0; l < TERRAIN_NUM_LAYERS; l++)
	{
		const std::string &nm = info.m_Layers[l];
		sig += nm;
		sig += '|';
		bHave[l] = false;
		if(nm.empty())
			continue;
		FILETIME ft = {0, 0};
		WIN32_FILE_ATTRIBUTE_DATA fa;
		if(GetFileAttributesEx((LPCTSTR)(Terrain_ProjectDir() + nm.c_str()), GetFileExInfoStandard, &fa))
			ft = fa.ftLastWriteTime;
		std::map<std::string, CachedLayer>::iterator it = s_Cache.find(nm);
		if(it == s_Cache.end() || CompareFileTime(&it->second.t, &ft) != 0)
		{
			CachedLayer cl;
			cl.ok = Terrain_LoadDtx(nm.c_str(), cl.img);
			cl.t = ft;
			s_Cache[nm] = cl;
			it = s_Cache.find(nm);
		}
		bHave[l] = it->second.ok;
		if(bHave[l])
			layers[l] = it->second.img;
	}

	int W, H;
	Terrain_TexDims(info, bFinal, W, H, !bFinal && !bGame);
	const float tile = Terrain_TileSize(info);

	std::vector<uint32> finalPixels;
	std::vector<uint32> *pPixels = &finalPixels;
	if(!bFinal)
	{
		BakeCache &bc = g_BakeCache;
		bool bValid = (bc.m_ID == ref.m_ID && bc.m_W == W && bc.m_H == H && bc.m_Sig == sig && bc.m_Px.size() == (size_t)W * H);
		if(!bValid)
		{
			bc.m_ID = ref.m_ID; bc.m_W = W; bc.m_H = H; bc.m_Sig = sig;
			bc.m_Px.assign((size_t)W * H, 0xFF000000);
		}
		pPixels = &bc.m_Px;
		if(bValid && pDirty)
		{
			int u0 = (int)floor((pDirty[0] - info.m_x0) / info.m_sx * W) - 1;
			int u1 = (int)ceil((pDirty[1] - info.m_x0) / info.m_sx * W) + 1;
			int v0 = (int)floor((info.m_z0 + info.m_sz - pDirty[3]) / info.m_sz * H) - 1;
			int v1 = (int)ceil((info.m_z0 + info.m_sz - pDirty[2]) / info.m_sz * H) + 1;
			if(u0 < 0) u0 = 0;
			if(v0 < 0) v0 = 0;
			if(u1 > W) u1 = W;
			if(v1 > H) v1 = H;
			if(u1 > u0 && v1 > v0)
				Terrain_BakePixels(info, layers, bHave, tile, W, H, u0, u1, v0, v1, bc.m_Px);
		}
		else
			Terrain_BakePixels(info, layers, bHave, tile, W, H, 0, W, 0, H, bc.m_Px);
	}
	else
	{
		finalPixels.assign((size_t)W * H, 0xFF000000);
		Terrain_BakePixels(info, layers, bHave, tile, W, H, 0, W, 0, H, finalPixels);
		g_BakeCache.m_ID = -1;	// the file is no longer the preview
	}

	Terrain_MakeDirs();
	CString path = Terrain_ProjectDir() + Terrain_TextureName(ref.m_ID);
	CString ddsPath = Terrain_ProjectDir() + Terrain_DdsName(ref.m_ID);
	if(!WriteCondemnedDdsBGRA(ddsPath, (const unsigned char*)&(*pPixels)[0], W, H))
		return false;

	// The game's material (tiled layers + blend map), when the layers allow it.
	if(bGame)
		Terrain_WriteGame(ref);

	// Preview bakes keep the material that is already there.
	if(!bFinal && GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES)
		return true;

	// The material the faces use: shader and settings copied from the first layer
	// that is a material, pointing at the baked .dds.
	MatFile::Material mat;
	bool bHaveMat = false;
	for(l=0; l < TERRAIN_NUM_LAYERS && !bHaveMat; l++)
	{
		const std::string &lay = info.m_Layers[l];
		if(lay.empty() || !MaterialTexture_IsMaterialName(lay.c_str()))
			continue;
		CString full = Terrain_ProjectDir() + lay.c_str();
		bHaveMat = MatFile::LoadFile(full, mat);
	}
	if(!bHaveMat)
	{
		// Fall back to any existing terrain material.
		CString full;
		full.Format("%sTextures\\Terrainmesh\\%04d.Mat00", (LPCTSTR)Terrain_ProjectDir(), ref.m_ID);
		bHaveMat = MatFile::LoadFile(full, mat);
	}
	if(!bHaveMat)
	{
		static bool s_bWarned = false;
		if(!s_bWarned) { s_bWarned = true; AfxMessageBox("Terrain: layer 1 needs to be a material (.Mat00) so the painted terrain has a shader in the game.", MB_ICONEXCLAMATION); }
		return false;
	}
	std::string param = MaterialTexture_PreviewParam(mat);
	if(param.empty())
		param = "tDiffuseMap";
	mat.SetTexture(param.c_str(), (LPCTSTR)Terrain_DdsName(ref.m_ID));
	// The faces map the whole footprint across TERRAIN_TEX_SIZE texture units: one tile.
	mat.SetFloat("DefaultWidth", (float)TERRAIN_TEX_SIZE);
	mat.SetFloat("DefaultHeight", (float)TERRAIN_TEX_SIZE);
	return MatFile::SaveFile(path, mat);
}

// Writes what the game uses: NNNN_b.dds (red = how much of layer 2 over layer 1, footprint
// mapped) and NNNN_g.Mat00 (terrain_blend: layer 1 tiled, layer 2 tiled, the blend map).
static bool Terrain_WriteGame(const TerrainRef &ref)
{
	const TerrainInfo &info = ref.m_Info;
	MatFile::Material base;
	if(!Terrain_CanSplat(info, &base))
		return false;
	float tile = Terrain_TileSize(info);
	if(tile < 1.0f)
		tile = 256.0f;

	MatFile::Material lm[TERRAIN_NUM_LAYERS];
	bool bLM[TERRAIN_NUM_LAYERS];
	std::string tex[TERRAIN_NUM_LAYERS];
	int l;
	for(l=0; l < TERRAIN_NUM_LAYERS; l++)
	{
		bLM[l] = Terrain_LoadLayerMat(info, l, lm[l]);
		if(bLM[l])
			tex[l] = lm[l].GetTexture(MaterialTexture_PreviewParam(lm[l]).c_str(), "");
	}
	if(tex[0].empty())
		return false;

	// blend maps at the paint map's resolution
	int W = 16, H = 16;
	while(W < 1024 && (float)W * 4.0f < info.m_sx) W <<= 1;
	while(H < 1024 && (float)H * 4.0f < info.m_sz) H <<= 1;
	if(info.m_PW > 0 && info.m_PH > 0) { W = info.m_PW; H = info.m_PH; }
	std::vector<float> wts((size_t)W * H * TERRAIN_NUM_LAYERS);
	for(int v=0; v < H; v++)
	{
		float z = info.m_z0 + info.m_sz - ((float)v + 0.5f) * info.m_sz / (float)H;
		for(int u=0; u < W; u++)
			Terrain_WeightsAt(info, info.m_x0 + ((float)u + 0.5f) * info.m_sx / (float)W, z, &wts[((size_t)v * W + u) * TERRAIN_NUM_LAYERS]);
	}

	Terrain_MakeDirs();
	std::vector<uint32> px((size_t)W * H);
	bool bOK = true;
	for(int k=0; k < TERRAIN_NUM_PAIRS; k++)
	{
		const int a = g_PairA[k], b = g_PairB[k];
		if(k > 0)
			continue;	// two layers per terrain	// only pairs of layers that are set (pair 0 always exists)
		for(size_t t=0; t < (size_t)W * H; t++)
		{
			float wa = wts[t * TERRAIN_NUM_LAYERS + a], wb = wts[t * TERRAIN_NUM_LAYERS + b];
			float tt = wa + wb;
			float r = (tt > 0.0001f) ? wb / tt : 0.0f;
			uint32 c = (uint32)(r * 255.0f + 0.5f);
			if(c > 255) c = 255;
			px[t] = 0xFF000000 | (c << 16) | (c << 8) | c;
		}
		if(!WriteCondemnedDdsBGRA(Terrain_ProjectDir() + Terrain_BlendName(ref.m_ID, k), (const unsigned char*)&px[0], W, H))
		{
			bOK = false;
			continue;
		}

		MatFile::Material mat = base;
		const std::string &ta = tex[a].empty() ? tex[0] : tex[a];
		const std::string &tb = tex[b].empty() ? ta : tex[b];
		mat.SetTexture("tDiffuseMap", ta.c_str());
		mat.SetTexture("tTerrainDiffuseMap", tb.c_str());
		const char *maps[3][2] = { { "tNormalMap", "tTerrainNormalMap" }, { "tSpecularMap", "tTerrainSpecularMap" }, { "tEmissiveMap", "tTerrainEmissiveMap" } };
		for(int m=0; m < 3; m++)
		{
			if(bLM[a] && lm[a].Find(maps[m][0]))
				mat.SetTexture(maps[m][0], lm[a].GetTexture(maps[m][0], "").c_str());
			if(bLM[b] && lm[b].Find(maps[m][0]))
				mat.SetTexture(maps[m][1], lm[b].GetTexture(maps[m][0], "").c_str());
		}
		mat.SetTexture("tBlendMap", (LPCTSTR)Terrain_BlendName(ref.m_ID, k));
		mat.SetFloat("DefaultWidth", tile);
		mat.SetFloat("DefaultHeight", tile);
		if(!MatFile::SaveFile(Terrain_ProjectDir() + Terrain_GameMatName(ref.m_ID, k), mat))
			bOK = false;
	}
	return bOK;
}

// The pair of layers (see g_PairA/B) with the most weight over a poly.
static int Terrain_PolyPair(CEditPoly *pPoly, const TerrainInfo &info)
{
	return 0;	// one pair (layers 1 and 2) for the whole terrain
}
static int Terrain_PolyPairUnused(CEditPoly *pPoly, const TerrainInfo &info)
{
	float sum[TERRAIN_NUM_LAYERS] = { 0, 0, 0, 0 };
	const uint32 n = pPoly->NumVerts();
	if(n >= 3)
	{
		const int steps = 6;
		for(uint32 t=1; t + 1 < n; t++)
		{
			LTVector p0 = pPoly->Pt(0), p1 = pPoly->Pt(t), p2 = pPoly->Pt(t + 1);
			for(int i=0; i <= steps; i++)
				for(int j=0; j <= steps - i; j++)
				{
					float bi = (float)i / steps, bj = (float)j / steps;
					LTVector q = p0 + (p1 - p0) * bi + (p2 - p0) * bj;
					float w[TERRAIN_NUM_LAYERS];
					Terrain_WeightsAt(info, q.x, q.z, w);
					for(int l=0; l < TERRAIN_NUM_LAYERS; l++)
						sum[l] += w[l];
				}
		}
	}
	int a = 0, b = 1, l;
	for(l=0; l < TERRAIN_NUM_LAYERS; l++) if(sum[l] > sum[a]) a = l;
	b = (a == 0) ? 1 : 0;
	for(l=0; l < TERRAIN_NUM_LAYERS; l++) if(l != a && sum[l] > sum[b]) b = l;
	if(sum[b] <= 0.0001f)	// only one layer here: prefer a pair with layer 1
		b = (a == 0) ? 1 : 0;
	if(a > b) { int t = a; a = b; b = t; }
	for(int k=0; k < TERRAIN_NUM_PAIRS; k++)
		if(g_PairA[k] == a && g_PairB[k] == b)
			return k;
	return 0;
}

// Texture 1 of every terrain poly carries the footprint mapping used for the game's blend
// map (O.y = tile size, negative on non-top faces). Cleared when the layers can't be splatted.
static void Terrain_FixPolyMapping(CEditPoly *pPoly, const TerrainInfo &info, float T, bool bSplat)
{
	bool bTop = pPoly->Normal().y > 0.5f;
	CTexturedPlane &t1 = pPoly->GetTexture(1);
	if(bSplat)
	{
		float packed = T + (float)Terrain_PolyPair(pPoly, info) * TERRAIN_PAIR_SCALE;
		t1.SetO(LTVector(info.m_x0, bTop ? packed : -packed, info.m_z0 + info.m_sz));
		t1.SetP(LTVector(1.0f / info.m_sx, 0.0f, 0.0f));
		t1.SetQ(LTVector(0.0f, 0.0f, -1.0f / info.m_sz));
	}
	else if(t1.GetO().y != 0.0f)
	{
		t1.SetO(LTVector(0.0f, 0.0f, 0.0f));
		pPoly->SetupBaseTextureSpace(1);
	}
}

// Live painting: bakes only the painted area straight into the editor's in-memory copy of
// the terrain texture and has Direct3D re-upload it. No files, no compression, no reload.
void d3d_DropBindings(CTexture *pSystemTexture);

static bool Terrain_LiveUpdate(const TerrainRef &ref, const float *pDirty)
{
	const TerrainInfo &info = ref.m_Info;
	DFileIdent *pIdent = NULL;
	dfm_GetFileIdentifier(GetFileMgr(), Terrain_TextureName(ref.m_ID), &pIdent);
	if(!pIdent)
		return false;
	CTexture *pTex = dib_GetDibTexture(pIdent);
	if(!pTex || !pTex->m_pDib)
		return false;
	CDib *pDib = pTex->m_pDib;
	const int W = pDib->GetWidth(), H = pDib->GetHeight();
	if(W <= 0 || H <= 0)
		return false;

	// layer images, kept while the layer names stay the same
	static LayerImage s_L[TERRAIN_NUM_LAYERS];
	static bool s_Have[TERRAIN_NUM_LAYERS];
	static std::string s_LSig = "\x01";
	std::string sig;
	int l;
	for(l=0; l < TERRAIN_NUM_LAYERS; l++) { sig += info.m_Layers[l]; sig += '|'; }
	if(sig != s_LSig)
	{
		s_LSig = sig;
		for(l=0; l < TERRAIN_NUM_LAYERS; l++)
			s_Have[l] = !info.m_Layers[l].empty() && Terrain_LoadDtx(info.m_Layers[l].c_str(), s_L[l]);
	}
	const float tile = Terrain_TileSize(info);

	// composite at the dib's size; only the painted area is recomputed
	static int s_ID = -1, s_W = 0, s_H = 0;
	static std::string s_Sig;
	static std::vector<uint32> s_Px;
	int u0 = 0, u1 = W, v0 = 0, v1 = H;
	if(s_ID != ref.m_ID || s_W != W || s_H != H || s_Sig != sig || s_Px.size() != (size_t)W * H || !pDirty)
	{
		s_ID = ref.m_ID; s_W = W; s_H = H; s_Sig = sig;
		s_Px.assign((size_t)W * H, 0xFF000000);
	}
	else
	{
		u0 = (int)floor((pDirty[0] - info.m_x0) / info.m_sx * W) - 1;
		u1 = (int)ceil((pDirty[1] - info.m_x0) / info.m_sx * W) + 1;
		v0 = (int)floor((info.m_z0 + info.m_sz - pDirty[3]) / info.m_sz * H) - 1;
		v1 = (int)ceil((info.m_z0 + info.m_sz - pDirty[2]) / info.m_sz * H) + 1;
		if(u0 < 0) u0 = 0;
		if(v0 < 0) v0 = 0;
		if(u1 > W) u1 = W;
		if(v1 > H) v1 = H;
		if(u1 <= u0 || v1 <= v0)
			return true;
	}
	Terrain_BakePixels(info, s_L, s_Have, tile, W, H, u0, u1, v0, v1, s_Px);

	WORD *pBuf = pDib->GetBuf16();
	const int pitch = pDib->GetPitch();
	for(int v=v0; v < v1; v++)
	{
		WORD *pOut = pBuf + (size_t)v * pitch;
		const uint32 *pIn = &s_Px[(size_t)v * W];
		for(int u=u0; u < u1; u++)
		{
			uint32 c = pIn[u];
			pOut[u] = (WORD)((((c >> 19) & 31) << 10) | (((c >> 11) & 31) << 5) | ((c >> 3) & 31));
		}
	}
	d3d_DropBindings(pTex);
	return true;
}

// Makes the editor drop its cached copy of the baked texture and re-point
// the terrain's polies at it.
static void Terrain_ReloadTexture(TerrainRef &ref, bool bFast = false)
{
	DFileIdent *pIdent = NULL;
	dfm_GetFileIdentifier(GetFileMgr(), Terrain_TextureName(ref.m_ID), &pIdent);
	if(pIdent && pIdent->m_pUser && pIdent->m_UserType == 0)
		dib_DestroyDibTexture((CTexture*)pIdent->m_pUser);
	// The baked .dds may have its own cached copy as well.
	pIdent = NULL;
	dfm_GetFileIdentifier(GetFileMgr(), Terrain_DdsName(ref.m_ID), &pIdent);
	if(pIdent && pIdent->m_pUser && pIdent->m_UserType == 0)
		dib_DestroyDibTexture((CTexture*)pIdent->m_pUser);

	// While painting only the cached images need to go; the polys keep their file identifiers.
	if(bFast)
		return;

	// Terrains made before the .Mat00 switch still point at a .dtx: re-point them.
	static std::list<std::string> s_Names;
	s_Names.push_back((LPCTSTR)Terrain_TextureName(ref.m_ID));
	const char *pName = s_Names.back().c_str();
	bool bSplat = Terrain_CanSplat(ref.m_Info);
	float T = Terrain_TileSize(ref.m_Info);
	if(T < 1.0f) T = 256.0f;
	for(size_t b=0; b < ref.m_Brushes.size(); b++)
	{
		CEditBrush *pBrush = ref.m_Brushes[b];
		for(uint32 p=0; p < pBrush->m_Polies.GetSize(); p++)
		{
			CTexturedPlane &tex = pBrush->m_Polies[p]->GetTexture(0);
			const char *pOld = tex.m_pTextureName;
			if(pOld && strstr(pOld, "Terrainmesh") && stricmp(pOld, pName) != 0)
				tex.m_pTextureName = (char*)pName;
			tex.UpdateTextureID();
			// the layer pair can change with painting, so always refresh
			if(bSplat || pBrush->m_Polies[p]->GetTexture(1).GetO().y != 0.0f)
				Terrain_FixPolyMapping(pBrush->m_Polies[p], ref.m_Info, T, bSplat);
		}
	}
}


//------------------------------------------------------------------
// Conversion
//------------------------------------------------------------------

static void Terrain_SetupPolyTexture(CEditPoly *pPoly, const TerrainInfo &info, const char *pTexName, bool bTop)
{
	CTexturedPlane &tex = pPoly->GetTexture(0);
	tex.m_pTextureName = (char*)pTexName;

	if(bTop)
	{
		// Planar from above: the whole footprint maps onto the baked texture.
		tex.SetO(LTVector(info.m_x0, 0.0f, info.m_z0 + info.m_sz));
		tex.SetP(LTVector((float)TERRAIN_TEX_SIZE / info.m_sx, 0.0f, 0.0f));
		tex.SetQ(LTVector(0.0f, 0.0f, -(float)TERRAIN_TEX_SIZE / info.m_sz));
	}
	else
	{
		tex.SetO(LTVector(0.0f, 0.0f, 0.0f));
		pPoly->SetupBaseTextureSpace(0);
	}

	for(uint32 nTex = 1; nTex < CEditPoly::NUM_TEXTURES; nTex++)
	{
		pPoly->GetTexture(nTex).SetO(LTVector(0.0f, 0.0f, 0.0f));
		pPoly->SetupBaseTextureSpace(nTex);
	}
	if(Terrain_CanSplat(info))
	{
		float T = Terrain_TileSize(info);
		Terrain_FixPolyMapping(pPoly, info, T < 1.0f ? 256.0f : T, true);
	}
}

static void Terrain_AddPoly(CEditBrush *pBrush, const int *pIdx, int nIdx, const LTVector &centroid,
	const TerrainInfo &info, const char *pTexName, bool bTop)
{
	CEditPoly *pPoly = new CEditPoly(pBrush);
	LTVector center(0.0f, 0.0f, 0.0f);
	for(int i=0; i < nIdx; i++)
	{
		pPoly->m_Indices.Append((uint32)pIdx[i]);
		center += pBrush->m_Points[pIdx[i]];
	}
	center *= 1.0f / (float)nIdx;

	pPoly->UpdatePlane();
	if(pPoly->Normal().Dot(center - centroid) < 0.0f)
	{
		pPoly->Flip();
		pPoly->UpdatePlane();
	}

	Terrain_SetupPolyTexture(pPoly, info, pTexName, bTop);
	pBrush->m_Polies.Append(pPoly);
}

static CEditBrush* Terrain_MakePrism(CEditRegion *pRegion, CWorldNode *pParent, const TerrainInfo &info,
	const LTVector top[3], const char *pTexName)
{
	CEditBrush *pBrush = no_CreateNewBrush(pRegion, pParent);

	LTVector centroid(0.0f, 0.0f, 0.0f);
	int k;
	for(k=0; k < 3; k++)
		pBrush->m_Points.Append(CEditVert(top[k]));
	for(k=0; k < 3; k++)
		pBrush->m_Points.Append(CEditVert(LTVector(top[k].x, info.m_BaseY, top[k].z)));
	for(k=0; k < 6; k++)
		centroid += pBrush->m_Points[k];
	centroid *= 1.0f / 6.0f;

	int topIdx[3] = { 0, 1, 2 };
	int botIdx[3] = { 3, 4, 5 };
	Terrain_AddPoly(pBrush, topIdx, 3, centroid, info, pTexName, true);
	Terrain_AddPoly(pBrush, botIdx, 3, centroid, info, pTexName, false);
	for(k=0; k < 3; k++)
	{
		int a = k, b = (k + 1) % 3;
		int side[4] = { a, b, b + 3, a + 3 };
		Terrain_AddPoly(pBrush, side, 4, centroid, info, pTexName, false);
	}

	pBrush->UpdatePlanes();
	pBrush->UpdateBoundingInfo();
	return pBrush;
}

// A single one-sided triangle facing up (no sides, no bottom).
// The whole terrain as ONE brush of one-sided triangles that share their corner points.
// Sharing points inside a single brush is what lets the Processor smooth the lighting
// across the triangles (it only averages normals within a brush).
static CEditBrush* Terrain_MakeSurface(CEditRegion *pRegion, CWorldNode *pParent, const TerrainInfo &info,
	float topY, const char *pTexName)
{
	CEditBrush *pBrush = no_CreateNewBrush(pRegion, pParent);
	const int N = info.m_N;
	int i, j;

	for(j=0; j <= N; j++)
		for(i=0; i <= N; i++)
			pBrush->m_Points.Append(CEditVert(LTVector(info.GX(i), topY, info.GZ(j))));

	for(j=0; j < N; j++)
	{
		for(i=0; i < N; i++)
		{
			uint32 p00 = j * (N + 1) + i, p10 = p00 + 1, p01 = p00 + (N + 1), p11 = p01 + 1;
			uint32 tris[2][3] = { { p00, p10, p11 }, { p00, p11, p01 } };
			for(int t=0; t < 2; t++)
			{
				CEditPoly *pPoly = new CEditPoly(pBrush);
				for(int k=0; k < 3; k++)
					pPoly->m_Indices.Append(tris[t][k]);
				pPoly->UpdatePlane();
				if(pPoly->Normal().y < 0.0f)
				{
					pPoly->Flip();
					pPoly->UpdatePlane();
				}
				Terrain_SetupPolyTexture(pPoly, info, pTexName, true);
				pBrush->m_Polies.Append(pPoly);
			}
		}
	}

	pBrush->UpdatePlanes();
	pBrush->UpdateBoundingInfo();
	return pBrush;
}

static int Terrain_FreeID(CEditRegion *pRegion)
{
	std::vector<CWorldNode*> nodes;
	Terrain_FindNodes(pRegion->GetRootNode(), nodes);

	for(int id=1; id < 10000; id++)
	{
		bool bUsed = false;
		for(size_t n=0; n < nodes.size(); n++)
			if(Terrain_NodeID(nodes[n]) == id)
				bUsed = true;
		if(!bUsed && GetFileAttributes(Terrain_SidecarPath(id)) == INVALID_FILE_ATTRIBUTES)
			return id;
	}
	return -1;
}


class CTerrainConvertDlg : public CDialog
{
public:
	CTerrainConvertDlg() : CDialog(IDD_TERRAIN_CONVERT), m_N(8) {}
	int m_N;

protected:
	virtual BOOL OnInitDialog()
	{
		CDialog::OnInitDialog();
		CheckRadioButton(IDC_TERRAIN_RES8, IDC_TERRAIN_RES32, IDC_TERRAIN_RES32);
		return TRUE;
	}
	virtual void OnOK()
	{
		if(IsDlgButtonChecked(IDC_TERRAIN_RES8))		m_N = 2;
		else if(IsDlgButtonChecked(IDC_TERRAIN_RES16))	m_N = 4;
		else											m_N = 8;
		CDialog::OnOK();
	}
};


void Terrain::ConvertSelection(CRegionView *pView)
{
	CEditRegion *pRegion = pView->GetRegion();
	if(pRegion->GetNumSelections() != 1 || pRegion->GetSelection(0)->GetType() != Node_Brush)
	{
		AfxMessageBox("Select one brush to turn into terrain.");
		return;
	}

	CEditBrush *pSource = pRegion->GetSelection(0)->AsBrush();
	if(pSource->m_Points.GetSize() < 4)
		return;

	CTerrainConvertDlg dlg;
	if(dlg.DoModal() != IDOK)
		return;

	int id = Terrain_FreeID(pRegion);
	if(id < 0)
		return;

	bool bOneSided = AfxMessageBox(
		"Build the terrain from one-sided triangles?\n\n"
		"Yes: one brush of upward-facing triangles with no sides or bottom. The Processor smooths\n"
		"the lighting across it. Recommended for Jupiter.\n"
		"No: closed triangular prisms (lit as separate flat pieces).",
		MB_YESNO | MB_ICONQUESTION) == IDYES;

	// Footprint from the brush's bounds. The top of the brush becomes the
	// starting height and the bottom the flat base of every prism.
	LTVector vMin = pSource->m_Points[0], vMax = pSource->m_Points[0];
	for(uint32 i=1; i < pSource->m_Points.GetSize(); i++)
	{
		const CEditVert &pt = pSource->m_Points[i];
		if(pt.x < vMin.x) vMin.x = pt.x;
		if(pt.y < vMin.y) vMin.y = pt.y;
		if(pt.z < vMin.z) vMin.z = pt.z;
		if(pt.x > vMax.x) vMax.x = pt.x;
		if(pt.y > vMax.y) vMax.y = pt.y;
		if(pt.z > vMax.z) vMax.z = pt.z;
	}
	if(vMax.x - vMin.x < 1.0f || vMax.z - vMin.z < 1.0f)
	{
		AfxMessageBox("The brush needs some width and depth to become terrain.");
		return;
	}

	TerrainInfo info;
	info.m_N = dlg.m_N;
	info.m_x0 = vMin.x;
	info.m_z0 = vMin.z;
	info.m_sx = vMax.x - vMin.x;
	info.m_sz = vMax.z - vMin.z;
	info.m_BaseY = vMin.y;
	float topY = vMax.y;
	if(topY < info.m_BaseY + TERRAIN_MIN_THICK)
		topY = info.m_BaseY + TERRAIN_MIN_THICK;

	// Layer 1 is the brush's own texture unless the tool window says otherwise.
	const char *pSourceTex = pSource->m_Polies.GetSize() ? pSource->m_Polies[0]->GetTexture(0).m_pTextureName : NULL;
	for(int l=0; l < TERRAIN_NUM_LAYERS; l++)
		info.m_Layers[l] = (LPCTSTR)g_Layers[l];
	if(info.m_Layers[0].empty() && pSourceTex)
		info.m_Layers[0] = pSourceTex;

	info.m_Weights.assign(info.NumVerts() * TERRAIN_NUM_LAYERS, 0.0f);
	for(int v=0; v < info.NumVerts(); v++)
		info.m_Weights[v * TERRAIN_NUM_LAYERS] = 1.0f;
	Terrain_EnsurePaint(info);

	if(!Terrain_SaveInfo(id, info))
	{
		AfxMessageBox("Couldn't write to Textures\\Terrainmesh in the project folder.");
		return;
	}

	CString texName = Terrain_TextureName(id);
	const char *pTexName = pRegion->m_pStringHolder->AddString((char*)(LPCTSTR)texName);

	CWorldNode *pParent = pSource->GetParent();
	CWorldNode *pGroup = pRegion->AddNullNode(pParent);
	CString label;
	label.Format("Terrain%04d", id);
	pGroup->SetNodeLabel(label);

	if(bOneSided)
		Terrain_MakeSurface(pRegion, pGroup, info, topY, pTexName);

	for(int j=0; j < info.m_N && !bOneSided; j++)
	{
		for(int i=0; i < info.m_N; i++)
		{
			LTVector p00(info.GX(i), topY, info.GZ(j));
			LTVector p10(info.GX(i+1), topY, info.GZ(j));
			LTVector p11(info.GX(i+1), topY, info.GZ(j+1));
			LTVector p01(info.GX(i), topY, info.GZ(j+1));

			LTVector t0[3] = { p00, p10, p11 };
			LTVector t1[3] = { p00, p11, p01 };
			Terrain_MakePrism(pRegion, pGroup, info, t0, pTexName);
			Terrain_MakePrism(pRegion, pGroup, info, t1, pTexName);
		}
	}

	TerrainRef ref;
	if(Terrain_Gather(pGroup, ref))
	{
		Terrain_Bake(ref, false);
		Terrain_ReloadTexture(ref);
	}

	PreActionList actionList;
	actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, pSource));
	actionList.AddTail(new CPreAction(ACTION_ADDEDNODE, pGroup));
	pView->GetRegionDoc()->Modify(&actionList, TRUE);

	pRegion->ClearSelections();
	GetNodeView()->DeleteNode(pSource);
	no_DestroyNode(pRegion, pSource, FALSE);

	GetNodeView()->UpdateTreeItemLabel(pGroup);
	GetNodeView()->UpdateNodeImage(pRegion->GetRootNode());

	// Select the whole terrain group.
	pRegion->RecurseAndSelect(pGroup);
	pView->GetRegionDoc()->NotifySelectionChange();
	pView->GetRegionDoc()->RedrawAllViews();
}


//------------------------------------------------------------------
// Painting
//------------------------------------------------------------------

static bool			g_bStroking = false;
static TerrainRef	g_Stroke;
static float		g_FlattenHeight = 0.0f;
static bool			g_bWeightsChanged = false;

static bool Terrain_RayTri(const LTVector &orig, const LTVector &dir, const LTVector &a, const LTVector &b, const LTVector &c, float &t)
{
	LTVector e1 = b - a, e2 = c - a;
	LTVector p = dir.Cross(e2);
	float det = e1.Dot(p);
	if((float)fabs(det) < 1e-8f)
		return false;
	float inv = 1.0f / det;
	LTVector s = orig - a;
	float u = s.Dot(p) * inv;
	if(u < 0.0f || u > 1.0f)
		return false;
	LTVector q = s.Cross(e1);
	float v = dir.Dot(q) * inv;
	if(v < 0.0f || u + v > 1.0f)
		return false;
	t = e2.Dot(q) * inv;
	return t > 0.0f;
}

// Finds the terrain under the mouse. Returns the hit point in pHit.
static bool Terrain_Pick(CRegionView *pView, CPoint point, CWorldNode **ppNode, LTVector *pHit)
{
	CEditRay ray = pView->ViewDef()->MakeRayFromScreenPoint(point);
	LTVector orig = ray.m_Pos, dir = ray.m_Dir;

	std::vector<CWorldNode*> nodes;
	Terrain_FindNodes(pView->GetRegion()->GetRootNode(), nodes);

	float bestT = 1e30f;
	CWorldNode *pBest = NULL;
	for(size_t n=0; n < nodes.size(); n++)
	{
		for(GPOS pos = nodes[n]->m_Children.GetHeadPosition(); pos; )
		{
			CWorldNode *pChild = nodes[n]->m_Children.GetNext(pos);
			if(pChild->GetType() != Node_Brush)
				continue;
			CEditBrush *pBrush = pChild->AsBrush();
			if(!Terrain_IsTerrainBrush(pBrush))
				continue;

			if(Terrain_RayBrush(pBrush, orig, dir, bestT))
				pBest = nodes[n];
		}
	}

	if(!pBest)
		return false;
	*ppNode = pBest;
	*pHit = orig + dir * bestT;
	return true;
}

static int g_DabMode = Terrain::MODE_RAISE;
static bool g_bStrokeRight = false;

// The mode a stroke actually uses: Shift smooths (height modes only), and the
// right button swaps Raise and Lower.
static int Terrain_EffectiveMode(bool bRight)
{
	int mode = g_Mode;
	if(mode >= Terrain::MODE_PAINT1)
		return mode;
	if(GetKeyState(VK_SHIFT) & 0x8000)
		return Terrain::MODE_SMOOTH;
	if(bRight)
	{
		if(mode == Terrain::MODE_RAISE)	return Terrain::MODE_LOWER;
		if(mode == Terrain::MODE_LOWER)	return Terrain::MODE_RAISE;
	}
	return mode;
}

static void Terrain_Dab(TerrainRef &ref, const LTVector &hit)
{
	TerrainInfo &info = ref.m_Info;
	const int stride = info.m_N + 1;
	const float s = (float)g_Strength / 100.0f;
	float radius = g_Radius;
	if(radius < 1.0f)
		radius = 1.0f;

	std::vector<float> src = ref.m_Heights;

	if(g_DabMode >= Terrain::MODE_PAINT1)
	{
		float pad = radius + 2.0f * TERRAIN_PAINT_UNITS + 8.0f;
		if(!g_bDirty)
		{
			g_Dirty[0] = hit.x - pad; g_Dirty[1] = hit.x + pad;
			g_Dirty[2] = hit.z - pad; g_Dirty[3] = hit.z + pad;
			g_bDirty = true;
		}
		else
		{
			if(hit.x - pad < g_Dirty[0]) g_Dirty[0] = hit.x - pad;
			if(hit.x + pad > g_Dirty[1]) g_Dirty[1] = hit.x + pad;
			if(hit.z - pad < g_Dirty[2]) g_Dirty[2] = hit.z - pad;
			if(hit.z + pad > g_Dirty[3]) g_Dirty[3] = hit.z + pad;
		}
	}

	if(g_DabMode >= Terrain::MODE_PAINT1)
	{
		// paint into the fine paint map, not the height grid
		Terrain_EnsurePaint(info);
		const int PW = info.m_PW, PH = info.m_PH;
		int layer = g_DabMode - Terrain::MODE_PAINT1;
		if(layer > 1) layer = 1;	// two layers per terrain
		const float tx = info.m_sx / PW, tz = info.m_sz / PH;
		int ua = (int)floor((hit.x - radius - info.m_x0) / tx), ub = (int)ceil((hit.x + radius - info.m_x0) / tx);
		int va = (int)floor((hit.z - radius - info.m_z0) / tz), vb = (int)ceil((hit.z + radius - info.m_z0) / tz);
		if(ua < 0) ua = 0; if(ub > PW - 1) ub = PW - 1;
		if(va < 0) va = 0; if(vb > PH - 1) vb = PH - 1;
		for(int v=va; v <= vb; v++)
			for(int u=ua; u <= ub; u++)
			{
				float dx = info.m_x0 + (u + 0.5f) * tx - hit.x, dz = info.m_z0 + (v + 0.5f) * tz - hit.z;
				float d = (float)sqrt(dx*dx + dz*dz);
				if(d >= radius)
					continue;
				float f = 1.0f - d / radius;
				f = f * f * (3.0f - 2.0f * f);
				uint8 *p = &info.m_Paint[((size_t)v * PW + u) * TERRAIN_NUM_LAYERS];
				float w[TERRAIN_NUM_LAYERS];
				int l;
				for(l=0; l < TERRAIN_NUM_LAYERS; l++) w[l] = p[l] / 255.0f;
				w[layer] += s * f * 0.5f;
				if(w[layer] > 1.0f) w[layer] = 1.0f;
				float others = 0.0f;
				for(l=0; l < TERRAIN_NUM_LAYERS; l++) if(l != layer) others += w[l];
				if(others > 0.0f)
				{
					float scale = (1.0f - w[layer]) / others;
					for(l=0; l < TERRAIN_NUM_LAYERS; l++) if(l != layer) w[l] *= scale;
				}
				else
					w[layer] = 1.0f;
				for(l=0; l < TERRAIN_NUM_LAYERS; l++)
				{
					float q = w[l] * 255.0f + 0.5f;
					p[l] = (uint8)(q < 0 ? 0 : (q > 255 ? 255 : q));
				}
				g_bWeightsChanged = true;
			}
		return;
	}

	for(int j=0; j <= info.m_N; j++)
	{
		for(int i=0; i <= info.m_N; i++)
		{
			float dx = info.GX(i) - hit.x, dz = info.GZ(j) - hit.z;
			float d = (float)sqrt(dx*dx + dz*dz);
			if(d >= radius)
				continue;
			float f = 1.0f - d / radius;
			f = f * f * (3.0f - 2.0f * f);

			int g = j * stride + i;
			float &h = ref.m_Heights[g];

			switch(g_DabMode)
			{
				case Terrain::MODE_RAISE:	h += s * f * 8.0f; break;
				case Terrain::MODE_LOWER:	h -= s * f * 8.0f; break;

				case Terrain::MODE_SMOOTH:
				{
					float sum = 0.0f;
					int count = 0;
					for(int oj=-1; oj <= 1; oj++)
						for(int oi=-1; oi <= 1; oi++)
						{
							int ni = i + oi, nj = j + oj;
							if(ni < 0 || nj < 0 || ni > info.m_N || nj > info.m_N)
								continue;
							sum += src[nj * stride + ni];
							count++;
						}
					h += (sum / (float)count - h) * s * f;
					break;
				}

				case Terrain::MODE_FLATTEN:	h += (g_FlattenHeight - h) * s * f; break;

				default:
				{
					int layer = g_DabMode - Terrain::MODE_PAINT1;
					float *w = &info.m_Weights[g * TERRAIN_NUM_LAYERS];
					float add = s * f * 0.5f;
					w[layer] += add;
					if(w[layer] > 1.0f)
						w[layer] = 1.0f;
					float others = 0.0f;
					int l;
					for(l=0; l < TERRAIN_NUM_LAYERS; l++)
						if(l != layer)
							others += w[l];
					if(others > 0.0f)
					{
						float scale = (1.0f - w[layer]) / others;
						for(l=0; l < TERRAIN_NUM_LAYERS; l++)
							if(l != layer)
								w[l] *= scale;
					}
					else
					{
						w[layer] = 1.0f;
					}
					g_bWeightsChanged = true;
					break;
				}
			}

			if(h < info.m_BaseY + TERRAIN_MIN_THICK)
				h = info.m_BaseY + TERRAIN_MIN_THICK;
		}
	}

	if(g_DabMode < Terrain::MODE_PAINT1)
		Terrain_Apply(ref);
}


bool Terrain::IsStroking()
{
	return g_bStroking;
}

bool Terrain::IsStrokeRightButton()
{
	return g_bStroking && g_bStrokeRight;
}

bool Terrain::BeginStroke(CRegionView *pView, CPoint point, bool bRightButton)
{
	if(!IsActive() || (GetKeyState(VK_MENU) & 0x8000) || (GetKeyState(VK_CONTROL) & 0x8000))
		return false;

	// Shift smooths in the height modes; in the paint modes it keeps its usual meaning.
	if((GetKeyState(VK_SHIFT) & 0x8000) && g_Mode >= MODE_PAINT1)
		return false;

	// The right button only does something in Raise / Lower.
	if(bRightButton && g_Mode != MODE_RAISE && g_Mode != MODE_LOWER)
		return false;

	CWorldNode *pNode;
	LTVector hit;
	if(!Terrain_Pick(pView, point, &pNode, &hit))
		return false;
	if(!Terrain_Gather(pNode, g_Stroke))
		return false;

	g_bStrokeRight = bRightButton;
	g_DabMode = Terrain_EffectiveMode(bRightButton);

	if(g_Mode < MODE_PAINT1)
	{
		PreActionList actionList;
		for(size_t b=0; b < g_Stroke.m_Brushes.size(); b++)
			actionList.AddTail(new CPreAction(ACTION_MODIFYNODE, g_Stroke.m_Brushes[b]));
		pView->GetRegionDoc()->Modify(&actionList, TRUE);
	}

	g_FlattenHeight = hit.y;
	g_bWeightsChanged = false;
	g_bDirty = false;
	g_bStrokeDirty = false;
	g_bStroking = true;

	Terrain_Dab(g_Stroke, hit);
	pView->GetRegionDoc()->RedrawAllViews();
	return true;
}

void Terrain::ContinueStroke(CRegionView *pView, CPoint point)
{
	if(!g_bStroking)
		return;

	CWorldNode *pNode;
	LTVector hit;
	if(!Terrain_Pick(pView, point, &pNode, &hit) || pNode != g_Stroke.m_pNode)
		return;

	g_bHover = true;
	g_pHoverNode = pNode;
	g_vHover = hit;

	g_DabMode = Terrain_EffectiveMode(g_bStrokeRight);
	Terrain_Dab(g_Stroke, hit);

	// Live paint: update the texture in memory (the files are written when the stroke ends).
	static DWORD s_LastLive = 0;
	DWORD now = GetTickCount();
	if(g_DabMode >= Terrain::MODE_PAINT1 && g_bDirty && g_bWeightsChanged && now - s_LastLive >= 30)
	{
		Terrain_MergeStrokeDirty();
		Terrain_LiveUpdate(g_Stroke, g_Dirty);
		g_bDirty = false;
		s_LastLive = GetTickCount();
	}
	pView->GetRegionDoc()->RedrawAllViews();
}

void Terrain::EndStroke(CRegionView *pView)
{
	if(!g_bStroking)
		return;
	g_bStroking = false;

	if(g_bWeightsChanged)
	{
		Terrain_SaveInfo(g_Stroke.m_ID, g_Stroke.m_Info);
		Terrain_MergeStrokeDirty();
		if(g_bStrokeDirty || g_BakeCache.m_ID != g_Stroke.m_ID)
			if(Terrain_Bake(g_Stroke, false, g_bStrokeDirty ? g_StrokeDirty : NULL))
				Terrain_ReloadTexture(g_Stroke);
		g_bDirty = false;
		g_bStrokeDirty = false;
	}

	pView->GetRegionDoc()->Modify();
	pView->GetRegionDoc()->RedrawAllViews();
}


//------------------------------------------------------------------
// Tool window
//------------------------------------------------------------------

static const char *g_ModeNames[Terrain::NUM_MODES] =
{
	"Raise", "Lower", "Smooth", "Flatten",
	"Paint layer 1", "Paint layer 2", "Paint layer 3", "Paint layer 4"
};

class CTerrainToolsDlg : public CDialog
{
public:
	CTerrainToolsDlg() : m_bInit(false) {}

	BOOL Create(CWnd *pParent)	{ return CDialog::Create(IDD_TERRAIN_TOOLS, pParent); }

protected:
	bool m_bInit;

	virtual BOOL OnInitDialog()
	{
		CDialog::OnInitDialog();

		CComboBox *pCombo = (CComboBox*)GetDlgItem(IDC_TERRAIN_MODE);
		for(int i=0; i < Terrain::NUM_MODES - 2; i++)
			pCombo->AddString(g_ModeNames[i]);
		pCombo->SetCurSel(g_Mode);

		SetDlgItemInt(IDC_TERRAIN_RADIUS, (UINT)g_Radius);
		SetDlgItemInt(IDC_TERRAIN_STRENGTH, (UINT)g_Strength);
		for(int l=0; l < TERRAIN_NUM_LAYERS; l++)
			SetDlgItemText(IDC_TERRAIN_LAYER1 + l, g_Layers[l]);
		CheckDlgButton(IDC_TERRAIN_ACTIVE, g_bPaint ? BST_CHECKED : BST_UNCHECKED);

		m_bInit = true;
		return TRUE;
	}

	virtual void OnOK()		{}
	virtual void OnCancel()	{ ShowWindow(SW_HIDE); }

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam)
	{
		UINT id = LOWORD(wParam), code = HIWORD(wParam);
		if(m_bInit)
		{
			if(id == IDC_TERRAIN_MODE && code == CBN_SELCHANGE)
			{
				int sel = ((CComboBox*)GetDlgItem(IDC_TERRAIN_MODE))->GetCurSel();
				if(sel >= 0)
					g_Mode = sel;
				return TRUE;
			}
			if(id == IDC_TERRAIN_RADIUS && code == EN_CHANGE)
			{
				BOOL bOK;
				UINT v = GetDlgItemInt(IDC_TERRAIN_RADIUS, &bOK, FALSE);
				if(bOK && v > 0)
					g_Radius = (float)v;
				return TRUE;
			}
			if(id == IDC_TERRAIN_STRENGTH && code == EN_CHANGE)
			{
				BOOL bOK;
				UINT v = GetDlgItemInt(IDC_TERRAIN_STRENGTH, &bOK, FALSE);
				if(bOK)
					g_Strength = v < 1 ? 1 : (v > 100 ? 100 : (int)v);
				return TRUE;
			}
			if(id >= IDC_TERRAIN_LAYER1 && id < IDC_TERRAIN_LAYER1 + TERRAIN_NUM_LAYERS && code == EN_CHANGE)
			{
				GetDlgItemText(id, g_Layers[id - IDC_TERRAIN_LAYER1]);
				return TRUE;
			}
			if(id >= IDC_TERRAIN_USE1 && id < IDC_TERRAIN_USE1 + TERRAIN_NUM_LAYERS && code == BN_CLICKED)
			{
				char szName[MAX_PATH];
				szName[0] = 0;
				GetProjectBar()->GetCurrentTextureName(szName, FALSE);
				SetDlgItemText(IDC_TERRAIN_LAYER1 + (id - IDC_TERRAIN_USE1), szName);
				return TRUE;
			}
			if(id == IDC_TERRAIN_ACTIVE && code == BN_CLICKED)
			{
				g_bPaint = IsDlgButtonChecked(IDC_TERRAIN_ACTIVE) == BST_CHECKED;
				return TRUE;
			}
			if(id == IDC_TERRAIN_REBAKE && code == BN_CLICKED)
			{
				Rebake();
				return TRUE;
			}
		}
		return CDialog::OnCommand(wParam, lParam);
	}

	// Applies the layer textures to the selected terrains (or all of them
	// when none are selected) and bakes them again.
	void Rebake()
	{
		CRegionDoc *pDoc = GetActiveRegionDoc();
		if(!pDoc)
			return;
		CEditRegion *pRegion = pDoc->GetRegion();

		std::vector<CWorldNode*> nodes, picked;
		Terrain_FindNodes(pRegion->GetRootNode(), nodes);
		for(size_t n=0; n < nodes.size(); n++)
		{
			bool bSel = nodes[n]->IsFlagSet(NODEFLAG_SELECTED) != 0;
			for(GPOS pos = nodes[n]->m_Children.GetHeadPosition(); pos && !bSel; )
				if(nodes[n]->m_Children.GetNext(pos)->IsFlagSet(NODEFLAG_SELECTED))
					bSel = true;
			if(bSel)
				picked.push_back(nodes[n]);
		}
		if(picked.empty())
			picked = nodes;
		if(picked.empty())
		{
			AfxMessageBox("There are no terrains in this level. Use Brush > Convert to Terrain first.");
			return;
		}

		CWaitCursor wait;
		for(size_t n=0; n < picked.size(); n++)
		{
			TerrainRef ref;
			if(!Terrain_Gather(picked[n], ref))
				continue;
			for(int l=0; l < TERRAIN_NUM_LAYERS; l++)
				if(!g_Layers[l].IsEmpty())
					ref.m_Info.m_Layers[l] = (LPCTSTR)g_Layers[l];
			Terrain_SaveInfo(ref.m_ID, ref.m_Info);
			if(Terrain_Bake(ref))
				Terrain_ReloadTexture(ref);
		}
		pDoc->RedrawAllViews();
	}
};

static CTerrainToolsDlg *g_pToolsDlg = NULL;

bool Terrain::HandleMouseWheel(short zDelta)
{
	if(!IsToolsWindowVisible() || !(GetKeyState(VK_CONTROL) & 0x8000) || zDelta == 0)
		return false;

	int nSteps = zDelta / WHEEL_DELTA;
	if(nSteps == 0)
		nSteps = (zDelta > 0) ? 1 : -1;

	if(GetKeyState(VK_SHIFT) & 0x8000)
	{
		int s = g_Strength + nSteps * 5;
		g_Strength = s < 1 ? 1 : (s > 100 ? 100 : s);
		g_pToolsDlg->SetDlgItemInt(IDC_TERRAIN_STRENGTH, (UINT)g_Strength);
	}
	else
	{
		float r = g_Radius;
		for(int i=0; i < abs(nSteps); i++)
		{
			float n = (nSteps > 0) ? r * 1.1f : r / 1.1f;
			if(nSteps > 0 && n < r + 1.0f) n = r + 1.0f;
			if(nSteps < 0 && n > r - 1.0f) n = r - 1.0f;
			r = n;
		}
		int ri = (int)(r + 0.5f);
		if(ri < 1) ri = 1;
		if(ri > 4096) ri = 4096;
		g_Radius = (float)ri;
		g_pToolsDlg->SetDlgItemInt(IDC_TERRAIN_RADIUS, (UINT)ri);
	}

	CRegionDoc *pDoc = GetActiveRegionDoc();
	if(pDoc)
		pDoc->RedrawAllViews();
	return true;
}

void Terrain::ToggleToolsWindow()
{
	if(!g_pToolsDlg)
	{
		g_pToolsDlg = new CTerrainToolsDlg;
		if(!g_pToolsDlg->Create(AfxGetMainWnd()))
		{
			delete g_pToolsDlg;
			g_pToolsDlg = NULL;
			return;
		}
		g_pToolsDlg->ShowWindow(SW_SHOW);
		return;
	}

	g_pToolsDlg->ShowWindow(g_pToolsDlg->IsWindowVisible() ? SW_HIDE : SW_SHOW);
}

bool Terrain::IsToolsWindowVisible()
{
	return g_pToolsDlg && g_pToolsDlg->GetSafeHwnd() && g_pToolsDlg->IsWindowVisible();
}

bool Terrain::IsActive()
{
	return g_bPaint && IsToolsWindowVisible();
}


//------------------------------------------------------------------
// Reticle
//------------------------------------------------------------------


// Height of the terrain's surface at (x, z), from its prism tops.
static bool Terrain_HeightAt(CWorldNode *pNode, float x, float z, float &y)
{
	LTVector orig(x, 1e6f, z), dir(0.0f, -1.0f, 0.0f);
	float bestT = 1e30f;
	for(GPOS pos = pNode->m_Children.GetHeadPosition(); pos; )
	{
		CWorldNode *pChild = pNode->m_Children.GetNext(pos);
		if(pChild->GetType() != Node_Brush)
			continue;
		CEditBrush *pBrush = pChild->AsBrush();
		if(!Terrain_IsTerrainBrush(pBrush))
			continue;
		Terrain_RayBrush(pBrush, orig, dir, bestT);
	}
	if(bestT > 1e29f)
		return false;
	y = orig.y - bestT;
	return true;
}

static bool Terrain_NodeInRegion(CEditRegion *pRegion, CWorldNode *pNode)
{
	std::vector<CWorldNode*> nodes;
	Terrain_FindNodes(pRegion->GetRootNode(), nodes);
	for(size_t n=0; n < nodes.size(); n++)
		if(nodes[n] == pNode)
			return true;
	return false;
}

void Terrain::UpdateHover(CRegionView *pView, CPoint point)
{
	if(!IsActive())
	{
		if(g_bHover)
		{
			g_bHover = false;
			pView->GetRegionDoc()->RedrawAllViews();
		}
		return;
	}

	CWorldNode *pNode;
	LTVector hit;
	bool bHit = Terrain_Pick(pView, point, &pNode, &hit);
	if(!bHit && !g_bHover)
		return;

	g_bHover = bHit;
	if(bHit)
	{
		g_pHoverNode = pNode;
		g_vHover = hit;

		// Show the layers of the terrain under the cursor in the tool window.
		static CWorldNode *s_pSynced = NULL;
		if(pNode != s_pSynced)
		{
			s_pSynced = pNode;
			TerrainRef r;
			if(Terrain_Gather(pNode, r))
			{
				for(int l=0; l < TERRAIN_NUM_LAYERS; l++)
				{
					g_Layers[l] = r.m_Info.m_Layers[l].c_str();
					if(g_pToolsDlg && g_pToolsDlg->GetSafeHwnd())
						g_pToolsDlg->SetDlgItemText(IDC_TERRAIN_LAYER1 + l, g_Layers[l]);
				}
			}
		}
	}
	pView->GetRegionDoc()->RedrawAllViews();
}

void Terrain::DrawReticle(DrawBase *pDraw)
{
	if(!g_bHover || !IsActive() || !g_pHoverNode)
		return;
	if(!Terrain_NodeInRegion(pDraw->m_pView->GetRegion(), g_pHoverNode))
	{
		g_bHover = false;
		return;
	}

	const DWORD color = 0xFF30FF30;
	const DWORD centerColor = 0xFFFFFF30;
	const int nSegs = 40;
	const float lift = 1.0f;

	pDraw->ZEnable(FALSE);

	// Brush circle draped over the surface.
	LTVector prev;
	for(int i=0; i <= nSegs; i++)
	{
		float a = (float)i / (float)nSegs * 6.2831853f;
		float x = g_vHover.x + (float)cos(a) * g_Radius;
		float z = g_vHover.z + (float)sin(a) * g_Radius;
		float y = g_vHover.y;
		Terrain_HeightAt(g_pHoverNode, x, z, y);
		LTVector cur(x, y + lift, z);
		if(i > 0)
			pDraw->TransformAndDrawLine(prev, cur, color);
		prev = cur;
	}

	// Inner ring at half radius and a centre spike scaled by strength.
	for(int i=0; i <= nSegs / 2; i++)
	{
		float a = (float)i / (float)(nSegs / 2) * 6.2831853f;
		float x = g_vHover.x + (float)cos(a) * g_Radius * 0.5f;
		float z = g_vHover.z + (float)sin(a) * g_Radius * 0.5f;
		float y = g_vHover.y;
		Terrain_HeightAt(g_pHoverNode, x, z, y);
		LTVector cur(x, y + lift, z);
		if(i > 0)
			pDraw->TransformAndDrawLine(prev, cur, color);
		prev = cur;
	}

	float spike = 8.0f + g_Radius * 0.25f * (float)g_Strength / 100.0f;
	{
		int eff = Terrain_EffectiveMode(false);
		if(eff == MODE_LOWER)
			spike = -spike;
		else if(eff == MODE_SMOOTH)
			spike *= 0.25f;
	}
	LTVector top(g_vHover.x, g_vHover.y + spike, g_vHover.z);
	pDraw->TransformAndDrawLine(g_vHover, top, centerColor);
	float c = g_Radius * 0.08f + 2.0f;
	pDraw->TransformAndDrawLine(LTVector(g_vHover.x - c, g_vHover.y + lift, g_vHover.z), LTVector(g_vHover.x + c, g_vHover.y + lift, g_vHover.z), centerColor);
	pDraw->TransformAndDrawLine(LTVector(g_vHover.x, g_vHover.y + lift, g_vHover.z - c), LTVector(g_vHover.x, g_vHover.y + lift, g_vHover.z + c), centerColor);

	pDraw->ZEnable(pDraw->m_pView->IsPerspectiveViewType());
}

bool Terrain::BlocksSelection()
{
	if(!IsToolsWindowVisible())
		return false;

	// Let the navigation hotkeys (move / rotate / orbit / draw poly) through.
	static const int navKeys[] = { 'I', 'O', 'L', VK_SPACE };
	for(int i=0; i < sizeof(navKeys) / sizeof(navKeys[0]); i++)
		if(GetKeyState(navKeys[i]) & 0x8000)
			return false;
	return true;
}

// Called by World > Process: bakes every terrain whose texture is still a
// painting preview (or missing) at full resolution.
void Terrain_BakeAllForProcess(CEditRegion *pRegion)
{
	if(!pRegion)
		return;
	std::vector<CWorldNode*> nodes;
	Terrain_FindNodes(pRegion->GetRootNode(), nodes);
	CWaitCursor wait;
	for(size_t n=0; n < nodes.size(); n++)
	{
		TerrainRef ref;
		if(!Terrain_Gather(nodes[n], ref))
			continue;
		CString dds = Terrain_ProjectDir() + Terrain_DdsName(ref.m_ID);
		unsigned int w = 0, h = 0;
		FILE *f = fopen(dds, "rb");
		if(f)
		{
			unsigned char hdr[32];
			if(fread(hdr, 1, sizeof(hdr), f) == sizeof(hdr) && !memcmp(hdr, "TEXR", 4))
			{
				h = *(unsigned int*)&hdr[12 + 12];
				w = *(unsigned int*)&hdr[12 + 16];
			}
			fclose(f);
		}
		int fw, fh;
		Terrain_TexDims(ref.m_Info, true, fw, fh);
		bool bGameOK = !Terrain_CanSplat(ref.m_Info) ||
			GetFileAttributes(Terrain_ProjectDir() + Terrain_GameMatName(ref.m_ID, 0)) != INVALID_FILE_ATTRIBUTES;
		if(w == (unsigned int)fw && h == (unsigned int)fh && bGameOK)
		{
			Terrain_ReloadTexture(ref);	// makes sure the polys carry the game mapping
			continue;	// already full resolution
		}
		if(Terrain_Bake(ref))
			Terrain_ReloadTexture(ref);
	}
}
