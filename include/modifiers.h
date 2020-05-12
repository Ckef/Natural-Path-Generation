
#ifndef MODIFIERS_H
#define MODIFIERS_H

/**
 * Flattens the terrain to 1D.
 * It copies the center (rounded down) column to all other columns.
 */
int mod_flatten(unsigned int size, float* data);


#endif
