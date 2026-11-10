//------------------------------------------------------------------
//
//	FILE	  : MaterialSurfaces.cpp
//
//	PURPOSE	  : See MaterialSurfaces.h
//
//------------------------------------------------------------------

#include "bdefs.h"
#include "dedit.h"
#include "edithelpers.h"
#include "MaterialSurfaces.h"
#include "d_filemgr.h"
#include <stdlib.h>

#include <windows.h>
#include <string.h>
#include <stdio.h>

// Same layout as the engine's IGameWorldEdit (sdk\inc\igameworldedit.h).
class IGameWorldEditLocal
{
public:
	virtual unsigned long	GetSurfaceCount() = 0;
	virtual bool			GetSurfaceName(unsigned long nSurface, char *szSurface, unsigned long nLength) = 0;
	virtual bool			GetSurfaceId(unsigned long nSurface, unsigned long &nId) = 0;
	virtual bool			ValidateSurfaceRecords(char *szError, unsigned long nLength) = 0;
};

typedef IGameWorldEditLocal* (*PFNGETIGAMEWORLDEDITLOCAL)();

CMaterialSurfaces& CMaterialSurfaces::Get()
{
	static CMaterialSurfaces s;
	return s;
}

// Plain C style helper so __try is allowed (no C++ objects with destructors in here).
static int ReadSurfacesSEH(PFNGETIGAMEWORLDEDITLOCAL pFn, char *pNames, unsigned long *pIds, int maxCount, int nameLen)
{
	int count = 0;
	__try
	{
		IGameWorldEditLocal *pEdit = pFn();
		if(!pEdit)
			return -1;
		unsigned long n = pEdit->GetSurfaceCount();
		if(n > (unsigned long)maxCount)
			n = (unsigned long)maxCount;
		for(unsigned long i = 0; i < n; i++)
		{
			char *pName = pNames + (size_t)count * nameLen;
			unsigned long id = 0;
			pName[0] = 0;
			if(!pEdit->GetSurfaceName(i, pName, (unsigned long)nameLen))
				continue;
			pName[nameLen - 1] = 0;
			if(!pEdit->GetSurfaceId(i, id))
				id = i;
			pIds[count] = id;
			count++;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return -2;
	}
	return count;
}

bool CMaterialSurfaces::Load(const char *pGameServerDll, std::string *pError)
{
	m_Surfaces.clear();
	m_LoadedFrom.clear();

	if(!pGameServerDll || !pGameServerDll[0])
	{
		if(pError) *pError = "no path given";
		return false;
	}

	// The DLL needs its neighbours (the engine DLLs, its own dependencies), so look in its folder.
	char dir[MAX_PATH];
	strncpy(dir, pGameServerDll, MAX_PATH - 1);
	dir[MAX_PATH - 1] = 0;
	char *pSlash = strrchr(dir, '\\');
	if(!pSlash) pSlash = strrchr(dir, '/');
	if(pSlash) *pSlash = 0; else dir[0] = 0;

	// The game finds its database relative to the game folder (the parent of "Game"), so run from there.
	char gameDir[MAX_PATH];
	strcpy(gameDir, dir);
	{
		char *pLast = strrchr(gameDir, '\\');
		if(pLast && stricmp(pLast + 1, "Game") == 0)
			*pLast = 0;
	}

	struct CwdRestore
	{
		char old[MAX_PATH];
		CwdRestore(const char *pNew) { old[0] = 0; GetCurrentDirectory(MAX_PATH, old); if(pNew && pNew[0]) SetCurrentDirectory(pNew); }
		~CwdRestore() { if(old[0]) SetCurrentDirectory(old); SetDllDirectory(NULL); }
	} cwdGuard(gameDir);

	char oldCwd[MAX_PATH];
	oldCwd[0] = 0;
	if(dir[0])
		SetDllDirectory(dir);

	// The game database module sits next to the exe; load it first if it is there.
	{
		char dbDll[MAX_PATH];
		sprintf(dbDll, "%s\\GameDatabase.dll", gameDir);
		if(GetFileAttributes(dbDll) != INVALID_FILE_ATTRIBUTES)
			LoadLibraryEx(dbDll, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	}

	// VC7.1 runtimes live in the kit's Tools folder.
	{
		static const char *kRel[] = { "\\CondemnedMapKit\\Tools", "\\..\\CondemnedMapKit\\Tools", 0 };
		for(int i = 0; kRel[i]; i++)
		{
			char d2[MAX_PATH], full2[MAX_PATH], dll2[MAX_PATH];
			sprintf(d2, "%s%s", gameDir, kRel[i]);
			if(!_fullpath(full2, d2, MAX_PATH))
				continue;
			sprintf(dll2, "%s\\msvcr71.dll", full2);
			if(GetFileAttributes(dll2) == INVALID_FILE_ATTRIBUTES)
				continue;
			LoadLibraryEx(dll2, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
			sprintf(dll2, "%s\\msvcp71.dll", full2);
			LoadLibraryEx(dll2, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
			break;
		}
	}

	HMODULE hMod = LoadLibraryEx(pGameServerDll, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	DWORD err = GetLastError();

	if(!hMod)
	{
		if(pError)
		{
			char buf[200];
			sprintf(buf, "could not load the DLL (Windows error %lu)", (unsigned long)err);
			*pError = buf;
		}
		return false;
	}

	PFNGETIGAMEWORLDEDITLOCAL pFn = (PFNGETIGAMEWORLDEDITLOCAL)GetProcAddress(hMod, "GetIGameWorldEdit");
	if(!pFn)
	{
		if(pError) *pError = "the DLL does not export GetIGameWorldEdit";
		return false;
	}

	const int kMax = 512, kLen = 128;
	std::vector<char> names((size_t)kMax * kLen);
	std::vector<unsigned long> ids(kMax);
	int n = ReadSurfacesSEH(pFn, &names[0], &ids[0], kMax, kLen);
	if(n < 0)
	{
		if(pError) *pError = (n == -1) ? "the game returned no surface interface (its surface data may need the game to be running)"
										: "the game crashed while listing surfaces";
		return false;
	}
	if(n == 0)
	{
		if(pError) *pError = "the game reported no surface types";
		return false;
	}

	for(int i = 0; i < n; i++)
	{
		MaterialSurface s;
		s.name = &names[(size_t)i * kLen];
		s.id = (uint32_t)ids[i];
		m_Surfaces.push_back(s);
	}
	m_LoadedFrom = pGameServerDll;
	return true;
}

bool CMaterialSurfaces::TryAutoLoad()
{
	if(IsLoaded())
		return true;

	// Remembered path.
	CWinApp *pApp = AfxGetApp();
	if(pApp)
	{
		CString saved = pApp->GetProfileString("Materials", "GameServerDll", "");
		if(!saved.IsEmpty() && Load((LPCTSTR)saved))
			return true;
	}

	// The project's own game: <project folder>\..\Game\GameServer.dll (the Condemned layout).
	CString root = dfm_GetBaseDir(GetFileMgr());
	while(!root.IsEmpty() && (root[root.GetLength()-1] == '\\' || root[root.GetLength()-1] == '/'))
		root = root.Left(root.GetLength() - 1);
	if(!root.IsEmpty())
	{
		CString cand = root + "\\..\\Game\\GameServer.dll";
		char full[MAX_PATH];
		if(_fullpath(full, (LPCTSTR)cand, MAX_PATH) && GetFileAttributes(full) != INVALID_FILE_ATTRIBUTES)
		{
			std::string err;
			if(Load(full, &err))
			{
				if(pApp)
					pApp->WriteProfileString("Materials", "GameServerDll", full);
				return true;
			}
			m_LoadError = err;
		}
	}
	return false;
}

std::string CMaterialSurfaces::NameOf(uint32_t id) const
{
	for(size_t i = 0; i < m_Surfaces.size(); i++)
		if(m_Surfaces[i].id == id)
			return m_Surfaces[i].name;
	return std::string();
}
