#include "stdafx.h"
#include "TimelineSubWindow.h"
#include "../../!!adGlobals/adOpenGLUtilities.h"
#include "../../!!adGUI/button.h"
#include "../../!!adGUI/Slider.h"
#include "../../!!adGlobals/glut/glut.h"


const int g_iTrackHeight	= 40;
const int g_iTrackPadding	= 10;
const int g_iClipPadding	= 7;
const int g_iTrackCount		= 5;
const int g_iTimelineBorder = 5;
const int g_iHorrScrollBarBorder = 1;

TimelineSubWindow::TimelineSubWindow(int iParentWidth, int iParentHeight,
									 float fBottomLeftXperc, float fBottomLeftYperc,
									 float fWidthPerc, float fHeightPerc) :
				   OpenGLSubWindowWithGUI(iParentWidth, iParentHeight,
										  fBottomLeftXperc, fBottomLeftYperc, fWidthPerc, fHeightPerc)
{
	matrSliderNonInverted = Mat4MakeIdent();

	m_fSliderPos01 = 0;
	iBorder        = g_iTimelineBorder;

	OnSliderPosChange = [this](double fPos)
	                    {  PositionMediator::Get()->SetPos0_1(this, fPos); };
	OnSelectionChange = [this](double fSelectionStart, double fSelectionEnd)
	                    { PositionMediator::Get()->SetSelection0_1(this, fSelectionStart, fSelectionEnd); };
	
	PositionMediator::Get()->subscribeForPos(this, [this](void* origin, double fVal)
						{ if (origin != this) SetSliderPos0_1(fVal); });
	PositionMediator::Get()->subscribeForPosInit(this, [this](void* origin, double fVal, int _iDuration10msTicks)
						{ if (origin != this) SetSliderPos0_1(fVal); });
	PositionMediator::Get()->subscribeForMarker(this, [this](void* origin, double fVal)
						{ if (origin != this) SetMarker0_1(fVal); });

	PopulateGUI();

	iVerticalPan = 0.0;

	bSelectionIsValid   = false;
	m_fSelStartX0_1		= 0.0;
	m_fSelEndX0_1		= 1.0;
	OnSelectionChange( 0.0f, 1.0f );	// empty selection means the whole timeline

	stateTimeLine = STATE_TIMELINE_IDLE;

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

int TimelineSubWindow::FindLastClipOnTrack_Tail(int iTrackNumber)
{
	int iMax = 0;
	for (auto iterEl : TrackClip::liClips)
	{
		if (iterEl->iTrack != iTrackNumber) continue;

		iMax = max(iMax, iterEl->m_iStartPos10msUnits + iterEl->m_iLength10msUnits);
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
	
	int iTail10msUnit = FindLastClipOnTrack_Tail(iActiveTrack);

	// nice touch: position clip starting at playhead on empty track
	if (iTail10msUnit == 0)
		iTail10msUnit = PositionMediator::Get()->Pos10msUnits();

	clip->SetAttr(iTail10msUnit, i10msCount);

	// ASSOCIATE CLIP TO TRACK
	clip->iTrack = iActiveTrack;

	// SET ALIGNMENT
	clip->SetAlignment(HALIGN_LEFT, VALIGN_TOP);
	clip->Reposition(-m_iWidth/2 + clip->iHPosShift, m_iHeight/2 + clip->iVPosShift);
	clip->Resize(Width(), 0);

	liGUI_Elements.push_back(clip);
	TrackClip::liClips.push_back(clip);

	// SET SELECTED TRACK TO THE ID OF THIS CLIP
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

// Called from outside
void TimelineSubWindow::SetMarker0_1(double _val)
{
	m_fMarkerPos01 = _val;
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
			//                 float                    int
			float m_fSliderX = m_fSliderPos01*Width() - Width()/2;

			glColor3f(1,0,0);

			glLineWidth(1);
			glLine( m_fSliderX, - m_iHeight/2,
					m_fSliderX,   m_iHeight/2, 20);
		}

		// Draw selection moir
		if (bSelectionIsValid)
		{
			float fSelectionStartX = m_fSelStartX0_1 * Width() - Width()/2;
			float fSelectionEndX   = m_fSelEndX0_1   * Width() - Width()/2;

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

		// DRAW marker if set
		if (m_fMarkerPos01 >= 0)
		{
			//                 float                    int
			float m_fMarkerX = m_fMarkerPos01*Width() - Width()/2;

			glColor4f(1,1,1, 0.5);

			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				glLineWidth(1);
				glLine( m_fMarkerX, - m_iHeight/2,
						m_fMarkerX,   m_iHeight/2, 20);

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


bool TimelineSubWindow::PassiveMotionFunc(int x, int y)
{
	// here go tracks and clips
	if (OpenGLSubWindowWithGUI::PassiveMotionFunc(x, y)) return true;

	if ((x > m_iBottomLeftX) && (x < m_iBottomLeftX + m_iWidth) &&
		(y > m_iBottomLeftY) && (y < m_iBottomLeftY + m_iHeight))
	{
		SetupGraphicsPipelineWithIdentityModelViewMatrix();

			Vec3d v3DCoords;
			gluUnProjectFriendlyZ(x, y, 0.5, 0, v3DCoords.X, v3DCoords.Y, v3DCoords.Z);

			float fX = (matrSliderNonInverted * Vecc3(v3DCoords.X)).X;

			if (bSelectionIsValid)
			{
				if (abs(fX + Width()/2 - m_fSelStartX0_1*Width()) < 2*matrSliderNonInverted.m[0][0])
				{
					glutSetCursor(GLUT_CURSOR_LEFT_RIGHT);
					return true;
				}
				else
				if (abs(fX + Width()/2 - m_fSelEndX0_1*Width()) < 2*matrSliderNonInverted.m[0][0])
				{
					glutSetCursor(GLUT_CURSOR_LEFT_RIGHT);
					return true;
				}
				else
					glutSetCursor(GLUT_CURSOR_INHERIT);
			}

			return false;
	}

	return false;
}

// OnClick
bool TimelineSubWindow::MouseFunc(int button, int state, int x, int y)
{
	// here go tracks and clips
	if (OpenGLSubWindowWithGUI::MouseFunc(button, state, x, y)) return true;

	if ((x > m_iBottomLeftX) && (x < m_iBottomLeftX + m_iWidth) &&
		(y > m_iBottomLeftY) && (y < m_iBottomLeftY + m_iHeight))
	{
		if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
		{
			SetupGraphicsPipelineWithIdentityModelViewMatrix();

				Vec3d v3DCoords;
				gluUnProjectFriendlyZ(x, y, 0.5, 0, v3DCoords.X, v3DCoords.Y, v3DCoords.Z);

				double fX = (matrSliderNonInverted * Vecc3(v3DCoords.X)).X;

				if ((abs(fX + Width()/2 - m_fSelStartX0_1*Width()) < 2*matrSliderNonInverted.m[0][0]) && bSelectionIsValid)
				{
					iStartDragX = x;
					iStartDragY = y;

					stateTimeLine = STATE_TIMELINE_DRAG_SELECTION_HEAD;
					return true;
				}
				else if ((abs(fX + Width()/2 - m_fSelEndX0_1*Width()) < 2*matrSliderNonInverted.m[0][0]) && bSelectionIsValid)
				{
					iStartDragX = x;
					iStartDragY = y;

					stateTimeLine = STATE_TIMELINE_DRAG_SELECTION_TAIL;
					return true;
				}
				else 
				{
					stateTimeLine = STATE_TIMELINE_DEFINE_SELECTION;
				
					// reset selection first
					if (bSelectionIsValid)
					{
						bSelectionIsValid   = false;
						if (OnSelectionChange != NULL) OnSelectionChange( 0.0f, 1.0f );	// empty selection means the whole timeline
					}

					iStartDragX = x;
					iStartDragY = y;

					m_fSelStartX0_1 = CLAMP((fX + Width()/2) / Width(), 0.0, 1.0);

					//if (OnChange != NULL) OnChange(Mat4MakeTrans(vUserSceneTranslation.X, 0, 0)*matrUserScale);

					return true;
				}
		}
	}

	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP) && (stateTimeLine = STATE_TIMELINE_DEFINE_SELECTION))
	{
		// make sure start is earlier than end
		if (m_fSelStartX0_1 > m_fSelEndX0_1)
		{
			std::swap(m_fSelStartX0_1, m_fSelEndX0_1);
			if (OnSelectionChange != NULL) OnSelectionChange( m_fSelStartX0_1, m_fSelEndX0_1 );
		}
	}

	stateTimeLine = STATE_TIMELINE_IDLE;

	return false;
}

// OnDrag
void TimelineSubWindow::MotionFunc(int x, int y)
{
	int iSnapPx = 9;

	OpenGLSubWindowWithGUI::MotionFunc(x, y);

	if ((x > m_iBottomLeftX) && (x < m_iBottomLeftX + m_iWidth) &&
		(y > m_iBottomLeftY) && (y < m_iBottomLeftY + m_iHeight))
	{
		// Ignore under 1 pixel drag for all cases
		if (abs(x - iStartDragX) < 1) return;

		// Pixels per 10ms
		float fPPU = float(m_iWidth)/PositionMediator::Get()->Duration10msUnits();
		
		SetupGraphicsPipelineWithIdentityModelViewMatrix();

			Vec3d v3DCoords;
			gluUnProjectFriendlyZ(x, y, 0.5, 0, v3DCoords.X, v3DCoords.Y, v3DCoords.Z);

			double fX0_1 = ((matrSliderNonInverted * Vecc3(v3DCoords.X)).X + Width()/2) / Width();

			if (stateTimeLine == STATE_TIMELINE_DEFINE_SELECTION)
			{
				bSelectionIsValid = true;

				m_fSliderPos01 = m_fSelStartX0_1;
				if (OnSliderPosChange != NULL) OnSliderPosChange( m_fSliderPos01 );

				m_fSelEndX0_1 = CLAMP(fX0_1, 0.0, 1.0);
				if (OnSelectionChange != NULL) OnSelectionChange( m_fSelStartX0_1, m_fSelEndX0_1 );

				return;
			}
			if ((stateTimeLine == STATE_TIMELINE_DRAG_SELECTION_HEAD) && bSelectionIsValid)
			{
				PositionMediator* mediator = PositionMediator::Get();

				if ( abs(fX0_1*mediator->Duration10msUnits() - mediator->Pos10msUnits()) < (iSnapPx*matrSliderNonInverted.m[0][0])/fPPU )
					fX0_1 = mediator->Pos0_1();

				int iTail10msUnits = mediator->Duration10msUnits()*m_fSelEndX0_1;

				// make sure the head of selection is not earlier than 0ms timeline and not closer to the tail of selection than 100ms
				m_fSelStartX0_1 = CLAMP(fX0_1, 0.0, double(iTail10msUnits - 10)/double(mediator->Duration10msUnits()));
				if (OnSelectionChange != NULL) OnSelectionChange( m_fSelStartX0_1, m_fSelEndX0_1 );

				return;
			}
			if ((stateTimeLine == STATE_TIMELINE_DRAG_SELECTION_TAIL) && bSelectionIsValid)
			{
				PositionMediator* mediator = PositionMediator::Get();

				if ( abs(fX0_1*mediator->Duration10msUnits() - mediator->Pos10msUnits()) < (iSnapPx*matrSliderNonInverted.m[0][0])/fPPU )
					fX0_1 = mediator->Pos0_1();

				int iHead10msUnits = PositionMediator::Get()->Duration10msUnits()*m_fSelStartX0_1;

				// make sure the tail of selection is not later than the end of the timeline and not closer to the head of selection than 100ms
				m_fSelEndX0_1 = CLAMP(fX0_1, double(iHead10msUnits + 10)/double(mediator->Duration10msUnits()), 1.0);
				if (OnSelectionChange != NULL) OnSelectionChange( m_fSelStartX0_1, m_fSelEndX0_1 );

				return;
			}
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

		vUserSceneTranslation = Vecc3(0.0, iVerticalPan);

		if (OnVerticalPanChange) OnVerticalPanChange(vUserSceneTranslation);

		return true;
	}

	return false;

}

void TimelineSubWindow::DeleteGUIelement(GUI_Element* _GUIelement)
{
	for (auto it = liGUI_Elements.begin(); it != liGUI_Elements.end(); ++it)
	{
		if ((*it) == _GUIelement)
		{
			delete *it;					// list owns the pointer
			liGUI_Elements.erase(it);
			break;
		}
	}
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
	PositionMediator::Get()->subscribeForPos(this, [this](void* origin, double fVal)
	{
		if (origin != videoSlider) videoSlider->SetPos0_1(fVal);
	});
	PositionMediator::Get()->subscribeForPosInit(this, [this](void* origin, double fVal, int _iDuration10msTicks)
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
	scrollBar = new HorScrollBar("", g_iHorrScrollBarBorder,1);
	scrollBar->SetAlignment(HALIGN_LEFT, VALIGN_BOTTOM);
	PositionMediator::Get()->subscribeForPosUpdateFromTimer(this, [this](void* origin, double fVal)
	{
		scrollBar->ScrollToMakePlayheadVisible(fVal);
	});
	scrollBar->iBorder = g_iTimelineBorder-g_iHorrScrollBarBorder;
	liGUI_Elements.push_back(scrollBar);
}


void TimelineScrollBarSubWindow::Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight)
{
	OpenGLSubWindowWithGUI::Reshape(iBottomLeftX, iBottomLeftY, iWidth, iHeight);

	scrollBar->Resize(iWidth-2*g_iHorrScrollBarBorder, iHeight);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////


TrackParamsSubWindow::TrackParamsSubWindow(int iParentWidth, int iParentHeight,
										   float fBottomLeftXperc, float fBottomLeftYperc,
										   float fWidthPerc, float fHeightPerc) :
					  OpenGLSubWindowWithGUI(iParentWidth, iParentHeight,
											 fBottomLeftXperc, fBottomLeftYperc, fWidthPerc, fHeightPerc)
{
	PopulateGUI();
}


std::vector<bool> TrackParamsSubWindow::liVisible;

void TrackParamsSubWindow::PopulateGUI()
{
	for (int16_t iTrack = 1; iTrack <= g_iTrackCount; iTrack++)
	{
		ButtonImage* buttonImg = new ButtonImage("", -60, -g_iTrackHeight*iTrack - g_iTrackPadding*iTrack + 4, 32);
		std::string filename = "Icons\\Number" + std::to_string(iTrack) + ".bmp";
		buttonImg->LoadImg(filename.c_str());
		buttonImg->SetAlignment(HALIGN_RIGHT, VALIGN_TOP);
		buttonImg->bEnabled = false;
		buttonImg->bDrawFrame = false;
		liGUI_Elements.push_back(buttonImg);
	}

	liVisible.push_back(true);	// zero track

	for (int16_t iTrack = 1; iTrack <= g_iTrackCount; iTrack++)
	{
		ButtonImage* buttonImg = new ButtonImage("", -20, -g_iTrackHeight*iTrack - g_iTrackPadding*iTrack + 4, 12);
		buttonImg->LoadImg("Icons\\Image12.bmp");
		buttonImg->LoadImgDownState("Icons\\Image13.bmp");
		buttonImg->SetAlignment(HALIGN_RIGHT, VALIGN_TOP);
		buttonImg->bDrawFrame = false;
		buttonImg->OnClick = [buttonImg, iTrack]()
							 {
								liVisible[iTrack] = buttonImg->bDownState;
								return true;
							 };
		liGUI_Elements.push_back(buttonImg);

		liVisible.push_back(true);
	}
}

float TrackParamsSubWindow::GetTrackVisibility(int iTrack)
{
	return liVisible[iTrack];
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//std::vector<float> TrackTranspSubWindow::liTransp;
//
//TrackTranspSubWindow::TrackTranspSubWindow( int iParentWidth, int iParentHeight,
//											float fBottomLeftXperc, float fBottomLeftYperc,
//											float fWidthPerc, float fHeightPerc) :
//					  OpenGLSubWindowWithGUI(iParentWidth, iParentHeight,
//											 fBottomLeftXperc, fBottomLeftYperc, fWidthPerc, fHeightPerc)
//{
//	PopulateGUI();
//}
//
//void TrackTranspSubWindow::PopulateGUI()
//{
//	liTransp.reserve(g_iTrackCount);
//	for (int16_t iTrack = 1; iTrack <= g_iTrackCount; iTrack++)
//	{
//		liTransp.push_back(100);
//
//		Slider<SL_INT>* SliderTrackTransparency;
//		SliderTrackTransparency = new Slider<SL_INT>("%", 10,-g_iTrackHeight*iTrack - g_iTrackPadding*iTrack + 14, 0,100, &liTransp.back(), 7.2);
//		SliderTrackTransparency->SetAlignment(HALIGN_LEFT, VALIGN_TOP);
//		SliderTrackTransparency->SetBoxWidth(50);
//		SliderTrackTransparency->SetBoxSeparation(1);
//		SliderTrackTransparency->bDrawMinMaxValues = false;
//		SliderTrackTransparency->fValueGranularity = 1;
//		SliderTrackTransparency->fTickGranularity = 25;
//		liGUI_Elements.push_back(SliderTrackTransparency);
//	}
//
//}
//
//float TrackTranspSubWindow::GetTrackTransp(int iTrack)
//{
//	return liTransp[iTrack];
//}
//

