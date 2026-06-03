#ifndef WARPINGTOOLSUBWINDOW_H
#define WARPINGTOOLSUBWINDOW_H

#include "MorphingToolSubWindow.h"


enum StateTransform_enum {
	STATE_TRANS_IDLE,
	STATE_TRANS_SCALE_PROPORTIONAL,
	STATE_TRANS_SCALE_NONPROPORTIONAL,
	STATE_TRANS_ROTATE,
	STATE_TRANS_ROTATE_W_STEPS,
	STATE_TRANS_TRANSLATE
};

#define WARPING_TOOL_WELCOME	NULL


class WarpingToolSubWindow : public MorphingToolSubWindow
{
public:
	WarpingToolSubWindow(int iParentWidth, int iParentHeight,
						 float fBottomLeftXperc, float fBottomLeftYperc,
						 float fWidthPerc, float fHeightPerc);
	~WarpingToolSubWindow() {}

	static void RemoveSibling(OpenGLSubWindowWithGUI* _sibling);

	void Draw() override;

	bool PassiveMotionFunc(int x, int y) override;
	bool MouseFunc(int button, int state, int x, int y) override;
	void MotionFunc(int x, int y) override;
	void KeyboardAux(int key, int state, int x, int y) override;

	void SetupGraphicsPipeline() override;
	void DrawFBOquad();

protected:

	std::vector<Vec2>   liScalingHandles;
	Vec2 ptTranslHandle;
	Vec2 ptRotateHandle;

	// matrix for immediate visualization
	Matr4 matrImmediateVisualization;
	
	// matrix calculated how to go from fbo coordinates that are static to a new configuration
	Matr4 matrObjectOrigin2joystickBasis;

	StateTransform_enum stateTransform;

	static std::vector<WarpingToolSubWindow*> m_liSiblings;

private:

	// immediate named copies of the handles' coordinates
	Vec2 m_ptHandleClicked;
	Vec2 m_ptHandlePivot;
	Vec2 m_ptHandlePivotUp;
	Vec2 m_ptHandlePivotRight;

	Vec2 ptPrevPoint;

	HCURSOR hCursorScaleProportional;
	HCURSOR hCursorRotateAngle;

	// shortcut for fbo width and height
	int m_iJoystickFrameWidth;
	int m_iJoystickFrameHeight;

};

#endif