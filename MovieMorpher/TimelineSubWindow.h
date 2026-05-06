#ifndef TIMELINESUBSUBWINDOW_H
#define TIMELINESUBSUBWINDOW_H

#include "../../!!adGUI/SubWindowWithGUI.h"
#include "../../!!adGUI/HorScrollBar.h"
#include "../../!!adGUI/VideoSlider.h"
#include "../../!!adGUI/TimelineTrack.h"
#include "../../!!adGUI/button.h"
#include <vector>
#include "../../!!adGUI/VideoPositionMediator.h"


class TimelineSliderSubWindow : public OpenGLSubWindowWithGUI
{
public:
	TimelineSliderSubWindow(int iParentWidth, int iParentHeight,
							float fBottomLeftXperc, float fBottomLeftYperc,
							float fWidthPerc, float fHeightPerc,
							PositionMediator* mediator);
	~TimelineSliderSubWindow();

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

	VideoSlider*  videoSlider;

protected:

private:

};


class TimelineScrollBarSubWindow : public OpenGLSubWindowWithGUI
{
public:
	TimelineScrollBarSubWindow(int iParentWidth, int iParentHeight,
							float fBottomLeftXperc, float fBottomLeftYperc,
							float fWidthPerc, float fHeightPerc);
	~TimelineScrollBarSubWindow();

	virtual void Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight);

	HorScrollBar* scrollBar;

protected:

private:

};


class TimelineSubWindow : public OpenGLSubWindowWithGUI
{
public:
	TimelineSubWindow(int iParentWidth, int iParentHeight,
					  float fBottomLeftXperc, float fBottomLeftYperc,
					  float fWidthPerc, float fHeightPerc,
					  PositionMediator* mediator);
	~TimelineSubWindow();

	std::function<void(float)>      OnChange;

	virtual void Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight);

	// We render liGUI_Elements through Draw to have matrUserScale applied during rendering of liGUI_Elements
	void RenderGUI() {};

	void Draw() override;
	bool MouseWheelFunc(int state, int delta, int x, int y) override;

	Matr4 matrSliderNonInverted;

	void SetPos(float _val);

	void SetZoom(Matr4 _matrUserScale)
	{
		gluInvertMatrix(&_matrUserScale.m[0][0], &matrUserScale.m[0][0]);
		matrSliderNonInverted = _matrUserScale;
	}

protected:

	float m_fSliderX;

	int iVerticalPan;

private:


};

#endif