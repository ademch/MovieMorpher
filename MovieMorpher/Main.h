#pragma once

//#include "resource.h"
#include "../../!!adGlobals/glut/glut.h"
#include "../../!!adGUI/glfont.h"
#include "../../!!adGUI/SubWindowWithGUI.h"
#include <vector>
#include "GLSL_Pipeline.h"
#include "MorphFBOprocessor.h"
#include <chrono>

#include "WarpingToolSubWindow.h"
#include "ParamsSubWindow.h"
#include "MediaSubWindow.h"
#include "TimelineSubWindow.h"

#include "../../!!adGlobals/globalToolTip.h"


GLFONT font;

int iAppWndWidth = 1200;
int iAppWndHeight = 800;


// объявления прототипов
void ReshapeFunc(GLsizei w, GLsizei h);
void globaldraw();
void keyboard(unsigned char, int, int);
void keyboardSpecial(int key, int x, int y);
void MouseFunc(int, int, int, int);
void MouseWheelFunc(int, int, int, int);
void MouseHWheelFunc(int, int, int, int);
void MotionFunc(int x, int y);
void PassiveMotionFunc(int x, int y);
void WMClose();

static WNDPROC currentWndProc;
static HWND handle = NULL;
static LRESULT CALLBACK winProcUser(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam);

std::vector<OpenGLSubWindowWithGUI*> liWindows;

GLSL_Pipeline glsl_pipeline;

MorphFBOprocessor* fbo;

WarpingToolSubWindow*       windowToolEditor;
ParamsSubWindow*            windowParams;
MediaSubWindow*             windowMedia;

TimelineSliderSubWindow*    timelineSliderWindow;
TimelineSubWindow*          timelineWindow;
TimelineScrollBarSubWindow* timelineScrollBarWindow;
TrackParamsSubWindow*       timelineTrackParams;

ToolTip* tooltip;


std::string msSince1970()
{
	using namespace std::chrono;

	auto now = system_clock::now();
	auto ms = duration_cast<milliseconds>(now.time_since_epoch()).count();

	return std::to_string(ms);
}


