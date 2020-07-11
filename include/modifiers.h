
#ifndef MODIFIERS_H
#define MODIFIERS_H

#include "patch.h"

/**
 * Returns the indices of a point its two neighbors for calculations based on gradient.
 *
 * @param  size  Width and height of the patch data in vertices.
 * @param  ix    Index of the point we want its neighbours from.
 * @param  dir   Direction of the quadrant, in { 0, 1, 2, 3 } (starting at north east going clockwise).
 * @param  ixx   Output index of the neighbour in the local x direction.
 * @param  ixy   Output index of the neighbor in the local y direction.
 */
int get_neighbours(
	unsigned int size,
	unsigned int ix,
	unsigned int dir,
	int*         ixx,
	int*         ixy);

/**
 * Calculates the roughness of the terrain at a given position.
 *
 * @param  size   Width and height of the patch data in vertices.
 * @param  data   Data array of size * size length (column-major).
 * @param  ix     Position we want the roughness of.
 * @param  scale  Scale of the terrain.
 */
float calc_roughness(
	unsigned int size,
	Vertex*      data,
	unsigned int ix,
	float        scale);

/**
 * Simply outputs some statistics about the terrain.
 */
int mod_stats(unsigned int size, Vertex* data, ModData* mod);

/**
 * Outputs the terrain to an output .json file.
 */
int mod_output(unsigned int size, Vertex* data, ModData* mod);

/**
 * Outputs the terrain's flags to an output .json file.
 */
int mod_output_flags(unsigned int size, Vertex* data, ModData* mod);

/**
 * Outputs the constraint values to an output .json file.
 */
int mod_output_constrs(unsigned int size, Vertex* data, ModData* mod);

/**
 * Makes a subdivision of the terrain.
 * Each region has its own constraints (i.e. flags).
 */
int mod_subdivide(unsigned int size, Vertex* data, ModData* mod);

/**
 * Applies iterative relaxation to solve all 2D constraints.
 * Constraints:
 * - gradient (slope)
 * - directional derivative (dir_slope)
 * - roughness
 * - position
 */
int mod_relax(unsigned int size, Vertex* data, ModData* mod);

/**
 * Flattens the terrain to 1D.
 * It copies the center (rounded down) column to all other columns.
 */
int mod_flatten(unsigned int size, Vertex* data, ModData* mod);

/**
 * Applies iterative relaxation to solve 1D slope constraints.
 * It computes this on the center (rounded down) column.
 */
int mod_relax_slope_1d(unsigned int size, Vertex* data, ModData* mod);


#endif
