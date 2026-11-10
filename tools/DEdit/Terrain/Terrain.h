//------------------------------------------------------------------
//
//	FILE	  : Terrain.h
//
//	PURPOSE	  : Paintable terrain. A brush is converted into a grid of
//	            triangular prism brushes (one per triangle) grouped under
//	            a "TerrainNNNN" node. Heights are painted straight into the
//	            prisms; blended texture layers are baked into
//	            Textures\Terrainmesh\NNNN.dtx (32 bit DTX) and kept in
//	            Textures\Terrainmesh\NNNN.trn.
//
//------------------------------------------------------------------

#ifndef __TERRAIN_H__
#define __TERRAIN_H__

class CRegionView;
class DrawBase;

namespace Terrain
{
	enum
	{
		MODE_RAISE = 0,
		MODE_LOWER,
		MODE_SMOOTH,
		MODE_FLATTEN,
		MODE_PAINT1,
		MODE_PAINT2,
		MODE_PAINT3,
		MODE_PAINT4,
		NUM_MODES
	};

	// Brush -> Convert to Terrain.
	void	ConvertSelection(CRegionView *pView);

	// Brush -> Terrain Tools (shows / hides the tool window).
	void	ToggleToolsWindow();
	bool	IsToolsWindowVisible();

	// Painting is active while the tool window is open with "Paint" checked.
	bool	IsActive();
	bool	IsStroking();

	// While the tool window is open, left clicks don't select (camera
	// navigation keys still work).
	bool	BlocksSelection();

	// Mouse hooks for the region views. BeginStroke returns false if the
	// click didn't hit a terrain (so the view handles it normally).
	// Left button uses the selected mode; the right button inverts Raise / Lower.
	// Holding Shift (height modes) smooths instead.
	bool	BeginStroke(CRegionView *pView, CPoint point, bool bRightButton = false);
	bool	IsStrokeRightButton();
	void	ContinueStroke(CRegionView *pView, CPoint point);
	void	EndStroke(CRegionView *pView);

	// Ctrl + wheel changes the radius, Ctrl + Shift + wheel the strength. Only
	// handled while the tool window is open; returns true when it used the wheel.
	bool	HandleMouseWheel(short zDelta);

	// Brush reticle under the mouse while the tool window is open.
	void	UpdateHover(CRegionView *pView, CPoint point);
	void	DrawReticle(DrawBase *pDraw);
}

#endif
