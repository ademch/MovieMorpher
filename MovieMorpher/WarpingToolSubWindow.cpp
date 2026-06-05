#include "stdafx.h"
#include "WarpingToolSubWindow.h"
#include "GlobalParamsSubWindow.h"
#include "../../!!adGlobals/adOpenGLUtilities.h"
#include "../../!!adGUI/VideoPositionMediator.h"
#include "../../!!adGUI/TrackClip.h"
#include "GLSL_Pipeline.h"
#include <algorithm>


const float const_fPointsDepth   = 3;
const float const_fPointsSize    = 10;
const float const_fHandleRadius  = 5;
const float const_fHandleJitter  = 3;

#define TRANS_PIVOTRIGHT	1
#define TRANS_PIVOT			2
#define TRANS_PIVOTUP		3


std::vector<WarpingToolSubWindow*> WarpingToolSubWindow::m_liSiblings;


WarpingToolSubWindow::WarpingToolSubWindow(int iParentWidth, int iParentHeight,
										   float fBottomLeftXperc, float fBottomLeftYperc,
										   float fWidthPerc, float fHeightPerc) :
					  MorphingToolSubWindow(iParentWidth, iParentHeight,
						                    fBottomLeftXperc, fBottomLeftYperc, fWidthPerc, fHeightPerc)

{
	stateTransform = STATE_TRANS_IDLE;

	m_liSiblings.push_back(this);

	m_iJoystickFrameWidth  = morphFBOprocessor->Width();
	m_iJoystickFrameHeight = morphFBOprocessor->Height();

	liScalingHandles.push_back( Vecc2(  m_iJoystickFrameWidth/2.0,  m_iJoystickFrameHeight/2.0 ) );
	liScalingHandles.push_back( Vecc2(  m_iJoystickFrameWidth/2.0, -m_iJoystickFrameHeight/2.0 ) );
	liScalingHandles.push_back( Vecc2( -m_iJoystickFrameWidth/2.0, -m_iJoystickFrameHeight/2.0 ) );
	liScalingHandles.push_back( Vecc2( -m_iJoystickFrameWidth/2.0,  m_iJoystickFrameHeight/2.0 ) );

	ptTranslHandle = Vecc2();
	ptRotateHandle = Vecc2(m_iJoystickFrameWidth/2.0*0.618, 0);

	matrImmediateVisualization     = Mat4MakeIdent();
	matrObjectOrigin2joystickBasis = Mat4MakeIdent();

	hCursorScaleProportional = LoadCursorFromFileW(L"Cursors\\aero_moveProp.cur");
	hCursorRotateAngle		 = LoadCursorFromFileW(L"Cursors\\aero_rotateAngle.cur");

}

void WarpingToolSubWindow::SetupGraphicsPipeline()
{
	// Call SetupGraphicsPipeline of a parent to observe basic user pan/zoom
	MorphingToolSubWindow::SetupGraphicsPipeline();

	// the last transformation is previewed until the mouse button is released using this matrix
	// When the button is released matrObjectOrigin2joystickBasis is recomputed and this matrix is reset to identity
	glMultMatrixf(&matrImmediateVisualization.m[0][0]);

	// basis 1,1,1 is transformed to joystick basis on every mouseup
	glMultMatrixf(&matrObjectOrigin2joystickBasis.m[0][0]);
}


void WarpingToolSubWindow::DrawFBOquad()
{
	// As soon as siblings are drawn from single place, each quad has to have full transformation stack prepared
	// We call here SetupGraphicsPipeline of WarpingToolSubWindow to apply warping transformations also
	SetupGraphicsPipeline();

		float fAlpha = m_ParamsSubWindow->fTransparency/100.0f;

		TextureDescriptor* texDescr;
		// Show output of the shader, while invisible input texture holds original
		if (GlobalParamsSubWindow::Get()->ShouldShowOriginal())
			texDescr = texBank[TEXTURE_INPUT_IMAGE];
		else
			texDescr = texBank[TEXTURE_MORPHED_IMAGE];

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBindTexture(GL_TEXTURE_2D, texDescr->m_uiTextureID);

			// override sampler default settings (set in fbo->Init)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			//std::string sSelected = comboBox->GetSelected();
			glColor4f(1.0, 1.0, 1.0, fAlpha);
			RenderTexturedQuadTransparent( texDescr->m_uiTextureID,	// texture
										  -texDescr->m_width/2,		// bottomX
										  -texDescr->m_height/2,	// bottomY
										   texDescr->m_width,		// width
										   texDescr->m_height,		// z
										  -110 + 10*zOrder );		// 0-> -110, 1-> -100, 2-> -90, 3-> -80

		glDisable(GL_BLEND);

}


void WarpingToolSubWindow::Draw()
{
	// sort features from farthest (track 0) to closest (track 5)
	std::sort(	m_liSiblings.begin(), m_liSiblings.end(),
				[](const auto& a, const auto& b)
				{
					return a->zOrder < b->zOrder;
				});

	bool bPlayheadWithinThisClip = false;

	if (m_liSiblings.size() == 1)
	{
		// welcome screen window
		m_liSiblings[0]->DrawFBOquad();
		
		bPlayheadWithinThisClip = true;
	}
	else
	{
		int iPlayhead10msTicks = PositionMediator::Get()->Pos10msUnits();

		// draw sibling fbos (do not draw the first window as that is welcome screen window)
		for (unsigned int i=1; i < m_liSiblings.size(); i++ )
		{
			//if (m_liSiblings[i] == this) continue;

			TrackClip* clip = TrackClip::GetClip(m_liSiblings[i]);
			if ( (iPlayhead10msTicks >= clip->m_iStartPos10msUnits) &&
				 (iPlayhead10msTicks <  clip->m_iStartPos10msUnits + clip->m_iLength10msUnits) )
			{
				//TrackTranspSubWindow::GetTrackTransp(clip->iTrack-1)/100.0f;
				m_liSiblings[i]->DrawFBOquad();

				// analyze this tool window
				if (m_liSiblings[i] == this) bPlayheadWithinThisClip |= true;
			}
		}
	}

	// IF THIS TOOL'S CLIP IS OUTSIDE PLAYHEAD WE DO NOT DRAW ITS CONTROL POINTS
	if (!bPlayheadWithinThisClip) return;

	// draw object
	MorphingToolSubWindow::Draw();

	// here we call parents SetupGraphicsPipeline for the reason described below 
	MorphingToolSubWindow::SetupGraphicsPipeline();

		// Apply only immediate matrix and do not apply object2joysticks, because here we draw only joysticks
		// and joysticks become physically transformed
		glMultMatrixf(&matrImmediateVisualization.m[0][0]);

		if (GlobalParamsSubWindow::Get()->PointsAreVisible())
		{
			glEnable(GL_LINE_STIPPLE);
			glLineStipple(2, 0xAAAA);
				glLineWidth(2);
				glColor3f(0, 0.69, 0);

				// frame
				glBegin(GL_LINE_LOOP);
				for (auto& element : liScalingHandles) {
					glVertex3f(element.X, element.Y, 2);
				}
				glEnd();

				// Line from the center to rotation handle
				glBegin(GL_LINES);
					glVertex3f(ptTranslHandle.X, ptTranslHandle.Y, const_fPointsDepth);
					glVertex3f(ptRotateHandle.X, ptRotateHandle.Y, const_fPointsDepth);
				glEnd();

				if (stateTransform == STATE_TRANS_SCALE_PROPORTIONAL)
				{
					glLineWidth(1);

					Vec2 vDirLine = VecNormalize( m_ptHandleClicked - m_ptHandlePivot );
					Vec2 ptOnLine = m_ptHandlePivot + 10000.0*vDirLine;
					glBegin(GL_LINES);
						glVertex3f(m_ptHandlePivot.X,   m_ptHandlePivot.Y,   const_fPointsDepth);
						glVertex3f(ptOnLine.X,          ptOnLine.Y,          const_fPointsDepth);
					glEnd();
				}
			glDisable(GL_LINE_STIPPLE);

			// Draw handles of a rectangle
			glPointSize(const_fPointsSize);
			glColor3f(0.9, 0.0, 0.0);
			glBegin(GL_POINTS);
				for (auto& element : liScalingHandles) {
					glVertex3f(element.X, element.Y, const_fPointsDepth);
				}
			glEnd();

			// Translation and rotation handles
			glLineWidth(1);
			DrawCircle(Vecc3(ptTranslHandle, 3), 50, 40);
			glEnable(GL_POINT_SMOOTH);
				glPointSize(const_fPointsSize);
				glBegin(GL_POINTS);
					glVertex3f(ptTranslHandle.X, ptTranslHandle.Y, const_fPointsDepth);
				glEnd();

				glPointSize(const_fPointsSize);
				glBegin(GL_POINTS);
					glVertex3f(ptRotateHandle.X, ptRotateHandle.Y, const_fPointsDepth);
				glEnd();

			glDisable(GL_POINT_SMOOTH);
		}

}

bool WarpingToolSubWindow::PassiveMotionFunc(int x, int y)
{
	Vec3d v3DCoords;

	if (MorphingToolSubWindow::PassiveMotionFunc(x, y)) return true;

	if ((x > m_iBottomLeftX) && (x < m_iBottomLeftX + m_iWidth) &&
		(y > m_iBottomLeftY) && (y < m_iBottomLeftY + m_iHeight))
	{
		if (stateTransform == STATE_TRANS_IDLE)
		{
			MorphingToolSubWindow::SetupGraphicsPipeline();

			gluUnProjectFriendly(x, y, 0, v3DCoords.X, v3DCoords.Y, v3DCoords.Z);

			for (auto& point : liScalingHandles)
			{
				Vec3 dxdydz = Vecc3(v3DCoords) - Vecc3(point.X, point.Y, const_fPointsDepth);
				if ( VecLengthSqr(dxdydz) < sqr(const_fHandleRadius/fUserScale) )
				{
					if (GetKeyState(VK_SHIFT) & 0x8000)
						glutSetCursor(GLUT_CURSOR_INFO);
					else
					{
						glutSetCursor(GLUT_CURSOR_NONE);
						SetCursor(hCursorScaleProportional);
					}
					
					return true;
				}
			}

			Vec2& ptTrans = ptTranslHandle;
			{
				Vec3 dxdydz = Vecc3(v3DCoords) - Vecc3(ptTrans.X, ptTrans.Y, const_fPointsDepth);
				if ( VecLengthSqr(dxdydz) < sqr(const_fHandleRadius/fUserScale) )
				{
					glutSetCursor(GLUT_CURSOR_INFO);
					return true;
				}
			}

			Vec2& ptRotate = ptRotateHandle;
			{
				Vec3 dxdydz = Vecc3(v3DCoords) - Vecc3(ptRotate.X, ptRotate.Y, const_fPointsDepth);
				if ( VecLengthSqr(dxdydz) < sqr(const_fHandleRadius/fUserScale) )
				{
					if (GetKeyState(VK_SHIFT) & 0x8000)
					{
						glutSetCursor(GLUT_CURSOR_NONE);
						SetCursor(hCursorRotateAngle);
					}
					else
					{
						glutSetCursor(GLUT_CURSOR_INFO);
					}
					return true;
				}
			}

		}
	}
	return false;
}


bool WarpingToolSubWindow::MouseFunc(int button, int state, int x, int y)
{
	if ((x > m_iBottomLeftX) && (x < m_iBottomLeftX + m_iWidth) &&
		(y > m_iBottomLeftY) && (y < m_iBottomLeftY + m_iHeight))
	{

		MorphingToolSubWindow::SetupGraphicsPipeline();

			Vec3d v3DCoords;
			gluUnProjectFriendly(x, y, 0, v3DCoords.X, v3DCoords.Y, v3DCoords.Z);

			if ((stateTransform == STATE_TRANS_IDLE) && (button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
			{
				// SCALING
				for (size_t i = 0; i < liScalingHandles.size(); i++)
				{
					auto& point = liScalingHandles[i];

					Vec3 dxdydz = Vecc3(v3DCoords) - Vecc3(point.X, point.Y, const_fPointsDepth);
					if ( VecLengthSqr(dxdydz) < sqr(const_fHandleRadius/fUserScale) )
					{
						if (GetKeyState(VK_SHIFT) & 0x8000)
						{
							stateTransform = STATE_TRANS_SCALE_NONPROPORTIONAL;

							glutSetCursor(GLUT_CURSOR_INFO);
						}
						else
						{
							stateTransform = STATE_TRANS_SCALE_PROPORTIONAL;

							glutSetCursor(GLUT_CURSOR_NONE);
							SetCursor(hCursorScaleProportional);
						}

						m_ptHandleClicked = point;

						// find the opposite corner
						size_t iCorner = (i + 2) % liScalingHandles.size();
						m_ptHandlePivot = liScalingHandles[iCorner];

						iCorner = (i + 3) % liScalingHandles.size();
						m_ptHandlePivotUp = liScalingHandles[iCorner];

						iCorner = (i + 1) % liScalingHandles.size();
						m_ptHandlePivotRight = liScalingHandles[iCorner];

						ptPrevPoint = Vecc2(x, y);

						return true;
					}
				}

				// TRANSLATION
				auto& ptTranslate = ptTranslHandle;

				Vec3 dxdydz;
				dxdydz = Vecc3(v3DCoords) - Vecc3(ptTranslate.X, ptTranslate.Y, const_fPointsDepth);
				if ( VecLengthSqr(dxdydz) < sqr(const_fHandleRadius/fUserScale) )
				{
					stateTransform = STATE_TRANS_TRANSLATE;

					glutSetCursor(GLUT_CURSOR_INFO);

					m_ptHandleClicked = ptTranslate;

					ptPrevPoint = Vecc2(x, y);

					return true;
				}

				// ROTATION
				auto& ptRotate = ptRotateHandle;

				dxdydz = Vecc3(v3DCoords) - Vecc3(ptRotate.X, ptRotate.Y, const_fPointsDepth);
				if ( VecLengthSqr(dxdydz) < sqr(const_fHandleRadius/fUserScale) )
				{
					//stateTransform = STATE_TRANS_ROTATE;

					//glutSetCursor(GLUT_CURSOR_INFO);

					if (GetKeyState(VK_SHIFT) & 0x8000)
					{
						stateTransform = STATE_TRANS_ROTATE_W_STEPS;

						glutSetCursor(GLUT_CURSOR_NONE);
						SetCursor(hCursorRotateAngle);
					}
					else
					{
						stateTransform = STATE_TRANS_ROTATE;

						glutSetCursor(GLUT_CURSOR_INFO);
					}

					m_ptHandleClicked = ptRotate;
					m_ptHandlePivot   = ptTranslHandle;

					ptPrevPoint = Vecc2(x, y);

					return true;
				}
			}
	}
	
	if ((stateTransform != STATE_TRANS_IDLE) && (button == GLUT_LEFT_BUTTON) && (state == GLUT_UP))
	{
		stateTransform = STATE_TRANS_IDLE;

		// Transform all handles
		{
			for (auto& element : liScalingHandles) {
				element = Vecc2( matrImmediateVisualization*Vecc3(element) );
			}

			ptTranslHandle = Vecc2( matrImmediateVisualization*Vecc3(ptTranslHandle) );
			ptRotateHandle = Vecc2( matrImmediateVisualization*Vecc3(ptRotateHandle) );
		}

		matrImmediateVisualization = Mat4MakeIdent();

		m_ptHandlePivot			= liScalingHandles[TRANS_PIVOT];
		m_ptHandlePivotUp		= liScalingHandles[TRANS_PIVOTUP];
		m_ptHandlePivotRight	= liScalingHandles[TRANS_PIVOTRIGHT];

		matrObjectOrigin2joystickBasis = Mat4MakeTransformFromVectors(Vecc3(),											// origin
																	  Vecc3(m_iJoystickFrameWidth, 0),
			                                                          Vecc3(0, m_iJoystickFrameHeight),
																	  Vecc3(ptTranslHandle),							// new origin
																	  Vecc3(m_ptHandlePivotRight - m_ptHandlePivot),
																	  Vecc3(m_ptHandlePivotUp    - m_ptHandlePivot));
		// non-normalized 2D transformation produces non identity Z
		matrObjectOrigin2joystickBasis.m[2][2] = 1.0;
		matrObjectOrigin2joystickBasis.m[3][3] = 1.0;
	}

	return MorphingToolSubWindow::MouseFunc(button, state, x, y);
}


void WarpingToolSubWindow::MotionFunc(int x, int y)
{
	MorphingToolSubWindow::MotionFunc(x, y);

	if ((x > m_iBottomLeftX) && (x < m_iBottomLeftX + m_iWidth) &&
		(y > m_iBottomLeftY) && (y < m_iBottomLeftY + m_iHeight))
	{
		MorphingToolSubWindow::SetupGraphicsPipeline();

		Vec3d v3DCoords;
		gluUnProjectFriendly(x, y, 0, v3DCoords.X, v3DCoords.Y, v3DCoords.Z);

		if (stateTransform == STATE_TRANS_SCALE_PROPORTIONAL)
		{
			if (PointDist(Vecc2(x, y), ptPrevPoint) > const_fHandleJitter)
			{
				Matr4 matrTrans = Mat4MakeTrans(-m_ptHandlePivot.X, -m_ptHandlePivot.Y, 0.0);

				// Prepare a local coordinate system of a current rectangle
				Vec3 vPivotUp    = Vecc3(m_ptHandlePivotUp    - m_ptHandlePivot);
				Vec3 vPivotRight = Vecc3(m_ptHandlePivotRight - m_ptHandlePivot);

				// Project v3DCoords on rays starting at m_ptHandlePivot
				float fDotX, fDotY;
				Vec3 xProj = PointLineProject(Vecc3(v3DCoords), Vecc3(m_ptHandlePivot), VecNormalize(vPivotRight), fDotX);
				Vec3 yProj = PointLineProject(Vecc3(v3DCoords), Vecc3(m_ptHandlePivot), VecNormalize(vPivotUp),    fDotY);

				float fAspect = VecLength(vPivotRight) / VecLength(vPivotUp);

				if (fDotX/fDotY < fAspect)
					yProj = Vecc3(m_ptHandlePivot) + fDotX/fAspect*VecNormalize(vPivotUp);
				else
					xProj = Vecc3(m_ptHandlePivot) + fDotY*fAspect*VecNormalize(vPivotRight);

				// Calulcate matrix that transforms existing local coordinate system into new coordinate system
				matrTrans = Mat4MakeTransformFromVectors(vPivotUp, vPivotRight,
														 yProj - Vecc3(m_ptHandlePivot), xProj - Vecc3(m_ptHandlePivot))*matrTrans;
				// non-normalized 2D transformation produces non identity Z
				matrTrans.m[2][2] = 1.0;
				matrTrans.m[3][3] = 1.0;

				matrTrans = Mat4MakeTrans(m_ptHandlePivot.X, m_ptHandlePivot.Y, 0.0)*matrTrans;

				matrImmediateVisualization = matrTrans;
			}
		}
		else
		if (stateTransform == STATE_TRANS_SCALE_NONPROPORTIONAL)
		{
			if (PointDist(Vecc2(x, y), ptPrevPoint) > const_fHandleJitter)
			{
				Matr4 matrTrans = Mat4MakeTrans(-m_ptHandlePivot.X, -m_ptHandlePivot.Y, 0.0);

				// Prepare a local coordinate system of a current rectangle
				Vec3 vPivotUp    = Vecc3(m_ptHandlePivotUp    - m_ptHandlePivot);
				Vec3 vPivotRight = Vecc3(m_ptHandlePivotRight - m_ptHandlePivot);

				// Project v3DCoords on rays starting at m_ptHandlePivot
				Vec3 xProj = PointLineProject(Vecc3(v3DCoords), Vecc3(m_ptHandlePivot), VecNormalize(vPivotRight) );
				Vec3 yProj = PointLineProject(Vecc3(v3DCoords), Vecc3(m_ptHandlePivot), VecNormalize(vPivotUp) );

				// Calulcate matrix that transforms existing local coordinate system into new coordinate system
				matrTrans = Mat4MakeTransformFromVectors(vPivotUp, vPivotRight,
														 yProj - Vecc3(m_ptHandlePivot), xProj - Vecc3(m_ptHandlePivot))*matrTrans;
				// non-normalized 2D transformation produces non identity Z
				matrTrans.m[2][2] = 1.0;
				matrTrans.m[3][3] = 1.0;

				matrTrans = Mat4MakeTrans(m_ptHandlePivot.X, m_ptHandlePivot.Y, 0.0)*matrTrans;

				matrImmediateVisualization = matrTrans;
			}
		}


		if (stateTransform == STATE_TRANS_TRANSLATE)
		{
			if (PointDist(Vecc2(x, y), ptPrevPoint) > const_fHandleJitter)
			{
				Matr4 matrTrans = Mat4MakeTrans( v3DCoords.X - m_ptHandleClicked.X, 
												 v3DCoords.Y - m_ptHandleClicked.Y,
												 1.0f);

				matrImmediateVisualization = matrTrans;
			}
		}

		if ((stateTransform == STATE_TRANS_ROTATE) || (stateTransform == STATE_TRANS_ROTATE_W_STEPS))
		{
			if (PointDist(Vecc2(x, y), ptPrevPoint) > const_fHandleJitter)
			{
				Vec2 vec1 = m_ptHandleClicked - m_ptHandlePivot;
				Vec2 vec2 = Vecc2(v3DCoords)  - m_ptHandlePivot;
				float angle = AngleBetweenVectorsCCW(vec1, vec2)*RADIANS_TO_DEGREES;

				if (stateTransform == STATE_TRANS_ROTATE_W_STEPS)
					angle = floorf((angle + 7.5f) / 15.0f) * 15.0f;

				Matr4 matrRot = Mat4MakeTrans(-m_ptHandlePivot.X, -m_ptHandlePivot.Y, 0.0);

				      matrRot = Mat4MakeRot(angle, Vecc3(0,0, 1.0) )*matrRot;

					  matrRot = Mat4MakeTrans(m_ptHandlePivot.X, m_ptHandlePivot.Y, 0.0)*matrRot;

				matrImmediateVisualization = matrRot;
			}
		}
	}
}


void WarpingToolSubWindow::KeyboardAux(int key, int state, int x, int y)
{
	if (key == VK_SHIFT)
	{
		PassiveMotionFunc(x, y);
	}
}


void WarpingToolSubWindow::RemoveSibling(OpenGLSubWindowWithGUI* _sibling)
{
	for (auto it = m_liSiblings.begin(); it != m_liSiblings.end(); ++it)
	{
		if ((*it) == _sibling)
		{
			//delete *it;					// m_liSiblings does not own the pointer
			it = m_liSiblings.erase(it);	// erase returns next iterator
			break;
		}
	}
}