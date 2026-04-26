#include "stdafx.h"
#include "TimelineSubWindow.h"
#include "../../!!adGlobals/adOpenGLUtilities.h"
#include "../../!!adGUI/fps.h"
#include "GLSL_Pipeline.h"
#include "../../!!adExtensions/extensions.h"


extern GLSL_Pipeline glsl_pipeline;
extern TextureBank texBank;

TimelineSubWindow::TimelineSubWindow(int iParentWidth, int iParentHeight,
								    float fBottomLeftXperc, float fBottomLeftYperc,
								    float fWidthPerc, float fHeightPerc) :
	      OpenGLSubWindowWithGUI(iParentWidth, iParentHeight,
								 fBottomLeftXperc, fBottomLeftYperc, fWidthPerc, fHeightPerc)
{
	fMorphRatio = 90;
	SliderMorphRatio = new Slider<SL_TIME>(" ", 30,10, 0, 100, &fMorphRatio, 7);
	SliderMorphRatio->SetAlignment(HALIGN_LEFT, VALIGN_CENTER);
	SliderMorphRatio->SetBoxWidth(iParentWidth*fWidthPerc*0.8);
	SliderMorphRatio->SetBoxSeparation(10);
	SliderMorphRatio->fValueGranularity = 1;
	SliderMorphRatio->fTickGranularity = 10;
	liGUI_Elements.push_back(SliderMorphRatio);


}

TimelineSubWindow::~TimelineSubWindow()
{
	std::vector<GUI_Element*>::iterator iterElement;
	for (iterElement = liGUI_Elements.begin(); iterElement != liGUI_Elements.end(); iterElement++)
		delete (*iterElement);
}


void TimelineSubWindow::Render()
{
	OpenGLSubWindow::Render();

	glDisable(GL_LIGHTING);

	RenderGUI();
}

void TimelineSubWindow::PassiveMotionFunc(int x, int y)
{
	OpenGLSubWindow::PassiveMotionFunc(x, y);

	if ((x > m_iBottomLeftX) && (x < m_iBottomLeftX + m_iWidth) &&
		(y > m_iBottomLeftY) && (y < m_iBottomLeftY + m_iHeight))
	{
		PassiveMotionFuncGUI(x, y);
	}
}

void TimelineSubWindow::MouseFunc(int button, int state, int x, int y)
{
	OpenGLSubWindow::MouseFunc(button, state, x, y);

	if (MouseFuncGUI(button, state, x, y)) return;
}


void TimelineSubWindow::MotionFunc(int x, int y)
{
	OpenGLSubWindow::MotionFunc(x, y);

	MotionFuncGUI(x, y);
}


