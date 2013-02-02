#include "window.h"

#include "fgl.h"

fgl::Window::Window(int x, int y) {
	this->width = x;
	this->height = y;
}

bool fgl::Window::init() {
	static TCHAR szAppName[] = TEXT("Simple Window");
	WNDCLASS wndclass;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = GetModuleHandle(NULL);
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;

	wndclass.lpszClassName = szAppName;

	if (!RegisterClass(&wndclass)) {
		MessageBox(NULL, TEXT("This program requires Windows NT!"), szAppName, MB_ICONERROR);
		return 0;
	}

	hWnd = CreateWindow(szAppName,
		TEXT("Simple Application Window"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		NULL,
		NULL,
		GetModuleHandle(NULL),
		NULL);

	initOGL();

	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);

	return false;
}

bool fgl::Window::initOGL() {
	hDC = GetDC( hWnd );
	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory( &pfd, sizeof( pfd ) );
	pfd.nSize = sizeof( pfd );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16;
	pfd.iLayerType = PFD_MAIN_PLANE;
	int iFormat = ChoosePixelFormat( hDC, &pfd );
	SetPixelFormat( hDC, iFormat, &pfd );

	hRC = wglCreateContext( hDC );

	wglMakeCurrent( hDC, hRC );

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		glewGetString(err);
		MessageBox(hWnd, L"GLEW init failed\n", L"NO GLEW", MB_OK);
	}
	if (!GLEW_VERSION_2_0) {
		MessageBox(hWnd, L"OpenGL 2.0 not available\n", L"NO OGL", MB_OK);
		return 1;
	}

	return true;
}

void fgl::Window::disableOpenGL() {
    wglMakeCurrent( NULL, NULL );
    wglDeleteContext( hRC );
    ReleaseDC( hWnd, hDC );
}

bool fgl::Window::handleMessages() {
	MSG msg;
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

	if(msg.message == WM_QUIT) {
		return false;
	}

    return true;
}

void fgl::Window::swapBuffers() {
	SwapBuffers(hDC);
}

LRESULT CALLBACK fgl::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rect;

	switch (message) {
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		GetClientRect(hwnd, &rect);
		DrawText(hdc, TEXT("Hello world!"), -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		EndPaint(hwnd, &ps);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_SIZE:
		glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
		return 0;
	}

	return DefWindowProc(hwnd, message, wParam, lParam);
}