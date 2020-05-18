
#ifndef MODIFIERS_H
#define MODIFIERS_H

#include "patch.h"

/**
 * Simply outputs some statistics about the terrain.
 */
int mod_stats(unsigned int size, float** data, ModData* mod);

/**
 * Flattens the terrain to 1D.
 * It copies the center (rounded down) column to all other columns.
 */
int mod_flatten(unsigned int size, float** data, ModData* mod);

/**
 * Applies iterative relaxation to solve 1D slope constraints.
 * It computes this on the center (rounded down) column.
 */
int mod_relax_slope_1d(unsigned int size, float** data, ModData* mod);

/**
 * Applies iterative relaxation to solve 2D gradient constraints.
 */
int mod_relax_slope(unsigned int size, float** data, ModData* mod);


#endif
