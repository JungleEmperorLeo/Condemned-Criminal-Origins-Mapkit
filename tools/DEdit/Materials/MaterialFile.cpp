//------------------------------------------------------------------
//
//	FILE	  : MaterialFile.cpp
//
//	PURPOSE	  : See MaterialFile.h
//
//------------------------------------------------------------------

#include "MaterialFile.h"

#include <string.h>
#include <stdio.h>

namespace MatFile
{

static const char kMagic[4] = { 'L', 'T', 'M', 'I' };

bool NameEquals(const std::string &a, const char *b)
{
	if(!b)
		return false;
	size_t n = a.size();
	if(strlen(b) != n)
		return false;
	for(size_t i = 0; i < n; i++)
	{
		char x = a[i], y = b[i];
		if(x >= 'A' && x <= 'Z') x = (char)(x - 'A' + 'a');
		if(y >= 'A' && y <= 'Z') y = (char)(y - 'A' + 'a');
		if(x != y)
			return false;
	}
	return true;
}

Param* Material::Find(const char *pName)
{
	for(size_t i = 0; i < params.size(); i++)
		if(NameEquals(params[i].name, pName))
			return &params[i];
	return 0;
}

const Param* Material::Find(const char *pName) const
{
	for(size_t i = 0; i < params.size(); i++)
		if(NameEquals(params[i].name, pName))
			return &params[i];
	return 0;
}

std::string Material::GetTexture(const char *pName, const char *pDefault) const
{
	const Param *p = Find(pName);
	if(p && p->type == PT_TEXTURE)
		return p->str;
	return pDefault ? pDefault : "";
}

float Material::GetFloat(const char *pName, float fDefault) const
{
	const Param *p = Find(pName);
	return (p && p->type == PT_FLOAT) ? p->f : fDefault;
}

uint32_t Material::GetUInt(const char *pName, uint32_t nDefault) const
{
	const Param *p = Find(pName);
	return (p && p->type == PT_UINT) ? p->u : nDefault;
}

bool Material::GetVector(const char *pName, float *pOut, int *pCount) const
{
	const Param *p = Find(pName);
	if(!p || (p->type != PT_VECTOR3 && p->type != PT_VECTOR4))
		return false;
	int n = (p->type == PT_VECTOR3) ? 3 : 4;
	for(int i = 0; i < n; i++)
		pOut[i] = p->v[i];
	if(pCount)
		*pCount = n;
	return true;
}

static Param* FindOrAdd(Material &m, const char *pName, uint32_t type)
{
	Param *p = m.Find(pName);
	if(!p)
	{
		m.params.push_back(Param());
		p = &m.params.back();
		p->name = pName;
	}
	p->type = type;
	return p;
}

void Material::SetTexture(const char *pName, const char *pPath)
{
	Param *p = FindOrAdd(*this, pName, PT_TEXTURE);
	p->str = pPath ? pPath : "";
}

void Material::SetFloat(const char *pName, float f)
{
	FindOrAdd(*this, pName, PT_FLOAT)->f = f;
}

void Material::SetUInt(const char *pName, uint32_t n)
{
	FindOrAdd(*this, pName, PT_UINT)->u = n;
}

void Material::SetVector3(const char *pName, float x, float y, float z)
{
	Param *p = FindOrAdd(*this, pName, PT_VECTOR3);
	p->v[0] = x; p->v[1] = y; p->v[2] = z; p->v[3] = 0.0f;
}

void Material::SetVector4(const char *pName, float x, float y, float z, float w)
{
	Param *p = FindOrAdd(*this, pName, PT_VECTOR4);
	p->v[0] = x; p->v[1] = y; p->v[2] = z; p->v[3] = w;
}

bool Material::Remove(const char *pName)
{
	for(size_t i = 0; i < params.size(); i++)
	{
		if(NameEquals(params[i].name, pName))
		{
			params.erase(params.begin() + i);
			return true;
		}
	}
	return false;
}

//--------------------------------------------------------------------
// Reading
//--------------------------------------------------------------------

namespace
{
	struct Reader
	{
		const uint8_t *p;
		size_t n, pos;
		bool ok;

		Reader(const void *d, size_t s) : p((const uint8_t*)d), n(s), pos(0), ok(true) {}

		bool Have(size_t k) { if(pos + k > n) ok = false; return ok; }

		uint32_t U32()
		{
			if(!Have(4)) return 0;
			uint32_t v = (uint32_t)p[pos] | ((uint32_t)p[pos+1] << 8) | ((uint32_t)p[pos+2] << 16) | ((uint32_t)p[pos+3] << 24);
			pos += 4;
			return v;
		}

		uint32_t U16()
		{
			if(!Have(2)) return 0;
			uint32_t v = (uint32_t)p[pos] | ((uint32_t)p[pos+1] << 8);
			pos += 2;
			return v;
		}

		float F32()
		{
			uint32_t bits = U32();
			float f;
			memcpy(&f, &bits, 4);
			return f;
		}

		std::string Str16()
		{
			uint32_t len = U16();
			if(!Have(len)) return std::string();
			std::string s((const char*)p + pos, len);
			pos += len;
			return s;
		}
	};
}

static bool Fail(std::string *pError, const char *pMsg, uint32_t type = 0, const std::string *pName = 0)
{
	if(pError)
	{
		char buf[256];
		if(pName)
			sprintf(buf, "%s (parameter \"%.100s\", type %u)", pMsg, pName->c_str(), (unsigned)type);
		else
			sprintf(buf, "%s", pMsg);
		*pError = buf;
	}
	return false;
}

bool Parse(const void *pData, size_t nSize, Material &out, std::string *pError)
{
	out = Material();

	if(nSize < 8 || memcmp(pData, kMagic, 4) != 0)
		return Fail(pError, "not a material file (missing LTMI header)");

	Reader r(pData, nSize);
	r.pos = 4;
	out.version = r.U32();
	if(out.version != 1)
		return Fail(pError, "unsupported material file version");

	out.shader = r.Str16();
	uint32_t nParams = r.U32();
	if(!r.ok)
		return Fail(pError, "material file is truncated");
	if(nParams > 4096)
		return Fail(pError, "material file has an implausible parameter count");

	for(uint32_t i = 0; i < nParams; i++)
	{
		// Some shipped files stop early. Keep every complete parameter we have.
		if(r.pos >= nSize)
		{
			out.truncated = true;
			break;
		}

		Param prm;
		prm.type = r.U32();
		prm.name = r.Str16();
		if(!r.ok)
		{
			out.truncated = true;
			break;
		}

		switch(prm.type)
		{
		case PT_TEXTURE:	prm.str = r.Str16();	break;
		case PT_VECTOR3:	prm.v[0] = r.F32(); prm.v[1] = r.F32(); prm.v[2] = r.F32();	break;
		case PT_VECTOR4:	prm.v[0] = r.F32(); prm.v[1] = r.F32(); prm.v[2] = r.F32(); prm.v[3] = r.F32();	break;
		case PT_UINT:		prm.u = r.U32();		break;
		case PT_FLOAT:		prm.f = r.F32();		break;
		default:
			return Fail(pError, "unknown parameter type", prm.type, &prm.name);
		}

		if(!r.ok)
		{
			out.truncated = true;
			break;
		}
		out.params.push_back(prm);
	}

	// Anything left over would be dropped when the file is saved again.
	if(!out.truncated && r.pos != nSize)
		return Fail(pError, "unexpected data after the last parameter");

	return true;
}

//--------------------------------------------------------------------
// Writing
//--------------------------------------------------------------------

static void PutU32(std::vector<uint8_t> &o, uint32_t v)
{
	o.push_back((uint8_t)(v & 0xFF));
	o.push_back((uint8_t)((v >> 8) & 0xFF));
	o.push_back((uint8_t)((v >> 16) & 0xFF));
	o.push_back((uint8_t)((v >> 24) & 0xFF));
}

static void PutF32(std::vector<uint8_t> &o, float f)
{
	uint32_t bits;
	memcpy(&bits, &f, 4);
	PutU32(o, bits);
}

static void PutStr16(std::vector<uint8_t> &o, const std::string &s)
{
	size_t len = s.size() > 0xFFFF ? 0xFFFF : s.size();
	o.push_back((uint8_t)(len & 0xFF));
	o.push_back((uint8_t)((len >> 8) & 0xFF));
	o.insert(o.end(), s.begin(), s.begin() + len);
}

void Serialize(const Material &mat, std::vector<uint8_t> &out)
{
	out.clear();
	out.insert(out.end(), kMagic, kMagic + 4);
	PutU32(out, mat.version);
	PutStr16(out, mat.shader);
	PutU32(out, (uint32_t)mat.params.size());

	for(size_t i = 0; i < mat.params.size(); i++)
	{
		const Param &p = mat.params[i];
		PutU32(out, p.type);
		PutStr16(out, p.name);
		switch(p.type)
		{
		case PT_TEXTURE:	PutStr16(out, p.str);	break;
		case PT_VECTOR3:	PutF32(out, p.v[0]); PutF32(out, p.v[1]); PutF32(out, p.v[2]);	break;
		case PT_VECTOR4:	PutF32(out, p.v[0]); PutF32(out, p.v[1]); PutF32(out, p.v[2]); PutF32(out, p.v[3]);	break;
		case PT_UINT:		PutU32(out, p.u);		break;
		case PT_FLOAT:		PutF32(out, p.f);		break;
		}
	}
}

//--------------------------------------------------------------------
// Files
//--------------------------------------------------------------------

bool LoadFile(const char *pFilename, Material &out, std::string *pError)
{
	FILE *f = fopen(pFilename, "rb");
	if(!f)
		return Fail(pError, "unable to open the material file");

	std::vector<uint8_t> buf;
	uint8_t tmp[4096];
	size_t got;
	while((got = fread(tmp, 1, sizeof(tmp), f)) > 0)
	{
		buf.insert(buf.end(), tmp, tmp + got);
		if(buf.size() > (16u << 20))
			break;
	}
	fclose(f);

	return Parse(buf.empty() ? (const void*)tmp : (const void*)&buf[0], buf.size(), out, pError);
}

bool SaveFile(const char *pFilename, const Material &mat, std::string *pError)
{
	std::vector<uint8_t> buf;
	Serialize(mat, buf);

	FILE *f = fopen(pFilename, "wb");
	if(!f)
		return Fail(pError, "unable to open the material file for writing");

	bool ok = fwrite(&buf[0], 1, buf.size(), f) == buf.size();
	if(fclose(f) != 0)
		ok = false;

	if(!ok)
		return Fail(pError, "failed while writing the material file");
	return true;
}

} // namespace MatFile
