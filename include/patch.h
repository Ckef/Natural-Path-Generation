
#ifndef PATCH_H
#define PATCH_H

#include "deps.h"

/* Patch definition */
typedef struct
{
	vec3 pos; /* Position, modify at free will */

	unsigned int size; /* Width and height in vertices (always a square) */
	float*       data; /* Column-major, generally speaking values are in [0,1] */
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
 * Patch modifier.
 *
 * @param  size  Width and height of the patch in vertices.
 * @param  data  Modifiable input data array of size * size length (column-major).
 * @return       Zero if the modification failed for some reason.
 */
typedef int (*PatchModifier)(unsigned int size, float* data);

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
 * Draws a patch.
 *
 * Note: Assumes some shader program is bound and set up.
 */
void draw_patch(Patch* patch);

/**
 * Populates the patch with vertex data given a generator and modifiers.
 *
 * @param  generator  Function that generates a terrain.
 * @param  opt        Optional pointer to pass.
 * @param  numMods    Number of modifiers passed.
 * @param  mods       Array of numMods modifiers (can be NULL).
 * @return            Zero if population failed.
 */
int populate_patch(
	Patch*         patch,
	PatchGenerator generator,
	void*          opt,
	unsigned int   numMods,
	PatchModifier* mods);


#endif
