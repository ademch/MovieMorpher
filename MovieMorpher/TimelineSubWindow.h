#ifndef TIMELINESUBSUBWINDOW_H
#define TIMELINESUBSUBWINDOW_H

#include "../../!!adGUI/SubWindowWithGUI.h"
#include "../../!!adGUI/HorScrollBar.h"
#include "../../!!adGUI/VideoSlider.h"
#include "../../!!adGUI/button.h"
#include <vector>


class TimelineSubWindow : public OpenGLSubWindowWithGUI
{
public:
	TimelineSubWindow(int iParentWidth, int iParentHeight,
				      float fBottomLeftXperc, float fBottomLeftYperc,
				      float fWidthPerc, float fHeightPerc);
	~TimelineSubWindow();

	virtual void Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight);

	virtual	void Draw()
	{
		OpenGLSubWindowWithGUI::Draw();
	}

	virtual void SetupGraphicsPipelineWithIdentityModelViewMatrix() {};

	void SetZoom(Matr4 _matrUserScale)
	{
		gluInvertMatrix(&_matrUserScale.m[0][0], &matrUserScale.m[0][0]);
	}

protected:

private:
	VideoSlider*  videoSlider;

};

class TimelineSliderSubWindow : public OpenGLSubWindowWithGUI
{
public:
	TimelineSliderSubWindow(int iParentWidth, int iParentHeight,
							float fBottomLeftXperc, float fBottomLeftYperc,
							float fWidthPerc, float fHeightPerc);
	~TimelineSliderSubWindow();

	virtual void Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight);

	HorScrollBar* scrollBar;

protected:

private:


};


#endif