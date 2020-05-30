
#include "output.h"
#include "patch.h"
#include "scene.h"
#include <float.h>
#include <math.h>
#include <string.h>

/* Hardcoded slope constraint for now */
#define MAX_SLOPE       0.005f
#define MAX_ITERATIONS  10000
#define STEP_SIZE       10

/*****************************/
static unsigned int move_slope(
	float   delta,
	float   scale,
	Vertex* o1,
	Vertex* o2,
	float   maxSlope,
	float   weight)
{
	/* The current slope and the indices */
	/* a is the lowest vertex, b the highest */
	float s = delta / scale;
	Vertex* b = (s > 0) ? o2 : o1;
	Vertex* a = (s > 0) ? o1 : o2;
	s = (s > 0) ? s : -s;

	/* Add epsilon to the comparison, otherwise it never exits */
	/* Note: 2 * 1/weight times epsilon, cause we divide the difference by this number */
	/* Note: this is probably way inaccurate */
	/* TODO: find some accurate way to account for floating point errors */
	if(s > maxSlope + FLT_EPSILON / (.5f * weight))
	{
		/* If the slope is too great, move a and b closer to each other */
		/* The weight is so all vertex pairs can be done in parallel */
		float move = (s - maxSlope) * scale * (.5f * weight);
		a->h += move;
		b->h -= move;

		/* Modification applied, return 0, indicating we are not done yet */
		return 0;
	}

	return 1;
}

/*****************************/
int mod_relax_slope_1d(unsigned int size, Vertex* data, ModData* mod)
{
	float scale = GET_SCALE(size);

	/* Only modify the center column */
	Vertex* mid = data + ((size >> 1) * size);

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
			/* Use a weight of 1 because this one works sequentially only */
			float d = mid[r+1].h - mid[r].h;
			done &= move_slope(d, scale, mid + r, mid + (r+1), MAX_SLOPE, 1);
		}

		/* Exit if no changes were made */
		if(done) break;
	}

	output("Slope relaxation took %u iterations.", i);

	/* We don't need to iterate this modifier */
	mod->done = 1;
	return 1;
}

/*****************************/
int mod_relax_slope(unsigned int size, Vertex* data, ModData* mod)
{
	float scale = GET_SCALE(size);

	/* Allocate a buffer for input if it wasn't there yet */
	/* We just leave it empty if no parallelism allowed */
	size_t buffSize = sizeof(Vertex) * size * size;
	if(mod->mode == PARALLEL && mod->buffer == NULL)
	{
		mod->buffer = malloc(buffSize);
		if(mod->buffer == NULL)
		{
			throw_error("Failed to allocate memory for a relaxation buffer.");
			return 0;
		}
	}

	/* Now define the input buffer */
	/* For parallelism we use the buffer, otherwise just data */
	/* Also define a weight */
	Vertex* inp = mod->mode == PARALLEL ? mod->buffer : data;
	float w = mod->mode == PARALLEL ? .25f : 1;

	/* Count the number of iterations */
	unsigned int i = 0;
	while(i < STEP_SIZE)
	{
		int done = 1;
		++i;
		++mod->iterations;

		/* Prepare input buffer if parallel */
		if(mod->mode == PARALLEL)
			memcpy(mod->buffer, data, buffSize);

		/* Loop over all vertices */
		unsigned int c, r;
		for(c = 0; c < size; ++c)
			for(r = 0; r < size; ++r)
			{
				/* Check if the gradient constraint applies */
				if(!(inp[c * size + r].flags & 1))
					continue;

				/* Get the vertex in question and its two neighbours */
				/* If it is at the boundary, it gets the opposite neighbour */
				/* TODO: Obviously this does not consider the bottom and left neighbor */
				unsigned int ix = c * size + r;
				unsigned int ixx = (c == size-1 ? c-1 : c+1) * size + r;
				unsigned int ixy = c * size + (r == size-1 ? r-1 : r+1);

				/* This scales gradient vector g by MaxSlope/|g| */
				float dx = inp[ixx].h - inp[ix].h;
				float dy = inp[ixy].h - inp[ix].h;
				float sx = dx / scale;
				float sy = dy / scale;
				float g = MAX_SLOPE / sqrtf(sx*sx + sy*sy);

				done &= move_slope(dx, scale, data + ix, data + ixx, (sx > 0 ? sx : -sx) * g, w);
				done &= move_slope(dy, scale, data + ix, data + ixy, (sy > 0 ? sy : -sy) * g, w);

				/* This applies different scales to the delta x and delta y */
				/* It retains the ratio of the two directional delta's squared */
				/*
				float a = sqrtf(sx*sx / (sx*sx + sy*sy)) * MAX_SLOPE;
				float b = sqrtf(sy*sy / (sx*sx + sy*sy)) * MAX_SLOPE;

				done &= move_slope(dx, scale, data + ix, data + ixx, a, w);
				done &= move_slope(dy, scale, data + ix, data + ixy, b, w);
				*/
			}

		/* Exit if no changes were made */
		/* Or when the maximum number of iterations ended */
		if(done || mod->iterations == MAX_ITERATIONS)
		{
			output("Slope relaxation took %u iterations.", mod->iterations);

			free(mod->buffer);
			mod->buffer = NULL;
			mod->done = 1;

			break;
		}
	}

	return 1;
}
