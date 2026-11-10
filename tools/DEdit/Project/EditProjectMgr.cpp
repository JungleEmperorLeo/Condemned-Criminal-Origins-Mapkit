//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
//------------------------------------------------------------------
//
//	FILE	  : EditProjectMgr.cpp
//
//	PURPOSE	  : 
//
//	CREATED	  : December 16 1996
//
//
//------------------------------------------------------------------

// Includes....
#include "bdefs.h"
#include "oldtypes.h"
#include "objectimporter.h"
#include "editprojectmgr.h"
#include "resource.h"
#include "texture.h"
#include "edithelpers.h"
#include "iobjectplugin.h"
#include "deditinternal.h"
#include "objectselfilterdlg.h"
#include "lt1compat.h"

#define TEXTURE_TYPECODE		1


static CString edExtension;
static CString datExtension;
static CString texExtension;
static CString sndExtension;
static CString mdlExtension;
static CString sprExtension;



// called by the file manager before deleting a file identifier to clean up user data
static void EPM_GoingAwayFn(DFileIdent *pIdent, void *pUser)
{
	// file identifier is a texture, delete the texture
	if(pIdent->m_pUser && pIdent->m_UserType == 0)
	{
		dib_DestroyDibTexture((CTexture*)pIdent->m_pUser);
	}
}


static char* _CopyStringPointer(char *pIn)
{
	char *pRet;

	if(pIn)
	{
		pRet = (char*)malloc(strlen(pIn) + 1);
		strcpy(pRet, pIn);
	}
	else
	{
		pRet = (char*)malloc(1);
		pRet[0] = 0;
	}

	return pRet;
}


static void _ExtractClassHeirarchy(ClassDef *pClass, ClassDef **pHeirarchy, int *pnInHeirarchy)
{
	int i, nInHeirarchy;
	ClassDef *pCur;

	
	// Count the number of classes.
	nInHeirarchy = 0;
	pCur = pClass;
	while(pCur)
	{
		++nInHeirarchy;
		pCur = pCur->m_ParentClass;
	}

	*pnInHeirarchy = nInHeirarchy;

	// Now put them in the heirarchy.
	pCur = pClass;
	--nInHeirarchy;
	pCur = pClass;
	while(pCur)
	{
		pHeirarchy[nInHeirarchy] = pCur;
		--nInHeirarchy;
		pCur = pCur->m_ParentClass;
	}
}




CProjectClass::~CProjectClass()
{
	DeleteAndClearArray( m_Children );
}



CEditProjectMgr::CEditProjectMgr()
{
	m_hFileMgr = NULL;
	m_bProjectDirectorySet = FALSE;
	m_ClassDefs = NULL;
	m_nClassDefs = 0;

	m_pTemplateObjectImporter=new CObjectImporter;

	m_hModule = NULL;
	m_ObjectLayout = OBJLAYOUT_UNKNOWN;
}


CEditProjectMgr::~CEditProjectMgr()
{
	Term();	

	delete []m_pTemplateObjectImporter;
	m_pTemplateObjectImporter=NULL;
}


void CEditProjectMgr::OnOpenProject(const char *pBaseDir)
{
	Term();

	ASSERT(!m_hFileMgr);
	
	m_DibMgr.Init( AfxGetInstanceHandle(), GetDesktopWindow() );
	m_BaseProjectDir = pBaseDir;
	m_bProjectDirectorySet = TRUE;
	m_hFileMgr = dfm_Open((char*)pBaseDir);

	UpdateClasses();

	//update the filter dialog to reflect the new classes
	GetObjectSelFilterDlg()->UpdateClassListing();
}


void CEditProjectMgr::Term()
{
	if(m_hFileMgr)
	{
		// EPM_GoingAwayFn is at the top of this file.
		dfm_Close(m_hFileMgr, EPM_GoingAwayFn, this);
		m_hFileMgr = NULL;
	}

	UnloadBinaries();
	m_DibMgr.Term( );
	m_bProjectDirectorySet = FALSE;

	//update the filter dialog to reflect no classes
	GetObjectSelFilterDlg()->ClearClassListing();
}

void CEditProjectMgr::UpdateBUTFile()
{
	//get the resource directory for the currently open project, and add on
	//our filename to get the BUT file
	CString sBUTFile = m_BaseProjectDir + "\\ClassHlp.but";

	//now create a but manager to load this
	CButeMgr* pNewMgr = new CButeMgr;

	//bail if memory failed
	if(pNewMgr == NULL)
		return;

	if (!pNewMgr->Parse(sBUTFile))
	{
		//failed to find a new but file, no worries, just bail
		delete pNewMgr;
		return;
	}

	//this worked, we want to override the old but file with this one
	GetApp()->SetClassHelpButeAgg(pNewMgr);
	
}

void CEditProjectMgr::UpdateClasses()
{
	DeleteAndClearArray(m_Classes);

	// Load all binaries.
	LoadBinaries();

	// Make the class tree.
	MakeClassTree();

	// Update the BUT file if necessary
	UpdateBUTFile();
}


// Optional per-project override for the object module layout. If a file named
// dedit_engine.txt exists in the project directory and starts with "lt1" or
// "jupiter", that layout is used instead of auto-detection.
static EObjectModuleLayout ReadLayoutOverride(const CString &sProjectDir)
{
	CString sFile = sProjectDir + "\\dedit_engine.txt";
	FILE *pFile = fopen(sFile, "rt");
	if(!pFile)
		return OBJLAYOUT_UNKNOWN;

	char szLine[64];
	szLine[0] = 0;
	fgets(szLine, sizeof(szLine), pFile);
	fclose(pFile);

	if(strnicmp(szLine, "lt1", 3) == 0)
		return OBJLAYOUT_LT1;

	if(strnicmp(szLine, "jupiter", 7) == 0)
		return OBJLAYOUT_JUPITER;

	return OBJLAYOUT_UNKNOWN;
}


ClassDef** Condemned_GetClassDefs(int* pnClasses);
const char* const* Condemned_GetPropList(const char* pProp, int* pn);


#include <stdarg.h>
static char g_GsLogPath[MAX_PATH] = "";
static int  g_GsFailIdx = -1, g_GsFailWhy = 0, g_GsNClasses = 0;
static void GSLog(const char *fmt, ...)
{
	char buf[1024];
	va_list va;
	va_start(va, fmt);
	_vsnprintf(buf, sizeof(buf) - 1, fmt, va);
	buf[sizeof(buf) - 1] = 0;
	va_end(va);
	GetDebugDlg()->AddMessage("%s", buf);
	if(g_GsLogPath[0])
	{
		FILE *f = fopen(g_GsLogPath, "a");
		if(f) { fputs(buf, f); fclose(f); }
	}
}

// ---------------------------------------------------------------------------
// Condemned keeps its object classes in Game\GameServer.dll (a normal object
// module with ObjectDLLSetup). Load it for the real class hierarchy and
// properties; the result is validated so a mismatching layout can't crash us.
// ---------------------------------------------------------------------------
static int GS_LoadAndValidate(const char *pPath, HCLASSMODULE *phModule, int *pVersion, int *pnClasses)
{
	__try
	{
		int status = cb_LoadModuleRaw(pPath, NULL, phModule, pVersion);
		if(status != CB_NOERROR)
			return -10 - status;
		int n = cb_GetNumClassDefs(*phModule);
		ClassDef **pp = cb_GetClassDefs(*phModule);
		g_GsNClasses = n;
		if(!pp || n < 10 || n > 5000)
			return -2;
		for(int i = 0; i < n; i++)
		{
			ClassDef *pc = pp[i];
			if(!pc || !pc->m_ClassName)
				{ g_GsFailIdx = i; g_GsFailWhy = 1; return -1; }
			size_t len = strlen(pc->m_ClassName);
			if(len == 0 || len > 80)
				{ g_GsFailIdx = i; g_GsFailWhy = 2; return -1; }
			for(size_t k = 0; k < len; k++)
				if((unsigned char)pc->m_ClassName[k] < 32 || (unsigned char)pc->m_ClassName[k] > 126)
					{ g_GsFailIdx = i; g_GsFailWhy = 3; return -1; }
			if(pc->m_nProps < 0 || pc->m_nProps > 2000)
				{ g_GsFailIdx = i; g_GsFailWhy = 4; return -1; }
			for(int j = 0; j < pc->m_nProps; j++)
			{
				if(!pc->m_Props[j].m_PropName || strlen(pc->m_Props[j].m_PropName) > 120)
					{ g_GsFailIdx = i; g_GsFailWhy = 5; return -1; }
			}
			if(pc->m_ParentClass)
			{
				const char *pn = pc->m_ParentClass->m_ClassName;
				if(!pn || strlen(pn) > 80)
					{ g_GsFailIdx = i; g_GsFailWhy = 6; return -1; }
			}
		}
		*pnClasses = n;
		return 1;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return -3;
	}
}

static int GS_PreHook(IObjectPlugin *pPlugin, const char *pDir, const char *pProp, char **aszStrings, DDWORD *pcStrings, DDWORD cMax, DDWORD cMaxLen)
{
	__try
	{
		return (int)pPlugin->PreHook_EditStringList(pDir, pProp, aszStrings, pcStrings, cMax, cMaxLen);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return -1;
	}
}

static char g_PlugLock[MAX_PATH] = "", g_PlugSkip[MAX_PATH] = "";
static bool GS_InSkipList(const char *key)
{
	if(strstr(key, "Sound") || strstr(key, "SmartObject") || !strncmp(key, "GameBase.", 9)) return true;	// GameBase plugin touches an engine pointer we do not have
		// DB-driven sound lists crash the foreign plugin code
	FILE *f = fopen(g_PlugSkip, "r"); if(!f) return false;
	char line[256]; bool hit = false;
	while(fgets(line, sizeof(line), f)) { size_t n = strlen(line); while(n && (line[n-1]=='\n'||line[n-1]=='\r')) line[--n]=0; if(!strcmp(line, key)) { hit = true; break; } }
	fclose(f); return hit;
}
static void GS_SetLock(const char *key)
{
	FILE *f = fopen(g_PlugLock, "w"); if(f) { fputs(key, f); fclose(f); }
}

struct GsCacheEnt { char name[96]; DDWORD n; char **s; };
static GsCacheEnt g_GsCache[1024];
static int g_GsCacheN = 0;
static GsCacheEnt* GS_CacheFind(const char *name)
{
	for(int i = 0; i < g_GsCacheN; i++) if(!strcmp(g_GsCache[i].name, name)) return &g_GsCache[i];
	return NULL;
}
static void GS_CacheAdd(const char *name, DDWORD n, char **s)
{
	if(g_GsCacheN >= 1024) return;
	GsCacheEnt &e = g_GsCache[g_GsCacheN++];
	strncpy(e.name, name, sizeof(e.name) - 1); e.name[sizeof(e.name) - 1] = 0;
	e.n = n; e.s = s;
}

static LONG WINAPI GS_CrashFilter(EXCEPTION_POINTERS *ep)
{
	static int busy = 0;
	if(busy++ == 0 && g_GsLogPath[0])
	{
		FILE *f = fopen(g_GsLogPath, "a");
		if(f)
		{
			BYTE *base = (BYTE*)GetModuleHandle(NULL);
			IMAGE_NT_HEADERS *nt = (IMAGE_NT_HEADERS*)(base + ((IMAGE_DOS_HEADER*)base)->e_lfanew);
			DWORD size = nt->OptionalHeader.SizeOfImage;
			DWORD eip = (DWORD)ep->ExceptionRecord->ExceptionAddress;
			fprintf(f, "CRASH code=%08X addr=%08X exeRVA=%08X accessAddr=%08X\n", (unsigned)ep->ExceptionRecord->ExceptionCode, eip, (unsigned)(eip - (DWORD)base),
				ep->ExceptionRecord->NumberParameters > 1 ? (unsigned)ep->ExceptionRecord->ExceptionInformation[1] : 0u);
			fprintf(f, "STACK(exe RVAs):");
			DWORD *sp = (DWORD*)ep->ContextRecord->Esp;
			int n = 0;
			__try { for(int i = 0; i < 2048 && n < 48; i++) if(sp[i] > (DWORD)base && sp[i] < (DWORD)base + size) { fprintf(f, " %X", (unsigned)(sp[i] - (DWORD)base)); n++; } } __except(EXCEPTION_EXECUTE_HANDLER) {}
			fprintf(f, "\n");
			fclose(f);
		}
	}
	return EXCEPTION_CONTINUE_SEARCH;
}

static void GS_Delete(IObjectPlugin *p)
{
	__try { delete p; } __except(EXCEPTION_EXECUTE_HANDLER) {}
}

static void GS_Unload(HCLASSMODULE h)
{
	__try { cb_UnloadModule(h); } __except(EXCEPTION_EXECUTE_HANDLER) {}
}

static IObjectPlugin* GS_MakePlugin(ClassDef *pDef)
{
	__try { return pDef->m_PluginFn(); } __except(EXCEPTION_EXECUTE_HANDLER) { return NULL; }
}

// GameServer.dll was built with VC7.1 and needs msvcr71/msvcp71, which ship with the kit's Tools folder.
static void GS_PreloadRuntimes(const char *pGameDir)
{
	static const char *kRel[] = { "\\..\\CondemnedMapKit\\Tools", "\\CondemnedMapKit\\Tools", "\\Tools", 0 };
	for(int i = 0; kRel[i]; i++)
	{
		char dir[MAX_PATH], full[MAX_PATH], dll[MAX_PATH];
		sprintf(dir, "%s%s", pGameDir, kRel[i]);
		if(!_fullpath(full, dir, MAX_PATH))
			continue;
		sprintf(dll, "%s\\msvcr71.dll", full);
		if(GetFileAttributes(dll) == INVALID_FILE_ATTRIBUTES)
			continue;
		LoadLibraryEx(dll, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
		sprintf(dll, "%s\\msvcp71.dll", full);
		LoadLibraryEx(dll, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
		SetDllDirectory(full);
		return;
	}
}

static bool TryLoadGameServer(const CString &sProjectDir, HCLASSMODULE *phModule, int *pVersion, CString *pPathOut)
{
	char full[MAX_PATH];
	CString cand = sProjectDir + "\\..\\Game\\GameServer.dll";
	if(!_fullpath(full, (LPCTSTR)cand, MAX_PATH) || GetFileAttributes(full) == INVALID_FILE_ATTRIBUTES)
		return false;

	char gameDir[MAX_PATH];
	strcpy(gameDir, full);
	char *slash = strrchr(gameDir, '\\');
	if(slash) *slash = 0;

	char oldDir[MAX_PATH];
	oldDir[0] = 0;
	GetCurrentDirectory(MAX_PATH, oldDir);
	SetCurrentDirectory(gameDir);
	SetDllDirectory(gameDir);
	{
		char root[MAX_PATH];
		strcpy(root, gameDir);
		char *sl = strrchr(root, '\\');
		if(sl) *sl = 0;
		GS_PreloadRuntimes(root);
	}

	HCLASSMODULE h = NULL;
	int version = 0, n = 0;
	int r = GS_LoadAndValidate(full, &h, &version, &n);

	if(oldDir[0])
		SetCurrentDirectory(oldDir);

	if(r == 1)
	{
		*phModule = h;
		*pVersion = version;
		*pPathOut = full;
		return true;
	}
	GSLog(" Could not use %s as the class table (code %d, Windows error %lu, fail class #%d why %d of %d); using the built-in class list.\n", full, r, (unsigned long)GetLastError(), g_GsFailIdx, g_GsFailWhy, g_GsNClasses);
	if(r <= -2 && r > -10 && h)
	{
		GS_Unload(h);
	}
	return false;
}


// The game database (Dark.Gamdb00p) is loaded through the DLL's DEditLoadModule export,
// which the real editor calls with the project folder. Class property lists (WeaponType...)
// and the surface names need it.
class IGameWorldEditProbe
{
public:
	virtual unsigned long	GetSurfaceCount() = 0;
	virtual bool			GetSurfaceName(unsigned long, char*, unsigned long) = 0;
	virtual bool			GetSurfaceId(unsigned long, unsigned long&) = 0;
	virtual bool			ValidateSurfaceRecords(char*, unsigned long) = 0;
};
typedef bool (__cdecl *PFNDEDITLOADMODULE_)(const char *);
typedef IGameWorldEditProbe* (*PFNGETIGAMEWORLDEDIT_)();

static int GS_InitDB(HMODULE h, const char *pDir, int resetCounter)
{
	__try
	{
		if(resetCounter)
			*(DWORD*)((BYTE*)h + 0x243c14) = 0;	// "already loaded" counter inside DEditLoadModule
		PFNDEDITLOADMODULE_ fn = (PFNDEDITLOADMODULE_)GetProcAddress(h, "DEditLoadModule");
		if(!fn)
			return -1;
		bool ok = fn(pDir);
		PFNGETIGAMEWORLDEDIT_ pGet = (PFNGETIGAMEWORLDEDIT_)GetProcAddress(h, "GetIGameWorldEdit");
		if(!pGet)
			return ok ? 0 : -2;
		IGameWorldEditProbe *p = pGet();
		if(!p)
			return ok ? 0 : -2;
		return (int)p->GetSurfaceCount();
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return -3;
	}
}

static void GS_InitDatabase(const CString &sProjectDir, const char *pGsPath)
{
	HMODULE h = GetModuleHandle(pGsPath);
	if(!h)
		return;

	CString dir = sProjectDir;
	while(!dir.IsEmpty() && (dir[dir.GetLength()-1] == '\\' || dir[dir.GetLength()-1] == '/'))
		dir = dir.Left(dir.GetLength() - 1);
	char full[MAX_PATH];
	if(_fullpath(full, (LPCTSTR)dir, MAX_PATH))
		dir = full;

	char oldDir[MAX_PATH];
	oldDir[0] = 0;
	GetCurrentDirectory(MAX_PATH, oldDir);
	CString gameDir = dir + "\\..";
	if(_fullpath(full, (LPCTSTR)gameDir, MAX_PATH))
		SetCurrentDirectory(full);

	// Candidates for what DEditLoadModule wants: the folder, the folder with a slash, a file inside it.
	CString cands[3];
	cands[0] = dir;
	cands[1] = dir + "\\";
	cands[2] = dir + "\\FEAR.dep";
	int n = 0;
	for(int i = 0; i < 3 && n <= 0; i++)
	{
		n = GS_InitDB(h, (LPCTSTR)cands[i], i);
		GSLog(" DEditLoadModule(\"%s\") -> %d surface type(s)\n", (LPCTSTR)cands[i], n);
	}

	if(oldDir[0])
		SetCurrentDirectory(oldDir);
}


extern HMODULE g_hCdmGameServer;

// Loads the game's GameServer.dll (its property plugins and database back the drop-down lists and
// surface names) and initialises its database like the real WorldEdit does.
static void GS_PrepareGameServer(const CString &sProjectDir)
{
	char full[MAX_PATH];
	CString cand = sProjectDir + "\\..\\Game\\GameServer.dll";
	if(!_fullpath(full, (LPCTSTR)cand, MAX_PATH) || GetFileAttributes(full) == INVALID_FILE_ATTRIBUTES)
	{
		GSLog(" GameServer.dll not found at %s\n", (LPCTSTR)cand);
		return;
	}
	char gameDir[MAX_PATH];
	strcpy(gameDir, full);
	char *slash = strrchr(gameDir, '\\');
	if(slash) *slash = 0;

	char oldDir[MAX_PATH];
	oldDir[0] = 0;
	GetCurrentDirectory(MAX_PATH, oldDir);
	SetCurrentDirectory(gameDir);
	SetDllDirectory(gameDir);
	{
		char root[MAX_PATH];
		strcpy(root, gameDir);
		char *sl = strrchr(root, '\\');
		if(sl) *sl = 0;
		GS_PreloadRuntimes(root);
	}
	HMODULE h = LoadLibraryEx(full, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	DWORD err = GetLastError();
	if(oldDir[0])
		SetCurrentDirectory(oldDir);
	if(!h)
	{
		GSLog(" Could not load %s (Windows error %lu)\n", full, (unsigned long)err);
		return;
	}
	g_hCdmGameServer = h;
	GSLog(" Loaded %s\n", full);
	GS_InitDatabase(sProjectDir, full);
}

void CEditProjectMgr::LoadBinaries()
{
	CString dllFilename, str;
	DWORD i, j;
	int status, version, nClasses;
	ClassDef **pClasses, *pInClass, *pOutClass;
	HKEY hKey;
	DWORD valType, dataBufSize;
	BYTE dataBuf[261];
	LT1ConvertedTable lt1Table;

	memset(&lt1Table, 0, sizeof(lt1Table));
	m_ObjectLayout = OBJLAYOUT_UNKNOWN;


	UnloadBinaries();


	if(!m_bProjectDirectorySet)
		return;

	sprintf(g_GsLogPath, "%s\\dedit_gs.log", (LPCTSTR)m_BaseProjectDir);
	{ FILE *lf = fopen(g_GsLogPath, "w"); if(lf) { fputs("DEdit class/database load log\n", lf); fclose(lf); } }
	// Look in the project directory for the object DLL.
	dllFilename = dfm_BuildName(m_BaseProjectDir, "object.lto");
	// Raw load: the module may be an LT 1.0 module, whose class table can't be
	// interpreted with the Jupiter structures until we know which one it is.
	// Condemned has no object.lto: use the built-in class table generated from the kit's object reference.
	bool bBuiltin = (GetFileAttributes(dllFilename) == INVALID_FILE_ATTRIBUTES);
	bool bGameModule = false;
	if(bBuiltin)
		GS_PrepareGameServer(m_BaseProjectDir);
	if(bGameModule)
	{
	}
	else if(bBuiltin)
	{
		status = CB_NOERROR;
		m_hModule = NULL;
		version = SERVEROBJ_VERSION;
	}
	else
		status = cb_LoadModuleRaw((char*)(LPCTSTR)dllFilename, NULL, &m_hModule, &version);

	if(status != CB_NOERROR)
	{
		if(status == CB_CANTFINDMODULE)
		{
			
			// If we can't load it, let's check out the error.
			DWORD dwErrorCode = GetLastError();

			// Allocacte a string containing a human readable description of the error
			LPVOID lpMsgBuf;
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				dwErrorCode,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
				);

            // Print out to the DEdit debug window.
			GetDebugDlg()->AddMessage(" Failed to load:\n %s\n"
									  " System Error Code: %d\n"
									  " Description: %s\n", dllFilename.GetBuffer(), dwErrorCode, (LPCTSTR)lpMsgBuf);

			// deallocate the string
			LocalFree( lpMsgBuf );

			str.Format(IDS_CANTFINDCLASSMODULE, dllFilename.GetBuffer());
		}
		else if(status == CB_NOTCLASSMODULE)
		{
			str.Format(IDS_INVALIDCLASSMODULE, dllFilename.GetBuffer());
		}
		else if(status == CB_VERSIONMISMATCH)
		{
			str.Format(IDS_INVALIDCLASSMODULEVERSION, dllFilename.GetBuffer(), SERVEROBJ_VERSION, version);
		}

		AppMessageBox(str, MB_OK);
		return;
	}
	
	// Get the ClassDefs.
	if(bBuiltin)
	{
		pClasses = Condemned_GetClassDefs(&nClasses);
		m_ObjectLayout = OBJLAYOUT_JUPITER;
		GetDebugDlg()->AddMessage(" No object.lto in %s - using the built-in Condemned class table (%d classes).\n", (LPCTSTR)m_BaseProjectDir, nClasses);
	}
	else
	{
	nClasses = cb_GetNumClassDefs(m_hModule);
	pClasses = cb_GetClassDefs(m_hModule);

	// Work out which engine this object module was built for.
	m_ObjectLayout = ReadLayoutOverride(m_BaseProjectDir);
	if(m_ObjectLayout == OBJLAYOUT_UNKNOWN)
		m_ObjectLayout = LT1Compat_DetectLayout((void**)pClasses, nClasses);
	if(bGameModule && m_ObjectLayout == OBJLAYOUT_UNKNOWN)
		m_ObjectLayout = OBJLAYOUT_JUPITER;	// validated in TryLoadGameServer
	}

	if(m_ObjectLayout == OBJLAYOUT_LT1)
	{
		GetDebugDlg()->AddMessage(" %s: LithTech 1.0 object module detected.\n", dllFilename.GetBuffer());

		if(!LT1Compat_Convert((void**)pClasses, nClasses, &lt1Table))
		{
			str.Format("Failed to convert the LithTech 1.0 class table in:\n%s", dllFilename.GetBuffer());
			AppMessageBox(str, MB_OK);

			cb_UnloadModule(m_hModule);
			m_hModule = NULL;
			return;
		}

		pClasses = lt1Table.m_ppClasses;
	}
	else if(m_ObjectLayout == OBJLAYOUT_JUPITER)
	{
		GetDebugDlg()->AddMessage(" %s: Jupiter object module detected.\n", dllFilename.GetBuffer());
	}
	else
	{
		str.Format("Couldn't recognize the class table layout of:\n%s\n\n"
				   "If this is an LT 1.0 or Jupiter module, put 'lt1' or 'jupiter' in a file called "
				   "dedit_engine.txt in the project directory.", dllFilename.GetBuffer());
		AppMessageBox(str, MB_OK);

		cb_UnloadModule(m_hModule);
		m_hModule = NULL;
		return;
	}

	// Clamp bad property types (now that the table is in the Jupiter layout).
	cb_VerifyClassDefs(pClasses, nClasses);

	// Copy the class data over.
	m_ClassDefs = (ClassDef*)malloc(sizeof(ClassDef) * nClasses);
	m_nClassDefs = nClasses;

	// Prepare some temporary data for editstring lists
	const DDWORD cMaxStrings = 8192;
	const DDWORD cMaxStringLen = 1024;
	char* aszStrings[cMaxStrings];
	static char* aszBuf[cMaxStrings];	// pristine buffers: the foreign plugin may overwrite aszStrings[i] pointers
	{for ( int iString = 0 ; iString < cMaxStrings ; iString++ )
	{
		aszStrings[iString] = aszBuf[iString] = new char[cMaxStringLen];
		aszBuf[iString][0] = 0;
	}}

	int cProps = 0;

	// Crash guard: if the previous run died inside the foreign plugin code, the lock file is still there; skip plugins then.
	char lockPath[MAX_PATH];
	_snprintf(lockPath, sizeof(lockPath) - 1, "%s\\dedit_plugins.lock", (LPCTSTR)m_BaseProjectDir);
	lockPath[sizeof(lockPath) - 1] = 0;
	bool bSkipPlugins = bBuiltin;	/* in-process plugin calls corrupt memory: disabled */ g_GsCacheN = 0;
	strcpy(g_PlugLock, lockPath);
	_snprintf(g_PlugSkip, sizeof(g_PlugSkip) - 1, "%s\\dedit_plugins.skip", (LPCTSTR)m_BaseProjectDir);
	g_PlugSkip[sizeof(g_PlugSkip) - 1] = 0;
	{	// a leftover lock names the property that killed the last run: blacklist it
		FILE *lf = fopen(lockPath, "r");
		if(lf)
		{
			char key[256] = ""; fgets(key, sizeof(key), lf); fclose(lf);
			if(strchr(key, '.') && !GS_InSkipList(key))
			{
				FILE *sf = fopen(g_PlugSkip, "a"); if(sf) { fprintf(sf, "%s\n", key); fclose(sf); }
				GSLog(" Previous run crashed in plugin %s - blacklisted.\n", key);
			}
		}
	}
	GSLog(" Reading %d classes...\n", m_nClassDefs);
	SetUnhandledExceptionFilter(GS_CrashFilter);

	// Read in all class definitions
	for(i=0; i < m_nClassDefs; i++)
	{
		pInClass = pClasses[i];
		pOutClass = &m_ClassDefs[i];
		
		memcpy(pOutClass, pInClass, sizeof(ClassDef));

		pOutClass->m_ClassName = _CopyStringPointer(pInClass->m_ClassName);

		// {BP 2/11/98}
		// Only allocate if there are some props to deal with...
		if( pInClass->m_nProps )
		{
			pOutClass->m_Props = (PropDef*)malloc(sizeof(PropDef) * pInClass->m_nProps);
			memcpy(pOutClass->m_Props, pInClass->m_Props, sizeof(PropDef) * pInClass->m_nProps);

			cProps += pInClass->m_nProps;

			for(j=0; j < pInClass->m_nProps; j++)
			{
				PropDef* pPropDef = &pOutClass->m_Props[j];
				pPropDef->m_PropName = _CopyStringPointer(pInClass->m_Props[j].m_PropName);
				pPropDef->m_DefaultValueString = _CopyStringPointer(pInClass->m_Props[j].m_DefaultValueString);
				
				// {BL 11/06/99}
				// See if this property supports EditStringLists

				ClassDef* pDef = pOutClass;
				if(bBuiltin && ((pPropDef->m_PropFlags & PF_STATICLIST) || (pPropDef->m_PropFlags & PF_DYNAMICLIST)))
				{
					GsCacheEnt *ce = GS_CacheFind(pPropDef->m_PropName);
					if(ce)
					{
						if(ce->n)
						{
							DEditInternal* pd = new DEditInternal;
							pd->m_cStrings = ce->n;
							pd->m_aszStrings = new char*[ce->n];
							for(DDWORD k = 0; k < ce->n; k++) pd->m_aszStrings[k] = _CopyStringPointer(ce->s[k]);
							pPropDef->m_pDEditInternal = pd;
						}
						pDef = NULL;	// already resolved (or known to have no list)
					}
				}
				bool bFound = false;
				while(pDef)
				{
					if(!bSkipPlugins && pDef->m_PluginFn && ((pPropDef->m_PropFlags & PF_STATICLIST) || (pPropDef->m_PropFlags & PF_DYNAMICLIST)) )
					{
						char pkey[256];
						_snprintf(pkey, sizeof(pkey) - 1, "%s.%s", pDef->m_ClassName, pPropDef->m_PropName);
						pkey[sizeof(pkey) - 1] = 0;
						if(bBuiltin && GS_InSkipList(pkey)) { pDef = pDef->m_ParentClass; continue; }
						if(bBuiltin) { GSLog("  plugin %s\n", pkey); GS_SetLock(pkey); }
						for(DDWORD q = 0; q < cMaxStrings; q++) { aszStrings[q] = aszBuf[q]; aszBuf[q][0] = 0; }
						IObjectPlugin* pPlugin = GS_MakePlugin(pDef);
						if( pPlugin )
						{
							DDWORD cStrings = 0;
							if ( LT_OK == GS_PreHook(pPlugin, m_BaseProjectDir, pPropDef->m_PropName,
																		  aszStrings, &cStrings, cMaxStrings, cMaxStringLen) )
							{
								if(cStrings > cMaxStrings) cStrings = cMaxStrings;
								DEditInternal* pDEditInternal = new DEditInternal;
								pPropDef->m_pDEditInternal = pDEditInternal;

								pDEditInternal->m_cStrings = cStrings;

								if( cStrings <= 0 )
 								{
 									pDEditInternal->m_aszStrings = NULL;
 								}
 								else
								{
									pDEditInternal->m_aszStrings = new char*[cStrings];
								
									for ( uint32 iString = 0 ; iString < cStrings ; iString++ )
									{
										pDEditInternal->m_aszStrings[iString] = _CopyStringPointer(aszStrings[iString]);
									}
								}

								GS_Delete(pPlugin);
								bFound = true;
								if(bBuiltin) GS_CacheAdd(pPropDef->m_PropName, cStrings, pDEditInternal->m_aszStrings);
								break;
							}

							GS_Delete(pPlugin);
						}
					}
					
					pDef = pDef->m_ParentClass;
				}
				if(bBuiltin && !bFound && !bSkipPlugins && ((pPropDef->m_PropFlags & PF_STATICLIST) || (pPropDef->m_PropFlags & PF_DYNAMICLIST)) && !GS_CacheFind(pPropDef->m_PropName))
					GS_CacheAdd(pPropDef->m_PropName, 0, NULL);
			}
		}
		else
			pOutClass->m_Props = NULL;
	}

	for ( int iString = 0 ; iString < cMaxStrings ; iString++ )
	{
		delete [] aszBuf[iString];
	}
	if(bBuiltin) { DeleteFile(lockPath); GSLog(" Class read finished.\n"); }
	// Drop-down values from the game database (the game DLL's own list code can't run in the editor).
	if(bBuiltin)
	{
		int nLists = 0;
		for(i = 0; i < m_nClassDefs; i++)
		{
			ClassDef *pC = &m_ClassDefs[i];
			for(j = 0; j < pC->m_nProps; j++)
			{
				PropDef *pd = &pC->m_Props[j];
				if(pd->m_pDEditInternal || !(pd->m_PropFlags & (PF_STATICLIST | PF_DYNAMICLIST)) || !pd->m_PropName)
					continue;
				int n = 0;
				// "Class.Prop" lists first (e.g. Brush.Type), then the plain property name
				char qual[256];
				_snprintf(qual, sizeof(qual) - 1, "%s.%s", pC->m_ClassName ? pC->m_ClassName : "", pd->m_PropName);
				qual[sizeof(qual) - 1] = 0;
				const char* const* list = Condemned_GetPropList(qual, &n);
				if(!list || n <= 0)
					list = Condemned_GetPropList(pd->m_PropName, &n);
				if(!list || n <= 0)
					continue;
				const char *def = pd->m_DefaultValueString ? pd->m_DefaultValueString : "";
				bool bHasDef = false;
				for(int k = 0; k < n && !bHasDef; k++) if(!stricmp(list[k], def)) bHasDef = true;
				int extra = bHasDef ? 0 : 1;
				DEditInternal *pdi = new DEditInternal;
				pdi->m_cStrings = n + extra;
				pdi->m_aszStrings = new char*[n + extra];
				int o = 0;
				if(extra) pdi->m_aszStrings[o++] = _CopyStringPointer((char*)def);
				for(int k = 0; k < n; k++) pdi->m_aszStrings[o++] = _CopyStringPointer((char*)list[k]);
				pd->m_pDEditInternal = pdi;
				nLists++;
			}
		}
		GSLog(" Filled %d drop-down list(s) from the game database.\n", nLists);
	}

	// Setup the parent info.
	for(i=0; i < m_nClassDefs; i++)
	{
		pInClass = pClasses[i];
		pOutClass = &m_ClassDefs[i];

		if(pInClass->m_ParentClass)
		{
			pOutClass->m_ParentClass = FindClassDef(pInClass->m_ParentClass->m_ClassName);
			ASSERT(pOutClass->m_ParentClass);
		}
		else
		{
			pOutClass->m_ParentClass = NULL;
		}
	}		

	// The converted table (LT 1.0 only) is no longer referenced.
	LT1Compat_Free(&lt1Table);

	LoadTemplateClasses();
}

void CEditProjectMgr::LoadTemplateClasses()
{	
	// Clear the currently loaded templates
	DeleteAndClearArray(m_TemplateClasses);

	// Load the template objects
	CObjectImporter *pImporter=GetTemplateObjectImporter();
	if (!pImporter)
	{
		return;
	}

	if (!pImporter->LoadObjectFile("lithtech.cls"))
	{
		return;
	}

	// Create each class
	int i;
	for (i=0; i < pImporter->GetNumObjects(); i++)
	{
		// Allocate the class
		TemplateClass *pClass=new TemplateClass;		

		// Get the class name and parent type
		CString sName=pImporter->GetObjectName(i);
		CString sParentType=pImporter->GetObjectType(i);

		SAFE_STRCPY(pClass->m_ClassName, sName);
		SAFE_STRCPY(pClass->m_ParentClassName, sParentType);
		m_TemplateClasses.Append(pClass);
	}
}


void CEditProjectMgr::UnloadBinaries()
{
	DWORD i, j;
	ClassDef *pClass;

	DeleteAndClearArray(m_TemplateClasses);
	DeleteAndClearArray(m_Classes);

	if(m_ClassDefs)
	{
		for(i=0; i < m_nClassDefs; i++)
		{
			pClass = &m_ClassDefs[i];

			for(j=0; j < pClass->m_nProps; j++)
			{
				free(pClass->m_Props[j].m_PropName);
				free(pClass->m_Props[j].m_DefaultValueString);

				// {BL 11/07/99}
				// Deallocate all edit string lists
				DEditInternal* pDEditInternal = pClass->m_Props[j].m_pDEditInternal;
				if ( pDEditInternal && pDEditInternal->m_aszStrings && (pDEditInternal->m_cStrings > 0) )
				{
					for ( int iString = 0 ; iString < pDEditInternal->m_cStrings ; iString++ )
					{
						delete pDEditInternal->m_aszStrings[iString];
						pDEditInternal->m_aszStrings[iString] = NULL;
					}

					delete [] pDEditInternal->m_aszStrings;
					pDEditInternal->m_aszStrings = NULL;
					pDEditInternal->m_cStrings = 0;
				}
				delete pDEditInternal;
			}
		
			free(pClass->m_ClassName);

			// {BP 2/11/98}
			// Only deallocate if there were props allocated...
			if( pClass->m_Props )
				free(pClass->m_Props);
		}

		free(m_ClassDefs);
	
		m_ClassDefs = NULL;
		m_nClassDefs = 0;
	}


	// [4/26/00 KLS] make sure we free this bad boy...
	if (m_hModule)
	{
		cb_UnloadModule(m_hModule);
		m_hModule = NULL;
	}
}


void CEditProjectMgr::MakeClassTree()
{
	int i, j;
	uint32 iClass;
	ClassDef *pClass, *pHeir;
	CMoArray<CProjectClass*> *pArray;
	CProjectClass *pFound, *pProjectClass;
	ClassDef *heirarchy[512];
	int nInHeirarchy;


	if(!m_ClassDefs)
		return;
	
	
	for(i=0; i < m_nClassDefs; i++)
	{
		pClass = &m_ClassDefs[i];
		pArray = &m_Classes;

		_ExtractClassHeirarchy(pClass, heirarchy, &nInHeirarchy);
		for(j=0; j < nInHeirarchy; j++)
		{
			pHeir = heirarchy[j];
		
			if( !(pFound = FindClassInArray(pHeir, pArray)) )
			{
				pFound = new CProjectClass;
				
				pFound->m_pClass = pHeir;
				pFound->m_pClass->DEditSetClassHook(pFound);

				pArray->Append(pFound);
			}

			pArray = &pFound->m_Children;
		}
	}

	for(iClass=0; iClass < m_Classes; iClass++)
	{
		pProjectClass = m_Classes[iClass];

		RecurseAndBuildClassDefPropList(pProjectClass);
	}
}


CProjectClass* CEditProjectMgr::FindClassInArray(ClassDef *pClass, CMoArray<CProjectClass*> *pArray)
{
	for( DWORD i=0; i < pArray->GetSize(); i++ )
		if(pArray->Get(i)->m_pClass == pClass)
			return pArray->Get(i);

	return NULL;
}


void CEditProjectMgr::LoadProjectFile( CAbstractIO &file )
{
	DWORD		extra=0;


	if( file.GetLen() == 0 )
	{
	}
	else
	{
		file >> extra >> extra >> extra >> extra;
	}
}


void CEditProjectMgr::SaveProjectFile( CAbstractIO &file )
{
	DWORD		extra=0;

	file << extra << extra << extra << extra;
}


ClassDef* CEditProjectMgr::FindClassDef(const char *pName)
{
	DWORD i;

	for(i=0; i < m_nClassDefs; i++)
	{
		if(strcmp(m_ClassDefs[i].m_ClassName, pName) == 0)
		{
			return &m_ClassDefs[i];
		}
	}

	return NULL;
}


PropDef* CEditProjectMgr::FindPropInList(CMoArray<PropDef*> &propList, char *pPropName, DWORD *pIndex)
{
	DWORD i;

	for(i=0; i < propList.GetSize(); i++)
	{
		if(strcmp(propList[i]->m_PropName, pPropName) == 0)
		{
			if(pIndex)
				*pIndex = i;

			return propList[i];
		}
	}

	return NULL;
}


void CEditProjectMgr::RecurseAndBuildClassDefPropList(
	CProjectClass *pProjectClass)
{
	uint32 i;

	BuildClassDefPropList(
		pProjectClass->m_pClass,
		pProjectClass->m_Props);

	for(i=0; i < pProjectClass->m_Children; i++)
	{
		RecurseAndBuildClassDefPropList(pProjectClass->m_Children[i]);
	}
}


void CEditProjectMgr::BuildClassDefPropList(
	ClassDef *pClass, 
	CMoArray<PropDef*> &propList)
{
	int i, chainLen;
	DWORD j, propIndex, curPropIndex;
	ClassDef *pCurClass;
	CMoArray<ClassDef*> classChain;
	PropDef *pProp, *pAlreadyThere;
	CMoArray<PropDef*> unhiddenProps, hiddenProps;


	propList.Term();
	propList.SetCacheSize(128);
	hiddenProps.SetCacheSize(128);
	unhiddenProps.SetCacheSize(128);


	// Unwind the chain.
	chainLen = 0;
	pCurClass = pClass;
	while(pCurClass)
	{
		pCurClass = pCurClass->m_ParentClass;
		++chainLen;
	}

	classChain.SetSize(chainLen);
	--chainLen;
	pCurClass = pClass;
	while(pCurClass)
	{
		classChain[chainLen] = pCurClass;
		pCurClass = pCurClass->m_ParentClass;
		--chainLen;
	}

	// Now go up the chain and add properties.
	for(i=0; i < classChain; i++)
	{
		pCurClass = classChain[i];

		for(j=0; j < pCurClass->m_nProps; j++)
		{
			pProp = &pCurClass->m_Props[j];

			pAlreadyThere = FindPropInList(propList, pProp->m_PropName, &propIndex);
			if(pAlreadyThere)
			{
				propList.Remove(propIndex);
			}
			
			propList.Append(pProp);
		}
	}

	// Move hidden properties to the end of the list.
	for(i=0; i < propList; i++)
	{
		if((propList[i]->m_PropFlags & PF_HIDDEN) || 
			((propList[i]->m_PropFlags & PF_GROUPMASK) && !(propList[i]->m_PropFlags & PF_GROUPOWNER)))
		{
			hiddenProps.Append(propList[i]);
		}
		else
		{
			unhiddenProps.Append(propList[i]);
		}
	}

	curPropIndex = 0;
	for(i=0; i < unhiddenProps; i++)
		propList[curPropIndex++] = unhiddenProps[i];

	for(i=0; i < hiddenProps; i++)
		propList[curPropIndex++] = hiddenProps[i];
}


CMoArray<PropDef*>* CEditProjectMgr::GetClassDefProps(
	ClassDef *pClass)
{
	CProjectClass *pProjectClass;

	
	if(pProjectClass = pClass->DEditGetClassHook())
	{
		return &pProjectClass->m_Props;
	}
	else
	{
		return NULL;
	}
}


CProjectClass* CEditProjectMgr::FindClassInTree( const char *pName )
{
	return RecurseAndFindClass(m_Classes, pName);
}


CProjectClass* CEditProjectMgr::RecurseAndFindClass( CMoArray<CProjectClass*> &classes, const char *pName )
{
	DWORD				i;
	CProjectClass		*pClass;


	for( i=0; i < classes; i++ )
		if(strcmp(classes[i]->m_pClass->m_ClassName, pName) == 0)
			return classes[i];

	for( i=0; i < classes; i++ )
	{
		pClass = RecurseAndFindClass( classes[i]->m_Children, pName );
		if( pClass )
			return pClass;
	}
	
	return NULL;
}


TemplateClass* CEditProjectMgr::FindTemplateClass(const char *pName)
{
	DWORD i;

	for(i=0; i < m_TemplateClasses; i++)
	{
		if(stricmp(m_TemplateClasses[i]->m_ClassName, pName) == 0)
			return m_TemplateClasses[i];
	}

	return NULL;
}


/************************************************************************/
// Fills sDestBaseClass in with the base class of sClassName.  FALSE is returned if
// sClassName could not be found.
BOOL CEditProjectMgr::GetBaseClassName(CString sClassName, CString &sDestBaseClass)
{	
	// Find this class
	CProjectClass *pClass=FindClassInTree(sClassName);
	if (!pClass)
	{
		return FALSE;
	}
	
	// Get the base class name
	ASSERT(pClass->m_pClass);
	ClassDef *pBaseClass=pClass->m_pClass->m_ParentClass;
	if (pBaseClass && pBaseClass->m_ClassName)
	{
		sDestBaseClass=pBaseClass->m_ClassName;
	}
	else
	{
		sDestBaseClass=sClassName;
	}

	return TRUE;
}

