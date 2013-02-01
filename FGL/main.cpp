#include <Windows.h>
#include "fgl.h"

class MyGame : public fgl::Game {
public:
	void update() {
		fgl::Game::update();
	}
};

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	MyGame game;
	game.init();

	while(game.isRunning()) {
		game.update();
		game.render();
	}
}