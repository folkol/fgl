#include "model.h"

#include <Windows.h>

static void displayGLError(GLuint object, PFNGLGETSHADERIVPROC glGet__iv, PFNGLGETSHADERINFOLOGPROC glGet__InfoLog) {
	GLint log_length;
	char *log;

	glGet__iv(object, GL_INFO_LOG_LENGTH, &log_length);
	log = (char*) malloc(log_length);
	glGet__InfoLog(object, log_length, NULL, log);
	fprintf(stderr, log);
	MessageBoxA(NULL, log, "Error", MB_OK);
	free(log);
}

void *file_contents(const char *filename, GLint *length) {
	FILE *f = fopen(filename, "r");
	void *buffer;

	if (!f) {
		fprintf(stderr, "Unable to open %s for reading\n", filename);
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	*length = ftell(f);
	fseek(f, 0, SEEK_SET);

	buffer = malloc(*length+1);
	*length = fread(buffer, 1, *length, f);
	fclose(f);
	((char*)buffer)[*length] = '\0';

	return buffer;
}

static short le_short(unsigned char *bytes) {
	return bytes[0] | ((char)bytes[1] << 8);
}

void *read_tga(const char *filename, int *width, int *height) {
	struct tga_header {
		char  id_length;
		char  color_map_type;
		char  data_type_code;
		unsigned char  color_map_origin[2];
		unsigned char  color_map_length[2];
		char  color_map_depth;
		unsigned char  x_origin[2];
		unsigned char  y_origin[2];
		unsigned char  width[2];
		unsigned char  height[2];
		char  bits_per_pixel;
		char  image_descriptor;
	} header;
	int i, color_map_size, pixels_size;
	FILE *f;
	size_t read;
	void *pixels;

	f = fopen(filename, "rb");

	if (!f) {
		fprintf(stderr, "Unable to open %s for reading\n", filename);
		return NULL;
	}

	read = fread(&header, 1, sizeof(header), f);

	if (read != sizeof(header)) {
		fprintf(stderr, "%s has incomplete tga header\n", filename);
		fclose(f);
		return NULL;
	}
	if (header.data_type_code != 2) {
		fprintf(stderr, "%s is not an uncompressed RGB tga file\n", filename);
		fclose(f);
		return NULL;
	}
	if (header.bits_per_pixel != 24) {
		fprintf(stderr, "%s is not a 24-bit uncompressed RGB tga file\n", filename);
		fclose(f);
		return NULL;
	}

	for (i = 0; i < header.id_length; ++i)
		if (getc(f) == EOF) {
			fprintf(stderr, "%s has incomplete id string\n", filename);
			fclose(f);
			return NULL;
		}

		color_map_size = le_short(header.color_map_length) * (header.color_map_depth/8);
		for (i = 0; i < color_map_size; ++i)
			if (getc(f) == EOF) {
				fprintf(stderr, "%s has incomplete color map\n", filename);
				fclose(f);
				return NULL;
			}

			*width = le_short(header.width); *height = le_short(header.height);
			pixels_size = *width * *height * (header.bits_per_pixel/8);
			pixels = malloc(pixels_size);

			read = fread(pixels, 1, pixels_size, f);
			fclose(f);

			if (read != pixels_size) {
				fprintf(stderr, "%s has incomplete image\n", filename);
				free(pixels);
				return NULL;
			}

			return pixels;
}

static GLuint make_shader(GLenum type, const char *filename) {
	GLint length;
	GLchar *source = (GLchar*) file_contents(filename, &length);
	GLuint shader;
	GLint shader_ok;

	if (!source)
		return 0;
	shader = glCreateShader(type);
	glShaderSource(shader, 1, (const GLchar**)&source, &length);
	free(source);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_ok);
	if (!shader_ok) {
		fprintf(stderr, "Failed to compile %s:\n", filename);
		displayGLError(shader, glGetShaderiv, glGetShaderInfoLog);
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

static GLuint make_buffer(GLenum target, const void *buffer_data, GLsizei buffer_size) {
		GLuint buffer;
		glGenBuffers(1, &buffer);
		glBindBuffer(target, buffer);
		glBufferData(target, buffer_size, buffer_data, GL_STATIC_DRAW);
		return buffer;
}

static const GLfloat g_vertex_buffer_data[] = { 
	-1.0f, -1.0f,
	1.0f, -1.0f,
	-1.0f,  1.0f,
	1.0f,  1.0f
};
static const GLushort g_element_buffer_data[] = { 0, 1, 2, 3 };


static GLuint make_texture(const char *filename) {
	GLuint texture;
	int width, height;
	void *pixels = read_tga(filename, &width, &height);

	if (!pixels)
		return 0;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

	glTexImage2D(
		GL_TEXTURE_2D, 0,           /* target, level of detail */
		GL_RGB8,                    /* internal format */
		width, height, 0,           /* width, height, border */
		GL_BGR, GL_UNSIGNED_BYTE,   /* external format, type */
		pixels                      /* pixels */
		);
	free(pixels);
	return texture;
}


static GLuint make_program(GLuint vertex_shader, GLuint fragment_shader) {
	GLint program_ok;

	GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
	if (!program_ok) {
		fprintf(stderr, "Failed to link shader program:\n");
		displayGLError(program, glGetProgramiv, glGetProgramInfoLog);
		glDeleteProgram(program);
		return 0;
	}
	return program;
}

int fgl::Model::loadResources(void)
{
	resources.vertex_buffer = make_buffer(
		GL_ARRAY_BUFFER,
		g_vertex_buffer_data,
		sizeof(g_vertex_buffer_data)
		);
	resources.element_buffer = make_buffer(
		GL_ELEMENT_ARRAY_BUFFER,
		g_element_buffer_data,
		sizeof(g_element_buffer_data)
		);

	WCHAR strbuf[100];
	GetCurrentDirectory(100, strbuf);
	resources.textures[0] = make_texture("resources\\gubbe.tga");
	resources.textures[1] = make_texture("resources\\gubbe2.tga");

	if (resources.textures[0] == 0 || resources.textures[1] == 0)
		return 0;


	resources.vertex_shader = make_shader(
		GL_VERTEX_SHADER,
		"resources\\vertex.glsl"
		);
	if (resources.vertex_shader == 0)
		return 0;

	resources.fragment_shader = make_shader(
		GL_FRAGMENT_SHADER,
		"resources\\fragment.glsl"
		);
	if (resources.fragment_shader == 0)
		return 0;

	resources.program = make_program(
		resources.vertex_shader,
		resources.fragment_shader
		);
	if (resources.program == 0)
		return 0;

	resources.uniforms.fade_factor
		= glGetUniformLocation(resources.program, "fade_factor");
	resources.uniforms.textures[0]
	= glGetUniformLocation(resources.program, "textures[0]");
	resources.uniforms.textures[1]
	= glGetUniformLocation(resources.program, "textures[1]");

	resources.attributes.position
		= glGetAttribLocation(resources.program, "position");

	return 1;

}


fgl::Model::Model(const char* name) {
	loadResources();

}

void fgl::Model::draw() {
	static int milliseconds = 1000/60;
	milliseconds+=1000/60;
	resources.fade_factor = sinf((float)milliseconds * 0.01f) * 0.5f + 0.5f;

	glUseProgram(resources.program);
	glUniform1f(resources.uniforms.fade_factor, resources.fade_factor);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, resources.textures[0]);
	glUniform1i(resources.uniforms.textures[0], 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, resources.textures[1]);
	glUniform1i(resources.uniforms.textures[1], 1);
	glBindBuffer(GL_ARRAY_BUFFER, resources.vertex_buffer);
	glVertexAttribPointer(
		resources.attributes.position,  /* attribute */
		2,                                /* size */
		GL_FLOAT,                         /* type */
		GL_FALSE,                         /* normalized? */
		sizeof(GLfloat)*2,                /* stride */
		(void*)0                          /* array buffer offset */
		);
	glEnableVertexAttribArray(resources.attributes.position);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, resources.element_buffer);
	glDrawElements(
		GL_TRIANGLE_STRIP,  /* mode */
		4,                  /* count */
		GL_UNSIGNED_SHORT,  /* type */
		(void*)0            /* element array buffer offset */
		);
	glDisableVertexAttribArray(resources.attributes.position);
}