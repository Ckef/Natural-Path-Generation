
#include "output.h"

/*****************************/
static float max_slope_1d(unsigned int size, float* data)
{
	/* Maximum 1D slope */
	/* Not to confuse with the gradient */
	/* To test the 1D version */
	float m = 0;
	unsigned int c, r;
	for(c = 0; c < size-1; ++c)
		for(r = 0; r < size-1; ++r)
		{
			/* Get slope in both directions */
			float s1 = data[c * size + r+1] - data[c * size + r];
			s1 = s1 > 0 ? s1 : -s1;

			float s2 = data[(c+1) * size + r] - data[c * size + r];
			s2 = s2 > 0 ? s2 : -s2;

			/* Get max slope */
			m = s1 > s2 ? (s1 > m ? s1 : m) : (s2 > m ? s2 : m);
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
int mod_stats(unsigned int size, float* data, void* opt)
{
	output("");
	output("-- Terrain Stats --");
	output("-- max slope 1D:   %f", max_slope_1d(size, data));
	output("-- total supplies: %f", total_supplies(size, data));
	output("");

	return 1;
}
