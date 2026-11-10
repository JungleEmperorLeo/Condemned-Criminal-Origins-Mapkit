//------------------------------------------------------------------
//
//	FILE	  : MaterialFile.h
//
//	PURPOSE	  : Reads and writes Jupiter EX material instance files
//	            (.Mat00, used by F.E.A.R. and Condemned: Criminal Origins).
//
//	            A material instance is a shader path plus a list of named,
//	            typed parameters. The file layout (little endian):
//
//	              char[4]  "LTMI"
//	              u32      version            (1)
//	              u16 len, char[len]          shader path, e.g. shaders\rigid\solid\specular.fx
//	              u32      parameter count
//	              per parameter:
//	                u32      type             1 = texture path, 2 = float[3], 3 = float[4],
//	                                          4 = unsigned int, 5 = float
//	                u16 len, char[len]        parameter name, e.g. tDiffuseMap
//	                value                     type 1: u16 len + path
//	                                          type 2: 3 x f32 (colors, e.g. vTintColor)
//	                                          type 3: 4 x f32 (e.g. vBaseColor, vReflectionPlane)
//	                                          type 4: u32
//	                                          type 5: f32
//
//	            A few shipped files are cut short (the header counts more
//	            parameters than the file holds, usually empty texture slots the
//	            original tool never wrote). Parse() keeps what is there and sets
//	            Material::truncated; saving writes a complete, consistent file.
//
//	            This file has no dependencies beyond the C++ standard library so
//	            it can be built and tested on its own.
//
//------------------------------------------------------------------

#ifndef __MATERIALFILE_H__
#define __MATERIALFILE_H__

#include <string>
#include <vector>
#include <stdint.h>

namespace MatFile
{
	enum ParamType
	{
		PT_TEXTURE	= 1,	// string: path of a .dds file, relative to the resource root
		PT_VECTOR3	= 2,	// float[3]
		PT_VECTOR4	= 3,	// float[4]
		PT_UINT		= 4,	// 32 bit unsigned integer (SurfaceFlags)
		PT_FLOAT	= 5		// 32 bit float
	};

	struct Param
	{
		Param() : type(PT_FLOAT), u(0), f(0.0f) { v[0] = v[1] = v[2] = v[3] = 0.0f; }

		uint32_t	type;
		std::string	name;
		std::string	str;	// PT_TEXTURE
		uint32_t	u;		// PT_UINT
		float		f;		// PT_FLOAT
		float		v[4];	// PT_VECTOR3 (first 3 used), PT_VECTOR4
	};

	struct Material
	{
		Material() : version(1), truncated(false) {}

		uint32_t			version;
		bool				truncated;	// set by Parse() for files that end early
		std::string			shader;
		std::vector<Param>	params;

		// Case-insensitive lookup, NULL if the parameter is not present.
		Param*			Find(const char *pName);
		const Param*	Find(const char *pName) const;

		// Convenience getters. Return the fallback if the parameter is missing or has another type.
		std::string		GetTexture(const char *pName, const char *pDefault = "") const;
		float			GetFloat(const char *pName, float fDefault) const;
		uint32_t		GetUInt(const char *pName, uint32_t nDefault) const;
		// Copies up to 4 floats into pOut. Returns false (pOut untouched) if the parameter is
		// missing or is not a vector.
		bool			GetVector(const char *pName, float *pOut, int *pCount = 0) const;

		// Set a parameter, adding it if it doesn't exist. Order of new parameters is preserved.
		void			SetTexture(const char *pName, const char *pPath);
		void			SetFloat(const char *pName, float f);
		void			SetUInt(const char *pName, uint32_t n);
		void			SetVector3(const char *pName, float x, float y, float z);
		void			SetVector4(const char *pName, float x, float y, float z, float w);

		// Removes a parameter, returns false if it wasn't there.
		bool			Remove(const char *pName);
	};

	// Parse from memory. On failure returns false and (if pError) fills in a description.
	bool	Parse(const void *pData, size_t nSize, Material &out, std::string *pError = 0);

	// Serialize. The output is byte for byte what Parse() read for an unmodified material.
	void	Serialize(const Material &mat, std::vector<uint8_t> &out);

	bool	LoadFile(const char *pFilename, Material &out, std::string *pError = 0);
	bool	SaveFile(const char *pFilename, const Material &mat, std::string *pError = 0);

	// Case-insensitive string compare helper (also used by the dialogs).
	bool	NameEquals(const std::string &a, const char *b);
}

#endif // __MATERIALFILE_H__
