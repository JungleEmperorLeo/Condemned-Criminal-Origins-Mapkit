//------------------------------------------------------------------
//
//	FILE	  : MaterialShaderInfo.h
//
//	PURPOSE	  : What a compiled shader (.fxo) tells the material editor:
//	            the text for the "Shader Information" box, the documentation
//	            link behind the "?" button, and which parameters are per-material
//	            ("InstanceParam") with their descriptions and defaults.
//
//	            The material editor works without it (it then lists only the
//	            parameters stored in the material); when the shader can be read,
//	            the list follows the shader.
//
//------------------------------------------------------------------

#ifndef __MATERIALSHADERINFO_H__
#define __MATERIALSHADERINFO_H__

#include <string>
#include <vector>
#include "MaterialFile.h"

struct ShaderParamInfo
{
	ShaderParamInfo() : type(MatFile::PT_FLOAT), isInstance(true), hasDefault(false)
	{ defaultVec[0] = defaultVec[1] = defaultVec[2] = defaultVec[3] = 0.0f; defaultUInt = 0; }

	std::string		name;
	uint32_t		type;			// MatFile::ParamType
	bool			isInstance;		// editable per material
	std::string		description;
	bool			hasDefault;
	float			defaultVec[4];	// float default in [0]
	uint32_t		defaultUInt;
	std::string		defaultString;	// texture default
};

struct ShaderInfo
{
	std::string					description;	// "Shader Information"
	std::string					docLink;		// e.g. Shaders\Docs\specular\main.htm
	std::vector<ShaderParamInfo> params;
};

// Reads the compiled shader. pShaderPath is the material's shader name
// (shaders\rigid\solid\specular.fx); the compiled file is found by swapping
// the extension for .fxo under pResourceRoot. Returns false if it can't be read.
bool ShaderInfo_Load(const char *pResourceRoot, const char *pShaderPath, ShaderInfo &out, std::string *pError = 0);

// Same, from memory (used by the tests).
bool ShaderInfo_Parse(const void *pData, size_t nSize, ShaderInfo &out, std::string *pError = 0);

#endif // __MATERIALSHADERINFO_H__
