#ifndef FGL_H
#define FGL_H

#include "window.h"

namespace fgl {

	class Game {
	Window* window;
		bool running;
	public:
		Game();
		bool init();
		bool isRunning();
		void update();
		void render();
		void stop();
	};

}

#endif