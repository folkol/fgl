#ifndef WINDOW_H
#define WINDOW_H

#include <Windows.h>

namespace fgl {

	LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	class Window {
	HWND hWnd;
	HDC hDC;
	HGLRC hRC;

	public:
		Window();
		bool init();
		bool initOGL();
		void disableOpenGL();
		bool handleMessages();
		void swapBuffers();
	};

}

#endif