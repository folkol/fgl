#ifndef WINDOW_H
#define WINDOW_H

#include <Windows.h>

namespace fgl {

	LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	class Window {
	HWND hWnd;
	HDC hDC;
	HGLRC hRC;
	int width, height;

	public:
		Window(int width = 800, int height = 600);
		bool init();
		bool initOGL();
		void disableOpenGL();
		bool handleMessages();
		void swapBuffers();
	};

}

#endif