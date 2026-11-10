#pragma once
#include "RenderShadowSelection.h"
#include "ShaderDeclarationClosure.h"
// Source Mat00 metadata -> shader eligibility/cache identity -> final vertex
// declaration. There are no material names or declarations specific to a map.
namespace MaterialRenderLayout {
struct Layout {
 RenderShadowSelection::ShaderKey shader;unsigned int identity,count,warnings;
 const ShaderRenderDeclaration::Element* declaration;long compilerResult;
};
inline bool Resolve(ShaderDeclarationClosure::Cache& cache,const MaterialResources::Material& material,Layout* output){
 if(!output||!material.shader)return false;
 output->shader.name=0;output->shader.state=RenderShadowSelection::Unresolved;output->identity=0xffffffffu;output->count=output->warnings=0;output->declaration=0;output->compilerResult=0;
 if(!cache.Resolve(material.shader,&output->identity))return false;
 const ShaderDeclarationClosure::Record* record=cache.Get(output->identity);if(!record)return false;output->compilerResult=record->primary.result;
 // The original packed writer falls back to the Working declaration only for
 // a NULL Shader*/declaration pointer. Failed load gives NULL Shader*; an
 // END-only successfully loaded declaration stays nonnull and is not replaced.
 if(record->state==ShaderDeclarationClosure::Failed){output->shader.state=RenderShadowSelection::Failed;output->declaration=RenderVertexPacking::Working;output->count=10;return true;}
 if(record->state!=ShaderDeclarationClosure::Ready)return false;
 output->shader.name=record->key;output->shader.state=RenderShadowSelection::Loaded;output->count=record->count;output->warnings=record->warnings;output->declaration=record->elements;return true;
}
}
