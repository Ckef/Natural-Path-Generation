
#ifndef PATCH_H
#define PATCH_H

#include "include.h"

/* Patch definition */
typedef struct
{
	unsigned int size; /* Width and height in vertices (always a square) */
	float*       data; /* Column-major */
	GLuint       vao;
	GLuint       vertices;
	GLuint       indices;

} Patch;

/**
 * Creates a new patch of some specified size.
 *
 * @param  size  Width and height of the patch in vertices.
 * @return       Zero if creation failed.
 */
int create_patch(Patch* patch, unsigned int size);

/**
 * Destroys a patch.
 */
void destroy_patch(Patch* patch);


#endif
