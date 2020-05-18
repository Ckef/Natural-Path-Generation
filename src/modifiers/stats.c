
#include "output.h"
#include "patch.h"
#include "scene.h"
#include <math.h>

/*****************************/
static float max_slope_1d(unsigned int size, float* data)
{
	float scale = GET_SCALE(size);

	/* Maximum 1D slope */
	/* Not to confuse with the gradient */
	/* To test the 1D version */
	float m = 0;
	unsigned int c, r;
	for(c = 0; c < size-1; ++c)
		for(r = 0; r < size; ++r)
		{
			/* Get slope in x direction */
			float s = (data[(c+1) * size + r] - data[c * size + r]) / scale;
			m = s > 0 ? (s > m ? s : m) : (-s > m ? -s : m);
		}

	for(c = 0; c < size; ++c)
		for(r = 0; r < size-1; ++r)
		{
			/* Get slope in y direction */
			float s = (data[c * size + r+1] - data[c * size + r]) / scale;
			m = s > 0 ? (s > m ? s : m) : (-s > m ? -s : m);
		}

	return m;
}

/*****************************/
static float max_slope(unsigned int size, float* data)
{
	float scale = GET_SCALE(size);

	/* Maximum 2D slope, i.e. the magnitude of the gradient vector */
	float m = 0;
	unsigned int c, r;
	for(c = 0; c < size; ++c)
		for(r = 0; r < size; ++r)
		{
			/* Get the gradient */
			/* If it is at the boundary, it gets the opposite neighbour */
			float sx = (data[(c == size-1 ? c-1 : c+1) * size + r] - data[c * size + r]) / scale;
			float sy = (data[c * size + (r == size-1 ? r-1 : r+1)] - data[c * size + r]) / scale;
			float g = sqrtf(sx * sx + sy * sy);

			m = g > m ? g : m;
		}

	return m;
}

/*****************************/
static float total_supplies(unsigned int size, float* data)
{
	/* Total supplies */
	/* Thinking in Earth Mover's Distance terms, */
	/* The total supply of a landscape is its total volume */
	/* i.e. sum all the weights (heights) of all suppliers/consumers (vertices) */
	float t = 0;
	unsigned int i;
	for(i = 0; i < size * size; ++i)
		t += data[i];

	return t;
}

/*****************************/
int mod_stats(unsigned int size, float** data, ModData* mod)
{
	output("");
	output("-- Terrain Stats --");
	output("-- max slope 1D:   %f", max_slope_1d(size, *data));
	output("-- max slope 2D:   %f", max_slope(size, *data));
	output("-- total supplies: %f", total_supplies(size, *data));
	output("");

	/* We don't need to iterate this modifier */
	mod->done = 1;
	return 1;
}
