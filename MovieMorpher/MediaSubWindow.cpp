#include "stdafx.h"
#include "MediaSubWindow.h"
#include "GLSL_Pipeline.h"
#include "../../!!adGUI/button.h"
#include "../../!!adExtensions/extensions.h"
#include "../../!!adGUI/TrackClip.h"


extern GLSL_Pipeline glsl_pipeline;
extern TextureBank texBank;

MediaSubWindow::MediaSubWindow(int iParentWidth, int iParentHeight,
							   float fBottomLeftXperc, float fBottomLeftYperc, float fWidthPerc, float fHeightPerc) :
	            OpenGLSubWindowWithGUI(iParentWidth, iParentHeight,
				         			   fBottomLeftXperc, fBottomLeftYperc, fWidthPerc, fHeightPerc)
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

	windowTimeLine = NULL;

	ButtonImage* buttonImg;
	buttonImg = new ButtonImage("", -300, 10, 30);
	buttonImg->LoadImg("Icons\\Image4.bmp");
	buttonImg->strHint = "Play selection in loop";
	buttonImg->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	liGUI_Elements.push_back(buttonImg);

	buttonImg = new ButtonImage("", -266, 10, 30);
	buttonImg->LoadImg("Icons\\Image7.bmp");
	buttonImg->strHint = "Play from start to end";
	buttonImg->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	liGUI_Elements.push_back(buttonImg);

	buttonImg = new ButtonImage("", -232, 10, 30);
	buttonImg->LoadImg("Icons\\Image6.bmp");
	buttonImg->strHint = "Pause playback";
	buttonImg->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	liGUI_Elements.push_back(buttonImg);

	buttonImg = new ButtonImage("", -198, 10, 30);
	buttonImg->LoadImg("Icons\\Image5.bmp");
	buttonImg->strHint = "Play from cursor";
	buttonImg->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	liGUI_Elements.push_back(buttonImg);

	buttonImg = new ButtonImage("", -164, 10, 30);
	buttonImg->LoadImg("Icons\\Image8.bmp");
	buttonImg->strHint = "Stop playback";
	buttonImg->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	liGUI_Elements.push_back(buttonImg);

	labelPlayhead = new Label("00:00:00.234", -301, -40, 11.9f);
	labelPlayhead->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	labelPlayhead->vColor = Vecc4(0.1f, 0.7f, 0.1f, 1.0f);
	liGUI_Elements.push_back(labelPlayhead);

	buttonImg = new ButtonImage("", -80, 20, 32);
	buttonImg->LoadImg("Icons\\Image9.bmp");
	buttonImg->strHint = "Add video track...";
	buttonImg->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	liGUI_Elements.push_back(buttonImg);

	buttonImg = new ButtonImage("", -80, -20, 32);
	buttonImg->LoadImg("Icons\\Image10.bmp");
	buttonImg->strHint = "Add image...";
	buttonImg->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	buttonImg->OnClick = [this]() {	return this->AddTrackPicture(); };
	liGUI_Elements.push_back(buttonImg);
}



bool MediaSubWindow::AddTrackPicture()
{
	windowTimeLine->AddClip(NULL);

	return true;
}



void MediaSubWindow::RenderGUI()
{
	PositionMediator* mediator = PositionMediator::Get();

	int hours   = int(mediator->Pos01()*mediator->Duration())/3600;
	int minutes = int(mediator->Pos01()*mediator->Duration())/60;
	int seconds = int(mediator->Pos01()*mediator->Duration())%60;
	
	double intpart;
	int milliseconds = int(modf(mediator->Pos01()*mediator->Duration(), &intpart)*1000);

	static char buf[256];
	snprintf(buf, sizeof(buf), "%02d:%02d:%02d.%03d", hours, minutes, seconds, milliseconds);

	labelPlayhead->_text = buf;

	OpenGLSubWindowWithGUI::RenderGUI();
}
