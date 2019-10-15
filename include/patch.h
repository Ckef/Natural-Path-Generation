
#ifndef PATCH_H
#define PATCH_H

#include "cglm/cglm.h"
#include "include.h"

/* Patch definition */
typedef struct
{
	mat4 mod;  /* Model matrix, modify at free will */

	unsigned int size; /* Width and height in vertices (always a square) */
	float*       data; /* Column-major */
	GLuint       vao;
	GLuint       vertices;
	GLuint       indices;

} Patch;

/**
 * Patch generator definition, a function pointer.
 *
 * @param  size  Width and height of the patch in vertices.
 * @param  data  Output data array of size * size length (column-major).
 * @param  opt   Optional pointer to pass.
 * @return       Zero if the generation failed for some reason.
 */
typedef int (*PatchGenerator)(unsigned int size, float* data, void* opt);

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

/**
 * Populates the patch with vertex data given a generator.
 *
 * @param  generator  Function that generates a terrain. 
 * @param  opt        Optional pointer to pass.
 * @return            Zero if population failed.
 */
int populate_patch(Patch* patch, PatchGenerator generator, void* opt);


#endif