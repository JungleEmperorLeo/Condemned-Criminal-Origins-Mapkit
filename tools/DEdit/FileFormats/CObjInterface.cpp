// CObjInterface.cpp : implementation file
//

#include "bdefs.h"
#include "objectimporter.h"
#include "dedit.h"
#include "regiondoc.h"
#include "regionview.h"
#include "regionframe.h"
#include "usefuldc.h"
#include "editpoly.h"
#include "stringdlg.h"
#include "edithelpers.h"
#include "addobjectdlg.h"
#include "propertiesdlg.h"
#include "worldinfodlg.h"
#include "resource.h"
#include "classdlg.h"
#include "mainfrm.h"
#include "maptexturecoordsdlg.h"
#include "projectbar.h"
#include "geomroutines.h"
#include "rotateselection.h"
#include "texture.h"
#include "edit_actions.h"
#include "de_world.h"
#include "resource.h"
#include "node_ops.h"
#include "colorpicker.h"
#include "navigatordlg.h"
#include "navigatorstoredlg.h"
#include "advancedselect.h"
#include "advancedselectdlg.h"
#include "objectbrowserdlg.h"
#include "optionsobjectbrowser.h"
#include "vectoredit.h"
#include "generateuniquenamesdlg.h"
#include "namechangereportdlg.h"
#include "optionsgenerateuniquenames.h"

#include "CObjInterface.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




/************************************************************************/
void CObjInterface::FileGetLine(char* line, int n, FILE* file)
{
	assert(line && n && file);
	if(line && n && file)
	{
		char ch; 
		int i;

		
		for( i=0; (i < n ) && ( feof( file ) == 0 ); i++ )
		{
			ch = fgetc( file );
			line[i] = (char)ch;
			// ch = fgetc( file );
			if(ch == '\r')
			  break;
			else if(ch == '\n')
			  break;
		}
		line[i] = '\0';
	}
}
/************************************************************************/
int CObjInterface::FileSize( FILE* file )
{
	int i = 0;
	assert(file);
	if( file )
	{
		rewind(file);
		while(  feof( file ) == 0 )
		{
			char ch;
			ch = fgetc( file );
			i++;
		}
		rewind(file);
	}
	return i;
}
/************************************************************************/
void CObjInterface::FileCopyToBuffer(FILE* file, char* buffer)
{
	assert(file);
	if( file )
	{
		int i = 0;
		rewind(file);
		while(  feof( file ) == 0 )
		{
		   buffer[i] = fgetc( file );
		   i++;
		}
		rewind(file);
	}
}
/************************************************************************/
void CObjInterface::ImportObj(FILE* file, int flag, int from)
{
	assert(file);
	if( file ) 
	{
		m_flag = flag;
		m_from = from;
		// read in all the verts			token(v)
		ImportObjVertices( file );
 		// read in all the texture coords	token(vt)
		ImportObjTexCoords( file );
		// read in all the normals			token(vn)
//		ImportObjNormals( file );
		// read in all the objects			token(g,f)
		ImportObjObjects( file );
	}
}
/************************************************************************/
#define OBJ_MAX_BUFFER_SIZE 256
void CObjInterface::ImportObjVertices( FILE* file )
{
	assert(file);
	if( file )
	{
		char buffer[OBJ_MAX_BUFFER_SIZE+1];
		rewind(file);
		while(feof( file ) == 0)
		{
			FileGetLine(buffer, OBJ_MAX_BUFFER_SIZE, file);
			if( strlen(buffer) > 2 )
			{
				if( buffer[0] == 'v'  && buffer[1] == ' ' )
				{
					char seps[] = " \t\n\r";	
					char *token = NULL;	
					int   i = 0;
					float v[3];

					token = strtok( buffer, seps );
					if(token != NULL)
					{
						token = strtok( NULL, seps );  // let's eat the leading token
						i = 0;
						while( (token != NULL) && (i < 3) )
						{
							v[i++] = atof(token);
							token = strtok( NULL, seps ); // Get next token	
						}
						switch(m_from)
						{
						case F3DSMAX:
						case FMAYA:
						case FSOFT:
							{
								LTVector vert(v[0],v[2],v[1]); 
								m_vertexList.Append(vert);
							}
							break;
						default:
							{
								LTVector vert(v[0],v[1],v[2]); 
								m_vertexList.Append(vert);
							}
							break;
						}
					}
				}
			}
				
		}
	}
}
/************************************************************************/
void CObjInterface::ImportObjTexCoords( FILE* file )
{
	assert(file);
	if( file )
	{
		char buffer[OBJ_MAX_BUFFER_SIZE+1];
		rewind(file);
		while(feof( file ) == 0)
		{
			FileGetLine(buffer, OBJ_MAX_BUFFER_SIZE, file);
			if( strlen(buffer) > 2 )
			{
				if( buffer[0] == 'v'  && buffer[1] == 't' )
				{
					char seps[] = " \t\n\r";	
					char *token = NULL;	
					int   i = 0;
					float v[3];

					token = strtok( buffer, seps );
					if(token != NULL)
					{
						token = strtok( NULL, seps );  // let's eat the leading token
						i = 0;
						while( (token != NULL) && (i < 3) )
						{
							v[i++] = atof(token);
							token = strtok( NULL, seps ); // Get next token	
						}
						LTVector vert(v[0],v[1],v[2]); 
						m_texCoordList.Append(vert);
					}
				}
			}
				
		}
	}
}
/************************************************************************/
void CObjInterface::ImportObjNormals( FILE* file )
{
	assert(file);
	if( file )
	{
		char buffer[OBJ_MAX_BUFFER_SIZE+1];
		rewind(file);
		while(feof( file ) == 0)
		{
			FileGetLine(buffer, OBJ_MAX_BUFFER_SIZE, file);
			if( strlen(buffer) > 2 )
			{
				if( buffer[0] == 'v'  && buffer[1] == 'n' )
				{
					char seps[] = " \t\n\r";	
					char *token = NULL;	
					int   i = 0;
					float v[3];

					token = strtok( buffer, seps );
					if(token != NULL)
					{
						token = strtok( NULL, seps );  // let's eat the leading token
						i = 0;
						while( (token != NULL) && (i < 3) )
						{
							v[i++] = atof(token);
							token = strtok( NULL, seps ); // Get next token	
						}
						//! gmVector3 vert(v[0],v[1],v[2]); 
						//! m_normalList.push_back(vert);		
					}
				}
			}
				
		}
	}
}
/************************************************************************/
void CObjInterface::ImportObjObjects( FILE* file )
{
	assert(file);
	if( file )
	{
		char buffer[OBJ_MAX_BUFFER_SIZE+1];
		rewind(file);
		while(feof( file ) == 0)
		{
			FileGetLine(buffer, OBJ_MAX_BUFFER_SIZE, file);
			if( strlen(buffer) > 2 )
			{
				if( buffer[0] == 'f'  && buffer[1] == ' ' )
				{
					char seps[] = "f /\t\n\r";	
					char *token = NULL;	
					int   i = 0;
					CMoArray< int > intList;

					token = strtok( buffer, seps );
					if(token != NULL)
					{
						// found a face
						while( (token != NULL) )
						{
							intList.Append(atoi(token));
							token = strtok( NULL, seps ); // Get next token	
						}
						m_faceSetList.Append(intList);
					}
				}
			}
				
		}
	}
}
/************************************************************************/
void CObjInterface::ExportObj(FILE* file, int flag)
{
	assert(file);
	if(file)
	{
		m_flag = flag;
		int i = 0;
		fprintf(file, "\ng default");
		for( i = 0; i < m_vertexList.GetSize(); i++ )
		{
			fprintf(file, "\nv %f %f %f",m_vertexList[i][0],m_vertexList[i][1],m_vertexList[i][2]);
		}
		fprintf(file, "\ng object");
		for( int j = 0; j < m_faceSetList.GetSize(); j++ )
		{
			if(m_faceSetList[j].GetSize())
			{
				fprintf(file, "\nf " );
				if( m_flag == CObjInterface::RAW )
				{
					for( int k = 0; k < m_faceSetList[j].GetSize(); k++ )
					{
						fprintf(file, "%d ",m_faceSetList[j][k]);
					}
				}
				else
				{
					for( int k = m_faceSetList[j].GetSize()-1; k >= 0 ; k-- )
					{
						fprintf(file, "%d ",m_faceSetList[j][k]);
					}
				}
			}
		}	
	}
}




/************************************************************************/
//  New .obj importer
/************************************************************************/
#include <wincodec.h>
#include <algorithm>
#include <list>
#include "d_filemgr.h"
#include "../Materials/MaterialFile.h"
#include "../Materials/MaterialTexture.h"

bool WriteCondemnedDdsBGRA(const char *pPath, const unsigned char *pBGRA, int w, int h);

static std::string ObjLower(const std::string &s)
{
	std::string r = s;
	for(size_t i=0; i < r.size(); i++) r[i] = (char)tolower((unsigned char)r[i]);
	return r;
}

static std::string ObjTrim(const std::string &s)
{
	size_t a = 0, b = s.size();
	while(a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r' || s[a] == '\n')) a++;
	while(b > a && (s[b-1] == ' ' || s[b-1] == '\t' || s[b-1] == '\r' || s[b-1] == '\n')) b--;
	return s.substr(a, b - a);
}

static std::string ObjSlashes(const std::string &s)
{
	std::string r = s;
	for(size_t i=0; i < r.size(); i++) if(r[i] == '/') r[i] = '\\';
	return r;
}

static std::string ObjDirOf(const std::string &path)
{
	size_t p = path.find_last_of("\\/");
	return (p == std::string::npos) ? std::string(".") : path.substr(0, p);
}

static std::string ObjStemOf(const std::string &path)
{
	size_t p = path.find_last_of("\\/");
	std::string f = (p == std::string::npos) ? path : path.substr(p + 1);
	size_t d = f.find_last_of('.');
	return (d == std::string::npos) ? f : f.substr(0, d);
}

static std::string ObjExtOf(const std::string &path)
{
	size_t p = path.find_last_of("\\/");
	std::string f = (p == std::string::npos) ? path : path.substr(p + 1);
	size_t d = f.find_last_of('.');
	return (d == std::string::npos) ? std::string("") : ObjLower(f.substr(d));
}

static bool ObjFileExists(const std::string &p)
{
	DWORD a = GetFileAttributes(p.c_str());
	return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}

static bool ObjReadFile(const char *path, std::string &out)
{
	FILE *f = fopen(path, "rb");
	if(!f)
		return false;
	fseek(f, 0, SEEK_END);
	long n = ftell(f);
	fseek(f, 0, SEEK_SET);
	out.resize(n > 0 ? (size_t)n : 0);
	if(n > 0 && fread(&out[0], 1, (size_t)n, f) != (size_t)n)
	{
		fclose(f);
		return false;
	}
	fclose(f);
	return true;
}

// Lines of a text file; handles \r\n, \n, \r and trailing '\' continuation.
static void ObjSplitLines(const std::string &txt, std::vector<std::string> &lines)
{
	std::string cur;
	for(size_t i=0; i < txt.size(); i++)
	{
		char c = txt[i];
		if(c == '\r' || c == '\n')
		{
			if(c == '\r' && i + 1 < txt.size() && txt[i+1] == '\n') i++;
			if(!cur.empty() && cur[cur.size()-1] == '\\')
			{
				cur.erase(cur.size()-1);
				continue;
			}
			lines.push_back(cur);
			cur.clear();
		}
		else
			cur += c;
	}
	if(!cur.empty()) lines.push_back(cur);
}

static void ObjTokens(const std::string &line, std::vector<std::string> &tok)
{
	size_t i = 0;
	while(i < line.size())
	{
		while(i < line.size() && (line[i] == ' ' || line[i] == '\t')) i++;
		size_t j = i;
		while(j < line.size() && line[j] != ' ' && line[j] != '\t') j++;
		if(j > i) tok.push_back(line.substr(i, j - i));
		i = j;
	}
}

// "1", "1/2", "1/2/3", "1//3" -> v, t (0 based, -1 = none)
static bool ObjParseCorner(const std::string &tk, int nVerts, int nTex, int &v, int &t)
{
	v = t = -1;
	size_t s1 = tk.find('/');
	std::string a = tk.substr(0, s1);
	std::string b;
	if(s1 != std::string::npos)
	{
		size_t s2 = tk.find('/', s1 + 1);
		b = tk.substr(s1 + 1, (s2 == std::string::npos) ? std::string::npos : s2 - s1 - 1);
	}
	if(a.empty())
		return false;
	int iv = atoi(a.c_str());
	if(iv > 0) v = iv - 1; else if(iv < 0) v = nVerts + iv; else return false;
	if(v < 0 || v >= nVerts)
		return false;
	if(!b.empty())
	{
		int it = atoi(b.c_str());
		if(it > 0) t = it - 1; else if(it < 0) t = nTex + it;
		if(t < 0 || t >= nTex) t = -1;
	}
	return true;
}

bool CObjInterface::ParseObjFile(const char* pPath)
{
	std::string txt;
	if(!ObjReadFile(pPath, txt))
		return false;

	m_vertexList.SetSize(0);
	m_texCoordList.SetSize(0);
	m_faces.clear();
	m_matNames.clear();
	m_groupNames.clear();
	m_mtlDiffuse.clear();

	std::vector<std::string> lines;
	ObjSplitLines(txt, lines);

	std::map<std::string,int> matIndex, groupIndex;
	int curMat = -1, curGroup = 0;
	m_groupNames.push_back("default");
	groupIndex["default"] = 0;
	std::string objDir = ObjDirOf(pPath);
	int nV = 0, nT = 0;

	for(size_t li=0; li < lines.size(); li++)
	{
		std::string ln = ObjTrim(lines[li]);
		if(ln.empty() || ln[0] == '#')
			continue;
		std::vector<std::string> tk;
		ObjTokens(ln, tk);
		if(tk.empty())
			continue;
		const std::string &c = tk[0];

		if(c == "v" && tk.size() >= 4)
		{
			float v[3] = { (float)atof(tk[1].c_str()), (float)atof(tk[2].c_str()), (float)atof(tk[3].c_str()) };
			switch(m_from)
			{
				case FBLENDER:
					m_vertexList.Append(LTVector(v[0], v[1], -v[2]));	// Y up, right handed -> left handed
					break;
				case F3DSMAX:
				case FMAYA:
				case FSOFT:
					m_vertexList.Append(LTVector(v[0], v[2], v[1]));
					break;
				default:
					m_vertexList.Append(LTVector(v[0], v[1], v[2]));
					break;
			}
			nV++;
		}
		else if(c == "vt" && tk.size() >= 3)
		{
			m_texCoordList.Append(LTVector((float)atof(tk[1].c_str()), (float)atof(tk[2].c_str()), 0.0f));
			nT++;
		}
		else if(c == "f" && tk.size() >= 4)
		{
			ObjFace f;
			f.mat = curMat;
			f.group = curGroup;
			bool bAllT = true;
			for(size_t k=1; k < tk.size(); k++)
			{
				int v, t;
				if(!ObjParseCorner(tk[k], nV, nT, v, t))
					continue;
				f.v.push_back(v);
				f.t.push_back(t);
				if(t < 0) bAllT = false;
			}
			if(!bAllT)
				f.t.clear();
			if(f.v.size() >= 3)
				m_faces.push_back(f);
		}
		else if(c == "g" || c == "o")
		{
			std::string name = (tk.size() > 1) ? ObjTrim(ln.substr(ln.find(tk[1]))) : std::string("default");
			std::map<std::string,int>::iterator it = groupIndex.find(name);
			if(it == groupIndex.end())
			{
				curGroup = (int)m_groupNames.size();
				m_groupNames.push_back(name);
				groupIndex[name] = curGroup;
			}
			else
				curGroup = it->second;
		}
		else if(c == "usemtl")
		{
			std::string name = (tk.size() > 1) ? ObjTrim(ln.substr(ln.find(tk[1]))) : std::string("");
			if(name.empty())
				curMat = -1;
			else
			{
				std::map<std::string,int>::iterator it = matIndex.find(name);
				if(it == matIndex.end())
				{
					curMat = (int)m_matNames.size();
					m_matNames.push_back(name);
					matIndex[name] = curMat;
				}
				else
					curMat = it->second;
			}
		}
		else if(c == "mtllib" && tk.size() > 1)
		{
			std::string fn = ObjTrim(ln.substr(ln.find(tk[1])));
			fn = ObjSlashes(fn);
			std::string full = objDir + "\\" + fn;
			if(!ObjFileExists(full))
				full = objDir + "\\" + fn.substr(fn.find_last_of('\\') == std::string::npos ? 0 : fn.find_last_of('\\') + 1);
			std::string mtl;
			if(ObjFileExists(full) && ObjReadFile(full.c_str(), mtl))
			{
				std::vector<std::string> ml;
				ObjSplitLines(mtl, ml);
				std::string cur;
				for(size_t mi=0; mi < ml.size(); mi++)
				{
					std::string m = ObjTrim(ml[mi]);
					if(m.empty() || m[0] == '#') continue;
					std::vector<std::string> mt;
					ObjTokens(m, mt);
					if(mt.size() < 2) continue;
					if(mt[0] == "newmtl")
						cur = ObjTrim(m.substr(m.find(mt[1])));
					else if(!cur.empty() && (mt[0] == "map_Kd" || mt[0] == "map_kd"))
					{
						// drop options (-s 1 1 1, -o ..., -clamp on ...)
						std::string path;
						for(size_t k=1; k < mt.size(); k++)
						{
							if(mt[k][0] == '-')
							{
								// skip the option's numeric / on|off arguments
								while(k + 1 < mt.size() && (isdigit((unsigned char)mt[k+1][0]) || mt[k+1][0] == '.' || mt[k+1][0] == '-' && mt[k+1].size() > 1 && isdigit((unsigned char)mt[k+1][1]) || mt[k+1] == "on" || mt[k+1] == "off")) k++;
								continue;
							}
							path += (path.empty() ? "" : " ") + mt[k];
						}
						if(!path.empty() && m_mtlDiffuse.find(cur) == m_mtlDiffuse.end())
							m_mtlDiffuse[cur] = path;
					}
				}
			}
		}
	}
	return m_faces.size() > 0 && nV > 0;
}

// ---- finding files ----------------------------------------------------

static bool ObjIsImageExt(const std::string &e)
{
	return e == ".dds" || e == ".png" || e == ".tga" || e == ".jpg" || e == ".jpeg" || e == ".bmp" || e == ".tif" || e == ".tiff";
}

static void ObjCollect(const std::string &dir, int depth, std::vector<std::string> &out)
{
	WIN32_FIND_DATA fd;
	HANDLE h = FindFirstFile((dir + "\\*").c_str(), &fd);
	if(h == INVALID_HANDLE_VALUE)
		return;
	do
	{
		std::string n = fd.cFileName;
		if(n == "." || n == "..")
			continue;
		if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if(depth > 0)
				ObjCollect(dir + "\\" + n, depth - 1, out);
		}
		else
		{
			std::string e = ObjExtOf(n);
			if(ObjIsImageExt(e) || e == ".mat00")
				out.push_back(dir + "\\" + n);
		}
	} while(FindNextFile(h, &fd));
	FindClose(h);
}

// Priority: .Mat00, .dds, .png, .tga, .jpg, .bmp
static int ObjExtRank(const std::string &e)
{
	if(e == ".mat00") return 0;
	if(e == ".dds") return 1;
	if(e == ".png") return 2;
	if(e == ".tga") return 3;
	if(e == ".jpg" || e == ".jpeg") return 4;
	if(e == ".bmp") return 5;
	return 6;
}

static bool ObjIsNormalOrAuxName(const std::string &stemLower)
{
	static const char *aux[] = { "_n", "_normal", "_nrm", "_norm", "_s", "_spec", "_specular", "_e", "_emissive", "_ao", "_rough", "_roughness", "_metal", "_metallic", "_height", "_bump", "_opacity", "_mask" };
	for(size_t i=0; i < sizeof(aux)/sizeof(aux[0]); i++)
	{
		size_t n = strlen(aux[i]);
		if(stemLower.size() > n && stemLower.compare(stemLower.size() - n, n, aux[i]) == 0)
			return true;
	}
	return false;
}

// Best file in 'files' for a material name: stem == name, or name + a diffuse suffix.
static std::string ObjFindByName(const std::vector<std::string> &files, const std::string &matName)
{
	static const char *suf[] = { "", "_d", "_diffuse", "_diff", "_albedo", "_basecolor", "_base_color", "_color", "_col", "_c", "_dif" };
	std::string m = ObjLower(matName);
	std::string best;
	int bestScore = 1000;
	for(size_t i=0; i < files.size(); i++)
	{
		std::string stem = ObjLower(ObjStemOf(files[i]));
		int rank = ObjExtRank(ObjExtOf(files[i]));
		for(size_t s=0; s < sizeof(suf)/sizeof(suf[0]); s++)
		{
			if(stem == m + suf[s])
			{
				int score = (int)s * 10 + rank;
				if(score < bestScore) { bestScore = score; best = files[i]; }
			}
		}
	}
	return best;
}

static std::string ObjFindByBaseName(const std::vector<std::string> &files, const std::string &baseName)
{
	std::string b = ObjLower(baseName);
	std::string best;
	int bestRank = 1000;
	for(size_t i=0; i < files.size(); i++)
	{
		std::string f = files[i];
		size_t p = f.find_last_of("\\/");
		std::string n = ObjLower(p == std::string::npos ? f : f.substr(p + 1));
		if(n == b)
			return files[i];
		// same name, other extension (the .mtl points at a .tga, the folder has a .png)
		if(ObjStemOf(n) == ObjStemOf(b))
		{
			int r = ObjExtRank(ObjExtOf(files[i]));
			if(r < bestRank) { bestRank = r; best = files[i]; }
		}
	}
	return best;
}

// ---- image loading ----------------------------------------------------

static bool ObjLoadTga(const std::string &path, std::vector<unsigned char> &bgra, int &w, int &h)
{
	std::string d;
	if(!ObjReadFile(path.c_str(), d) || d.size() < 18)
		return false;
	const unsigned char *b = (const unsigned char*)&d[0];
	int idLen = b[0], cmType = b[1], type = b[2];
	int cmLen = b[5] | (b[6] << 8), cmBits = b[7];
	w = b[12] | (b[13] << 8);
	h = b[14] | (b[15] << 8);
	int bpp = b[16], desc = b[17];
	if(w <= 0 || h <= 0 || (type != 2 && type != 3 && type != 10 && type != 11 && type != 1 && type != 9))
		return false;
	size_t pos = 18 + idLen;
	std::vector<unsigned char> pal;
	if(cmType == 1)
	{
		size_t cmBytes = (size_t)cmLen * ((cmBits + 7) / 8);
		if(pos + cmBytes > d.size()) return false;
		pal.assign(b + pos, b + pos + cmBytes);
		pos += cmBytes;
	}
	int bytes = bpp / 8;
	if(bytes < 1 || bytes > 4)
		return false;
	std::vector<unsigned char> raw((size_t)w * h * bytes);
	bool rle = (type >= 9);
	size_t o = 0;
	if(!rle)
	{
		if(pos + raw.size() > d.size()) return false;
		memcpy(&raw[0], b + pos, raw.size());
	}
	else
	{
		while(o < raw.size() && pos < d.size())
		{
			unsigned char hd = b[pos++];
			int n = (hd & 0x7f) + 1;
			if(hd & 0x80)
			{
				if(pos + bytes > d.size()) return false;
				for(int i=0; i < n && o < raw.size(); i++) { memcpy(&raw[o], b + pos, bytes); o += bytes; }
				pos += bytes;
			}
			else
			{
				for(int i=0; i < n && o < raw.size(); i++)
				{
					if(pos + bytes > d.size()) return false;
					memcpy(&raw[o], b + pos, bytes); o += bytes; pos += bytes;
				}
			}
		}
	}
	bgra.assign((size_t)w * h * 4, 255);
	for(int y=0; y < h; y++)
	{
		int sy = (desc & 0x20) ? y : (h - 1 - y);
		for(int x=0; x < w; x++)
		{
			const unsigned char *s = &raw[((size_t)sy * w + x) * bytes];
			unsigned char *p = &bgra[((size_t)y * w + x) * 4];
			if(type == 1 || type == 9)
			{
				size_t idx = s[0];
				int eb = (cmBits + 7) / 8;
				if(idx * eb + 2 < pal.size()) { p[0] = pal[idx*eb]; p[1] = pal[idx*eb+1]; p[2] = pal[idx*eb+2]; }
			}
			else if(type == 3 || type == 11) { p[0] = p[1] = p[2] = s[0]; }
			else if(bytes == 2)
			{
				unsigned v = s[0] | (s[1] << 8);
				p[0] = (unsigned char)(((v & 31) * 255) / 31); p[1] = (unsigned char)((((v >> 5) & 31) * 255) / 31); p[2] = (unsigned char)((((v >> 10) & 31) * 255) / 31);
			}
			else { p[0] = s[0]; p[1] = s[1]; p[2] = s[2]; }
		}
	}
	return true;
}

static bool ObjLoadWic(const std::string &path, std::vector<unsigned char> &bgra, int &w, int &h)
{
	static const GUID kClsidFactory = { 0xcacaf262, 0x9370, 0x4615, { 0xa1, 0x3b, 0x9f, 0x55, 0x39, 0xda, 0x4c, 0x0a } };
	static const GUID kPixBGRA = { 0x6fddc324, 0x4e03, 0x4bfe, { 0xb1, 0x85, 0x3d, 0x77, 0x76, 0x8d, 0xc9, 0x0f } };
	HRESULT hrInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	bool ok = false;
	IWICImagingFactory *pF = NULL;
	if(SUCCEEDED(CoCreateInstance(kClsidFactory, NULL, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), (void**)&pF)) && pF)
	{
		WCHAR wp[MAX_PATH * 2];
		MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, wp, MAX_PATH * 2);
		IWICBitmapDecoder *pD = NULL;
		if(SUCCEEDED(pF->CreateDecoderFromFilename(wp, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pD)) && pD)
		{
			IWICBitmapFrameDecode *pFr = NULL;
			if(SUCCEEDED(pD->GetFrame(0, &pFr)) && pFr)
			{
				IWICFormatConverter *pC = NULL;
				if(SUCCEEDED(pF->CreateFormatConverter(&pC)) && pC)
				{
					if(SUCCEEDED(pC->Initialize(pFr, kPixBGRA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom)))
					{
						UINT uw = 0, uh = 0;
						pC->GetSize(&uw, &uh);
						if(uw > 0 && uh > 0 && uw < 16384 && uh < 16384)
						{
							w = (int)uw; h = (int)uh;
							bgra.resize((size_t)uw * uh * 4);
							ok = SUCCEEDED(pC->CopyPixels(NULL, uw * 4, (UINT)bgra.size(), &bgra[0]));
						}
					}
					pC->Release();
				}
				pFr->Release();
			}
			pD->Release();
		}
		pF->Release();
	}
	if(SUCCEEDED(hrInit))
		CoUninitialize();
	return ok;
}

static int ObjPow2Near(int v)
{
	int p = 4;
	while(p < v && p < 2048) p <<= 1;
	if(p > 4 && (p - v) > (v - p / 2) && p / 2 >= 4) p /= 2;
	return p;
}

static void ObjResizeBGRA(std::vector<unsigned char> &px, int w, int h, int nw, int nh)
{
	std::vector<unsigned char> out((size_t)nw * nh * 4);
	for(int y=0; y < nh; y++)
	{
		float fy = ((float)y + 0.5f) * h / nh - 0.5f;
		int y0 = (int)floor(fy); float ay = fy - y0;
		int y1 = y0 + 1;
		if(y0 < 0) y0 = 0; if(y1 < 0) y1 = 0; if(y0 > h-1) y0 = h-1; if(y1 > h-1) y1 = h-1;
		for(int x=0; x < nw; x++)
		{
			float fx = ((float)x + 0.5f) * w / nw - 0.5f;
			int x0 = (int)floor(fx); float ax = fx - x0;
			int x1 = x0 + 1;
			if(x0 < 0) x0 = 0; if(x1 < 0) x1 = 0; if(x0 > w-1) x0 = w-1; if(x1 > w-1) x1 = w-1;
			for(int c=0; c < 4; c++)
			{
				float v = (px[((size_t)y0*w+x0)*4+c] * (1-ax) + px[((size_t)y0*w+x1)*4+c] * ax) * (1-ay)
					+ (px[((size_t)y1*w+x0)*4+c] * (1-ax) + px[((size_t)y1*w+x1)*4+c] * ax) * ay;
				out[((size_t)y*nw+x)*4+c] = (unsigned char)(v + 0.5f);
			}
		}
	}
	px.swap(out);
}

static std::string ObjSanitize(const std::string &s)
{
	std::string r;
	for(size_t i=0; i < s.size(); i++)
	{
		char c = s[i];
		r += (isalnum((unsigned char)c) || c == '_' || c == '-') ? c : '_';
	}
	return r.empty() ? std::string("tex") : r;
}

static std::string ObjRoot()
{
	std::string r = dfm_GetBaseDir(GetFileMgr());
	while(!r.empty() && (r[r.size()-1] == '\\' || r[r.size()-1] == '/')) r.erase(r.size()-1);
	return r;
}

static bool ObjUnderRoot(const std::string &full, const std::string &root, std::string &rel)
{
	char a[MAX_PATH], b[MAX_PATH];
	if(!_fullpath(a, full.c_str(), MAX_PATH) || !_fullpath(b, root.c_str(), MAX_PATH))
		return false;
	std::string fa = ObjLower(ObjSlashes(a)), fb = ObjLower(ObjSlashes(b));
	if(fb.empty() || fa.size() <= fb.size() + 1 || fa.compare(0, fb.size(), fb) != 0 || fa[fb.size()] != '\\')
		return false;
	rel = ObjSlashes(a).substr(fb.size() + 1);
	return true;
}

// Brings one image into the project as <root>\<objStem>\<name>.dds + .Mat00. Returns the .Mat00 path
// relative to the project ("" if the image couldn't be converted).
static std::string ObjImportImage(const std::string &src, const std::string &objStem, std::string *pWhy)
{
	std::string root = ObjRoot();
	std::string dirName = "importedtextures";	// textures and materials both live here
	std::string dir = root + "\\" + dirName;
	CreateDirectory(dir.c_str(), NULL);
	std::string stem = ObjSanitize(ObjStemOf(src));
	std::string ddsRel = dirName + "\\" + stem + ".dds";
	std::string matRel = dirName + "\\" + stem + ".Mat00";
	std::string ddsFull = root + "\\" + ddsRel;
	std::string matFull = root + "\\" + matRel;

	bool bHave = false;
	{
		WIN32_FILE_ATTRIBUTE_DATA a, b;
		if(GetFileAttributesEx(ddsFull.c_str(), GetFileExInfoStandard, &a) && GetFileAttributesEx(src.c_str(), GetFileExInfoStandard, &b))
			bHave = CompareFileTime(&a.ftLastWriteTime, &b.ftLastWriteTime) >= 0;
	}
	if(!bHave)
	{
		std::string ext = ObjExtOf(src);
		if(ext == ".dds")
		{
			std::string d;
			if(!ObjReadFile(src.c_str(), d) || d.size() < 128) { if(pWhy) *pWhy = "can't read " + src; return ""; }
			std::string out;
			if(memcmp(d.data(), "TEXR", 4) == 0)
				out = d;
			else if(memcmp(d.data(), "DDS ", 4) == 0)
			{
				unsigned char hdr[128];
				memcpy(hdr, d.data(), 128);
				if(memcmp(hdr + 84, "DXT1", 4) && memcmp(hdr + 84, "DXT3", 4) && memcmp(hdr + 84, "DXT5", 4))
				{
					if(pWhy) *pWhy = ".dds format not supported (use DXT1/DXT5 or a png/tga): " + src;
					return "";
				}
				uint32 *u = (uint32*)hdr;
				u[2] |= 0x20000;
				u[27] = 0x401008;
				u[28] = 0;
				static const unsigned char pre[12] = {0x54,0x45,0x58,0x52, 1,0,0,0, 0,1,2,0};
				out.assign((const char*)pre, 12);
				out.append((const char*)hdr, 128);
				out.append(d.data() + 128, d.size() - 128);
			}
			else { if(pWhy) *pWhy = "not a .dds: " + src; return ""; }
			FILE *fo = fopen(ddsFull.c_str(), "wb");
			if(!fo) { if(pWhy) *pWhy = "can't write " + ddsFull; return ""; }
			fwrite(out.data(), 1, out.size(), fo);
			fclose(fo);
		}
		else
		{
			std::vector<unsigned char> px;
			int w = 0, h = 0;
			bool ok = (ext == ".tga") ? ObjLoadTga(src, px, w, h) : ObjLoadWic(src, px, w, h);
			if(!ok || w < 1 || h < 1) { if(pWhy) *pWhy = "can't read image " + src; return ""; }
			int nw = ObjPow2Near(w), nh = ObjPow2Near(h);
			if(nw != w || nh != h)
				ObjResizeBGRA(px, w, h, nw, nh);
			if(!WriteCondemnedDdsBGRA(ddsFull.c_str(), &px[0], nw, nh)) { if(pWhy) *pWhy = "can't write " + ddsFull; return ""; }
		}
	}

	if(!ObjFileExists(matFull))
	{
		MatFile::Material mat;
		mat.shader = "shaders\\rigid\\solid\\diffuse.fx";
		mat.SetUInt("SurfaceFlags", 0);
		mat.SetFloat("DefaultWidth", 100.0f);
		mat.SetFloat("DefaultHeight", 100.0f);
		mat.SetTexture("tDiffuseMap", ddsRel.c_str());
		mat.SetTexture("tEmissiveMap", "global\\materials\\specular_black.dds");
		mat.SetTexture("tNormalMap", "global\\materials\\flat_normal_map.dds");
		if(!MatFile::SaveFile(matFull.c_str(), mat)) { if(pWhy) *pWhy = "can't write " + matFull; return ""; }
	}
	return matRel;
}

void CObjInterface::ResolveMaterials(const char* pObjPath)
{
	std::string objPath = pObjPath;
	std::string objDir = ObjDirOf(objPath);
	std::string objStem = ObjStemOf(objPath);
	std::string root = ObjRoot();

	std::vector<std::string> files;
	ObjCollect(objDir, 4, files);
	// textures are often in a sibling folder ("..\textures")
	std::string parent = ObjDirOf(objDir);
	std::vector<std::string> parentFiles;
	if(parent != objDir && parent != ".")
		ObjCollect(parent, 2, parentFiles);

	m_matRel.assign(m_matNames.size(), std::string());
	m_missing.clear();

	for(size_t i=0; i < m_matNames.size(); i++)
	{
		const std::string &name = m_matNames[i];
		std::string found;

		// 1. what the .mtl says
		std::map<std::string,std::string>::iterator it = m_mtlDiffuse.find(name);
		if(it != m_mtlDiffuse.end())
		{
			std::string p = ObjSlashes(it->second);
			std::string cand = (p.size() > 1 && p[1] == ':') ? p : objDir + "\\" + p;
			if(ObjFileExists(cand))
				found = cand;
			else
			{
				std::string base = p.substr(p.find_last_of('\\') == std::string::npos ? 0 : p.find_last_of('\\') + 1);
				found = ObjFindByBaseName(files, base);
				if(found.empty()) found = ObjFindByBaseName(parentFiles, base);
			}
		}
		// 2. a file named like the material
		if(found.empty()) found = ObjFindByName(files, name);
		if(found.empty()) found = ObjFindByName(parentFiles, name);
		// 3. one material and one picture next to the model: that's the one
		if(found.empty() && m_matNames.size() == 1)
		{
			found = ObjFindByName(files, objStem);
			if(found.empty())
			{
				std::string only;
				int n = 0;
				for(size_t k=0; k < files.size(); k++)
				{
					std::string e = ObjExtOf(files[k]);
					if(e != ".mat00" && !ObjIsNormalOrAuxName(ObjLower(ObjStemOf(files[k])))) { only = files[k]; n++; }
				}
				if(n == 1) found = only;
			}
		}

		if(found.empty())
		{
			m_missing.push_back(name);
			continue;
		}

		std::string why;
		if(ObjExtOf(found) == ".mat00")
		{
			std::string rel;
			if(ObjUnderRoot(found, root, rel))
				m_matRel[i] = rel;
			else
			{
				// a material outside the project: use a picture of the same name instead
				std::string alt;
				std::vector<std::string> imgs;
				for(size_t k=0; k < files.size(); k++) if(ObjExtOf(files[k]) != ".mat00") imgs.push_back(files[k]);
				alt = ObjFindByName(imgs, ObjStemOf(found));
				if(alt.empty()) alt = ObjFindByName(imgs, name);
				if(!alt.empty())
					m_matRel[i] = ObjImportImage(alt, objStem, &why);
			}
		}
		else
			m_matRel[i] = ObjImportImage(found, objStem, &why);

		if(m_matRel[i].empty())
			m_missing.push_back(name + (why.empty() ? "" : " (" + why + ")"));
	}
}
