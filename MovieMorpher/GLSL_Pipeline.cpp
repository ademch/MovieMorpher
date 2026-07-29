
#include "stdafx.h"
#include "GLSL_Pipeline.h"
#include <assert.h>

// Raw string literal
//char FSsource[] =
//#include "../Debug/Shaders/Morph.frag"
//;

char FSsource[] = "..\\Debug\\Shaders\\Morph.frag";

// Raw string literal
//char VSsource[] =
//#include "../Debug/Shaders/Morph.vert"
//;

char VSsource[] = "..\\Debug\\Shaders\\Morph.vert";

// Raw string literal
//char GSsource[] =
//#include "../Debug/Shaders/Morph.geom"
//;

char GSsource[] = "..\\Debug\\Shaders\\Morph.geom";


bool GLSL_Pipeline::init_shaders()
{
bool bSuccess;

	//--Shader--
	bSuccess = CreateShader("morph", &VSsource[0], &FSsource[0], &GSsource[0]);
	if (!bSuccess) return false;

	// get uniform locations
	GPUPrograms["morph"]->Uniforms["tex0"]         = glGetUniformLocationARBassert(GPUPrograms["morph"]->programObj, "tex0");
	GPUPrograms["morph"]->Uniforms["tex1"]         = glGetUniformLocationARBassert(GPUPrograms["morph"]->programObj, "tex1");
	GPUPrograms["morph"]->Uniforms["fMorphRadius"] = glGetUniformLocationARBassert(GPUPrograms["morph"]->programObj, "fMorphRadius");
	GPUPrograms["morph"]->Uniforms["fMorphPower"]  = glGetUniformLocationARBassert(GPUPrograms["morph"]->programObj, "fMorphPower");
	GPUPrograms["morph"]->Uniforms["fMorphRatio"]  = glGetUniformLocationARBassert(GPUPrograms["morph"]->programObj, "fMorphRatio");
	GPUPrograms["morph"]->Uniforms["fShadow"]	   = glGetUniformLocationARBassert(GPUPrograms["morph"]->programObj, "fShadow");

	return true;
}


