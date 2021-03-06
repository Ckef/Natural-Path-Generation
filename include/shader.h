
#ifndef SHADER_H
#define SHADER_H

#include "deps.h"

/* Shader definition */
typedef struct
{
	GLuint program;

} Shader;


/**
 * Creates a new shader from two files.
 *
 * @param  v  String of the vertex shader filename to open.
 * @param  f  String of the fragment shader filenme to open.
 * @return    Zero if the shader creation failed.
 */
int create_shader(Shader* shader, const char* v, const char* f);

/**
 * Destroys a shader.
 */
void destroy_shader(Shader* shader);

#endif
