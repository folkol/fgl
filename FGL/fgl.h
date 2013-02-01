#ifndef FGL_H
#define FGL_H

namespace fgl {
	class Game {
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