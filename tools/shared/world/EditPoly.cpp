//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : EditPoly.cpp
//
//	PURPOSE	  : Defines the CEditPoly class.
//
//	CREATED	  : October 5 1996
//
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include <string.h>
#include "editpoly.h"
#include "geomroutines.h"
#include "editbrush.h"
#include "de_world.h"
#include "ltamgr.h"
#include "ltasaveutils.h"
#include "uvtoopq.h"
#include "PolyLightMap.h"


#ifdef DIRECTEDITOR_BUILD
	#include "texture.h"
	#include "dedit.h"
	#include "MaterialTexture.h"
#endif

static void FearTileSize(const char* pMat, float& w, float& h)
{
#ifdef DIRECTEDITOR_BUILD
	MaterialTexture_GetTileSize(pMat, w, h);
#else
	w = h = 100.0f;
#endif
}



// Terrain meshes: the editor shows a baked composite (texture 0 mapped over the footprint),
// the game gets "<name>_g.Mat00": two tiled layers (mapping slots 0 and 1) blended by a
// blend map over the footprint (mapping slot 2). Texture 1 of such a poly carries the
// footprint mapping: O.y = tile size in world units (negative for non-top faces).
#define TERRAIN_EDIT_TEX	1024.0f
static bool TerrainIsMat(const char* p) { return p && strstr(p, "Terrainmesh") != NULL; }
// Length of the "_g" / "_gK" suffix before the extension (0 = not a game material).
static int TerrainIsGameMat(const char* p)
{
	const char* e = p ? strrchr(p, '.') : NULL;
	if(!e) return 0;
	if((e - p) >= 2 && e[-1] == 'g' && e[-2] == '_') return 2;
	if((e - p) >= 3 && e[-1] >= '0' && e[-1] <= '9' && e[-2] == 'g' && e[-3] == '_') return 3;
	return 0;
}
// Texture 1's O.y packs the layer pair (k * 100000) and the tile size; sign = top face.
#define TERRAIN_PAIR_SCALE	100000.0f
static float TerrainTile(float packed, int* pPair)
{
	float a = (packed < 0.0f) ? -packed : packed;
	int k = (int)(a / TERRAIN_PAIR_SCALE);
	if(pPair) *pPair = k;
	a -= (float)k * TERRAIN_PAIR_SCALE;
	return (a < 1.0f) ? 256.0f : a;
}

// The main texture alignment planes.
CVector g_TexturePlanes[] =
{
	CVector(0.0f, 1.0f, 0.0f),		// Bottom
	CVector(0.0f, -1.0f, 0.0f),		// Top

	CVector(1.0f, 0.0f, 0.0f),		// East-facing wall
	CVector(-1.0f, 0.0f, 0.0f),		// West-facing wall
	CVector(0.0f, 0.0f, 1.0f),		// North-facing wall
	CVector(0.0f, 0.0f, -1.0f)		// South-facing wall
};

CVector g_RightVectors[] =
{
	CVector(1.0f, 0.0f, 0.0f),
	CVector(-1.0f, 0.0f, 0.0f),

	CVector(0.0f, 0.0f, 1.0f),	
	CVector(0.0f, 0.0f, -1.0f),
	CVector(-1.0f, 0.0f, 0.0f),
	CVector(1.0f, 0.0f, 0.0f),	
};

#define NUM_TEXTURE_PLANES	(sizeof(g_TexturePlanes) / sizeof(CVector))

inline void CEditPoly::CommonConstructor( )
{
#ifndef DIRECTEDITOR_BUILD
	m_pUser1				= NULL;
#endif

	m_pBrush				= NULL;
}


CEditPoly::CEditPoly()
{
	CommonConstructor( );
}


CEditPoly::CEditPoly( CEditPoly *pCopyFrom )
{
	CommonConstructor( );
	CopyEditPoly( pCopyFrom );
}


CEditPoly::CEditPoly( CEditBrush *pBrush )
{
	CommonConstructor( );
	m_pBrush = pBrush;
}


CEditPoly::~CEditPoly()
{
	Term();
}


void CEditPoly::Term()
{
	m_Indices.SetSize(0);
}

bool CEditPoly::LoadEditPolyTBW( CAbstractIO& InFile, CStringHolder *pStringHolder )
{
	if (!LoadBasePolyTBW(InFile) )
	{
		return false;
	}

	//load in the texture information
	for(uint32 nCurrTex = 0; nCurrTex < NUM_TEXTURES; nCurrTex++)
	{
		//now save out this texture
		GetTexture(nCurrTex).LoadTextureInfoTBW(InFile, pStringHolder);
	}

	return true;
}

void CEditPoly::SaveEditPolyTBW( CAbstractIO& OutFile )
{
	SaveBasePolyTBW( OutFile );
		
	//now save out each additional texture
	for(uint32 nCurrTex = 0; nCurrTex < NUM_TEXTURES; nCurrTex++)
	{
		//now save out this texture
		GetTexture(nCurrTex).SaveTextureInfoTBW(OutFile);
	}
}



// ---------------------------------------------------------------------------
// F.E.A.R. / Condemned "editpoly" dialect:
//   ( editpoly ( f i i i ) ( material "x.Mat00" ) ( occlusion "" )
//              ( mappings ( ( 0 ( textureinfo (O) (P) (Q) ) ) ... ) ) )
// Floats are written as raw hex bits. Loading auto-detects the dialect per poly;
// saving uses it whenever g_bSaveFearPolyDialect is set (the default).
// ---------------------------------------------------------------------------
bool g_bSaveFearPolyDialect = true;

static CLTANode* FearFindNamed(CLTANode* pNode, const char* pName)
{
	for(uint32 i = 1; i < pNode->GetNumElements(); i++)
	{
		CLTANode* pChild = pNode->GetElement(i);
		if(pChild && pChild->IsList() && pChild->GetNumElements() > 0 &&
		   pChild->GetElement(0)->IsAtom() && stricmp(pChild->GetElement(0)->GetValue(), pName) == 0)
			return pChild;
	}
	return NULL;
}

static bool IsFearPoly(CLTANode* pNode)
{
	return FearFindNamed(pNode, "material") != NULL;
}

static void FearWriteHex(CLTAFile* pFile, float f)
{
	uint32 bits;
	memcpy(&bits, &f, 4);
	pFile->WriteStrF("0x%08X", bits);
}

static void FearWriteVec(CLTAFile* pFile, const LTVector& v)
{
	pFile->WriteStr("( ");
	FearWriteHex(pFile, v.x); pFile->WriteStr(" ");
	FearWriteHex(pFile, v.y); pFile->WriteStr(" ");
	FearWriteHex(pFile, v.z); pFile->WriteStr(" )");
}

BOOL CEditPoly::LoadFearPolyLTA( CLTANode* pNode, CStringHolder *pStringHolder )
{
	// face indices
	CLTANode* pF = FearFindNamed(pNode, "f");
	if(!pF || pF->GetNumElements() < 4)
		return FALSE;
	uint32 listSize = pF->GetNumElements();
	m_Indices.SetSize(listSize - 1);
	for(uint32 i = 1; i < listSize; i++)
	{
		Index(i-1) = GetUint32(pF->GetElement(i));
		if(Index(i-1) >= m_pBrush->m_Points)
		{
			Index(i-1) = 0;
			return FALSE;
		}
	}

	// material
	const char* pMat = "";
	CLTANode* pMatNode = FearFindNamed(pNode, "material");
	if(pMatNode && pMatNode->GetNumElements() > 1)
		pMat = pMatNode->GetElement(1)->GetValue();

	int nGameSuffix = TerrainIsMat(pMat) ? TerrainIsGameMat(pMat) : 0;
	bool bGameMat = nGameSuffix != 0;
	if(bGameMat)
	{
		// the editor works with the preview material (without "_g")
		char edName[512];
		const char* e = strrchr(pMat, '.');
		_snprintf(edName, sizeof(edName) - 1, "%.*s%s", (int)(e - pMat) - nGameSuffix, pMat, e);
		edName[sizeof(edName) - 1] = 0;
		pMat = pStringHolder->AddString(edName);
	}
	LTVector gv0[3], gv2[3];
	bool bHave0 = false, bHave2 = false;

	for(uint32 t = 0; t < NUM_TEXTURES; t++)
	{
		GetTexture(t).m_pTextureName = pStringHolder->AddString(pMat);
	}

	// mappings: ( mappings ( ( idx ( textureinfo (O)(P)(Q) ) ) ... ) )
	CLTANode* pMaps = FearFindNamed(pNode, "mappings");
	if(pMaps && pMaps->GetNumElements() > 1 && pMaps->GetElement(1)->IsList())
	{
		CLTANode* pList = pMaps->GetElement(1);
		for(uint32 i = 0; i < pList->GetNumElements(); i++)
		{
			CLTANode* pMap = pList->GetElement(i);
			if(!pMap || !pMap->IsList() || pMap->GetNumElements() < 2) continue;
			uint32 nTex = GetUint32(pMap->GetElement(0));
			if(nTex >= NUM_TEXTURES && !(bGameMat && nTex == 2)) continue;
			CLTANode* pInfo = pMap->GetElement(1);
			if(!pInfo || !pInfo->IsList() || pInfo->GetNumElements() < 4) continue;
			LTVector v[3];
			for(uint32 k = 0; k < 3; k++)
			{
				CLTANode* pV = pInfo->GetElement(k + 1);
				v[k].x = GetFloat(pV->GetElement(0));
				v[k].y = GetFloat(pV->GetElement(1));
				v[k].z = GetFloat(pV->GetElement(2));
			}
			if(bGameMat)
			{
				if(nTex == 0) { gv0[0] = v[0]; gv0[1] = v[1]; gv0[2] = v[2]; bHave0 = true; }
				if(nTex == 2) { gv2[0] = v[0]; gv2[1] = v[1]; gv2[2] = v[2]; bHave2 = true; }
				continue;
			}
			// File: tiles per unit. Editor: texels per unit (axis length 1 = one tile per tile-size units).
			float tw, th;
			FearTileSize(pMat, tw, th);
			GetTexture(nTex).SetO(v[0]);
			GetTexture(nTex).SetP(LTVector(v[1].x * tw, v[1].y * tw, v[1].z * tw));
			GetTexture(nTex).SetQ(LTVector(v[2].x * th, v[2].y * th, v[2].z * th));
		}
	}
	if(bGameMat && bHave0 && bHave2)
	{
		// slot 2 is the footprint mapping (raw tiles per unit), O.y is the tile size.
		float T = gv2[0].y;
		float aT = TerrainTile(T, NULL);
		GetTexture(1).SetO(gv2[0]);
		GetTexture(1).SetP(gv2[1]);
		GetTexture(1).SetQ(gv2[2]);
		if(T > 0.0f)
		{
			GetTexture(0).SetO(LTVector(gv2[0].x, 0.0f, gv2[0].z));
			GetTexture(0).SetP(LTVector(gv2[1].x * TERRAIN_EDIT_TEX, 0.0f, gv2[1].z * TERRAIN_EDIT_TEX));
			GetTexture(0).SetQ(LTVector(gv2[2].x * TERRAIN_EDIT_TEX, 0.0f, gv2[2].z * TERRAIN_EDIT_TEX));
		}
		else
		{
			GetTexture(0).SetO(gv0[0]);
			GetTexture(0).SetP(LTVector(gv0[1].x * aT, gv0[1].y * aT, gv0[1].z * aT));
			GetTexture(0).SetQ(LTVector(gv0[2].x * aT, gv0[2].y * aT, gv0[2].z * aT));
		}
	}
	// the plane is rebuilt from the points by the brush after loading
	return TRUE;
}

void CEditPoly::SaveFearPolyLTA( CLTAFile* pFile, uint32 level )
{
	PrependTabs(pFile, level);
	pFile->WriteStr("( editpoly ");

	PrependTabs(pFile, level+1);
	pFile->WriteStr("( f");
	for(uint32 i = 0; i < m_Indices; i++)
		pFile->WriteStrF(" %d", Index(i));
	pFile->WriteStr(" )");

	const char* pMat = GetTexture(0).m_pTextureName;
	if(!pMat || !pMat[0]) pMat = "global\\materials\\grid_wall.Mat00";
	// terrain polys (texture 1 carries the footprint mapping) are saved with the game material
	char gMat[512];
	float terrT = GetTexture(1).GetO().y;
	bool bSplat = TerrainIsMat(pMat) && !TerrainIsGameMat(pMat) && terrT != 0.0f && strrchr(pMat, '.') != NULL;
	LTVector sO[4], sP[4], sQ[4];
	if(bSplat)
	{
		const char* e = strrchr(pMat, '.');
		int nPair = 0;
		float aT = TerrainTile(terrT, &nPair);
		_snprintf(gMat, sizeof(gMat) - 1, "%.*s_g%d%s", (int)(e - pMat), pMat, nPair, e);
		gMat[sizeof(gMat) - 1] = 0;
		LTVector o = GetTexture(1).GetO(), p = GetTexture(1).GetP(), q = GetTexture(1).GetQ();
		GetTexture(1).SetO(LTVector(0.0f, 0.0f, 0.0f));
		SetupBaseTextureSpace(1);
		LTVector tp = GetTexture(1).GetP(), tq = GetTexture(1).GetQ();
		GetTexture(1).SetO(o);
		GetTexture(1).SetP(p);
		GetTexture(1).SetQ(q);
		for(int k = 0; k < 4; k++)
		{
			if(k == 2)
			{
				sO[k] = o; sP[k] = p; sQ[k] = q;	// raw tiles per unit
			}
			else
			{
				sO[k] = LTVector(0.0f, 0.0f, 0.0f);
				sP[k] = LTVector(tp.x / aT, tp.y / aT, tp.z / aT);
				sQ[k] = LTVector(tq.x / aT, tq.y / aT, tq.z / aT);
			}
		}
	}

	PrependTabs(pFile, level+1);
	pFile->WriteStrF("( material \"%s\" )", bSplat ? gMat : pMat);
	PrependTabs(pFile, level+1);
	pFile->WriteStr("( occlusion \"\" )");

	PrependTabs(pFile, level+1);
	pFile->WriteStr("( mappings (");
	for(uint32 t = 0; t < 4; t++)
	{
		// the kit has four mapping slots; we edit two, so extra slots repeat slot 0
		const CTexturedPlane& tex = GetTexture(t < NUM_TEXTURES ? t : 0);
		PrependTabs(pFile, level+2);
		pFile->WriteStrF("( %d", t);
		PrependTabs(pFile, level+3);
		pFile->WriteStr("( textureinfo");
		float tw, th;
		FearTileSize(pMat, tw, th);
		const LTVector& P = tex.GetP();
		const LTVector& Q = tex.GetQ();
		if(bSplat)
		{
			PrependTabs(pFile, level+4); FearWriteVec(pFile, sO[t]);
			PrependTabs(pFile, level+4); FearWriteVec(pFile, sP[t]);
			PrependTabs(pFile, level+4); FearWriteVec(pFile, sQ[t]);
		}
		else
		{
		PrependTabs(pFile, level+4); FearWriteVec(pFile, tex.GetO());
		PrependTabs(pFile, level+4); FearWriteVec(pFile, LTVector(P.x / tw, P.y / tw, P.z / tw));
		PrependTabs(pFile, level+4); FearWriteVec(pFile, LTVector(Q.x / th, Q.y / th, Q.z / th));
		}
		PrependTabs(pFile, level+3);
		pFile->WriteStr(")");
		PrependTabs(pFile, level+2);
		pFile->WriteStr(") ");
	}
	PrependTabs(pFile, level+1);
	pFile->WriteStr(") ) ");

	PrependTabs(pFile, level);
	pFile->WriteStr(") ");
}

BOOL CEditPoly::LoadEditPolyLTA( CLTANode* pNode, CStringHolder *pStringHolder )
{
	BOOL retval = FALSE;
	if( IsFearPoly(pNode) )
		return LoadFearPolyLTA(pNode, pStringHolder);
	if ( LoadBasePolyLTA(pNode) )
	{	
		//Load in the information for the first texture
		CLTANode* pTextureInfo	= pNode->GetElement(4); // shallow_find_list(pNode, "textureinfo");
		GetTexture(0).LoadTextureInfoLTA(pTextureInfo, pStringHolder);

		//Node 5 is reserved for the flags
		//Node 6 is reserved for the shade
		//Node 7 is reserved for the physics information
		//Node 8 is reserved for the surface key

		//initialize all of our surfaces jsut in case some aren't specified
		for(uint32 nCurrTex = 1; nCurrTex < NUM_TEXTURES; nCurrTex++)
		{
			SetupBaseTextureSpace(nCurrTex);
		}

		//Node 9 is all of our additional textures, load them up
		if(pNode->GetNumElements() >= 10)
		{
			//we have additional textures
			CLTANode* pAdditional = pNode->GetElement(9);

			//sanity checks
			assert(pAdditional->IsList());
			assert(pAdditional->GetNumElements() >= 2);
			assert(pAdditional->GetElement(0)->IsAtom());
			assert(pAdditional->GetElement(1)->IsList());
			assert(strcmp(pAdditional->GetElement(0)->GetValue(), "textures") == 0);

			//ok, so now we need to load in each texture
			CLTANode* pTexList = pAdditional->GetElement(1);

			for(uint32 nCurrTex = 0; nCurrTex < pTexList->GetNumElements(); nCurrTex++)
			{
				CLTANode* pTexture = pTexList->GetElement(nCurrTex);

				//read in the index of this texture
				uint32 nTex = atoi(pTexture->GetElement(0)->GetValue());

				//only proceed if it is a valid index
				if(nTex < NUM_TEXTURES)
				{
					GetTexture(nTex).LoadTextureInfoLTA(pTexture->GetElement(1), pStringHolder);
				}
			}
		}

		retval=TRUE;
	}
	return retval;
}

void CEditPoly::SaveEditPolyLTA( CLTAFile* pFile, uint32 level )
{
	if( g_bSaveFearPolyDialect )
	{
		SaveFearPolyLTA(pFile, level);
		return;
	}
	PrependTabs(pFile, level);
	pFile->WriteStr("( editpoly " );
		SaveBasePolyLTA( pFile, level+1 );
		
		//save out the first texture
		GetTexture(0).SaveTextureInfoLTA( pFile, level+1 );
		GetTexture(0).SaveFlagsLTA( pFile, level + 1);
		GetTexture(0).SaveShadeLTA( pFile, level + 1);
		GetTexture(0).SavePhysicsMaterialLTA(pFile, level + 1);

		//write out the surface key
		PrependTabs(pFile, level + 1);
		pFile->WriteStr("( surfacekey \"\" )");

		//write out auxilery textures
		PrependTabs(pFile, level + 1);
		pFile->WriteStr("( textures ( ");
		
		//now save out each additional texture
		for(uint32 nCurrTex = 1; nCurrTex < NUM_TEXTURES; nCurrTex++)
		{
			//save out this texture...
			PrependTabs(pFile, level + 2);
			pFile->WriteStrF("( %d ", nCurrTex);

			//now save out this texture
			GetTexture(nCurrTex).SaveTextureInfoLTA(pFile, level + 3);

			//close off the texture tab
			PrependTabs(pFile, level + 2);
			pFile->WriteStr(") ");
		}

		//close off the texture list
		PrependTabs(pFile, level + 1);
		pFile->WriteStr(") ) ");
	
	PrependTabs(pFile, level);
	pFile->WriteStr(") " );
}


void CEditPoly::CopyAttributes( CBasePoly *pBase, CStringHolder *pStringHolder )
{
	CEditPoly	*pOther = (CEditPoly*)pBase;

	m_Plane = pOther->m_Plane;
	
	for(uint32 nCurrTex = 0; nCurrTex < NUM_TEXTURES; nCurrTex++)
		GetTexture(nCurrTex).CopyTextureAttributes(&pOther->GetTexture(nCurrTex), pStringHolder);

}


void CEditPoly::DecrementPoints( uint32 index )
{
	CBasePoly::DecrementPoints(index);
}


BOOL CEditPoly::IntersectRay( CEditRay &ray, CReal &t, BOOL bBackface )
{
	ASSERT(NumVerts() > 1);

	CReal dot1, dot2, div;
	LTVector testPt;

	// Trivial reject.
	if( (Normal().Dot(ray.m_Pos) - Dist()) >= 0.0f )
	{
		// It's pointing away from (or is perpendicular to) the polygon.
		if( Normal().Dot(ray.m_Dir) >= -0.001f )
			return FALSE;
	}
	else
	{
		// Backfacing.
		if( !bBackface )
		{
			return FALSE;
		}
		else
		{
			// It's pointing away from (or is perpendicular to) the polygon.
			if( Normal().Dot(ray.m_Dir) <= 0.001f )
				return FALSE;
		}
	}

	dot1 = Normal().Dot( ray.m_Pos ) - Dist();
	dot2 = Normal().Dot( ray.m_Pos + ray.m_Dir ) - Dist();

	div = dot2 - dot1;
	if(fabs(div) < 0.00001f)
		t = 0.0f;
	else
		t = -dot1 / div;
	
	ASSERT( t >= 0 );

	testPt = ray.m_Pos + (ray.m_Dir * t);
	
	// See if the intersection point lies inside the poly.
	uint32 nFanPt  = NumVerts() - 1;
	uint32 nPrevPt = 0;
	uint32 nCurrPt = 1;

	//the count of the number of triangles it is in
	uint32 nInCount = 0;

	for(; nCurrPt < NumVerts(); nPrevPt = nCurrPt, nCurrPt++)
	{
		//see if it the point is within the triangle defined by 
		//the edges fanpt->prev, fanpt->curr
		if( (g_TriArea2( Normal(), Pt(nFanPt),  Pt(nPrevPt), testPt ) > 0) ||
			(g_TriArea2( Normal(), Pt(nPrevPt), Pt(nCurrPt), testPt ) > 0) ||
			(g_TriArea2( Normal(), Pt(nCurrPt), Pt(nFanPt),  testPt ) > 0))
			continue;

		//we are in this tri
		nInCount++;
	}

	return (nInCount % 2) ? TRUE : FALSE;
}


BOOL CEditPoly::CopyEditPoly( CEditPoly *pPoly, BOOL bCopyIndices, CStringHolder *pStringHolder )
{
	if( bCopyIndices )
		m_Indices.CopyArray( pPoly->m_Indices );

	CopyAttributes( pPoly, pStringHolder );
	m_pBrush = pPoly->m_pBrush;
	
	return TRUE;
}

void CEditPoly::Flip()
{
	uint32		i, temp;

	for( i=0; i < m_Indices/2; i++ )
	{
		temp = m_Indices[i];
		m_Indices[i] = m_Indices[m_Indices-i-1];
		m_Indices[m_Indices-i-1] = temp;
	}

	Normal() = -Normal();
	Dist() = -Dist();
}


void CEditPoly::SetupBaseTextureSpace(uint32 nTex)
{
	uint32 i, closestPlane;
	CReal dot, maxDot;


	// Find the closest texture alignment plane to this poly.
	maxDot = (CReal)-MAX_CREAL;

	closestPlane = 0;
	for( i=0; i < NUM_TEXTURE_PLANES; i++ )
	{
		dot = Normal().Dot( g_TexturePlanes[i] );
		if( dot > maxDot )
		{
			maxDot = dot;
			closestPlane = i;
		}
	}

	// SCHLEGZ 1/22/98: fixed problem with deleting previous mag
	// MD 3/25/98: Polies now use explicit scale variables.
	GetTexture(nTex).SetP(g_RightVectors[closestPlane]);
	GetTexture(nTex).SetQ(GetTexture(nTex).GetP().Cross( g_TexturePlanes[closestPlane] ));
}

void CEditPoly::SetTextureSpace(uint32 nTex, const LTVector& newO, const LTVector& newP, const LTVector& newQ)
{
	GetTexture(nTex).SetTextureSpace(GetTextureNormal(), newO, newP, newQ);
}

// sets up OPQs based on uv coordinates for each vertex
// takes an array of 6 floats, 2 per vertex for the first 3 verts plus the width and height of the texture
bool CEditPoly::SetUVTextureSpace( uint32 nTex, const float* coords, const int texWidth, const int texHeight )
{
	if ( this->m_pBrush->m_Points == NULL || this->m_pBrush->m_Points.GetSize() < 3)  
		return false; 

	//build up a vertex list
	ASSERT(this->NumVerts() >= 3);

	LTVector vPos[3];
	vPos[0] = m_pBrush->m_Points[Index(0)];
	vPos[1] = m_pBrush->m_Points[Index(1)];
	vPos[2] = m_pBrush->m_Points[Index(2)];

	//now get the OPQ vectors
	CTexturedPlane& Texture = GetTexture(nTex);

	LTVector vO, vP, vQ;
	bool bRV = ConvertUVToOPQ(vPos, coords, texWidth, texHeight, vO, vP, vQ);	

	Texture.SetO(vO);
	Texture.SetP(vP);
	Texture.SetQ(vQ);

	return bRV;
}

//determines if this polygon is concave or not
bool CEditPoly::IsConcave()
{
	uint32 nNumPts = NumVerts();

	if(nNumPts <= 3)
	{
		return false;
	}

	//get the normal for this polygon
	LTVector vNormal = Normal();

	//now go through every edge
	uint32 nPrevPt = nNumPts - 1;
	for(uint32 nCurrPt = 0; nCurrPt < nNumPts; nPrevPt = nCurrPt, nCurrPt++)
	{
		//build the edge normal
		LTVector vEdge = m_pBrush->m_Points[Index(nCurrPt)] - m_pBrush->m_Points[Index(nPrevPt)];

		//find the normal
		LTVector vEdgeNorm = vNormal.Cross(vEdge);
		vEdgeNorm.Norm();

		//now run through all the other points
		for(uint32 nTestPt = 0; nTestPt < nNumPts; nTestPt++)
		{
			//ignore the points on the edge
			if((nTestPt == nCurrPt) || (nTestPt == nPrevPt))
				continue;

			//see if it is on the correct side
			if(vEdgeNorm.Dot(m_pBrush->m_Points[Index(nTestPt)] - m_pBrush->m_Points[Index(nCurrPt)]) > 0.001f)
			{
				return true;
			}
		}
	}

	return false;

}

//determines if this polygon lies entirely within a single plane
bool CEditPoly::IsCoplanar()
{
	//run through all the points, projecting them onto the normal. The
	//value should be relatively the same for all the points
	if(NumVerts() < 4)
	{
		//if we have 3 or less points, we mathmatically have to be coplanar, so just bail to
		//prevent mathmatical errors from causing problems
		return true;
	}

	//the epsilon value that we allow points to stray from the plane
	static const float kfPlaneEpsilon = 0.01f;

	LTVector vNormal = Normal();

	CReal fDist = m_pBrush->m_Points[Index(0)].Dot(Normal());

	for(uint32 nCurrPt = 1; nCurrPt < NumVerts(); nCurrPt++)
	{
		CReal fTestDist = m_pBrush->m_Points[Index(nCurrPt)].Dot(Normal());

		fTestDist -= fDist;

		if((fTestDist < -kfPlaneEpsilon) || (fTestDist > kfPlaneEpsilon))
		{
			return false;
		}
	}

	return true;
}

//determines the surface area of the polygon
CReal CEditPoly::GetSurfaceArea()
{
	//accumulate the area occupied by each triangle
	CReal fArea = 0.0f;

	uint32 nNumPts = NumVerts();

	//safety first...
	if(nNumPts < 3)
		return 0.0f;

	//fan out from vertex 0 to all other verts
	LTVector vFanPt = m_pBrush->m_Points[Index(0)];

	for(uint32 nCurrPt = 1; nCurrPt < nNumPts - 1; nCurrPt++)
	{
		//get the two edges
		LTVector vEdge1 = m_pBrush->m_Points[Index(nCurrPt)] - vFanPt;
		LTVector vEdge2 = m_pBrush->m_Points[Index(nCurrPt + 1)] - vFanPt;

		//the cross product gives us the area of the rect, so we want to add
		//half of that
		fArea += vEdge1.Cross(vEdge2).Mag() / 2;
	}

	return fArea;
}

