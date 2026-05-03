// .cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Main.h"
#include "../../!!adGlobals/wdir.h"
#include "MorphFBOprocessor.h"
#include <windowsx.h>
#include "../../!!adExtensions/extensions.h"
#include "ImageSaveLoad.h"
#include "VideoSaveLoad.h"
#include "../../!!adGlobals/globalToolTip.h"

TextureBank  texBank;


int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	GLfloat light_position[] = { 100.0, 100.0, 100.0, 0.0 };

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

	printf("Loading font...");
		unsigned int iTextureFont;
		glGenTextures(1, &iTextureFont);
		if (!glFontCreate(&font, FullPathToFile("arial.glf"), iTextureFont))
		{
			printf("fail\n");
			return 1;
		}
	printf("done\n");

	fbo = new MorphFBOprocessor(0, 0, 800, 450);

	windowToolEditor = new MorphingToolSubWindow(iAppWndWidth,iAppWndHeight, 0.01,0.3, 0.70,0.68);
	sprintf(windowToolEditor->m_strCaption, "%s", "Zoom");
	windowToolEditor->lambdaPointsAreVisible  = []() { return windowParams ? windowParams->PointsAreVisible() : true; };
	windowToolEditor->lambdaMakePointsVisible = []() { return windowParams->MakePointsVisible(); };
	windowToolEditor->bSceneRotationAllowed = false;
	liWindows.push_back(windowToolEditor);

	windowParams = new ParamsSubWindow(iAppWndWidth,iAppWndHeight, 0.72,0.3, 0.27,0.68);
	sprintf(windowParams->m_strCaption, "%s", "Params");
	windowParams->bSceneRotationAllowed = false;
	windowParams->bSceneDragAllowed = false;
	windowParams->bSceneZoomAllowed = false;
	liWindows.push_back(windowParams);

	windowMedia = new MediaSubWindow(iAppWndWidth, iAppWndHeight, 0.01, 0.02, 0.98, 0.26);
	//sprintf(windowMedia->m_strCaption, "%s", "Timeline");
	windowMedia->bSceneRotationAllowed = false;
	windowMedia->bSceneDragAllowed = false;
	windowMedia->bSceneZoomAllowed = false;
	liWindows.push_back(windowMedia);

	timelineSliderWindow = new TimelineSliderSubWindow(iAppWndWidth, iAppWndHeight, 0.04, 0.22, 0.64, 0.026);
	timelineSliderWindow->bSceneRotationAllowed = false;
	timelineSliderWindow->bSceneDragAllowed = false;
	timelineSliderWindow->bSceneZoomAllowed = false;
	timelineSliderWindow->clrFrame = Vecc3(0.1, 0.5, 0.1);
	liWindows.push_back(timelineSliderWindow);

	timelineWindow = new TimelineSubWindow(iAppWndWidth, iAppWndHeight, 0.04, 0.07, 0.64, 0.15);
	timelineWindow->bSceneRotationAllowed = false;
	timelineWindow->bSceneDragAllowed = false;
	timelineWindow->bSceneZoomAllowed = false;
	timelineWindow->clrFrame = Vecc3(0.1, 0.5, 0.1);
	liWindows.push_back(timelineWindow);

	timelineScrollBarWindow = new TimelineScrollBarSubWindow(iAppWndWidth, iAppWndHeight, 0.04, 0.05, 0.64, 0.02);
	timelineScrollBarWindow->scrollBar->OnChange = [](Matr4 matrUserScale)
	{
		timelineSliderWindow->SetZoom(matrUserScale);
		timelineWindow->SetZoom(matrUserScale);
	};
	timelineScrollBarWindow->bSceneRotationAllowed = false;
	timelineScrollBarWindow->bSceneDragAllowed = false;
	timelineScrollBarWindow->bSceneZoomAllowed = false;
	timelineScrollBarWindow->bRenderGUIdecoration = false;
	timelineScrollBarWindow->clrFrame = Vecc3(0.1, 0.5, 0.1);
	liWindows.push_back(timelineScrollBarWindow);

	// register mutual pointers
	fbo->m_ParamsSubWindow        = windowParams;
	windowToolEditor->m_ParamsSubWindow = windowParams;

	ShowWindow(GetConsoleWindow(), SW_HIDE);

	VideoInit();

	ToolTip::Get();

	//передача управления циклу прорисовки сцены
	glutMainLoop();

	return 0;
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

	for (auto iterWindow : liWindows)
		iterWindow->Reshape(iAppWndWidth*iterWindow->fBottomLeftXperc,
							iAppWndHeight*iterWindow->fBottomLeftYperc,
			                iAppWndWidth*iterWindow->fWidthPerc,
							iAppWndHeight*iterWindow->fHeightPerc);
}

void globaldraw()
{
	//очистка активного буфера цвета кадра и буфера глубины
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();

	fbo->Render();

	for (auto iterWindow : liWindows)
		iterWindow->Render();

	Sleep(25);

	ToolTip::Get()->UpdateTimer();

	glFinish();

	//перевод заднего буфера в режим переднего, переднего в режим заднего
	glutSwapBuffers();
}


void keyboard(unsigned char key, int x, int y)
{
	bool res = false;
	
	for (auto iterWindow : liWindows) {
		res = iterWindow->KeyboardFunc(key, x, iAppWndHeight - y);
		if (res) break;
	}

	res = fbo->KeyboardFunc(key, x, y);
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
		// Load image
		case '2':
		{
			unsigned int width, height;
			unsigned char* image = ImageSaveLoadHelper::LoadImageFromDisk(width, height);

			if (image)
			{
				fbo->Reshape(0, 0, width, height);
				fbo->TextureUpdate(width, height, image);

				//TextureDescriptor* texDesc = LoadTexture(width, height, image);
				//free(image);

				//std::string id = msSince1970();
				//texBank.bank[id] = texDesc;

				//windowParams->listBox->items.push_back(id);
			}
			break;
		}
		// Save image
		case '5':
		{
			ImageSaveLoadHelper::SaveImageToDisk();
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
void MotionFunc(int x, int y)
{
	for (auto iterWindow : liWindows)
		iterWindow->MotionFunc(x, iAppWndHeight - y);
}

// Passive motion is special, global window has to care about all child windows
// to make sure focus, cursor is updated correcly
void PassiveMotionFunc(int x, int y)
{
	bool bResult = false;

	for (auto iterWindow : liWindows)
		bResult |= iterWindow->PassiveMotionFunc(x, iAppWndHeight - y);

	if (!bResult)
	{
		glutSetCursor(GLUT_CURSOR_INHERIT);

		ToolTip::Get()->Finish();
	}
}

void MouseFunc(int button, int state, int x, int y)
{
	for (auto iterWindow : liWindows)
		iterWindow->MouseFunc(button, state, x, iAppWndHeight - y);
}

void MouseWheelFunc(int state, int delta, int x, int y)
{
	// translate from screen to window coordinates
	int posX = glutGet(GLUT_WINDOW_X);
	int posY = glutGet(GLUT_WINDOW_Y);

	x = x - posX;
	y = y - posY;
	
	for (auto iterWindow : liWindows)
		iterWindow->MouseWheelFunc(state, delta, x, iAppWndHeight - y);
}

void MouseHWheelFunc(int state, int delta, int x, int y)
{
	// translate from screen to window coordinates
	int posX = glutGet(GLUT_WINDOW_X);
	int posY = glutGet(GLUT_WINDOW_Y);

	x = x - posX;
	y = y - posY;

	for (auto iterWindow : liWindows)
		iterWindow->MouseHWheelFunc(state, delta, x, iAppWndHeight - y);
}


static LRESULT CALLBACK winProcUser(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{

	switch (Msg)
	{
	case WM_MOUSEWHEEL:
		MouseWheelFunc(GET_KEYSTATE_WPARAM(wParam), GET_WHEEL_DELTA_WPARAM(wParam),
			           GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
		break;
	case WM_MOUSEHWHEEL:
		MouseHWheelFunc(GET_KEYSTATE_WPARAM(wParam), GET_WHEEL_DELTA_WPARAM(wParam),
					    GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
		break;
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

	delete fbo;

	glFontDestroy(&font);

	// Clean up and exit.
	printf("Exiting.\n");

	exit(0);
}

