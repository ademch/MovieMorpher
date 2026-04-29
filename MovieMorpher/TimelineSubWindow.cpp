#include "stdafx.h"
#include "TimelineSubWindow.h"
#include "../../!!adGlobals/adOpenGLUtilities.h"
#include "../../!!adGUI/fps.h"
#include "GLSL_Pipeline.h"
#include "../../!!adGlobals/glut/glut.h"
#include "../../!!adExtensions/extensions.h"


extern GLSL_Pipeline glsl_pipeline;
extern TextureBank texBank;

TimelineSubWindow::TimelineSubWindow(int iParentWidth, int iParentHeight,
									 float fBottomLeftXperc, float fBottomLeftYperc,
									 float fWidthPerc, float fHeightPerc) :
				   OpenGLSubWindowWithGUI(iParentWidth, iParentHeight,
										  fBottomLeftXperc, fBottomLeftYperc, fWidthPerc, fHeightPerc)
{
	scrollBar = new HorScrollBar("", 1,1, int(iParentWidth*fWidthPerc)-2, 14);
	scrollBar->SetAlignment(HALIGN_LEFT, VALIGN_BOTTOM);
	liGUI_Elements.push_back(scrollBar);
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

void TimelineSubWindow::Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight)
{
	OpenGLSubWindowWithGUI::Reshape(iBottomLeftX, iBottomLeftY, iWidth, iHeight);

	scrollBar->Resize(iWidth-2, iHeight);

}


// Passive motion is special, global window cares about all windows
// to make sure focus, cursor is updated correcly. We do not check for boundaries
bool TimelineSubWindow::PassiveMotionFunc(int x, int y)
{
	OpenGLSubWindow::PassiveMotionFunc(x, y);

	return PassiveMotionFuncGUI(x, y);
}

void TimelineSubWindow::MouseFunc(int button, int state, int x, int y)
{
	OpenGLSubWindow::MouseFunc(button, state, x, y);

	if ((x > m_iBottomLeftX) && (x < m_iBottomLeftX + m_iWidth) &&
		(y > m_iBottomLeftY) && (y < m_iBottomLeftY + m_iHeight))
	{
		if (MouseFuncGUI(button, state, x, y)) return;
	}
}


void TimelineSubWindow::MotionFunc(int x, int y)
{
	OpenGLSubWindow::MotionFunc(x, y);

	MotionFuncGUI(x, y);
}

void TimelineSubWindow::MouseWheelFunc(int state, int delta, int x, int y)
{
	OpenGLSubWindow::PassiveMotionFunc(x, y);

	if ((x > m_iBottomLeftX) && (x < m_iBottomLeftX + m_iWidth) &&
		(y > m_iBottomLeftY) && (y < m_iBottomLeftY + m_iHeight))
	{
		MouseWheelFuncGUI(state, delta, x, y);
	}
}


