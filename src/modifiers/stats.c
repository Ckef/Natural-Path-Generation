
#include "output.h"
#include "patch.h"
#include "scene.h"
#include <math.h>

/* Check if two indices are on the same column */
#define SAME_COLUMN(i,j,size) ((i/size) == (j/size))

/*****************************/
static float max_slope_1d(unsigned int size, Vertex* data, int flags)
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
			if(!(data[c * size + r].flags & flags))
				continue;

			/* Get slope in x direction */
			float s = (data[(c+1) * size + r].h - data[c * size + r].h) / scale;
			m = s > 0 ? (s > m ? s : m) : (-s > m ? -s : m);
		}

	for(c = 0; c < size; ++c)
		for(r = 0; r < size-1; ++r)
		{
			if(!(data[c * size + r].flags & flags))
				continue;

			/* Get slope in y direction */
			float s = (data[c * size + r+1].h - data[c * size + r].h) / scale;
			m = s > 0 ? (s > m ? s : m) : (-s > m ? -s : m);
		}

	return m;
}

/*****************************/
static float max_slope(unsigned int size, Vertex* data, int flags)
{
	float scale = GET_SCALE(size);

	/* Maximum 2D slope, i.e. the magnitude of the gradient vector */
	float m = 0;
	unsigned int ix;
	for(ix = 0; ix < size*size; ++ix)
	{
		if(!(data[ix].flags & flags))
			continue;

		/* Loop over all 4 cardinal directions */
		unsigned int d;
		for(d = 0; d < 4; ++d)
		{
			/* Get the vertex in question and its two neighbours */
			/* This loops over all 4 cardinal directions */
			/* Rotating the neighbours clockwise around their center */
			/* Yes they're signed integers... */
			int ixx = ix +
				((d == 0) ? (int)size : (d == 1) ? -1 : (d == 2) ? -(int)size : 1);
			int ixy = ix +
				((d == 0) ? 1 : (d == 1) ? (int)size : (d == 2) ? -1 : -(int)size);

			/* Check bounds */
			if(ixx < 0 || (unsigned int)ixx >= size*size)
				continue;
			if(ixy < 0 || (unsigned int)ixy >= size*size)
				continue;
			if(!SAME_COLUMN(ix, (unsigned int)((d==0||d==2) ? ixy : ixx), size))
				continue;

			/* Get the gradient */
			float sx = (data[ixx].h - data[ix].h) / scale;
			float sy = (data[ixy].h - data[ix].h) / scale;
			float g = sqrtf(sx * sx + sy * sy);

			m = g > m ? g : m;
		}
	}

	return m;
}

/*****************************/
static float total_supplies(unsigned int size, Vertex* data)
{
	/* Total supplies */
	/* Thinking in Earth Mover's Distance terms, */
	/* The total supply of a landscape is its total volume */
	/* i.e. sum all the weights (heights) of all suppliers/consumers (vertices) */
	float t = 0;
	unsigned int i;
	for(i = 0; i < size * size; ++i)
		t += data[i].h;

	return t;
}

/*****************************/
int mod_stats(unsigned int size, Vertex* data, ModData* mod)
{
	output("");
	output("-- Terrain Stats --");
	output("-- max slope 1D:   %f", max_slope_1d(size, data, F_SLOPE));
	output("-- max slope 2D:   %f", max_slope(size, data, F_SLOPE));
	output("-- total supplies: %f", total_supplies(size, data));
	output("");

	/* We don't need to iterate this modifier */
	mod->done = 1;
	return 1;
}
