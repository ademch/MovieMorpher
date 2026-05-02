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

	void RenderGUI() {};


	//virtual void SetupGraphicsPipelineWithIdentityModelViewMatrix()
	//{
	//	// with this call we convert GUI behavior to a tool behavior,
	//	// ie. gui starts to work in the same coordinates as the tool
	//	OpenGLSubWindowWithGUI::SetupGraphicsPipeline();
	//};

	//// observe special modelview matix for this window
	//void RenderGUI() override
	//{
	//	SetupGraphicsPipeline();

	//	for (auto iterElement : liGUI_Elements)
	//		iterElement->Draw();
	//}

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