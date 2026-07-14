#ifndef MORPHINGTOOLSUBWINDOW_H
#define MORPHINGTOOLSUBWINDOW_H

#include "../../!!adGUI/SubWindowWithGUI.h"
#include "ParamsSubWindow.h"
#include "MorphFBOprocessor.h"
#include "../../!!adGUI/button.h"
#include <functional>


enum StateInput_enum {
	STATE_IDLE,
	STATE_SOURCE_DRAWING_INPUT, STATE_SOURCE_POINT_INPUT,
	STATE_DESTINATION_DRAWING_INPUT, STATE_DESTINATION_POINT_INPUT,
	STATE_POINT_DRAG
};


class MorphingToolSubWindow : public OpenGLSubWindowWithGUI
{
public:
	MorphingToolSubWindow(int iParentWidth, int iParentHeight,
						  float fBottomLeftXperc, float fBottomLeftYperc,
						  float fWidthPerc, float fHeightPerc);
	~MorphingToolSubWindow();

	AnimatedParamPolyline2D animatedPolylineSrc;
	AnimatedParamPolyline2D animatedPolylineDst;

	virtual void RecalcAnimatedParamsFromKeyframes();

	void Draw() override;

	bool PassiveMotionFunc(int x, int y) override;
	bool MouseFunc(int button, int state, int x, int y) override;
	void MotionFunc(int x, int y) override;
	bool KeyboardFunc(unsigned char key, int x, int y) override;

	void ClearSourceLine();
	void ClearDestinationLine();
	bool MorphNow();

	void SetParamsSubWindow(ParamsSubWindow* m_Wnd)
	{
		m_ParamsSubWindow = m_Wnd;
	}
	ParamsSubWindow* GetParamsSubWindow() { return m_ParamsSubWindow; }

	void ReshapeFBOprocessors(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight);
	void TextureUpdateInputFBOprocessor(int iWidth, int iHeight, unsigned char* image);

	void ReDrawFBOprocessors();

protected:

	void PopulateGUI() override;

	void UploadMorphingLines();

	Button* buttonResetView;
	Button* buttonSource;
	Button* buttonDestination;
	Button* buttonMorphNow;
	Button* buttonClear;

	bool ResetView();
	bool ClearMorph();

	std::vector<Vec2> liSource;
	std::vector<Vec2> liDestination;
	Vec2 ptPrevPoint;

	MorphFBOprocessor* morphFBOprocessor;

	ParamsSubWindow* m_ParamsSubWindow;

	TextureBank  texBank;

	double GetClipLocalTimeS();

	void SaveMorphingLinesIntoAnimationSequence();

private:
	StateInput_enum stateCurrent;

	bool m_bMouseDrawingInProgress;
	Vec2* m_ptrDraggedPoint;

	bool bSrcCurveIsDone;
	bool bDstCurveIsDone;

	bool SourcePolylineClicked();
	bool DestinationPolylineClicked();

	void StartNextGeneration();

};

#endif