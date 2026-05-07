#include "stdafx.h"
#include "MediaSubWindow.h"
#include "../../!!adGlobals/adOpenGLUtilities.h"
#include "GLSL_Pipeline.h"
#include "../../!!adExtensions/extensions.h"


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

	label = new Label("00:00:00.234", -300, -40, 12 );
	label->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	liGUI_Elements.push_back(label);

	buttonImg = new ButtonImage("", -80, 20, 32);
	buttonImg->LoadImg("Icons\\Image9.bmp");
	buttonImg->strHint = "Add video track...";
	buttonImg->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	liGUI_Elements.push_back(buttonImg);

	buttonImg = new ButtonImage("", -80, -20, 32);
	buttonImg->LoadImg("Icons\\Image10.bmp");
	buttonImg->strHint = "Add image...";
	buttonImg->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	liGUI_Elements.push_back(buttonImg);
}






