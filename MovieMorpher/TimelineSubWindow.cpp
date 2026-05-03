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

	TimelineTrack*  track = new TimelineTrack(5,-40 - 10, m_iWidth-10, m_iHeight/3.0);
	track->SetAlignment(HALIGN_LEFT, VALIGN_TOP);
	liGUI_Elements.push_back(track);

	track = new TimelineTrack(5,-80 -10 -5, m_iWidth-10, m_iHeight/3.0);
	track->SetAlignment(HALIGN_LEFT, VALIGN_TOP);
	liGUI_Elements.push_back(track);

	track = new TimelineTrack(5,-120 -10 -10, m_iWidth-10, m_iHeight/3.0);
	track->SetAlignment(HALIGN_LEFT, VALIGN_TOP);
	liGUI_Elements.push_back(track);

}

TimelineSubWindow::~TimelineSubWindow()
{
	for (auto* iterElement : liGUI_Elements)
		delete iterElement;
}


void TimelineSubWindow::Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight)
{
	OpenGLSubWindowWithGUI::Reshape(iBottomLeftX, iBottomLeftY, iWidth, iHeight);

	for (auto* iterElement : liGUI_Elements)
	{
		if (auto* track = dynamic_cast<TimelineTrack*>(iterElement))
			track->Resize(iWidth-10, m_iHeight/3.0);
	}
	
}


void TimelineSubWindow::Draw()
{
	// propagate common values
	//track->matrSliderNonInverted = matrSliderNonInverted;

	{
		glColor3f(1,0,0);

		glLineWidth(1);
		glLine( 0, 0,
			    0, 20,     5);
	}

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
	matrSliderNonInverted = Mat4MakeIdent();

	static float f = 1000;
	videoSlider = new VideoSlider(1,0, 0, 4000, f, 20);
	videoSlider->SetAlignment(HALIGN_LEFT, VALIGN_CENTER);
	liGUI_Elements.push_back(videoSlider);

}

TimelineSliderSubWindow::~TimelineSliderSubWindow()
{
	for (auto* iterElement : liGUI_Elements)
		delete iterElement;
}


void TimelineSliderSubWindow::Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight)
{
	OpenGLSubWindowWithGUI::Reshape(iBottomLeftX, iBottomLeftY, iWidth, iHeight);

	videoSlider->Resize(iWidth-2, iHeight);
}


void TimelineSliderSubWindow::Draw()
{
	// propagate common values
	videoSlider->matrSliderNonInverted = matrSliderNonInverted;

	for (auto iterElement : liGUI_Elements)
		iterElement->Draw();

	printf("Val=%d\n", videoSlider->GetValue());
}


////////////////////////////////////////////////////////////////////////////////////////////////


TimelineScrollBarSubWindow::TimelineScrollBarSubWindow(int iParentWidth, int iParentHeight,
													   float fBottomLeftXperc, float fBottomLeftYperc,
													   float fWidthPerc, float fHeightPerc) :
						 OpenGLSubWindowWithGUI(iParentWidth, iParentHeight,
												fBottomLeftXperc, fBottomLeftYperc, fWidthPerc, fHeightPerc)
{
	scrollBar = new HorScrollBar("", 1,1, int(iParentWidth*fWidthPerc)-2, 15);
	scrollBar->SetAlignment(HALIGN_LEFT, VALIGN_BOTTOM);
	liGUI_Elements.push_back(scrollBar);

}

TimelineScrollBarSubWindow::~TimelineScrollBarSubWindow()
{
	std::vector<GUI_Element*>::iterator iterElement;
	for (iterElement = liGUI_Elements.begin(); iterElement != liGUI_Elements.end(); iterElement++)
		delete (*iterElement);
}


void TimelineScrollBarSubWindow::Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight)
{
	OpenGLSubWindowWithGUI::Reshape(iBottomLeftX, iBottomLeftY, iWidth, iHeight);

	scrollBar->Resize(iWidth-2, iHeight);

}


