
#include "output.h"
#include <float.h>
#include <limits.h>

/* Hardcoded slope constraint for now */
#define MAX_SLOPE       0.005f
#define MAX_ITERATIONS  UINT_MAX

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
		{
			/* The current slope and the indices */
			/* a is the lowest vertex, b the highest */
			float s = mid[r+1] - mid[r];
			int b = s > 0;
			int a = 1-b;
			s = b ? s : -s;

			/* Add epsilon to the comparison, otherwise it never exits */
			if(s > MAX_SLOPE + FLT_EPSILON)
			{
				/* We are now modifying, so add another iteration */
				done = 0;

				/* If the slope is too great, move a and b closer to each other */
				/* Weigh the added difference by 1/2 */
				/* This weight is so all vertex pairs can be done in parallel */
				float move = (s - MAX_SLOPE) * .5f;
				mid[r+a] += move * .5f;
				mid[r+b] -= move * .5f;
			}
		}

		/* Exit if no changes were made */
		if(done) break;
	}

	output("Slope relaxation took %u iterations.", i);

	return 1;
}
