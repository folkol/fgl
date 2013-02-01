#include <Windows.h>
#include "fgl.h"

class MyGame : public FGL {
};

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	MyGame game;

	while(game.isRunning()) {
		game.update();
		game.render();
	}
}