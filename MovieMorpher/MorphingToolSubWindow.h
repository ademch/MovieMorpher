#ifndef MORPHINGTOOLSUBWINDOW_H
#define MORPHINGTOOLSUBWINDOW_H

#include "../../!!adGUI/SubWindowWithGUI.h"
#include "ParamsSubWindow.h"
#include "MorphFBOprocessor.h"
#include "../../!!adGUI/gui_element.h"
#include "../../!!adGUI/ComboBox.h"
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

protected:
	void UploadMorphingLines();

	ComboBox* comboBox;
	Arrow*  arrow;
	Button* buttonResetView;
	Button* buttonSource;
	Button* buttonDestination;
	Button* buttonMorphNow;

	bool ResetView();

	std::vector<Vec2> liSource;
	std::vector<Vec2> liDestination;
	Vec2 ptPrevPoint;

	MorphFBOprocessor* fbo;

	void ReDrawFBO();

	ParamsSubWindow* m_ParamsSubWindow;

private:
	StateInput_enum stateCurrent;

	bool m_bMouseDrawingInProgress;

	bool bSrcCurveIsDone;
	bool bDstCurveIsDone;

	bool SourcePolylineClicked();
	bool DestinationPolylineClicked();

	void StartNextGeneration();

	Vec2* m_ptrDraggedPoint;
};

#endif