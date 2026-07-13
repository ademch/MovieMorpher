#include "stdafx.h"
#include "MediaSubWindow.h"
#include "WarpingToolSubWindow.h"
#include "GLSL_Pipeline.h"
#include "../../!!adGUI/TrackClip.h"
#include "../../!!adVideo/FFMS_Video.h"
#include "../../!!adGUI/VideoPositionMediator.h"
#include "../../!!adGlobals/wdir.h"
#include "ImageSaveLoad.h"
#include "../../!!adGUI/TrackClipMenu.h"


// welcome screen
extern WarpingToolSubWindow*       windowToolEditorDefault;
extern ParamsSubWindow*            windowParamsDefault;


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

	PositionMediator::Get()->subscribeForPos(this, [this](void* origin, double fVal)
	{
		if (origin != this) UpdateVideoTrackPosition(fVal);
	});

	fElapsedTimer10ms = 0.0;
	iSlider10msUnitsAtStart = 0.0;

	bRecordingInProgress = false;
	ffmpegPipe = NULL;

	PopulateGUI();

	callback_RegisterTrackClip();
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
	pushButtonImg = new PushButtonImage("Record", -240, 10, 30);
	pushButtonImg->LoadImg("Icons\\Image2.bmp");
	pushButtonImg->strHint = "Record";
	pushButtonImg->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	liGUI_Elements.push_back(pushButtonImg);

	//liButtonsGroup.push_back(pushButtonImg);
	pushButtonImg->OnClick = [this, pushButtonImg]() { return OnButtonRecord(pushButtonImg); };

	pushButtonImg = new PushButtonImage("PlayLoopBackForth", -200, 10, 30);
	pushButtonImg->LoadImg("Icons\\Image3.bmp");
	pushButtonImg->strHint = "Play selection in loop back and forth";
	pushButtonImg->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	liGUI_Elements.push_back(pushButtonImg);

	liButtonsGroup.push_back(pushButtonImg);
	pushButtonImg->OnClick = [this, pushButtonImg]() { return OnButtonPush(pushButtonImg); };

	pushButtonImg = new PushButtonImage("PlayLoop", -166, 10, 30);
	pushButtonImg->LoadImg("Icons\\Image4.bmp");
	pushButtonImg->strHint = "Play selection in loop";
	pushButtonImg->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	liGUI_Elements.push_back(pushButtonImg);

	liButtonsGroup.push_back(pushButtonImg);
	pushButtonImg->OnClick = [this, pushButtonImg]() { return OnButtonPush(pushButtonImg); };

	pushButtonImg = new PushButtonImage("PlayFromStart", -132, 10, 30);
	pushButtonImg->LoadImg("Icons\\Image7.bmp");
	pushButtonImg->strHint = "Play from start to end";
	pushButtonImg->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	liGUI_Elements.push_back(pushButtonImg);

	liButtonsGroup.push_back(pushButtonImg);
	pushButtonImg->OnClick = [this, pushButtonImg]() { return OnButtonPush(pushButtonImg); };

	pushButtonImg = new PushButtonImage("PlayFromCursor", -98, 10, 30);
	pushButtonImg->LoadImg("Icons\\Image5.bmp");
	pushButtonImg->strHint = "Play from cursor";
	pushButtonImg->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	liGUI_Elements.push_back(pushButtonImg);

	liButtonsGroup.push_back(pushButtonImg);
	pushButtonImg->OnClick = [this, pushButtonImg]() { return OnButtonPush(pushButtonImg); };

	//pushButtonImg = new PushButtonImage("Pause", -98, 10, 30);
	//pushButtonImg->LoadImg("Icons\\Image6.bmp");
	//pushButtonImg->strHint = "Pause playback";
	//pushButtonImg->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	//liGUI_Elements.push_back(pushButtonImg);

	////liButtonsGroup.push_back(pushButtonImg);
	//pushButtonImg->OnClick = [this, pushButtonImg]() { return OnButtonPause(pushButtonImg); };

	pushButtonStop = new PushButtonImage("Stop", -64, 10, 30);
	pushButtonStop->LoadImg("Icons\\Image8.bmp");
	pushButtonStop->strHint = "Stop playback";
	pushButtonStop->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	liGUI_Elements.push_back(pushButtonStop);

	liButtonsGroup.push_back(pushButtonStop);
	pushButtonStop->OnClick = [this]() { return OnButtonPush(pushButtonStop); };

	labelPlayhead = new Label("00:00:00.234", -201, -40, 11.9f);
	labelPlayhead->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	labelPlayhead->vColor = Vecc4(0.1f, 0.7f, 0.1f, 1.0f);
	liGUI_Elements.push_back(labelPlayhead);

	ButtonImage* buttonImg;
	buttonImg = new ButtonImage("", -210, 60, 32);
	buttonImg->LoadImg("Icons\\Image9.bmp");
	buttonImg->strHint = "Add video track...";
	buttonImg->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	buttonImg->OnClick = [this]() {	return AddTrackVideo(); };
	liGUI_Elements.push_back(buttonImg);

	buttonImg = new ButtonImage("", -176, 60, 32);
	buttonImg->LoadImg("Icons\\Image10.bmp");
	buttonImg->strHint = "Add image...";
	buttonImg->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	buttonImg->OnClick = [this]() {	return AddTrackPicture(); };
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
		clip->mediaType = CLIP_IMAGE;
		clip->textureIcon = LoadTexture(width, height, image);
		clip->RegisterTRSparam( &(wndWarpingTool->animatedTRSTransform.liKeys) );
		clip->RegisterMorphDSTparam( &(wndWarpingTool->animatedPolylineDst.liKeys) );

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
	if (PositionMediator::Get()->Duration10msUnits() < video->liIndex[video->iTotalFrames-1]*100.0)
	{
		PositionMediator::Get()->Init(NULL, 0.0f, 1.2*video->liIndex[video->iTotalFrames-1]*100.0);
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
		clip->RegisterTRSparam( &(wndWarpingTool->animatedTRSTransform.liKeys) );
		clip->RegisterMorphDSTparam( &(wndWarpingTool->animatedPolylineDst.liKeys) );

	return true;
}


bool MediaSubWindow::OnButtonRecord(PushButtonImage* target)
{
	target->bPushed = !target->bPushed;
	bRecordingInProgress = target->bPushed;

	if (bRecordingInProgress)
	{
		Vec2i vDim = Vecc2i(windowToolEditorDefault->Width(), windowToolEditorDefault->Height());
		vDim.X &= ~1;	// make number even
		vDim.Y &= ~1;	// make number even

		char cmd[4096];
		sprintf(cmd, "E:\\Or\\MovieMorpher\\Debug\\ffmpeg.exe "
					 "-f rawvideo -pix_fmt rgb24 -s %dx%d -r 20 -i - "
					 "-vf vflip "
					 "-c:v libx264 -preset veryfast -pix_fmt yuv420p E:\\Or\\MovieMorpher\\Debug\\output.mp4",
					 vDim.X, vDim.Y);

		ffmpegPipe = _popen( cmd, "wb" );
	}
	else
	{
		if (!ffmpegPipe) return false;

		fflush(ffmpegPipe);
		_pclose(ffmpegPipe); 
	}

return true;
}


void MediaSubWindow::Draw()
{
	if ((stateMediaPlayer == STATE_MEDIAPLAYER_PLAYING) ||
		(stateMediaPlayer == STATE_MEDIAPLAYER_PLAYING_LOOPED) ||
		(stateMediaPlayer == STATE_MEDIAPLAYER_PLAYING_LOOPED_BACKFORTH))
	{
		int iPlayhead10msTicks = PositionMediator::Get()->Pos10msUnits();

		for (auto iterClip : TrackClip::liClips)
		{
			if ( (iPlayhead10msTicks >= iterClip->m_iStartPos10msUnits) &&
				 (iPlayhead10msTicks < (iterClip->m_iStartPos10msUnits + iterClip->m_iLength10msUnits)) )
			{
				WarpingToolSubWindow* wndWarpingTool;
				wndWarpingTool = dynamic_cast<WarpingToolSubWindow*>(iterClip->windowTool);

				if (iterClip->mediaType == CLIP_IMAGE)
				{
					wndWarpingTool->ReDrawFBOprocessors();
					wndWarpingTool->DrawFBOquad();
				}
				else
				{
					GetFrameFromVideoAndRender(iterClip, iPlayhead10msTicks);
				}
			}
		}

		if (bRecordingInProgress)
		{
			Vec2i ptStart = Vecc2i(windowToolEditorDefault->BottomLeftX(), windowToolEditorDefault->BottomLeftY());
			Vec2i vDim    = Vecc2i(windowToolEditorDefault->Width(),       windowToolEditorDefault->Height());

			vDim.X &= ~1;	// make number even
			vDim.Y &= ~1;	// make number even

			std::vector<unsigned char> pixels(vDim.X * vDim.Y * 3);
			glReadPixels(  ptStart.X, ptStart.Y,  vDim.X, vDim.Y,  GL_RGB,  GL_UNSIGNED_BYTE,  pixels.data() );

			if (ffmpegPipe)
				fwrite( pixels.data(), 1, pixels.size(), ffmpegPipe );
		}
	}
}


void MediaSubWindow::RenderGUI()
{
	PositionMediator* mediator = PositionMediator::Get();

	int iTotal10msUnits = mediator->Duration10msUnits();

	LARGE_INTEGER T1;
	QueryPerformanceCounter(&T1);
	fElapsedTimer10ms = 100.0*( double(T1.QuadPart - T0.QuadPart) / double(ticksPerSecond.QuadPart) );

	int iStart10msUnits, iEnd10msUnits;
	mediator->PosSel10msUnits(iStart10msUnits, iEnd10msUnits);

	if (stateMediaPlayer == STATE_MEDIAPLAYER_PLAYING)
	{
		if (iSlider10msUnitsAtStart + fElapsedTimer10ms > iEnd10msUnits)
			OnButtonPush(pushButtonStop);
		else
			mediator->SetPos0_1(this, (iSlider10msUnitsAtStart + fElapsedTimer10ms)/iTotal10msUnits, true);
	}
	else if (stateMediaPlayer == STATE_MEDIAPLAYER_PLAYING_LOOPED)
	{
		if (iSlider10msUnitsAtStart + fElapsedTimer10ms > iEnd10msUnits)
		{
			// RESET THE TIME AS IF PLAYBACK HAS JUST STARTED
			QueryPerformanceCounter(&T0);
			fElapsedTimer10ms = 0.0;
			iSlider10msUnitsAtStart = iStart10msUnits;
		}

		mediator->SetPos0_1(this, (iSlider10msUnitsAtStart + fElapsedTimer10ms )/iTotal10msUnits, true);
	}
	else if (stateMediaPlayer == STATE_MEDIAPLAYER_PLAYING_LOOPED_BACKFORTH)
	{
		int iLength10msUnits = iEnd10msUnits - iStart10msUnits;

		double fCurrentTime10msUnits = iSlider10msUnitsAtStart + fElapsedTimer10ms;

		if (fCurrentTime10msUnits >= iStart10msUnits + 2*iLength10msUnits)
		{
			QueryPerformanceCounter(&T0);
			fElapsedTimer10ms = 0.0;
			iSlider10msUnitsAtStart = iStart10msUnits;
			fCurrentTime10msUnits = iStart10msUnits;
		}

		if (fCurrentTime10msUnits - iStart10msUnits < iLength10msUnits)
		{
			// FORWARD: start -> end
			mediator->SetPos0_1(this, fCurrentTime10msUnits/iTotal10msUnits, true);
		}
		else
		{
			// BACKWARD: end -> start
			mediator->SetPos0_1(this, (2*iEnd10msUnits - fCurrentTime10msUnits)/iTotal10msUnits, true);
		}

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
			if (iterClip->mediaType == CLIP_IMAGE) continue;

			GetFrameFromVideoAndRender(iterClip, iPlayhead10msTicks);
		}
	}

}


void MediaSubWindow::GetFrameFromVideoAndRender(TrackClip* clip, int iPlayhead10msTicks)
{
	WarpingToolSubWindow* wndWarpingTool;
	wndWarpingTool = dynamic_cast<WarpingToolSubWindow*>(clip->windowTool);

	//if (wndWarpingTool->bActive) continue;

	FFMS_Video* vid = clip->video;

	int iFramePrev = vid->iCurrentFrame;

	// PRESENTATION TIME IS A RULE TO FOLLOW
	float _fClipLocalTimeSec = (iPlayhead10msTicks - clip->m_iStartPos10msUnits)/100.0;

	// DURING SEEK GET INDEX FROM PRESENTATION TIME
	vid->iCurrentFrame = vid->NextIndexOrEqualFromTime(_fClipLocalTimeSec);

	// ACQUIRE NEEDED FRAME, IF IT IS NOT IN CACHE, REVISIT SOON AFTER
	FrameItem* frame = NULL;
	while ((frame = vid->videoCacheThread->GetFrame(vid->iCurrentFrame)) == NULL)
	{
		Sleep(10);
		printf("Waiting for frame: %d\n", vid->iCurrentFrame);
	}

	if (iFramePrev != vid->iCurrentFrame)
	{
		//wndWarpingTool->ReshapeFBOprocessors(0, 0, frame->width, frame->height);
		wndWarpingTool->TextureUpdateInputFBOprocessor(frame->width, frame->height, frame->data);
		wndWarpingTool->ReDrawFBOprocessors();

		printf("Frame Rendered: %d\n", vid->iCurrentFrame);
	}
	else
		printf("Duplicate frame time!!!!!!!!!!!!!!!!!!! good\n");

	wndWarpingTool->DrawFBOquad();
}


bool MediaSubWindow::OnButtonPush(PushButtonImage* target)
{
	if (target->bPushed) return true;

	for (auto* b : liButtonsGroup)
		b->bPushed = false;

	PositionMediator* mediator = PositionMediator::Get();

	if (target->_text == "PlayLoopBackForth")
	{
		target->bPushed = true;

		QueryPerformanceCounter(&T0);

		int iSelStart10msUnits, iSelEnd10msUnits;
		mediator->PosSel10msUnits(iSelStart10msUnits, iSelEnd10msUnits);

		iSlider10msUnitsAtStart = CLAMP(mediator->Pos10msUnits(), iSelStart10msUnits, iSelEnd10msUnits);

		stateMediaPlayer = STATE_MEDIAPLAYER_PLAYING_LOOPED_BACKFORTH;

		if (OnPlaybackStarted)	OnPlaybackStarted(true);
	}
	if (target->_text == "PlayLoop")
	{
		target->bPushed = true;

		QueryPerformanceCounter(&T0);

		int iSelStart10msUnits, iSelEnd10msUnits;
		mediator->PosSel10msUnits(iSelStart10msUnits, iSelEnd10msUnits);

		iSlider10msUnitsAtStart = CLAMP(mediator->Pos10msUnits(), iSelStart10msUnits, iSelEnd10msUnits);

		stateMediaPlayer = STATE_MEDIAPLAYER_PLAYING_LOOPED;

		if (OnPlaybackStarted)	OnPlaybackStarted(true);
	}
	if (target->_text == "PlayFromStart")
	{
		target->bPushed = true;

		QueryPerformanceCounter(&T0);

		// PLAY FROM START
		iSlider10msUnitsAtStart = 0;

		mediator->SetMarker(NULL, 0.0);

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

		// PLAY FROM CURRENT PLAYHEAD POSITION
		iSlider10msUnitsAtStart = mediator->Pos10msUnits();
		
		mediator->SetMarker(NULL, mediator->Pos0_1());

		stateMediaPlayer = STATE_MEDIAPLAYER_PLAYING;

		if (OnPlaybackStarted)	OnPlaybackStarted(true);
	}
	if (target->_text == "Stop")
	{
		if (stateMediaPlayer == STATE_MEDIAPLAYER_PLAYING)
		{
			//mediator->SetPos0_1(NULL, mediator->PosMarker0_1(), false);		// NULL has to trigger self update, nice!
			mediator->SetMarker(NULL, -1);
		}
		if ((stateMediaPlayer == STATE_MEDIAPLAYER_PLAYING_LOOPED) ||
			(stateMediaPlayer == STATE_MEDIAPLAYER_PLAYING_LOOPED_BACKFORTH))
		{
			//mediator->SetPos0_1(NULL, mediator->PosSelStart0_1(), false);	// NULL has to trigger self update, nice!
			mediator->SetMarker(NULL, -1);
		}

		stateMediaPlayer = STATE_MEDIAPLAYER_IDLE;
		if (OnPlaybackStarted)
			OnPlaybackStarted(false);
	}

	return true;
}


void MediaSubWindow::callback_RegisterTrackClip()
{
	TrackClipMenu::Get()->OnClick =	[this](int item)
	{
		switch(item)
		{
		case TrackClipMenu::ITEM_KEYFRAME_EDITING:
		{
			// GET SELECTED CLIP (CLIP HAS TO BE SELECTED BEFORE CALLING DELETE)
			TrackClip* clipSelected = TrackClip::GetSelectedClip();
			clipSelected->bKeyframeEditing = !clipSelected->bKeyframeEditing;

			break;
		}
		case TrackClipMenu::ITEM_DELETE:
		{
			// GET SELECTED CLIP (CLIP HAS TO BE SELECTED BEFORE CALLING DELETE)
			TrackClip* clipSelected = TrackClip::GetSelectedClip();

			// UNREGISTER ANIMATED PARAMS
			clipSelected->RegisterTRSparam(NULL);

			// REMOVE TOOL WINDOW FROM SIBLINGS LIST
			WarpingToolSubWindow::RemoveSibling(clipSelected->windowTool);

			// REMOVE CLIP FROM TRACKS LIST
			TrackClip::RemoveSelectedClip();

			// DELETE TOOL AND PARAMS WNDS & SELECT WELCOME TOOL
			clipSelected->OnClipChange(WARPING_TOOL_WELCOME);

			// DELETE CLIP
			windowTimeLine->DeleteGUIelement(clipSelected);

			break;
		}
		}
	};
}