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
	matrSliderNonInverted = Mat4MakeIdent();

	static float f = 1000;
	videoSlider = new VideoSlider("Hello", 1,-22, 0, 4000, f, 20);
	videoSlider->SetAlignment(HALIGN_LEFT, VALIGN_TOP);
	liGUI_Elements.push_back(videoSlider);

}

TimelineSubWindow::~TimelineSubWindow()
{
	std::vector<GUI_Element*>::iterator iterElement;
	for (iterElement = liGUI_Elements.begin(); iterElement != liGUI_Elements.end(); iterElement++)
		delete (*iterElement);
}


void TimelineSubWindow::Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight)
{
	OpenGLSubWindowWithGUI::Reshape(iBottomLeftX, iBottomLeftY, iWidth, iHeight);

	videoSlider->Resize(iWidth-2, iHeight);
}

//void TimelineSubWindow::Draw()
//{
//	OpenGLSubWindowWithGUI::Draw();
//
//	// propagate common values
//	videoSlider->matrSliderNonInverted = matrSliderNonInverted;
//}

void TimelineSubWindow::Draw()
{
	videoSlider->matrSliderNonInverted = matrSliderNonInverted;

	for (auto iterElement : liGUI_Elements)
		iterElement->Draw();
}


////////////////////////////////////////////////////////////////////////////////////////////////


TimelineSliderSubWindow::TimelineSliderSubWindow(int iParentWidth, int iParentHeight,
										         float fBottomLeftXperc, float fBottomLeftYperc,
										         float fWidthPerc, float fHeightPerc) :
						 OpenGLSubWindowWithGUI(iParentWidth, iParentHeight,
												fBottomLeftXperc, fBottomLeftYperc, fWidthPerc, fHeightPerc)
{
	scrollBar = new HorScrollBar("", 1,1, int(iParentWidth*fWidthPerc)-2, 15);
	scrollBar->SetAlignment(HALIGN_LEFT, VALIGN_BOTTOM);
	liGUI_Elements.push_back(scrollBar);

}

TimelineSliderSubWindow::~TimelineSliderSubWindow()
{
	std::vector<GUI_Element*>::iterator iterElement;
	for (iterElement = liGUI_Elements.begin(); iterElement != liGUI_Elements.end(); iterElement++)
		delete (*iterElement);
}


void TimelineSliderSubWindow::Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight)
{
	OpenGLSubWindowWithGUI::Reshape(iBottomLeftX, iBottomLeftY, iWidth, iHeight);

	scrollBar->Resize(iWidth-2, iHeight);

}


