#ifndef TIMELINESUBSUBWINDOW_H
#define TIMELINESUBSUBWINDOW_H

#include "../../!!adGUI/SubWindowWithGUI.h"
#include "../../!!adGUI/Slider.h"
#include "../../!!adGUI/OnOffFlipSwitch.h"
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

	virtual void PassiveMotionFunc(int x, int y);
	virtual void MouseFunc(int button, int state, int x, int y);
	virtual void MotionFunc(int x, int y);

	float fMorphRatio;

protected:

private:
	Slider<SL_TIME>   *SliderMorphRatio;

};


#endif