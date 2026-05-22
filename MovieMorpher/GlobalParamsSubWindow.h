#ifndef GLOBALPARAMSSUBSUBWINDOW_H
#define GLOBALPARAMSSUBSUBWINDOW_H

#include "../../!!adGUI/SubWindowWithGUI.h"
#include "../../!!adGUI/OnOffFlipSwitch.h"
#include "../../!!adGUI/button.h"
#include <vector>


class GlobalParamsSubWindow : public OpenGLSubWindowWithGUI
{
public:
	GlobalParamsSubWindow(int iParentWidth, int iParentHeight,
						  float fBottomLeftXperc, float fBottomLeftYperc,
						  float fWidthPerc, float fHeightPerc);
	~GlobalParamsSubWindow() {}

	bool ShouldShowOriginal();

	bool PointsAreVisible();
	bool IsWireframeShown();
	void MakePointsVisible();

	static GlobalParamsSubWindow* Get();

protected:

	bool CompileShaders(bool);

	void PopulateGUI() override;

private:

	static GlobalParamsSubWindow* _this;

	OnOffFlipSwitch* onoffswitchWireframe;
	OnOffFlipSwitch* onoffswitchShowPoints;
	OnOffFlipSwitch* onoffpushbuttonOriginal;
};


#endif