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
	sliderMorphRatio = new Slider<SL_INT>(" Ratio", 30,80, 0,100, &fMorphRatio, 7);
	sliderMorphRatio->SetAlignment(HALIGN_LEFT, VALIGN_CENTER);
	sliderMorphRatio->SetBoxWidth(200);
	sliderMorphRatio->SetBoxSeparation(1);
	sliderMorphRatio->fValueGranularity = 1;
	sliderMorphRatio->fTickGranularity = 10;
	liGUI_Elements.push_back(sliderMorphRatio);

	fMorphRadius = 80;
	sliderMorphRadius = new Slider<SL_INT>(" Radius", 30,40, 0,500, &fMorphRadius, 7);
	sliderMorphRadius->SetAlignment(HALIGN_LEFT, VALIGN_CENTER);
	sliderMorphRadius->SetBoxWidth(200);
	sliderMorphRadius->SetBoxSeparation(1);
	sliderMorphRadius->fValueGranularity = 1;
	sliderMorphRadius->fTickGranularity = 30;
	liGUI_Elements.push_back(sliderMorphRadius);

	fMorphPower = 1.0f;
	sliderMorphPower = new SliderCenterLine("Power", 30,0, 0.1, 10.1, &fMorphPower, 7);
	sliderMorphPower->SetAlignment(HALIGN_LEFT, VALIGN_CENTER);
	sliderMorphPower->SetBoxWidth(200);
	sliderMorphPower->SetBoxSeparation(1);
	sliderMorphPower->fValueGranularity = 0.1;
	sliderMorphPower->fTickGranularity = 0.5;
	liGUI_Elements.push_back(sliderMorphPower);

	onoffswitchShadow = new OnOffFlipSwitch("Shadow", 30,-40, 6);
	onoffswitchShadow->SetAlignment(HALIGN_LEFT, VALIGN_CENTER);
	//onoffswitchShaders->OnPreClick = [this](bool bON_Request) { return CompileShaders(bON_Request); };
	liGUI_Elements.push_back(onoffswitchShadow);

	fTransparency = 100.0f;
	sliderTransparency = new Slider<SL_INT>("Alpha", 30,-100, 0,100, &fTransparency, 7);
	sliderTransparency->SetAlignment(HALIGN_LEFT, VALIGN_CENTER);
	sliderTransparency->SetBoxWidth(200);
	sliderTransparency->SetBoxSeparation(1);
	sliderTransparency->fValueGranularity = 1;
	sliderTransparency->fTickGranularity = 25;
	sliderTransparency->OnClick = [this]()
	{
		animatedfTransparency.SetValueAt(TrackClip::GetSelectedClipLocalTimeS(), fTransparency);

		return true;
	};
	sliderTransparency->OnClickDrag = sliderTransparency->OnClick;

	liGUI_Elements.push_back(sliderTransparency);

	animatedfTransparency.SetValueAt(0, fTransparency);
}

