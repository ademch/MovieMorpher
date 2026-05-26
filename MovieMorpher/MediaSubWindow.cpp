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

	pushButtonImg = new PushButtonImage("PlayFcursor", -98, 10, 30);
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

	TrackClip* clip = windowTimeLine->AddClip(newToolWindow);
		clip->textureIcon = LoadTexture(width, height, image);
		clip->mediaType = CLIP_IMAGE;

	free(image);

	return true;
}


bool MediaSubWindow::AddTrackVideo()
{
	FFMS_Video* video = new FFMS_Video();
	video->LoadMPEG("E:\\Or\\MovieMorpher\\Debug\\output00.mp4");

	OpenGLSubWindowWithGUI* newToolWindow = OnNewMedia("video");

	WarpingToolSubWindow* wndWarpingTool = dynamic_cast<WarpingToolSubWindow*>(newToolWindow);

	// load image into the tool input fbo
	FrameItem* frame = video->videoCacheThread->GetFrame(0);
		wndWarpingTool->ReshapeFBOprocessors(0, 0, frame->width, frame->height);
		wndWarpingTool->TextureUpdateInputFBOprocessor(frame->width, frame->height, frame->data);

	TrackClip* clip = windowTimeLine->AddClip(newToolWindow);

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
		static int iFFF = 0;

		for (auto iter : TrackClip::liClips)
		{
			if ((elapsed_sec > iter->m_iStartPosFrame) && (elapsed_sec < iter->m_iStartPosFrame + iter->m_iLengthFrames))
			{
				WarpingToolSubWindow* wndWarpingTool = dynamic_cast<WarpingToolSubWindow*>(iter->windowTool);
				if (iter->mediaType == CLIP_IMAGE)
					wndWarpingTool->DrawFBOquad();
				else
				{
					FrameItem* frame = iter->video->videoCacheThread->GetFrame(iFFF++);
					//FFMS_Track* videoTrack = FFMS_GetTrackFromVideo(iter->video->videoSource);
					//const FFMS_FrameInfo* info = FFMS_GetFrameInfo(videoTrack, 0);

					//int64_t pts = info->PTS;

					//wndWarpingTool->ReshapeFBOprocessors(0, 0, frame->width, frame->height);
					wndWarpingTool->TextureUpdateInputFBOprocessor(frame->width, frame->height, frame->data);
					wndWarpingTool->ReDrawFBOprocessors();
					wndWarpingTool->DrawFBOquad();
				}
			}
		}
	}
}



void MediaSubWindow::RenderGUI()
{
	PositionMediator* mediator = PositionMediator::Get();
	int iDuration = mediator->DurationFrames();

	if (stateMediaPlayer == STATE_MEDIAPLAYER_PLAYING)
	{
		LARGE_INTEGER T1;
		QueryPerformanceCounter(&T1);
		elapsed_sec = float(T1.QuadPart - T0.QuadPart) / float(ticksPerSecond.QuadPart);
		mediator->SetPos0_1(NULL, elapsed_sec/iDuration);
	}

//	float fSecondPassed = video->audioThread->GetCurrentSecond();
//	mediator->SetPos0_1(NULL, fSecondPassed/iDuration);

	int hours   = int(mediator->Pos01()*mediator->DurationFrames())/3600;
	int minutes = int(mediator->Pos01()*mediator->DurationFrames())/60;
	int seconds = int(mediator->Pos01()*mediator->DurationFrames())%60;
	
	double intpart;
	int milliseconds = int(modf(mediator->Pos01()*mediator->DurationFrames(), &intpart)*1000);

	static char buf[256];
	snprintf(buf, sizeof(buf), "%02d:%02d:%02d.%03d", hours, minutes, seconds, milliseconds);

	labelPlayhead->_text = buf;

	OpenGLSubWindowWithGUI::RenderGUI();
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

		QueryPerformanceCounter(&T0);
		stateMediaPlayer = STATE_MEDIAPLAYER_PLAYING;

		if (OnPlaybackStarted)	OnPlaybackStarted(true);
	}
	if (target->_text == "Pause")
	{
		target->bPushed = true;
	}
	if (target->_text == "PlayFcursor")
	{
		target->bPushed = true;
	}
	if (target->_text == "Stop")
	{
		stateMediaPlayer = STATE_MEDIAPLAYER_IDLE;
		if (OnPlaybackStarted)
			OnPlaybackStarted(false);

	}

	return true;
}


