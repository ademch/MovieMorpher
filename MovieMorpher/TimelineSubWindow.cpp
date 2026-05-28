#include "stdafx.h"
#include "TimelineSubWindow.h"
#include "../../!!adGlobals/adOpenGLUtilities.h"
#include "../../!!adGUI/button.h"
#include "../../!!adGlobals/glut/glut.h"


const int g_iTrackHeight	= 40;
const int g_iTrackPadding	= 10;
const int g_iClipPadding	= 7;
const int g_iTrackCount		= 5;
const int g_iTimelineBorder = 5;

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
	iBorder        = g_iTimelineBorder;

	OnSliderPosChange = [this](double fPos)
	                    {  PositionMediator::Get()->SetPos0_1(this, fPos); };
	OnSelectionChange = [this](double fSelectionStart, double fSelectionEnd)
	                    { PositionMediator::Get()->SetSelection(this, fSelectionStart, fSelectionEnd); };
	PositionMediator::Get()->subscribeForPos([this](void* origin, double fVal)
						{ if (origin != this) SetSliderPos0_1(fVal); });
	PositionMediator::Get()->subscribeForPosInit([this](void* origin, double fVal, int _iDuration10msTicks)
						{ if (origin != this) SetSliderPos0_1(fVal); });

	PopulateGUI();

	iVerticalPan = 0.0;
}


void TimelineSubWindow::PopulateGUI()
{
	for (int16_t iTrack = 1; iTrack <= g_iTrackCount; iTrack++)
	{
		TimelineTrack*  track = new TimelineTrack(iTrack,
												  iBorder,
												  -g_iTrackHeight*iTrack - g_iTrackPadding*iTrack,
												  Width(),
												  g_iTrackHeight);
		track->SetAlignment(HALIGN_LEFT, VALIGN_TOP);
		liGUI_Elements.push_back(track);
		// shortcut
		//liTracks.push_back(track);
	}
}

int TimelineSubWindow::FindLastClipOnTrack_TailFrame(int iTrackNumber)
{
	int iMax = 0;
	for (auto iterEl : TrackClip::liClips)
	{
		if (iterEl->iTrack != iTrackNumber) continue;

		iMax = max(iMax, iterEl->m_iStartPos10msTicks + iterEl->m_iLength10msTicks);
	}

	return iMax;
}



TrackClip* TimelineSubWindow::AddClip(OpenGLSubWindowWithGUI* wndTool, int i10msCount)
{
	int iActiveTrack = TimelineTrack::iSelected;

	TrackClip* clip = new TrackClip(TrackClip::liClips.size(),														// id
		                            iBorder,																		// x
									-g_iTrackHeight*iActiveTrack - g_iTrackPadding*iActiveTrack + g_iClipPadding,	// y
									Width(),																		// width
		                            26);																			// height
	
	int iTailFrame = FindLastClipOnTrack_TailFrame(iActiveTrack);
	clip->SetAttr(iTailFrame, i10msCount);

	clip->iTrack = iActiveTrack;
	clip->SetAlignment(HALIGN_LEFT, VALIGN_TOP);
	clip->Reposition(-m_iWidth/2 + clip->iHPosShift, m_iHeight/2 + clip->iVPosShift);
	clip->Resize(Width(), 0);

	liGUI_Elements.push_back(clip);
	TrackClip::liClips.push_back(clip);

	TrackClip::iSelected = clip->id;

	// Pass callback deeper
	clip->OnClipChange = m_setWindowCallback;
	clip->windowTool   = wndTool;
	clip->windowTool->zOrder = iActiveTrack;

	return clip;
}


// Called from outside
void TimelineSubWindow::SetSliderPos0_1(double _val)
{
	m_fSliderPos01 = _val;
}


void TimelineSubWindow::Draw()
{
	for (auto iterElement : liGUI_Elements)
	{
		// propagate common values
		auto* element = dynamic_cast<GUI_ElementResizable*>(iterElement);
		element->matrSliderNonInverted = Mat4MakeTrans(0, -vUserSceneTranslation.Y, 0)*matrSliderNonInverted; // minus to get nonInverted vUserSceneTranslation

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
		auto* elemRes = dynamic_cast<GUI_ElementResizable*>(iterElement);
		elemRes->Resize(Width(), 0);
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
			if (m_iHeight + iVerticalPan < (g_iTrackHeight + g_iTrackPadding)*g_iTrackCount + 10)
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

	iBorder = g_iTimelineBorder;

	PopulateGUI();
}


void TimelineSliderSubWindow::PopulateGUI()
{
	videoSlider = new VideoSlider(iBorder,0, 20);
	videoSlider->SetAlignment(HALIGN_LEFT, VALIGN_CENTER);
	videoSlider->OnChange = [this](double fVal)
	{
		PositionMediator::Get()->SetPos0_1(videoSlider, fVal);
	};
	PositionMediator::Get()->subscribeForPos([this](void* origin, double fVal)
	{
		if (origin != videoSlider) videoSlider->SetPos0_1(fVal);
	});
	PositionMediator::Get()->subscribeForPosInit([this](void* origin, double fVal, int _iDuration10msTicks)
	{
		if (origin != videoSlider) videoSlider->SetPosInit(fVal, _iDuration10msTicks);
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
	PopulateGUI();
}

void TimelineScrollBarSubWindow::PopulateGUI()
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
	//mediator->subscribeForPosInit([this](void* origin, float fVal, unsigned int _iStartSec, int _iDuration10msTicks)
	//{
	//	if (origin != videoSlider) SetPosInit(fVal, _iStartSec, _iDuration10msTicks);
	//});

	PopulateGUI();
}

void TrackParamsSubWindow::PopulateGUI()
{
	for (int16_t iTrack = 1; iTrack <= g_iTrackCount; iTrack++)
	{
		ButtonImage* buttonImg = new ButtonImage("", -60, -g_iTrackHeight*iTrack - g_iTrackPadding*iTrack + 4, 32);
		std::string filename = "Icons\\Number" +  std::to_string(iTrack) + ".bmp";
		buttonImg->LoadImg(filename.c_str());
		buttonImg->SetAlignment(HALIGN_RIGHT, VALIGN_TOP);
		buttonImg->bEnabled = false;
		buttonImg->bDrawFrame = false;
		liGUI_Elements.push_back(buttonImg);
	}

	for (int16_t iTrack = 1; iTrack <= g_iTrackCount; iTrack++)
	{
		ButtonImage* buttonImg = new ButtonImage("", -20, -g_iTrackHeight*iTrack - g_iTrackPadding*iTrack + 4, 12);
		buttonImg->LoadImg("Icons\\Image12.bmp");
		buttonImg->LoadImgDownState("Icons\\Image13.bmp");
		buttonImg->SetAlignment(HALIGN_RIGHT, VALIGN_TOP);
		buttonImg->bDrawFrame = false;
		liGUI_Elements.push_back(buttonImg);
	}
}


