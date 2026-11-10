// CObjInterface.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRegionView view


#ifndef __COBJINTERFACE_H__
#define __COBJINTERFACE_H__


	#include "editgrid.h"
	#include "editpoly.h"
	#include "editray.h"
	#include "navigator.h"
	#include "editbrush.h"
	#include "refs.h"
	#include "viewrender.h"
	#include "trackers\trackermgr.h"
	#include <vector>
	#include <string>
	#include <map>

	// One face of the .obj as read from the file (0 based indices, -1 = none).
	struct ObjFace
	{
		std::vector<int>	v;		// position indices
		std::vector<int>	t;		// texture coordinate indices (same size as v, or empty)
		int					mat;	// index into m_matNames (-1 = none)
		int					group;	// index into m_groupNames
	};

	class CEditRegion;
	class CEditPoly;
	/************************************************************************/
	class CObjInterface
	{
	public:
		CObjInterface(void)					
		{ 
			m_vertexList.SetCacheSize(4096);
			m_texCoordList.SetCacheSize(4096);
			m_faceSetList.SetCacheSize(4096);
												
			m_flag = RAW; 
			m_from = F3DSMAX;
		}
		~CObjInterface(void)				{ ; }
	public:
		void	ImportObj(FILE* file, int flag = CObjInterface::RAW, int from = F3DSMAX);
		void	ExportObj(FILE* file, int flag = CObjInterface::RAW);
		int		FileSize( FILE* file );
		void	FileCopyToBuffer(FILE* file, char* buffer);
		void	FileGetLine(char* line, int n, FILE* file);

		// New importer: reads positions, UVs, faces (v, v/t, v/t/n, v//n, negative indices, any
		// number of corners), groups/objects and materials (usemtl / mtllib).
		bool	ParseObjFile(const char* pPath);
		// Finds the texture of every material next to the .obj (using the .mtl, or by name),
		// imports it into the project as a Condemned .dds + .Mat00 and fills m_matRel.
		void	ResolveMaterials(const char* pObjPath);
	public:
		enum
		{
			RAW,
			REVERSE,
			F3DSMAX,	// from 3DSMax
			FMAYA,		// from Maya
			FSOFT,		// from SOFT
			FBLENDER,	// from Blender (Y up, -Z forward)
		};

		CMoArray< LTVector >			m_vertexList;
		CMoArray< LTVector >			m_texCoordList;
		CMoArray< CMoArray< int > >		m_faceSetList;
		int								m_flag;
		int								m_from;

		std::vector<ObjFace>			m_faces;
		std::vector<std::string>		m_matNames;
		std::vector<std::string>		m_matRel;		// project relative .Mat00 per material ("" = not found)
		std::vector<std::string>		m_groupNames;
		std::map<std::string,std::string> m_mtlDiffuse;	// material name -> map_Kd path from the .mtl
		std::vector<std::string>		m_missing;		// materials without a texture

	private:
		void	ImportObjVertices( FILE* file );
		void	ImportObjTexCoords( FILE* file );
		void	ImportObjNormals( FILE* file );
		void	ImportObjObjects( FILE* file );
	};
	
#endif  // __COBJINTERFACE_H__




