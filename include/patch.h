
#ifndef PATCH_H
#define PATCH_H

#include "deps.h"

/* Calculation mode */
typedef enum
{
	SEQUENTIAL,
	PARALLEL,
	GPU /* TODO: Not yet operational */

} ModMode;


/* Constraint flags applied to vertices */
typedef enum
{
	SLOPE     = 0b0001,
	DIR_SLOPE = 0b0010, /* Directional derivative instead of gradient */
	ROUGHNESS = 0b0100,
	POSITION  = 0b1000

} VertexFlag;


/* A vertex */
typedef struct
{
	float h;
	float c[3]; /* Constraint values */
	int flags;  /* See type VertexFlag */

} Vertex;


/* Intermediate data for iterative modification */
typedef struct
{
	void*        mod;        /* In reality a function pointer to the modifier in question */
	const char*  out;        /* Output file, if any */
	ModMode      mode;
	Vertex*      snap; /* TODO: Not yet operational */

	int          done;       /* Non-zero when no iterations will be done anymore */
	unsigned int iterations; /* Number of iterations done */
	Vertex*      buffer;
	Vertex*      local[9];   /* The 3x3 (column-major) constraining local neighbourhood of patches */

} ModData;


/* Patch definition */
typedef struct
{
	vec3 pos; /* Position, modify at free will */

	unsigned int size; /* Width and height in vertices (always a square) */
	Vertex*      data; /* Column-major, generally speaking values are in [0,1] */
	ModData*     mods; /* Modifiers running 'in the background' */
	size_t       num_mods;
	ModMode      mode;

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
typedef int (*PatchGenerator)(unsigned int size, Vertex* data);

/**
 * Patch modifier, yet again, a function pointer.
 *
 * @param  size  Width and height of the patch in vertices.
 * @param  data  Pointer to an input data array of size * size length (column-major).
 * @param  mod   Modifier specific data to pass.
 * @return       Zero if the modification failed for some reason.
 */
typedef int (*PatchModifier)(unsigned int size, Vertex* data, ModData* mod);

/**
 * Creates a new patch of some specified size.
 *
 * @param  mode  Mode to use for calculation logic in all modifiers.
 * @param  size  Width and height of the patch in vertices.
 * @return       Zero if creation failed.
 */
int create_patch(Patch* patch, ModMode mode, unsigned int size);

/**
 * Destroys a patch.
 */
void destroy_patch(Patch* patch);

/**
 * Checks if a patch is valid, if it is destroyed, it will be rendered invalid.
 * If a piece of memory is all 0's, it will register as an invalid patch.
 */
int is_patch(Patch* patch);

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
 * @param  outs       Output files (can be NULL), elements can be NULL, must be same length as mods.
 * @param  local      The 3x3 neighborhoud of this patch at time of creation, can be NULL.
 * @return            Zero if population failed.
 */
int populate_patch(
	Patch*         patch,
	PatchGenerator generator,
	PatchModifier* mods,
	const char**   outs,
	Patch*         local[]);

/**
 * Updates a patch, i.e. runs all modifiers that still need to iterate.
 *
 * @return  Zero if some modifier failed.
 */
int update_patch(Patch* patch);


#endif
