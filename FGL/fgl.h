#ifndef FGL_H
#define FGL_H

class FGL {
	bool running;
public:
	FGL() { running = true; }
	bool isRunning() { return running; }
	bool update() { running = false; }
	bool render() { }
};

#endif