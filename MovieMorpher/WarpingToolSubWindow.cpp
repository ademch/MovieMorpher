#include "stdafx.h"
#include "WarpingToolSubWindow.h"
#include "../../!!adGlobals/adOpenGLUtilities.h"
#include <vector>
#include <functional>

const float const_fPointsDepth = 3;
const float const_fPointsSize = 7;
const float const_fHandleRadius   = 5;

WarpingToolSubWindow::WarpingToolSubWindow(int iParentWidth, int iParentHeight,
										   float fBottomLeftXperc, float fBottomLeftYperc,
										   float fWidthPerc, float fHeightPerc) :
					  MorphingToolSubWindow(iParentWidth, iParentHeight,
						                    fBottomLeftXperc, fBottomLeftYperc, fWidthPerc, fHeightPerc)

{
	stateTransform = STATE_TRANS_IDLE;

	liScalingHandles.push_back(Vecc2( 200,  200));
	liScalingHandles.push_back(Vecc2( 200, -200));
	liScalingHandles.push_back(Vecc2(-200, -200));
	liScalingHandles.push_back(Vecc2(-200,  200));

	ptTranslateHandle = Vecc2();
	ptRotateHandle = Vecc2(100, 0);
}


void WarpingToolSubWindow::Draw()
{
	MorphingToolSubWindow::Draw();

	for (auto& matr : liMatrixConveyor)
		glMultMatrixf(&matr.m.m[0][0]);

	glEnable(GL_LINE_STIPPLE);
	glLineStipple(2, 0xAAAA);
		glLineWidth(2);
		glColor3f(0, 0.69, 0);

		glBegin(GL_LINE_LOOP);
		for (auto element : liScalingHandles) {
			glVertex3f(element.X, element.Y, 2);
		}
		glEnd();

		glBegin(GL_LINES);
			glVertex3f(ptTranslateHandle.X, ptTranslateHandle.Y, const_fPointsDepth);
			glVertex3f(ptRotateHandle.X,    ptRotateHandle.Y,    const_fPointsDepth);
		glEnd();
	glDisable(GL_LINE_STIPPLE);

	glPointSize(9);
	glColor3f(0.93, 0.8, 0);
	glBegin(GL_POINTS);
		for (auto element : liScalingHandles) {
			glVertex3f(element.X, element.Y, const_fPointsDepth);
		}
	glEnd();

	glLineWidth(1);
	DrawCircle(Vecc3(ptTranslateHandle, 3), 50, 40);
	glEnable(GL_POINT_SMOOTH);
		glPointSize(10);
		glBegin(GL_POINTS);
			glVertex3f(ptTranslateHandle.X, ptTranslateHandle.Y, const_fPointsDepth);
		glEnd();

		glPointSize(10);
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
			SetupGraphicsPipeline();

			Vec3 vHandle = Vecc3(1.0, 1.0, 1.0);
			for (auto& matr : liMatrixConveyor)
			{
				vHandle = matr.m^vHandle;
				glMultMatrixf(&matr.m.m[0][0]);
			}

			gluUnProjectFriendly(x, y, 0, v3DCoords.X, v3DCoords.Y, v3DCoords.Z);

			for (auto& point : liScalingHandles)
			{
				Vec3 dxdydz = Vecc3(v3DCoords) - Vecc3(point.X, point.Y, const_fPointsDepth);

				if (( sqr(dxdydz.X) < sqr(const_fHandleRadius/(fUserScale*vHandle.X)) ) &&
					( sqr(dxdydz.Y) < sqr(const_fHandleRadius/(fUserScale*vHandle.Y)) ) && 
					( sqr(dxdydz.Z) < sqr(const_fHandleRadius/(fUserScale*vHandle.Z)) ))
				{
					glutSetCursor(GLUT_CURSOR_INFO);
					return true;
				}
			}

			Vec2& ptTrans = ptTranslateHandle;
			{
				Vec3 dxdydz = Vecc3(v3DCoords) - Vecc3(ptTrans.X, ptTrans.Y, const_fPointsDepth);

				if (( sqr(dxdydz.X) < sqr(const_fHandleRadius/(fUserScale*vHandle.X)) ) &&
					( sqr(dxdydz.Y) < sqr(const_fHandleRadius/(fUserScale*vHandle.Y)) ) && 
					( sqr(dxdydz.Z) < sqr(const_fHandleRadius/(fUserScale*vHandle.Z)) ))
				{
					glutSetCursor(GLUT_CURSOR_INFO);
					return true;
				}
			}

			Vec2& ptRotate = ptRotateHandle;
			{
				Vec3 dxdydz = Vecc3(v3DCoords) - Vecc3(ptRotate.X, ptRotate.Y, const_fPointsDepth);

				if (( sqr(dxdydz.X) < sqr(const_fHandleRadius/(fUserScale*vHandle.X)) ) &&
					( sqr(dxdydz.Y) < sqr(const_fHandleRadius/(fUserScale*vHandle.Y)) ) && 
					( sqr(dxdydz.Z) < sqr(const_fHandleRadius/(fUserScale*vHandle.Z)) ))
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
	float fJitter       = 5;

	if (MorphingToolSubWindow::MouseFunc(button, state, x, y)) return true;

	if ((x > m_iBottomLeftX) && (x < m_iBottomLeftX + m_iWidth) &&
		(y > m_iBottomLeftY) && (y < m_iBottomLeftY + m_iHeight))
	{

		SetupGraphicsPipeline();

		Vec3 vHandle = Vecc3(1.0, 1.0, 1.0);
		for (auto& matr : liMatrixConveyor)
		{
			vHandle = matr.m^vHandle;
			glMultMatrixf(&matr.m.m[0][0]);
		}

		Vec3d v3DCoords;
		gluUnProjectFriendly(x, y, 0, v3DCoords.X, v3DCoords.Y, v3DCoords.Z);

		if ((stateTransform == STATE_TRANS_IDLE) && (button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
		{
			// SCALING
			for (size_t i = 0; i < liScalingHandles.size(); i++)
			{
				auto& point = liScalingHandles[i];

				Vec3 dxdydz = Vecc3(v3DCoords) - Vecc3(point.X, point.Y, const_fPointsDepth);

				if (( sqr(dxdydz.X) < sqr(fJitter/(fUserScale*vHandle.X)) ) &&
					( sqr(dxdydz.Y) < sqr(fJitter/(fUserScale*vHandle.Y)) ) && 
					( sqr(dxdydz.Z) < sqr(fJitter/(fUserScale*vHandle.Z)) ))
				{
					stateTransform = STATE_TRANS_SCALE;

					glutSetCursor(GLUT_CURSOR_INFO);

					m_ptHandleClicked = point;

					// find the opposite corner
					size_t iCorner = (i + 2) % liScalingHandles.size();
					m_ptHandlePivot = liScalingHandles[iCorner];

					ptPrevPoint = Vecc2(x, y);

					return true;
				}
			}

			// TRANSLATION
			auto& ptTranslate = ptTranslateHandle;

			Vec3 dxdydz;
			dxdydz = Vecc3(v3DCoords) - Vecc3(ptTranslate.X, ptTranslate.Y, const_fPointsDepth);

			if (( sqr(dxdydz.X) < sqr(fJitter/(fUserScale*vHandle.X)) ) &&
				( sqr(dxdydz.Y) < sqr(fJitter/(fUserScale*vHandle.Y)) ) && 
				( sqr(dxdydz.Z) < sqr(fJitter/(fUserScale*vHandle.Z)) ))
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

			if (( sqr(dxdydz.X) < sqr(fJitter/(fUserScale*vHandle.X)) ) &&
				( sqr(dxdydz.Y) < sqr(fJitter/(fUserScale*vHandle.Y)) ) && 
				( sqr(dxdydz.Z) < sqr(fJitter/(fUserScale*vHandle.Z)) ))
			{
				stateTransform = STATE_TRANS_ROTATE;

				glutSetCursor(GLUT_CURSOR_INFO);

				m_ptHandleClicked = ptRotate;
				m_ptHandlePivot   = ptTranslateHandle;

				ptPrevPoint = Vecc2(x, y);

				return true;
			}
		}

	}
	
	if ((stateTransform != STATE_TRANS_IDLE) && (button == GLUT_LEFT_BUTTON) && (state == GLUT_UP))
	{
		stateTransform = STATE_TRANS_IDLE;

		if (liMatrixConveyor.size() > 0)
			liMatrixConveyor.back().bFinalized = true;
	}

	return false;
}


void WarpingToolSubWindow::MotionFunc(int x, int y)
{
	MorphingToolSubWindow::MotionFunc(x, y);

	if ((x > m_iBottomLeftX) && (x < m_iBottomLeftX + m_iWidth) &&
		(y > m_iBottomLeftY) && (y < m_iBottomLeftY + m_iHeight))
	{

		SetupGraphicsPipeline();

		for (auto& matr : liMatrixConveyor)
		{
			if (!matr.bFinalized) continue;
			glMultMatrixf(&matr.m.m[0][0]);
		}

		Vec3d v3DCoords;
		gluUnProjectFriendly(x, y, 0, v3DCoords.X, v3DCoords.Y, v3DCoords.Z);

		if (stateTransform == STATE_TRANS_SCALE)
		{
			if (PointDist(Vecc2(x, y), ptPrevPoint) > 3)
			{

				if ((liMatrixConveyor.size() == 0) || (liMatrixConveyor.back().bFinalized))
					liMatrixConveyor.emplace_back();
				
				Matr4 matrTrans = Mat4MakeTrans(-m_ptHandlePivot.X, -m_ptHandlePivot.Y, 0.0);

				matrTrans = Mat4MakeScale((v3DCoords.X - m_ptHandlePivot.X)/(m_ptHandleClicked.X - m_ptHandlePivot.X),
										  (v3DCoords.Y - m_ptHandlePivot.Y)/(m_ptHandleClicked.Y - m_ptHandlePivot.Y),
					                      1.0f)*matrTrans;

				matrTrans = Mat4MakeTrans(m_ptHandlePivot.X, m_ptHandlePivot.Y, 0.0)*matrTrans;

				liMatrixConveyor.back() = matrTrans;
			}
		}

		if (stateTransform == STATE_TRANS_TRANSLATE)
		{
			if (PointDist(Vecc2(x, y), ptPrevPoint) > 3)
			{
				if ((liMatrixConveyor.size() == 0) || (liMatrixConveyor.back().bFinalized))
					liMatrixConveyor.emplace_back();

				Matr4 matrTrans = Mat4MakeTrans( v3DCoords.X - m_ptHandleClicked.X, 
												 v3DCoords.Y - m_ptHandleClicked.Y,
												 1.0f);

				liMatrixConveyor.back() = matrTrans;
			}
		}

		if (stateTransform == STATE_TRANS_ROTATE)
		{
			if (PointDist(Vecc2(x, y), ptPrevPoint) > 3)
			{
				if ((liMatrixConveyor.size() == 0) || (liMatrixConveyor.back().bFinalized))
					liMatrixConveyor.emplace_back();

				Vec2 vec1 = m_ptHandleClicked - m_ptHandlePivot;
				Vec2 vec2 = Vecc2(v3DCoords)  - m_ptHandlePivot;
				float angle = AngleBetweenVectorsCCW(vec1, vec2)*RADIANS_TO_DEGREES;

				Matr4 matrRot = Mat4MakeRot(angle, Vecc3(m_ptHandlePivot, 1.0) );

				liMatrixConveyor.back() = matrRot;
			}
		}
	}
}


bool WarpingToolSubWindow::KeyboardFunc(unsigned char key, int x, int y)
{
	return false;
}