#ifndef MORPHINGTOOL_H
#define MORPHINGTOOL_H

#include <vector>
#include "../../!!adGUI/ToolBase.h"
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

class MorphingTool : public ToolBase
{
public:
	MorphingTool();
	~MorphingTool();

	virtual	void Render();

	virtual bool PassiveMotionFunc(Vec3 ptMouse);
	virtual bool MouseFunc(int button, int state, Vec3 ptMouse);
	virtual void MotionFunc(Vec3 ptMouse);
	virtual bool KeyboardFunc(unsigned char key, Vec3 ptMouse);

	bool SrcCurveIsDone() { return bSrcCurveIsDone; }
	bool DstCurveIsDone() { return bDstCurveIsDone; }

	void ClearSourceLine();
	void ClearDestinationLine();
	bool MorphNow();

	StateInput_enum stateCurrent;

	std::function<bool()> lambdaPointsAreVisible  = []() { return true; };
	std::function<void()> lambdaMakePointsVisible = []() {  };

protected:
	void UploadMorphingLines();

	Arrow*  arrow;
	Button* buttonSource;
	Button* buttonDestination;
	Button* buttonMorphNow;

	std::vector<Vec2> liSource;
	std::vector<Vec2> liDestination;
	Vec2 ptPrevPoint;

private:

	bool SourcePolylineClicked();
	bool DestinationPolylineClicked();
	void StartNextGeneration();

	bool m_bMouseDrawingInProgress;

	bool bSrcCurveIsDone;
	bool bDstCurveIsDone;

	Vec2* m_ptrDraggedPoint;
};

#endif