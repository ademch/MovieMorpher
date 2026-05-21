#ifndef MEDIASUBSUBWINDOW_H
#define MEDIASUBSUBWINDOW_H

#include "../../!!adGUI/SubWindowWithGUI.h"
#include "TimelineSubWindow.h"
#include "../../!!adGUI/label.h"
#include "../../!!adGUI/button.h"
#include "../../!!adVideo/FFMS_Video.h"
#include "../../!!adVideo/SoundAL.h"


class MediaSubWindow : public OpenGLSubWindowWithGUI
{
public:
	MediaSubWindow(int iParentWidth, int iParentHeight,
				   float fBottomLeftXperc, float fBottomLeftYperc,
				   float fWidthPerc, float fHeightPerc);
	~MediaSubWindow()
	{
		delete video;
		SoundAL::DestroyOpenAL();
	}

	void SetTimelineSubWindow(TimelineSubWindow* wnd)
	{
		windowTimeLine = wnd;
	}

	void RenderGUI() override;

	std::function<OpenGLSubWindowWithGUI*(char*)> OnNewMedia;

protected:

	bool AddTrackPicture();
	bool AddTrackVideo();

	void PopulateGUI() override;

	TimelineSubWindow* windowTimeLine;
	Label*             labelPlayhead;

private:
	std::vector<PushButtonImage*> liButtons;

	bool Push(PushButtonImage* target);

	FFMS_Video* video;

};


#endif