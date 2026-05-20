#include "stdafx.h"
#include "WarpingToolSubWindow.h"
#include "../../!!adGlobals/adOpenGLUtilities.h"
#include <vector>
#include <functional>

const float const_fPointsDepth   = 3;
const float const_fPointsSize    = 12;
const float const_fHandleRadius  = 5;

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

	m_iJoystickFrameWidth  = fbo->Width();
	m_iJoystickFrameHeight = fbo->Height();

	liScalingHandles.push_back( Vecc2(  m_iJoystickFrameWidth/2.0,  m_iJoystickFrameHeight/2.0));
	liScalingHandles.push_back( Vecc2(  m_iJoystickFrameWidth/2.0, -m_iJoystickFrameHeight/2.0));
	liScalingHandles.push_back( Vecc2( -m_iJoystickFrameWidth/2.0, -m_iJoystickFrameHeight/2.0));
	liScalingHandles.push_back( Vecc2( -m_iJoystickFrameWidth/2.0,  m_iJoystickFrameHeight/2.0));

	ptTranslHandle = Vecc2();
	ptRotateHandle = Vecc2(m_iJoystickFrameWidth/2.0*0.618, 0);

	matrImmediateVisualization     = Mat4MakeIdent();
	matrObjectOrigin2joystickBasis = Mat4MakeIdent();

	hCurSpecial     = LoadCursorFromFileW(L"aero_moveProp.cur");

}

void WarpingToolSubWindow::SetupGraphicsPipeline()
{
	MorphingToolSubWindow::SetupGraphicsPipeline();

	// the last transformation is previewed until the mouse button is released using this matrix
	// When the button is released matrObjectOrigin2joystickBasis is recomputed and this matrix is reset to identity
	glMultMatrixf(&matrImmediateVisualization.m[0][0]);

	// basis 1,1,1 is transformed to joystick basis on every mouseup
	glMultMatrixf(&matrObjectOrigin2joystickBasis.m[0][0]);
}


void WarpingToolSubWindow::Draw()
{
	
	// the last transformation is previewed until the mouse button is released using this matrix
	// When the button is released matrObjectOrigin2joystickBasis is recomputed and this matrix is reset to identity
	glMultMatrixf(&matrImmediateVisualization.m[0][0]);

	// basis 1,1,1 is transformed to joystick basis on every mouseup
	glMultMatrixf(&matrObjectOrigin2joystickBasis.m[0][0]);
	
	// draw siblings' object
	for (auto sibling : m_liSiblings)
	{
		if (sibling == this) continue;

		sibling->DrawFBOquad();
	}

	// draw object
	MorphingToolSubWindow::Draw();

	// here we call parents SetupGraphicsPipeline for the reason described below 
	MorphingToolSubWindow::SetupGraphicsPipeline();

		// Apply only immediate matrix and do not apply object2joysticks, because here we draw only joysticks
		// and joysticks become physically transformed
		glMultMatrixf(&matrImmediateVisualization.m[0][0]);

		glEnable(GL_LINE_STIPPLE);
		glLineStipple(2, 0xAAAA);
			glLineWidth(3);
			glColor3f(0, 0.69, 0);

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
		glColor3f(0.93, 0.8, 0);
		glBegin(GL_POINTS);
			for (auto& element : liScalingHandles) {
				glVertex3f(element.X, element.Y, const_fPointsDepth);
			}
		glEnd();

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
						SetCursor(hCurSpecial);
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
					glutSetCursor(GLUT_CURSOR_INFO);
					return true;
				}
			}

		}
	}
	return false;
}


bool WarpingToolSubWindow::MouseFunc(int button, int state, int x, int y)
{
	if (MorphingToolSubWindow::MouseFunc(button, state, x, y)) return true;

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
						SetCursor(hCurSpecial);
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
				stateTransform = STATE_TRANS_ROTATE;

				glutSetCursor(GLUT_CURSOR_INFO);

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

		matrObjectOrigin2joystickBasis = Mat4MakeTransformFromVectors(Vecc3(),
																	  Vecc3(m_iJoystickFrameWidth, 0),
			                                                          Vecc3(0, m_iJoystickFrameHeight),
																	  Vecc3(ptTranslHandle),
																	  Vecc3(m_ptHandlePivotRight - m_ptHandlePivot),
																	  Vecc3(m_ptHandlePivotUp    - m_ptHandlePivot));
	}

	return false;
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
			if (PointDist(Vecc2(x, y), ptPrevPoint) > 3)
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

				matrTrans = Mat4MakeTrans(m_ptHandlePivot.X, m_ptHandlePivot.Y, 0.0)*matrTrans;

				matrImmediateVisualization = matrTrans;
			}
		}
		else
		if (stateTransform == STATE_TRANS_SCALE_NONPROPORTIONAL)
		{
			if (PointDist(Vecc2(x, y), ptPrevPoint) > 3)
			{
				Matr4 matrTrans = Mat4MakeTrans(-m_ptHandlePivot.X, -m_ptHandlePivot.Y, 0.0);

				// Prepare a local coordinate system of a current rectangle
				Vec3 vPivotUp    = Vecc3(m_ptHandlePivotUp    - m_ptHandlePivot);
				Vec3 vPivotRight = Vecc3(m_ptHandlePivotRight - m_ptHandlePivot);

				// Project v3DCoords on rays starting at m_ptHandlePivot
				Vec3 xProj = PointLineProject(Vecc3(v3DCoords), Vecc3(m_ptHandlePivot), VecNormalize(vPivotRight)    );
				Vec3 yProj = PointLineProject(Vecc3(v3DCoords), Vecc3(m_ptHandlePivot), VecNormalize(vPivotUp) );

				// Calulcate matrix that transforms existing local coordinate system into new coordinate system
				matrTrans = Mat4MakeTransformFromVectors(vPivotUp, vPivotRight,
														 yProj - Vecc3(m_ptHandlePivot), xProj - Vecc3(m_ptHandlePivot))*matrTrans;

				matrTrans = Mat4MakeTrans(m_ptHandlePivot.X, m_ptHandlePivot.Y, 0.0)*matrTrans;

				matrImmediateVisualization = matrTrans;
			}
		}


		if (stateTransform == STATE_TRANS_TRANSLATE)
		{
			if (PointDist(Vecc2(x, y), ptPrevPoint) > 3) // todo 3 userscale
			{
				Matr4 matrTrans = Mat4MakeTrans( v3DCoords.X - m_ptHandleClicked.X, 
												 v3DCoords.Y - m_ptHandleClicked.Y,
												 1.0f);

				matrImmediateVisualization = matrTrans;
			}
		}

		if (stateTransform == STATE_TRANS_ROTATE)
		{
			if (PointDist(Vecc2(x, y), ptPrevPoint) > 3)
			{
				Vec2 vec1 = m_ptHandleClicked - m_ptHandlePivot;
				Vec2 vec2 = Vecc2(v3DCoords)  - m_ptHandlePivot;
				float angle = AngleBetweenVectorsCCW(vec1, vec2)*RADIANS_TO_DEGREES;

				Matr4 matrRot = Mat4MakeTrans(-m_ptHandlePivot.X, -m_ptHandlePivot.Y, 0.0);

				      matrRot = Mat4MakeRot(angle, Vecc3(0,0, 1.0) )*matrRot;

					  matrRot = Mat4MakeTrans(m_ptHandlePivot.X, m_ptHandlePivot.Y, 0.0)*matrRot;

				matrImmediateVisualization = matrRot;
			}
		}
	}
}


bool WarpingToolSubWindow::KeyboardFunc(unsigned char key, int x, int y)
{
	return false;
}

void WarpingToolSubWindow::KeyboardAux(int key, int state, int x, int y)
{
	if (key == GLUT_ACTIVE_SHIFT)
	{
		PassiveMotionFunc(x, y);
	}
}