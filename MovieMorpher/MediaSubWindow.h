#ifndef MEDIASUBSUBWINDOW_H
#define MEDIASUBSUBWINDOW_H

#include "../../!!adGUI/SubWindowWithGUI.h"
#include "../../!!adGUI/label.h"
#include "../../!!adGUI/ListBox.h"
#include "../../!!adGUI/button.h"


class MediaSubWindow : public OpenGLSubWindowWithGUI
{
public:
	MediaSubWindow(int iParentWidth, int iParentHeight,
				   float fBottomLeftXperc, float fBottomLeftYperc,
				   float fWidthPerc, float fHeightPerc);
	~MediaSubWindow();

protected:

private:
	ListBox* listBox;
	ButtonImage*  buttonImg;
	Label* label;
};


#endif