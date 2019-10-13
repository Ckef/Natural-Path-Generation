
#include "include.h"
#include "io.h"
#include "shader.h"
#include <stdio.h>
#include <stdlib.h>

/*****************************/
inline int create_shader(Shader* shader, const char* v, const char* f)
{
	/* Open the files */
	FILE* vf = fopen(v, "r");
	FILE* ff = fopen(f, "r");

	if(vf == NULL || ff == NULL)
	{
		throw_error("Could not open file: %s", vf == NULL ? v : f);
		return 0;
	}

	/* Read the files */
	long int vfs, ffs;
	fseek(vf, 0, SEEK_END);
	fseek(ff, 0, SEEK_END);
	vfs = ftell(vf);
	ffs = ftell(ff);
	fseek(vf, 0, SEEK_SET);
	fseek(ff, 0, SEEK_SET);

	char* vs = malloc(vfs + 1);
	char* fs = malloc(ffs + 1);

	if(vs == NULL || fs == NULL)
	{
		throw_error("Failed to allocate memory for a file string.");
		free(vs);
		free(fs);
		fclose(vf);
		fclose(ff);

		return 0;
	}

	fread(vs, 1, vfs, vf);
	fread(fs, 1, ffs, ff);
	fclose(vf);
	fclose(ff);
	vs[vfs] = '\0';
	fs[ffs] = '\0';

	/* Create GPU shaders */
	GLuint vert = glCreateShader(GL_VERTEX_SHADER);
	GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vert, 1, (const GLchar* const*)&vs, NULL);
	glShaderSource(frag, 1, (const GLchar* const*)&fs, NULL);
	glCompileShader(vert);
	glCompileShader(frag);
	free(vs);
	free(fs);

	int s1, s2;
	char log[512];
	glGetShaderiv(vert, GL_COMPILE_STATUS, &s1);
	glGetShaderiv(frag, GL_COMPILE_STATUS, &s2);

	if(!s1 || !s2)
	{
		glGetShaderInfoLog(!s1 ? vert : frag, 512, NULL, log);
		throw_error("Shader could not be compiled: %s: %s", !s1 ? v : f, log);

		glDeleteShader(vert);
		glDeleteShader(frag);

		return 0;
	}

	/* Link the shaders into a program */
	shader->program = glCreateProgram();
	glAttachShader(shader->program, vert);
	glAttachShader(shader->program, frag);
	glLinkProgram(shader->program);
	glDeleteShader(vert);
	glDeleteShader(frag);

	glGetProgramiv(shader->program, GL_LINK_STATUS, &s1);

	if(!s1)
	{
		glGetProgramInfoLog(shader->program, 512, NULL, log);
		throw_error("Shaders could not be linked: %s - %s: %s", v, f, log);
		glDeleteProgram(shader->program);

		return 0;
	}

	/* Print that the shader files are correct */
	output("Shaders succesfully loaded, compiled and linked: %s %s", v, f);

	return 1;
}

/*****************************/
void destroy_shader(Shader* shader)
{
	glDeleteProgram(shader->program);
}
