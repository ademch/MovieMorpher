#ifndef MEDIASUBSUBWINDOW_H
#define MEDIASUBSUBWINDOW_H

#include "../../!!adGUI/SubWindowWithGUI.h"
#include "TimelineSubWindow.h"
#include "../../!!adGUI/label.h"


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

	void RenderGUI() override;

protected:

	bool AddTrackPicture();

	TimelineSubWindow* windowTimeLine;
	Label* labelPlayhead;

private:

};


#endif