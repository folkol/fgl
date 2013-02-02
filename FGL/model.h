#ifndef MODEL_H
#define MODEL_H

#define GLEW_STATIC
#include <GL/glew.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

namespace fgl {

	class Model {

		struct {
			GLuint vertex_buffer, element_buffer;
			GLuint textures[2];

			GLuint vertex_shader, fragment_shader, program;

			struct {
				GLint fade_factor;
				GLint textures[2];
			} uniforms;

			struct {
				GLint position;
			} attributes;

			GLfloat fade_factor;
		} resources;

		int loadResources();
	public:
		Model(const char* name);
		void draw();
	};
}

#endif