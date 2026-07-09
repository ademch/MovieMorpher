#ifndef PARAMS_SUBWINDOW_H
#define PARAMS_SUBWINDOW_H

#include "../../!!adGUI/SubWindowWithGUI.h"
#include "../../!!adGUI/Slider.h"
#include "../../!!adGUI/button.h"


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


class ParamKeyframeFloat
{
public:
	double time;
	float value;
};

class ParamKeyframeTRSTransform
{
public:
	double time;
	TRSTransform value;
};


class AnimatedParamFloat
{
public:
	std::vector<ParamKeyframeFloat> liKeys;

	float Evaluate(double time)
	{
		if (liKeys.empty())
			return 0;

		if (liKeys.size() == 1)
			return liKeys[0].value;

		if (time <= liKeys.front().time)
			return liKeys.front().value;

		if (time >= liKeys.back().time)
			return liKeys.back().value;

		for (size_t i = 0; i + 1 < liKeys.size(); ++i)
		{
			const ParamKeyframeFloat& a = liKeys[i];
			const ParamKeyframeFloat& b = liKeys[i + 1];

			if (time >= a.time && time <= b.time)
			{
				double u = (time - a.time) / (b.time - a.time);

				return a.value*(1.0-u) + b.value*u;
			}
		}

		return liKeys.back().value;
	}


	void SetValueAt(double time, float value)
	{
		for (ParamKeyframeFloat& k : liKeys)
		{
			if (fabs(k.time - time) < 1e-6)
			{
				k.value = value;
				return;
			}
		}

		ParamKeyframeFloat k{time, value};

		auto it = std::lower_bound(	liKeys.begin(),
									liKeys.end(),
									time,
									[](const ParamKeyframeFloat& k, double time)
									{
										return k.time < time;
									}
								  );

		liKeys.insert(it, k);
	}
};


class AnimatedParamTRSTransform
{
public:
	std::vector<ParamKeyframeTRSTransform> liKeys;

	Matr4 Evaluate(double time)
	{
		if (liKeys.empty())
			return Mat4MakeIdent();

		if (liKeys.size() == 1)
			return Mat4Compose(liKeys[0].value);

		if (time <= liKeys.front().time)
			return Mat4Compose(liKeys.front().value);

		if (time >= liKeys.back().time)
			return Mat4Compose(liKeys.back().value);

		for (size_t i = 0; i + 1 < liKeys.size(); ++i)
		{
			const ParamKeyframeTRSTransform& a = liKeys[i];
			const ParamKeyframeTRSTransform& b = liKeys[i + 1];

			if (time >= a.time && time <= b.time)
			{
				double u = (time - a.time) / (b.time - a.time);

				return Mat4Compose( TRSTransformInterpolate(a.value,b.value, u) );
			}
		}

		return Mat4Compose(liKeys.back().value);
	}


	void SetValueAt(double time, TRSTransform value)
	{
		for (ParamKeyframeTRSTransform& k : liKeys)
		{
			if (fabs(k.time - time) < 1e-4)
			{
				k.value = value;
				return;
			}
		}

		ParamKeyframeTRSTransform k{time, value};

		auto it = std::lower_bound(	liKeys.begin(),
									liKeys.end(),
									time,
									[](const ParamKeyframeTRSTransform& k, double time)
									{
										return k.time < time;
									}
								  );

		liKeys.insert(it, k);
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