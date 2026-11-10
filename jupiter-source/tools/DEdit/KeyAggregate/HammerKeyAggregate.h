#ifndef __HAMMERKEYAGGREGATE_H__
#define __HAMMERKEYAGGREGATE_H__

#ifndef __KEYDEFAULTAGGREGATE_H__
#	include "KeyDefaultAggregate.h"
#endif

#include "eventnames.h"
#include "hotkey.h"

// Controls modelled on Valve's Hammer editor (H / U to hide and unhide, Esc to
// clear the selection, [ and ] for the grid size, middle mouse to pan). Polygon drawing stays on Space. Anything not listed here keeps
// DEdit's own default.
class CHammerKeyAggregate : 
	public CKeyDefaultAggregate
{
private:

	bool InternalDefineKey(CHotKey& HotKey)
	{
		const char* pszName = HotKey.GetEventName();

		// Middle mouse drag pans the 2D views / slides the 3D camera sideways and up
		// (Hammer uses Space+LMB, but Space is DEdit's polygon drawing key).
		if(stricmp(pszName, UIE_NAV_DRAG) == 0)
		{
			HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSEMIDDLE));
		}
		else if(stricmp(pszName, UIE_NAV_DRAG_FAST) == 0)
		{
			HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_SHIFT));
		}

		// Z toggles free flight: mouse look + W A S D
		else if(stricmp(pszName, UIE_MOUSE_LOOK) == 0)
		{
			HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'Z'));
		}

		// Z is taken by free flight, so the grid size drag moves to Ctrl + right mouse
		else if(stricmp(pszName, UIE_GRID_SIZE) == 0)
		{
			HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_CONTROL));
			HotKey.AddEvent(CUIMouseEvent(UIEVENT_MOUSEDOWN, UI_MOUSERIGHT));
		}

		// Selection / visibility
		else if(stricmp(pszName, UIE_SELECT_NONE) == 0)
		{
			HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, VK_ESCAPE));
		}
		else if(stricmp(pszName, UIE_HIDE_SELECTED) == 0)
		{
			HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'H'));
		}
		else if(stricmp(pszName, UIE_UNHIDE_SELECTED) == 0)
		{
			HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 'U'));
		}

		// Grid size: [ and ] like Hammer (VK_OEM_4 / VK_OEM_6)
		else if(stricmp(pszName, UIE_SHRINK_GRID_SPACING) == 0)
		{
			HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 0xDB));
		}
		else if(stricmp(pszName, UIE_EXPAND_GRID_SPACING) == 0)
		{
			HotKey.AddEvent(CUIKeyEvent(UIEVENT_KEYDOWN, 0xDD));
		}

		// Clone while moving stays on DEdit's default (D): Shift is box select / axis lock here

		else
		{
			//didn't match any of the others, we want the default database
			//to set up the key configuration
			return false;
		}

		return true;
	}
};

#endif
