#ifndef MEDIASUBSUBWINDOW_H
#define MEDIASUBSUBWINDOW_H

#include "../../!!adGUI/SubWindowWithGUI.h"
#include "TimelineSubWindow.h"


class MediaSubWindow : public OpenGLSubWindowWithGUI
{
public:
	MediaSubWindow(int iParentWidth, int iParentHeight,
				   float fBottomLeftXperc, float fBottomLeftYperc,
				   float fWidthPerc, float fHeightPerc);
	~MediaSubWindow() {}

	void SetTimelineSubWindow(TimelineSubWindow* wnd)
	{
		windowTimeLine = wnd;
	}

protected:

	bool AddTrackPicture();

	TimelineSubWindow* windowTimeLine;

private:

};


#endif