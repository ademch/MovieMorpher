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

	// We render liGUI_Elements through Draw to have matrUserScale applied during rendering of liGUI_Elements
	void RenderGUI() {};

	void Draw() override;

	Matr4 matrSliderNonInverted;

	void SetZoom(Matr4 _matrUserScale)
	{
		gluInvertMatrix(&_matrUserScale.m[0][0], &matrUserScale.m[0][0]);
		matrSliderNonInverted = _matrUserScale;
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