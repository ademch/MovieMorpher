#ifndef EDITORSUBWINDOW_H
#define EDITORSUBWINDOW_H

#include "../../!!adGUI/ToolBoxSubWindowWithGUI.h"
#include "../../!!adGUI/ComboBox.h"
#include "../../!!adGUI/button.h"
#include "ParamsSubWindow.h"


class PreviewSubWindow : public ToolBoxSubWindowWithGUI
{
public:
	PreviewSubWindow(int iParentWidth, int iParentHeight,
					 float fBottomLeftXperc, float fBottomLeftYperc,
					 float fWidthPerc, float fHeightPerc);
	~PreviewSubWindow();

	virtual	void Draw() override;
	virtual bool KeyboardFunc(unsigned char key, int x, int y);

	ParamsSubWindow* m_ParamsSubWindow;

protected:

	ComboBox* comboBox;
	Button*   buttonResetView;

	bool ResetView();

private:

};

#endif