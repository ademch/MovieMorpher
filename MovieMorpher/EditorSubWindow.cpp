#include "stdafx.h"
#include "EditorSubWindow.h"
#include "GLSL_Pipeline.h"
#include "../../!!adGUI/glfont.h"
#include "../../!!adExtensions/extensions.h"
#include "../../!!adGlobals/TextureDescriptor.h"
#include "../../!!adGlobals/adOpenGLUtilities.h"
#include "../../!!adGlobals/wdir.h"
#include "MorphingTool.h"


extern GLSL_Pipeline glsl_pipeline;
extern TextureBank texBank;
extern GLFONT font;


EditorSubWindow::EditorSubWindow(int iParentWidth, int iParentHeight,
								float fBottomLeftXperc, float fBottomLeftYperc,
								float fWidthPerc, float fHeightPerc) :
	             OpenGLSubWindowWithGUI(iParentWidth, iParentHeight, 
										fBottomLeftXperc, fBottomLeftYperc, fWidthPerc, fHeightPerc)
{

	m_ParamsSubWindow = NULL;

	comboBox = new ComboBox("Default", -180, 10, 170, 6.3);
	comboBox->SetAlignment(HALIGN_RIGHT, VALIGN_BOTTOM);
	comboBox->bVisible = false;
	comboBox->bEnabled = false;
	liGUI_Elements.push_back(comboBox);

	buttonResetView = new Button("Reset view", -180, -30, 100, 6);
	buttonResetView->SetAlignment(HALIGN_RIGHT, VALIGN_TOP);
	buttonResetView->OnClick = [this]() { return ResetView(); };
	liGUI_Elements.push_back(buttonResetView);

	typeTexBankIter iter;
	for (iter = texBank.bank.begin(); iter != texBank.bank.end(); ++iter)
		comboBox->items.push_back(iter->first.c_str());

	comboBox->SetSelected(TEXTURE_MORPHED_IMAGE);

	MorphingTool* tool = new MorphingTool();
	tool->lambdaPointsAreVisible  = [this]() { return m_ParamsSubWindow ? m_ParamsSubWindow->PointsAreVisible() : true; };
	tool->lambdaMakePointsVisible = [this]() { return m_ParamsSubWindow->MakePointsVisible(); };
	liTools.push_back(tool);
}


EditorSubWindow::~EditorSubWindow()
{
	std::vector<GUI_Element*>::iterator iterElement;
	for (iterElement = liGUI_Elements.begin(); iterElement != liGUI_Elements.end(); iterElement++)
		delete (*iterElement);

	std::vector<ToolBase*>::iterator toolIter;
	for (toolIter = liTools.begin(); toolIter != liTools.end(); toolIter++)
		delete (*toolIter);
}

void EditorSubWindow::Render()
{
	OpenGLSubWindow::Render();

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	// Show output of the shader, while invisible input texture holds original
	TextureDescriptor* texDescr = texBank[TEXTURE_MORPHED_IMAGE];
	if (m_ParamsSubWindow->ShowOriginal())
		texDescr = texBank[TEXTURE_INPUT_IMAGE];

	//std::string sSelected = comboBox->GetSelected();
	RenderTexturedQuad(texDescr->m_uiTextureID,
				  	  -texDescr->m_width/2, -texDescr->m_height/2,
					   texDescr->m_width, texDescr->m_height);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	sprintf(m_strCaption, "%s %5.0f%%", "Zoom", fUserScale*100.0f);

	
	///////

	RenderGUI();

	// propagate common values
	for (auto iterElement : liTools)
		iterElement->fUserScale = fUserScale;

	SetupGraphicsPipeline();

		for (auto iterElement : liTools)
			iterElement->Render();

	SetupGraphicsPipelineWithIdentityModelViewMatrix();

		for (auto iterElement : liTools)
			iterElement->RenderGUI();

}


void EditorSubWindow::Reshape(int iBottomLeftX, int iBottomLeftY, int iWidth, int iHeight)
{
	OpenGLSubWindowWithGUI::Reshape(iBottomLeftX, iBottomLeftY, iWidth, iHeight);

	for (auto iterElement : liTools)
		iterElement->Reshape(iBottomLeftX, iBottomLeftY, iWidth, iHeight);
}


void EditorSubWindow::PassiveMotionFunc(int x, int y)
{
	OpenGLSubWindow::PassiveMotionFunc(x, y);

	if ((x > m_iBottomLeftX) && (x < m_iBottomLeftX + m_iWidth) &&
		(y > m_iBottomLeftY) && (y < m_iBottomLeftY + m_iHeight))
	{
		bool bResult = PassiveMotionFuncGUI(x, y);
		// if some GUI element of Editor handled the event then do not traverse further
		if (bResult) return;

		// Tools are hendled before their GUI
		SetupGraphicsPipeline();

			Vec3d v3DCoords;
			gluUnProjectFriendly(x, y, 0, v3DCoords.X, v3DCoords.Y, v3DCoords.Z);

			for (auto iterElement : liTools)
			{
				bResult = iterElement->PassiveMotionFunc(Vecc3(v3DCoords));
				// if some Tool handled the event then do not traverse further
				if (bResult) return;
			}

		// Tools' GUI are hendled after tools itself
		SetupGraphicsPipelineWithIdentityModelViewMatrix();

			gluUnProjectFriendly(x, y, 0, v3DCoords.X, v3DCoords.Y, v3DCoords.Z);

			for (auto iterElement : liTools)
			{
				bResult = iterElement->PassiveMotionFuncGUI(Vecc3(v3DCoords));
				// if some Tool GUI handled the event then do not traverse further
				if (bResult) return;
			}

		// Nobody handled the event
		if (!bResult) glutSetCursor(GLUT_CURSOR_INHERIT);
	}
}

// Mouse button clicked callback
void EditorSubWindow::MouseFunc(int button, int state, int x, int y)
{
	OpenGLSubWindow::MouseFunc(button, state, x, y);

	if ((x > m_iBottomLeftX) && (x < m_iBottomLeftX + m_iWidth) &&
		(y > m_iBottomLeftY) && (y < m_iBottomLeftY + m_iHeight))
	{
		// Process subwindow GUI first
		if (MouseFuncGUI(button, state, x, y)) return;

		SetupGraphicsPipeline();

			Vec3d v3DCoords;
			gluUnProjectFriendly(x, y, 0, v3DCoords.X, v3DCoords.Y, v3DCoords.Z);

			for (auto iterElement : liTools)
			{
				if (iterElement->MouseFunc(button, state, Vecc3(v3DCoords))) return;
			}

		SetupGraphicsPipelineWithIdentityModelViewMatrix();

			gluUnProjectFriendly(x, y, 0, v3DCoords.X, v3DCoords.Y, v3DCoords.Z);

			for (auto iterElement : liTools)
			{
				if (iterElement->MouseFuncGUI(button, state, Vecc3(v3DCoords))) return;
			}
	}

}

// Motion with button pressed
void EditorSubWindow::MotionFunc(int x, int y)
{
	OpenGLSubWindow::MotionFunc(x, y);

	if ((x > m_iBottomLeftX) && (x < m_iBottomLeftX + m_iWidth) &&
		(y > m_iBottomLeftY) && (y < m_iBottomLeftY + m_iHeight))
	{
		// Process GUI first
		MotionFuncGUI(x, y);

		SetupGraphicsPipeline();

			Vec3d v3DCoords;
			gluUnProjectFriendly(x, y, 0, v3DCoords.X, v3DCoords.Y, v3DCoords.Z);

			for (auto iterElement : liTools)
				iterElement->MotionFunc(Vecc3(v3DCoords));

		SetupGraphicsPipelineWithIdentityModelViewMatrix();

			gluUnProjectFriendly(x, y, 0, v3DCoords.X, v3DCoords.Y, v3DCoords.Z);

			for (auto iterElement : liTools)
				iterElement->MotionFuncGUI(Vecc3(v3DCoords));
	}

}

bool EditorSubWindow::ResetView()
{
	OpenGLSubWindow::ResetView();

	return true;
}


bool EditorSubWindow::KeyboardFunc(unsigned char key, int x, int y)
{
	bool res = false;

	if (OpenGLSubWindow::KeyboardFunc(key, x, y)) return true;

	switch (key)
	{
		// Load image
		case '2':
		{
			//ClearSourceLine();		TODO
			//ClearDestinationLine();   TODO
			//UploadMorphingLines();	TODO
			break;
		}
		// Start next generation
		case '4':
		{
			//StartNextGeneration();	TODO
			break;
		}
		break;
	}

	return res;
}
