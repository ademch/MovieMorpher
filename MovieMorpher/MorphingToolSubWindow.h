#ifndef MORPHINGTOOLSUBWINDOW_H
#define MORPHINGTOOLSUBWINDOW_H

#include "../../!!adGUI/SubWindowWithGUI.h"
#include "ParamsSubWindow.h"
#include "MorphFBOprocessor.h"
#include "../../!!adGUI/gui_element.h"
#include "../../!!adGUI/button.h"
#include "../../!!adGUI/arrow.h"
#include <vector>
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
	~MorphingToolSubWindow()
	{
		delete fbo;
	}

	void Draw() override;
	virtual void DrawFBOquad();

	bool PassiveMotionFunc(int x, int y) override;
	bool MouseFunc(int button, int state, int x, int y) override;
	void MotionFunc(int x, int y) override;
	bool KeyboardFunc(unsigned char key, int x, int y) override;

	bool SrcCurveIsDone() { return bSrcCurveIsDone; }
	bool DstCurveIsDone() { return bDstCurveIsDone; }

	void ClearSourceLine();
	void ClearDestinationLine();
	bool MorphNow();

	void SetParamsSubWindow(ParamsSubWindow* m_Wnd)
	{
		m_ParamsSubWindow = m_Wnd;
	}
	ParamsSubWindow* GetParamsSubWindow() { return m_ParamsSubWindow; }
	MorphFBOprocessor* GetFBO()           { return fbo; }

protected:

	void PopulateGUI() override;

	void UploadMorphingLines();

	Arrow*  arrow;
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

	MorphFBOprocessor* fbo;

	void ReDrawFBO();

	ParamsSubWindow* m_ParamsSubWindow;

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