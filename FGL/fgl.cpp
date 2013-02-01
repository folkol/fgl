#include "fgl/fgl.h"

fgl::Game::Game() : running(true) {}
bool fgl::Game::init() { return false; }
bool fgl::Game::isRunning() { return running; }
void fgl::Game::update() { running = false; }
void fgl::Game::render() {}
void fgl::Game::stop() { running = false; }