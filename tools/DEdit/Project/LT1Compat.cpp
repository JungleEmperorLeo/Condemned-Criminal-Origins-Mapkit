//------------------------------------------------------------------
//
//	FILE	  : LT1Compat.cpp
//
//	PURPOSE	  : See LT1Compat.h.
//
//------------------------------------------------------------------

#include "bdefs.h"
#include "LT1Compat.h"


#ifndef NUM_PROPERTYTYPES
#define NUM_PROPERTYTYPES 8
#endif

#define LT1COMPAT_MAX_CLASSES		8192
#define LT1COMPAT_MAX_PROPS			2048
#define LT1COMPAT_MAX_NAME_LEN		256


static bool IsReadableString(const char *pStr)
{
	if(!pStr)
		return false;

	if(IsBadStringPtrA(pStr, LT1COMPAT_MAX_NAME_LEN))
		return false;

	// Must be terminated within the limit.
	for(int i=0; i < LT1COMPAT_MAX_NAME_LEN; i++)
	{
		if(pStr[i] == 0)
			return true;
	}

	return false;
}


// Checks that a class table is self consistent when read using the layout of
// CD/PD. Returns the total number of properties found, or -1 if the table
// doesn't make sense under this layout.
template<class CD, class PD>
static int ValidateTable(void **ppClassDefs, int nClassDefs)
{
	int i, j;
	int nTotalProps = 0;

	for(i=0; i < nClassDefs; i++)
	{
		CD *pClass = (CD*)ppClassDefs[i];

		if(!pClass || IsBadReadPtr(pClass, sizeof(CD)))
			return -1;

		if(!IsReadableString(pClass->m_ClassName) || pClass->m_ClassName[0] == 0)
			return -1;

		// The parent must be NULL or another class in this table.
		if(pClass->m_ParentClass)
		{
			bool bFound = false;
			for(j=0; j < nClassDefs; j++)
			{
				if((void*)pClass->m_ParentClass == ppClassDefs[j])
				{
					bFound = true;
					break;
				}
			}

			if(!bFound)
				return -1;
		}

		if(pClass->m_nProps < 0 || pClass->m_nProps > LT1COMPAT_MAX_PROPS)
			return -1;

		if(pClass->m_nProps > 0)
		{
			if(!pClass->m_Props || IsBadReadPtr(pClass->m_Props, sizeof(PD) * pClass->m_nProps))
				return -1;

			for(j=0; j < pClass->m_nProps; j++)
			{
				PD *pProp = &pClass->m_Props[j];

				if(!IsReadableString(pProp->m_PropName))
					return -1;

				if(pProp->m_PropType < 0 || pProp->m_PropType >= NUM_PROPERTYTYPES)
					return -1;
			}

			nTotalProps += pClass->m_nProps;
		}
	}

	return nTotalProps;
}


EObjectModuleLayout LT1Compat_DetectLayout(void **ppClassDefs, int nClassDefs)
{
	if(!ppClassDefs || nClassDefs <= 0 || nClassDefs > LT1COMPAT_MAX_CLASSES)
		return OBJLAYOUT_UNKNOWN;

	if(IsBadReadPtr(ppClassDefs, sizeof(void*) * nClassDefs))
		return OBJLAYOUT_UNKNOWN;

	int nJupiter = ValidateTable<ClassDef, PropDef>(ppClassDefs, nClassDefs);
	int nLT1	 = ValidateTable<LT1ClassDef, LT1PropDef>(ppClassDefs, nClassDefs);

	// LithTech 1.0 is the primary target, so it wins ties.
	//
	// A layout is only accepted if it found at least one property. Reading a
	// table with the wrong layout can pass validation by accident when every
	// misread field happens to be zero (it then looks like a table of classes
	// with no properties), and a real class table always has properties. Without
	// this rule a table that is bad under the LT 1.0 layout could silently load
	// as an empty Jupiter class list instead of being reported as unrecognized.
	if(nLT1 > 0 && nLT1 >= nJupiter)
		return OBJLAYOUT_LT1;

	if(nJupiter > 0)
		return OBJLAYOUT_JUPITER;

	return OBJLAYOUT_UNKNOWN;
}


bool LT1Compat_Convert(void **ppLT1ClassDefs, int nClassDefs, LT1ConvertedTable *pOut)
{
	int i, j, k;
	int nTotalProps;

	memset(pOut, 0, sizeof(*pOut));

	if(!ppLT1ClassDefs || nClassDefs <= 0)
		return false;

	// Count properties so storage can be allocated in one block.
	nTotalProps = 0;
	for(i=0; i < nClassDefs; i++)
		nTotalProps += ((LT1ClassDef*)ppLT1ClassDefs[i])->m_nProps;

	pOut->m_ppClasses		= (ClassDef**)calloc(nClassDefs, sizeof(ClassDef*));
	pOut->m_pClassStorage	= (ClassDef*)calloc(nClassDefs, sizeof(ClassDef));
	pOut->m_pPropStorage	= (PropDef*)calloc(nTotalProps > 0 ? nTotalProps : 1, sizeof(PropDef));
	pOut->m_nClasses		= nClassDefs;

	if(!pOut->m_ppClasses || !pOut->m_pClassStorage || !pOut->m_pPropStorage)
	{
		LT1Compat_Free(pOut);
		return false;
	}

	PropDef *pNextProp = pOut->m_pPropStorage;

	for(i=0; i < nClassDefs; i++)
	{
		LT1ClassDef *pIn = (LT1ClassDef*)ppLT1ClassDefs[i];
		ClassDef *pDest = &pOut->m_pClassStorage[i];

		pOut->m_ppClasses[i] = pDest;

		pDest->m_ClassName			= pIn->m_ClassName;
		pDest->m_ClassFlags			= pIn->m_ClassFlags;			// CF_ bits 0-3 are identical
		pDest->m_ConstructFn		= (ConstructObjectFn)pIn->m_ConstructFn;
		pDest->m_DestructFn			= (DestructObjectFn)pIn->m_DestructFn;
		pDest->m_PluginFn			= NULL;							// LT 1.0 has no object plugins
		pDest->m_nProps				= pIn->m_nProps;
		pDest->m_ClassObjectSize	= pIn->m_ClassObjectSize;
		pDest->m_pInternal[0]		= NULL;
		pDest->m_pInternal[1]		= NULL;

		// Resolve the parent to the converted class.
		pDest->m_ParentClass = NULL;
		if(pIn->m_ParentClass)
		{
			for(k=0; k < nClassDefs; k++)
			{
				if((void*)pIn->m_ParentClass == ppLT1ClassDefs[k])
				{
					pDest->m_ParentClass = &pOut->m_pClassStorage[k];
					break;
				}
			}
		}

		if(pIn->m_nProps > 0)
		{
			pDest->m_Props = pNextProp;

			for(j=0; j < pIn->m_nProps; j++)
			{
				LT1PropDef *pPropIn = &pIn->m_Props[j];
				PropDef *pPropOut = &pNextProp[j];

				pPropOut->m_PropName				= pPropIn->m_PropName;
				pPropOut->m_PropType				= pPropIn->m_PropType;
				pPropOut->m_DefaultValueVector.x	= pPropIn->m_DefaultValueVector[0];
				pPropOut->m_DefaultValueVector.y	= pPropIn->m_DefaultValueVector[1];
				pPropOut->m_DefaultValueVector.z	= pPropIn->m_DefaultValueVector[2];
				pPropOut->m_DefaultValueFloat		= pPropIn->m_DefaultValueFloat;
				pPropOut->m_DefaultValueString		= pPropIn->m_DefaultValueString;

				// 16 bit flags -> 32 bit. Go through unsigned short so the
				// group bits don't get sign extended. PF_ bits 0-11 mean the
				// same thing in both engines.
				pPropOut->m_PropFlags				= (unsigned long)(unsigned short)pPropIn->m_PropFlags;

				pPropOut->m_pInternal				= pPropIn->m_pInternal;
				pPropOut->m_pDEditInternal			= NULL;
			}

			pNextProp += pIn->m_nProps;
		}
		else
		{
			pDest->m_Props = NULL;
		}
	}

	return true;
}


void LT1Compat_Free(LT1ConvertedTable *pTable)
{
	if(!pTable)
		return;

	free(pTable->m_ppClasses);
	free(pTable->m_pClassStorage);
	free(pTable->m_pPropStorage);

	memset(pTable, 0, sizeof(*pTable));
}
