#include "stdafx.h"
#include "WarpingToolSubWindow.h"
#include <vector>
#include <functional>

WarpingToolSubWindow::WarpingToolSubWindow(int iParentWidth, int iParentHeight,
										   float fBottomLeftXperc, float fBottomLeftYperc,
										   float fWidthPerc, float fHeightPerc) :
	MorphingToolSubWindow(iParentWidth, iParentHeight,
		fBottomLeftXperc, fBottomLeftYperc, fWidthPerc, fHeightPerc)

{

}


void WarpingToolSubWindow::Draw()
{

}

bool WarpingToolSubWindow::PassiveMotionFunc(int x, int y)
{
	return false;
}
bool WarpingToolSubWindow::MouseFunc(int button, int state, int x, int y)
{
	return false;
}
void WarpingToolSubWindow::MotionFunc(int x, int y)
{

}
bool WarpingToolSubWindow::KeyboardFunc(unsigned char key, int x, int y)
{
	return false;
}