#include "stdafx.h"
#include "../../!!adGUI/arrow.h"
#include "MorphingToolSubWindow.h"
#include "GlobalParamsSubWindow.h"
#include "GLSL_Pipeline.h"
#include "../../!!adGlobals/glut/glut.h"
#include "../../!!adExtensions/extensions.h"
#include "../../!!adGlobals/adOpenGLUtilities.h"
#include "../../!!adGUI/TrackClip.h"
#include "../../!!adGUI/VideoPositionMediator.h"


const int  _fFinalizationRadius = 9;
const float const_fPointsDepth	= 0.2;
const float const_fPointsSize	= 7;
const float const_fLineWidth	= 2;
const float const_fJitter       = 5;


bool bDoubleClick = false;

void DoubleClickTimer(int value)
{
	bDoubleClick = false;
}


MorphingToolSubWindow::MorphingToolSubWindow(int iParentWidth, int iParentHeight,
											 float fBottomLeftXperc, float fBottomLeftYperc,
											 float fWidthPerc, float fHeightPerc) :
	                   OpenGLSubWindowWithGUI(iParentWidth, iParentHeight,
											  fBottomLeftXperc, fBottomLeftYperc, fWidthPerc, fHeightPerc)
{
	stateCurrent = STATE_IDLE;

	bSrcCurveIsDone = false;
	bDstCurveIsDone = false;

	m_ParamsSubWindow = NULL;

	m_bMouseDrawingInProgress = false;

	ptPrevPoint = Vecc2();

	PopulateGUI();

	morphFBOprocessor = new MorphFBOprocessor(0, 0, 800, 450, texBank);

	PositionMediator::Get()->subscribeForPos(this, [this](void* origin, double fVal)
	{
		RecalcAnimatedParamsFromKeyframes();
	});
}


MorphingToolSubWindow::~MorphingToolSubWindow()
{
	delete morphFBOprocessor;
}


void MorphingToolSubWindow::RecalcAnimatedParamsFromKeyframes()
{
	liSource      = animatedPolylineSrc.Evaluate( GetClipLocalTimeS() );
	liDestination = animatedPolylineDst.Evaluate( GetClipLocalTimeS() );

	// keyframes dictate current state, the spline not saved into keyframe is lost
	if (liSource.size())
		bSrcCurveIsDone = true;

	if (liDestination.size())
		bDstCurveIsDone = true;

	if (liSource.size() == liDestination.size())
		UploadMorphingLines();
}



void MorphingToolSubWindow::PopulateGUI()
{
	buttonSource = new Button("Draw src", -230,10, 100, 6.3);
	buttonSource->SetAlignment(HALIGN_CENTER, VALIGN_BOTTOM);
	buttonSource->OnClick = [this]() { return SourcePolylineClicked(); };
	liGUI_Elements.push_back(buttonSource);

	Arrow*  arrow;
	arrow = new Arrow("", -120,10 + 8, 30, 6.3);
	arrow->SetAlignment(HALIGN_CENTER, VALIGN_BOTTOM);
	liGUI_Elements.push_back(arrow);

	buttonDestination = new Button("Draw dst", -80,10, 100, 6.3);
	buttonDestination->SetAlignment(HALIGN_CENTER, VALIGN_BOTTOM);
	buttonDestination->OnClick = [this]() { return DestinationPolylineClicked(); };
	liGUI_Elements.push_back(buttonDestination);

	buttonMorphNow = new Button("Morph now", 40,10, 100, 6.3);
	buttonMorphNow->SetAlignment(HALIGN_CENTER, VALIGN_BOTTOM);
	buttonMorphNow->OnClick = [this]() { return MorphNow(); };
	liGUI_Elements.push_back(buttonMorphNow);

	buttonResetView = new Button("Reset view", -180,-30, 100, 6); 
	buttonResetView->SetAlignment(HALIGN_RIGHT, VALIGN_TOP);
	buttonResetView->OnClick = [this]() { return ResetView(); };
	liGUI_Elements.push_back(buttonResetView);

	buttonClear = new Button("Clear", 190,10, 100, 6.3);
	buttonClear->SetAlignment(HALIGN_CENTER, VALIGN_BOTTOM);
	buttonClear->OnClick = [this]() { return ClearMorph(); };
	liGUI_Elements.push_back(buttonClear);

}


void MorphingToolSubWindow::ReshapeFBOprocessors(int _iBottomLeftX, int _iBottomLeftY, int _iWidth, int _iHeight)
{
	morphFBOprocessor->Reshape(_iBottomLeftX, _iBottomLeftY, _iWidth, _iHeight);
}

void MorphingToolSubWindow::TextureUpdateInputFBOprocessor(int _iWidth, int _iHeight, unsigned char* image)
{
	morphFBOprocessor->TextureUpdate(_iWidth, _iHeight, image);
}


void MorphingToolSubWindow::Draw()
{
	OpenGLSubWindowWithGUI::Draw();

	SetupGraphicsPipeline();

	GLfloat mv[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, mv);

		sprintf(m_strCaption, "%s %5.0f%%", "Zoom", fUserScale*100.0f);

		if (GlobalParamsSubWindow::Get()->PointsAreVisible())
		{
			// SOURCE
			{
				//glEnable(GL_LINE_SMOOTH);
				glLineWidth(const_fLineWidth);
				glColor3f(0.83, 0.69, 0);
				std::vector<Vec2> listOutSrc;
				CatmullSubdivide(liSource, listOutSrc, 10);
				glBegin(GL_LINE_STRIP);
					for (auto element : listOutSrc) {
						glVertex3f(element.X, element.Y, 0.1);
					}
				glEnd();

				glPointSize(const_fPointsSize);
				glColor3f(0.93, 0.8, 0);
				glBegin(GL_POINTS);
					for (auto element : liSource) {
						glVertex3f(element.X, element.Y, const_fPointsDepth);
					}
				glEnd();
			}

			// DESTINATION
			{
				glLineWidth(const_fLineWidth);
				glColor3f(0.23, 0.71, 0);
				std::vector<Vec2> listOutDst;
				CatmullSubdivide(liDestination, listOutDst, 10);
				glBegin(GL_LINE_STRIP);
					for (auto element : listOutDst) {
						glVertex3f(element.X, element.Y, 0.1);
					}
				glEnd();

				glPointSize(const_fPointsSize);
				glColor3f(0.3, 0.8, 0);
				glBegin(GL_POINTS);
					for (auto element : liDestination) {
						glVertex3f(element.X, element.Y, const_fPointsDepth);
					}
				glEnd();
			}

			glColor3f(1,0,0);
			glLineWidth(3);
			float fFinRadCorrected = _fFinalizationRadius / mv[0];

			if ((stateCurrent == STATE_SOURCE_POINT_INPUT) && (liSource.size() > 0))
				DrawCircle(Vecc3(liSource.back(), 0.3), fFinRadCorrected, 20);
			if ((stateCurrent == STATE_DESTINATION_POINT_INPUT) && (liDestination.size() > 0))
				DrawCircle(Vecc3(liDestination.back(), 0.3), fFinRadCorrected, 20);

			// Highlight matching point in a source curve when adding points to destination
			if ((stateCurrent == STATE_DESTINATION_POINT_INPUT) &&
				(liDestination.size() > 0) && (liSource.size() > 0))
			{
				unsigned int indexLast = liDestination.size();
				if (indexLast <= liSource.size())
					DrawCircle(Vecc3(liSource[indexLast-1].X, liSource[indexLast-1].Y, 0.3), fFinRadCorrected, 20);
			}

		}
	///////

	if (bSrcCurveIsDone) {
		buttonSource->_text = "Clear Src";
		buttonSource->bEnabled = true;
	}
	if (bDstCurveIsDone) {
		buttonDestination->_text = "Clear Dst";
		buttonDestination->bEnabled = true;
	}

	bool bFBOparamsInSync = (morphFBOprocessor->fMorphRadius   == m_ParamsSubWindow->MorphRadius()) &&
						    (morphFBOprocessor->fMorphPower    == m_ParamsSubWindow->MorphPower()) &&
						    (morphFBOprocessor->fMorphRatio    == m_ParamsSubWindow->MorphRatio()) &&
						    (morphFBOprocessor->bShowWireframe == GlobalParamsSubWindow::Get()->IsWireframeShown());

	if (morphFBOprocessor->bOutdated || !bFBOparamsInSync)
		ReDrawFBOprocessors();
}


void MorphingToolSubWindow::SaveMorphingLinesIntoAnimationSequence()
{
	// save keyframe only when src and destination is done
	if (bSrcCurveIsDone && bDstCurveIsDone)
	{
		animatedPolylineSrc.SetValueAt( GetClipLocalTimeS(), liSource );
		animatedPolylineDst.SetValueAt( GetClipLocalTimeS(), liDestination );
	}
}


// Texture layout       ____________________ 
//           SRC float | st st st st ... st |
//           DST float | st st st st ... st |
//                     ---------------------
void MorphingToolSubWindow::UploadMorphingLines()
{
	assert(liSource.size() == liDestination.size());

	std::vector<Vec2> listOutSrc;
	CatmullSubdivide(liSource, listOutSrc, 20);

	std::vector<Vec2> listOutDst;
	CatmullSubdivide(liDestination, listOutDst, 20);

	assert(listOutSrc.size() == listOutDst.size());

	glActiveTextureARB(GL_TEXTURE1);

		glBindTexture(GL_TEXTURE_2D, texBank[TEXTURE_FLOAT_BUFFER]->m_uiTextureID);

		if (liSource.size() > 0)
		{
			texBank[TEXTURE_FLOAT_BUFFER]->m_width  = listOutSrc.size();
			texBank[TEXTURE_FLOAT_BUFFER]->m_height = 2;

			//           targ         mml  int frmt  w                  h brdr inc: frmt    type    data
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, listOutSrc.size(), 2,   0,    GL_RG, GL_FLOAT, NULL);

			glTexSubImage2D(GL_TEXTURE_2D, 0, 0,0, listOutSrc.size(), 1, GL_RG, GL_FLOAT, listOutSrc.data());
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0,1, listOutDst.size(), 1, GL_RG, GL_FLOAT, listOutDst.data());
		}
		else
		{
			// empty texture 1x1
			texBank[TEXTURE_FLOAT_BUFFER]->m_width  = 1;
			texBank[TEXTURE_FLOAT_BUFFER]->m_height = 1;

			//           targ         mml  int frmt  w  h brdr inc: frmt    type    data
			Vec2 data = Vecc2();
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, 1, 1,   0,    GL_RG, GL_FLOAT, &data);
		}

	glActiveTextureARB(GL_TEXTURE0);

	morphFBOprocessor->bOutdated = true;
}


bool MorphingToolSubWindow::PassiveMotionFunc(int x, int y)
{
	Vec3d v3DCoords;

	if (OpenGLSubWindowWithGUI::PassiveMotionFunc(x, y)) return true;

	if ((x > m_iBottomLeftX) && (x < m_iBottomLeftX + m_iWidth) &&
		(y > m_iBottomLeftY) && (y < m_iBottomLeftY + m_iHeight))
	{
		if (stateCurrent == STATE_IDLE)
		{
			SetupGraphicsPipeline();

			// transform mouse screen coords back to object coords
			gluUnProjectFriendly(x, y, 0, v3DCoords.X, v3DCoords.Y, v3DCoords.Z);

			GLfloat mv[16];
			glGetFloatv(GL_MODELVIEW_MATRIX, mv);

				for (auto point : liDestination)
				{
					if (VecLengthSqr(v3DCoords - Vecc3d(point.X, point.Y, const_fPointsDepth)) < sqr(const_fJitter/(mv[0])))
					{
						glutSetCursor(GLUT_CURSOR_TOP_SIDE);
						return true;
					}
				}
		}
		else if ((stateCurrent == STATE_SOURCE_POINT_INPUT)   || (stateCurrent == STATE_DESTINATION_POINT_INPUT) ||
			     (stateCurrent == STATE_SOURCE_DRAWING_INPUT) || (stateCurrent == STATE_DESTINATION_DRAWING_INPUT))
		{
			glutSetCursor(GLUT_CURSOR_TOP_SIDE);
			return true;
		}
	}

	return false;
}

// Mouse button callback
bool MorphingToolSubWindow::MouseFunc(int button, int state, int x, int y)
{
	float fJitterLine   = 12;
	float fBlindZoneRad = 20;

	if (OpenGLSubWindowWithGUI::MouseFunc(button, state, x, y)) return true;

	if ((x > m_iBottomLeftX) && (x < m_iBottomLeftX + m_iWidth) &&
		(y > m_iBottomLeftY) && (y < m_iBottomLeftY + m_iHeight))
	{

		SetupGraphicsPipeline();

		Vec3d v3DCoords;
		gluUnProjectFriendly(x, y, 0, v3DCoords.X, v3DCoords.Y, v3DCoords.Z);

		GLfloat mv[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, mv);

			// We enter here if drawing mode has been initiated from outside and that is our first mouse click
			if ( ((stateCurrent == STATE_SOURCE_DRAWING_INPUT) || (stateCurrent == STATE_DESTINATION_DRAWING_INPUT)) &&
				 (button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
			{
				if (stateCurrent == STATE_SOURCE_DRAWING_INPUT)
					liSource.push_back( Vecc2(v3DCoords) );
				else if (stateCurrent == STATE_DESTINATION_DRAWING_INPUT)
					liDestination.push_back( Vecc2(v3DCoords) );

				ptPrevPoint = Vecc2(x, y);

				// MouseMove does not have a way to get button state, we signal the drawing has started
				m_bMouseDrawingInProgress = true;

				return true;
			}
			// If button has been released in drawing mode there are two options:
			// 1. released at the same !starting! point- switch to point input mode
			// 2. released at the other point- input is done
			else if (((stateCurrent == STATE_SOURCE_DRAWING_INPUT) || (stateCurrent == STATE_DESTINATION_DRAWING_INPUT)) &&
					 (button == GLUT_LEFT_BUTTON) && (state == GLUT_UP))
			{
				if ((stateCurrent == STATE_SOURCE_DRAWING_INPUT) &&
					(liSource.size() == 1) && (PointDist(Vecc2(x, y), ptPrevPoint) < 5))
				{
					stateCurrent = STATE_SOURCE_POINT_INPUT;

					m_bMouseDrawingInProgress = false;

					return true;
				}
				else if ((stateCurrent == STATE_DESTINATION_DRAWING_INPUT) &&
					(liDestination.size() == 1) && (PointDist(Vecc2(x, y), ptPrevPoint) < 5))
				{
					stateCurrent = STATE_DESTINATION_POINT_INPUT;

					m_bMouseDrawingInProgress = false;

					return true;
				}
				else
				{
					if (stateCurrent == STATE_SOURCE_DRAWING_INPUT)
						bSrcCurveIsDone = true;
					else if (stateCurrent == STATE_DESTINATION_DRAWING_INPUT)
						bDstCurveIsDone = true;

					SaveMorphingLinesIntoAnimationSequence();

					stateCurrent = STATE_IDLE;

					m_bMouseDrawingInProgress = false;

					return true;
				}
			}
			// if button has been pressed in point input mode there are two options:
			// * pressed at the same point- input is done
			// * pressed at the other point- register input
			else if (((stateCurrent == STATE_SOURCE_POINT_INPUT) || (stateCurrent == STATE_DESTINATION_POINT_INPUT)) &&
					 (button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
			{
				if (PointDist(Vecc2(x, y), ptPrevPoint) <= _fFinalizationRadius)
				{
					if (stateCurrent == STATE_SOURCE_POINT_INPUT)
						bSrcCurveIsDone = true;
					else if (stateCurrent == STATE_DESTINATION_POINT_INPUT)
						bDstCurveIsDone = true;

					SaveMorphingLinesIntoAnimationSequence();

					stateCurrent = STATE_IDLE;

					return true;
				}
				else
				{
					if (stateCurrent == STATE_SOURCE_POINT_INPUT)
						liSource.push_back( Vecc2(v3DCoords) );
					else if (stateCurrent == STATE_DESTINATION_POINT_INPUT)
						liDestination.push_back( Vecc2(v3DCoords) );

					ptPrevPoint = Vecc2(x, y);

					return true;
				}
			}
			// 1. We enter here for recording of initial position of a point for drag
			// 2. We enter here to add additional point to a finished line
			else if ((stateCurrent == STATE_IDLE) && (button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
			{
				if (bSrcCurveIsDone || bDstCurveIsDone)
				{
					for (auto& point : liDestination)
					{
						if (VecLengthSqr(v3DCoords - Vecc3d(point.X, point.Y, const_fPointsDepth)) < sqr(const_fJitter/mv[0]))
						{
							glutSetCursor(GLUT_CURSOR_TOP_SIDE);
							stateCurrent = STATE_POINT_DRAG;
							m_ptrDraggedPoint = &point;

							return true;
						}
					}
				}

				// allow adding points only after both curves are done (disputable)
				if (bSrcCurveIsDone && bDstCurveIsDone)
				{
					for (unsigned int i = 0; i < liDestination.size() - 1; i++)
					{
						if ((PointDistSqr(Vecc2(v3DCoords), liDestination[i])   > sqr(fBlindZoneRad/mv[0])) &&
							(PointDistSqr(Vecc2(v3DCoords), liDestination[i+1]) > sqr(fBlindZoneRad/mv[0])))
						{
							Vec3 ptOut;
							if (PointDistToLineSegment(Vecc3(v3DCoords), Vecc3(liDestination[i]), Vecc3(liDestination[i + 1]), ptOut) <
							   (fJitterLine / mv[0]))
							{

								if (bDoubleClick)
								{
									// insert before second point
									liDestination.insert(liDestination.begin() + i + 1, Vecc2(v3DCoords));

									if (liSource.size() + 1 == liDestination.size())
									{
										liSource.insert(liSource.begin()+i + 1, (liSource[i] + liSource[i + 1]) / 2.0f);
										SaveMorphingLinesIntoAnimationSequence();
										UploadMorphingLines();
									}

									bDoubleClick = false;

									return true;
								}
								else
								{
									bDoubleClick = true;
									glutTimerFunc(250, DoubleClickTimer, 0);

									return true;
								}
							}
						}
					}
				
				}
			}
			// 1. We enter here to remove a point from a finished line
			else if ((stateCurrent == STATE_IDLE) && (button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN))
			{
				// allow removing points only after both curves are done (disputable)
				if (bSrcCurveIsDone && bDstCurveIsDone)
				{
					if (liDestination.size() > 1)
					{
						for (unsigned int i = 0; i < liDestination.size(); i++)
						{
							if (PointDistSqr(Vecc2(v3DCoords), liDestination[i]) < sqr(const_fJitter / mv[0]))
							{

								if (bDoubleClick)
								{
									liDestination.erase(liDestination.begin() + i);

									if (liSource.size() - 1 == liDestination.size())
									{
										liSource.erase(liSource.begin() + i);
										SaveMorphingLinesIntoAnimationSequence();
										UploadMorphingLines();
									}

									bDoubleClick = false;

									return true;
								}
								else
								{
									bDoubleClick = true;
									glutTimerFunc(250, DoubleClickTimer, 0);

									return true;
								}
							}
						}
					}
				}
			}
			// Left mouse button has been released being in drag mode
			else if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP) && (stateCurrent == STATE_POINT_DRAG))
			{
				stateCurrent = STATE_IDLE;

				return true;
			}
	}

	return false;
}


void MorphingToolSubWindow::MotionFunc(int x, int y)
{
	OpenGLSubWindowWithGUI::MotionFunc(x, y);

	if ((x > m_iBottomLeftX) && (x < m_iBottomLeftX + m_iWidth) &&
		(y > m_iBottomLeftY) && (y < m_iBottomLeftY + m_iHeight))
	{

		SetupGraphicsPipeline();

		Vec3d v3DCoords;
		gluUnProjectFriendlyZ(x, y, 0.5f, 0, v3DCoords.X, v3DCoords.Y, v3DCoords.Z);

		if (((stateCurrent == STATE_SOURCE_DRAWING_INPUT) || (stateCurrent == STATE_DESTINATION_DRAWING_INPUT)) && m_bMouseDrawingInProgress)
		{
			float fRadius = 30;
			if (VecLength(Vecc2(x, y) - ptPrevPoint) > fRadius)
			{
				if (stateCurrent == STATE_SOURCE_DRAWING_INPUT)
					liSource.push_back( Vecc2(v3DCoords) );
				else if (stateCurrent == STATE_DESTINATION_DRAWING_INPUT)
					liDestination.push_back( Vecc2(v3DCoords) );

				ptPrevPoint = Vecc2(x, y);
			}
		}
		else if (stateCurrent == STATE_POINT_DRAG)
		{
			m_ptrDraggedPoint->X = v3DCoords.X;
			m_ptrDraggedPoint->Y = v3DCoords.Y;

			// both curves are done (condition of mouse down)
			if (liDestination.size() == liSource.size())
			{
				SaveMorphingLinesIntoAnimationSequence();
				UploadMorphingLines();
			}
		}
	}
}


void MorphingToolSubWindow::ClearSourceLine()
{
	liSource.clear();

	bSrcCurveIsDone = false;
	stateCurrent = STATE_IDLE;

	buttonSource->_text = "Draw src";
}

void MorphingToolSubWindow::ClearDestinationLine()
{
	liDestination.clear();

	bDstCurveIsDone = false;
	stateCurrent = STATE_IDLE;

	buttonDestination->_text = "Draw dst";
}


bool MorphingToolSubWindow::ResetView()
{
	OpenGLSubWindow::ResetView();

	return true;
}

bool MorphingToolSubWindow::ClearMorph()
{
	ClearSourceLine();
	ClearDestinationLine();

	UploadMorphingLines();

	return true;
}


bool MorphingToolSubWindow::KeyboardFunc(unsigned char key, int x, int y)
{
	bool res = false;

	if (OpenGLSubWindow::KeyboardFunc(key, x, y)) return true;

	switch (key)
	{
	case '4':
	{
		StartNextGeneration();
		break;
	}
	break;
	}

	return res;
}


void MorphingToolSubWindow::ReDrawFBOprocessors()
{
	morphFBOprocessor->fMorphRadius   = m_ParamsSubWindow->MorphRadius();
	morphFBOprocessor->fMorphPower    = m_ParamsSubWindow->MorphPower();
	morphFBOprocessor->fMorphRatio    = m_ParamsSubWindow->MorphRatio();
	morphFBOprocessor->bShowWireframe = GlobalParamsSubWindow::Get()->IsWireframeShown();

	morphFBOprocessor->Render();
}


bool MorphingToolSubWindow::SourcePolylineClicked()
{
	// Sequence here matters
	if (bSrcCurveIsDone)
	{
		ClearSourceLine();
	}
	else
	if (stateCurrent == STATE_IDLE)
	{
		stateCurrent = STATE_SOURCE_DRAWING_INPUT;

		glutSetCursor(GLUT_CURSOR_TOP_SIDE);

		buttonSource->bEnabled = false;
		buttonSource->_text = "Drawing";

		// nice touch
		GlobalParamsSubWindow::Get()->MakePointsVisible();
	}

	return true;
}


bool MorphingToolSubWindow::DestinationPolylineClicked()
{
	// Sequence here matters
	if (bDstCurveIsDone)
	{
		ClearDestinationLine();
	}
	else
	if (stateCurrent == STATE_IDLE)
	{
		stateCurrent = STATE_DESTINATION_DRAWING_INPUT;

		glutSetCursor(GLUT_CURSOR_TOP_SIDE);

		buttonDestination->_text = "Drawing";

		// nice touch
		GlobalParamsSubWindow::Get()->MakePointsVisible();
	}

	return true;
}

bool MorphingToolSubWindow::MorphNow()
{
	if (liSource.size() == liDestination.size())
	{
		SaveMorphingLinesIntoAnimationSequence();
		UploadMorphingLines();
	}

	return true;
}


void MorphingToolSubWindow::StartNextGeneration()
{
	ClearSourceLine();
	ClearDestinationLine();
	     
	// load empty lines
	UploadMorphingLines();

	unsigned int idSrc      = texBank[TEXTURE_MORPHED_IMAGE]->m_uiTextureID;
	unsigned int iWidthSrc  = texBank[TEXTURE_MORPHED_IMAGE]->m_width;
	unsigned int iHeightSrc = texBank[TEXTURE_MORPHED_IMAGE]->m_height;

	unsigned int idDst      = texBank[TEXTURE_INPUT_IMAGE]->m_uiTextureID;
	unsigned int iWidthDst  = texBank[TEXTURE_INPUT_IMAGE]->m_width;
	unsigned int iHeightDst = texBank[TEXTURE_INPUT_IMAGE]->m_height;
	unsigned int nrChannels = 4;

	assert(iWidthSrc == iWidthDst);
	assert(iHeightSrc == iHeightDst);

	unsigned char* data = (unsigned char *)malloc(iWidthSrc*iHeightSrc*nrChannels);

		glBindTexture(GL_TEXTURE_2D, idSrc);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		glBindTexture(GL_TEXTURE_2D, idDst);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, iWidthDst, iHeightDst, GL_RGBA, GL_UNSIGNED_BYTE, data);

	free(data);
}

double MorphingToolSubWindow::GetClipLocalTimeS()
{
	TrackClip* clip = TrackClip::GetClip(this);
	if (!clip) return 0;	// eg welcome screen

	int iPlayhead10msTicks = PositionMediator::Get()->Pos10msUnits();

	return (iPlayhead10msTicks - clip->m_iStartPos10msUnits)/100.0;
}

