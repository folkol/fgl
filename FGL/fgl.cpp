#include "fgl.h"

fgl::Game::Game() : running(true) {}

bool fgl::Game::init() {
	window = new Window();
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
void fgl::Game::stop() { running = false; }