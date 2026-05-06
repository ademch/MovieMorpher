#include "stdafx.h"
#include "TimelineSubWindow.h"
#include "../../!!adGlobals/adOpenGLUtilities.h"
#include "../../!!adGUI/fps.h"
#include "GLSL_Pipeline.h"
#include "../../!!adGlobals/glut/glut.h"
#include "../../!!adExtensions/extensions.h"


const int iTrackHeight  = 40;
const int iTrackPadding = 10;
const int iTrackCount   = 5;

TimelineSubWindow::TimelineSubWindow(int iParentWidth, int iParentHeight,
									 float fBottomLeftXperc, float fBottomLeftYperc,
									 float fWidthPerc, float fHeightPerc,
									 PositionMediator* mediator) :
				   OpenGLSubWindowWithGUI(iParentWidth, iParentHeight,
										  fBottomLeftXperc, fBottomLeftYperc, fWidthPerc, fHeightPerc)
{
	matrSliderNonInverted = Mat4MakeIdent();

	m_fSliderX = 0;

	//OnChange = [this, mediator](float fVal)
	//{
	//	mediator->SetPos(this, fVal);
	//};
	mediator->subscribeForPos([this](void* origin, float fVal)
	{
		if (origin != this) SetPos(fVal);
	});
	//mediator->subscribeForPosInit([this](void* origin, float fVal, unsigned int _iStartSec, unsigned int _iEndSec)
	//{
	//	if (origin != videoSlider) SetPosInit(fVal, _iStartSec, _iEndSec);
	//});

	for (int16_t iTrack = 1; iTrack <= iTrackCount; iTrack++)
	{
		TimelineTrack*  track = new TimelineTrack(5, -iTrackHeight*iTrack - iTrackPadding*iTrack, m_iWidth-10, iTrackHeight);
		track->SetAlignment(HALIGN_LEFT, VALIGN_TOP);
		liGUI_Elements.push_back(track);
	}

	iVerticalPan = 0.0;
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


bool TimelineSubWindow::MouseWheelFunc(int state, int delta, int x, int y)
{
	OpenGLSubWindowWithGUI::MouseWheelFunc(state, delta, x, y);

	if ((x > m_iBottomLeftX) && (x < m_iBottomLeftX + m_iWidth) &&
		(y > m_iBottomLeftY) && (y < m_iBottomLeftY + m_iHeight))
	{


		if (delta < 0)
		{
			if (m_iHeight + iVerticalPan < (iTrackHeight + iTrackPadding)*iTrackCount + 10)
				iVerticalPan += 10;
		}
		else
		{
			if (iVerticalPan > 0)
				iVerticalPan -= 10;
		}

		vUserSceneTranslation = Vecc3(0.0, float(iVerticalPan));

		return true;
	}

	return false;

}


void TimelineSubWindow::Draw()
{

	{
		glColor3f(1,0,0);

		glLineWidth(1);
		glLine( m_fSliderX, - m_iHeight/2,
			    m_fSliderX,   m_iHeight/2, 5);
	}

	for (auto iterElement : liGUI_Elements)
	{
		// propagate common values
		if (auto* track = dynamic_cast<TimelineTrack*>(iterElement))
			track->matrSliderNonInverted = Mat4MakeTrans(0, -vUserSceneTranslation.Y, 0)*matrSliderNonInverted; // minus to get nonInverted vUserSceneTranslation
		
		iterElement->Draw();
	}
}

void TimelineSubWindow::SetPos(float _val)
{
	m_fSliderX = _val*m_iWidth - m_iWidth/2.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////

TimelineSliderSubWindow::TimelineSliderSubWindow(int iParentWidth, int iParentHeight,
												 float fBottomLeftXperc, float fBottomLeftYperc,
												 float fWidthPerc, float fHeightPerc,
												 PositionMediator* mediator) :
						 OpenGLSubWindowWithGUI(iParentWidth, iParentHeight,
												fBottomLeftXperc, fBottomLeftYperc, fWidthPerc, fHeightPerc)
{
	matrSliderNonInverted = Mat4MakeIdent();

	static float f = 1000;
	videoSlider = new VideoSlider(1,0, 20, mediator);
	videoSlider->SetAlignment(HALIGN_LEFT, VALIGN_CENTER);
	videoSlider->OnChange = [this, mediator](float fVal)
	{
		mediator->SetPos(videoSlider, fVal);
	};
	mediator->subscribeForPos([this](void* origin, float fVal)
	{
		if (origin != videoSlider) videoSlider->SetPos(fVal);
	});
	mediator->subscribeForPosInit([this](void* origin, float fVal, unsigned int _iStartSec, unsigned int _iEndSec)
	{
		if (origin != videoSlider) videoSlider->SetPosInit(fVal, _iStartSec, _iEndSec);
	});
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

	printf("Val=%f\n", videoSlider->GetValue());
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


