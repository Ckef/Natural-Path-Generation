
#ifndef MODIFIERS_H
#define MODIFIERS_H

/**
 * Flattens the terrain to 1D.
 * It copies the center (rounded down) column to all other columns.
 */
int mod_flatten(unsigned int size, float* data, void* opt);

/**
 * Applies iterative relaxation to solve 1D slope constraints.
 * It computes this on the center (rounded down) column.
 */
int mod_relax_slope_1d(unsigned int size, float* data, void* opt);


#endif
