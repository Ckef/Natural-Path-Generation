
#include "output.h"
#include "patch.h"
#include "scene.h"
#include <float.h>
#include <math.h>
#include <string.h>

/* Hardcoded slope constraint for now */
#define MAX_SLOPE       0.0025f
#define CONV_THRESHOLD  0.00001f /* Convergence threshold of slope error */
#define MAX_ITERATIONS  10000
#define STEP_SIZE       10

/* Get if a point is a corner or edge, given an index and the scale of the terrain */
#define IS_CORNER(i,size) (i==0||i==size-1||i==size*size-1||i==(size-1)*size)
#define IS_EDGE(i,size)   (i<size||i>(size-1)*size||i%size==0||(i+1)%size==0)

/* Check if two indices are on the same column */
#define SAME_COLUMN(i,j,size) ((i/size) == (j/size))

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

	/* Add the convergence threshold to the comparison */
	/* If we do not, it may never exit due to floating point errors */
	/* We could calculate the error that could accumulate, but this is hard */
	/* So we have this hardcoded threshold :) */
	if(s > maxSlope + CONV_THRESHOLD)
	{
		/* If the slope is too great, move a and b closer to each other */
		float move = (s - maxSlope) * scale * .5f;
		a->h += move * weight;
		b->h -= move * weight;

		/* Modification applied, return 0, indicating we are not done yet */
		return 0;
	}

	return 1;
}

/*****************************/
static int relax_slope(
	unsigned int size,
	unsigned int ix,
	float        weight,
	Vertex*      inp,
	Vertex*      out)
{
	int done = 1;
	float scale = GET_SCALE(size);

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

		/* This scales gradient vector g by MaxSlope/|g| */
		float dx = inp[ixx].h - inp[ix].h;
		float dy = inp[ixy].h - inp[ix].h;
		float sx = dx / scale;
		float sy = dy / scale;
		float g = MAX_SLOPE / sqrtf(sx*sx + sy*sy);

		done &= move_slope(dx, scale, out + ix, out + ixx, (sx > 0 ? sx : -sx) * g, weight);
		done &= move_slope(dy, scale, out + ix, out + ixy, (sy > 0 ? sy : -sy) * g, weight);

		/* This applies different scales to the delta x and delta y */
		/* It retains the ratio of the two directional delta's squared */
		/*
		float a = sqrtf(sx*sx / (sx*sx + sy*sy)) * MAX_SLOPE;
		float b = sqrtf(sy*sy / (sx*sx + sy*sy)) * MAX_SLOPE;

		done &= move_slope(dx, scale, out + ix, out + ixx, a, weight);
		done &= move_slope(dy, scale, out + ix, out + ixy, b, weight);
		*/
	}

	return done;
}

/*****************************/
static int relax_dir_slope(
	unsigned int size,
	unsigned int ix,
	float        weight,
	Vertex*      inp,
	Vertex*      out)
{
	return 1;
}

/*****************************/
static int relax_roughness(
	unsigned int size,
	unsigned int ix,
	float        weight,
	Vertex*      inp,
	Vertex*      out)
{
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
int mod_relax(unsigned int size, Vertex* data, ModData* mod)
{
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
	Vertex* inp = mod->mode == PARALLEL ? mod->buffer : data;

	/* And also define a weight */
	/* Each point is touched 4 times by each of the 4 cardinal directions */
	/* So that's a weight of 1/(4*4) = 1/16 for any point */
	/* Do note: each vertex needs to have the same weight to preserve the EMD property */
	float weight = mod->mode == PARALLEL ? 1/16.0f : 1;

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

		/* Loop over all vertices and apply the relevant constraints */
		unsigned int ix;
		for(ix = 0; ix < size*size; ++ix)
		{
			if(inp[ix].flags & F_SLOPE)
				done &= relax_slope(size, ix, weight, inp, data);
			if(inp[ix].flags & F_DIR_SLOPE)
				done &= relax_dir_slope(size, ix, weight, inp, data);
			if(inp[ix].flags & F_ROUGHNESS)
				done &= relax_roughness(size, ix, weight, inp, data);
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
