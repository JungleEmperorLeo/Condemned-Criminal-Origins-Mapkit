//------------------------------------------------------------------
//
//	FILE	  : MaterialSurfaces.h
//
//	PURPOSE	  : Gets the list of surface types (concrete, metal, wood...) that
//	            a material's SurfaceFlags value refers to. The names live in the
//	            game, not in the material, so - like F.E.A.R.'s WorldEdit - this
//	            loads the game's GameServer.dll and asks its IGameWorldEdit
//	            interface (exported as GetIGameWorldEdit).
//
//	            If the DLL can't be loaded or doesn't answer, nothing breaks:
//	            the properties dialog just shows SurfaceFlags as a number.
//
//------------------------------------------------------------------

#ifndef __MATERIALSURFACES_H__
#define __MATERIALSURFACES_H__

#include <string>
#include <vector>
#include <stdint.h>

struct MaterialSurface
{
	std::string	name;
	uint32_t	id;
};

class CMaterialSurfaces
{
public:
	static CMaterialSurfaces& Get();

	// Loads GameServer.dll from the given full path. Returns true if surfaces were read.
	// On failure pError (if given) describes why.
	bool	Load(const char *pGameServerDll, std::string *pError = 0);

	// Tries the remembered path, then a few likely places. Silent; returns IsLoaded().
	bool	TryAutoLoad();

	bool	IsLoaded() const	{ return !m_Surfaces.empty(); }
	const std::vector<MaterialSurface>& Surfaces() const	{ return m_Surfaces; }

	// Name for an id, or "" if unknown.
	std::string NameOf(uint32_t id) const;

	std::string	m_LoadedFrom;
	std::string	m_LoadError;		// why the automatic load failed, if it did

private:
	CMaterialSurfaces() {}
	std::vector<MaterialSurface>	m_Surfaces;
};

#endif // __MATERIALSURFACES_H__
