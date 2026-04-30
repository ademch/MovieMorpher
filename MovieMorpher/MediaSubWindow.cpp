#include "stdafx.h"
#include "MediaSubWindow.h"
#include "../../!!adGlobals/adOpenGLUtilities.h"
#include "../../!!adGUI/fps.h"
#include "GLSL_Pipeline.h"
#include "../../!!adExtensions/extensions.h"


extern GLSL_Pipeline glsl_pipeline;
extern TextureBank texBank;

MediaSubWindow::MediaSubWindow(int iParentWidth, int iParentHeight,
							   float fBottomLeftXperc, float fBottomLeftYperc, float fWidthPerc, float fHeightPerc) :
	            OpenGLSubWindowWithGUI(iParentWidth, iParentHeight,
				         			   fBottomLeftXperc, fBottomLeftYperc, fWidthPerc, fHeightPerc)
{
	listBox = new ListBox("Resources", -400,-40, 200, 5, 6.3);
	listBox->SetAlignment(HALIGN_RIGHT, VALIGN_CENTER);
	listBox->items.push_back("First");
	listBox->items.push_back("Second1234567890123456789");
	listBox->items.push_back("Third");
	listBox->items.push_back("Fourth");
	listBox->items.push_back("Fifth");
	listBox->items.push_back("Sixth");
	listBox->items.push_back("Seven");
	listBox->items.push_back("Eight");
	liGUI_Elements.push_back(listBox);

}

MediaSubWindow::~MediaSubWindow()
{
	std::vector<GUI_Element*>::iterator iterElement;
	for (iterElement = liGUI_Elements.begin(); iterElement != liGUI_Elements.end(); iterElement++)
		delete (*iterElement);
}




