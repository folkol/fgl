#include "model.h"

#include <Windows.h>
#include <fstream>
#include <string>
#include <vector>


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

void *read_bmp(const std::string filename, int *width, int *height) {
	/*Offset   Length   Contents
	0       2 bytes  "BM"
	2       4 bytes  Total size included "BM" magic (s)
	6       2 bytes  Reserved1
	8       2 bytes  Reserved2
	10      4 bytes  Offset bits
	14      4 bytes  Header size (n)
	18    n-4 bytes  Header (See bellow)
	14+n .. s-1      Image data

	(Header: n>12 (Microsoft Windows BMP image file))

	Offset   Length   Contents
	18      4 bytes  Width
	22      4 bytes  Height
	26      2 bytes  Planes
	28      2 bytes  Bits per Pixel
	30      4 bytes  Compression
	34      4 bytes  Image size
	38      4 bytes  X Pixels per meter
	42      4 bytes  Y Pixels per meter
	46      4 bytes  Number of Colors
	50      4 bytes  Colors Important
	54 (n-40) bytes  OS/2 new extentional fields??*/

	size_t read;
	std::vector<char> buffer;
    std::ifstream file(filename, std::ios::in | std::ios::binary);

    if (file) {
        file.seekg(0,std::ios::end);
        std::streampos length = file.tellg();
        file.seekg(0,std::ios::beg);

        buffer.resize(length);
        file.read(&buffer[0],length);

		std::streamsize bytes = file.gcount();

		file.close();
    } else {
		fprintf(stderr, "Unable to open %s for reading\n", filename);
		return NULL;
	}

	if(buffer[0] != 'B' || buffer[1] != 'M') {
		fprintf(stderr, "Unexpected magic number in %s\n", filename);
		return NULL;
	}

	*width = *(int*)&buffer[0x12];
	*height = *(int*)&buffer[0x16];
	int* image_data_offset = (int*)&buffer[10];
	short* pixel_size = (short*)&buffer[28]; 
	int row_size = ((*pixel_size*(*width) + 31)/32)*4; // Row aligned on 4 bytes

	char sample = buffer[400*3*200];
	int pixels_size = *width * *height * 3;
	unsigned char* pixels = (unsigned char*) malloc(pixels_size);
	for (int i = 0; i < *height; ++i) {
		int pixel_pointer = i*(*width)*3;
		int buffer_pointer = (*image_data_offset) + i * row_size;
		for(int j = 0; j < *width; j++) {
			// BGR order, but we handle that in the GL call to fill the texture object
			unsigned char r = buffer[buffer_pointer];
			unsigned char g = buffer[buffer_pointer + 1];
			unsigned char b = buffer[buffer_pointer + 2];
			pixels[pixel_pointer] = r;
			pixels[pixel_pointer + 1] = g; 
			pixels[pixel_pointer + 2] = b;
			pixel_pointer += 3;
			buffer_pointer += 3;
		}
	}
	
	return (void*) pixels;
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

static const GLfloat g_vertex_buffer_data_3d[] = { 
	-1.0f, -1.0f, 0.0f, 1.0f,
	1.0f, -1.0f, 0.0f, 1.0f,
	-1.0f,  1.0f, 0.0f, 1.0f,
	1.0f,  1.0f, 0.0f, 1.0f
};

static const GLushort g_element_buffer_data[] = { 0, 1, 2, 3 };


static GLuint make_texture(const std::string filename) {
	GLuint texture;
	int width, height;

	void *pixels;

	std::string suffix(".bmp");
    if(0 == filename.compare (filename.length() - suffix.length(), suffix.length(), suffix)) {
		pixels = read_bmp(filename.c_str(), &width, &height);
    } else {
        pixels = read_tga(filename.c_str(), &width, &height);
    }

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

int fgl::Model::loadResources(std::string name)
{
	resources.vertex_buffer = make_buffer(
		GL_ARRAY_BUFFER,
		g_vertex_buffer_data_3d,
		sizeof(g_vertex_buffer_data_3d)
		);
	resources.element_buffer = make_buffer(
		GL_ELEMENT_ARRAY_BUFFER,
		g_element_buffer_data,
		sizeof(g_element_buffer_data)
		);

	WCHAR strbuf[100];
	GetCurrentDirectory(100, strbuf);
	resources.textures[0] = make_texture((std::string("resources\\") + name + std::string("\\gubbe.tga")).c_str());
	resources.textures[1] = make_texture((std::string("resources\\") + name + std::string("\\gubbe.bmp")).c_str());

	if (resources.textures[0] == 0 || resources.textures[1] == 0)
		return 0;

	resources.vertex_shader = make_shader(
		GL_VERTEX_SHADER,
		(std::string("resources\\") + name + std::string("\\vertex.glsl")).c_str()
		);
	if (resources.vertex_shader == 0)
		return 0;

	resources.fragment_shader = make_shader(
		GL_FRAGMENT_SHADER,
		(std::string("resources\\") + name + std::string("\\fragment.glsl")).c_str()
		);
	if (resources.fragment_shader == 0)
		return 0;

	resources.program = make_program(
		resources.vertex_shader,
		resources.fragment_shader
		);
	if (resources.program == 0)
		return 0;

	resources.uniforms.timer
		= glGetUniformLocation(resources.program, "timer");
	resources.uniforms.textures[0]
	= glGetUniformLocation(resources.program, "textures[0]");
	resources.uniforms.textures[1]
	= glGetUniformLocation(resources.program, "textures[1]");

	resources.attributes.position
		= glGetAttribLocation(resources.program, "position");

	return 1;
}

fgl::Model::Model(const char* name) {
	loadResources(name);
}

void fgl::Model::draw() {
	static int milliseconds = 1000/60;
	milliseconds+=1000/60;
	resources.timer = (float)milliseconds * 0.001f;

	glUseProgram(resources.program);
	glUniform1f(resources.uniforms.timer, resources.timer);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, resources.textures[0]);
	glUniform1i(resources.uniforms.textures[0], 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, resources.textures[1]);
	glUniform1i(resources.uniforms.textures[1], 1);
	glBindBuffer(GL_ARRAY_BUFFER, resources.vertex_buffer);
	glVertexAttribPointer(
		resources.attributes.position,  /* attribute */
		4,                                /* size per vertex */
		GL_FLOAT,                         /* type */
		GL_FALSE,                         /* normalized? */
		sizeof(GLfloat)*4,                /* stride between vertices*/
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