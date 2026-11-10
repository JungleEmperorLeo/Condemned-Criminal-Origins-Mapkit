//------------------------------------------------------------------
//
//	FILE	  : LightPreview.cpp
//
//	PURPOSE	  : See LightPreview.h.
//
//	Lighting model: the Processor's (LightMapMaker.cpp CalcLightSample):
//	  - Light / DirLight colour = (LightColor, or InnerColor if present) * |BrightScale|
//	  - distance falloff from the light's "Attenuation" property: Quartic (default,
//	    (1 - d^2/R^2)^2), Linear, or D3D (1 / (A + B*d + C*d^2) from AttCoefs / AttExps)
//	  - Lambert against the direction to the light; DirLights are limited to a cone
//	    of FOV degrees and fade to nothing at its edge
//	  - Lights with ClipLight set are shadowed by brushes that have ClipLight set
//	    (the default). Brushes that don't receive shadows skip the test.
//	  - Only brushes whose Lighting is Lightmap / Gouraud / ShadowMesh and
//	    that have ReceiveLight set show the preview; Flat brushes keep the
//	    editor's normal face shading.
//	  - The world's "AmbientLight r g b" (info string) is added everywhere.
//	  - Vertex normals are smoothed within a brush by its CreaseAngle (default 45),
//	    like the Processor does, and interpolated over each polygon.
//	  Not modelled: StaticSunLight, brush ambient, light groups.
//
//------------------------------------------------------------------

#include "bdefs.h"
#include "LightPreview.h"
#include "editregion.h"
#include "editbrush.h"
#include "editpoly.h"
#include "editobjects.h"
#include "worldnode.h"
#include "proplist.h"
#include "geomroutines.h"
#include "draw_d3d.h"
#include "d3d_texturemgr.h"

#include <vector>
#include <map>
#include <math.h>


#define LP_PAGE_SIZE		256
#define LP_DEFAULT_GRID		16.0f
#define LP_MAX_LM_SIZE		62		// texels per side, so a block plus border fits easily
#define LP_SURFACE_OFFSET	1.0f	// samples are lifted off the surface by this much


// ------------------------------------------------------------------
// Data
// ------------------------------------------------------------------

struct LPLight
{
	LTVector	m_vPos;
	float		m_fRadius;
	float		m_fRadiusSqr;
	LTVector	m_vInner;		// LightColor / InnerColor * |BrightScale|
	bool		m_bShadows;
	bool		m_bDirectional;
	LTVector	m_vForward;
	float		m_fCosHalfFOV;

	// Distance attenuation, as the Processor does it (the light's "Attenuation" property).
	enum { ATT_D3D, ATT_QUARTIC, ATT_LINEAR };
	int			m_eAtt;
	float		m_fCoA, m_fCoB, m_fCoC;		// D3D: 1 / (A + B*d + C*d^2)
};

struct LPPoly
{
	LTVector	m_vNormal;
	float		m_fDist;
	uint32		m_nFirstVert;
	uint32		m_nNumVerts;
	LTVector	m_vMin, m_vMax;
	bool		m_bOccluder;
	bool		m_bReceiveShadows;
	bool		m_bGouraud;		// Lighting = Gouraud: lit at the corners only, interpolated
};

struct LPLightmap
{
	uint32		m_nPage;
	uint32		m_nX, m_nY;		// top-left of the lightmap inside the page
	LTVector	m_vU, m_vV;		// in-plane axes
	float		m_fSMin, m_fTMin;
	float		m_fStep;
};

struct LPRenderTextures
{
	D3DRender							*m_pRender;
	TextureFormat						*m_pFormat;
	std::vector<LPDIRECTDRAWSURFACE4>	m_Surfaces;
	std::vector<IDirect3DTexture2*>		m_Textures;
};

static CEditRegion							*g_pRegion = NULL;
static std::map<CEditPoly*, uint32>			g_PolyMap;
static std::vector<LPLightmap>				g_Lightmaps;
static std::vector< std::vector<uint8> >	g_Pages;		// RGB, LP_PAGE_SIZE^2 * 3
static std::vector<LPRenderTextures*>		g_Renders;


// ------------------------------------------------------------------
// Property helpers
// ------------------------------------------------------------------

static CBaseProp* LP_FindProp(CWorldNode *pNode, const char *pName, int type1, int type2 = -1)
{
	CBaseProp *pProp = pNode->m_PropList.GetProp(pName);
	if(pProp && (pProp->GetType() == type1 || pProp->GetType() == type2))
		return pProp;
	return NULL;
}

static float LP_GetReal(CWorldNode *pNode, const char *pName, float fDefault)
{
	CBaseProp *pProp = LP_FindProp(pNode, pName, LT_PT_REAL, LT_PT_LONGINT);
	return pProp ? ((CRealProp*)pProp)->m_Value : fDefault;
}

static LTVector LP_GetVec(CWorldNode *pNode, const char *pName, const LTVector &vDefault)
{
	CBaseProp *pProp = LP_FindProp(pNode, pName, LT_PT_VECTOR, LT_PT_COLOR);
	return pProp ? ((CVectorProp*)pProp)->m_Vector : vDefault;
}

static bool LP_GetBool(CWorldNode *pNode, const char *pName, bool bDefault)
{
	CBaseProp *pProp = LP_FindProp(pNode, pName, LT_PT_BOOL);
	return pProp ? (((CBoolProp*)pProp)->m_Value != 0) : bDefault;
}

static const char* LP_GetString(CWorldNode *pNode, const char *pName, const char *pDefault)
{
	CBaseProp *pProp = LP_FindProp(pNode, pName, LT_PT_STRING);
	return pProp ? ((CStringProp*)pProp)->GetString() : pDefault;
}

// Orthogonal in-plane axes for a poly with normal n, picked by the largest
// normal component. Any in-plane pair would do; using the usual planar axes
// keeps lightmap rows lined up with the axis-aligned geometry.
static void LP_GetBaseAxes(const LTVector &n, LTVector &U, LTVector &V)
{
	float ax = (float)fabs(n.x), ay = (float)fabs(n.y), az = (float)fabs(n.z);

	if(ay >= ax && ay >= az)
	{
		U.Init(n.y > 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f);
		V.Init(0.0f, 0.0f, -1.0f);
	}
	else if(ax >= az)
	{
		U.Init(0.0f, 0.0f, n.x > 0.0f ? 1.0f : -1.0f);
		V.Init(0.0f, -1.0f, 0.0f);
	}
	else
	{
		U.Init(n.z > 0.0f ? -1.0f : 1.0f, 0.0f, 0.0f);
		V.Init(0.0f, -1.0f, 0.0f);
	}
}


// ------------------------------------------------------------------
// Ray casting against the level's polies through a uniform grid
// ------------------------------------------------------------------

class CLPRayGrid
{
public:
	std::vector<LPPoly>				*m_pPolies;
	std::vector<LTVector>			*m_pVerts;

	LTVector						m_vMin;
	float							m_fCell;
	int								m_nDim[3];
	std::vector< std::vector<uint32> >	m_Cells;
	std::vector<uint32>				m_Stamps;
	uint32							m_nStamp;

	void Build(std::vector<LPPoly> &polies, std::vector<LTVector> &verts)
	{
		m_pPolies = &polies;
		m_pVerts = &verts;
		m_nStamp = 0;
		m_Stamps.assign(polies.size(), 0);

		LTVector vMin(1e30f, 1e30f, 1e30f), vMax(-1e30f, -1e30f, -1e30f);
		uint32 i;
		for(i=0; i < polies.size(); i++)
		{
			if(!polies[i].m_bOccluder)
				continue;
			for(int k=0; k < 3; k++)
			{
				if(polies[i].m_vMin[k] < vMin[k]) vMin[k] = polies[i].m_vMin[k];
				if(polies[i].m_vMax[k] > vMax[k]) vMax[k] = polies[i].m_vMax[k];
			}
		}
		if(vMin.x > vMax.x)
		{
			vMin.Init(0, 0, 0);
			vMax.Init(1, 1, 1);
		}

		vMin -= LTVector(1, 1, 1);
		vMax += LTVector(1, 1, 1);
		m_vMin = vMin;

		float fLargest = vMax.x - vMin.x;
		if(vMax.y - vMin.y > fLargest) fLargest = vMax.y - vMin.y;
		if(vMax.z - vMin.z > fLargest) fLargest = vMax.z - vMin.z;

		m_fCell = 128.0f;
		if(fLargest / m_fCell > 96.0f)
			m_fCell = fLargest / 96.0f;

		for(int k=0; k < 3; k++)
		{
			m_nDim[k] = (int)((vMax[k] - vMin[k]) / m_fCell) + 1;
			if(m_nDim[k] < 1) m_nDim[k] = 1;
		}

		m_Cells.clear();
		m_Cells.resize((size_t)m_nDim[0] * m_nDim[1] * m_nDim[2]);

		for(i=0; i < polies.size(); i++)
		{
			const LPPoly &poly = polies[i];
			if(!poly.m_bOccluder)
				continue;

			int c0[3], c1[3];
			for(int k=0; k < 3; k++)
			{
				c0[k] = Clamp((int)((poly.m_vMin[k] - m_vMin[k]) / m_fCell), m_nDim[k]);
				c1[k] = Clamp((int)((poly.m_vMax[k] - m_vMin[k]) / m_fCell), m_nDim[k]);
			}

			for(int z=c0[2]; z <= c1[2]; z++)
				for(int y=c0[1]; y <= c1[1]; y++)
					for(int x=c0[0]; x <= c1[0]; x++)
						m_Cells[CellIndex(x, y, z)].push_back(i);
		}
	}

	// True if anything (other than skipPoly) is between a and b.
	bool Blocked(const LTVector &a, const LTVector &b, uint32 skipPoly)
	{
		if(++m_nStamp == 0)
		{
			m_Stamps.assign(m_Stamps.size(), 0);
			m_nStamp = 1;
		}

		LTVector dir = b - a;

		// 3D DDA from a to b.
		float pos[3], d[3];
		int cell[3], step[3];
		float tMax[3], tDelta[3];
		for(int k=0; k < 3; k++)
		{
			pos[k] = (a[k] - m_vMin[k]) / m_fCell;
			d[k] = dir[k] / m_fCell;
			cell[k] = (int)floor(pos[k]);
			if(d[k] > 0.0f)
			{
				step[k] = 1;
				tDelta[k] = 1.0f / d[k];
				tMax[k] = ((float)(cell[k] + 1) - pos[k]) / d[k];
			}
			else if(d[k] < 0.0f)
			{
				step[k] = -1;
				tDelta[k] = -1.0f / d[k];
				tMax[k] = (pos[k] - (float)cell[k]) / -d[k];
			}
			else
			{
				step[k] = 0;
				tDelta[k] = 1e30f;
				tMax[k] = 1e30f;
			}
		}

		for(int nIter=0; nIter < 4096; nIter++)
		{
			if(cell[0] >= 0 && cell[1] >= 0 && cell[2] >= 0 &&
				cell[0] < m_nDim[0] && cell[1] < m_nDim[1] && cell[2] < m_nDim[2])
			{
				std::vector<uint32> &list = m_Cells[CellIndex(cell[0], cell[1], cell[2])];
				for(uint32 i=0; i < list.size(); i++)
				{
					uint32 iPoly = list[i];
					if(iPoly == skipPoly || m_Stamps[iPoly] == m_nStamp)
						continue;
					m_Stamps[iPoly] = m_nStamp;

					if(SegmentHitsPoly(a, b, (*m_pPolies)[iPoly]))
						return true;
				}
			}

			// Next cell.
			int axis = 0;
			if(tMax[1] < tMax[axis]) axis = 1;
			if(tMax[2] < tMax[axis]) axis = 2;
			if(tMax[axis] > 1.0f)
				break;
			cell[axis] += step[axis];
			tMax[axis] += tDelta[axis];
		}

		return false;
	}

private:

	int CellIndex(int x, int y, int z) const
	{
		return (z * m_nDim[1] + y) * m_nDim[0] + x;
	}

	static int Clamp(int v, int dim)
	{
		if(v < 0) return 0;
		if(v >= dim) return dim - 1;
		return v;
	}

	bool SegmentHitsPoly(const LTVector &a, const LTVector &b, const LPPoly &poly) const
	{
		float da = poly.m_vNormal.Dot(a) - poly.m_fDist;
		float db = poly.m_vNormal.Dot(b) - poly.m_fDist;
		if((da > 0.0f && db > 0.0f) || (da < 0.0f && db < 0.0f))
			return false;

		float denom = da - db;
		if((float)fabs(denom) < 1e-6f)
			return false;

		float t = da / denom;
		if(t <= 0.001f || t >= 0.999f)
			return false;

		LTVector hit = a + (b - a) * t;

		// Inside test that doesn't care about winding direction.
		const std::vector<LTVector> &verts = *m_pVerts;
		bool bPos = false, bNeg = false;
		for(uint32 i=0; i < poly.m_nNumVerts; i++)
		{
			const LTVector &v0 = verts[poly.m_nFirstVert + i];
			const LTVector &v1 = verts[poly.m_nFirstVert + ((i + 1) % poly.m_nNumVerts)];
			float side = (v1 - v0).Cross(hit - v0).Dot(poly.m_vNormal);
			if(side > 0.01f) bPos = true;
			else if(side < -0.01f) bNeg = true;
			if(bPos && bNeg)
				return false;
		}
		return true;
	}
};


// ------------------------------------------------------------------
// Render textures
// ------------------------------------------------------------------

static void LP_ReleaseRenderTextures(LPRenderTextures *pRT)
{
	for(uint32 i=0; i < pRT->m_Textures.size(); i++)
		if(pRT->m_Textures[i])
			pRT->m_Textures[i]->Release();
	for(uint32 j=0; j < pRT->m_Surfaces.size(); j++)
		if(pRT->m_Surfaces[j])
			pRT->m_Surfaces[j]->Release();
	pRT->m_Textures.clear();
	pRT->m_Surfaces.clear();
}

static void LP_ReleaseAllRenders()
{
	for(uint32 i=0; i < g_Renders.size(); i++)
	{
		LP_ReleaseRenderTextures(g_Renders[i]);
		delete g_Renders[i];
	}
	g_Renders.clear();
}

static uint32 LP_CountBits(DWORD mask)
{
	uint32 n = 0;
	for(; mask; mask >>= 1)
		n += (mask & 1);
	return n;
}

static uint32 LP_LowBit(DWORD mask)
{
	uint32 n = 0;
	if(!mask)
		return 0;
	while(!(mask & 1))
	{
		mask >>= 1;
		n++;
	}
	return n;
}

// Picks the most precise opaque RGB texture format the device offers. The lightmap
// needs 8 bits per channel where possible: a 16 bit format has only 32 levels and the
// 2x brightening makes the steps between them show up as bands.
static TextureFormat* LP_ChooseFormat(D3DRender *pRender)
{
	TextureFormat *pBest = NULL;
	uint32 nBestBits = 0;
	for(DLink *pCur = pRender->m_TextureFormats.m_pNext; pCur != &pRender->m_TextureFormats; pCur = pCur->m_pNext)
	{
		TextureFormat *pFmt = (TextureFormat*)pCur->m_pData;
		if(!(pFmt->m_PF.dwFlags & DDPF_RGB) || (pFmt->m_PF.dwFlags & (DDPF_PALETTEINDEXED1 | DDPF_PALETTEINDEXED2 |
			DDPF_PALETTEINDEXED4 | DDPF_PALETTEINDEXED8)))
			continue;
		DWORD nBitCount = pFmt->m_PF.dwRGBBitCount;
		if(nBitCount != 16 && nBitCount != 32)
			continue;
		uint32 nBits = LP_CountBits(pFmt->m_PF.dwRBitMask) + LP_CountBits(pFmt->m_PF.dwGBitMask) + LP_CountBits(pFmt->m_PF.dwBBitMask);
		if(nBits < 15)
			continue;
		// Prefer formats with no alpha channel (so nothing but the colour is stored).
		if(pFmt->m_PF.dwFlags & DDPF_ALPHAPIXELS)
			nBits -= 1;
		if(nBits > nBestBits)
		{
			nBestBits = nBits;
			pBest = pFmt;
		}
	}
	return pBest ? pBest : pRender->m_pTextureFormat;
}

static bool LP_UploadPage(TextureFormat *pFmt, const std::vector<uint8> &page, LPDIRECTDRAWSURFACE4 pSurface)
{
	DDSURFACEDESC2 desc;
	memset(&desc, 0, sizeof(desc));
	desc.dwSize = sizeof(desc);

	DWORD nBits = pFmt->m_PF.dwRGBBitCount;
	if(nBits != 16 && nBits != 32)
		return false;

	if(pSurface->Lock(NULL, &desc, DDLOCK_WAIT, NULL) != DD_OK)
		return false;

	DWORD rMask = pFmt->m_PF.dwRBitMask, gMask = pFmt->m_PF.dwGBitMask, bMask = pFmt->m_PF.dwBBitMask;
	uint32 rBits = LP_CountBits(rMask), gBits = LP_CountBits(gMask), bBits = LP_CountBits(bMask);
	uint32 rShift = LP_LowBit(rMask), gShift = LP_LowBit(gMask), bShift = LP_LowBit(bMask);
	DWORD aOpaque = 0;
	if(pFmt->m_PF.dwFlags & DDPF_ALPHAPIXELS)
		aOpaque = pFmt->m_PF.dwRGBAlphaBitMask;

	// A 4x4 ordered dither (matters for 16 bit formats) turns the rounding error into
	// fine noise instead of flat bands.
	static const int bayer[4][4] = { {0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5} };

	for(uint32 y=0; y < LP_PAGE_SIZE; y++)
	{
		const uint8 *pIn = &page[y * LP_PAGE_SIZE * 3];
		uint8 *pRow = (uint8*)desc.lpSurface + y * desc.lPitch;
		for(uint32 x=0; x < LP_PAGE_SIZE; x++)
		{
			float fDither = ((float)bayer[y & 3][x & 3] + 0.5f) / 16.0f;	// 0..1
			DWORD pix = aOpaque;
			uint32 c[3] = { pIn[0], pIn[1], pIn[2] };
			uint32 bits[3] = { rBits, gBits, bBits };
			uint32 shift[3] = { rShift, gShift, bShift };
			for(int k=0; k < 3; k++)
			{
				uint32 nMax = (1u << bits[k]) - 1;
				uint32 v;
				if(bits[k] >= 8)
					v = c[k] << (bits[k] - 8);
				else
				{
					float f = (float)c[k] * (float)nMax / 255.0f + fDither - 0.5f;
					int iv = (int)floor(f + 0.5f);
					v = iv < 0 ? 0 : (iv > (int)nMax ? nMax : (uint32)iv);
				}
				pix |= (v << shift[k]);
			}
			if(nBits == 32)
				((DWORD*)pRow)[x] = pix;
			else
				((WORD*)pRow)[x] = (WORD)pix;
			pIn += 3;
		}
	}

	pSurface->Unlock(NULL);
	return true;
}

static LPRenderTextures* LP_GetRenderTextures(D3DRender *pRender)
{
	for(uint32 i=0; i < g_Renders.size(); i++)
		if(g_Renders[i]->m_pRender == pRender)
			return g_Renders[i];

	LPRenderTextures *pRT = new LPRenderTextures;
	pRT->m_pRender = pRender;
	pRT->m_pFormat = NULL;

	pRT->m_pFormat = LP_ChooseFormat(pRender);
	if(pRT->m_pFormat)
	{
		for(uint32 i=0; i < g_Pages.size(); i++)
		{
			IDirect3DTexture2 *pTexture = NULL;
			LPDIRECTDRAWSURFACE4 pSurface = d3d_CreateTextureSurface2(pRender, LP_PAGE_SIZE, LP_PAGE_SIZE, 0, pRT->m_pFormat);
			if(pSurface)
			{
				pSurface->QueryInterface(IID_IDirect3DTexture2, (void**)&pTexture);
				if(!pTexture || !LP_UploadPage(pRT->m_pFormat, g_Pages[i], pSurface))
				{
					if(pTexture)
						pTexture->Release();
					pTexture = NULL;
					pSurface->Release();
					pSurface = NULL;
				}
			}
			pRT->m_Surfaces.push_back(pSurface);
			pRT->m_Textures.push_back(pTexture);
		}
	}

	g_Renders.push_back(pRT);
	return pRT;
}


// ------------------------------------------------------------------
// Lighting
// ------------------------------------------------------------------

int DEdit_LightmapMul();

static void LP_GatherLights(CEditRegion *pRegion, std::vector<LPLight> &lights)
{
	for(uint32 i=0; i < pRegion->m_Objects.GetSize(); i++)
	{
		CBaseEditObj *pObj = pRegion->m_Objects[i];
		const char *pClass = pObj->GetClassName();
		// Condemned / F.E.A.R. light classes.
		bool bCdmPoint = !stricmp(pClass, "LightPoint") || !stricmp(pClass, "LightPointFill") || !stricmp(pClass, "LightCube");
		bool bCdmSpot = !stricmp(pClass, "LightSpot");
		bool bCdmDir = !stricmp(pClass, "LightDirectional");
		if(bCdmPoint || bCdmSpot || bCdmDir)
		{
			LPLight light;
			light.m_vPos = pObj->GetPos();
			LTVector vDims = LP_GetVec(pObj, "Dims", LTVector(256.0f, 256.0f, 1024.0f));
			light.m_fRadius = bCdmDir ? (float)fabs(vDims.z) * 2.0f : LP_GetReal(pObj, "LightRadius", 500.0f);
			if(bCdmSpot && light.m_fRadius <= 0.0f) light.m_fRadius = LP_GetReal(pObj, "FarZ", 500.0f);
			light.m_fRadiusSqr = light.m_fRadius * light.m_fRadius;
			LTVector vColor = LP_GetVec(pObj, "LightColor", LTVector(255.0f, 255.0f, 255.0f));
			// LightColor 255 = the texture at its own brightness, whatever the preview's modulate (1x/2x/4x) is.
			light.m_vInner = vColor * ((float)fabs(LP_GetReal(pObj, "IntensityScale", 1.0f)) / (float)DEdit_LightmapMul());
			light.m_bShadows = stricmp(pClass, "LightPointFill") != 0;
			light.m_bDirectional = bCdmSpot || bCdmDir;
			light.m_vForward.Init(0.0f, 0.0f, 1.0f);
			light.m_fCosHalfFOV = -1.0f;
			light.m_eAtt = bCdmDir ? LPLight::ATT_LINEAR : LPLight::ATT_QUARTIC;
			light.m_fCoA = 1.0f; light.m_fCoB = 0.0f; light.m_fCoC = 19.0f / light.m_fRadiusSqr;
			if(light.m_bDirectional)
			{
				CBaseProp *pRot = LP_FindProp(pObj, "Rotation", LT_PT_ROTATION);
				if(pRot)
				{
					LTVector vRight, vUp;
					gr_GetEulerVectors(((CRotationProp*)pRot)->GetEulerAngles(), vRight, vUp, light.m_vForward);
				}
				float fFOV = bCdmDir ? 170.0f : (LP_GetReal(pObj, "FovX", 90.0f) > LP_GetReal(pObj, "FovY", 90.0f) ? LP_GetReal(pObj, "FovX", 90.0f) : LP_GetReal(pObj, "FovY", 90.0f));
				light.m_fCosHalfFOV = (float)cos(MATH_DEGREES_TO_RADIANS(fFOV * 0.5f));
			}
			if(light.m_fRadius > 0.0f)
				lights.push_back(light);
			continue;
		}

		bool bDir = (stricmp(pClass, "DirLight") == 0);
		if(!bDir && stricmp(pClass, "Light") != 0)
			continue;	// (ObjectLights only light objects, not the world)

		LPLight light;
		light.m_vPos = pObj->GetPos();
		light.m_fRadius = LP_GetReal(pObj, "LightRadius", 300.0f);
		light.m_fRadiusSqr = light.m_fRadius * light.m_fRadius;

		// The Processor reads LightColor, then InnerColor over it, and scales by |BrightScale|.
		LTVector vColor(0.0f, 0.0f, 0.0f);
		vColor = LP_GetVec(pObj, "LightColor", vColor);
		vColor = LP_GetVec(pObj, "InnerColor", vColor);
		float fBright = (float)fabs(LP_GetReal(pObj, "BrightScale", 1.0f));
		light.m_vInner = vColor * fBright;

		light.m_bShadows = LP_GetBool(pObj, "ClipLight", true);
		light.m_bDirectional = bDir;
		light.m_vForward.Init(0.0f, 0.0f, 1.0f);
		light.m_fCosHalfFOV = -1.0f;

		// Attenuation type (default Quartic) and the D3D coefficients.
		light.m_eAtt = LPLight::ATT_QUARTIC;
		const char *pAtt = LP_GetString(pObj, "Attenuation", "Quartic");
		if(stricmp(pAtt, "D3D") == 0)			light.m_eAtt = LPLight::ATT_D3D;
		else if(stricmp(pAtt, "Linear") == 0)	light.m_eAtt = LPLight::ATT_LINEAR;

		LTVector vCoefs = LP_GetVec(pObj, "AttCoefs", LTVector(1.0f, 0.0f, 19.0f));
		LTVector vExps = LP_GetVec(pObj, "AttExps", LTVector(0.0f, 0.0f, -2.0f));
		light.m_fCoA = vCoefs.x * (float)pow(light.m_fRadius, vExps.x);
		light.m_fCoB = vCoefs.y * (float)pow(light.m_fRadius, vExps.y);
		light.m_fCoC = vCoefs.z * (float)pow(light.m_fRadius, vExps.z);

		if(bDir)
		{
			CBaseProp *pRot = LP_FindProp(pObj, "Rotation", LT_PT_ROTATION);
			if(pRot)
			{
				LTVector vRight, vUp;
				gr_GetEulerVectors(((CRotationProp*)pRot)->GetEulerAngles(), vRight, vUp, light.m_vForward);
			}
			float fFOV = LP_GetReal(pObj, "FOV", 90.0f);
			light.m_fCosHalfFOV = (float)cos(MATH_DEGREES_TO_RADIANS(fFOV * 0.5f));
		}

		if(light.m_fRadius > 0.0f)
			lights.push_back(light);
	}
}

// Adds one light's contribution at a sample point, the way the Processor's
// CalcLightSample does.
static void LP_AddLight(const LPLight &light, const LTVector &vSample, const LTVector &n,
	bool bShadows, CLPRayGrid &grid, uint32 iSkipPoly, LTVector &vColor)
{
	LTVector vToLight = light.m_vPos - vSample;
	float fDistSqr = vToLight.MagSqr();
	if(fDistSqr > light.m_fRadiusSqr)
		return;

	float fDist = (float)sqrt(fDistSqr);
	if(fDist < 0.001f)
		return;
	vToLight /= fDist;

	// Lambert, against the direction to the light (spot lights too).
	float fLambert = vToLight.Dot(n);
	if(fLambert <= 0.0f)
		return;

	float fAnglePct = 1.0f;
	if(light.m_bDirectional)
	{
		float fDot = -vToLight.Dot(light.m_vForward);
		if(fDot < light.m_fCosHalfFOV)
			return;	// outside the cone
		float fRange = 1.0f - light.m_fCosHalfFOV;
		fAnglePct = (fRange > 1e-5f) ? (fDot - light.m_fCosHalfFOV) / fRange : 1.0f;
	}

	float fDistPct;
	switch(light.m_eAtt)
	{
		case LPLight::ATT_D3D:
			fDistPct = 1.0f / (light.m_fCoA + light.m_fCoB * fDist + light.m_fCoC * fDistSqr);
			break;
		case LPLight::ATT_LINEAR:
			fDistPct = 1.0f - fDist / light.m_fRadius;
			break;
		default:	// quartic
			fDistPct = 1.0f - fDistSqr / light.m_fRadiusSqr;
			fDistPct *= fDistPct;
			break;
	}
	if(fDistPct <= 0.0f)
		return;

	if(bShadows && grid.Blocked(vSample, light.m_vPos, iSkipPoly))
		return;

	vColor += light.m_vInner * (fAnglePct * fDistPct * fLambert);
}

static LTVector LP_GetAmbient(CEditRegion *pRegion)
{
	LTVector vAmbient(0.0f, 0.0f, 0.0f);
	const char *pInfo = pRegion->m_pInfoString;
	if(!pInfo)
		return vAmbient;

	// The Processor's parser is case-insensitive ("ambientlight 100 100 100" works),
	// and the keyword has to be a whole word.
	const size_t nKey = strlen("AmbientLight");
	for(const char *p = pInfo; *p; p++)
	{
		if(strnicmp(p, "AmbientLight", nKey) != 0)
			continue;
		if(p > pInfo && !isspace((unsigned char)p[-1]) && p[-1] != ';' && p[-1] != '"')
			continue;
		if(p[nKey] != 0 && !isspace((unsigned char)p[nKey]))
			continue;

		float r = 0.0f, g = 0.0f, b = 0.0f;
		if(sscanf(p + nKey, "%f %f %f", &r, &g, &b) == 3)
		{
			vAmbient.Init(r < 0.0f ? 0.0f : (r > 255.0f ? 255.0f : r),
						  g < 0.0f ? 0.0f : (g > 255.0f ? 255.0f : g),
						  b < 0.0f ? 0.0f : (b > 255.0f ? 255.0f : b));
			break;
		}
	}
	return vAmbient;
}

// Shelf packer for the pages.
struct LPPacker
{
	uint32	m_nX, m_nY, m_nRowHeight;

	void Alloc(uint32 w, uint32 h, uint32 &page, uint32 &x, uint32 &y)
	{
		// Blocks get a one texel border on every side.
		w += 2;
		h += 2;

		if(g_Pages.empty())
			NewPage();

		if(m_nX + w > LP_PAGE_SIZE)
		{
			m_nX = 0;
			m_nY += m_nRowHeight;
			m_nRowHeight = 0;
		}
		if(m_nY + h > LP_PAGE_SIZE)
			NewPage();

		page = (uint32)g_Pages.size() - 1;
		x = m_nX + 1;
		y = m_nY + 1;

		m_nX += w;
		if(h > m_nRowHeight)
			m_nRowHeight = h;
	}

	void NewPage()
	{
		g_Pages.push_back(std::vector<uint8>(LP_PAGE_SIZE * LP_PAGE_SIZE * 3, 255));
		m_nX = m_nY = m_nRowHeight = 0;
	}
};


// Is (s,t) inside the polygon given by its (s,t) corner coordinates?
static bool LP_InsidePoly2D(const std::vector<float> &ps, const std::vector<float> &pt, float s, float t)
{
	bool bIn = false;
	size_t n = ps.size();
	for(size_t i=0, j=n-1; i < n; j = i++)
	{
		if(((pt[i] > t) != (pt[j] > t)) && (s < (ps[j] - ps[i]) * (t - pt[i]) / (pt[j] - pt[i]) + ps[i]))
			bIn = !bIn;
	}
	return bIn;
}

void LightPreview::Compute(CEditRegion *pRegion)
{
	Clear();
	if(!pRegion)
		return;

	// Collect the geometry.
	std::vector<LPPoly>		polies;
	std::vector<LTVector>	verts;
	std::vector<CEditPoly*>	editPolies;
	std::vector<LTVector>	vnormals;	// per vertex, parallel to verts (the Processor's smoothed normals)
	std::vector<bool>		wantsLightmap;

	for(LPOS pos=pRegion->m_Brushes; pos; )
	{
		CEditBrush *pBrush = pRegion->m_Brushes.GetNext(pos);

		// Brush type / lighting, the way the Processor reads them.
		const char *pType = LP_GetString(pBrush, "Type", "Normal");
		// Condemned brushes have no "Lighting" property: the game lights per pixel, so preview them with a lightmap, not corner-only Gouraud.
		const char *pLighting = LP_GetString(pBrush, "Lighting", "Lightmap");

		bool bSky			= (stricmp(pType, "SkyPortal") == 0);
		bool bNoLightDraw	= bSky || (stricmp(pType, "Occluder") == 0) || (stricmp(pType, "RBSplitter") == 0) ||
							  (stricmp(pType, "Blocker") == 0) || (stricmp(pType, "ParticleBlocker") == 0);
		// These types never make it into the BSP, so they can't block light either.
		bool bNonexistant	= bNoLightDraw ? !bSky : (stricmp(pType, "RenderOnly") == 0);
		bool bLitType		= (stricmp(pLighting, "Lightmap") == 0) || (stricmp(pLighting, "Gouraud") == 0) ||
							  (stricmp(pLighting, "ShadowMesh") == 0);
		bool bClipLight		= LP_GetBool(pBrush, "ClipLight", true);
		bool bReceiveLight	= LP_GetBool(pBrush, "ReceiveLight", true);
		bool bRecvShadows	= LP_GetBool(pBrush, "ReceiveShadows", true);

		// Vertex normals the way the Processor makes them (BrushToWorld.cpp): within one
		// brush, a corner's normal is the average of the faces that share the point and
		// are within the brush's CreaseAngle (default 45) of the face being lit.
		float fCosCrease = (float)MATH_DEGREES_TO_RADIANS(LP_GetReal(pBrush, "CreaseAngle", 45.0f));
		fCosCrease = (float)cos(fCosCrease);
		if(fCosCrease < 0.0f) fCosCrease = 0.0f;
		std::vector< std::vector<CEditPoly*> > pointPolies(pBrush->m_Points.GetSize());
		{
			for(uint32 ip=0; ip < pBrush->m_Polies.GetSize(); ip++)
			{
				CEditPoly *pp = pBrush->m_Polies[ip];
				if(!pp)
					continue;
				for(uint32 iv=0; iv < pp->m_Indices.GetSize(); iv++)
					if(pp->m_Indices[iv] < pointPolies.size())
						pointPolies[pp->m_Indices[iv]].push_back(pp);
			}
		}

		for(uint32 i=0; i < pBrush->m_Polies.GetSize(); i++)
		{
			CEditPoly *pPoly = pBrush->m_Polies[i];
			if(!pPoly || pPoly->NumVerts() < 3)
				continue;

			LPPoly poly;
			poly.m_vNormal = pPoly->Normal();
			poly.m_fDist = pPoly->Dist();
			poly.m_nFirstVert = (uint32)verts.size();
			poly.m_nNumVerts = pPoly->NumVerts();
			poly.m_vMin.Init(1e30f, 1e30f, 1e30f);
			poly.m_vMax.Init(-1e30f, -1e30f, -1e30f);
			for(uint32 v=0; v < pPoly->NumVerts(); v++)
			{
				LTVector pt = pPoly->Pt(v);
				verts.push_back(pt);
				{
					LTVector vn(0.0f, 0.0f, 0.0f);
					uint32 idx = pPoly->m_Indices[v];
					if(idx < pointPolies.size())
					{
						for(size_t q=0; q < pointPolies[idx].size(); q++)
						{
							const LTVector &qn = pointPolies[idx][q]->Normal();
							if(pPoly->Normal().Dot(qn) >= fCosCrease)
								vn += qn;
						}
					}
					if(vn.Mag() < 0.001f)
						vn = pPoly->Normal();
					vn.Norm();
					vnormals.push_back(vn);
				}
				for(int k=0; k < 3; k++)
				{
					if(pt[k] < poly.m_vMin[k]) poly.m_vMin[k] = pt[k];
					if(pt[k] > poly.m_vMax[k]) poly.m_vMax[k] = pt[k];
				}
			}
			poly.m_bOccluder = (bSky || bClipLight) && !bNonexistant;	// sky brushes always have ClipLight set
			poly.m_bReceiveShadows = bRecvShadows;
			poly.m_bGouraud = (stricmp(pLighting, "Gouraud") == 0);

			polies.push_back(poly);
			editPolies.push_back(pPoly);
			wantsLightmap.push_back(bLitType && bReceiveLight && !bNoLightDraw);
		}
	}

	std::vector<LPLight> lights;
	LP_GatherLights(pRegion, lights);
	LTVector vAmbient = LP_GetAmbient(pRegion);

	CLPRayGrid grid;
	grid.Build(polies, verts);

	LPPacker packer;
	packer.m_nX = packer.m_nY = packer.m_nRowHeight = 0;

	std::vector<uint32> nearLights;
	std::vector<LTVector> texels;

	for(uint32 iPoly=0; iPoly < polies.size(); iPoly++)
	{
		if(!wantsLightmap[iPoly])
			continue;

		const LPPoly &poly = polies[iPoly];
		const LTVector &n = poly.m_vNormal;

		// In-plane axes based on the texture projection axes.
		LTVector U, V;
		LP_GetBaseAxes(n, U, V);
		U = U - n * n.Dot(U);
		if(U.Mag() < 0.01f)
			U = V - n * n.Dot(V);
		U.Norm();
		V = n.Cross(U);
		V.Norm();

		float sMin = 1e30f, sMax = -1e30f, tMin = 1e30f, tMax = -1e30f;
		for(uint32 v=0; v < poly.m_nNumVerts; v++)
		{
			const LTVector &pt = verts[poly.m_nFirstVert + v];
			float s = pt.Dot(U), t = pt.Dot(V);
			if(s < sMin) sMin = s;
			if(s > sMax) sMax = s;
			if(t < tMin) tMin = t;
			if(t > tMax) tMax = t;
		}

		float fStep = LP_DEFAULT_GRID;
		float fExtent = (sMax - sMin) > (tMax - tMin) ? (sMax - sMin) : (tMax - tMin);
		if(fExtent / fStep > (float)(LP_MAX_LM_SIZE - 1))
			fStep = fExtent / (float)(LP_MAX_LM_SIZE - 1);

		uint32 w = (uint32)ceil((sMax - sMin) / fStep) + 1;
		uint32 h = (uint32)ceil((tMax - tMin) / fStep) + 1;
		if(w < 2) w = 2;
		if(h < 2) h = 2;
		if(w > LP_MAX_LM_SIZE) w = LP_MAX_LM_SIZE;
		if(h > LP_MAX_LM_SIZE) h = LP_MAX_LM_SIZE;

		// Lights that can reach this poly.
		nearLights.clear();
		LTVector vCenter = (poly.m_vMin + poly.m_vMax) * 0.5f;
		float fPolyRadius = (poly.m_vMax - poly.m_vMin).Mag() * 0.5f;
		for(uint32 l=0; l < lights.size(); l++)
		{
			const LPLight &light = lights[l];
			float fPlaneDist = n.Dot(light.m_vPos) - poly.m_fDist;
			if(fPlaneDist <= 0.0f)
				continue;	// behind the poly
			if((light.m_vPos - vCenter).Mag() > light.m_fRadius + fPolyRadius)
				continue;
			nearLights.push_back(l);
		}

		// The polygon's corners in (s,t), for keeping samples on the polygon.
		std::vector<float> polyS(poly.m_nNumVerts), polyT(poly.m_nNumVerts);
		float cS = 0.0f, cT = 0.0f;
		for(uint32 v=0; v < poly.m_nNumVerts; v++)
		{
			const LTVector &pv = verts[poly.m_nFirstVert + v];
			polyS[v] = pv.Dot(U);
			polyT[v] = pv.Dot(V);
			cS += polyS[v];
			cT += polyT[v];
		}
		cS /= (float)poly.m_nNumVerts;
		cT /= (float)poly.m_nNumVerts;

		// Gouraud brushes are lit per vertex in the game and interpolated across the face,
		// so light just the corners here and interpolate the same way.
		std::vector<LTVector> vcolors;
		if(poly.m_bGouraud)
		{
			vcolors.resize(poly.m_nNumVerts);
			for(uint32 v=0; v < poly.m_nNumVerts; v++)
			{
				LTVector vc = vAmbient;
				LTVector vp = verts[poly.m_nFirstVert + v] + n * LP_SURFACE_OFFSET;
				for(uint32 l=0; l < nearLights.size(); l++)
				{
					const LPLight &light = lights[nearLights[l]];
					LP_AddLight(light, vp, vnormals[poly.m_nFirstVert + v], light.m_bShadows && poly.m_bReceiveShadows, grid, iPoly, vc);
				}
				vcolors[v] = vc;
			}
		}

		// Light every texel.
		texels.resize(w * h);
		LTVector vPlaneBase = n * poly.m_fDist;
		for(uint32 y=0; y < h; y++)
		{
			for(uint32 x=0; x < w; x++)
			{
				float fS = sMin + x * fStep, fT = tMin + y * fStep;

				// Texels outside the polygon (the grid covers its bounding box) are pulled
				// back onto it. Otherwise they sit inside neighbouring brushes and come out
				// black, which shows up as patches along the edges.
				if(!LP_InsidePoly2D(polyS, polyT, fS, fT))
				{
					float fLo = 0.0f, fHi = 1.0f;	// fraction of the way to the centre
					for(int it=0; it < 12; it++)
					{
						float fMid = (fLo + fHi) * 0.5f;
						if(LP_InsidePoly2D(polyS, polyT, fS + (cS - fS) * fMid, fT + (cT - fT) * fMid))
							fHi = fMid;
						else
							fLo = fMid;
					}
					fS += (cS - fS) * fHi;
					fT += (cT - fT) * fHi;
				}

				LTVector vSample = vPlaneBase + U * fS + V * fT + n * LP_SURFACE_OFFSET;
				LTVector vN = n;
				LTVector vGouraud(0.0f, 0.0f, 0.0f);
				{
					// Interpolate the corner normals over the polygon (fan of triangles).
					float fBest = -1e30f;
					for(uint32 tv=1; tv + 1 < poly.m_nNumVerts; tv++)
					{
						float x0 = polyS[0], y0 = polyT[0];
						float d1x = polyS[tv] - x0, d1y = polyT[tv] - y0;
						float d2x = polyS[tv+1] - x0, d2y = polyT[tv+1] - y0;
						float fDet = d1x * d2y - d1y * d2x;
						if(fabs(fDet) < 1e-6f)
							continue;
						float b1 = ((fS - x0) * d2y - (fT - y0) * d2x) / fDet;
						float b2 = (d1x * (fT - y0) - d1y * (fS - x0)) / fDet;
						float b0 = 1.0f - b1 - b2;
						float fMinB = b0 < b1 ? (b0 < b2 ? b0 : b2) : (b1 < b2 ? b1 : b2);
						if(fMinB > fBest)
						{
							fBest = fMinB;
							if(poly.m_bGouraud)
								vGouraud = vcolors[0] * b0 + vcolors[tv] * b1 + vcolors[tv + 1] * b2;
							LTVector vi = vnormals[poly.m_nFirstVert] * b0 + vnormals[poly.m_nFirstVert + tv] * b1 +
										  vnormals[poly.m_nFirstVert + tv + 1] * b2;
							// Lightmapped faces use the flat face normal in the Processor; only
							// Gouraud faces use the smoothed corner normals.
							if(poly.m_bGouraud && vi.Mag() > 0.001f)
							{
								vi.Norm();
								vN = vi;
							}
						}
					}
				}
				LTVector vColor = vAmbient;

				for(uint32 l=0; !poly.m_bGouraud && l < nearLights.size(); l++)
				{
					const LPLight &light = lights[nearLights[l]];
					LP_AddLight(light, vSample, vN, light.m_bShadows && poly.m_bReceiveShadows, grid, iPoly, vColor);
				}

				texels[y * w + x] = poly.m_bGouraud ? vGouraud : vColor;
			}
		}

		// Pack it.
		LPLightmap lm;
		packer.Alloc(w, h, lm.m_nPage, lm.m_nX, lm.m_nY);
		lm.m_vU = U;
		lm.m_vV = V;
		lm.m_fSMin = sMin;
		lm.m_fTMin = tMin;
		lm.m_fStep = fStep;

		std::vector<uint8> &page = g_Pages[lm.m_nPage];
		for(int by=-1; by <= (int)h; by++)
		{
			for(int bx=-1; bx <= (int)w; bx++)
			{
				// The border repeats the edge texels so filtering doesn't bleed.
				int sx = bx < 0 ? 0 : (bx >= (int)w ? (int)w - 1 : bx);
				int sy = by < 0 ? 0 : (by >= (int)h ? (int)h - 1 : by);
				const LTVector &c = texels[sy * w + sx];

				uint8 *pOut = &page[((lm.m_nY + by) * LP_PAGE_SIZE + (lm.m_nX + bx)) * 3];
				pOut[0] = (uint8)(c.x < 0.0f ? 0 : (c.x > 255.0f ? 255 : (int)c.x));
				pOut[1] = (uint8)(c.y < 0.0f ? 0 : (c.y > 255.0f ? 255 : (int)c.y));
				pOut[2] = (uint8)(c.z < 0.0f ? 0 : (c.z > 255.0f ? 255 : (int)c.z));
			}
		}

		g_PolyMap[editPolies[iPoly]] = (uint32)g_Lightmaps.size();
		g_Lightmaps.push_back(lm);
	}

	g_pRegion = pRegion;
}


// ------------------------------------------------------------------
// Interface
// ------------------------------------------------------------------

bool LightPreview::IsEnabled(CEditRegion *pRegion)
{
	return pRegion && pRegion == g_pRegion;
}

void LightPreview::Toggle(CEditRegion *pRegion)
{
	if(IsEnabled(pRegion))
		Clear();
	else
		Compute(pRegion);
}

void LightPreview::NotifyRegionClosed(CEditRegion *pRegion)
{
	if(pRegion && pRegion == g_pRegion)
		Clear();
}

void LightPreview::Clear()
{
	LP_ReleaseAllRenders();
	g_PolyMap.clear();
	g_Lightmaps.clear();
	g_Pages.clear();
	g_pRegion = NULL;
}

IDirect3DTexture2* LightPreview::GetPolyLightmap(D3DRender *pRender, CEditRegion *pRegion, CEditPoly *pPoly, float *pUV)
{
	if(!g_pRegion || pRegion != g_pRegion)
		return NULL;

	std::map<CEditPoly*, uint32>::iterator it = g_PolyMap.find(pPoly);
	if(it == g_PolyMap.end())
		return NULL;

	const LPLightmap &lm = g_Lightmaps[it->second];

	LPRenderTextures *pRT = LP_GetRenderTextures(pRender);
	if(lm.m_nPage >= pRT->m_Textures.size() || !pRT->m_Textures[lm.m_nPage])
		return NULL;

	const float fInvPage = 1.0f / (float)LP_PAGE_SIZE;
	for(uint32 i=0; i < pPoly->NumVerts(); i++)
	{
		const LTVector &pt = pPoly->Pt(i);
		float s = (pt.Dot(lm.m_vU) - lm.m_fSMin) / lm.m_fStep;
		float t = (pt.Dot(lm.m_vV) - lm.m_fTMin) / lm.m_fStep;
		pUV[i*2 + 0] = ((float)lm.m_nX + s + 0.5f) * fInvPage;
		pUV[i*2 + 1] = ((float)lm.m_nY + t + 0.5f) * fInvPage;
	}

	return pRT->m_Textures[lm.m_nPage];
}

void LightPreview::ReleaseRender(D3DRender *pRender)
{
	for(uint32 i=0; i < g_Renders.size(); i++)
	{
		if(g_Renders[i]->m_pRender == pRender)
		{
			LP_ReleaseRenderTextures(g_Renders[i]);
			delete g_Renders[i];
			g_Renders.erase(g_Renders.begin() + i);
			return;
		}
	}
}
