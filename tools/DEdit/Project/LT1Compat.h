//------------------------------------------------------------------
//
//	FILE	  : LT1Compat.h
//
//	PURPOSE	  : Lets DEdit load object modules (object.lto) built for
//	            LithTech 1.0 (Blood 2 / Shogo) as well as Jupiter.
//
//	            Both engines report SERVEROBJ_VERSION 1, but their ClassDef
//	            and PropDef structures are laid out differently, so the
//	            version number can't be used to tell them apart. This file
//	            defines the LT 1.0 layouts, detects which layout a loaded
//	            module uses, and converts an LT 1.0 class table into the
//	            Jupiter-layout structures the rest of DEdit works with.
//
//------------------------------------------------------------------

#ifndef __LT1COMPAT_H__
#define __LT1COMPAT_H__

	#ifndef __LTSERVEROBJ_H__
	#include "ltserverobj.h"
	#endif

	// Which ClassDef/PropDef layout an object module uses.
	enum EObjectModuleLayout
	{
		OBJLAYOUT_UNKNOWN = 0,
		OBJLAYOUT_JUPITER,
		OBJLAYOUT_LT1
	};


	// LithTech 1.0 (serverobj_de.h) layouts. Member names intentionally match
	// the Jupiter structures so the validation code can be shared.
	// The module is a 32 bit binary, so DEdit must be built as 32 bit as well.
	struct LT1PropDef
	{
		char			*m_PropName;
		short			m_PropType;
		float			m_DefaultValueVector[3];
		float			m_DefaultValueFloat;
		char			*m_DefaultValueString;
		short			m_PropFlags;			// 16 bit in LT 1.0 (Jupiter: 32 bit)
		void			*m_pInternal;
	};

	struct LT1ClassDef
	{
		char			*m_ClassName;
		LT1ClassDef		*m_ParentClass;
		unsigned long	m_ClassFlags;
		void			*m_ConstructFn;
		void			*m_DestructFn;
		void			*m_EngineMessageFn;		// Jupiter has m_PluginFn in this area
		void			*m_ObjectMessageFn;
		short			m_nProps;
		LT1PropDef		*m_Props;
		long			m_ClassObjectSize;
		void			*m_pInternal[2];
	};


	// Result of converting an LT 1.0 class table. Everything the converted
	// ClassDefs point at (except strings and function pointers, which still
	// live in the loaded module) is owned by this struct, so the module must
	// stay loaded until LT1Compat_Free is called.
	struct LT1ConvertedTable
	{
		ClassDef	**m_ppClasses;
		ClassDef	*m_pClassStorage;
		PropDef		*m_pPropStorage;
		int			m_nClasses;
	};


	// Inspects a raw class table returned by an object module and decides
	// which layout it uses. Tries both layouts and keeps the one whose
	// contents are self-consistent.
	EObjectModuleLayout LT1Compat_DetectLayout(void **ppClassDefs, int nClassDefs);

	// Converts an LT 1.0 class table to Jupiter-layout ClassDefs.
	// Returns false (and leaves pOut empty) on failure.
	bool LT1Compat_Convert(void **ppLT1ClassDefs, int nClassDefs, LT1ConvertedTable *pOut);

	void LT1Compat_Free(LT1ConvertedTable *pTable);

#endif  // __LT1COMPAT_H__
