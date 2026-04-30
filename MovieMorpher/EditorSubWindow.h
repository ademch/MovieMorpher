#ifndef EDITORSUBWINDOW_H
#define EDITORSUBWINDOW_H

#include "../../!!adGUI/SubWindowWithGUI.h"
#include <vector>
#include "../../!!adGUI/gui_element.h"
#include "../../!!adGUI/ComboBox.h"
#include "../../!!adGUI/button.h"
#include "../../!!adGUI/arrow.h"
#include <vector>
#include "ParamsSubWindow.h"
#include "../../!!adGUI/ToolBase.h"


class EditorSubWindow : public OpenGLSubWindowWithGUI
{
public:
	EditorSubWindow(int iParentWidth, int iParentHeight,
					float fBottomLeftXperc, float fBottomLeftYperc,
					float fWidthPerc, float fHeightPerc);
	~EditorSubWindow();

	virtual	void Draw();

	void Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight);

	virtual bool PassiveMotionFunc(int x, int y);
	virtual bool MouseFunc(int button, int state, int x, int y);
	virtual void MotionFunc(int x, int y);
	virtual bool KeyboardFunc(unsigned char key, int x, int y);

	ParamsSubWindow* m_ParamsSubWindow;

protected:

	std::vector<ToolBase*> liTools;

	ComboBox* comboBox;
	Button*   buttonResetView;

	bool ResetView();

private:

};

#endif