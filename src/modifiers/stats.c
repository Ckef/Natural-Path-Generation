
#include "constants.h"
#include "modifiers.h"
#include "output.h"
#include "patch.h"
#include <math.h>

/* Check if two indices are on the same column */
#define SAME_COLUMN(i,j,size) ((i/size) == (j/size))

/*****************************/
static float total_supplies(
	unsigned int size,
	Vertex*      data)
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
static float max_slope_1d(
	unsigned int size,
	Vertex*      data)
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
			if(!(data[c * size + r].flags & SLOPE))
				continue;

			/* Get slope in x direction */
			float s = (data[(c+1) * size + r].h - data[c * size + r].h) / scale;
			m = s > 0 ? (s > m ? s : m) : (-s > m ? -s : m);
		}

	for(c = 0; c < size; ++c)
		for(r = 0; r < size-1; ++r)
		{
			if(!(data[c * size + r].flags & SLOPE))
				continue;

			/* Get slope in y direction */
			float s = (data[c * size + r+1].h - data[c * size + r].h) / scale;
			m = s > 0 ? (s > m ? s : m) : (-s > m ? -s : m);
		}

	return m;
}

/*****************************/
static float max_slope(
	unsigned int  size,
	Vertex*       data,
	unsigned int* count,
	unsigned int* satisfied,
	unsigned int* unsatisfied,
	float*        avgDistance)
{
	float scale = GET_SCALE(size);

	*count = 0;
	*satisfied = 0;
	*unsatisfied = 0;
	*avgDistance = 0;

	/* Maximum 2D slope, i.e. the magnitude of the gradient vector */
	float m = 0;
	unsigned int ix;
	for(ix = 0; ix < size*size; ++ix)
	{
		if(!(data[ix].flags & SLOPE))
			continue;

		++(*count);

		/* Loop over all 4 cardinal directions */
		unsigned int sat = 1;
		unsigned int d;
		for(d = 0; d < 4; ++d)
		{
			int ixx, ixy;
			if(!get_neighbours(size, ix, d, &ixx, &ixy))
				continue;

			/* Get the gradient */
			float sx = (data[ixx].h - data[ix].h) / scale;
			float sy = (data[ixy].h - data[ix].h) / scale;
			float g = sqrtf(sx * sx + sy * sy);

			*avgDistance += fmaxf(0.0f, g - data[ix].c[0]);
			sat &= (g <= data[ix].c[0] + S_THRESHOLD);
			m = g > m ? g : m;
		}

		if(sat)
			++(*satisfied);
		else
			++(*unsatisfied);
	}

	if(*count)
		*avgDistance /= (*count) * 4;

	return m;
}

/*****************************/
static void count_dir_slope(
	unsigned int  size,
	Vertex*       data,
	unsigned int* count,
	unsigned int* satisfied,
	unsigned int* unsatisfied,
	float*        avgDistance)
{
	float scale = GET_SCALE(size);

	*count = 0;
	*satisfied = 0;
	*unsatisfied = 0;
	*avgDistance = 0;

	/* Just count satisfied and unsatisfied, there is no global "maximum" or anything */
	unsigned int ix;
	for(ix = 0; ix < size*size; ++ix)
	{
		if(!(data[ix].flags & DIR_SLOPE))
			continue;

		++(*count);

		/* Loop over all 4 cardinal directions */
		unsigned int sat = 1;
		unsigned int d;
		for(d = 0; d < 4; ++d)
		{
			int ixx, ixy;
			if(!get_neighbours(size, ix, d, &ixx, &ixy))
				continue;

			/* This scales directional derivative d by MaxSlope/d */
			float maxSlope = hypotf(data[ix].c[0], data[ix].c[1]);
			float dx = data[ix].c[0] / maxSlope;
			float dy = data[ix].c[1] / maxSlope;
			float sx = (data[ixx].h - data[ix].h) / scale;
			float sy = (data[ixy].h - data[ix].h) / scale;
			float d = fabs(sx * dx + sy * dy);

			*avgDistance += fmaxf(0.0f, d - maxSlope);
			sat &= (d <= maxSlope + S_THRESHOLD);
		}

		if(sat)
			++(*satisfied);
		else
			++(*unsatisfied);
	}

	if(*count)
		*avgDistance /= (*count) * 4;
}

/*****************************/
static void count_roughness(
	unsigned int  size,
	Vertex*       data,
	unsigned int* count,
	unsigned int* satisfied,
	unsigned int* unsatisfied,
	float*        avgDistance)
{
	float scale = GET_SCALE(size);

	*count = 0;
	*satisfied = 0;
	*unsatisfied = 0;
	*avgDistance = 0;

	/* Just count satisfied and unsatisfied, there is no global "maximum" or anything */
	unsigned int ix;
	for(ix = 0; ix < size*size; ++ix)
	{
		if(!(data[ix].flags & ROUGHNESS))
			continue;

		float R = calc_roughness(size, data, ix, scale);
		float dist = fabs(R - data[ix].c[0]);

		++(*count);
		*avgDistance += dist;

		if(dist <= R_THRESHOLD)
			++(*satisfied);
		else
			++(*unsatisfied);
	}

	if(*count)
		*avgDistance /= *count;
}

/*****************************/
static void count_position(
	unsigned int  size,
	Vertex*       data,
	unsigned int* count,
	unsigned int* satisfied,
	unsigned int* unsatisfied,
	float*        avgDistance)
{
	*count = 0;
	*satisfied = 0;
	*unsatisfied = 0;
	*avgDistance = 0;

	/* Just count satisfied and unsatisfied, there is no global "maximum" or anything */
	unsigned int ix;
	for(ix = 0; ix < size*size; ++ix)
	{
		if(!(data[ix].flags & POSITION))
			continue;

		++(*count);
		*avgDistance += fabs(data[ix].h - data[ix].c[2]);

		if(data[ix].h == data[ix].c[2])
			++(*satisfied);
		else
			++(*unsatisfied);
	}

	if(*count)
		*avgDistance /= *count;
}

/*****************************/
int mod_stats(unsigned int size, Vertex* data, ModData* mod)
{
	/* Get all the data */
	unsigned int numS, numD, numR, numP;
	unsigned int satS, unsatS;
	unsigned int satD, unsatD;
	unsigned int satR, unsatR;
	unsigned int satP, unsatP;
	float distS, distD, distR, distP;

	float mSlope = max_slope(size, data, &numS, &satS, &unsatS, &distS);
	count_dir_slope(size, data, &numD, &satD, &unsatD, &distD);
	count_roughness(size, data, &numR, &satR, &unsatR, &distR);
	count_position(size, data, &numP, &satP, &unsatP, &distP);

	/* Output it */
	output("");
	output("-- TERRAIN STATS");
	output("-- total points:   %u", size * size);
	output("-- total supplies: %f", total_supplies(size, data));
	output("-- max slope 1D:   %f", max_slope_1d(size, data));
	output("-- max slope 2D:   %f", mSlope);
	output("--");
	output("-- Total constraints");
	output("-- #slope:         %u", numS);
	output("-- #directional:   %u", numD);
	output("-- #roughness:     %u", numR);
	output("-- #position:      %u", numP);
	output("--");
	output("-- Satisfied constraints");
	output("-- #slope:         %u", satS);
	output("-- #directional:   %u", satD);
	output("-- #roughness:     %u", satR);
	output("-- #position:      %u", satP);
	output("--");
	output("-- Unsatisfied constraints");
	output("-- #slope:         %u", unsatS);
	output("-- #directional:   %u", unsatD);
	output("-- #roughness:     %u", unsatR);
	output("-- #position:      %u", unsatP);
	output("--");
	output("-- Average distance from goal");
	output("-- slope:          %f", distS);
	output("-- directional:    %f", distD);
	output("-- roughness:      %f", distR);
	output("-- position:       %f", distP);

	output("");

	/* Open a file to append this terrain's stats to it */
	/* Obviously only do this when an output file was given */
	if(mod->out != NULL)
	{
		FILE* f = fopen(mod->out, "a");
		if(f == NULL)
			throw_error("Could not open file: %s", mod->out);
		else
		{
			/* Write stats to file, do this with a JSON object on a new line */
			/* This object contains a bunch of fields for each constraint */
			/* The constraints are: */
			/* _s  slope (or gradient) */
			/* _d  directional derivative */
			/* _r  roughness */
			/* _p  position */
			/* All the fields then are: */
			/* n_s, n_d, n_r, n_p  The number of constraints */
			/* s_s, s_d, s_r, s_p  Satisfied constraints */
			/* u_s, u_d, u_r, u_p  Unsatisfied constraints */
			/* d_s, d_d, d_r, d_p  Average distance from goal */
			fprintf(f,
				"{ \"n_s\" : %u, \"n_d\" : %u, \"n_r\" : %u, \"n_p\" : %u,"
				"  \"s_s\" : %u, \"s_d\" : %u, \"s_r\" : %u, \"s_p\" : %u,"
				"  \"u_s\" : %u, \"u_d\" : %u, \"u_r\" : %u, \"u_p\" : %u,"
				"  \"d_s\" : %f, \"d_d\" : %f, \"d_r\" : %f, \"d_p\" : %f }\n",
				numS, numD, numR, numP,
				satS, satD, satR, satP,
				unsatS, unsatD, unsatR, unsatP,
				distS, distD, distR, distP);

			fclose(f);
			output("Terrain stats have been written to file: %s", mod->out);
		}
	}

	/* We don't need to iterate this modifier */
	mod->done = 1;
	return 1;
}
