#include "stdafx.h"
#include "TimelineSubWindow.h"
#include "../../!!adGlobals/adOpenGLUtilities.h"
#include "../../!!adGUI/button.h"
#include "../../!!adGUI/fps.h"
#include "GLSL_Pipeline.h"
#include "../../!!adGlobals/glut/glut.h"
#include "../../!!adExtensions/extensions.h"


const int iTrackHeight  = 40;
const int iTrackPadding = 10;
const int const_iTrackCount   = 5;

TimelineSubWindow::TimelineSubWindow(int iParentWidth, int iParentHeight,
									 float fBottomLeftXperc, float fBottomLeftYperc,
									 float fWidthPerc, float fHeightPerc) :
				   OpenGLSubWindowWithGUI(iParentWidth, iParentHeight,
										  fBottomLeftXperc, fBottomLeftYperc, fWidthPerc, fHeightPerc)
{
	bMouseButtonPressed   = false;
	bSelectionIsValid     = false;
	
	matrSliderNonInverted = Mat4MakeIdent();

	m_fSliderX = 0;

	OnSliderPosChange = [this](float fPos)
	{
		PositionMediator::Get()->SetPos(this, fPos);
	};
	OnSelectionChange = [this](float fSelectionStart, float fSelectionEnd)
	{
		PositionMediator::Get()->SetSelection(this, fSelectionStart, fSelectionEnd);
	};
	PositionMediator::Get()->subscribeForPos([this](void* origin, float fVal)
	{
		if (origin != this) SetSliderPos(fVal);
	});
	//mediator->subscribeForPosInit([this](void* origin, float fVal, unsigned int _iStartSec, unsigned int _iEndSec)
	//{
	//	if (origin != videoSlider) SetPosInit(fVal, _iStartSec, _iEndSec);
	//});

	for (int16_t iTrack = 1; iTrack <= const_iTrackCount; iTrack++)
	{
		TimelineTrack*  track = new TimelineTrack(iTrack, 5, -iTrackHeight*iTrack - iTrackPadding*iTrack, m_iWidth-10, iTrackHeight);
		track->SetAlignment(HALIGN_LEFT, VALIGN_TOP);
		liGUI_Elements.push_back(track);
		// shortcut
		//liTracks.push_back(track);
	}

	iVerticalPan = 0.0;
}

void TimelineSubWindow::AddClip(TrackClip* _clip)
{
	int iTrack = TimelineTrack::iSelected;
	TrackClip* clip = new TrackClip(1, 5, -iTrackHeight*iTrack - iTrackPadding*iTrack +10, Width()-10, 20);
	
	//float iDuration = mediator->Duration();
	clip->SetPos(0, 100.0);

	clip->iTrack = iTrack;
	clip->SetAlignment(HALIGN_LEFT, VALIGN_TOP);
	liGUI_Elements.push_back(clip);

	clip->Reposition(-Width()/2 + clip->iHPosShift, Height()/2 + clip->iVPosShift);
}


float TimelineSubWindow::GetSliderValue()
{
	return (m_fSliderX + m_iWidth/2.0) / m_iWidth; 
}

float TimelineSubWindow::GetSelectionStartValue()
{
	return (m_fSelectionStartX + m_iWidth/2.0) / m_iWidth;
}

float TimelineSubWindow::GetSelectionEndValue()
{
	return (m_fSelectionEndX + m_iWidth/2.0) / m_iWidth;
}


void TimelineSubWindow::Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight)
{
	OpenGLSubWindowWithGUI::Reshape(iBottomLeftX, iBottomLeftY, iWidth, iHeight);

	for (auto* iterElement : liGUI_Elements)
	{
		auto* track = dynamic_cast<GUI_ElementResizable*>(iterElement);
		track->Resize(iWidth-10, m_iHeight/3.0);
	}
}

// OnClick
bool TimelineSubWindow::MouseFunc(int button, int state, int x, int y)
{
	bool bResult = OpenGLSubWindowWithGUI::MouseFunc(button, state, x, y);

	if ((x > m_iBottomLeftX) && (x < m_iBottomLeftX + m_iWidth) &&
		(y > m_iBottomLeftY) && (y < m_iBottomLeftY + m_iHeight))
	{
		SetupGraphicsPipelineWithIdentityModelViewMatrix();

		Vec3d v3DCoords;
		gluUnProjectFriendly(x, y, 0, v3DCoords.X, v3DCoords.Y, v3DCoords.Z);

		if (button == GLUT_LEFT_BUTTON)
		{
			if (state == GLUT_DOWN)
			{
				bMouseButtonPressed = true;
				
				// reset selection first
				if (bSelectionIsValid)
				{
					bSelectionIsValid   = false;
					if (OnSelectionChange != NULL) OnSelectionChange( 0.0f, 0.0f );
				}

				iStartDragX = x;
				iStartDragY = y;

				m_fSelectionStartX = (matrSliderNonInverted * Vecc3(v3DCoords.X)).X;

				//if (OnChange != NULL) OnChange(Mat4MakeTrans(vUserSceneTranslation.X, 0, 0)*matrUserScale);

				return true;
			}
		}
	}

	bMouseButtonPressed = false;

	return bResult;
}

// OnDrag
void TimelineSubWindow::MotionFunc(int x, int y)
{
	OpenGLSubWindowWithGUI::MotionFunc(x, y);

	if (bMouseButtonPressed)
	{
		if (!bSelectionIsValid)
			if (PointDist(Vecc2(x,y), Vecc2(iStartDragX, iStartDragY)) < 3) return;

		bSelectionIsValid = true;

		SetupGraphicsPipelineWithIdentityModelViewMatrix();

			Vec3d v3DCoords;
			gluUnProjectFriendly(x, y, 0, v3DCoords.X, v3DCoords.Y, v3DCoords.Z);

			m_fSliderX = m_fSelectionStartX;
			if (OnSliderPosChange != NULL) OnSliderPosChange( GetSliderValue() );

			m_fSelectionEndX = (matrSliderNonInverted * Vecc3(v3DCoords.X)).X;
			if (OnSelectionChange != NULL) OnSelectionChange( GetSelectionStartValue(), GetSelectionEndValue() );

//		iStartDragX = x;
//		iStartDragY = y;

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
			if (m_iHeight + iVerticalPan < (iTrackHeight + iTrackPadding)*const_iTrackCount + 10)
				iVerticalPan += 10;
		}
		else
		{
			if (iVerticalPan > 0)
				iVerticalPan -= 10;
		}

		vUserSceneTranslation = Vecc3(0.0, float(iVerticalPan));

		if (OnVerticalPanChange) OnVerticalPanChange(vUserSceneTranslation);

		return true;
	}

	return false;

}


void TimelineSubWindow::Draw()
{
	for (auto iterElement : liGUI_Elements)
	{
		// propagate common values
		auto* track = dynamic_cast<GUI_ElementResizable*>(iterElement);
		track->matrSliderNonInverted = Mat4MakeTrans(0, -vUserSceneTranslation.Y, 0)*matrSliderNonInverted; // minus to get nonInverted vUserSceneTranslation
		
		iterElement->Draw();
	}


	SetupGraphicsPipelineWithIdentityModelViewMatrix();

	// matrUserScale contains transformation from HorrScrollBar,
	// while vUserSceneTranslation contains vertical scroll which we do not want
	glMultMatrixf(&matrUserScale.m[0][0]);

		// draw position line
		{
			glColor3f(1,0,0);

			glLineWidth(1);
			glLine( m_fSliderX, - m_iHeight/2,
					m_fSliderX,   m_iHeight/2, 5);
		}

		// Draw selection moir
		if (bSelectionIsValid)
		{
			glColor4f(1,1,1, 0.2);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				glQuad(m_fSelectionStartX, - m_iHeight/2, m_fSelectionEndX - m_fSelectionStartX, m_iHeight, 10);

			glDisable(GL_BLEND);
		}
}



// Called from outside
void TimelineSubWindow::SetSliderPos(float _val)
{
	m_fSliderX = _val*m_iWidth - m_iWidth/2.0;
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
	videoSlider = new VideoSlider(5,0, 20);
	videoSlider->SetAlignment(HALIGN_LEFT, VALIGN_CENTER);
	videoSlider->OnChange = [this](float fVal)
	{
		PositionMediator::Get()->SetPos(videoSlider, fVal);
	};
	PositionMediator::Get()->subscribeForPos([this](void* origin, float fVal)
	{
		if (origin != videoSlider) videoSlider->SetPos(fVal);
	});
	PositionMediator::Get()->subscribeForPosInit([this](void* origin, float fVal, unsigned int _iEndSec)
	{
		if (origin != videoSlider) videoSlider->SetPosInit(fVal, _iEndSec);
	});
	liGUI_Elements.push_back(videoSlider);

}


void TimelineSliderSubWindow::Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight)
{
	OpenGLSubWindowWithGUI::Reshape(iBottomLeftX, iBottomLeftY, iWidth, iHeight);

	videoSlider->Resize(iWidth-10, iHeight);
}


void TimelineSliderSubWindow::Draw()
{
	// propagate common values
	videoSlider->matrSliderNonInverted = matrSliderNonInverted;

	for (auto iterElement : liGUI_Elements)
		iterElement->Draw();

	//printf("Val=%f\n", videoSlider->GetValue());
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


void TimelineScrollBarSubWindow::Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight)
{
	OpenGLSubWindowWithGUI::Reshape(iBottomLeftX, iBottomLeftY, iWidth, iHeight);

	scrollBar->Resize(iWidth-2, iHeight);

}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////


TrackParamsSubWindow::TrackParamsSubWindow(int iParentWidth, int iParentHeight,
										   float fBottomLeftXperc, float fBottomLeftYperc,
										   float fWidthPerc, float fHeightPerc) :
					  OpenGLSubWindowWithGUI(iParentWidth, iParentHeight,
											 fBottomLeftXperc, fBottomLeftYperc, fWidthPerc, fHeightPerc)
{
	//OnChange = [this, mediator](float fVal)
	//{
	//	mediator->SetSliderPos(this, fVal);
	//};
	//mediator->subscribeForPosInit([this](void* origin, float fVal, unsigned int _iStartSec, unsigned int _iEndSec)
	//{
	//	if (origin != videoSlider) SetPosInit(fVal, _iStartSec, _iEndSec);
	//});

	for (int16_t iTrack = 1; iTrack <= const_iTrackCount; iTrack++)
	{
		ButtonImage* buttonImg = new ButtonImage("", -60, -iTrackHeight*iTrack - iTrackPadding*iTrack + 4, 32);
		std::string filename = "Icons\\Number" +  std::to_string(iTrack) + ".bmp";
		buttonImg->LoadImg(filename.c_str());
		buttonImg->SetAlignment(HALIGN_RIGHT, VALIGN_TOP);
		buttonImg->bEnabled = false;
		buttonImg->bDrawFrame = false;
		liGUI_Elements.push_back(buttonImg);
	}

	for (int16_t iTrack = 1; iTrack <= const_iTrackCount; iTrack++)
	{
		ButtonImage* buttonImg = new ButtonImage("", -20, -iTrackHeight*iTrack - iTrackPadding*iTrack + 4, 12);
		buttonImg->LoadImg("Icons\\Image12.bmp");
		buttonImg->LoadImgDownState("Icons\\Image13.bmp");
		buttonImg->SetAlignment(HALIGN_RIGHT, VALIGN_TOP);
		buttonImg->bDrawFrame = false;
		liGUI_Elements.push_back(buttonImg);
	}

}


