#ifndef WARPINGTOOLSUBWINDOW_H
#define WARPINGTOOLSUBWINDOW_H

#include "MorphingToolSubWindow.h"
#include "ParamsSubWindow.h"
#include "../../!!adGUI/gui_element.h"
#include "../../!!adGUI/ComboBox.h"
#include "../../!!adGUI/button.h"
#include "../../!!adGUI/arrow.h"
#include <vector>
#include <functional>

class WarpingToolSubWindow : public MorphingToolSubWindow
{
public:
	WarpingToolSubWindow(int iParentWidth, int iParentHeight,
						 float fBottomLeftXperc, float fBottomLeftYperc,
						 float fWidthPerc, float fHeightPerc);
	~WarpingToolSubWindow() {}

	virtual	void Draw();

	virtual bool PassiveMotionFunc(int x, int y);
	virtual bool MouseFunc(int button, int state, int x, int y);
	virtual void MotionFunc(int x, int y);
	virtual bool KeyboardFunc(unsigned char key, int x, int y);


protected:


private:

};

#endif