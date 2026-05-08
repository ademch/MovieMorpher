#ifndef WARPINGTOOLSUBWINDOW_H
#define WARPINGTOOLSUBWINDOW_H

#include "MorphingToolSubWindow.h"
#include "ParamsSubWindow.h"
#include "../../!!adGUI/gui_element.h"
#include "../../!!adGUI/ComboBox.h"
#include "../../!!adGUI/button.h"
#include <vector>
#include <functional>


enum StateTransform_enum {
	STATE_TRANS_IDLE,
	STATE_TRANS_SCALE,
	STATE_TRANS_ROTATE,
	STATE_TRANS_TRANSLATE
};

class WarpingToolSubWindow : public MorphingToolSubWindow
{
public:
	WarpingToolSubWindow(int iParentWidth, int iParentHeight,
						 float fBottomLeftXperc, float fBottomLeftYperc,
						 float fWidthPerc, float fHeightPerc);
	~WarpingToolSubWindow() {}

	void Draw() override;

	bool PassiveMotionFunc(int x, int y) override;
	bool MouseFunc(int button, int state, int x, int y) override;
	void MotionFunc(int x, int y) override;
	bool KeyboardFunc(unsigned char key, int x, int y) override;

	StateTransform_enum stateTransform;

protected:

	std::vector<Vec2>   liScalingHandles;
	Vec2 ptTranslHandle;
	Vec2 ptRotateHandle;

	Matr4 matrImmediateVisualization;

private:

	Vec2 m_ptHandleClicked;
	Vec2 m_ptHandlePivot;
	Vec2 m_ptHandlePivotUp;
	Vec2 m_ptHandlePivotRight;

	Vec2 ptPrevPoint;

};

#endif