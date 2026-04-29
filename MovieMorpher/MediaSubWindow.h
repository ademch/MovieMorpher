#ifndef MEDIASUBSUBWINDOW_H
#define MEDIASUBSUBWINDOW_H

#include "../../!!adGUI/SubWindowWithGUI.h"
#include "../../!!adGUI/Slider.h"
#include "../../!!adGUI/ListBox.h"
#include "../../!!adGUI/button.h"
#include "TimelineSubWindow.h"
#include <vector>


class MediaSubWindow : public OpenGLSubWindowWithGUI
{
public:
	MediaSubWindow(int iParentWidth, int iParentHeight,
				   float fBottomLeftXperc, float fBottomLeftYperc,
				   float fWidthPerc, float fHeightPerc);
	~MediaSubWindow();

	virtual	void Render();

	virtual bool PassiveMotionFunc(int x, int y);
	virtual void MouseFunc(int button, int state, int x, int y);
	virtual void MotionFunc(int x, int y);
	virtual void MouseWheelFunc(int state, int delta, int x, int y);

	float fMorphRatio;

protected:

private:
	ListBox* listBox;

};


#endif