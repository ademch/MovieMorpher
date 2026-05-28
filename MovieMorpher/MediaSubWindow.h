#ifndef MEDIASUBSUBWINDOW_H
#define MEDIASUBSUBWINDOW_H

#include "../../!!adGUI/SubWindowWithGUI.h"
#include "TimelineSubWindow.h"
#include "../../!!adGUI/label.h"
#include "../../!!adGUI/button.h"
#include "../../!!adVideo/SoundAL.h"


enum StateMediaPlayer_enum {
	STATE_MEDIAPLAYER_IDLE,
	STATE_MEDIAPLAYER_PLAYING
};


class MediaSubWindow : public OpenGLSubWindowWithGUI
{
public:
	MediaSubWindow(int iParentWidth, int iParentHeight,
				   float fBottomLeftXperc, float fBottomLeftYperc,
				   float fWidthPerc, float fHeightPerc);
	~MediaSubWindow()
	{
		SoundAL::DestroyOpenAL();
	}

	void SetTimelineSubWindow(TimelineSubWindow* wnd)
	{
		windowTimeLine = wnd;
	}

	void RenderGUI() override;
	void Draw() override;

	std::function<OpenGLSubWindowWithGUI*(char*)> OnNewMedia;
	std::function<void(bool)> OnPlaybackStarted;

protected:

	bool AddTrackPicture();
	bool AddTrackVideo();

	void PopulateGUI() override;

	TimelineSubWindow* windowTimeLine;
	Label*             labelPlayhead;

private:
	std::vector<PushButtonImage*> liButtons;

	bool Push(PushButtonImage* target);

	LARGE_INTEGER ticksPerSecond;
	LARGE_INTEGER T0;

	StateMediaPlayer_enum stateMediaPlayer;
	double fElapsedS;

	void UpdateVideoTrackPosition(double fVal);

};


#endif