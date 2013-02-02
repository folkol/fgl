#ifndef FGL_H
#define FGL_H

#include "window.h"
#include "model.h"

#define GLEW_STATIC
#include <gl/glew.h>

namespace fgl {

	class Game {
	Window* window;
		bool running;
	public:
		Game();
		bool init(int x = 800, int y = 600);
		bool isRunning();
		void update();
		void render();
		void show();
		void stop();
	};

}

#endif