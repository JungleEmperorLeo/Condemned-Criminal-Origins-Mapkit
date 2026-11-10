//------------------------------------------------------------------
//
//	FILE	  : LightPreview.h
//
//	PURPOSE	  : Lightmap preview for the 3D views. Lights the level from
//	            its Light / DirLight objects (using the same attenuation,
//	            Lambert and cone model as the lightmap sample generator,
//	            with optional shadows) into per-poly lightmaps packed into
//	            256x256 pages, which the D3D renderer modulates over the
//	            base textures.
//
//------------------------------------------------------------------

#ifndef __LIGHTPREVIEW_H__
#define __LIGHTPREVIEW_H__

class CEditRegion;
class CEditPoly;
class D3DRender;
struct IDirect3DTexture2;

namespace LightPreview
{
	// Is the preview showing for this region?
	bool	IsEnabled(CEditRegion *pRegion);

	// Turns the preview on (computing the lighting) or off.
	void	Toggle(CEditRegion *pRegion);

	// Recomputes the lighting for the region and enables the preview.
	void	Compute(CEditRegion *pRegion);

	// Turns the preview off and frees everything.
	void	Clear();

	// Call when a region is about to go away. Turns the preview off if it
	// was showing that region.
	void	NotifyRegionClosed(CEditRegion *pRegion);

	// For drawing. Returns the lightmap texture for this poly on this
	// renderer and fills in pUV (2 floats per vertex, in poly vertex order).
	IDirect3DTexture2*	GetPolyLightmap(D3DRender *pRender, CEditRegion *pRegion, CEditPoly *pPoly, float *pUV);

	// Must be called before a renderer releases its DirectDraw objects.
	void	ReleaseRender(D3DRender *pRender);
}

#endif
