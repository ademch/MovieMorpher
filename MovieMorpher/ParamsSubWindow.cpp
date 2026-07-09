#include "stdafx.h"
#include "ParamsSubWindow.h"
#include "../../!!adGUI/TrackClip.h"
#include "../../!!adGUI/VideoPositionMediator.h"


ParamsSubWindow::ParamsSubWindow(int iParentWidth, int iParentHeight,
								 float fBottomLeftXperc, float fBottomLeftYperc,
								 float fWidthPerc, float fHeightPerc) :
				 OpenGLSubWindowWithGUI(iParentWidth, iParentHeight,
										fBottomLeftXperc, fBottomLeftYperc, fWidthPerc, fHeightPerc)
{
	PopulateGUI();

	PositionMediator::Get()->subscribeForPos(this, [this](void* origin, double fVal)
	{
		if (!bActive) return;
		fTransparency = animatedfTransparency.Evaluate( TrackClip::GetSelectedClipLocalTimeS() );
	});
}

ParamsSubWindow::~ParamsSubWindow()
{
	PositionMediator::Get()->unsubscribeForPos(this);
}


void ParamsSubWindow::PopulateGUI()
{
	fMorphRatio = 90;
	SliderMorphRatio = new Slider<SL_INT>(" Ratio", 30,80, 0,100, &fMorphRatio, 7);
	SliderMorphRatio->SetAlignment(HALIGN_LEFT, VALIGN_CENTER);
	SliderMorphRatio->SetBoxWidth(200);
	SliderMorphRatio->SetBoxSeparation(1);
	SliderMorphRatio->fValueGranularity = 1;
	SliderMorphRatio->fTickGranularity = 10;
	liGUI_Elements.push_back(SliderMorphRatio);

	fMorphRadius = 80;
	SliderMorphRadius = new Slider<SL_INT>(" Radius", 30,40, 0,500, &fMorphRadius, 7);
	SliderMorphRadius->SetAlignment(HALIGN_LEFT, VALIGN_CENTER);
	SliderMorphRadius->SetBoxWidth(200);
	SliderMorphRadius->SetBoxSeparation(1);
	SliderMorphRadius->fValueGranularity = 1;
	SliderMorphRadius->fTickGranularity = 30;
	liGUI_Elements.push_back(SliderMorphRadius);

	fMorphPower = 1.0f;
	SliderMorphPower = new SliderCenterLine("Power", 30,0, 0.1, 10.1, &fMorphPower, 7);
	SliderMorphPower->SetAlignment(HALIGN_LEFT, VALIGN_CENTER);
	SliderMorphPower->SetBoxWidth(200);
	SliderMorphPower->SetBoxSeparation(1);
	SliderMorphPower->fValueGranularity = 0.1;
	SliderMorphPower->fTickGranularity = 0.5;
	liGUI_Elements.push_back(SliderMorphPower);

	fTransparency = 100.0f;
	SliderTransparency = new Slider<SL_INT>("Alpha", 30,-70, 0,100, &fTransparency, 7);
	SliderTransparency->SetAlignment(HALIGN_LEFT, VALIGN_CENTER);
	SliderTransparency->SetBoxWidth(200);
	SliderTransparency->SetBoxSeparation(1);
	SliderTransparency->fValueGranularity = 1;
	SliderTransparency->fTickGranularity = 25;
	SliderTransparency->OnClick = [this]()
	{
		TrackClip* clip = TrackClip::GetSelectedClip();

		if (clip && clip->bKeyframeEditing)
			animatedfTransparency.SetValueAt(TrackClip::GetSelectedClipLocalTimeS(), fTransparency);
		else
			animatedfTransparency.SetValueAt(0.0, fTransparency);

		return true;
	};
	SliderTransparency->OnClickDrag = SliderTransparency->OnClick;

	liGUI_Elements.push_back(SliderTransparency);

	animatedfTransparency.SetValueAt(0, fTransparency);

	buttonMorphNext = new Button("Apply", 30,-130, 120, 6.3);
	buttonMorphNext->SetAlignment(HALIGN_LEFT, VALIGN_CENTER);
	buttonMorphNext->OnClick = [this]() { return StartNextGeneration(); };
	liGUI_Elements.push_back(buttonMorphNext);
}



bool ParamsSubWindow::StartNextGeneration()
{
	//get the HWND
	HWND handle = WindowFromDC(wglGetCurrentDC());
	PostMessage(handle, WM_KEYDOWN, 0x34, 0);	// 4

	return true;
}
