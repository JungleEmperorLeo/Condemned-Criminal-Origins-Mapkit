#include "bdefs.h"
#include "dedit.h"
#include "colorschemes.h"
#include "optionsdisplay.h"
#include "edithelpers.h"

namespace
{
	struct SchemeColor
	{
		int			m_nColor;		// COptionsDisplay::kColor...
		COLORREF	m_crColor;
	};

	// Every scheme starts from DEdit's default colours and replaces the ones listed.
	static const SchemeColor g_Hammer[] =
	{
		{ COptionsDisplay::kColorBackground,				RGB(0, 0, 0) },
		{ COptionsDisplay::kColorMajorGrid,					RGB(96, 96, 96) },
		{ COptionsDisplay::kColorMinorGrid,					RGB(45, 45, 45) },
		{ COptionsDisplay::kColorBrushDrawLines,			RGB(255, 255, 255) },
		{ COptionsDisplay::kColorBrushDrawLinesConcave,		RGB(255, 128, 0) },
		{ COptionsDisplay::kColorBrushDrawLinesIntersect,	RGB(255, 0, 0) },
		{ COptionsDisplay::kColorSelectedBrush,				RGB(255, 0, 0) },
		{ COptionsDisplay::kColorFrozenBrush,				RGB(110, 110, 255) },
		{ COptionsDisplay::kColorObject,					RGB(255, 0, 255) },
		{ COptionsDisplay::kColorSelectedObject,			RGB(255, 0, 0) },
		{ COptionsDisplay::kColorBoundingBox,				RGB(128, 128, 128) },
	};

	static const SchemeColor g_Blueprint[] =
	{
		{ COptionsDisplay::kColorBackground,				RGB(14, 42, 84) },
		{ COptionsDisplay::kColorMajorGrid,					RGB(60, 110, 170) },
		{ COptionsDisplay::kColorMinorGrid,					RGB(28, 70, 120) },
		{ COptionsDisplay::kColorBrushDrawLines,			RGB(220, 235, 255) },
		{ COptionsDisplay::kColorBrushDrawLinesConcave,		RGB(255, 160, 60) },
		{ COptionsDisplay::kColorBrushDrawLinesIntersect,	RGB(255, 90, 90) },
		{ COptionsDisplay::kColorSelectedBrush,				RGB(255, 230, 80) },
		{ COptionsDisplay::kColorFrozenBrush,				RGB(120, 150, 200) },
		{ COptionsDisplay::kColorObject,					RGB(120, 255, 210) },
		{ COptionsDisplay::kColorSelectedObject,			RGB(255, 255, 255) },
		{ COptionsDisplay::kColorObjectRadius,				RGB(90, 200, 255) },
		{ COptionsDisplay::kColorObjectLink,				RGB(255, 230, 80) },
		{ COptionsDisplay::kColorBoundingBox,				RGB(150, 180, 220) },
	};

	static const SchemeColor g_Amber[] =
	{
		{ COptionsDisplay::kColorBackground,				RGB(12, 8, 0) },
		{ COptionsDisplay::kColorMajorGrid,					RGB(120, 76, 0) },
		{ COptionsDisplay::kColorMinorGrid,					RGB(52, 32, 0) },
		{ COptionsDisplay::kColorBrushDrawLines,			RGB(255, 176, 0) },
		{ COptionsDisplay::kColorBrushDrawLinesConcave,		RGB(255, 96, 0) },
		{ COptionsDisplay::kColorBrushDrawLinesIntersect,	RGB(255, 48, 0) },
		{ COptionsDisplay::kColorNormals,					RGB(255, 210, 100) },
		{ COptionsDisplay::kColorSelectedBrush,				RGB(255, 240, 160) },
		{ COptionsDisplay::kColorFrozenBrush,				RGB(128, 80, 0) },
		{ COptionsDisplay::kColorObject,					RGB(255, 140, 0) },
		{ COptionsDisplay::kColorSelectedObject,			RGB(255, 255, 200) },
		{ COptionsDisplay::kColorObjectRadius,				RGB(200, 130, 0) },
		{ COptionsDisplay::kColorObjectLink,				RGB(255, 200, 60) },
		{ COptionsDisplay::kColorBoundingBox,				RGB(170, 110, 0) },
		{ COptionsDisplay::kColorOrigin,					RGB(255, 220, 120) },
		{ COptionsDisplay::kColorMarker,					RGB(255, 200, 0) },
	};

	static const SchemeColor g_Paper[] =
	{
		{ COptionsDisplay::kColorBackground,				RGB(236, 236, 230) },
		{ COptionsDisplay::kColorMajorGrid,					RGB(170, 170, 165) },
		{ COptionsDisplay::kColorMinorGrid,					RGB(206, 206, 200) },
		{ COptionsDisplay::kColorBrushDrawLines,			RGB(30, 30, 30) },
		{ COptionsDisplay::kColorBrushDrawLinesConcave,		RGB(200, 80, 0) },
		{ COptionsDisplay::kColorBrushDrawLinesIntersect,	RGB(200, 0, 0) },
		{ COptionsDisplay::kColorNormals,					RGB(0, 130, 150) },
		{ COptionsDisplay::kColorSelectedBrush,				RGB(0, 90, 220) },
		{ COptionsDisplay::kColorFrozenBrush,				RGB(120, 120, 180) },
		{ COptionsDisplay::kColorObject,					RGB(200, 0, 60) },
		{ COptionsDisplay::kColorSelectedObject,			RGB(0, 0, 0) },
		{ COptionsDisplay::kColorObjectRadius,				RGB(0, 130, 150) },
		{ COptionsDisplay::kColorObjectLink,				RGB(150, 100, 0) },
		{ COptionsDisplay::kColorObjectDims,				RGB(30, 30, 30) },
		{ COptionsDisplay::kColorPath,						RGB(150, 100, 0) },
		{ COptionsDisplay::kColorBoundingBox,				RGB(90, 90, 90) },
		{ COptionsDisplay::kColorOrigin,					RGB(150, 100, 0) },
		{ COptionsDisplay::kColorMarker,					RGB(0, 140, 0) },
		{ COptionsDisplay::kColorHandleOutline,				RGB(30, 30, 30) },
		{ COptionsDisplay::kColorHandleFill,				RGB(255, 255, 255) },
	};

	struct Scheme
	{
		const char			*m_pName;
		const SchemeColor	*m_pColors;
		int					m_nColors;
	};

#define SCHEME_COLORS(a)	a, (int)(sizeof(a) / sizeof(a[0]))

	static const Scheme g_Schemes[] =
	{
		{ "DEdit (default)",	NULL, 0 },
		{ "Hammer",				SCHEME_COLORS(g_Hammer) },
		{ "Blueprint",			SCHEME_COLORS(g_Blueprint) },
		{ "Amber Terminal",		SCHEME_COLORS(g_Amber) },
		{ "Paper",				SCHEME_COLORS(g_Paper) },
	};

	static const int g_nSchemes = (int)(sizeof(g_Schemes) / sizeof(g_Schemes[0]));

	static int		g_nCurrent		= 0;
	static bool		g_bCycle		= false;
	static int		g_nCycleSeconds	= 30;
	static UINT_PTR	g_nTimer		= 0;

	static COptionsDisplay* GetDisplay()
	{
		return GetApp() ? GetApp()->GetOptions().GetDisplayOptions() : NULL;
	}

	static void StopTimer()
	{
		if(g_nTimer)
		{
			::KillTimer(NULL, g_nTimer);
			g_nTimer = 0;
		}
	}

	static VOID CALLBACK CycleTimerProc(HWND, UINT, UINT_PTR, DWORD)
	{
		ColorSchemes::Next();
	}

	static void StartTimer()
	{
		StopTimer();
		if(g_bCycle)
			g_nTimer = ::SetTimer(NULL, 0, (UINT)g_nCycleSeconds * 1000, (TIMERPROC)CycleTimerProc);
	}

	static void SaveSettings()
	{
		COptionsDisplay *pDisplay = GetDisplay();
		if(!pDisplay)
			return;
		pDisplay->SetBoolValue("ColorSchemeCycle", g_bCycle ? TRUE : FALSE);
		pDisplay->SetDWordValue("ColorSchemeCycleSeconds", (DWORD)g_nCycleSeconds);
		pDisplay->SetDWordValue("ColorSchemeIndex", (DWORD)g_nCurrent);
	}
}

int ColorSchemes::Count()
{
	return g_nSchemes;
}

const char* ColorSchemes::Name(int nScheme)
{
	if(nScheme < 0 || nScheme >= g_nSchemes)
		return "";
	return g_Schemes[nScheme].m_pName;
}

int ColorSchemes::Current()
{
	return g_nCurrent;
}

void ColorSchemes::Apply(int nScheme, bool bRedraw)
{
	COptionsDisplay *pDisplay = GetDisplay();
	if(!pDisplay || nScheme < 0 || nScheme >= g_nSchemes)
		return;

	pDisplay->ResetToDefaultColors();

	const Scheme &scheme = g_Schemes[nScheme];
	for(int i=0; i < scheme.m_nColors; i++)
	{
		CDisplayColorItem *pItem = pDisplay->GetColorItem(scheme.m_pColors[i].m_nColor);
		if(pItem)
			pItem->SetColor(scheme.m_pColors[i].m_crColor);
	}

	g_nCurrent = nScheme;
	pDisplay->Save();
	SaveSettings();

	if(bRedraw)
		RedrawAllDocuments();
}

void ColorSchemes::Next()
{
	Apply((g_nCurrent + 1) % g_nSchemes);
}

bool ColorSchemes::IsCycling()
{
	return g_bCycle;
}

int ColorSchemes::GetCycleSeconds()
{
	return g_nCycleSeconds;
}

void ColorSchemes::SetCycle(bool bCycle, int nSeconds)
{
	if(nSeconds < 2)
		nSeconds = 2;
	if(nSeconds > 3600)
		nSeconds = 3600;

	g_bCycle = bCycle;
	g_nCycleSeconds = nSeconds;
	StartTimer();
	SaveSettings();
}

void ColorSchemes::Startup()
{
	COptionsDisplay *pDisplay = GetDisplay();
	if(!pDisplay)
		return;

	g_bCycle		= pDisplay->GetBoolValue("ColorSchemeCycle", FALSE) ? true : false;
	g_nCycleSeconds	= (int)pDisplay->GetDWordValue("ColorSchemeCycleSeconds", 30);
	g_nCurrent		= (int)pDisplay->GetDWordValue("ColorSchemeIndex", 0);
	if(g_nCycleSeconds < 2 || g_nCycleSeconds > 3600)
		g_nCycleSeconds = 30;
	if(g_nCurrent < 0 || g_nCurrent >= g_nSchemes)
		g_nCurrent = 0;

	StartTimer();
}
