// .cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "Main.h"
#include "../../!!adGlobals/wdir.h"
#include "MorphFBOprocessor.h"
#include <windowsx.h>
#include "ImageSaveLoad.h"
#include "../../!!adExtensions/extensions.h"
#include "../../!!adGUI/VideoPositionMediator.h"
#include "../../!!adGlobals/JPG/JPEG_library.h"
#include "../../!!adExtensions/debugger.h"
#include "../../!!adGUI/TrackClipMenu.h"



int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	GLfloat light_position[] = { 100.0, 100.0, 100.0, 0.0 };

	//freopen("log.txt", "w", stdout);
	//freopen("log.txt", "w", stderr);

	//инициализация драйвера OpenGL
	glutInit(&argc, (char**)argv);
	//установка размеров будущего окна приложения
	glutInitWindowSize(iAppWndWidth, iAppWndHeight);
	//запрос на создание контекста окна приложения с участием буфера цвета, буфера глубины, двойной буферизацией
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	//создание и регистрация класса окна приложения
	glutCreateWindow("MovieMorpher");

	//регистрация функции обратного вызова установки размеров окна
	glutReshapeFunc(ReshapeFunc);
	//регистрация функции обратного вызова простоя приложения
	glutIdleFunc(globaldraw);
	//регистрация функции обратного вызова принудительной перерисовки окна 
	glutDisplayFunc(globaldraw);

	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboardSpecial);
	glutMouseFunc(MouseFunc);
	glutMotionFunc(MotionFunc);
	glutPassiveMotionFunc(PassiveMotionFunc);
	glutWMCloseFunc(WMClose);

	//get the HWND
	handle = WindowFromDC(wglGetCurrentDC());

	//store the current message event handler for the window
	currentWndProc = (WNDPROC)GetWindowLongPtr(handle, GWL_WNDPROC);

	//tell the window to use now our event handler!
	SetWindowLongPtr(handle, GWL_WNDPROC, (long)winProcUser);

	//установка координат точеченого источника освещения номер 0
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	//разрешение применения освещения при расчете цветов объектов сцены
	glEnable(GL_LIGHTING);
	//включение в расчет источника освещения номер 0
	glEnable(GL_LIGHT0);
	//включение алгоритма z-буффера
	glEnable(GL_DEPTH_TEST);

	//установка объекта влияния функции glColor
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	//разрешение изменения цвета по функции glColor при включенном освещении
	glEnable(GL_COLOR_MATERIAL);

	printf("Renderer: %s\n",     glGetString(GL_RENDERER));
	printf("Version GL: %s\n",   glGetString(GL_VERSION));
	printf("Version GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	GLint size;
	glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &size);
	printf("Max UBO size: %d\n", size);

	if (!CheckExtensions()) return 1;
	linkExtensions();

	{
		glDebugMessageCallback(GLDebugCallback, nullptr);

		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

		glDebugMessageControlDisableDeprecatedMessages();
	}

	printf("Loading font...");
		unsigned int iTextureFont;
		glGenTextures(1, &iTextureFont);
		if (!glFontCreate(&font, FullPathToFile("arial.glf"), iTextureFont))
		{
			printf("fail\n");
			return 1;
		}
	printf("done\n");

	ConstructToolAndParamsSubWindows("");
	// load default welcome image into the first window
	{
		unsigned int width, height;
		unsigned char* image = NULL;
		read_JPEG_file( FullPathToFile("Data\\Welcome.jpg"), &image, width, height );
		if (image)
		{
			ImageSaveLoadHelper::_FlipImage(image, width, height);

			windowToolEditor->ReshapeFBOprocessors(0, 0, width, height);
			windowToolEditor->TextureUpdateInputFBOprocessor(width, height, image);

			free(image);
		}
		windowToolEditorDefault = windowToolEditor;
		windowParamsDefault     = windowParams;
	}

	windowGlobalParams = new GlobalParamsSubWindow(iAppWndWidth,iAppWndHeight, 0.74,0.35, 0.25,0.2);
	windowGlobalParams->SetFlags(ROTATION_ALLOWED_FALSE | DRAG_ALLOWED_FALSE | ZOOM_ALLOWED_FALSE);
	liWindows.push_back(windowGlobalParams);

	windowMedia = new MediaSubWindow(iAppWndWidth, iAppWndHeight, 0.01, 0.02, 0.98, 0.31);
	windowMedia->SetFlags(ROTATION_ALLOWED_FALSE | DRAG_ALLOWED_FALSE | ZOOM_ALLOWED_FALSE);
	windowMedia->OnNewMedia = ConstructToolAndParamsSubWindows;
	windowMedia->OnPlaybackStarted = OnPlayback;
	liWindows.push_back(windowMedia);


	timelineSliderWindow = new TimelineSliderSubWindow(iAppWndWidth, iAppWndHeight, 0.08, 0.27, 0.65, 0.024);
	timelineSliderWindow->SetFlags(ROTATION_ALLOWED_FALSE | DRAG_ALLOWED_FALSE | ZOOM_ALLOWED_FALSE);
	timelineSliderWindow->clrFrame = Vecc3(0.1, 0.5, 0.1);
	liWindows.push_back(timelineSliderWindow);


	timelineWindow = new TimelineSubWindow(iAppWndWidth, iAppWndHeight, 0.08, 0.07, 0.65, 0.2);
	timelineWindow->SetFlags(ROTATION_ALLOWED_FALSE | DRAG_ALLOWED_FALSE | ZOOM_ALLOWED_FALSE);
	timelineWindow->clrFrame = Vecc3(0.1, 0.5, 0.1);
	timelineWindow->SetSwitchToolWindowCallback(OnToolWindowSwitch);
	liWindows.push_back(timelineWindow);

	timelineTrackParams = new TrackParamsSubWindow(iAppWndWidth, iAppWndHeight, 0.02, 0.07, 0.06, 0.2);
	timelineTrackParams->SetFlags(ROTATION_ALLOWED_FALSE | DRAG_ALLOWED_FALSE | ZOOM_ALLOWED_FALSE | GUI_DECORATION_FALSE);
	timelineTrackParams->clrFrame = Vecc3(0.1, 0.5, 0.1);
	liWindows.push_back(timelineTrackParams);

	//timelineTrackTransp = new TrackTranspSubWindow(iAppWndWidth, iAppWndHeight, 0.73, 0.07, 0.085, 0.2);
	//timelineTrackTransp->SetFlags(ROTATION_ALLOWED_FALSE | DRAG_ALLOWED_FALSE | ZOOM_ALLOWED_FALSE | GUI_DECORATION_FALSE);
	//timelineTrackTransp->clrFrame = Vecc3(0.1, 0.5, 0.1);
	//liWindows.push_back(timelineTrackTransp);


	timelineScrollBarWindow = new TimelineScrollBarSubWindow(iAppWndWidth, iAppWndHeight, 0.08, 0.048, 0.65, 0.022);
	timelineScrollBarWindow->SetFlags(ROTATION_ALLOWED_FALSE | DRAG_ALLOWED_FALSE | ZOOM_ALLOWED_FALSE );
	timelineScrollBarWindow->scrollBar->OnChange = [](Matr4 matrUserScale)
	{
		timelineSliderWindow->SetZoom(matrUserScale);
		timelineWindow->SetZoom(matrUserScale);
	};
	timelineScrollBarWindow->clrFrame = Vecc3(0.1, 0.5, 0.1);
	liWindows.push_back(timelineScrollBarWindow);


	// register mutual pointers
	windowMedia->SetTimelineSubWindow(timelineWindow);

	timelineWindow->OnVerticalPanChange = [](Vec3 vTranslation)
	{
		timelineTrackParams->SetVerticalTranslation(vTranslation); 
//		timelineTrackTransp->SetVerticalTranslation(vTranslation); 
	};

	//ShowWindow(GetConsoleWindow(), SW_HIDE);

	PositionMediator::Get()->Init(NULL, 0.0f, 10*60*100);	// 10 [min] x 60 [s] x 100 [10ms]

	ToolTip::Get();

	TrackClipMenu::Get()->Create(handle);

	//передача управления циклу прорисовки сцены
    glutMainLoop();

	return 0;
}


OpenGLSubWindowWithGUI* ConstructToolAndParamsSubWindows(char *title)
{
	if (windowToolEditor)
		windowToolEditor->bActive = false;
	if (windowParams)
		windowParams->bActive = false;

	WarpingToolSubWindow* new_windowToolEditor;
	ParamsSubWindow*      new_windowParams;

	new_windowToolEditor = new WarpingToolSubWindow(iAppWndWidth,iAppWndHeight, 0.01,0.35, 0.72,0.63);
	sprintf(new_windowToolEditor->m_strCaption, "%s", "Zoom");
	new_windowToolEditor->bSceneRotationAllowed = false;
	liWindows.push_back(new_windowToolEditor);

	new_windowParams = new ParamsSubWindow(iAppWndWidth,iAppWndHeight, 0.74,0.57, 0.25,0.41);
	new_windowParams->SetFlags(ROTATION_ALLOWED_FALSE | DRAG_ALLOWED_FALSE | ZOOM_ALLOWED_FALSE);
	sprintf(new_windowParams->m_strCaption, "%s", title);
	liWindows.push_back(new_windowParams);

	// register mutual pointers
	new_windowToolEditor->SetParamsSubWindow(new_windowParams);

	// Reshape exactly these windows
	ReshapeWindow(new_windowToolEditor);
	ReshapeWindow(new_windowParams);

	// Copy user view from previous to a new window
	if (windowToolEditor)
		new_windowToolEditor->CopyView(windowToolEditor);

	// These become pointers to active window pair
	windowToolEditor = new_windowToolEditor;
	windowParams     = new_windowParams;

	// windowParams can always be deducted from tool
	return windowToolEditor;
}


void OnPlayback(bool bStarted)
{
	// activate/deactivate current windows
	windowToolEditor->bActive = !bStarted;
	windowParams->bActive     = !bStarted;
}



void OnToolWindowSwitch(OpenGLSubWindowWithGUI* switchedWnd)
{
	std::cout << "Switching window request...";

	if ((windowToolEditor) && (!windowToolEditor->bActive))
	{
		std::cout << "Blocked during playback";
		return;
	}

	if (switchedWnd != windowToolEditor)
	{
		std::cout << "Done\n";

		if (switchedWnd == WARPING_TOOL_WELCOME)
		{
			windowToolEditor = windowToolEditorDefault;
			windowParams     = windowParamsDefault;

			windowToolEditor->bActive = true;
			windowParams->bActive     = true;

			return;
		}

		// deactivate current windows
		windowToolEditor->bActive = false;
		windowParams->bActive     = false;

		WarpingToolSubWindow* new_windowToolEditor;
		ParamsSubWindow*      new_windowParams;

		new_windowToolEditor = dynamic_cast<WarpingToolSubWindow*>(switchedWnd);
		new_windowParams     = new_windowToolEditor->GetParamsSubWindow();

		// Copy user view from previous to a new window
		// NB: Seems to be redundant here as params are copied on Draw
		new_windowToolEditor->CopyView(windowToolEditor);

		// activate new windows
		new_windowToolEditor->bActive = true;
		new_windowParams->bActive     = true;

		// These become pointers to active window pair
		windowToolEditor = new_windowToolEditor;
		windowParams     = new_windowParams;
	}
	else
		std::cout << "The same\n";
}


void ReshapeWindow(OpenGLSubWindowWithGUI* wind)
{
	wind->Reshape(iAppWndWidth  * wind->fBottomLeftXperc, iAppWndHeight * wind->fBottomLeftYperc,
				  iAppWndWidth  * wind->fWidthPerc, 	  iAppWndHeight * wind->fHeightPerc);
}


void ReshapeFunc(GLsizei w, GLsizei h)
{
	iAppWndWidth  = w;
	iAppWndHeight = h;

	//установка преобраования: прямоугольник сцены- прямоугольник окна экрана
	glViewport(0, 0, w, h);
	//выбор стека матриц проекций 
	glMatrixMode(GL_PROJECTION);
	//инициализация верхней матрицы на стеке единичной матрицой
	glLoadIdentity();
	//умножение верхней матрицы на стеке на матрицу параллельного проецирования
	glOrtho(-w/2.0f, w/2.0f, -h/2.0f, h/2.0f, -200, 200);

	//	gluPerspective (30.0,                   // Field-of-view angle
	//		            float(w)/float(h),      // Aspect ratio of viewing volume
	//		            .1,						// Distance to near clipping plane
	//		            200.0);					// Distance to far clipping plane

	//выбор стека матриц модельно-видовых преобразований
	glMatrixMode(GL_MODELVIEW);

	for (auto iterWindow : liWindows) {
		ReshapeWindow(iterWindow);
	}
}

void globaldraw()
{
	//очистка активного буфера цвета кадра и буфера глубины
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();

	for (auto iterWindow : liWindows)
	{
		if (auto wnd = dynamic_cast<WarpingToolSubWindow*>(iterWindow)) {
			wnd->CopyView(windowToolEditor);
		}
		if (!iterWindow->bActive) continue;

		iterWindow->Render();
	}

	Sleep(20);

	ToolTip::Get()->UpdateTimer();

	glFinish();

	//перевод заднего буфера в режим переднего, переднего в режим заднего
	glutSwapBuffers();
}


void keyboard(unsigned char key, int x, int y)
{
	bool res = false;
	
	for (auto iterWindow : liWindows)
	{
		if (!iterWindow->bActive) continue;

		res = iterWindow->KeyboardFunc(key, x, iAppWndHeight - y);
		if (res) break;
	}

//	res = fbo->KeyboardFunc(key, x, y);TODO
	if (res) return;

	switch (key)
	{
		case '`':
		{
			if ( IsWindowVisible( GetConsoleWindow() ) )
				ShowWindow(GetConsoleWindow(), SW_HIDE);
			else
				ShowWindow(GetConsoleWindow(), SW_SHOW);

			break;
		}
		case 27:
		case 'Q':
			break;
	}
}

// Handle subwindows resize
void keyboardSpecial(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
	{
		break;
	}
	case GLUT_KEY_DOWN:
	{
		break;
	}
	case GLUT_KEY_RIGHT:
	{

		break;
	}
	case GLUT_KEY_LEFT:
	{
		break;
	}
	break;
	}

}

// (in) x,y - window coords from (0,0) to (w,h)
void keyboardAux(int key, int state, int x, int y)
{
	for (auto iterWindow : liWindows)
	{
		if (!iterWindow->bActive) continue;
		
		iterWindow->KeyboardAux(key, state, x, iAppWndHeight - y);
	}
}

// (in) x,y - window coords from (0,0) to (w,h)
void MotionFunc(int x, int y)
{
	for (auto iterWindow : liWindows)
	{
		if (!iterWindow->bActive) continue;
		
		iterWindow->MotionFunc(x, iAppWndHeight - y);
	}
}


// Passive motion is special, global window has to care about all child windows
// to make sure focus, cursor is updated correcly
void PassiveMotionFunc(int x, int y)
{
	bool bResult = false;

	for (auto iterWindow : liWindows)
	{
		if (!iterWindow->bActive) continue;
		
		bResult |= iterWindow->PassiveMotionFunc(x, iAppWndHeight - y);
	}

	if (!bResult)
	{
		glutSetCursor(GLUT_CURSOR_INHERIT);

		ToolTip::Get()->Finish();
	}
}

void MouseFunc(int button, int state, int x, int y)
{
	bool bResult;

	for (auto iterWindow : liWindows)
	{
		if (!iterWindow->bActive) continue;

		bResult = iterWindow->MouseFunc(button, state, x, iAppWndHeight - y);
		if (bResult) break;
	}
}

void MouseWheelFunc(int state, int delta, int x, int y)
{
	// translate from screen to window coordinates
	int posX = glutGet(GLUT_WINDOW_X);
	int posY = glutGet(GLUT_WINDOW_Y);

	x = x - posX;
	y = y - posY;
	
	for (auto iterWindow : liWindows)
	{
		if (!iterWindow->bActive) continue;
		
		iterWindow->MouseWheelFunc(state, delta, x, iAppWndHeight - y);
	}
}

void MouseHWheelFunc(int state, int delta, int x, int y)
{
	// translate from screen to window coordinates
	int posX = glutGet(GLUT_WINDOW_X);
	int posY = glutGet(GLUT_WINDOW_Y);

	x = x - posX;
	y = y - posY;

	for (auto iterWindow : liWindows)
	{
		if (!iterWindow->bActive) continue;

		iterWindow->MouseHWheelFunc(state, delta, x, iAppWndHeight - y);
	}
}


static LRESULT CALLBACK winProcUser(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{

	switch (Msg)
	{
	case WM_MOUSEWHEEL:
		MouseWheelFunc(GET_KEYSTATE_WPARAM(wParam),
			           GET_WHEEL_DELTA_WPARAM(wParam),
			           GET_X_LPARAM(lParam),
			           GET_Y_LPARAM(lParam));
		return 0;
		break;
	
	case WM_MOUSEHWHEEL:
		MouseHWheelFunc(GET_KEYSTATE_WPARAM(wParam),
			            GET_WHEEL_DELTA_WPARAM(wParam),
					    GET_X_LPARAM(lParam),
			            GET_Y_LPARAM(lParam));
		return 0;
		break;
	
	case WM_KEYDOWN:
	{
		POINT p;

		GetCursorPos(&p);
		ScreenToClient(hwnd, &p);

		keyboardAux(wParam, GLUT_DOWN, p.x, p.y);

		return CallWindowProc(currentWndProc, handle, Msg, wParam, lParam);
		break;
	}
	case WM_KEYUP:
	{
		POINT p;

		GetCursorPos(&p);
		ScreenToClient(hwnd, &p);

		keyboardAux(wParam, GLUT_UP, p.x, p.y);

		return CallWindowProc(currentWndProc, handle, Msg, wParam, lParam);
		break;
	}
	default:
		//printf("%d\n", Msg);

		return CallWindowProc(currentWndProc, handle, Msg, wParam, lParam);
		break;
	}

	return 0;
}


void WMClose()
{
	// Life: Lambda functions refer to killed windows
	for (auto* window : liWindows)
		delete window;

	glFontDestroy(&font);

	// Clean up and exit.
	printf("Exiting.\n");

	exit(0);
}

