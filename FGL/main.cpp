#include <Windows.h>

#include "fgl.h"

class MyGame : public fgl::Game {
	fgl::Model* model;

public:
	void init() {
		fgl::Game::init(400, 300);
		model = new fgl::Model("square_2d");
	}

	void update() {
		fgl::Game::update();
	}

	void render() {
		glClearColor(0.7f, 0.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		model->draw();
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