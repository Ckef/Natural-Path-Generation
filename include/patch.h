
#ifndef PATCH_H
#define PATCH_H

#include "deps.h"

/* Intermediate data for iterative modification */
typedef struct
{
	void*        mod;        /* In reality a function pointer to the modifier in question */
	float*       snap;       /* Snapshot buffer of the terrain, mainly for statistics */

	int          done;       /* Non-zero when no iterations will be done anymore */
	unsigned int iterations; /* Number of iterations done */
	float*       buffer;

} ModData;

/* Patch definition */
typedef struct
{
	vec3 pos; /* Position, modify at free will */

	unsigned int size; /* Width and height in vertices (always a square) */
	float*       data; /* Column-major, generally speaking values are in [0,1] */
	ModData*     mods; /* Modifiers running 'in the background' */
	size_t       num_mods;

	GLuint       vao;
	GLuint       vertices;
	GLuint       indices;

} Patch;

/**
 * Patch generator definition, a function pointer.
 *
 * @param  size  Width and height of the patch in vertices.
 * @param  data  Output data array of size * size length (column-major).
 * @return       Zero if the generation failed for some reason.
 */
typedef int (*PatchGenerator)(unsigned int size, float* data);

/**
 * Patch modifier, yet again, a function pointer.
 *
 * @param  size  Width and height of the patch in vertices.
 * @param  data  Pointer to an input data array of size * size length (column-major).
 * @param  mod   Modifier specific data to pass.
 * @return       Zero if the modification failed for some reason.
 */
typedef int (*PatchModifier)(unsigned int size, float* data, ModData* mod);

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
 * @param  mods       Array of modifiers (can be NULL), last element must be NULL.
 * @return            Zero if population failed.
 */
int populate_patch(Patch* patch, PatchGenerator generator, PatchModifier* mods);

/**
 * Updates a patch, i.e. runs all modifiers that still need to iterate.
 *
 * @return  Zero if some modifier failed.
 */
int update_patch(Patch* patch);


#endif
