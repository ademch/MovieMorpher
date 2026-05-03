#include "stdafx.h"
#include "PreviewSubWindow.h"
#include "GLSL_Pipeline.h"
#include "../../!!adGlobals/TextureDescriptor.h"
#include "../../!!adGlobals/adOpenGLUtilities.h"
#include "MorphingTool.h"


extern TextureBank texBank;
extern GLFONT font;

PreviewSubWindow::PreviewSubWindow(int iParentWidth, int iParentHeight,
								 float fBottomLeftXperc, float fBottomLeftYperc,
								 float fWidthPerc, float fHeightPerc) :
				 ToolBoxSubWindowWithGUI(iParentWidth, iParentHeight, 
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


PreviewSubWindow::~PreviewSubWindow()
{
	for (auto* iterElement : liGUI_Elements)
		delete iterElement;

	for (auto* toolIter : liTools)
		delete toolIter;
}

void PreviewSubWindow::Draw()
{
	ToolBoxSubWindowWithGUI::Draw();

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

}


bool PreviewSubWindow::ResetView()
{
	OpenGLSubWindow::ResetView();

	return true;
}


bool PreviewSubWindow::KeyboardFunc(unsigned char key, int x, int y)
{
	bool res = false;

	if (ToolBoxSubWindowWithGUI::KeyboardFunc(key, x, y)) return true;

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
