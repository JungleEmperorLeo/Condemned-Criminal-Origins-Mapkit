//------------------------------------------------------------------
//
//	FILE	  : ColorSchemes.h
//
//	PURPOSE	  : Whole-editor colour scheme presets (DEdit's own, Hammer's,
//	            and a few more) and an optional timer that steps through
//	            all of them automatically.
//
//------------------------------------------------------------------

#ifndef __COLORSCHEMES_H__
#define __COLORSCHEMES_H__

namespace ColorSchemes
{
	int			Count();
	const char*	Name(int nScheme);

	// Replaces every editor colour with the scheme's colours (and saves them).
	void		Apply(int nScheme, bool bRedraw = true);

	// The scheme applied last (what the options page selects); 0 = DEdit default.
	int			Current();

	// Applies the next scheme in the list, wrapping around.
	void		Next();

	// Automatic cycling. The settings are stored with the display options.
	bool		IsCycling();
	int			GetCycleSeconds();
	void		SetCycle(bool bCycle, int nSeconds);

	// Called once at start up: restores the cycling settings and starts the timer.
	void		Startup();
}

#endif
