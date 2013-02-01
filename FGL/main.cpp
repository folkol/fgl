#include <Windows.h>
#include "fgl.h"

class MyGame : public fgl::Game {
public:
	void update() {
		fgl::Game::update();
	}

	void render() {
		glClearColor(1.0f, 1.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
	}
};

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	MyGame game;
	game.init();

	while(game.isRunning()) {
		game.update();
		game.render();
		game.show();
	}
}