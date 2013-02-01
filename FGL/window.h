#ifndef WINDOW_H
#define WINDOW_H

#include <Windows.h>

namespace fgl {

	LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	class Window {
	HWND hWnd;

	public:
		Window();
		bool init();
		bool handleMessages();
	};

}

#endif