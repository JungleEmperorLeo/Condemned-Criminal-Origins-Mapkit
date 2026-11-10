//------------------------------------------------------------------
//
//	FILE	  : MaterialTexture.h
//
//	PURPOSE	  : Lets DEdit treat a Jupiter EX material (.Mat00) as if it were
//	            a texture. DEdit's texture code opens "textures" with
//	            streamsim_Open() and reads them with dtx_Create(). This module
//	            installs a streamsim open hook; when the file being opened is a
//	            .Mat00 it parses the material, decodes its diffuse .dds and
//	            hands back an in-memory DTX of that picture, so the existing
//	            texture display / mapping / apply code works on materials
//	            without being changed.
//
//	            The material's path is what ends up stored on a polygon, which
//	            is what Jupiter EX world files do ( material "path.Mat00" ).
//
//------------------------------------------------------------------

#ifndef __MATERIALTEXTURE_H__
#define __MATERIALTEXTURE_H__

#include <string>
#include "MaterialFile.h"

// Installs the streamsim hook. Safe to call more than once.
void MaterialTexture_Install();

// Folder that material texture paths (tDiffuseMap = "Tex\...") are relative to.
// If never set (or the file isn't found there) the folders above the material
// are searched.
void MaterialTexture_SetResourceRoot(const char *pRoot);

// Size (in world units) one tile of the material covers: its DefaultWidth / DefaultHeight
// (100 x 100 if the material can't be read). Condemned stores texture mapping as tiles per unit,
// i.e. axis / size, so the level loader and saver convert with this.
void MaterialTexture_GetTileSize(const char *pMaterialRelName, float &w, float &h);

// True if the filename ends in .Mat00 (any case).
bool MaterialTexture_IsMaterialName(const char *pFilename);

// Name of the texture parameter used for the preview ("tDiffuseMap", or the first
// fallback the material has). Returns "" if it has no texture at all.
std::string MaterialTexture_PreviewParam(const MatFile::Material &mat);

// Finds the file for a texture path taken from a material. Returns false if it
// can't be located. pMaterialFile is the full path of the .Mat00 it came from.
bool MaterialTexture_ResolveTexture(const char *pMaterialFile, const char *pTexturePath, std::string &fullPath);

#endif // __MATERIALTEXTURE_H__
