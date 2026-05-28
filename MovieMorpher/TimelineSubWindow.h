#ifndef TIMELINESUBSUBWINDOW_H
#define TIMELINESUBSUBWINDOW_H

#include "../../!!adGUI/SubWindowWithGUI.h"
#include "../../!!adGUI/HorScrollBar.h"
#include "../../!!adGUI/VideoSlider.h"
#include "../../!!adGUI/VideoPositionMediator.h"
#include "../../!!adGUI/TimelineTrack.h"
#include "../../!!adGUI/TrackClip.h"

extern const int g_iClipPadding;


class TimelineSliderSubWindow : public OpenGLSubWindowWithGUI
{
public:
	TimelineSliderSubWindow(int iParentWidth, int iParentHeight,
							float fBottomLeftXperc, float fBottomLeftYperc,
							float fWidthPerc, float fHeightPerc);
	~TimelineSliderSubWindow() {}

	int Width()  { return m_iWidth - iBorder*2;  }	// HIDES parent implementation

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

protected:

	void PopulateGUI() override;

private:

	int iBorder;

	Matr4 matrSliderNonInverted;

	VideoSlider*  videoSlider;
};


class TimelineScrollBarSubWindow : public OpenGLSubWindowWithGUI
{

public:
	TimelineScrollBarSubWindow(int iParentWidth, int iParentHeight,
							   float fBottomLeftXperc, float fBottomLeftYperc,
							   float fWidthPerc, float fHeightPerc);
	~TimelineScrollBarSubWindow() {}

	void Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight) override;

	HorScrollBar* scrollBar;

protected:
	void PopulateGUI() override;
};


class TimelineSubWindow : public OpenGLSubWindowWithGUI
{

public:
	TimelineSubWindow(int iParentWidth, int iParentHeight,
					  float fBottomLeftXperc, float fBottomLeftYperc,
					  float fWidthPerc, float fHeightPerc);
	~TimelineSubWindow() {}

	int Width()  { return m_iWidth - iBorder*2;  }	// HIDES parent implementation

	std::function<void(double)>         OnSliderPosChange;
	std::function<void(double, double)> OnSelectionChange;
	std::function<void(Vec3)>           OnVerticalPanChange;

	void Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight) override;

	// We render liGUI_Elements through Draw to have matrUserScale applied during rendering of liGUI_Elements
	void RenderGUI() {};

	void Draw() override;
	bool MouseWheelFunc(int state, int delta, int x, int y) override;
	bool MouseFunc(int button, int state, int x, int y) override;
	void MotionFunc(int x, int y) override;

	void SetSliderPos0_1(double _val);

	void SetZoom(Matr4 _matrUserScale)
	{
		gluInvertMatrix(&_matrUserScale.m[0][0], &matrUserScale.m[0][0]);
		matrSliderNonInverted = _matrUserScale;
	}

	TrackClip* AddClip(OpenGLSubWindowWithGUI* wndTool, int iFramesCount);

	void SetSwitchToolWindowCallback(std::function<void(OpenGLSubWindowWithGUI*)> callback)
	{
		m_setWindowCallback = callback;
	}

protected:

	void PopulateGUI() override;

	double m_fSliderPos01;

	bool  bSelectionIsValid;
	double m_fSelectionStartX0_1;
	double m_fSelectionEndX0_1;

	int iVerticalPan;
	Matr4 matrSliderNonInverted;

	bool bMouseButtonPressed;
	int  iStartDragX, iStartDragY;

	int iBorder;

	int FindLastClipOnTrack_Tail10msTick(int iTrackNumber);

private:

	std::function<void(OpenGLSubWindowWithGUI*)> m_setWindowCallback;

};


class TrackParamsSubWindow : public OpenGLSubWindowWithGUI
{
public:
	TrackParamsSubWindow(int iParentWidth, int iParentHeight,
					     float fBottomLeftXperc, float fBottomLeftYperc,
					     float fWidthPerc, float fHeightPerc);
	~TrackParamsSubWindow() {}

	//std::function<void(float)>      OnChange;
	std::function<void(Vec3)> OnVerticalPanChange;


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
	void PopulateGUI() override;

private:

};



#endif