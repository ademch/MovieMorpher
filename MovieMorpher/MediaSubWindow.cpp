#include "stdafx.h"
#include "MediaSubWindow.h"
#include "GLSL_Pipeline.h"
#include "../../!!adGUI/TrackClip.h"
#include "../../!!adVideo/FFMS_Video.h"
#include "../../!!adGUI/VideoPositionMediator.h"
#include "../../!!adGlobals/wdir.h"
#include "WarpingToolSubWindow.h"
#include "ImageSaveLoad.h"


MediaSubWindow::MediaSubWindow(int iParentWidth, int iParentHeight,
							   float fBottomLeftXperc, float fBottomLeftYperc, float fWidthPerc, float fHeightPerc) :
	            OpenGLSubWindowWithGUI(iParentWidth, iParentHeight,
				         			   fBottomLeftXperc, fBottomLeftYperc, fWidthPerc, fHeightPerc)
{
	windowTimeLine = NULL;

	SoundAL::Initialize();
	SoundAL::PrintSndInfo();

	FFMS_Video::Initialize();

	stateMediaPlayer = STATE_MEDIAPLAYER_IDLE;
	QueryPerformanceFrequency(&ticksPerSecond);

	PositionMediator::Get()->subscribeForPos([this](void* origin, double fVal)
	{
		if (origin != this) UpdateVideoTrackPosition(fVal);
	});

	fElapsedTimerS = 0.0;
	fSlider10msUnitsAtStart = 0.0;

	PopulateGUI();
}



void MediaSubWindow::PopulateGUI()
{
	//listBox = new ListBox("Resources", -400,-40, 200, 5, 6.3);
	//listBox->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	//listBox->items.push_back("First");
	//listBox->items.push_back("Second1234567890123456789");
	//listBox->items.push_back("Third");
	//listBox->items.push_back("Fourth");
	//listBox->items.push_back("Fifth");
	//listBox->items.push_back("Sixth");
	//listBox->items.push_back("Seven");
	//listBox->items.push_back("Eight");
	//liGUI_Elements.push_back(listBox);

	PushButtonImage* pushButtonImg;
	pushButtonImg = new PushButtonImage("PlayLoop", -200, 10, 30);
	pushButtonImg->LoadImg("Icons\\Image4.bmp");
	pushButtonImg->strHint = "Play selection in loop";
	pushButtonImg->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	liGUI_Elements.push_back(pushButtonImg);

	liButtons.push_back(pushButtonImg);
	pushButtonImg->OnClick = [this, pushButtonImg]() { return Push(pushButtonImg); };

	pushButtonImg = new PushButtonImage("PlayS2E", -166, 10, 30);
	pushButtonImg->LoadImg("Icons\\Image7.bmp");
	pushButtonImg->strHint = "Play from start to end";
	pushButtonImg->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	liGUI_Elements.push_back(pushButtonImg);

	liButtons.push_back(pushButtonImg);
	pushButtonImg->OnClick = [this, pushButtonImg]() { return Push(pushButtonImg); };

	pushButtonImg = new PushButtonImage("Pause", -132, 10, 30);
	pushButtonImg->LoadImg("Icons\\Image6.bmp");
	pushButtonImg->strHint = "Pause playback";
	pushButtonImg->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	liGUI_Elements.push_back(pushButtonImg);

	liButtons.push_back(pushButtonImg);
	pushButtonImg->OnClick = [this, pushButtonImg]() { return Push(pushButtonImg); };

	pushButtonImg = new PushButtonImage("PlayFromCursor", -98, 10, 30);
	pushButtonImg->LoadImg("Icons\\Image5.bmp");
	pushButtonImg->strHint = "Play from cursor";
	pushButtonImg->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	liGUI_Elements.push_back(pushButtonImg);

	liButtons.push_back(pushButtonImg);
	pushButtonImg->OnClick = [this, pushButtonImg]() { return Push(pushButtonImg); };

	pushButtonImg = new PushButtonImage("Stop", -64, 10, 30);
	pushButtonImg->LoadImg("Icons\\Image8.bmp");
	pushButtonImg->strHint = "Stop playback";
	pushButtonImg->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	liGUI_Elements.push_back(pushButtonImg);

	liButtons.push_back(pushButtonImg);
	pushButtonImg->OnClick = [this, pushButtonImg]() { return Push(pushButtonImg); };

	labelPlayhead = new Label("00:00:00.234", -201, -40, 11.9f);
	labelPlayhead->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	labelPlayhead->vColor = Vecc4(0.1f, 0.7f, 0.1f, 1.0f);
	liGUI_Elements.push_back(labelPlayhead);

	ButtonImage* buttonImg;
	buttonImg = new ButtonImage("", -210, 60, 32);
	buttonImg->LoadImg("Icons\\Image9.bmp");
	buttonImg->strHint = "Add video track...";
	buttonImg->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	buttonImg->OnClick = [this]() {	return this->AddTrackVideo(); };
	liGUI_Elements.push_back(buttonImg);

	buttonImg = new ButtonImage("", -176, 60, 32);
	buttonImg->LoadImg("Icons\\Image10.bmp");
	buttonImg->strHint = "Add image...";
	buttonImg->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	buttonImg->OnClick = [this]() {	return this->AddTrackPicture(); };
	liGUI_Elements.push_back(buttonImg);
}


bool MediaSubWindow::AddTrackPicture()
{
	unsigned int width, height;
	unsigned char* image;
	
	char filePath[512];
	image = ImageSaveLoadHelper::LoadImageFromDisk(width, height, filePath);

	if (!image) return false;

	char* fileName = GetFileName(filePath);

	OpenGLSubWindowWithGUI* newToolWindow;
	newToolWindow = OnNewMedia(fileName);

	// load image into the tool input fbo
	WarpingToolSubWindow* wndWarpingTool = dynamic_cast<WarpingToolSubWindow*>(newToolWindow);
		wndWarpingTool->ReshapeFBOprocessors(0, 0, width, height);
		wndWarpingTool->TextureUpdateInputFBOprocessor(width, height, image);

	TrackClip* clip = windowTimeLine->AddClip(newToolWindow, 60*100);
		clip->textureIcon = LoadTexture(width, height, image);
		clip->mediaType = CLIP_IMAGE;

	free(image);

	return true;
}


bool MediaSubWindow::AddTrackVideo()
{
	// SELECT VIDEO FILE
	char filePath[512];
	if (!ImageSaveLoadHelper::SelectVideoFromDisk(filePath)) return false;

	// LOAD VIDEO FILE
	FFMS_Video* video = new FFMS_Video();
	video->LoadMPEG(filePath);

	// WIDEN TIMELINE IF NEEDED
	if (PositionMediator::Get()->Duration10msUnits() < video->iTotalFrames)
	{
		PositionMediator::Get()->Init(NULL, 0.0f, 2*video->liIndex[video->iTotalFrames-1]*100.0);
	}

	// CREATE TOOL AND PARAMS WINDOWS
	char* fileName = GetFileName(filePath);
	OpenGLSubWindowWithGUI* newToolWindow = OnNewMedia(fileName);

	WarpingToolSubWindow* wndWarpingTool = dynamic_cast<WarpingToolSubWindow*>(newToolWindow);

	// load image into the tool input fbo
	FrameItem* frame = video->videoCacheThread->GetFrame(0);
		assert(frame);
		wndWarpingTool->ReshapeFBOprocessors(0, 0, frame->width, frame->height);
		wndWarpingTool->TextureUpdateInputFBOprocessor(frame->width, frame->height, frame->data);

	TrackClip* clip = windowTimeLine->AddClip(newToolWindow, video->liIndex[video->iTotalFrames-1]*100.0);

		ImageSaveLoadHelper::_FlipImage(frame->data, frame->width, frame->height);
			clip->textureIcon = LoadTexture(frame->width, frame->height, frame->data);
		ImageSaveLoadHelper::_FlipImage(frame->data, frame->width, frame->height);

		clip->mediaType = CLIP_VIDEO;
		clip->video     = video;

	return true;
}


void MediaSubWindow::Draw()
{
	if (stateMediaPlayer == STATE_MEDIAPLAYER_PLAYING)
	{
		int iPlayhead10msTicks = PositionMediator::Get()->Pos10msUnits();

		for (auto iterClip : TrackClip::liClips)
		{
			if ( (iPlayhead10msTicks >= iterClip->m_iStartPos10msUnits) &&
				 (iPlayhead10msTicks < (iterClip->m_iStartPos10msUnits + iterClip->m_iLength10msUnits)) )
			{
				WarpingToolSubWindow* wndWarpingTool;
				wndWarpingTool = dynamic_cast<WarpingToolSubWindow*>(iterClip->windowTool);

				if (wndWarpingTool->bActive) continue;

				if (iterClip->mediaType == CLIP_IMAGE)
				{
					//wndWarpingTool->ReDrawFBOprocessors();
					wndWarpingTool->DrawFBOquad();
				}
				else
				{
					// SHORTCUT
					FFMS_Video* vid = iterClip->video;

					// REMEMBER PREVIOUS FRAME FOR STATISTICS
					int iFramePrev = vid->iCurrentFrame;

					// CALC LOCAL TIME OF TRACK, TIME IS A RULE TO FOLLOW
					float _fClipLocalTimeS = (iPlayhead10msTicks - iterClip->m_iStartPos10msUnits)/100.0;

					// USE PREVIOUS FRAME AS A SEED, AND LEAVE IT AS IS OR MOVE TOWARDS PRESENTATION TIME INSIDE CACHE
					FrameItem* frame = vid->videoCacheThread->GetFrameByTime(_fClipLocalTimeS, vid->iCurrentFrame);

					// UPDATE IF NEW FRAME HAS BECOME AVAILABLE
					if (iFramePrev != iterClip->video->iCurrentFrame)
					{
						//wndWarpingTool->ReshapeFBOprocessors(0, 0, frame->width, frame->height);
						wndWarpingTool->TextureUpdateInputFBOprocessor(frame->width, frame->height, frame->data);
						wndWarpingTool->ReDrawFBOprocessors();

						printf("Frame second is: %f\n", frame->fS);
					}
					else
						printf("Duplicate frame time!!!!!!!!!!!!!!!!!!! good\n");

					wndWarpingTool->DrawFBOquad();
				}
			}
		}
	}
}



void MediaSubWindow::RenderGUI()
{
	PositionMediator* mediator = PositionMediator::Get();
	int iTotal10msUnits = mediator->Duration10msUnits();

	if (stateMediaPlayer == STATE_MEDIAPLAYER_PLAYING)
	{
		LARGE_INTEGER T1;
		QueryPerformanceCounter(&T1);
		fElapsedTimerS = double(T1.QuadPart - T0.QuadPart) / double(ticksPerSecond.QuadPart);
		mediator->SetPos0_1(this, (fElapsedTimerS*100.0 + fSlider10msUnitsAtStart)/iTotal10msUnits);
	}

//	float fSecondPassed = video->audioThread->GetCurrentSecond();
//	mediator->SetPos0_1(NULL, fSecondPassed/iDuration);

	float fS = mediator->Pos10msUnits()/100.0;

	int hours        =  int(fS)/3600;
	int minutes      = (int(fS)/60)%60;
	int seconds      = (int(fS))%60;
	int milliseconds =  int((fS - (int)fS)*1000.0);

	static char buf[256];
	snprintf(buf, sizeof(buf), "%02d:%02d:%02d.%03d", hours, minutes, seconds, milliseconds);

	labelPlayhead->_text = buf;

	OpenGLSubWindowWithGUI::RenderGUI();
}


void MediaSubWindow::UpdateVideoTrackPosition(double fVal)
{
	int iPlayhead10msTicks = PositionMediator::Get()->Pos10msUnits();

	//printf("Callback on frame: %d\n", iPlayhead10msTicks);

	for (auto iterClip : TrackClip::liClips)
	{
		if ( (iPlayhead10msTicks >= iterClip->m_iStartPos10msUnits) &&
			 (iPlayhead10msTicks < (iterClip->m_iStartPos10msUnits + iterClip->m_iLength10msUnits)) )
		{
			WarpingToolSubWindow* wndWarpingTool;
			wndWarpingTool = dynamic_cast<WarpingToolSubWindow*>(iterClip->windowTool);

			//if (wndWarpingTool->bActive) continue;

			FFMS_Video* vid = iterClip->video;

			int iFramePrev = vid->iCurrentFrame;

			// PRESENTATION TIME IS A RULE TO FOLLOW
			float _fClipLocalTimeSec = (iPlayhead10msTicks - iterClip->m_iStartPos10msUnits)/100.0;
			
			// DURING SEEK GET INDEX FROM PRESENTATION TIME
			vid->iCurrentFrame = vid->NextIndexAfter(_fClipLocalTimeSec);

			// ACQUIRE NEEDED FRAME, LIKELY IT IS NOT IN CACHE, REVISIT SOON AFTER
			FrameItem* frame = NULL;
			while ((frame = vid->videoCacheThread->GetFrameByTime(_fClipLocalTimeSec, vid->iCurrentFrame)) == NULL)
			{
				Sleep(50);
			}

			if (iFramePrev != vid->iCurrentFrame)
			{
				//wndWarpingTool->ReshapeFBOprocessors(0, 0, frame->width, frame->height);
				wndWarpingTool->TextureUpdateInputFBOprocessor(frame->width, frame->height, frame->data);
				wndWarpingTool->ReDrawFBOprocessors();

				printf("Frame Second is: %f\n", frame->fS);
			}
			else
				printf("Duplicate frame time!!!!!!!!!!!!!!!!!!! good\n");

			wndWarpingTool->DrawFBOquad();
		}
	}

}


bool MediaSubWindow::Push(PushButtonImage* target)
{
	if (target->bPushed) return true;

	for (auto* b : liButtons)
		b->bPushed = false;

	if (target->_text == "PlayLoop")
	{
		target->bPushed = true;
	}
	if (target->_text == "PlayS2E")
	{
		target->bPushed = true;

		for (auto iterClip : TrackClip::liClips)
		{
			WarpingToolSubWindow* wndWarpingTool = dynamic_cast<WarpingToolSubWindow*>(iterClip->windowTool);

			if (iterClip->mediaType != CLIP_VIDEO) continue;

			iterClip->video->iCurrentFrame = 0;
		}

		QueryPerformanceCounter(&T0);
		fSlider10msUnitsAtStart = 0.0;

		stateMediaPlayer = STATE_MEDIAPLAYER_PLAYING;

		if (OnPlaybackStarted)	OnPlaybackStarted(true);
	}
	if (target->_text == "Pause")
	{
		target->bPushed = true;
	}
	if (target->_text == "PlayFromCursor")
	{
		target->bPushed = true;

		QueryPerformanceCounter(&T0);
		fSlider10msUnitsAtStart = PositionMediator::Get()->Pos10msUnits();

		stateMediaPlayer = STATE_MEDIAPLAYER_PLAYING;

		if (OnPlaybackStarted)	OnPlaybackStarted(true);

	}
	if (target->_text == "Stop")
	{
		stateMediaPlayer = STATE_MEDIAPLAYER_IDLE;
		if (OnPlaybackStarted)
			OnPlaybackStarted(false);

	}

	return true;
}

