//------------------------------------------------------------------
//
//	FILE	  : MaterialShaderInfo.cpp
//
//	PURPOSE	  : Reads the compiled effect (.fxo, Direct3D 9 "fx_2_0" binary) of a
//	            material's shader. Only what the material editor needs is decoded:
//	            the effect's top level parameters, their annotations, and the
//	            string table. Techniques are skipped over, shader code is ignored.
//
//	            File layout (little endian):
//	              u32 0xFEFF0901, u32 bodyOffset        (body is at file offset bodyOffset+8)
//	              ... type descriptors, values, strings ...  (addressed by offsets relative to file offset 8)
//	              body:
//	                u32 paramCount, techniqueCount, unused, objectCount
//	                params:   u32 typeOffset, valueOffset, flags, annotationCount,
//	                          then annotationCount x (u32 typeOffset, valueOffset)
//	                techniques: u32 nameOffset, annotationCount, passCount, annotations,
//	                          passes: u32 nameOffset, annotationCount, stateCount, annotations,
//	                                  states: 4 x u32 each
//	                u32 stringCount, resourceCount
//	                strings:  u32 id, size, then size bytes (NUL included) padded to 4
//
//	            Type descriptor (at an offset): u32 type, class, nameOffset, semanticOffset,
//	            elements; then rows, columns (scalar/vector/matrix classes) or
//	            memberCount + member descriptors (struct class). Names are stored as
//	            u32 length + characters. String values are object ids into the string table.
//
//------------------------------------------------------------------

#include "MaterialShaderInfo.h"

#include <stdio.h>
#include <string.h>
#include <map>

namespace
{
	enum { FX_TYPE_INT = 2, FX_TYPE_FLOAT = 3, FX_TYPE_STRING = 4, FX_TYPE_TEXTURE = 5 };
	enum { FX_CLASS_SCALAR = 0, FX_CLASS_VECTOR = 1, FX_CLASS_MATRIX_ROWS = 2, FX_CLASS_MATRIX_COLS = 3,
		   FX_CLASS_OBJECT = 4, FX_CLASS_STRUCT = 5 };

	struct Type
	{
		Type() : type(0), cls(0), elements(0), rows(0), cols(0) {}
		uint32_t type, cls, elements, rows, cols;
		std::string name;
		std::vector<Type> members;
	};

	class Reader
	{
	public:
		Reader(const uint8_t *p, size_t n) : m_p(p), m_n(n), m_ok(true), m_pos(0) {}

		bool	ok() const { return m_ok; }

		// Data offsets are relative to file offset 8.
		bool	Have(size_t fileOff, size_t len) const { return fileOff <= m_n && len <= m_n - fileOff; }

		uint32_t U32At(uint32_t off)
		{
			size_t f = (size_t)off + 8;
			if(!Have(f, 4)) { m_ok = false; return 0; }
			return Load(f);
		}
		float F32At(uint32_t off)
		{
			uint32_t v = U32At(off);
			float f; memcpy(&f, &v, 4); return f;
		}
		uint32_t Next()
		{
			if(!Have(m_pos, 4)) { m_ok = false; return 0; }
			uint32_t v = Load(m_pos); m_pos += 4; return v;
		}
		std::string StringAt(uint32_t off)
		{
			uint32_t n = U32At(off);
			size_t f = (size_t)off + 8 + 4;
			if(!m_ok || n > 65536 || !Have(f, n)) { m_ok = false; return std::string(); }
			std::string s((const char*)m_p + f, n);
			while(!s.empty() && s[s.size()-1] == 0) s.erase(s.size()-1);
			return s;
		}

		bool	ReadType(uint32_t off, Type &t, uint32_t *pEnd, int depth = 0)
		{
			if(depth > 8) { m_ok = false; return false; }
			t.type = U32At(off); t.cls = U32At(off + 4);
			uint32_t nameOff = U32At(off + 8);
			t.elements = U32At(off + 16);
			if(!m_ok) return false;
			if(nameOff) t.name = StringAt(nameOff);
			uint32_t k = off + 20;
			if(t.cls <= FX_CLASS_MATRIX_COLS)
			{
				t.rows = U32At(k); t.cols = U32At(k + 4); k += 8;
			}
			else if(t.cls == FX_CLASS_STRUCT)
			{
				uint32_t n = U32At(k); k += 4;
				if(n > 256) { m_ok = false; return false; }
				for(uint32_t i = 0; i < n && m_ok; i++)
				{
					Type m; uint32_t e = 0;
					if(!ReadType(k, m, &e, depth + 1)) return false;
					t.members.push_back(m); k = e;
				}
			}
			if(pEnd) *pEnd = k;
			return m_ok;
		}

		size_t	m_pos;
		bool	m_ok;

	private:
		uint32_t Load(size_t f) const
		{
			return (uint32_t)m_p[f] | ((uint32_t)m_p[f+1] << 8) | ((uint32_t)m_p[f+2] << 16) | ((uint32_t)m_p[f+3] << 24);
		}
		const uint8_t *m_p;
		size_t m_n;
	};

	struct Ann { std::string name; Type type; uint32_t valueOff; };

	struct RawParam
	{
		Type type;
		uint32_t valueOff;
		std::vector<Ann> anns;
	};

	bool ReadAnn(Reader &r, Ann &a)
	{
		uint32_t t = r.Next();
		a.valueOff = r.Next();
		if(!r.ok()) return false;
		uint32_t e;
		if(!r.ReadType(t, a.type, &e)) return false;
		a.name = a.type.name;
		return true;
	}

	const Ann* FindAnn(const RawParam &p, const char *pName)
	{
		for(size_t i = 0; i < p.anns.size(); i++)
			if(_stricmp(p.anns[i].name.c_str(), pName) == 0) return &p.anns[i];
		return 0;
	}
}

static bool ParseFxo(const uint8_t *pData, size_t nSize, ShaderInfo &out, std::string *pError)
{
	out = ShaderInfo();
	if(nSize < 16 || pData[0] != 0x01 || pData[1] != 0x09 || pData[2] != 0xFF || pData[3] != 0xFE)
	{
		if(pError) *pError = "not a compiled Direct3D 9 effect";
		return false;
	}

	Reader r(pData, nSize);
	uint32_t bodyOff = (uint32_t)pData[4] | ((uint32_t)pData[5] << 8) | ((uint32_t)pData[6] << 16) | ((uint32_t)pData[7] << 24);
	r.m_pos = (size_t)bodyOff + 8;

	uint32_t nParams = r.Next(), nTech = r.Next();
	r.Next();
	r.Next();	// object count
	if(!r.ok() || nParams > 4096 || nTech > 256)
	{
		if(pError) *pError = "effect header is damaged";
		return false;
	}

	std::vector<RawParam> params(nParams);
	for(uint32_t i = 0; i < nParams && r.ok(); i++)
	{
		uint32_t typeOff = r.Next(), valOff = r.Next();
		r.Next();	// flags
		uint32_t nAnn = r.Next();
		if(!r.ok() || nAnn > 256) break;
		uint32_t e;
		if(!r.ReadType(typeOff, params[i].type, &e)) break;
		params[i].valueOff = valOff;
		for(uint32_t a = 0; a < nAnn; a++)
		{
			Ann ann;
			if(!ReadAnn(r, ann)) break;
			params[i].anns.push_back(ann);
		}
	}

	// Techniques: skip.
	for(uint32_t t = 0; t < nTech && r.ok(); t++)
	{
		r.Next();
		uint32_t nAnn = r.Next(), nPass = r.Next();
		if(nAnn > 256 || nPass > 256) { r.m_ok = false; break; }
		for(uint32_t a = 0; a < nAnn && r.ok(); a++) { r.Next(); r.Next(); }
		for(uint32_t p = 0; p < nPass && r.ok(); p++)
		{
			r.Next();
			uint32_t pa = r.Next(), ns = r.Next();
			if(pa > 256 || ns > 4096) { r.m_ok = false; break; }
			for(uint32_t a = 0; a < pa && r.ok(); a++) { r.Next(); r.Next(); }
			for(uint32_t s = 0; s < ns && r.ok(); s++) { r.Next(); r.Next(); r.Next(); r.Next(); }
		}
	}

	// String table.
	std::map<uint32_t, std::string> strings;
	uint32_t nStrings = r.Next();
	r.Next();	// resource count
	if(r.ok() && nStrings < 100000)
	{
		for(uint32_t i = 0; i < nStrings; i++)
		{
			uint32_t id = r.Next(), size = r.Next();
			if(!r.ok() || size > (1u << 20) || !r.Have(r.m_pos, size)) break;
			std::string s((const char*)pData + r.m_pos, size);
			while(!s.empty() && s[s.size()-1] == 0) s.erase(s.size()-1);
			strings[id] = s;
			r.m_pos += (size + 3) & ~3u;
		}
	}

	if(!r.ok() && params.empty())
	{
		if(pError) *pError = "effect data is damaged";
		return false;
	}

	// Anything that failed above just means fewer parameters; use what we have.
	r.m_ok = true;
	auto stringValue = [&](uint32_t valueOff) -> std::string
	{
		uint32_t id = r.U32At(valueOff);
		std::map<uint32_t, std::string>::iterator it = strings.find(id);
		return it == strings.end() ? std::string() : it->second;
	};

	for(size_t i = 0; i < params.size(); i++)
	{
		const RawParam &p = params[i];
		const std::string &name = p.type.name;
		if(name.empty())
			continue;

		if(p.type.type == FX_TYPE_STRING && p.type.cls == FX_CLASS_OBJECT)
		{
			if(_stricmp(name.c_str(), "Description") == 0)			out.description = stringValue(p.valueOff);
			else if(_stricmp(name.c_str(), "DocumentationLink") == 0)	out.docLink = stringValue(p.valueOff);
			continue;
		}

		const Ann *pInst = FindAnn(p, "InstanceParam");
		if(!pInst)
			continue;
		if(r.U32At(pInst->valueOff) == 0)
			continue;

		ShaderParamInfo info;
		info.name = name;
		info.isInstance = true;

		const Ann *pDefault = FindAnn(p, "Default");
		const Ann *pDesc = FindAnn(p, "Description");
		if(pDesc && pDesc->type.type == FX_TYPE_STRING)
			info.description = stringValue(pDesc->valueOff);

		if(p.type.type == FX_TYPE_TEXTURE)
		{
			info.type = MatFile::PT_TEXTURE;
			if(pDefault && pDefault->type.type == FX_TYPE_STRING)
			{
				info.defaultString = stringValue(pDefault->valueOff);
				info.hasDefault = true;
			}
		}
		else if(p.type.type == FX_TYPE_INT && p.type.cls == FX_CLASS_SCALAR)
		{
			info.type = MatFile::PT_UINT;
			info.defaultUInt = r.U32At(p.valueOff);
			info.hasDefault = true;
			if(pDefault && pDefault->type.cls == FX_CLASS_SCALAR)
				info.defaultUInt = (pDefault->type.type == FX_TYPE_FLOAT) ? (uint32_t)r.F32At(pDefault->valueOff) : r.U32At(pDefault->valueOff);
		}
		else if(p.type.type == FX_TYPE_FLOAT && p.type.cls == FX_CLASS_SCALAR)
		{
			info.type = MatFile::PT_FLOAT;
			info.defaultVec[0] = r.F32At(p.valueOff);
			info.hasDefault = true;
			if(pDefault && pDefault->type.cls == FX_CLASS_SCALAR)
				info.defaultVec[0] = (pDefault->type.type == FX_TYPE_FLOAT) ? r.F32At(pDefault->valueOff) : (float)(int)r.U32At(pDefault->valueOff);
		}
		else if(p.type.type == FX_TYPE_FLOAT && p.type.cls == FX_CLASS_VECTOR && (p.type.cols == 3 || p.type.cols == 4))
		{
			info.type = (p.type.cols == 3) ? MatFile::PT_VECTOR3 : MatFile::PT_VECTOR4;
			for(uint32_t k = 0; k < p.type.cols; k++)
				info.defaultVec[k] = r.F32At(p.valueOff + 4 * k);
			info.hasDefault = true;
			if(pDefault && pDefault->type.cls == FX_CLASS_VECTOR && pDefault->type.type == FX_TYPE_FLOAT)
				for(uint32_t k = 0; k < pDefault->type.cols && k < 4; k++)
					info.defaultVec[k] = r.F32At(pDefault->valueOff + 4 * k);
		}
		else
			continue;	// matrices, arrays, samplers: not editable per material

		out.params.push_back(info);
	}

	return true;
}

bool ShaderInfo_Parse(const void *pData, size_t nSize, ShaderInfo &out, std::string *pError)
{
	return ParseFxo((const uint8_t*)pData, nSize, out, pError);
}

bool ShaderInfo_Load(const char *pResourceRoot, const char *pShaderPath, ShaderInfo &out, std::string *pError)
{
	out = ShaderInfo();
	if(!pShaderPath || !pShaderPath[0])
	{
		if(pError) *pError = "the material names no shader";
		return false;
	}

	std::string rel = pShaderPath;
	size_t dot = rel.find_last_of('.');
	size_t slash = rel.find_last_of("\\/");
	if(dot != std::string::npos && (slash == std::string::npos || dot > slash))
		rel = rel.substr(0, dot);
	rel += ".fxo";
	while(!rel.empty() && (rel[0] == '\\' || rel[0] == '/')) rel.erase(0, 1);

	std::string full = pResourceRoot ? pResourceRoot : "";
	if(!full.empty()) full += "\\";
	full += rel;

	FILE *f = fopen(full.c_str(), "rb");
	if(!f)
	{
		if(pError) *pError = "compiled shader not found: " + full;
		return false;
	}
	std::vector<uint8_t> buf;
	uint8_t tmp[8192];
	size_t got;
	while((got = fread(tmp, 1, sizeof(tmp), f)) > 0)
	{
		buf.insert(buf.end(), tmp, tmp + got);
		if(buf.size() > (64u << 20)) break;
	}
	fclose(f);
	if(buf.empty())
	{
		if(pError) *pError = "compiled shader is empty: " + full;
		return false;
	}
	return ParseFxo(&buf[0], buf.size(), out, pError);
}
