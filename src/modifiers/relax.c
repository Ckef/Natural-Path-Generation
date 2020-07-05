
#include "output.h"
#include "patch.h"
#include "scene.h"
#include <float.h>
#include <math.h>
#include <string.h>

/* Hardcoded thresholds for now */
#define S_THRESHOLD        0.00001f /* Convergence threshold of slope error */
#define R_THRESHOLD        0.04f /* Convergence threshold of roughness error */
#define MAX_ITERATIONS     10000
#define STEP_SIZE          10

/* Check if two indices are on the same column */
#define SAME_COLUMN(i,j,size) (((i)/size) == ((j)/size))

/*****************************/
static void move_slope(
	float   slope,
	float   scale,
	Vertex* o1,
	Vertex* o2,
	float   maxSlope,
	float   weight)
{
	/* The current slope and the indices */
	/* a is the lowest point, b the highest */
	Vertex* b = (slope > 0) ? o2 : o1;
	Vertex* a = (slope > 0) ? o1 : o2;

	/* Move a and b closer to each other until the slope is satisfied */
	float move = (fabs(slope) - maxSlope) * scale * (.5f * weight);
	a->h += move;
	b->h -= move;
}

/*****************************/
static int get_neighbours(
	unsigned int size,
	unsigned int ix,
	unsigned int dir,
	int*         ixx,
	int*         ixy)
{
	/* Get the point its two neighbours */
	/* This depends on the cardinal direction given by dir */
	/* dir is in { 0, 1, 2, 3 } */
	*ixx = ix +
		((dir==0) ? (int)size : (dir==1) ? -1 : (dir==2) ? -(int)size : 1);
	*ixy = ix +
		((dir==0) ? 1 : (dir==1) ? (int)size : (dir==2) ? -1 : -(int)size);

	/* Check bounds */
	if(*ixx < 0 || (unsigned int)*ixx >= size*size)
		return 0;
	if(*ixy < 0 || (unsigned int)*ixy >= size*size)
		return 0;
	if(!SAME_COLUMN(ix, (unsigned int)((dir==0||dir==2) ? *ixy : *ixx), size))
		return 0;

	/* Return non-zero if all neighbours exist */
	return 1;
}

/*****************************/
static int relax_slope(
	unsigned int size,
	unsigned int ix,
	float        scale,
	float        weight,
	Vertex*      inp,
	Vertex*      out)
{
	int done = 1;

	/* Loop over all 4 cardinal directions */
	unsigned int d;
	for(d = 0; d < 4; ++d)
	{
		/* Get the point in question and its two neighbours */
		/* It basically rotates the neighbours clockwise around their center */
		int ixx, ixy;
		if(!get_neighbours(size, ix, d, &ixx, &ixy))
			continue;

		/* This scales gradient vector g by MaxSlope/|g| */
		float sx = (inp[ixx].h - inp[ix].h) / scale;
		float sy = (inp[ixy].h - inp[ix].h) / scale;
		float g = hypotf(sx, sy);

		/* And add the oh so familiar convergence threshold to the comparison */
		/* Again for floating point errors */
		if(g > inp[ix].c[0] + S_THRESHOLD)
		{
			g = inp[ix].c[0] / g;
			move_slope(sx, scale, out + ix, out + ixx, fabs(sx) * g, weight);
			move_slope(sy, scale, out + ix, out + ixy, fabs(sy) * g, weight);

			/* Modification applied, indiciate we are not done yet */
			done = 0;
		}
	}

	return done;
}

/*****************************/
static int relax_dir_slope(
	unsigned int size,
	unsigned int ix,
	float        scale,
	float        weight,
	Vertex*      inp,
	Vertex*      out)
{
	int done = 1;

	/* Loop over all 4 cardinal directions */
	unsigned int d;
	for(d = 0; d < 4; ++d)
	{
		/* Get the point in question and its two neighbours */
		int ixx, ixy;
		if(!get_neighbours(size, ix, d, &ixx, &ixy))
			continue;

		/* This scales directional derivative d by MaxSlope/d */
		float maxSlope = hypotf(inp[ix].c[0], inp[ix].c[1]);
		float dx = inp[ix].c[0] / maxSlope;
		float dy = inp[ix].c[1] / maxSlope;
		float sx = (inp[ixx].h - inp[ix].h) / scale;
		float sy = (inp[ixy].h - inp[ix].h) / scale;
		float d = fabs(sx * dx + sy * dy);

		/* The familiar convergence threshold */
		if(d > maxSlope + S_THRESHOLD)
		{
			d = maxSlope / d;
			move_slope(sx, scale, out + ix, out + ixx, fabs(sx) * d, weight);
			move_slope(sy, scale, out + ix, out + ixy, fabs(sy) * d, weight);

			/* Modification applied, indiciate we are not done yet */
			done = 0;
		}
	}

	return done;
}

/*****************************/
static float calc_roughness(
	unsigned int size,
	Vertex*      data,
	unsigned int ix,
	float        scale)
{
	/* Loop over all neighbors and sum their differences */
	float R = 0;
	int c, r;
	for(c = -1; c <= 1; ++c)
		for(r = -1; r <= 1; ++r)
		{
			if(c == 0 && r == 0)
				continue;

			/* Check bounds */
			int ixx = ix + c * (int)size + r;
			if(ixx < 0 || (unsigned int)ixx >= size*size)
				continue;
			if(!SAME_COLUMN(ix + c * (int)size, ixx, size))
				continue;

			/* Suuuuuuuuuuuuuuuum */
			/* Note we divide by scale to get slope */
			/* This is so this metric is scale invariant */
			float s = (data[ixx].h - data[ix].h) / scale;
			R += s*s;
		}

	return sqrtf(R);
}

/*****************************/
static int relax_roughness(
	unsigned int size,
	unsigned int ix,
	float        scale,
	float        weight,
	Vertex*      inp,
	Vertex*      out)
{
	/* Calculate current roughness and check for the threshold */
	/* Without this threshold the whole landscape goes mad :( */
	float R = calc_roughness(size, inp, ix, scale);
	if(fabs(R - inp[ix].c[0]) <= R_THRESHOLD)
		return 1;

	/* Get the factor to correct the current roughness to the desired one */
	R = inp[ix].c[0] / R;

	/* Smth to store the move values in */
	float move[9] = {0};
	float dSupp = 0;

	/* Now multiply each term with our factor */
	int c, r;
	for(c = -1; c <= 1; ++c)
		for(r = -1; r <= 1; ++r)
		{
			if(c == 0 && r == 0)
				continue;

			/* check bounds */
			int ixx = ix + c * (int)size + r;
			if(ixx < 0 || (unsigned int)ixx >= size*size)
				continue;
			if(!SAME_COLUMN(ix + c * (int)size, ixx, size))
				continue;

			/* And calculate how much we want to move the point */
			/* We do not actually apply it yet */
			unsigned int im = (c+1)*3+(r+1);

			/* We actually calculate what we want to move as if the point is 1 unit away */
			/* This so it all is scale invariant */
			float s = (inp[ixx].h - inp[ix].h) / scale;
			move[im] = s * R - s;
			dSupp += move[im];
		}

	/* Now get the total amount we want to move each point and do it */
	/* The total amount of supplies changed is distributed over all points */
	/* So EMD is preserved :) */
	dSupp /= 9;
	for(c = -1; c <= 1; ++c)
		for(r = -1; r <= 1; ++r)
		{
			/* check bounds */
			int ixx = ix + c * (int)size + r;
			if(ixx < 0 || (unsigned int)ixx >= size*size)
				continue;
			if(!SAME_COLUMN(ix + c * (int)size, ixx, size))
				continue;

			/* Obviously apply the weight as well */
			float m = (move[(c+1)*3+(r+1)] - dSupp) * scale;
			out[ixx].h += m * weight;
		}

	/* Well we modified something, so return 0 */
	return 0;
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
			float s = (mid[r+1].h - mid[r].h) / scale;
			float maxSlope = 0.0025f;

			/* Add the convergence threshold to the comparison */
			/* If we do not, it may never exit due to floating point errors */
			/* We could calculate the error that could accumulate, but this is hard */
			/* So we have this hardcoded threshold :) */
			if(fabs(s) > maxSlope + S_THRESHOLD)
			{
				move_slope(s, scale, mid + r, mid + (r+1), maxSlope, 1);

				/* Modification applied, indiciate we are not done yet */
				done = 0;
			}
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

	/* Okay so before we begin, calculate the initial roughness values */
	if(mod->iterations == 0)
	{
		unsigned int ix;
		for(ix = 0; ix < size*size; ++ix)
			if(data[ix].flags & ROUGHNESS)
				data[ix].c[0] = calc_roughness(size, data, ix, scale);
	}

	/* Now define the input buffer */
	/* For parallelism we use the buffer, otherwise just data */
	Vertex* inp = mod->mode == PARALLEL ? mod->buffer : data;

	/* And also define a weight */
	/* Each point is touched 4 times by each slope constraint in all 4 cardinal directions */
	/* Each point is touched by 9 roughness constraints */
	/* So that's a weight of 1/(4*4+9) = 1/25 for any point */
	/* Do note: each point needs to have the same weight to preserve the EMD property */
	float weight = mod->mode == PARALLEL ? 1/25.0f : 1;

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
			if(inp[ix].flags & SLOPE)
				done &= relax_slope(size, ix, scale, weight, inp, data);
			if(inp[ix].flags & DIR_SLOPE)
				done &= relax_dir_slope(size, ix, scale, weight, inp, data);
			if(inp[ix].flags & ROUGHNESS)
				done &= relax_roughness(size, ix, scale, weight, inp, data);
		}

		/* Loop over all vertices again for the position constraint */
		/* It is important this is handled as last and separately */
		/* This is because it overrides the height of a vertex completely */
		/* This is the part where we are allowed to create/destroy material */
		for(ix = 0; ix < size*size; ++ix)
			if(inp[ix].flags & POSITION)
			{
				done &= (data[ix].h == inp[ix].c[2]);
				data[ix].h = inp[ix].c[2];
			}

		/* Exit if no changes were made */
		/* Or when the maximum number of iterations ended */
		if(done || mod->iterations == MAX_ITERATIONS)
		{
			output("Relaxation took %u iterations.", mod->iterations);

			free(mod->buffer);
			mod->buffer = NULL;
			mod->done = 1;

			break;
		}
	}

	return 1;
}
