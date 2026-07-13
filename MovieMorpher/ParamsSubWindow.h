#ifndef PARAMS_SUBWINDOW_H
#define PARAMS_SUBWINDOW_H

#include "../../!!adGUI/SubWindowWithGUI.h"
#include "../../!!adGUI/Slider.h"
#include "../../!!adGUI/button.h"
#include "../../!!adGlobals/VideoAnimatedParams.h"


class SliderCenterLine : public Slider<SL_FLOAT>
{
public:
	SliderCenterLine(std::string strCaption, int x, int y, float _v_min, float _v_max, float* _v_cur, float scale) :
	Slider(strCaption, x,y, _v_min,_v_max, _v_cur, scale) {};

	void Draw()
	{
		Slider::Draw();

		glColor3f(0.9, 0.0, 0.0);
		glBegin(GL_LINES);
			glVertex3f(posx + m_iWidth / 10.0, posy, 5);
			glVertex3f(posx + m_iWidth / 10.0, posy + m_iHeight, 5);
		glEnd();
	}
};


class ParamsSubWindow : public OpenGLSubWindowWithGUI
{
public:
	ParamsSubWindow(int iParentWidth, int iParentHeight,
					float fBottomLeftXperc, float fBottomLeftYperc,
					float fWidthPerc, float fHeightPerc);
	~ParamsSubWindow();

	float MorphRadius()	{ return fMorphRadius;  }
	float MorphRatio()	{ return fMorphRatio;   }
	float MorphPower()	{ return fMorphPower;   }
	float Transparency(double time) { return animatedfTransparency.Evaluate(time); }

protected:

	void PopulateGUI() override;

	bool StartNextGeneration();

private:

	Slider<SL_INT>*   SliderMorphRatio;
	Slider<SL_INT>*   SliderMorphRadius;
	SliderCenterLine* SliderMorphPower;
	Slider<SL_INT>*   SliderTransparency;

	Button*			  buttonMorphNext;

	float fMorphRadius;
	float fMorphRatio;
	float fMorphPower;
	float fTransparency;

	AnimatedParamFloat animatedfTransparency;
};


#endif