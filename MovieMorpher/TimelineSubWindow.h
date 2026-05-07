#ifndef TIMELINESUBSUBWINDOW_H
#define TIMELINESUBSUBWINDOW_H

#include "../../!!adGUI/SubWindowWithGUI.h"
#include "../../!!adGUI/HorScrollBar.h"
#include "../../!!adGUI/VideoSlider.h"
#include "../../!!adGUI/button.h"
#include <vector>
#include "../../!!adGUI/VideoPositionMediator.h"


class TimelineSliderSubWindow : public OpenGLSubWindowWithGUI
{
	VideoSlider*  videoSlider;
	
	Matr4 matrSliderNonInverted;

public:
	TimelineSliderSubWindow(int iParentWidth, int iParentHeight,
							float fBottomLeftXperc, float fBottomLeftYperc,
							float fWidthPerc, float fHeightPerc,
							PositionMediator* mediator);
	~TimelineSliderSubWindow();

	void Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight) override;

	// We render liGUI_Elements through Draw to have matrUserScale applied during rendering of liGUI_Elements
	// matrSliderNonInverted is applied manually during mouse calls because float can not work through int types of Mouse handlers
	void RenderGUI() override {}

	void Draw() override;

	void SetZoom(Matr4 _matrUserScale)
	{
		gluInvertMatrix(&_matrUserScale.m[0][0], &matrUserScale.m[0][0]);
		matrSliderNonInverted = _matrUserScale;
	}

};


class TimelineScrollBarSubWindow : public OpenGLSubWindowWithGUI
{

public:
	TimelineScrollBarSubWindow(int iParentWidth, int iParentHeight,
							float fBottomLeftXperc, float fBottomLeftYperc,
							float fWidthPerc, float fHeightPerc);
	~TimelineScrollBarSubWindow();

	void Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight) override;

	HorScrollBar* scrollBar;
};


class TimelineSubWindow : public OpenGLSubWindowWithGUI
{

public:
	TimelineSubWindow(int iParentWidth, int iParentHeight,
					  float fBottomLeftXperc, float fBottomLeftYperc,
					  float fWidthPerc, float fHeightPerc,
					  PositionMediator* mediator);
	~TimelineSubWindow();

	std::function<void(float)>        OnSliderPosChange;
	std::function<void(float, float)> OnSelectionChange;
	std::function<void(Vec3)>         OnVerticalPanChange;

	void Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight) override;

	// We render liGUI_Elements through Draw to have matrUserScale applied during rendering of liGUI_Elements
	void RenderGUI() {};

	void Draw() override;
	bool MouseWheelFunc(int state, int delta, int x, int y) override;
	bool MouseFunc(int button, int state, int x, int y) override;
	void MotionFunc(int x, int y) override;

	void SetSliderPos(float _val);

	void SetZoom(Matr4 _matrUserScale)
	{
		gluInvertMatrix(&_matrUserScale.m[0][0], &matrUserScale.m[0][0]);
		matrSliderNonInverted = _matrUserScale;
	}

protected:

	float m_fSliderX;

	bool bSelectionIsValid;
	float m_fSelectionStartX;
	float m_fSelectionEndX;

	int iVerticalPan;
	Matr4 matrSliderNonInverted;

	bool bMouseButtonPressed;
	int  iStartDragX, iStartDragY;

	float GetSliderValue();
	float GetSelectionStartValue();
	float GetSelectionEndValue();

};


class TrackParamsSubWindow : public OpenGLSubWindowWithGUI
{
public:
	TrackParamsSubWindow(int iParentWidth, int iParentHeight,
					     float fBottomLeftXperc, float fBottomLeftYperc,
					     float fWidthPerc, float fHeightPerc);
	~TrackParamsSubWindow();

	//std::function<void(float)>      OnChange;
	std::function<void(Vec3)> OnVerticalPanChange;


	void Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight) override;

	// GUI is movable with MV matrix
	void SetupGraphicsPipelineWithIdentityModelViewMatrix() override
	{
		SetupGraphicsPipeline();
	}

	void SetVerticalTranslation(Vec3 _translation)
	{
		vUserSceneTranslation = _translation;
	}

protected:

private:

};



#endif