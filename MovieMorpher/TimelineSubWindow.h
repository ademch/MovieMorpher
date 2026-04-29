#ifndef TIMELINESUBSUBWINDOW_H
#define TIMELINESUBSUBWINDOW_H

#include "../../!!adGUI/SubWindowWithGUI.h"
#include "../../!!adGUI/HorScrollBar.h"
#include "../../!!adGUI/button.h"
#include <vector>


class TimelineSubWindow : public OpenGLSubWindowWithGUI
{
public:
	TimelineSubWindow(int iParentWidth, int iParentHeight,
				      float fBottomLeftXperc, float fBottomLeftYperc,
				      float fWidthPerc, float fHeightPerc);
	~TimelineSubWindow();

	virtual	void Render();

	virtual bool PassiveMotionFunc(int x, int y);
	virtual void MouseFunc(int button, int state, int x, int y);
	virtual void MotionFunc(int x, int y);
	virtual void MouseWheelFunc(int state, int delta, int x, int y);

	virtual void Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight);

	float fMorphRatio;

protected:

private:
	HorScrollBar* scrollBar;

};


#endif