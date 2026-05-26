#include "stdafx.h"
#include "GlobalParamsSubWindow.h"
#include "../../!!adGUI/fps.h"
#include "GLSL_Pipeline.h"
#include "../../!!adGlobals/glut/glut.h"


extern GLSL_Pipeline glsl_pipeline;


GlobalParamsSubWindow* GlobalParamsSubWindow::_this = NULL;
GlobalParamsSubWindow* GlobalParamsSubWindow::Get() { return _this; }


GlobalParamsSubWindow::GlobalParamsSubWindow(int iParentWidth, int iParentHeight,
											 float fBottomLeftXperc, float fBottomLeftYperc,
											 float fWidthPerc, float fHeightPerc) :
					   OpenGLSubWindowWithGUI(iParentWidth, iParentHeight,
											  fBottomLeftXperc, fBottomLeftYperc, fWidthPerc, fHeightPerc)
{
	_this = this;

	PopulateGUI();
}


void GlobalParamsSubWindow::PopulateGUI()
{
	onoffswitchWireframe = new OnOffFlipSwitch("Wireframe", 30,40, 6);
	onoffswitchWireframe->SetAlignment(HALIGN_LEFT, VALIGN_CENTER);
	liGUI_Elements.push_back(onoffswitchWireframe);

	OnOffFlipSwitch* onoffswitchShaders = new OnOffFlipSwitch("Shaders", 190,40, 6);
	onoffswitchShaders->SetAlignment(HALIGN_LEFT, VALIGN_CENTER);
	//onoffswitchShaders->bEnabled = false;
	onoffswitchShaders->OnPreClick = [this](bool bON_Request) { return CompileShaders(bON_Request); };
	liGUI_Elements.push_back(onoffswitchShaders);

	onoffswitchShowPoints = new OnOffFlipSwitch("Points", 30,0, 6);
	onoffswitchShowPoints->SetAlignment(HALIGN_LEFT, VALIGN_CENTER);
	onoffswitchShowPoints->bON = true;
	onoffswitchShowPoints->strHint = "Show/Hide control handles (p)";
	liGUI_Elements.push_back(onoffswitchShowPoints);

	onoffpushbuttonOriginal = new OnOffFlipSwitch("Original", 190,0, 6);
	onoffpushbuttonOriginal->SetAlignment(HALIGN_LEFT, VALIGN_CENTER);
	onoffpushbuttonOriginal->bON = false;
	onoffpushbuttonOriginal->bPushButton = true;
	onoffpushbuttonOriginal->strHint = "Show/Hide image before morphing (o)";
	liGUI_Elements.push_back(onoffpushbuttonOriginal);

	FPS* fpsElement = new FPS(188,-50, 6);
	fpsElement->SetAlignment(HALIGN_LEFT, VALIGN_CENTER);
	liGUI_Elements.push_back(fpsElement);

	// Enable shaders
	onoffswitchShaders->SetOnOff(true, true);
}



bool GlobalParamsSubWindow::ShouldShowOriginal()
{
	return onoffpushbuttonOriginal->bON;
}


bool GlobalParamsSubWindow::CompileShaders(bool bON_Request)
{
	if (!glsl_pipeline.bUseShaderPipeline)
	{
		assert(bON_Request);

		printf("Compiling shaders...\n");
			if (!glsl_pipeline.init_shaders()) return false;
		printf("done\n");

		glsl_pipeline.bUseShaderPipeline = true;
	}
	else
	{
		assert(!bON_Request);

		printf("Deleting shaders...\n");
		glsl_pipeline.destroy_shaders();
		printf("done\n");

		glsl_pipeline.bUseShaderPipeline = false;
	}

	return true;
}

bool GlobalParamsSubWindow::PointsAreVisible()
{
	return onoffswitchShowPoints->bON;
}

void GlobalParamsSubWindow::MakePointsVisible()
{
	onoffswitchShowPoints->bON = true;
}

bool GlobalParamsSubWindow::IsWireframeShown()
{
	return onoffswitchWireframe->bON;
}

bool GlobalParamsSubWindow::KeyboardFunc(unsigned char key, int x, int y)
{
	bool res = false;

	switch (key)
	{
	case 'p':
	{
		onoffswitchShowPoints->SetOnOff(!onoffswitchShowPoints->bON, false);
		break;
	}
	}

	return res;
}


void GlobalParamsSubWindow::KeyboardAux(int key, int state, int x, int y)
{
	if (key == 'O')
	{
		if (state == GLUT_DOWN)
			onoffpushbuttonOriginal->SetOnOff(true, false);
		if (state == GLUT_UP)
			onoffpushbuttonOriginal->SetOnOff(false, false);

		return;
	}
}