#ifndef MORPHFBOPROCESSORWINDOW_H
#define MORPHFBOPROCESSORWINDOW_H

#include "../../!!adGUI/SubWindow.h"
#include "../../!!adExtensions/Shaders.h"
#include <list>


class MorphFBOprocessor : public OpenGLSubWindow
{
public:
	FrameBufferObject* fbo;

	MorphFBOprocessor(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight, TextureBank& _texBank);
	~MorphFBOprocessor();

	virtual	void Render();
	virtual void Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight);

	void TextureUpdate(int iWidth, int iHeight, unsigned char* image);
	void _TextureUpdate_Test(int iWidth, int iHeight, int nrChannels);

	float fMorphRadius;
	float fMorphPower;
	float fMorphRatio;
	bool  bShowWireframe;

	bool bOutdated;

protected:

	std::list<Vec2> mesh_listEmu;

private:
	TextureDescriptor* AllocFrameTexture(int iWidth, int iHeight, int nrChannels);
	TextureDescriptor* AllocFloatBufferTexture(int iWidth, int iHeight, int nrChannels);

	void GenMeshToListEmu(float fX_bottom, float fY_bottom, float fWidth, float fHeight, float fZ, int iHorSlices, int iVertSlices);
	void RenderFromMeshListEmu(unsigned int tex, float fX_bottom, float fY_bottom, float fWidth, float fHeight, float fZ, int iHorSlices, int iVertSlices);
	void ShaderEmulate();

	TextureBank&  texBank;

};

#endif