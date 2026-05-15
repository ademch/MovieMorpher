#include "stdafx.h"
#include "TimelineSubWindow.h"
#include "../../!!adGlobals/adOpenGLUtilities.h"
#include "../../!!adGUI/button.h"
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

	m_fSliderPos01 = 0;
	iBorder        = 5;

	OnSliderPosChange = [this](float fPos)
	                    {  PositionMediator::Get()->SetPos(this, fPos); };
	OnSelectionChange = [this](float fSelectionStart, float fSelectionEnd)
	                    { PositionMediator::Get()->SetSelection(this, fSelectionStart, fSelectionEnd); };
	PositionMediator::Get()->subscribeForPos([this](void* origin, float fVal)
						{ if (origin != this) SetSliderPos0_1(fVal); });
	PositionMediator::Get()->subscribeForPosInit([this](void* origin, float fVal, unsigned int _iDuration)
						{ if (origin != this) SetSliderPos0_1(fVal); });

	for (int16_t iTrack = 1; iTrack <= const_iTrackCount; iTrack++)
	{
		TimelineTrack*  track = new TimelineTrack(iTrack,
			                                      iBorder,
			                                      -iTrackHeight*iTrack - iTrackPadding*iTrack,
			                                      Width(),
			                                      iTrackHeight);
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
	TrackClip* clip = new TrackClip(1,													// id
		                            iBorder,											// px
		                            -iTrackHeight*iTrack - iTrackPadding*iTrack +10,	// py
									Width(),											// width
		                            20);												// height
	
	//float iDuration = mediator->Duration();
	clip->SetAttr(0, 100.0);

	clip->iTrack = iTrack;
	clip->SetAlignment(HALIGN_LEFT, VALIGN_TOP);
	liGUI_Elements.push_back(clip);

	clip->Reposition(-Width()/2, Height()/2 + clip->iVPosShift);
}


// Called from outside
void TimelineSubWindow::SetSliderPos0_1(float _val)
{
	m_fSliderPos01 = _val;
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
		float m_fSliderX = (m_fSliderPos01 - 0.5)*Width();

		glColor3f(1,0,0);

		glLineWidth(1);
		glLine( m_fSliderX, - m_iHeight/2,
				m_fSliderX,   m_iHeight/2, 20);
	}

	// Draw selection moir
	if (bSelectionIsValid)
	{
		float fSelectionStartX = (m_fSelectionStartX0_1 - 0.5) * Width();
		float fSelectionEndX   = (m_fSelectionEndX0_1   - 0.5) * Width();

		glColor4f(1,1,1, 0.2);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			glQuad( fSelectionStartX,
				   -m_iHeight/2,
				    fSelectionEndX - fSelectionStartX,
				    m_iHeight,
				    10);

		glDisable(GL_BLEND);
	}
}


void TimelineSubWindow::Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight)
{
	OpenGLSubWindowWithGUI::Reshape(iBottomLeftX, iBottomLeftY, iWidth, iHeight);

	for (auto* iterElement : liGUI_Elements)
	{
		auto* track = dynamic_cast<GUI_ElementResizable*>(iterElement);
		track->Resize(Width(), m_iHeight/3.0);
	}
}

// OnClick
bool TimelineSubWindow::MouseFunc(int button, int state, int x, int y)
{
	// here go tracks and clips
	if (OpenGLSubWindowWithGUI::MouseFunc(button, state, x, y)) return true;

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

				m_fSelectionStartX0_1 = ((matrSliderNonInverted * Vecc3(v3DCoords.X)).X  + Width()/2.0) / Width();

				//if (OnChange != NULL) OnChange(Mat4MakeTrans(vUserSceneTranslation.X, 0, 0)*matrUserScale);

				return true;
			}
		}
	}

	bMouseButtonPressed = false;

	return false;
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

			m_fSliderPos01 = m_fSelectionStartX0_1;
			if (OnSliderPosChange != NULL) OnSliderPosChange( m_fSliderPos01 );

			m_fSelectionEndX0_1 = ((matrSliderNonInverted * Vecc3(v3DCoords.X)).X  + Width()/2.0) / Width();
			if (OnSelectionChange != NULL) OnSelectionChange( m_fSelectionStartX0_1, m_fSelectionEndX0_1 );
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



////////////////////////////////////////////////////////////////////////////////////////////////

TimelineSliderSubWindow::TimelineSliderSubWindow(int iParentWidth, int iParentHeight,
												 float fBottomLeftXperc, float fBottomLeftYperc,
												 float fWidthPerc, float fHeightPerc) :
						 OpenGLSubWindowWithGUI(iParentWidth, iParentHeight,
												fBottomLeftXperc, fBottomLeftYperc, fWidthPerc, fHeightPerc)
{
	matrSliderNonInverted = Mat4MakeIdent();

	iBorder = 5;

	static float f = 1000;
	videoSlider = new VideoSlider(iBorder,0, 20);
	videoSlider->SetAlignment(HALIGN_LEFT, VALIGN_CENTER);
	videoSlider->OnChange = [this](float fVal)
							{ PositionMediator::Get()->SetPos(videoSlider, fVal); };
	PositionMediator::Get()->subscribeForPos([this](void* origin, float fVal)
	{
		if (origin != videoSlider) videoSlider->SetPos0_1(fVal);
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

	videoSlider->Resize(Width(), iHeight);
}


void TimelineSliderSubWindow::Draw()
{
	// propagate common values
	videoSlider->matrSliderNonInverted = matrSliderNonInverted;

	for (auto iterElement : liGUI_Elements)
		iterElement->Draw();

	//printf("Val=%f\n", videoSlider->GetPos01());
}


////////////////////////////////////////////////////////////////////////////////////////////////


TimelineScrollBarSubWindow::TimelineScrollBarSubWindow(int iParentWidth, int iParentHeight,
													   float fBottomLeftXperc, float fBottomLeftYperc,
													   float fWidthPerc, float fHeightPerc) :
						    OpenGLSubWindowWithGUI(iParentWidth, iParentHeight,
												   fBottomLeftXperc, fBottomLeftYperc, fWidthPerc, fHeightPerc)
{
	scrollBar = new HorScrollBar("", 1,1);
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
	//	mediator->SetSliderPos0_1(this, fVal);
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


