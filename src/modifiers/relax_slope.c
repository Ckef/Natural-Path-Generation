
#include "output.h"
#include "scene.h"
#include <float.h>
#include <math.h>

/* Hardcoded slope constraint for now */
#define MAX_SLOPE       0.005f
#define MAX_ITERATIONS  10000

/*****************************/
static unsigned int move_slope(
	float* p1,
	float* p2,
	float  maxSlope,
	float  scale,
	float  weight)
{
	/* TODO: figure out what to do with weight, it appears to be faster if no weight? */
	/* ???????? */
	/* Oh this is probably because we're outputting to the same grid */
	weight = 1;

	/* The current slope and the indices */
	/* a is the lowest vertex, b the highest */
	float s = (*p2 - *p1) / scale;
	float* b = (s > 0) ? p2 : p1;
	float* a = (s > 0) ? p1 : p2;
	s = (s > 0) ? s : -s;

	/* Add epsilon to the comparison, otherwise it never exits */
	/* Note: 2 * 1/weight times epsilon, cause we divide the difference by this number */
	/* Note: this is probably way inaccurate */
	/* TODO: find some accurate way to account for floating point errors */
	if(s > maxSlope + FLT_EPSILON / (.5f * weight))
	{
		/* If the slope is too great, move a and b closer to each other */
		/* The weight is so all vertex pairs can be done in parallel */
		float move = (s - maxSlope) * scale * .5f * weight;
		*a += move;
		*b -= move;

		/* Modification applied, return 0, indicating we are not done yet */
		return 0;
	}

	return 1;
}

/*****************************/
int mod_relax_slope_1d(unsigned int size, float* data, void* opt)
{
	float scale = GET_SCALE(size);

	/* Only modify the center column */
	float* mid = data + ((size >> 1) * size);

	/* Count the number of iterations */
	unsigned int i = 0;
	while(i < MAX_ITERATIONS)
	{
		int done = 1;
		++i;

		/* Loop over all pairs of vertices */
		unsigned int r;
		for(r = 0; r < size-1; ++r)
			done &= move_slope(mid + r, mid + (r+1), MAX_SLOPE, scale, .5f);

		/* Exit if no changes were made */
		if(done) break;
	}

	output("Slope relaxation took %u iterations.", i);

	return 1;
}

/*****************************/
int mod_relax_slope(unsigned int size, float* data, void* opt)
{
	float scale = GET_SCALE(size);

	/* Count the number of iterations */
	unsigned int i = 0;
	while(i < MAX_ITERATIONS)
	{
		int done = 1;
		++i;

		/* Loop over all vertices */
		unsigned int c, r;
		for(c = 0; c < size; ++c)
			for(r = 0; r < size; ++r)
			{
				/* Get the vertex in question and its two neighbours */
				/* If it is at the boundary, it gets the opposite neighbour */
				float* x = data + (c * size + r);
				float* xx = data + ((c == size-1 ? c-1 : c+1) * size + r);
				float* xy = data + (c * size + (r == size-1 ? r-1 : r+1));

				/* This scales gradient vector g by MaxSlope/|g| */
				float sx = (*xx - *x) / scale;
				float sy = (*xy - *x) / scale;
				float g = MAX_SLOPE / sqrtf(sx*sx + sy*sy);

				done &= move_slope(x, xx, (sx > 0 ? sx : -sx) * g, scale, .25f);
				done &= move_slope(x, xy, (sy > 0 ? sy : -sy) * g, scale, .25f);

				/* This applies different scales to the delta x and delta y */
				/* It retains the ratio of the two delta's squared */
				/*
				float sx = (*xx - *x) / scale;
				float sy = (*xy - *x) / scale;
				float a = sqrtf(sx*sx / (sx*sx + sy*sy));
				float b = sqrtf(sy*sy / (sx*sx + sy*sy));

				done &= move_slope(x, xx, MAX_SLOPE * a, scale, .25f);
				done &= move_slope(x, xy, MAX_SLOPE * b, scale, .25f);
				*/
			}

		/* Exit if no changes were made */
		if(done) break;
	}

	output("Slope relaxation took %u iterations.", i);

	return 1;
}
