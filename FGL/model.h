#ifndef MODEL_H
#define MODEL_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

namespace fgl {

	class Model {

		struct {
			GLuint vertex_buffer, element_buffer;
			GLuint textures[2];

			GLuint vertex_shader, fragment_shader, program;

			struct {
				GLint timer;
				GLint textures[2];
			} uniforms;

			struct {
				GLint position;
			} attributes;

			GLfloat timer;
		} resources;

		int loadResources(std::string name);
	public:
		Model(const char* name);
		void draw();
	};
}

#endif