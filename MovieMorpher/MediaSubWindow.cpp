#include "stdafx.h"
#include "MediaSubWindow.h"
#include "../../!!adGlobals/adOpenGLUtilities.h"
#include "../../!!adGUI/fps.h"
#include "GLSL_Pipeline.h"
#include "../../!!adExtensions/extensions.h"


extern GLSL_Pipeline glsl_pipeline;
extern TextureBank texBank;

MediaSubWindow::MediaSubWindow(int iParentWidth, int iParentHeight,
							   float fBottomLeftXperc, float fBottomLeftYperc, float fWidthPerc, float fHeightPerc) :
	            OpenGLSubWindowWithGUI(iParentWidth, iParentHeight,
				         			   fBottomLeftXperc, fBottomLeftYperc, fWidthPerc, fHeightPerc)
{
	listBox = new ListBox("Resources", -400,-40, 200, 5, 6.3);
	listBox->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	listBox->items.push_back("First");
	listBox->items.push_back("Second1234567890123456789");
	listBox->items.push_back("Third");
	listBox->items.push_back("Fourth");
	listBox->items.push_back("Fifth");
	listBox->items.push_back("Sixth");
	listBox->items.push_back("Seven");
	listBox->items.push_back("Eight");
	liGUI_Elements.push_back(listBox);

}

MediaSubWindow::~MediaSubWindow()
{
	std::vector<GUI_Element*>::iterator iterElement;
	for (iterElement = liGUI_Elements.begin(); iterElement != liGUI_Elements.end(); iterElement++)
		delete (*iterElement);
}


void MediaSubWindow::Render()
{
	OpenGLSubWindow::Render();

	glDisable(GL_LIGHTING);

	RenderGUI();
}

void MediaSubWindow::PassiveMotionFunc(int x, int y)
{
	OpenGLSubWindow::PassiveMotionFunc(x, y);

	if ((x > m_iBottomLeftX) && (x < m_iBottomLeftX + m_iWidth) &&
		(y > m_iBottomLeftY) && (y < m_iBottomLeftY + m_iHeight))
	{
		bool bResult;
		bResult = PassiveMotionFuncGUI(x, y);

		if (!bResult)
			glutSetCursor(GLUT_CURSOR_INHERIT);
	}
}

void MediaSubWindow::MouseFunc(int button, int state, int x, int y)
{
	OpenGLSubWindow::MouseFunc(button, state, x, y);

	if ((x > m_iBottomLeftX) && (x < m_iBottomLeftX + m_iWidth) &&
		(y > m_iBottomLeftY) && (y < m_iBottomLeftY + m_iHeight))
	{
		if (MouseFuncGUI(button, state, x, y)) return;
	}
}


void MediaSubWindow::MotionFunc(int x, int y)
{
	OpenGLSubWindow::MotionFunc(x, y);

	MotionFuncGUI(x, y);
}

void MediaSubWindow::MouseWheelFunc(int state, int delta, int x, int y)
{
	OpenGLSubWindow::PassiveMotionFunc(x, y);

	if ((x > m_iBottomLeftX) && (x < m_iBottomLeftX + m_iWidth) &&
		(y > m_iBottomLeftY) && (y < m_iBottomLeftY + m_iHeight))
	{
		MouseWheelFuncGUI(state, delta, x, y);
	}
}


