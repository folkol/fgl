#include "fgl.h"

fgl::Game::Game() : running(true) {}

bool fgl::Game::init(int x, int y) {
	window = new Window(x, y);
	window->init();
	return true;
}

bool fgl::Game::isRunning() {
	return running;
}

void fgl::Game::update() {
	running = window->handleMessages();
}

void fgl::Game::render() {}

void fgl::Game::show() {
	window->swapBuffers();
}

void fgl::Game::stop() { running = false; }