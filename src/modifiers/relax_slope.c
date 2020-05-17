
#include "output.h"
#include <float.h>
#include <math.h>

/* Hardcoded slope constraint for now */
#define MAX_SLOPE       0.005f
#define MAX_ITERATIONS  10000

/*****************************/
static unsigned int move_slope(float* p1, float* p2, float maxSlope, float weight)
{
	/* The current slope and the indices */
	/* a is the lowest vertex, b the highest */
	float s = *p2 - *p1;
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
		float move = (s - maxSlope) * .5f * weight;
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
			done &= move_slope(mid + r, mid + (r+1), MAX_SLOPE, .5f);

		/* Exit if no changes were made */
		if(done) break;
	}

	output("Slope relaxation took %u iterations.", i);

	return 1;
}

/*****************************/
int mod_relax_slope(unsigned int size, float* data, void* opt)
{
	/* Count the number of iterations */
	unsigned int i = 0;
	while(i < MAX_ITERATIONS)
	{
		int done = 1;
		++i;

		/* Loop over all coordinates that have a neighbour above and below */
		/* TODO: Does not consider the boundaries */
		unsigned int c, r;
		for(c = 0; c < size-1; ++c)
			for(r = 0; r < size-1; ++r)
			{
				float* x = data + (c * size + r);
				float* xx = data + ((c+1) * size + r);
				float* xy = data + (c * size + r+1);

				float sx2 = (*xx - *x) * (*xx - *x);
				float sy2 = (*xy - *x) * (*xy - *x);
				float a = sqrtf(sx2 / (sx2 + sy2));
				float b = sqrtf(sy2 / (sx2 + sy2));

				done &= move_slope(x, xx, MAX_SLOPE * a, .25f);
				done &= move_slope(x, xy, MAX_SLOPE * b, .25f);
			}

		/* Exit if no changes were made */
		if(done) break;
	}

	output("Slope relaxation took %u iterations.", i);

	return 1;
}
